#include "cgmain.h"

void cgmain()
{
	mkobj("BooleanOps") {
		auto objs = []{
			translate(-1,-1,-1) fillet(0.1) box(2,2,2);
			auto cyl = []{
				translate(-2_Z) cylinder(0.8, 4);
			};
			cyl();
			rotate(90_X) cyl();
			rotate(90_Y) cyl();
		};

		/* simple examples; cut/fuse/common (aka difference/union/intersection) */
		translate(-4) cut    objs();
		              fuse   objs();
		translate(4)  common objs();

		/* do some stress testing; perforate a box with a bunch of cylinders */
		translate(0,10) translate(-5,-5) {
			cut {
				translate(-0.5_Z) box(10,10,1);
				for (int x = 0; x <= 10; x++) {
					for (int y = 0; y <= 10; y++) {
						translate(x, y, -1) cylinder(0.3, 4);
					}
				}
				for (int x = 0; x <= 10; x++) {
					translate(x) rotate(-90_X) translate(-1_Z) cylinder(0.3, 12);
				}
				for (int y = 0; y <= 10; y++) {
					translate(0,y) rotate(90_Y) translate(-1_Z) cylinder(0.3, 12);
				}
			}
		}
	}
}
