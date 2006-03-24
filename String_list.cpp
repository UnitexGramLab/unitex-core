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

//---------------------------------------------------------------------------
#include "String_list.h"
//---------------------------------------------------------------------------


struct string_list* new_string_list(unichar* s) {
struct string_list* tmp;
tmp=(struct string_list*)malloc(sizeof(struct string_list));
tmp->s=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(s)));
u_strcpy(tmp->s,s);
tmp->suivant=NULL;
return tmp;
}


void free_string_list(struct string_list* l) {
struct string_list* tmp;
while (l!=NULL) {
  tmp=l;
  if (tmp->s!=NULL) {
     free(tmp->s);
  }
  l=l->suivant;
  free(tmp);
}
}



void free_string_list_element(struct string_list* l) {
if (l==NULL) return;
if (l->s!=NULL) free(l->s);
free(l);
}



struct string_list* insert_in_string_list(unichar* s,struct string_list* l) {
if (l==NULL) return new_string_list(s);
if (!u_strcmp(s,l->s)) return l;
l->suivant=insert_in_string_list(s,l->suivant);
return l;
}



struct string_list* insert_at_end_of_string_list(unichar* s,struct string_list* l) {
if (l==NULL) return new_string_list(s);
l->suivant=insert_in_string_list(s,l->suivant);
return l;
}
