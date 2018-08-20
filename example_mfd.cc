#include "cgmain.h"
#include "cgutil.h"

double size = 10;

void cgmain()
{
	mkobj("MFD") {
		translate(-size/2, -size/2) {
			z_rounded_box(10,10,1,0.5);
		}
	}
}
