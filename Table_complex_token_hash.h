 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
// Filename: Table_complex_token_hash.h
// We store complex tokens in this table so 
//  we can can find them 
//     By Alexis Neme
//
//

#ifndef			TableComplexTokenHashH
#define			TableComplexTokenHashH
//---------------------------------------------------------------------------

#include		"unicode.h"



#define			TCT_HASH_SIZE 401 
#define			TCT_HASH_BLOCK_SIZE  512


//
//
// 
struct tct_hash_block {              //  codification : i.e. "navio de guerra (574,1,5,1,575,-1,cod-priority)	
								    // (cod_tok1,cod_tok2, ..., cod_tokN,-1,information)*
    unsigned int	len ;			// -1 =the end of the complex token	
	unsigned int	N_blocks;		//alloc N*TT_HASH_BLOCK_SIZE
	int				*complex_tokens	;   	        
};

//
//
//
struct tct_hash {
   unsigned int				tct_hash_size;
   unsigned int				tct_hash_block_size;
   int						tct_last_token_cod;
   struct tct_hash_block**	tct_hash_blocks ;
   int **					tct_tab;             // token´s codes
};



struct			tct_hash* new_tct_hash(int tct_hash_size,int tct_hash_block_size);  
void			new_tct_hash_blocks(struct tct_hash *th,int tct_hash_size, int tct_hash_size_block);
void			free_tct_hash(struct tct_hash *);
unsigned int	tct_hash(int *token_tab, const int tct_hashsize);



int				add_tct_token(int *token_tab, struct tct_hash *tct_h,int priority);
int				was_allready_in_tct_hash(int *token_tab, struct tct_hash *tct_h, int priority);
unichar			*get_text_token(int token_number ,struct text_tokens* tok);
void			save_lines_tct_hash(struct tct_hash *);

void            build_complex_token_tab(unichar *compound_form,struct text_tokens *tok, int *token_tab_coumpounds);			
void	        print_lines_tct_hash( struct text_tokens* tok, struct tct_hash *tct_h);

#endif
