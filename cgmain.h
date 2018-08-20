#ifndef CGMAIN_H

#include <stdio.h>
#include <stdlib.h>
#include "cg.h"

void cgmain(); // <<< this is your entry point; define this function

void init_main(int argc, char** argv);
int main(int argc, char** argv)
{
	init_main(argc, argv);
	cgmain();
	return EXIT_SUCCESS;
}

#define CGMAIN_H
#endif
