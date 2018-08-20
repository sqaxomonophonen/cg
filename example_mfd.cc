#include "cgmain.h"
#include "cgutil.h"

const double side = 15;
const double depth = 1;
const double margin = 1.5;
const double corner_radius = 0.5;
const double screen_r1 = 0.5;
const double screen_r2 = 0.3;
const double screen_depth = 0.5;

void cgmain()
{
	mkobj("MFD") {
		auto panel_outline = []{
			z_rounded_box(side, side, depth, corner_radius);
		};

		auto screen_cut = []{
			translate(margin,margin) cone_corner_cutter(
				side-margin*2, side-margin*2,
				-1, depth-screen_depth, depth, depth+1,
				screen_r2, screen_r1
			);
		};

		translate(-side/2, -side/2) {
			cut {
				panel_outline();
				screen_cut();
			}
		}
	}
}
