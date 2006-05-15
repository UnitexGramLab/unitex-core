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

#include <stdlib.h>
#include "WordList.h"
#include "error.h"


/**
 * Allocates, initializes and returns a new word list element with
 * the given 'list' as following element.
 */
struct word_list* new_word_list(unichar* word,struct word_list* list) {
struct word_list* tmp;
tmp=(struct word_list*)malloc(sizeof(struct word_list));
if (tmp==NULL) {
	fatal_error("Not enough memory in new_word_list\n");
}
tmp->word=u_strdup(word);
tmp->next=list;
return tmp;
}


/**
 * Frees the whole given word list.
 */
void free_word_list(struct word_list* list) {
struct word_list* tmp;
while (list!=NULL) {
	tmp=list;
	list=list->next;
	free(tmp->word);
	free(tmp);
}
}

