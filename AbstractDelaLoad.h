#ifndef ABSTRACT_DELA_LOAD_H
#define ABSTRACT_DELA_LOAD_H

struct INF_free_info
{
	int must_be_free;
} ;

struct BIN_free_info
{
	int must_be_free;
} ;

struct INF_codes* load_abstract_INF_file(char*,struct INF_free_info*);
void free_abstract_INF(struct INF_codes*,struct INF_free_info);

unsigned char* load_abstract_BIN_file(char*,struct BIN_free_info*);
void free_abstract_BIN(unsigned char*,struct BIN_free_info);

#endif
