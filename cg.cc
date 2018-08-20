#include <stdio.h>
#include <assert.h>

#include <vector>
#include <map>

// OpenCASCADE
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Face.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <gp_Ax1.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <Poly.hxx>

#include "cg.h"

struct node;

node* tree_root = NULL;
std::vector<node*> node_stack;


struct triangle {
	int v0,v1,v2,n;
};

struct mesh {
	std::vector<v3> vertices;
	std::vector<v3> normals;
	std::vector<triangle> triangles;
};

char* run_write_obj = NULL;
bool run_dump_tree = false;

enum node_type {
	MKOBJ = 1,
	GROUP,
	TRANSLATE,
	ROTATE,
	CUT,
	COMMON,
	FUSE,
	BOX,
	SPHERE,
	CYLINDER,
};

struct v3_less {
	bool operator()(const v3& a, const v3& b) const  {
		v3 d = a-b;
		if (d.x != 0) return d.x < 0;
		if (d.y != 0) return d.y < 0;
		return d.z < 0;
	}
};

static v3 gp_Pnt_to_v3(const gp_Pnt& p)
{
	return v3(p.X(), p.Y(), p.Z());
}

struct node {
	enum node_type type;
	std::vector<node*> children;

	union {
		struct {
			const char* name;
		} mkobj;

		struct {
			v3 v;
		} translate;

		struct {
			double degrees;
			v3 axis;
		} rotate;

		struct {
			v3 size;
		} box;

		struct {
			double radius;
		} sphere;

		struct {
			double radius, height;
		} cylinder;
	};

	node(enum node_type type) : type(type) {}

	bool is_leaf() {
		switch (type) {
		case MKOBJ:
		case GROUP:
		case TRANSLATE:
		case ROTATE:
		case CUT:
		case COMMON:
		case FUSE:
			return false;
		case BOX:
		case SPHERE:
		case CYLINDER:
			return true;
		}
		assert(!"unhandled type");
	}

	void dump_rec(int depth = 0)
	{
		auto tab = [](int depth){ for (int i = 0; i < depth; i++) printf("   "); };
		tab(depth);

		switch (type) {
		case MKOBJ: printf("mkobj(\"%s\")", mkobj.name); break;
		case GROUP: printf("group"); break;
		case TRANSLATE: printf("translate(%f,%f,%f)", translate.v.x, translate.v.y, translate.v.z); break;
		case ROTATE: printf("rotate(degrees=%f, axis={%f,%f,%f})", rotate.degrees, rotate.axis.x, rotate.axis.y, rotate.axis.z); break;
		case CUT: printf("cut"); break;
		case COMMON: printf("common"); break;
		case FUSE: printf("fuse"); break;
		case BOX: printf("box(%f,%f,%f)", box.size.x, box.size.y, box.size.z); break;
		case SPHERE: printf("sphere(%f)", sphere.radius); break;
		case CYLINDER: printf("cylinder(r=%f,h=%f)", cylinder.radius, cylinder.height); break;
		}

		if (is_leaf()) {
			printf(";\n");
		} else {
			printf(" {\n");
			for (auto it = children.begin(); it != children.end(); it++) {
				(*it)->dump_rec(depth+1);
			}
			tab(depth); printf("}\n");
		}
	}

	TopoDS_Shape build_group_shape(int offset = 0)
	{
		TopoDS_Compound shp;
		BRep_Builder b;
		b.MakeCompound(shp);
		for (int i = offset; i < children.size(); i++) {
			b.Add(shp, children[i]->build_shape_rec());
		}
		return shp;
	}

	TopoDS_Shape build_transform(gp_Trsf tx)
	{
		return BRepBuilderAPI_Transform(build_group_shape(), tx, true).Shape();
	}

	TopoDS_Shape build_shape_rec()
	{
		switch (type) {
		case MKOBJ:
		case GROUP:
			return build_group_shape();

		case TRANSLATE: {
			gp_Trsf tx;
			tx.SetTranslation(gp_Vec(translate.v.x, translate.v.y, translate.v.z));
			return build_transform(tx);
		}

		case ROTATE: {
			gp_Trsf tx;
			tx.SetRotation(
				gp_Ax1(gp_Pnt(), gp_Dir(rotate.axis.x, rotate.axis.y, rotate.axis.z)),
				deg2rad(rotate.degrees)
			);
			return build_transform(tx);
		}

		case BOX: return BRepPrimAPI_MakeBox(box.size.x, box.size.y, box.size.z);
		case CYLINDER: return BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(), gp::DZ()), cylinder.radius, cylinder.height);
		case SPHERE: return BRepPrimAPI_MakeSphere(sphere.radius);

		case CUT:
		case FUSE:
		case COMMON: {
			if (children.size() == 0) return TopoDS_Shape();
			if (children.size() == 1) return children[0]->build_shape_rec();
			TopoDS_Shape a = children[0]->build_shape_rec();
			TopoDS_Shape b = build_group_shape(1);
			if (type == CUT) {
				return BRepAlgoAPI_Cut(a, b);
			} else if (type == FUSE) {
				return BRepAlgoAPI_Fuse(a, b);
			} else if (type == COMMON) {
				return BRepAlgoAPI_Common(a, b);
			} else {
				assert(!"unhandled type");
			}
		}

		}

		assert(!"unhandled type");
	}

	mesh* build_mesh(TopoDS_Shape& shp)
	{
		mesh* m = new mesh;

		/* TODO the parameters should come from mkobj().. also, I might
		 * want two sets ... one for low poly and one for high poly?
		 * depends on whether I need high poly or not... */
		BRepMesh_IncrementalMesh(shp, 1, false, 0.5);

		std::map<v3,int,v3_less> vertex_map;
		int n_duplicate_vertices = 0;

		int offset = 0;
		for (TopExp_Explorer it(shp, TopAbs_FACE); it.More(); it.Next()) {
			TopoDS_Face fac = TopoDS::Face(it.Current());
			TopAbs_Orientation face_orientation = fac.Orientation();
			TopLoc_Location location;
			Handle(Poly_Triangulation) pt = BRep_Tool::Triangulation(fac, location);
			if (pt.IsNull()) continue;

			const TColgp_Array1OfPnt& vertex_nodes = pt->Nodes();
			const Poly_Array1OfTriangle& triangles = pt->Triangles();

			for (int i = 0; i < vertex_nodes.Length(); i++) {
				v3 point = gp_Pnt_to_v3(vertex_nodes(i+1).Transformed(location));
				if (!vertex_map.count(point)) {
					vertex_map[point] = m->vertices.size();
					m->vertices.push_back(point);
				} else {
					n_duplicate_vertices++;
				}
			}

			std::map<v3,int,v3_less> normal_map;

			int n = pt->NbTriangles();
			for (int i = 0; i < n; i++) {
				Standard_Integer vni0, vni1, vni2;
				triangles(i+1).Get(vni0, vni1, vni2);

				const v3& p0 = gp_Pnt_to_v3(vertex_nodes(vni0));
				const v3& p1 = gp_Pnt_to_v3(vertex_nodes(vni1));
				const v3& p2 = gp_Pnt_to_v3(vertex_nodes(vni2));

				int vi0 = vertex_map[p0];
				int vi1 = vertex_map[p1];
				int vi2 = vertex_map[p2];

				triangle tri;
				if (face_orientation == TopAbs_Orientation::TopAbs_FORWARD) {
					tri.v0 = vi0;
					tri.v1 = vi1;
					tri.v2 = vi2;
				} else {
					tri.v2 = vi0;
					tri.v1 = vi1;
					tri.v0 = vi2;
				}

				const v3& normal = ((p1-p0).cross(p2-p0)).unit();

				if (normal_map.count(normal) == 0) {
					normal_map[normal] = m->normals.size();
					m->normals.push_back(normal);
				}

				tri.n = normal_map[normal];

				m->triangles.push_back(tri);
			}

			offset += vertex_nodes.Length();
		}
		return m;
	}

	void leave_mkobj()
	{
		assert(type == MKOBJ);

		if (run_dump_tree) dump_rec();

		TopoDS_Shape shp = build_shape_rec();

		if (run_write_obj) {
			mesh* mesh = build_mesh(shp);

			auto str_concat = [](const char* s1, const char* s2) -> char* {
				char* result = (char*) malloc(strlen(s1)+strlen(s2)+1);
				assert(result != NULL);
				strcpy(result, s1);
				strcat(result, s2);
				return result;
			};

			auto fopen_for_write = [](char* path) -> FILE* {
				FILE* f = fopen(path, "wb");
				if (f == NULL) {
					fprintf(stderr, "could not open %s: %s\n", path, strerror(errno));
					exit(EXIT_FAILURE);
				}
				return f;
			};

			char* filename_obj = str_concat(run_write_obj, ".obj");
			char* filename_mtl = str_concat(run_write_obj, ".mtl");

			FILE* file_obj = fopen_for_write(filename_obj);
			FILE* file_mtl = fopen_for_write(filename_mtl);

			/* seems Y is up in Wavefront OBJ, so Blender actually
			 * swizzles the input coordinates; guess I have to
			 * unswizzle them then! */
			auto wavefront_obj_v3_swizzle = [](const v3& v) -> v3 {
				return v3(v.x, v.z, -v.y);
			};

			/* write .obj */
			fprintf(file_obj, "mtllib %s\n", filename_mtl);
			fprintf(file_obj, "o %s\n", tree_root->mkobj.name);
			for (auto it = mesh->vertices.begin(); it != mesh->vertices.end(); it++) {
				const v3 p = wavefront_obj_v3_swizzle(*it);
				fprintf(file_obj, "v %.6f %.6f %.6f\n", p.x, p.y, p.z);
			}
			for (auto it = mesh->normals.begin(); it != mesh->normals.end(); it++) {
				const v3 n = wavefront_obj_v3_swizzle(*it);
				fprintf(file_obj, "vn %.6f %.6f %.6f\n", n.x, n.y, n.z);
			}
			fprintf(file_obj, "usemtl Mat\n");
			fprintf(file_obj, "s off\n");
			for (auto it = mesh->triangles.begin(); it != mesh->triangles.end(); it++) {
				const triangle t = (*it);
				fprintf(file_obj, "f %d//%d %d//%d %d//%d\n", t.v0+1, t.n+1, t.v1+1, t.n+1, t.v2+1, t.n+1);
			}

			/* write .mtl */
			fprintf(file_mtl, "newmtl Mat\n");
			fprintf(file_mtl, "Ns 0\n");
			fprintf(file_mtl, "Ka 0.000000 0.000000 0.000000\n");
			fprintf(file_mtl, "Kd 0.8 0.8 0.8\n");
			fprintf(file_mtl, "Ks 0.8 0.8 0.8\n");
			fprintf(file_mtl, "d 1\n");
			fprintf(file_mtl, "illum 2\n");

			fclose(file_mtl);
			fclose(file_obj);
		}

		tree_root = NULL;
	}

	void leave() {
		switch (type) {
		case MKOBJ: leave_mkobj(); break;
		default: break;
		}
	}
};

static node* node_stack_top()
{
	assert(node_stack.size() > 0);
	return node_stack.back();
}

static void push_node(node* n)
{
	assert(!node_stack_top()->is_leaf());
	node_stack_top()->children.push_back(n);
}

static void enter_node(node* n)
{
	push_node(n);
	node_stack.push_back(n);
}

static void leave_node()
{
	assert(node_stack.size() > 0);
	node_stack.back()->leave();
	node_stack.pop_back();
}

void _grp_mkobj(const char* name)
{
	if (tree_root != NULL) {
		assert(!"mkobj() cannot be nested");
	}
	tree_root = new node(MKOBJ);
	tree_root->mkobj.name = name;
	node_stack.push_back(tree_root);
}

void _grp_translate(const v3& v)
{
	node* n = new node(TRANSLATE);
	n->translate.v = v;
	enter_node(n);
}

void _grp_translate(double x, double y, double z)
{
	_grp_translate(v3(x,y,z));
}

void _grp_rotate(double degrees, const v3& axis)
{
	node* n = new node(ROTATE);
	n->rotate.degrees = degrees;
	n->rotate.axis = axis;
	enter_node(n);
}

void _grp_rotate(const v3& axis_degrees)
{
	_grp_rotate(axis_degrees.length(), axis_degrees.unit());
}

void _grp_group()
{
	enter_node(new node(GROUP));
}

void _grp_cut()
{
	enter_node(new node(CUT));
}

void _grp_fuse()
{
	enter_node(new node(FUSE));
}

void _grp_common()
{
	enter_node(new node(COMMON));
}

void box(const v3& size)
{
	node* n = new node(BOX);
	n->box.size = size;
	push_node(n);
}

void box(double sx, double sy, double sz)
{
	box(v3(sx,sy,sz));
}

void sphere(double radius)
{
	node* n = new node(SPHERE);
	n->sphere.radius = radius;
	push_node(n);
}

void cylinder(double radius, double height)
{
	node* n = new node(CYLINDER);
	n->cylinder.radius = radius;
	n->cylinder.height = height;
	push_node(n);
}

int _HX_BEGIN = 0;
void _grp0()
{
	_HX_BEGIN = 1;
}
int _grp1()
{
	if (_HX_BEGIN) {
		assert(!node_stack_top()->is_leaf());
		_HX_BEGIN = 0;
		return 1;
	} else {
		leave_node();
		return 0;
	}
}

void init_main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <opts...>\n", argv[0]);
		fprintf(stderr, "  --write-obj <name>   writes Wavefront OBJ to <name>.obj and <name>.mtl\n");
		fprintf(stderr, "  --dump-tree          dumps node tree to stdout\n");
		exit(EXIT_FAILURE);
	}

	char* store_for = NULL;
	char** store_arg = NULL;

	for (int i = 1; i < argc; i++) {
		char* arg = argv[i];
		if (store_arg) {
			if (arg[0] == '-') {
				fprintf(stderr, "missing argument for %s\n", store_for);
				exit(EXIT_FAILURE);
			}
			*store_arg = arg;
			store_for = NULL;
			store_arg = NULL;
		} else {
			if (strcmp(arg, "--write-obj") == 0) {
				store_for = arg;
				store_arg = &run_write_obj;
			} else if (strcmp(arg, "--dump-tree") == 0) {
				run_dump_tree = true;
			} else {
				fprintf(stderr, "invalid arg: %s\n", arg);
				exit(EXIT_FAILURE);
			}
		}
	}

	if (store_for) {
		fprintf(stderr, "missing argument for %s\n", store_for);
		exit(EXIT_FAILURE);
	}
}
