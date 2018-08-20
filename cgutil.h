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

#define CGUTIL_H
#endif
