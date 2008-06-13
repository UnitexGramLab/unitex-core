#define POUND 0x00A3
#define L1 100
#define L2 300
#define L3 500
#define L4 1000
#define IMMEDIAT 0
#define SHORTEST 1
#define LONGEST 2
#define PROTEGE 4
#define VERBOSE 0
#include "Unicode.h"


int compare(unichar *, unichar **);
int filtrer(unichar * ,unichar **);
int get_indice_var_op(unichar *);
int flex_op_with_var(unichar (*)[L1],unichar *,unichar *,int *,int *,unsigned int *);
unsigned int get_flag_var(int,unsigned int);
