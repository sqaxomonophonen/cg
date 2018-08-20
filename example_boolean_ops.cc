#include "cgmain.h"

void cgmain()
{
	mkobj("BooleanOps") {
		auto cylinder_cross = []{
			translate(-2_Z) cylinder(0.5, 4);
			rotate(90_X) translate(-2_Z) cylinder(0.5, 4);
			rotate(90_Y) translate(-2_Z) cylinder(0.5, 4);
		};

		translate(-4) cut {
			translate(-1,-1,-1) box(2,2,2);
			cylinder_cross();
		}

		fuse {
			translate(-1,-1,-1) box(2,2,2);
			cylinder_cross();
		}

		translate(4) common {
			translate(-1,-1,-1) box(2,2,2);
			cylinder_cross();
		}

	}
}
