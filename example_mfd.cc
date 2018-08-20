#include "cgmain.h"
#include "cgutil.h"

double side = 15;
double depth = 1;
double margin = 1.5;
double corner_radius = 0.5;
double screen_r1 = 0.5;
double screen_r2 = 0.3;
double screen_depth = 0.5;

void cgmain()
{
	mkobj("MFD") {
		auto panel_outline = []{
			z_rounded_box(side, side, depth, corner_radius);
		};

		auto screen_cut = []{
			double smm = side - margin*2;
			translate(margin,margin,-depth) z_rounded_box(smm, smm, depth*3, screen_r2);
			translate(0,0,depth-screen_depth) {
				translate(margin+screen_r2, margin+screen_r2) cone(screen_r2, screen_r1, screen_depth);
				translate(side-margin-screen_r2, margin+screen_r2) cone(screen_r2, screen_r1, screen_depth);
				translate(margin+screen_r2, side-margin-screen_r2) cone(screen_r2, screen_r1, screen_depth);
				translate(side-margin-screen_r2, side-margin-screen_r2) cone(screen_r2, screen_r1, screen_depth);
			}
			double wh = 3;
			double wl = side - margin*2 - screen_r2*2;
			translate(0,0,wh+depth-screen_depth) {
				translate(margin + screen_r2, side - margin) rotate(90_Y) wedge(wh,screen_r1-screen_r2,wl,wh - screen_depth);
				translate(side - margin - screen_r2, margin) rotate(180_Z) rotate(90_Y) wedge(wh,screen_r1-screen_r2,wl,wh - screen_depth);
				translate(margin, margin + screen_r2) rotate(90_Z) rotate(90_Y) wedge(wh,screen_r1-screen_r2,wl,wh - screen_depth);
				translate(side - margin, side - margin - screen_r2) rotate(-90_Z) rotate(90_Y) wedge(wh,screen_r1-screen_r2,wl,wh - screen_depth);
			}
		};

		translate(-side/2, -side/2) {
			cut {
				panel_outline();
				screen_cut();
			}
		}

	}
}
