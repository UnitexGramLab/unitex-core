
#ifndef BUFFER_NG_H
#define BUFFER_NG_H

#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;

typedef struct {

	byte * beg;
	byte * end;
	byte * cpos;
	size_t size;
	size_t fill;

	FILE * file;
	size_t fpos; // pos of beg in the file
	size_t fsize;

} buffer_ng;

int   buffer_init( buffer_ng *b, size_t size, FILE *file );

/* advance n bytes in the file by making sure that there is still s bytes to read */
byte *buffer_next( buffer_ng *b, size_t n, size_t s ); 

/* set buffer pointer to the to pth byte of the file  by making sure that there is still s bytes to read */
byte *buffer_set ( buffer_ng *b, size_t p, size_t s );

byte *buffer_set_mid ( buffer_ng *b, size_t p );

void  buffer_free( buffer_ng *b );

#endif

