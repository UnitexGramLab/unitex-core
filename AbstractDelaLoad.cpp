#include "Unicode.h"
#include "DELA.h"
#include "AbstractDelaLoad.h"


struct INF_codes* load_abstract_INF_file(char* name,struct INF_free_info*)
{
	struct INF_codes* res = NULL;
	res = load_INF_file(name);
	return res;
}

void free_abstract_INF(struct INF_codes* INF,struct INF_free_info inf_free_info)
{
	if (INF != NULL)
		if (inf_free_info.must_be_free != 0)
			free_INF_codes(INF);
}

unsigned char* load_abstract_BIN_file(char* name,struct BIN_free_info* p_bin_free_info)
{
	unsigned char* tab = NULL;
	tab = load_BIN_file(name);
	if (tab != NULL)
		p_bin_free_info->must_be_free = 1;
	return tab;
}

void free_abstract_BIN(unsigned char* BIN,struct BIN_free_info bin_free_info)
{
	if (BIN != NULL)
		if (bin_free_info.must_be_free != 0)
			free(BIN);
}
