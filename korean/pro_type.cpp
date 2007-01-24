 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdlib.h>

using namespace std;
#include "unicode.h"
#include "etc.h"
#include "pro_type.h"
class pro_associ proType;
	unsigned int LEintRead4(unsigned char *a){
		unsigned int ret  = 0;
		for( int i  = 0; i < 4;i++)	ret = ret * 256 + a[i];
		return(ret);
	};
	unsigned int LEintRead3(unsigned char *a){
		unsigned int ret  = 0;
		for( int i  = 0; i < 3;i++)	ret = ret * 256 + a[i];
		return(ret);
	};
	unichar LEintRead2(unsigned char *a){
		return((unichar)*a++ * 256 + *a);
	};
	int LEoutbytes2(unichar v,FILE *f)
	{
		unsigned char outbin[2];
		outbin[0]= (unsigned char)((0x0000ff00 & v) >>  8);
		outbin[1]= (unsigned char)(0x000000ff & v);
		if(!fwrite(outbin,2,1,f)) 
			exitMessage("bin write error");
		return(2);
	}
	int LEoutbytes3(unsigned int v,FILE *f)
	{
		unsigned char outbin[4];
		outbin[0] = (unsigned char)((0x00ff0000 & v) >> 16);
		outbin[1] = (unsigned char)((0x0000ff00 & v) >>  8);
		outbin[2] = (unsigned char) (0x000000ff & v);
		if(!fwrite(outbin,3,1,f)) 
			exitMessage("bin write error");
		return(3);
	}
	int LEoutbytes4(unsigned int v,FILE *f)
	{
		unsigned char outbin[4];
		outbin[0] = (unsigned char)((0xff000000 & v) >> 24);
		outbin[1] = (unsigned char)((0x00ff0000 & v) >> 16);
		outbin[2] = (unsigned char)((0x0000ff00 & v) >>  8);
		outbin[3] = (unsigned char) (0x000000ff & v);
		if(!fwrite(outbin,4,1,f)) 
			exitMessage("bin write error");
		return(4);
	}
	unichar LEinbytes2(FILE *f)
	{
		return( ((fgetc(f) << 8) & 0xff00 ) | (fgetc(f) & 0xff) );
	}
	unsigned int LEinbytes3(FILE *f)
	{
		return( (unsigned int)(
			( (fgetc(f) << 16) & 0xff0000 ) |
			( (fgetc(f) <<  8) & 0x00ff00 ) |
			( fgetc(f)         & 0x0000ff)    )
			);
	}
	unsigned int LEinbytes4(FILE *f)
	{
		return((unsigned int)(
			( (fgetc(f) << 24) & 0xff000000 ) |
			( (fgetc(f) << 16) & 0x00ff0000 ) |
			( (fgetc(f) <<  8) & 0x0000ff00 ) |
			( fgetc(f)         & 0x000000ff)    )
			);
	}
	unsigned int GEintRead4(unsigned char *a){
		return(*(unsigned int *)a);
	};
	unsigned int GEintRead3(unsigned char *a){
		
		return( ( ( *(unsigned int *)a >> 8) & 0x00ffffff) );
	};
	unichar GEintRead2(unsigned char *a)
	{
		return(*(unichar *)a);
	};
	int GEoutbytes2(unichar v,FILE *f)
	{
		if(!fwrite(&v,2,1,f)) 
			exitMessage("bin write error");
		return(2);
	}
	int GEoutbytes3(unsigned int v,FILE *f)
	{
		unsigned int k = (v << 8 ) & 0xffffff00;
		if(!fwrite(&k,3,1,f)) 
			exitMessage("bin write error");
		return(3);
	}
	int GEoutbytes4(unsigned int v,FILE *f)
	{
		if(!fwrite(&v,4,1,f)) 
			exitMessage("bin write error");
		return(4);
	}
	unichar GEinbytes2(FILE *f)
	{
		unichar w;
		fread(&w,2,1,f);
		return(w);
	}
	unsigned int GEinbytes3(FILE *f)
	{
		unsigned int w;
		fread(&w,3,1,f);
		return(  (w >> 8) & 0xffffff );
	}
	unsigned int GEinbytes4(FILE *f)
	{
		unsigned int longV;
		fread(&longV,4,1,f);
		return(longV);
	}
	
