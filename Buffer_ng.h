
#ifndef BUFFER_NG_H
#define BUFFER_NG_H

#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;

typedef struct {

	byte * beg;
	byte * end;
	byte * cpos;
	size_t size; // capacity of the buffer
	size_t fill; // size of the buffer

	FILE * file;
	size_t fpos; // pos of beg in the file
	size_t fsize;// size of the file

} buffer_ng;


/* init the buffer */
int   buffer_init( buffer_ng *b, size_t size, FILE *file );

/* go n bytes forward in the file by making sure that there is still s bytes to read forward  */
byte *buffer_next( buffer_ng *b, size_t n, size_t s ); 

/* go n bytes backward in the file by making sure that there is still s bytes to read forward */
byte *buffer_prev( buffer_ng *b, size_t n, size_t s ); 

/* set buffer pointer to the to pth byte of the file  by making sure that there is still s bytes to read */
byte *buffer_set ( buffer_ng *b, size_t p, size_t s );

/* set buffer pointer to p-size/2 */
byte *buffer_set_mid ( buffer_ng *b, size_t p );

/* you'll end up with size bytes more available memory */
void  buffer_free( buffer_ng *b );

#endif

