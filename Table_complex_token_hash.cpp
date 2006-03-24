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

//
// Filename: Table_complex_token_hash.cpp
// By Alexis Neme
//---------------------------------------------------------------------------
#include "Table_complex_token_hash.h"
#include "Error.h"
#include "Text_tokens.h"
//---------------------------------------------------------------------------

#define DEBUG 0

//
//
//
void	new_tct_hash_blocks(struct tct_hash *tct_h,int tct_hash_size, int tct_hash_block_size) {
	
struct tct_hash_block *hb;

	for ( int i = 0; i< tct_hash_size;i++) {

		hb = (struct tct_hash_block *) malloc(sizeof(struct tct_hash_block ));
		if (hb==NULL) {
		   fprintf(stderr,"Not enough memory in new_hash_block\n");
		   fatal_error(1);
		};

		hb->complex_tokens = (int *) malloc(tct_hash_block_size * sizeof(int));
		if (hb->complex_tokens==NULL) {
		   fprintf(stderr,"Not enough memory in new_hash_block\n");
		   fatal_error(1);
		};

		hb->N_blocks = 1;			// allocate one block only
		hb->len = 0;
		tct_h->tct_hash_blocks[i] = hb;
		 
		}
};

//
//
//
struct tct_hash*		new_tct_hash(int tct_hash_size,int tct_hash_block_size) {
      struct tct_hash *tct_h;

      tct_h = (struct tct_hash *) malloc(sizeof(struct tct_hash));
      if (tct_h==NULL) {
       fprintf(stderr,"Not enough memory in new_tct_hash for table_hash \n");
       fatal_error(1);
	  };

	  tct_h->tct_hash_size			= tct_hash_size;
	  tct_h->tct_hash_block_size	= tct_hash_block_size;
      tct_h->tct_last_token_cod	= 0 ;

	  tct_h->tct_hash_blocks	= (struct tct_hash_block **) 
		                        malloc( tct_hash_size * sizeof(struct tct_hash_block *)) ;

	  new_tct_hash_blocks(tct_h,tct_hash_size,tct_hash_block_size);
 	  
	  tct_h->tct_tab			= (int **) malloc( 1000000 * sizeof(int *) ) ;
      if (tct_h->tct_tab ==  NULL) {
       fprintf(stderr,"Not enough memory in new_tct_hash for tab \n");
       fatal_error(1);
	  };

      return(tct_h);      
}



//
//
//
void	free_tct_hash(struct tct_hash *tct_hash){
	int i; 
	unsigned int j;

	if (tct_hash==NULL) {
		fprintf(stderr,"Call to free_tct_hash on NULL pointer\n");
     return;
	};

	for ( j= 0; j< tct_hash->tct_hash_size;j++) {
		free(tct_hash->tct_hash_blocks[j]->complex_tokens);
		free(tct_hash->tct_hash_blocks[j]);
	} ;

    free(tct_hash->tct_hash_blocks);

	for(i= 0; i<tct_hash->tct_last_token_cod; i++) {
		  free( tct_hash->tct_tab[i]);
    };

    free(tct_hash->tct_tab);    
	free(tct_hash)  ;           

};

//
//          TCT_length(574,1,5,1,575,-1) is 6 we count the -1
//

int tct_length(int *token_tab) {
    int i;
	for (i = 0 ; *(token_tab+i) !=-1; i++ ) ;
    return(++i); 
}

//
//
//
unsigned int	tct_hash(int *token_tab, const int hashsize){

    long unsigned int hashVal;
	hashVal = 0;
    //int i = 0 ; 
	for (int i = 0 ; *token_tab !=-1; i++ ) {
           hashVal += (*token_tab <<i)  + 1357 ;
		   token_tab++;
	} ;
    //int x = abs(hashVal) % hashsize ;
    return (abs(hashVal) % hashsize );
}



//
//
//
void	save_lines_tct_hash( struct tct_hash *tct_h){
	unsigned int i,j,k;
	printf("Table tct Hash\n");
    for (  i = 0 ; i < tct_h->tct_hash_size ; i++) {		      
        // Print the Block i

		j = 0;
		if (tct_h->tct_hash_blocks[i]->len == 0) continue;
		printf(">>>>>>>>>>>>>>>>>>>>>>>>BLOCK: %d <<<<<<<<<<<<<<<<<<<<<<<<<\n ",i); 
          
     	  printf(">>>> ");
         // print a complex token 
		 while(j< tct_h->tct_hash_blocks[i]->len) {
		    k = j;
			printf("(");
          // print a  token of the complex token
		  while (tct_h->tct_hash_blocks[i]->complex_tokens[k] != -1 && k< tct_h->tct_hash_blocks[i]->len) {
			printf("%d,", tct_h->tct_hash_blocks[i]->complex_tokens[k]);
            
			k++;
		  }; // j
		    printf("%d,", tct_h->tct_hash_blocks[i]->complex_tokens[k]);
		    k++;
		  printf("%d)", tct_h->tct_hash_blocks[i]->complex_tokens[k]);
		  k++;
		  printf("\n");
		  j =  k ;
		 } // 
	};// i
};

//
// copy the h-block in a longer block alocating one more hash_block_size of memory 
//
int *realloc_tct_hash_block(struct tct_hash_block *tct_hb, const int tct_hash_block_size) {
    

    unsigned int N_block	= tct_hb->N_blocks ;
	int *old_block			= tct_hb->complex_tokens;

	N_block++;
	int *new_block		= (int *) 
						  malloc(( N_block * tct_hash_block_size) * sizeof(int ));

	if ( new_block == NULL ) {
		fprintf(stderr,"Not enough memory in realloc_ttc_hash_block\n");
		fatal_error(1);
	};
//    Copy  the old block in the new one 
     for(unsigned int i=0; i< tct_hb->len;i++) 
	     new_block[i] = tct_hb->complex_tokens[i];


	tct_hb->N_blocks	= N_block ;
	tct_hb->complex_tokens = new_block;

	free(old_block);
	return(new_block);
}

	
//
//		Add a complex_token to  tct_hash and return the the priority 9 the nex integer
//      (574,1,5,1,575,-1,2) "navio de guerra" 2
int		add_tct_token(int *token_tab, struct tct_hash *tct_h,int priority){

	int h;

	h = tct_hash(token_tab, tct_h->tct_hash_size);

	struct tct_hash_block  *hb = tct_h->tct_hash_blocks[h] ;

    int N_block_h		= hb->N_blocks ; 
	int *toks			= hb->complex_tokens;
	int h_block_size	= tct_h->tct_hash_block_size;
    int tct_len			= tct_h->tct_hash_blocks[h]->len;
	//int last_cod		= tct_h->tct_last_token_cod;
	

// test for block reallocation
//  A complex token may be  12 tokens length
	if( (tct_len  + 15)  > (N_block_h * h_block_size)  ) {
		if (DEBUG) {	
            printf("realloc ...%d \n", h);
            printf("\n");
        }
		toks = realloc_tct_hash_block(hb, h_block_size) ;
	};
	int i;
    for (i = 0 ; *(token_tab+i) !=-1; i++ ) {
		toks[tct_len+i] = *(token_tab+i);
    }
	toks[tct_len+i] = -1;
	i++;
     
	toks[tct_len + i] = priority;
    i++; 
    tct_h->tct_hash_blocks[h]->len +=i; 



    return(0);

};

//
//
//  find the offset of the  integer´s subsequence token_tab  in hb 
//
int tct_match(struct tct_hash_block *hb, int *token_tab){

int i=0;
int j=0;
int len_hb = hb->len;
int l_b = tct_length(token_tab);
int* a;
a = hb->complex_tokens; 
  while (i < len_hb) {
    j=0;
	while ( i< len_hb && a[i+j] == token_tab[j] ) {
		if ( token_tab[j] ==-1 ) return(i); 
		if (j == l_b ) return(i);
		j++;         
	}  
	i++;  
  } 
  return(-1);
}



int		was_allready_in_tct_hash(int *token_tab, struct tct_hash *tct_h, int priority){
	
	int h;
    struct tct_hash_block *hb; 

	h = tct_hash(token_tab, tct_h->tct_hash_size);
	hb = tct_h->tct_hash_blocks[h] ;
    int *toks = tct_h->tct_hash_blocks[h]->complex_tokens ;
    	  
	// find token_tab in block
   int offset_tok = tct_match(tct_h->tct_hash_blocks[h],token_tab) ;
	if  (offset_tok != - 1) {
		 // get the token_cod
		 int offset_token_code = offset_tok + tct_length(token_tab);


		return(toks[offset_token_code]);					// found			  
	}

    return(add_tct_token(token_tab,tct_h,priority)); // not found


}

void   build_complex_token_tab(unichar *compound_word,struct text_tokens *tok, int *token_tab_coumpounds)			
// build the token_tab_coumpounds cod based on the simple forms  
// the compound form  "sans raison" will have token_tab_coumpounds (121,1,1643,-1,priority)		  
{
 int i,pdebut,pfin,k_tab;
 int l=u_strlen(compound_word);
 unichar simple_form[100]; 

     i=pdebut=pfin=0; k_tab=0;
     while (i<l) 
     { if (u_is_letter(compound_word[i]) ) {
		 pfin++;
		}
	   else {
			u_get_substring(compound_word,pdebut,pfin,simple_form);
		    token_tab_coumpounds[k_tab++] = get_token_number(simple_form,tok);          
            u_get_substring(compound_word,pfin,pfin+1,simple_form);             // one separator 
            token_tab_coumpounds[k_tab++] = get_token_number(simple_form,tok); 

			pdebut=++pfin;
	   }
	 i++;
	 }
	 u_get_substring(compound_word,pdebut,pfin,simple_form);
	 token_tab_coumpounds[k_tab++] = get_token_number(simple_form,tok); 
     token_tab_coumpounds[k_tab++] = -1;          
}

void	print_lines_tct_hash( struct text_tokens* tok, struct tct_hash *tct_h){
	unsigned int i,j,k;
   unichar mot[100]; 
	int token_numb;
	printf("Table tct Hash\n");

    for (  i = 0 ; i < tct_h->tct_hash_size ; i++) {		      
        // Print the Block i

		j = 0;
		if (tct_h->tct_hash_blocks[i]->len == 0) continue;
		printf(">>>>>>>>>>>>>>>>>>>>>>>>BLOCK: %d <<<<<<<<<<<<<<<<<<<<<<<<<\n ",i); 
          
     	  printf(">>>> ");
         // print a complex token 
		 while(j< tct_h->tct_hash_blocks[i]->len) {
		    k = j;
			printf("(");
          // print a  token of the complex token
		  while (tct_h->tct_hash_blocks[i]->complex_tokens[k] != -1 && k< tct_h->tct_hash_blocks[i]->len) {
            token_numb = tct_h->tct_hash_blocks[i]->complex_tokens[k] ;
			printf("%d-",token_numb );
            u_strcpy(mot,get_text_token(token_numb,tok)); 
            u_prints(mot);
			printf(", " ); 
			k++;
		  }; // j
		    printf("%d,", tct_h->tct_hash_blocks[i]->complex_tokens[k]);
		    k++;
		  printf("%d)", tct_h->tct_hash_blocks[i]->complex_tokens[k]);
		  k++;
		  printf("\n");
		  j =  k ;
		 } // 
	};// i
}


