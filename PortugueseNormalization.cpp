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

#include "PortugueseNormalization.h"
#include "Error.h"

//
// this function takes a string like "[aaa]bbb" and put aaa in the
// parameter prefix while the parameter s is replaced by bbb
// If there is no prefix, s is unchanged and prefix contains the empty string
//
void get_bracket_prefix(unichar* s, unichar* prefix) {
if (s[0]!='[') {
   prefix[0]='\0';
   return;
}
int i=1;
int j=0;
while (s[i]!=']' && s[i]!='\0') {
   prefix[j++]=s[i++];
}
if (s[i]!=']')  {
   prefix[0]='\0';
   return;
}
prefix[j]='\0';
i++;
j=0;
while (s[i]!='\0') {
   s[j++]=s[i++];
}
s[j]='\0';
}


//
// this function copies src into dest ignoring spaces
//
void u_strcpy_without_space(unichar* dest,unichar* src) {
int i=0;
int j=0;
do {
   if (src[i]!=' ') {
      dest[j++]=src[i];
   }
   i++;
} while (src[i-1]!='\0');
}



//
// this function builds the normalization grammar adapted to the match list
// passed in parameter 
//
void build_portuguese_normalization_grammar(Alphabet* alph,struct match_list* list,unsigned char* root_bin,
                                            struct INF_codes* root_inf,unsigned char* inflected_bin,
                                            struct INF_codes* inflected_inf,char* res_grf_name,
                                            struct normalization_tree* norm_tree,
                                            struct normalization_tree* nasal_norm_tree) {
struct match_list* L=list;
int N=0;
unichar temp[2000];
unichar prefix[2000];
struct string_hash* hash=new_string_hash();
while (L!=NULL) {
   if (L->output!=NULL) {
      // first, we normalize the sequences by removing all spaces
      u_strcpy_without_space(temp,L->output);
      u_strcpy(L->output,temp);
      // then we check if this sequence has already been processed
      int J=get_value_index(L->output,hash,DONT_INSERT);
      if (J!=-1) {
         // if the sequence has already been analyzed, we do nothing
      }
      else {
         get_value_index(L->output,hash);
         get_bracket_prefix(L->output,prefix);
         if (!u_strcmp(prefix,"FuturConditional")) {
            N=N+replace_match_output_by_normalization_line(L,alph,root_bin,root_inf,inflected_bin,
                                                           inflected_inf,norm_tree);
         }
         else if (!u_strcmp(prefix,"NasalSuffix")) {
           //      N=N+replace_match_output_by_normalization_line_nasal(L,alph,nasal_norm_tree);
         }
         else if (prefix[0]!='\0') {
                 error("Unknown normalization code: %S\n",prefix);
         }
      }
   }
   L=L->next;
}
free_string_hash(hash);
u_printf("Saving the grammar...\n");
save_portuguese_normalization_grammar(N,list,res_grf_name);
u_printf("%d normalization rules have been produced.\n",N);
}



/**
 * This function takes a string like ":F1s:F2s" and fills an array
 * like this: array[0]="F1s",    array[1]="F2s"
 * 
 * Note that the length of the array is stored in '*length'.

 * WARNING: this function does not consider protected chars. There could be 
 *          problems if someone had the strange idea to use ':' as an inflectional
 *          code letter like ":F1s:F\:s"
 */
void tokenize_inflectional_codes(unichar* s,int *length,unichar** array) {
int i=0;
(*length)=0;
while (s[i]==':') {
   i++;
   int j=0;
   unichar temp[DIC_WORD_SIZE];
   while (s[i]!='\0' && s[i]!=':') {
      temp[j++]=s[i++];
   }
   temp[j]='\0';
   array[*length]=u_strdup(temp);
   (*length)++;
}
}



//
// this function takes a sequence like "dir-se-ia[:C1s]" and
// replace it by the corresponding normalization line:
// "dir-se-ia/{...,...V:C1s}-{se,.PRO}"
// it returns i if i correct lines are produced, 0 else
//
int replace_match_output_by_normalization_line(struct match_list* L,Alphabet* alph,unsigned char* root_bin,
                                                struct INF_codes* root_inf,unsigned char* inflected_bin,
                                                struct INF_codes* inflected_inf,
                                                struct normalization_tree* norm_tree) {
if (L->output==NULL) {
   return 0;
}
unichar radical[1000];
unichar pronoun[1000];
unichar inflectional_code[1000];
unichar suffix[1000];
if (!tokenize_portuguese_match(L->output,radical,pronoun,suffix,inflectional_code)) {
   free(L->output);
   L->output=NULL;
   return 0;
}
// we put the radical, the pronoun and the suffix in lowercase,
// but not the inflectional codes in order to avoid errors like [:F3p] -> [:f3p]
turn_portuguese_sequence_to_lowercase(radical);
turn_portuguese_sequence_to_lowercase(pronoun);
turn_portuguese_sequence_to_lowercase(suffix);

// we tokenize the pronoun
struct list_ustring* pronouns=tokenize_portuguese_pronoun(pronoun);

struct list_ustring* lemmas=NULL;
// we look for the lemma in the radical form dictionary
if (!get_radical_lemma(radical,&lemmas,alph,root_bin,root_inf)) {
   free(L->output);
   L->output=NULL;
   return 0;
}

// we do the job for each lemma that has been found
unichar temp_result[4000];
temp_result[0]='\0';
int RESULT=0;

while (lemmas!=NULL) {
   unichar entry[1000];
   // we get the inf number associated to this lemma in the inflected form dictionary
   int res=get_inf_number_for_token(4,lemmas->string,0,entry,alph,inflected_bin);
   if (res==-1) {
      return 0;
   }
   int n_inflectional_codes;
   unichar* inflectional_codes[100];
   tokenize_inflectional_codes(inflectional_code,&n_inflectional_codes,inflectional_codes);
   struct list_ustring* tok=inflected_inf->codes[res];
   // then for each uncompressed form, we look if it matches with the inflectional code
   while (tok!=NULL) {
      unichar line[2000];
      uncompress_entry(entry,tok->string,line);
      struct dela_entry* tmp=tokenize_DELAF_line(line,0);
      if (compatible_portuguese_inflectional_codes(tmp,n_inflectional_codes,inflectional_codes)) {
         // if the code matches, we can produce a new line of the normalization grammar
         unichar temp_result2[4000];
         u_strcpy(temp_result2,"@");
         u_strcat(temp_result2,radical);
         u_strcat(temp_result2,"-");
         u_strcat(temp_result2,pronoun);
         u_strcat(temp_result2,"-");
         u_strcat(temp_result2,suffix);
         u_strcat(temp_result2,"/{");
         u_strcat(temp_result2,tmp->lemma);
         u_strcat(temp_result2,",");
         u_strcat(temp_result2,tmp->inflected);
         u_strcat(temp_result2,".");
         u_strcat(temp_result2,tmp->semantic_codes[0]);
         for (int z=1;z<tmp->n_semantic_codes;z++) {
            u_strcat(temp_result2,"+");
            u_strcat(temp_result2,tmp->semantic_codes[z]);
         }
         for (int z=0;z<tmp->n_inflectional_codes;z++) {
            u_strcat(temp_result2,":");
            u_strcat(temp_result2,tmp->inflectional_codes[z]);
         }
         u_strcat(temp_result2,"}");
         RESULT=RESULT+explore_portuguese_normalization_tree(temp_result,temp_result2,pronouns,norm_tree,alph);
      }
      free_dela_entry(tmp);
      tok=tok->next;
   }
   for (int i=0;i<n_inflectional_codes;i++) {
      free(inflectional_codes[i]);
   }

   // we go on with the next lemma
   struct list_ustring* tmp;
   tmp=lemmas;
   lemmas=lemmas->next;
   free_list_ustring_element(tmp);
}

// we clear the output in order to replace it by the normalization lines
free(L->output);
L->output=NULL;
if (temp_result[0]!='\0') {
   L->output=u_strdup(temp_result);
}
free_list_ustring(pronouns);
return RESULT;
}




//
// // this function returns 1 if every unichar of pattern is found in code
//
int are_compatible_portuguese_sub_codes(unichar* code,unichar* pattern) {
int i,j;
j=0;
while (pattern[j]!='\0') {
  i=0;
  while ((code[i]!='\0')&&(pattern[j]!=code[i])) i++;
  if (code[i]=='\0') return 0;
  j++;
}
return 1;
}




//
// this function checks if the dic entry is compatible with the inflectional code
// array passed in parameter
// it returns 1 on compatibility, 0 else
//
int compatible_portuguese_inflectional_codes(struct dela_entry* entry,int n_inflectional_codes,
                                             unichar** inflectional_codes) {
for (int n_dic=0;n_dic<entry->n_inflectional_codes;n_dic++) {
   for (int n=0;n<n_inflectional_codes;n++) {
      if (are_compatible_portuguese_sub_codes(entry->inflectional_codes[n_dic],inflectional_codes[n])) {
         return 1;
      }
   }
}
return 0;
}



//
// this function tokenizes a sequence like "dir-se-ia[:C1s]"
// into "dir", "se" and ":C1s"
// it returns 1 on success, 0 else
//
int tokenize_portuguese_match(unichar* s,unichar* radical,unichar* pronoun,unichar* suffix,unichar* inflectional_code) {
int i=0;
// we get the radical
int j=0;
while (s[i]!='\0' && s[i]!='-') {
   radical[j++]=s[i++];
}
radical[j]='\0';
if (s[i]=='\0') {
  return 0;
}
// we get the position of the last minus sign in the sequence, in order to process correctly
// the two-part pronouns in sequence like: "dar-no-los-as"
int last_minus_pos=u_strlen(s)-1;
while (last_minus_pos>=0 && s[last_minus_pos]!='-') {
   last_minus_pos--;
}
if (last_minus_pos<0) {
   // if there is an error, we return
   return 0;
}
// we get the pronoun
i++;
j=0;
while (s[i]!='\0' && i!=last_minus_pos) {
   pronoun[j++]=s[i++];
}
pronoun[j]='\0';
if (s[i]=='\0') {
  return 0;
}
// we get the suffix
i++;
j=0;
while (s[i]!='\0' && s[i]!='[') {
   suffix[j++]=s[i++];
}
suffix[j]='\0';
if (s[i]=='\0') {
  return 0;
}
// we get the inflectional code
while (s[i]!='\0' && s[i]!='[') {
   i++;
}
if (s[i]=='\0') {
  return 0;
}
i++;
j=0;
while (s[i]!='\0' && s[i]!=']') {
   inflectional_code[j++]=s[i++];
}
inflectional_code[j]='\0';
if (s[i]=='\0') {
  return 0;
}
return 1;
}



//
// this function look for the lemma corresponding to the radical
//
int get_radical_lemma(unichar* radical,struct list_ustring** lemmas,Alphabet* alph,unsigned char* root_bin,
                      struct INF_codes* root_inf) {
unichar entry[1000];
// we must use the entry variable because of the upper/lower case:
// if the radical is Dir, we want it to be dir in order to get the correct form
// after the call to uncompress_entry
int res=get_inf_number_for_token(4,radical,0,entry,alph,root_bin);
if (res==-1) {
   return 0;
}
(*lemmas)=NULL;
struct list_ustring* tok=root_inf->codes[res];
while (tok!=NULL) {
   unichar line[2000];
   uncompress_entry(entry,tok->string,line);
   struct dela_entry* tmp=tokenize_DELAF_line(line,0);
   if (!u_strcmp(tmp->semantic_codes[0],"V")) {
      // if we have a verb lemma, then we add it to the lemma list
      (*lemmas)=new_list_ustring(tmp->lemma,*lemmas);
   }
   free_dela_entry(tmp);
   tok=tok->next;
}
return 1;
}



//
// this function looks for the inf number associated to an inflected form
// it returns this number on success, -1 else
//
int get_inf_number_for_token(int pos,unichar* contenu,int string_pos,unichar* entry,Alphabet* ALPH,unsigned char* BIN) {
int n_transitions;
int ref;
n_transitions=((unsigned char)BIN[pos])*256+(unsigned char)BIN[pos+1];
pos=pos+2;
if (contenu[string_pos]=='\0') {
   entry[string_pos]='\0';
   // if we are at the end of the string
   if (!(n_transitions & 32768)) {
      ref=((unsigned char)BIN[pos])*256*256+((unsigned char)BIN[pos+1])*256+(unsigned char)BIN[pos+2];
      return ref;
   }
   return -1;
}
if ((n_transitions & 32768)) {
   // if we are in a normal node, we remove the control bit to
   // have the good number of transitions
   n_transitions=n_transitions-32768;
} else {
  // if we are in a final node, we must jump after the reference to the INF line number
  pos=pos+3;
}
for (int i=0;i<n_transitions;i++) {
  unichar c=(unichar)(((unsigned char)BIN[pos])*256+(unsigned char)BIN[pos+1]);
  pos=pos+2;
  int adr=((unsigned char)BIN[pos])*256*256+((unsigned char)BIN[pos+1])*256+(unsigned char)BIN[pos+2];
  pos=pos+3;
  if (is_equal_or_uppercase(c,contenu[string_pos],ALPH)) {
     // we explore the rest of the dictionary only
     // if the dico char is compatible with the token char
     entry[string_pos]=c;
     ref=get_inf_number_for_token(adr,contenu,string_pos+1,entry,ALPH,BIN);
     if (ref!=-1) {
        return ref;
     }
  }
}
return -1;
}




//
// this function saves the normalization rules into a file
//
void save_portuguese_normalization_grammar(int N,struct match_list* list,char* res_grf_name) {
FILE* f=u_fopen(res_grf_name,U_WRITE);
if (f==NULL) {
   error("Cannot create file %s\n",res_grf_name);
   return;
}
// we write the file header and the initial and final states
write_grf_header(800,100+50*N,N+2,NULL,f);
u_fprintf(f,"\"<E>\" 50 100 ",f);
u_fprintf(f,"%d ",N);
for (int i=2;i<2+N;i++) {
   u_fprintf(f,"%d ",i);
}
u_fprintf(f,"\n\"\" 600 100 0 \n");
// and then, we save the normalization rules
int current_state=2;
while (list!=NULL) {
   if (list->output!=NULL) {
      // if there is an output to process, we process it
      int i=0;
      while (list->output[i]=='@') {
         // as the string has the form @rule1@rule2@....@ruleN
         // we produce a rule for each of its tokens
         i++;
         int j=0;
         unichar s[1000];
         while (list->output[i]!='\0' && list->output[i]!='@') {
            s[j++]=list->output[i++];
         }
         s[j]='\0';
         u_fprintf(f,"\"%S",s);
         u_fprintf(f,"\" 200 ");
         u_fprintf(f,"%d 1 1 \n",50*(current_state-1));
         current_state++;
      }
   }
   struct match_list* l=list;
   list=list->next;
   if (l->output!=NULL) {
      free(l->output);
   }
   free(l);
}
u_fclose(f);
}



//
// This function takes a string like "@dir-lo-as/{diria,dizer.V:...}-" and a pronoun like "lo".
// It looks in the normalization tree all the rewriting possibilities for this pronoun.
// For each possibility, a complete line like "@dir-lo-as/{diria,dizer.V:...}-{lo,.PRO}" is
// produced, and then appended to the result string.
// It returns the number of lines produced.
//
int explore_portuguese_normalization_tree(unichar* result,unichar* partial_line,struct list_ustring* pronouns,
                                          struct normalization_tree* node,Alphabet* alph) {
int RES=0;
if (node==NULL) {
   error("Internal error: NULL node in explore_portuguese_normalization_tree\n");
   return 0;
}
if (pronouns==NULL) {
   // if we have followed all the pronoun tokens
   struct list_ustring* l=node->outputs;
   while (l!=NULL) {
      RES++;
      unichar temp[1000];
      u_strcpy(temp,partial_line);
      u_strcat(temp,l->string);
      u_strcat(result,temp);
      l=l->next;
   }
   return RES;
}
struct normalization_tree_transition* trans=node->trans;
while (trans!=NULL) {
   if (is_equal_or_uppercase(trans->s,pronouns->string,alph)) {
      // if the pronoun token is compatible with the tree token, according to
      // the upper/lower case variants described by the alphabet,
      // we continue the exploration of the tree
      RES=RES+explore_portuguese_normalization_tree(result,partial_line,pronouns->next,trans->node,alph);
   }
   trans=trans->next;
}
return RES;
}



//
// this function takes a pronoun sequence and tokenize looking at the minus signs
//
struct list_ustring* tokenize_portuguese_pronoun(unichar* pronoun) {
if (pronoun==NULL) {
   error("Internal error: NULL pronoun in tokenize_portuguese_pronoun");
   return NULL;
}
struct list_ustring* res=NULL;
int i=0;
unichar temp[1000];
int j;
while (pronoun[i]!='\0') {
   if (pronoun[i]=='-') {
      // if have a minus sign, we produce a minus string
      u_strcpy(temp,"-");
      // and we go on
      i++;
   } else {
      // if we have a letter sequence
      j=0;
      while (pronoun[i]!='\0' && pronoun[i]!='-') {
         temp[j++]=pronoun[i++];
      }
      temp[j]='\0';
      // there is no need to go to the next char of pronoun, because
      // it has been done in the while
   }
   // we insert the produced string in the string list
   res=insert_at_end_of_list(temp,res);
}
return res;
}
