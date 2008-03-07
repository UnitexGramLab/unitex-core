 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
//
//
//

#define _UNICODE
#include <time.h>

#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Unicode.h"
#include "etc.h"
#include "segtype.h" // define type of segment
#include "segment.h"
#include "Copyright.h"

void makeSegments(char *ifn,char *ofn,int sz)
{
	segmentation seg;
	seg.segmentFile(ifn,ofn,sz);
}


static void
usage(int flag)
{
printf("%s",COPYRIGHT);
	printf("Usage: TokenKr [-o outfilename] filename[.txt]\n");
	printf(" -d[0-5] : sizeof mem buff 2**n Mega\n");
	printf(" -o: output filename select\n");
	exit(flag);
}
#ifdef DELETE
static void debugmain()
{

   char buffer[_MAX_PATH];

   /* Get the current working directory: */
   if( _getcwd( buffer, _MAX_PATH ) == NULL )
      perror( "_getcwd error" );
   else
      printf( "%s\n", buffer );
}
#endif // DELETE
int
main(int argc,char *argv[])
{
	char *ofile_name = NULL;

	int argcount=0;
	int statFlag = 0;
	int decoFlag = 0;
	int allFlag  = 0;
	int listFlag = 0;
	int i;
	int displayLineNumber = 50;
	int memBufferSize = 0;
	
    if(argc == 1) usage(0);
	for(i = 1; i < argc;i++){
		switch(*argv[i]){
		case '-':
		case '+':
			switch(*(argv[i]+1)){
			case 'a':
				allFlag = (*argv[i] == '-') ? -1 : 1;break;
			case 's':
				statFlag =(*argv[i] == '-') ? -1 : 1;break;
			case 'd':
				memBufferSize = (int)*(argv[i]+2) - (int)'0'; 
				if( (memBufferSize < 0) ||
						(memBufferSize > 5)) 
						usage(1);
				memBufferSize = (int)pow(2,memBufferSize);
				break;
			case 'l':
				listFlag =(*argv[i] == '-') ? -1 : 1; break;
			case 'o':
				i++;
				ofile_name= new char[strlen(argv[i])+1];
				argcount++;
				strcpy(ofile_name,argv[i]);
				break;
			case 'n':
				i++;
				displayLineNumber = atoi(argv[i]);
				if(!displayLineNumber) 
					displayLineNumber = 50;
				break;
			default:
				usage(1);
			}	
			argcount++;
		default:
			break;

		}
	}


	if((argc - argcount) != 2){
		usage(1);
	}
	if(allFlag){
		if(allFlag <1){
			statFlag = 	decoFlag = 	listFlag = 0;
		} else {
			statFlag = 	decoFlag = 	listFlag = 1;
		}
	}

//	debugmain();
	if(!memBufferSize){
		memBufferSize = MAX_UNIT_FILE_SIZE;
	} else {
		memBufferSize *= 0x400000;
	}
		
//	localSetForLangueKorean();

	makeSegments(argv[argc-1],ofile_name,memBufferSize);

	if(ofile_name)	delete ofile_name;
	return(0);
}

