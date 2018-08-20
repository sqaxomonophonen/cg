#include "cgmain.h"

void cgmain()
{
	mkobj("BooleanOps") {
		auto objs = []{
			translate(-1,-1,-1) box(2,2,2);
			auto cyl = []{
				translate(-2_Z) cylinder(0.8, 4);
			};
			cyl();
			rotate(90_X) cyl();
			rotate(90_Y) cyl();
		};

		translate(-4) cut    objs();
		              fuse   objs();
		translate(4)  common objs();
	}
}
