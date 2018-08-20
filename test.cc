#include "cgmain.h"

void cgmain()
{
	mkobj("Thingy") {
		cut {
			translate(-1,-1) box(2,2,1);
			translate(-1_Z) cylinder(0.5, 3);
		}
	}

}
