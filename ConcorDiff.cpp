 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License as published by the Free Software Foundation; either
  * version 2.1 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Lesser General Public License for more details.
  * 
  * You should have received a copy of the GNU Lesser General Public
  * License along with this library; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
  *
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unicode.h"
#include "Text_tokens.h"
#include "String_hash.h"
#include "List_int.h"
#include "Alphabet.h"
#include "Matches.h"
#include "Concordance.h"
#include "FileName.h"
#include "Copyright.h"
#include "LocatePattern.h"
#include "Diff.h"
#include "IOBuffer.h"



void usage() {
printf("%s",COPYRIGHT);
printf("Usage: ConcorDiff <concor1> <concor2> <out> <font> <size>\n");
printf("   <concor1> : the first concord.ind file\n");
printf("   <concor2> : the second concord.ind file\n");
printf("   <out> : the result HTML file\n");
printf("   <font> : name of font\n");
printf("   <size> : size of font\n");
printf("\nProduces an HTML file that shows differences between input\n");
printf("concordance files.\n");
}


int main(int argc,char** argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();
if (argc!=6) {
	usage();
	return 0;
}
diff(argv[1],argv[2],argv[3],argv[4],argv[5]);
return 0;
}
