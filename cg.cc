#include <stdio.h>
#include <assert.h>

#include <vector>

// OpenCASCADE
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <gp_Ax1.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>

#include "cg.h"

struct node;

node* tree_root = NULL;
std::vector<node*> node_stack;

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

	void _tab(int depth)
	{
		for (int i = 0; i < depth; i++) printf("   ");
	}

	void dump_rec(int depth = 0)
	{
		_tab(depth);

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
			_tab(depth); printf("}\n");
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

		case BOX: return BRepPrimAPI_MakeBox(box.size.x, box.size.y, box.size.y);
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

	void leave_mkobj()
	{
		assert(type == MKOBJ);
		dump_rec();

		// BRepBuilderAPI_GTransform til at lave en transform...

		build_shape_rec();

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

int init_main(int argc, char** argv)
{
	return 0;
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
