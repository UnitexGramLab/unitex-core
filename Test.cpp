 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "IOBuffer.h"
#include "Pattern.h"
#include "PatternTree.h"
#include "unicode.h"
#include "String_hash.h"
#include "DELA.h"
#include "List_pointer.h"


/**
 * This program is designed for test purpose only.
 */

int main(int argc,char *argv[]) {
setBufferMode();

struct string_hash* h=new_string_hash();
get_value_index(u_strdup_char("N"),h);
get_value_index(u_strdup_char("A"),h);
get_value_index(u_strdup_char("z3"),h);

struct pattern* p1=build_pattern(u_strdup_char("dirty.-A:ms:sf:bga:msf"),h);
struct pattern* p2=build_pattern(u_strdup_char("N"),h);
struct pattern* p3=build_pattern(u_strdup_char("N+z3:ms:fs"),h);

struct pattern_node* root=new_pattern_node();
int n=0;
printf("p1=%d\n",add_pattern(&n,p1,root));
printf("p2=%d\n",add_pattern(&n,p2,root));
printf("p3=%d\n",add_pattern(&n,p3,root));


struct dela_entry* l2=tokenize_DELAF_line(u_strdup_char("dirty,.N+z3:sKm"));
struct list_pointer* result=get_matching_patterns(l2,root);
while (result!=NULL) {
   printf("Le pattern %d matche\n",((struct constraint_list*)(result->pointer))->pattern_number);
   result=result->next;
}
unichar* ss=u_strdup_char("<u!MOT>");
int negative_tag=(ss[1]=='!')?1:0;
unichar* tt=u_strdup(&(ss[1+negative_tag]),u_strlen(ss)-2-negative_tag);
printf("toto=");
u_prints(tt);
printf("\n");
free_pattern_node(root);
printf("Done\n");
return 0;
}




