#ifndef CGUTIL_H

void z_rounded_quad(double sx, double sy, double r) {
	face {
		double cc = sin(M_PI/4)*r;
		move_to(r, 0);
		line_to(sx-r, 0);
		circle_arc_to(v3(sx-r+cc,r-cc), v3(sx,r));
		line_to(sx, sy-r);
		circle_arc_to(v3(sx-r+cc, sy-r+cc), v3(sx-r, sy));
		line_to(r, sy);
		circle_arc_to(v3(r-cc, sy-r+cc), v3(0, sy-r));
		line_to(0, r);
		circle_arc_to(v3(r-cc, r-cc), v3(r, 0));
	}
}

void z_rounded_box(double sx, double sy, double sz, double r)
{
	prism(v3(0,0,sz)) z_rounded_quad(sx, sy, r);
}

void cone_corner_cutter(double sx, double sy, double z0, double z1, double z2, double z3, double r0, double r1)
{
	fuse {
		const double dr = r1-r0;
		const double dr2 = dr*2;
		const double wd = z2-z1;
		translate(0,0,z0) translate(dr, dr) z_rounded_box(sx-dr2, sy-dr2, z2-z0, r0);
		translate(0,0,z1) {
			auto mk_cone = [&](){ cone(r0,r1,wd); };
			translate(r1, r1) mk_cone();
			translate(sx-r1,r1) mk_cone();
			translate(r1,sy-r1) mk_cone();
			translate(sx-r1,sy-r1) mk_cone();
		}

		const double wh = 3;
		const double wlx = sx - r1*2;
		const double wly = sy - r1*2;
		translate(0,0,z1 + wh) {
			auto mk_wedge = [&](const double wl) { wedge(wh,r1-r0,wl,wh-wd); };
			translate(r1, sy-dr) rotate(90_Y) mk_wedge(wlx);
			translate(sx-r1, dr) rotate(180_Z) rotate(90_Y) mk_wedge(wlx);
			translate(dr, r1) rotate(90_Z) rotate(90_Y) mk_wedge(wly);
			translate(sx-dr,sy-r1) rotate(-90_Z) rotate(90_Y) mk_wedge(wly);
		}
	}
}

#define CGUTIL_H
#endif
