#ifndef CG_H

#include <math.h>

struct v3 {
	union {
		struct { double x,y,z; };
		double s[3];
	};

	v3(double x=0, double y=0, double z=0) : x(x),y(y),z(z) {}

	v3 operator - () const { return v3(-x,-y,-z); }
	v3 operator + () const { return *this; }

	v3 operator - (const v3& other) const {
		v3 r;
		for (int i = 0; i < 3; i++) r.s[i] = s[i] - other.s[i];
		return r;
	}

	v3 operator + (const v3& other) const {
		v3 r;
		for (int i = 0; i < 3; i++) r.s[i] = s[i] + other.s[i];
		return r;
	}


	v3 operator * (double scalar) const {
		v3 r;
		for (int i = 0; i < 3; i++) r.s[i] = s[i] * scalar;
		return r;
	}
	v3 operator / (double scalar) const {
		v3 r;
		for (int i = 0; i < 3; i++) r.s[i] = s[i] / scalar;
		return r;
	}
	double dot(const v3& other) const {
		double sum = 0;
		for (int i = 0; i < 3; i++) sum += s[i]*other.s[i];
		return sum;
	}
	double length() const { return sqrt(this->dot(*this)); }
	v3 unit() const { return *this / length(); }

	v3 cross(const v3& other) const {
		v3 r;
		r.x = y*other.z - z*other.y;
		r.y = z*other.x - x*other.z;
		r.z = x*other.y - y*other.x;
		return r;
	}
};

v3 x_axis(double x=1) { return v3(x,0,0); }
v3 y_axis(double y=1) { return v3(0,y,0); }
v3 z_axis(double z=1) { return v3(0,0,z); }

/* suffix axis literals, e.g. "90_X" is x_axis(90) which is v3(90,0,0) */
v3 operator "" _X(long double x)        { return x_axis(x); }
v3 operator "" _X(unsigned long long x) { return x_axis(x); }
v3 operator "" _Y(long double y)        { return y_axis(y); }
v3 operator "" _Y(unsigned long long y) { return y_axis(y); }
v3 operator "" _Z(long double z)        { return z_axis(z); }
v3 operator "" _Z(unsigned long long z) { return z_axis(z); }


void _grp0();
int _grp1();
#define _GRP0 for(
#define _GRP1 ,_grp0();_grp1();)

#define mkobj(...)     _GRP0 _grp_mkobj(__VA_ARGS__)     _GRP1
void _grp_mkobj(const char* name);

void box(const v3& size);
void box(double sx=1, double sy=1, double sz=1);
void wedge(double sx=1, double sy=1, double sz=1, double ltx=1);
void sphere(double radius=1);
void cylinder(double radius=1, double height=1);
void cone(double r0=1, double r1=0.5, double height=1);

#define translate(...) _GRP0 _grp_translate(__VA_ARGS__) _GRP1
void _grp_translate(const v3& v);
void _grp_translate(double x=0, double y=0, double z=0);

#define rotate(...)    _GRP0 _grp_rotate(__VA_ARGS__)    _GRP1
void _grp_rotate(double degrees, const v3& axis);
void _grp_rotate(const v3& axis_degrees);

#define group          _GRP0 _grp_group()                _GRP1
void _grp_group();

#define cut            _GRP0 _grp_cut()                  _GRP1
void _grp_cut();

#define fuse           _GRP0 _grp_fuse()                 _GRP1
void _grp_fuse();

#define common         _GRP0 _grp_common()               _GRP1
void _grp_common();

#define fillet(x)      _GRP0 _grp_fillet(x)              _GRP1
void _grp_fillet(double radius);

#define face           _GRP0 _grp_face()                 _GRP1
void _grp_face();

void move_to(const v3& p);
void move_to(double x=0, double y=0, double z=0);
void line_to(const v3& p);
void line_to(double x=0, double y=0, double z=0);
void circle_arc_to(const v3& point_on_circle, const v3& end_point);
// TODO OpenCASCADE has all the conic sections: circle, ellipse, parabola, hyperbola

#define prism(v)       _GRP0 _grp_prism(v)                   _GRP1
void _grp_prism(const v3& v);

static inline double deg2rad(double deg)
{
	return (deg*2.0*M_PI)/360.0;
}

#define CG_H
#endif
