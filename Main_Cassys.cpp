/*
 * Main_Cassys.cpp
 *
 *  Created on: 2 avr. 2010
 *      Author: David Nott
 */

#include "IOBuffer.h"
#include "Cassys.h"


int main(int argc,char* argv[]) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

return main_Cassys(argc,argv);
}


