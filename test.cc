#include "cgmain.h"

void cgmain()
{
	mkobj("Thingy") {
		cut {
			translate(-1,-1,-1) box(2,2,2);
			translate(-1_Z) cylinder(0.5, 3);
			rotate(90_X) translate(-1_Z) cylinder(0.5, 3);
			rotate(90_Y) translate(-1_Z) cylinder(0.5, 3);
		}
	}

}
