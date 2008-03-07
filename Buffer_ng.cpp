
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

/*
 * Author: Burak Arslan (arslan@univ-mlv.fr, plq@gsulinux.org)
 *         This File contains the fixed-memory read-only file buffer Buffer_ng api implementation.
 */


#include "Buffer_ng.h"

#define byte unsigned char

#define min( A, B ) ( (A)>(B)? (B) : (A) )
#define max( A, B ) ( (A)>(B)? (A) : (B) )

#ifdef BUFFER_NG_TEST

int main(int argc, char **argv) {
	
	system ("echo -n 1234567890abcdef > f");
	buffer_ng buf[1];
	byte * ret;

	FILE *file=fopen("f","r");

	buffer_init(buf,8,file);

	ret=buffer_next(buf,3,3); printf("n beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_prev(buf,3,3); printf("p beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_next(buf,3,3); printf("n beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_next(buf,3,3); printf("n beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_next(buf,3,3); printf("n beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_next(buf,3,3); printf("n beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_next(buf,3,3); printf("n beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_next(buf,3,3); printf("n beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);

	printf("\n");
	ret=buffer_prev(buf,3,3); printf("p beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_prev(buf,3,3); printf("p beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_prev(buf,3,3); printf("p beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_prev(buf,3,3); printf("p beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_prev(buf,3,3); printf("p beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_prev(buf,3,3); printf("p beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_prev(buf,3,3); printf("p beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_prev(buf,3,3); printf("p beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);

	printf("\n");
	ret=buffer_set(buf, 3,3); printf("s beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_set(buf, 6,3); printf("s beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_set(buf, 6,4); printf("s beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_set(buf, 6,5); printf("s beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_set(buf, 6,6); printf("s beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_set(buf, 6,7); printf("s beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_set(buf, 6,8); printf("s beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_set(buf, 6,9); printf("s beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);

	printf("\n");

	for (int i=0; i<=16; i++) {
		ret=buffer_set_mid(buf, i); printf("m beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	}
	system ("rm -fv f");
}

#endif

int buffer_init ( buffer_ng *b, size_t size, FILE *file ) {

	size_t pos;

	if (! b    ) return 1;

	if (! file ) return 1;
	pos = ftell (file);
	fseek(file, 0, SEEK_END);
	b->fsize = ftell(file);
	fseek (file, pos, SEEK_SET);	
	if ( ferror(file) ) return 1;

	b->beg    = (byte *)malloc(size+1);
	if (! b->beg ) return 1;

	b->end    = b->beg + size;
	*(b->end) = 0;
	b->cpos   = b->end-1;
	b->size   = size;
	b->fill   = 0;
	b->file   = file;	
	b->fpos   = pos;

	return 0;
}

byte *buffer_set_mid( buffer_ng *b, size_t p) { 

	int t=b->size-(b->size/2);

	b->fpos=max(0,((int)p)-((int)b->size/2));

	if (p > b->fsize-t) {
		b->fpos=b->fsize-b->size;
		fseek( b->file, b->fpos, SEEK_SET);
		b->cpos = b->beg+b->size - t;
		t=b->size;
	}
	else {	
		fseek(b->file, b->fpos, SEEK_SET);
	}

	b->fill=fread(b->beg, 1, b->size, b->file);
	if (! b->fill) return 0;

	b->cpos=b->beg+(p - b->fpos);

	*(b->beg + b->fill)=0;

	if (b->cpos >= b->end) return 0;
	return b->cpos;
}

byte *buffer_set( buffer_ng *b, size_t p, size_t s ) { 

	if ( s > b->size ) return 0;

	if ( (p < b->fpos) || (b->beg + (p-b->fpos+s) >= b->end) ) {
		clearerr(b->file);
		fseek(b->file, p, SEEK_SET); 
		clearerr(b->file);

		b->fill=fread( (void *) b->beg, 1, b->size, b->file );
				
		*(b->beg + b->fill)=0;
		b->cpos = b->beg;		

		if ( (ferror(b->file) && b->fill<=0) || (b->fill <s) ) {
			return 0;
		}
		else {
			b->fpos=p;
		}

		b->cpos=b->beg;
	}
	else {
		b->cpos=b->beg+( p-b->fpos );
	}

	return b->cpos;

}

byte *buffer_next( buffer_ng *b, size_t n, size_t s ) {
	byte *p;

	if ( s > b->size ) return 0;

	if (! n ) return b->cpos;	

	if (b->cpos+n+s > b->beg+b->fill) {

		if (b->fpos + (b->cpos-b->beg) + n + s > b->fsize) { // beyond the end of file
			b->cpos = b->beg; // so that the buffer is re-read by a buffer_move with d<0
			fseek(b->file, 0, SEEK_END);
#ifdef BUFFER_NG_TEST
			printf("!!");
#endif
			b->fpos=ftell(b->file)+1;
			return 0;
		}
#ifdef BUFFER_NG_TEST
		else {
			printf("! ");
		}
#endif

		p=b->cpos+n;
		int t = min(b->size - ( b->end -(b->cpos+n) ), b->size);

		while (p < b->end) {
			*( b->beg + (p- b->cpos -n) )=*p;
			p++;
		}
		b->cpos = b->beg;

		b->fill=fread( (void *) (b->beg + (b->size-t)) , 1, t, b->file ) + (b->size-t);
		b->fpos=ftell(b->file)- b->fill;

		*(b->beg + b->fill)=0;

	}
	else {
		b->cpos+=n;

#ifdef BUFFER_NG_TEST
		printf("  ");
#endif

	}

//	if ( b->cpos+s > b->beg + b->fill ) return 0;

	return b->cpos;
}

byte *buffer_prev( buffer_ng *b, size_t n, size_t s ) {
	byte *p;
	int t;
	int e=0;

	if ( s > b->size ) return 0;

	if ( n==0 ) return b->cpos;	

	if ( b->cpos-((int)n) < b->beg ) {

		if ( ((int)b->fpos)+ ((b->cpos-b->beg) -((int)n)) <0) {    // beyond the beginning of file
			b->cpos = b->end+1;           // so that the buffer is re-read by a buffer_move with d>0
			fseek(b->file, 0, SEEK_SET);
			return 0;
		}

		p=b->beg;
		t = min( min( b->size-(b->cpos-b->beg), b->fpos ), b->size );
		
		while (p < b->cpos) {
			*(p+t) =*p;
			p++;
			e++;
		}

		fseek(b->file,-t, SEEK_CUR );
		fread( (void *)b->beg, 1, t, b->file );

		b->fill=t+e;
		b->fpos=ftell(b->file) -t;
		fseek(b->file,b->fpos,SEEK_SET);
		b->cpos=b->beg+b->fill-s;

	}
	else {
		b->cpos-=n;
	}

	return b->cpos;
}

void buffer_free( buffer_ng * b ) {
	if (b)  {
		free(b->beg);
		b->beg=NULL;
	}
}

