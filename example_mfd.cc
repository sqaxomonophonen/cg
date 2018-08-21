#include "cgmain.h"
#include "cgutil.h"

const bool is_highpoly = false;

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
const double button_spacer_margin = 0.2;
const double button_spacer_r = 0.05;
const double corner_indent_size = 1.1;
const double corner_indent_depth = 0.4;
const double corner_indent_r = 0.5;


void cgmain()
{
	double linear_deflection = is_highpoly ? 0.1 : 2.0;
	bool is_relative = false;
	double angular_deflection = is_highpoly ? 0.1 : 2.0;
	mkobj("MFD", linear_deflection, is_relative, angular_deflection) {
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

		auto button_spacers = []{
			auto spacer = []{
				translate(0, button_spacer_margin) rotate(-90_X) {
					capsule(button_spacer_r, margin - button_spacer_margin*2);
				}
			};
			translate(0,0,depth) {
				for (int i = 0; i <= n_buttons_per_side; i++) {
					double d = side/2 + (i - (double)n_buttons_per_side/2) * button_spacing;
					translate(d) spacer();
					translate(d, side-margin) spacer();
					translate(0,d) rotate(-90_Z) spacer();
					translate(side-margin,d) rotate(-90_Z) spacer();
				}
			}
		};

		auto corner_indent = []{
			auto mk_indent = []{
				const double extend = 1;
				translate(-extend,-extend,depth-corner_indent_depth) z_rounded_box(
					corner_indent_size + extend,
					corner_indent_size + extend,
					corner_indent_depth + extend,
					corner_indent_r
				);
			};
			mk_indent();
			translate(side,0) rotate(90_Z) mk_indent();
			translate(0,side) rotate(-90_Z) mk_indent();
			translate(side,side) rotate(180_Z) mk_indent();
		};

		translate(-side/2, -side/2) {
			fillet(is_highpoly ? 0.05 : 0) cut {
				panel_outline();
				screen_cut();
				button_holes();
				corner_indent();
			}
			translate(margin/2,margin/2,depth-screen_depth-1) cullbox(side-margin,side-margin,1);
			translate(-1,-1,-0.9) cullbox(side+2,side+2,1);
			if (is_highpoly) button_spacers();
		}
	}
}
