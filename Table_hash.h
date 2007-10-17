 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
// Filename: table_hash.h
//     By Alexis Neme
//
//

#ifndef			TableHashH
#define			TableHashH
//---------------------------------------------------------------------------

#include		"Unicode.h"



#define			HASH_SIZE 4001 
#define			HASH_BLOCK_SIZE  512
#define                 STR_BUFFER_SIZE 4096 /* must be bigger than
                                                longest token
                                                (including tags) */

//
//
// 
struct hash_block {	
								// (token:token_code;token:token_code;token:token_code;token:token_code;)
    unsigned int	N ;			// alloc N*HASH_BLOCK_SIZE
	unichar *		tokens;   	        
};

//
//
//
struct table_hash {
   unsigned int			hash_size;
   unsigned int			hash_block_size;
   int					last_token_cod;
   struct hash_block**	tab_hash_blocks ;
   unichar**			tab;             // tokens
};



struct			table_hash* new_table_hash(int hash_size,int hash_block_size);   // 
void			new_hash_blocks(struct table_hash *th,int hash_size, int hash_size_block);
void			free_table_hash(struct table_hash *);
unsigned int	hash(unichar *s, const int hashsize);



int				add_token		(unichar *s , struct table_hash *th);
int				find_token_numb	(unichar *s,  struct table_hash *th);
int				u_strstr		(const unichar *a,const unichar *b);

void			save_lines_table_hash(FILE*,struct table_hash *);

#endif
