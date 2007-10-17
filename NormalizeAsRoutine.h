#ifndef NORMALIZEASROUTINE_H
#define NORMALIZEASROUTINE_H

#include "String_hash.h"

#define MAX_TAG_LENGTH 4000
#define KEEP_CARRIDGE_RETURN 0
#define REMOVE_CARRIDGE_RETURN 1
#define BUFFER_SIZE 1000000
/* When we are at less than 'MARGIN_BEFORE_BUFFER_END' from the end of the buffer,
 * we will refill it, unless we are at the end of the input file. */
#define MARGIN_BEFORE_BUFFER_END (MAX_TAG_LENGTH+1000)

int normalize(char*, char*, int, char*);

#endif
