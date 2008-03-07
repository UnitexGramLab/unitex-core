 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

//
//     Filename: Table_hash.cpp
//     By Alexis Neme
//---------------------------------------------------------------------------

#include "Table_hash.h"
#include "Unicode.h"
#include "Error.h"




//
//
//
void	new_hash_blocks(struct table_hash *th,int hash_size, int hash_block_size) {
	
struct hash_block *hb;

	for ( int i = 0; i< hash_size;i++) {

		hb = (struct hash_block *) malloc(sizeof(struct hash_block ));
		if (hb==NULL) {
		   fatal_error("Not enough memory in new_hash_block\n");
		}

		hb->tokens = (unichar *) malloc(hash_block_size * sizeof(unichar));
		if (hb->tokens ==NULL) {
		   fatal_error("Not enough memory in new_hash_block\n");
		}

		hb->tokens[0] = '\0';
		u_strcat( hb->tokens,";");
		hb->N = 1;						// allocate one block only
		th->tab_hash_blocks[i] = hb;
		 
		}
};

//
//
//
struct table_hash* new_table_hash(int hash_size,int hash_block_size) {
      struct table_hash *th;

      th = (struct table_hash *) malloc(sizeof(struct table_hash));
      if (th==NULL) {
       fatal_error("Not enough memory in new_table_hash for table_hash\n");
	  }

	  th->hash_size			= hash_size;
	  th->hash_block_size	= hash_block_size;
      th->last_token_cod	= 0 ;

	  th->tab_hash_blocks	= (struct hash_block **) 
		                        malloc( hash_size * sizeof(struct hash_block *)) ;

	  new_hash_blocks(th,hash_size,hash_block_size);
 	  
	  th->tab=(unichar**) malloc( 1000000 * sizeof(unichar *) ) ;
      if (th->tab ==  NULL) {
       fatal_error("Not enough memory in new_table_hash for tab\n");
	  }

      return(th);      
}



//
//
//
void	free_table_hash(struct table_hash *t_hash){
	
	if (t_hash==NULL) {
		error("Call to free_table_hash on NULL pointer\n");
     return;
	}

	for ( int i = 0; i< HASH_SIZE;i++) {
		free(t_hash->tab_hash_blocks[i]->tokens);
		free(t_hash->tab_hash_blocks[i]);
	}

    free(t_hash->tab_hash_blocks);

	for(int i= 0; i<t_hash->last_token_cod; i++) {
		  free( t_hash->tab[i]);
    };

    free(t_hash->tab);    
	free(t_hash)  ;           

};



//
//
//
unsigned int	hash(unichar *s, const int hashsize){

    long unsigned int hashVal;
	hashVal = 0;
    for ( int i = 0 ; *s !='\0' ; i++) {
           hashVal += ((*s++) << i) + 1357 ;
	} ;

    return (hashVal % hashsize );
}



//
//
//
void	save_lines_table_hash(FILE * f, struct table_hash *th){
	
	u_fprintf(f,"Table Hash\n");
    for ( int i = 0 ; i < HASH_SIZE ; i++) {		      
        u_fprintf(f,"%d- ",i); 
		u_fprintf(f,"%S\n",th->tab_hash_blocks[i]->tokens); 	
	};
};

//
// copy the h-block in a longer block alocating one more hash_block_size of memory 
//
unichar *realloc_hash_block(struct hash_block *hb, const int hash_block_size) {
    
	
    unsigned int N_block	= hb->N; 
	unichar *old_block		= hb->tokens;

	N_block++;
	unichar *new_block = (unichar *) malloc(( N_block * hash_block_size) * sizeof(unichar ));

	if ( new_block == NULL ) {
		fatal_error("Not enough memory in realloc_hash_block\n");
	};

    u_strcpy(new_block,old_block);
	hb->N = N_block ;
	hb->tokens = new_block;

	free(old_block);
	return(new_block);
}

	
//
//		Add a tokens to the table_hash and return the token_cod
//
int		add_token(unichar *s, struct table_hash *th){

	int h;
	unichar u_dest[STR_BUFFER_SIZE] ;  /* format
                                              "token:token_cod;" =
                                              max. token length +
                                              12 */
	h = hash(s, th->hash_size);

	struct hash_block  *hb = th->tab_hash_blocks[h] ;

        int N_block_h	= hb->N ; 
	unichar *toks	= hb->tokens ;
	int h_block_size = th->hash_block_size;
	int last_cod     = th->last_token_cod;
	
        // format s 
        /* prohibit a buffer overflow! */
        if ((u_strlen(s)+1) > (STR_BUFFER_SIZE-12)) // 1024-12 for ";" and ":" and u_token_cod
          {
            fatal_error("Error: token to long to be stored in hash:\n%S\n",s);
          }
         u_sprintf(u_dest,"%S:%d;",s,last_cod);
	// test for block reallocation
        int l_tok = u_strlen( hb->tokens);
	while ( (l_tok + u_strlen(u_dest) + 5)  > (N_block_h * h_block_size)  ) {
		toks = realloc_hash_block(hb, h_block_size) ;
                N_block_h = hb->N;
	};

	u_strcat(toks,u_dest);
	th->last_token_cod++; 
    return(last_cod);

}

//
//
//
//

int find_token_numb(unichar *s, struct table_hash *th){
	
  int h;
  unichar str[STR_BUFFER_SIZE] ;  

  h = hash(s, th->hash_size);

  unichar *toks = th->tab_hash_blocks[h]->tokens ;

  /* prohibit a buffer overflow! */
  if ((u_strlen(s)+1) > (STR_BUFFER_SIZE-2)) // 1024-2 for ";" and ":"
    {
      fatal_error("Error: token too long to be stored in hash:\n%S\n",s);
    }

  u_sprintf(str,";%S:",s); // str == ";token:"
    
  // find str in toks
  int offset_tok = u_strstr(toks,str) ;
  if  (offset_tok != - 1) {
    // get the token_cod
    int offset_token_code = offset_tok + u_strlen(str);
    int tok_cod=0;
    u_sscanf(&(toks[offset_token_code]),"%d",&tok_cod);
    return(tok_cod);					// found			  
  }

  return(-1) ;							// not found

};

//
//  unicode version of strstr
//  return the offset of the substring b in a 
//
int u_strstr(const unichar *a,const unichar *b) {
int i=0;
int j=0;
int l_b = u_strlen(b);
  while (a[i]) {
    j=0;
	while ( a[i+j] && a[i+j]==b[j] ) {
		if (j == l_b - 1) return(i);
		j++;         
	}  
	i++;  
  } 
  return(-1);
}

		/*
		printf("reallock %d= %d\n" ,h, u_strlen(toks));
		if (h == 3003) { 
			printf("toks :\n");u_prints(toks);printf("U_dest:\n");u_prints(u_dest);         
			printf("reallock lenTok=%d   strlenS=%d \n" ,u_strlen( th->tab_hash_blocks[h]->tokens),
			u_strlen(s));
			exit(1);} */
