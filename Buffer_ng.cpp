
#include "Buffer_ng.h"

#define min( A, B ) ( (A)>(B)? (B) : (A) )
#define max( A, B ) ( (A)>(B)? (A) : (B) )

int buffer_init ( buffer_ng *b, size_t size, FILE *file ) {

	if (!b) return 1;

	b->beg    = (byte *)malloc(size+1);
	b->end    = b->beg + size;
	*(b->end) = 0;
	b->cpos   = b->end-1;
	b->size   = size;
	b->fill   = 0;
	b->file   = file;

	size_t pos = ftell (file);
	fseek(file, 0, SEEK_END);
	b->fsize = ftell(file);
	fseek (file, pos, SEEK_SET);	
	
	b->fpos   = pos;

	if (! b->file    ) return 1;
	if (ferror(file) ) return 1;
	if (! b->beg     ) return 1;

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
		fseek(b->file, p, SEEK_SET); // implicit clearerr() according to man ferror

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

	if (b->cpos+n+s > b->end) {
		p=b->cpos+n;
		int t = min(b->size - ( b->end -(b->cpos+n) ), b->size);
		if (b->fpos + t + b->size > b->fsize) {
			b->fpos=b->fsize-b->size;
			fseek( b->file, b->fpos, SEEK_SET);
			b->cpos = b->beg+b->size - t;
			t=b->size;
		}
		else {
			while (p < b->end) {
				*( b->beg + (p- b->cpos -n) )=*p;
				p++;
			}
			b->cpos = b->beg;
		}

		b->fill=fread( (void *) (b->beg + (b->size-t)) , 1, t, b->file ) + (b->size-t);
		b->fpos=ftell(b->file)- b->fill;
		if (! b->fill) return 0;

		*(b->beg + b->fill)=0;

	}
	else {
		b->cpos+=n;
	}

	if ( b->cpos+s > b->beg + b->fill) return 0;

	return b->cpos;
}

void buffer_free( buffer_ng * b ) {
	if (b)  {
		free(b->beg);
	}
}

#ifdef BUFFER_NG_TEST

int main(int argc, char **argv) {
	
	system ("echo -n 1234567890abcdef > f");
	buffer_ng buf[1];
	byte * ret;

	FILE *file=fopen("f","r");

	buffer_init(buf,8,file);

	ret=buffer_next(buf,3,3); printf("beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_next(buf,3,3); printf("beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_next(buf,3,3); printf("beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_next(buf,3,3); printf("beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_next(buf,3,3); printf("beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_next(buf,3,3); printf("beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	printf("\n");
	ret=buffer_set(buf, 3,3); printf("beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_set(buf, 6,3); printf("beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_set(buf, 6,4); printf("beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_set(buf, 6,5); printf("beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_set(buf, 6,6); printf("beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_set(buf, 6,7); printf("beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_set(buf, 6,8); printf("beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	ret=buffer_set(buf, 6,9); printf("beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);

	printf("\n");

	for (int i=0; i<=16; i++) {
		ret=buffer_set_mid(buf, i); printf("beg: %s ret: 0x%x -> %s\n", buf->beg, ret, ret);
	}
	system ("rm -fv f");
}

#endif

