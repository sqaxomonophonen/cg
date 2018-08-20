#include "cgmain.h"
#include "cgutil.h"

const double side = 15;
const double depth = 1;
const double margin = 1.5;
const double corner_radius = 0.5;
const double screen_r1 = 0.5;
const double screen_r2 = 0.3;
const double screen_depth = 0.5;
const int n_buttons_per_side = 5;
const double button_spacing = 1.4;
const double button_size = 0.9;

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

		auto button_holes = []{
			auto button_hole = [](double sx, double sy) {
				translate(-sx/2, -sy/2, -1) box(sx, sy, depth+2);
			};

			for (int i = 0; i < n_buttons_per_side; i++) {
				double d = side/2 + (i - (double)n_buttons_per_side/2 + 0.5) * button_spacing;
				auto std_button_hole = [&]() {
					button_hole(button_size, button_size);
				};
				translate(d,margin/2) std_button_hole();
				translate(d,side-margin/2) std_button_hole();
				translate(margin/2,d) std_button_hole();
				translate(side-margin/2,d) std_button_hole();
			}

			auto xx_button = [&](double x0, double x1) {
				double sx = x1-x0;
				double dx = x0+sx/2;
				translate(dx,margin/2) button_hole(sx, button_size);
			};

			xx_button(
				margin + screen_r2,
				side/2 - button_spacing * ((double)n_buttons_per_side/2) - (button_spacing - button_size)/2
			);

			xx_button(
				side/2 + button_spacing * ((double)n_buttons_per_side/2) + (button_spacing - button_size)/2,
				side - margin - screen_r2
			);
		};

		translate(-side/2, -side/2) {
			cut {
				panel_outline();
				screen_cut();
				button_holes();
			}
		}
	}
}
