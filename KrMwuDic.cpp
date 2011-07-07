/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include <stdlib.h>
#include <string.h>
#include "Unicode.h"
#include "Error.h"
#include "KrMwuDic.h"
#include "StringParsing.h"
#include "DELA.h"
#include "MF_SU_morphoBase.h"
#include "MF_SU_morpho.h"
#include "Vector.h"
#include "String_hash.h"


#define MAX_LINE_SIZE 4096
#define MAX_PARTS 32

void write_grf_start_things(U_FILE* grf,int *offset,int *start_state_offset,int *current_state,
                            int *end_state);
void write_grf_end_things(U_FILE* grf,int offset,int start_state_offset,int current_state,
                          vector_int* state_index);
void tokenize_kr_mwu_dic_line(vector_ptr* parts,unichar* line,Ustring* foo);
void produce_mwu_entries(U_FILE* grf,int n_parts,struct dela_entry** entries,MultiFlex_ctx* ctx,
                         Korean* korean,
                         struct l_morpho_t* morpho,
                         const VersatileEncodingConfig* vec,
                         vector_int* state_index,int *current_state,int end_state,int *line,
                         struct string_hash* subgraphs,int *subgraph_Y);
int tokens_to_dela_entries(vector_ptr* line_tokens,struct dela_entry** entries,int *n_entries,Ustring* foo,int line_number);
struct string_hash* get_codes_from_inf(const struct INF_codes* inf);
int upgrade_entries(struct dela_entry** entries,int n_entries,Dictionary* d,
					struct string_hash* dic_codes,Ustring* foo,int line_number);

/**
 * Builds the .grf dictionary corresponding to the given Korean compound DELAS.
 */
void create_mwu_dictionary(U_FILE* delas,U_FILE* grf,MultiFlex_ctx* ctx,
                           Korean* korean,struct l_morpho_t* morpho,
                           const VersatileEncodingConfig* vec,
                           Dictionary* d) {
int line_number=0;
struct dela_entry* entries[MAX_PARTS];
int n_parts;
int offset;
int current_state;
int start_state_offset;
int end_state;
write_grf_start_things(grf,&offset,&start_state_offset,&current_state,&end_state);
vector_int* state_index=new_vector_int(16);
struct string_hash* subgraphs=new_string_hash(DONT_USE_VALUES);
int line_grf=1;
int subgraph_Y=20;
struct string_hash* dic_codes=get_codes_from_inf(d->inf);
/* foo is a debug presentation of the input line */
Ustring* foo=new_Ustring(256);
Ustring* line=new_Ustring(MAX_LINE_SIZE);
int n_errors=0;
while (EOF!=readline(line,delas)) {
   /* We place the line counter here, so we can use 'continue' */
   line_number++;
   if (line->str[0]=='\0') {
      continue;
   }
   /* We split the line */
   vector_ptr* line_tokens=new_vector_ptr(32);
   tokenize_kr_mwu_dic_line(line_tokens,line->str,foo);
   /* Then we check that the n_parts-1 first ones are valid DELAF entries
    * and that the last one is a valid DELAS entry. But first, we initialize
    * the array */
   for (int i=0;i<MAX_PARTS;i++) {
      entries[i]=NULL;
   }
   int OK=tokens_to_dela_entries(line_tokens,entries,&n_parts,foo,line_number)
		   && upgrade_entries(entries,n_parts,d,dic_codes,foo,line_number);
   if (OK) {
      /* If everything went OK, we can start inflecting the root of the last
       * component */
      produce_mwu_entries(grf,n_parts,entries,ctx,korean,morpho,
            vec,state_index,
            &current_state,end_state,&line_grf,subgraphs,&subgraph_Y);
   } else {
	   n_errors++;
   }
   /* We free the tokens as well as the 'part' and 'entries' tab */
   free_vector_ptr(line_tokens,free);
   for (int i=0;i<n_parts;i++) {
      free_dela_entry(entries[i]);
   }
}
free_Ustring(line);
if (n_errors>0) {
	error("%d error%s found\n",n_errors,(n_errors>1)?"s":"");
}
write_grf_end_things(grf,offset,start_state_offset,current_state,state_index);
free_vector_int(state_index);
free_string_hash(subgraphs);
free_string_hash(dic_codes);
free_Ustring(foo);
}


/**
 * 'line' is supposed to be made of tab separated strings. Strings can be empty.
 * foo is filled with a debug version of the line.
 */
void tokenize_kr_mwu_dic_line(vector_ptr* parts,unichar* line,Ustring* foo) {
if (line==NULL) {
   fatal_error("NULL error in tokenize_kr_mwu_dic_line\n");
}
if (line[0]=='\0') {
   fatal_error("Empty line error in tokenize_kr_mwu_dic_line\n");
}
int pos=0;
unichar temp[MAX_LINE_SIZE];
empty(foo);
while (line[pos]!='\0') {
   parse_string(line,&pos,temp,P_TAB,P_EMPTY,NULL);
   vector_ptr_add(parts,u_strdup(temp));
   if (temp[0]!='\0') {
	   u_strcat(foo,temp);
	   u_strcat(foo,' ');
   }
   if (line[pos]=='\0') {
	   break;
   }
   pos++;
}
/* Removing last space */
remove_n_chars(foo,1);
}


/**
 * Adds the couple name/state in the given structure.
 */
void add_subgraph(struct string_hash* subgraphs,unichar* name,int state) {
subgraphs->size=state;
get_value_index(name,subgraphs);
}


/**
 * Returns the state number associated to the given graph name or -1 if
 * not found.
 */
int get_subgraph(struct string_hash* subgraphs,unichar* name) {
int n=get_value_index(name,subgraphs,DONT_INSERT);
if (n==NO_VALUE_INDEX) {
   /* NO_VALUE_INDEX==-1 but it's safer to do that in case the value ever changes */
   return -1;
}
return n;
}


/**
 * Takes a code of the form +XXXX and turns it into a YYYY sequence with
 * no + at the beginning and where all # have been replaced by :
 */
void get_post_position_graph_name(const unichar* code,unichar* name) {
code++;
int i=0;
while ((name[i]=code[i])!='\0') {
   if (name[i]=='#') {
      name[i]=':';
   }
   i++;
}
}


/**
 * Adds the given compound entries to the given grf.
 */
void produce_mwu_entries(U_FILE* grf,int n_parts,struct dela_entry** entries,MultiFlex_ctx* ctx,
                         Korean* korean,struct l_morpho_t* morpho,
                         const VersatileEncodingConfig* vec,
                         vector_int* state_index,int *current_state,int end_state,int *line,
                         struct string_hash* subgraphs,int *subgraph_Y) {
SU_forms_T forms;
SU_init_forms(&forms); //Allocate the space for forms and initialize it to null values
char inflection_code[1024];
unichar code_gramm[1024];
int semitic;
/* We take the first grammatical code, and we extract from it the name
 * of the inflection transducer to use */
get_inflection_code(entries[n_parts-1]->semantic_codes[0],
                    inflection_code, code_gramm, &semitic);
/* And we inflect the word */
const char* pkgdir="";
SU_inflect(ctx,morpho,vec,
      entries[n_parts-1]->lemma,inflection_code,
      NULL, &forms, semitic, korean, pkgdir);
if (forms.no_forms==0) {
   /* If no form was generated, we have nothing to do */
   SU_delete_inflection(&forms);
   return;
}
/* We have to save the first n_parts-1 components in the graph. We also
 * have to add the first state of the path in the 'state_index' vector */
vector_int_add(state_index,*current_state);
/* line is an approximation for a row coordinate in the grf */
unichar inflected_jamo[1024];
int foo_offset=-1;
for (int i=0;i<n_parts-1;i++) {
   /* We convert all inflected forms to Jamo, in order to speed up
    * dictionary lookup */
   Hanguls_to_Jamos(entries[i]->inflected,inflected_jamo,korean,0);
   u_fprintf(grf,"\"%S\" %d %d 1 %d \n",inflected_jamo,200+i*500,20+(*line)*50,(*current_state)+1);
   (*current_state)++;
   u_fprintf(grf,"\"<E>/%S,%S.%S",entries[i]->inflected,
                               entries[i]->lemma,entries[i]->semantic_codes[0]);
   for (int j=1;j<entries[i]->n_semantic_codes;j++) {
      u_fprintf(grf,"+%S",entries[i]->semantic_codes[j]);
   }
   for (int j=0;j<entries[i]->n_inflectional_codes;j++) {
      u_fprintf(grf,":%S",entries[i]->inflectional_codes[j]);
   }
   if (i<n_parts-2) {
      u_fprintf(grf,"}{\" %d %d 1 %d \n",400+i*500,20+(*line)*50,(*current_state)+1);
   } else {
      /* If this is the n_parts-2 component, we may have to generate several outputs
       * transitions, so we use the offset+0000000000 trick again */
      u_fprintf(grf,"}{\" %d %d %d ",400+i*500,20+(*line)*50,forms.no_forms);
      foo_offset=(int)ftell(grf);
      for (int k=0;k<forms.no_forms;k++) {
         u_fprintf(grf,"0000000000 ");
      }
      u_fprintf(grf,"\n");
   }
   (*current_state)++;
}

/* Now, we have to produce all the forms generated from the root inflection process */
int x=n_parts-1;
for (int i = 0; i < forms.no_forms; i++,(*line)++) {
   (*current_state)++;
	unichar tmp[1024];
	/* We have to convert to Hangul the result of the inflection process that may have
	 * produced a combination of Hangul and Jamos */
	convert_jamo_to_hangul(forms.forms[i].form,tmp,korean);
	Hanguls_to_Jamos(tmp,inflected_jamo,korean,0);
   u_fprintf(grf,"\"%S\" %d %d 1 %d \n",inflected_jamo,200+x*500,20+(*line)*50,(*current_state));
   /* We must backtrack to add the transition to this state */
   fseek(grf,foo_offset,SEEK_SET);
   u_fprintf(grf,"%010d ",(*current_state)-1);
   /* We note the new offset and we return at the end of the file */
   foo_offset=(int)ftell(grf);
   fseek(grf,0,SEEK_END);

   (*current_state)++;
   u_fprintf(grf,"\"<E>/%S,%S.%S", tmp,
                                  entries[n_parts-1]->lemma, code_gramm);
   /* We add the semantic codes, if any */
   for (int j = 1; j < entries[n_parts-1]->n_semantic_codes; j++) {
      u_fprintf(grf, "+%S", entries[n_parts-1]->semantic_codes[j]);
   }
   /* We may not want to output this information */
   unichar* code=forms.forms[i].local_semantic_code;
   if (code != NULL && code[0]!='\0') {
      if (code[0]!='+') {
         fatal_error("Invalid code %S produced by inflection grammar %s: should start with a +\n",code,inflection_code);
      }
      u_fprintf(grf, "%S", code);
   } else {
      fatal_error("Error in inflection grammar %s: it does not produce a +XXX code \n",inflection_code);
   }
   if (forms.forms[i].raw_features != NULL
         && forms.forms[i].raw_features[0] != '\0') {
      u_fprintf(grf, ":%S", forms.forms[i].raw_features);
   }
   /* Now, we must get the number of the state corresponding to the
    * post-position graph */
   int subgraph_state=get_subgraph(subgraphs,code);
   if (subgraph_state==-1) {
      /* If this is the first time we see this graph, we add it */
      subgraph_state=(*current_state);
      (*current_state)++;
      add_subgraph(subgraphs,code,subgraph_state);
      u_fprintf(grf, "}\" %d %d 1 %d \n",400+x*500,20+(*line)*50,subgraph_state);
      unichar graph_name[FILENAME_MAX];
      get_post_position_graph_name(code,graph_name);
      u_fprintf(grf, "\":%S\" 3600 %d 1 %d \n",graph_name,(*subgraph_Y),end_state);
      (*subgraph_Y)=(*subgraph_Y)+50;
   } else {
      u_fprintf(grf, "}\" %d %d 1 %d \n",400+x*500,20+(*line)*50,subgraph_state);
   }
}
SU_delete_inflection(&forms);

}


/**
 * Writes the grf header and somes states. '*offset' is used to remember where
 * in the file is to be written the number of states that we don't know yet.
 * We will temporarily write ten zeros that should be overwritten at the end of the
 * graph creation process. We use the same trick for the state that will contain
 * all transitions to all created paths. As this state will contain as many transitions
 * as compound entries, we can't create it right now. We will create it at the end, but
 * we will need to backtrack in the file to write its actual number the unique
 * output of state #2. This is why we use '*start_state_offset'.
 */
void write_grf_start_things(U_FILE* grf,int *offset,int *start_state_offset,int *current_state,int *end_state) {
u_fprintf(grf,"#Unigraph\n");
u_fprintf(grf,"SIZE 4000 840\n");
u_fprintf(grf,"FONT Haansoft Batang:  10\n");
u_fprintf(grf,"OFONT Haansoft Batang:B 10\n");
u_fprintf(grf,"BCOLOR 16777215\n");
u_fprintf(grf,"FCOLOR 0\n");
u_fprintf(grf,"ACOLOR 13487565\n");
u_fprintf(grf,"SCOLOR 255\n");
u_fprintf(grf,"CCOLOR 255\n");
u_fprintf(grf,"DBOXES y\n");
u_fprintf(grf,"DFRAME y\n");
u_fprintf(grf,"DDATE y\n");
u_fprintf(grf,"DFILE y\n");
u_fprintf(grf,"DDIR n\n");
u_fprintf(grf,"DRIG n\n");
u_fprintf(grf,"DRST n\n");
u_fprintf(grf,"FITS 100\n");
u_fprintf(grf,"PORIENT L\n");
u_fprintf(grf,"#\n");
(*offset)=(int)ftell(grf);
u_fprintf(grf,"0000000000\n");
u_fprintf(grf,"\"<E>\" 70 20 1 2 \n");
u_fprintf(grf,"\"\" 3800 20 0 \n");
u_fprintf(grf,"\"$<\" 105 20 1 ");
(*start_state_offset)=(int)ftell(grf);
u_fprintf(grf,"0000000000 \n");
/*
 * We don't write this one, since it must written as the last state of the grf,
 * in 'write_grf_end_things'
 *
 * u_fprintf(grf,"\"<E>//{\" 152 200 0 \n");*/
u_fprintf(grf,"\"$>\" 3750 20 1 1 \n");
(*end_state)=3;
(*current_state)=4;
}


/**
 * We create the state that will have transitions to all states specified in the
 * 'state_index' vector. We add it as the last state of the grf, and then, we
 * go back in the file to write the number of states and the index of this last state.
 */
void write_grf_end_things(U_FILE* grf,int offset,int start_state_offset,int current_state,
                          vector_int* state_index) {
u_fprintf(grf,"\"<E>//{\" 152 20 %d ",state_index->nbelems);
for (int i=0;i<state_index->nbelems;i++) {
   u_fprintf(grf,"%d ",state_index->tab[i]);
}
u_fprintf(grf,"\n");
fseek(grf,offset,SEEK_SET);
u_fprintf(grf,"%010d",current_state+1);
fseek(grf,start_state_offset,SEEK_SET);
u_fprintf(grf,"%010d",current_state);
/* For safety, we return at the end of the file */
fseek(grf,0,SEEK_END);
}


/**
 * Moves *pos to the next non empty string index of tab. Returns 1
 * in case of success; 0 if the end of tab has been reached.
 */
int next(int *pos,unichar** tab,int max) {
if (*pos==max) return 0;
(*pos)++;
while ((*pos)<max) {
	if (tab[*pos][0]!='\0') return 1;
	(*pos)++;
}
return 0;
}


/**
 * Tries to turn a token sequence into a dic entry sequence. Tokens are
 * expected to be organized as follows:
 *
 * W { A B C ... } { D E F ... } ...
 *
 * W is to be ignored.
 *
 * Returns 1 in case of success; 0 otherwise.
 */
int tokens_to_dela_entries(vector_ptr* tokens,struct dela_entry** entries,int *n_entries,
							Ustring* foo,int line_number) {
(*n_entries)=0;
int n=tokens->nbelems;
if (n<=1) return 0;
int pos=0;
unichar** tok=(unichar**)(tokens->tab);
if (!next(&pos,tok,n)) return 0;
Ustring* line=new_Ustring(64);
for (;;) {
	empty(line);
	/* Matching { */
	if (u_strcmp(tok[pos],"{")) {
		goto err;
	}
	if (!next(&pos,tok,n)) goto err;
	/* Matching inflected part */
	if (!u_strcmp(tok[pos],"}")) {
		goto err;
	}
	u_strcat(line,tok[pos]);
	u_strcat(line,",");
	if (!next(&pos,tok,n)) goto err;
	/* Matching lemma part */
	u_strcat(line,tok[pos]);
	u_strcat(line,".");
	if (!next(&pos,tok,n)) goto err;
	/* Matching code part */
	u_strcat(line,tok[pos]);
	if (!next(&pos,tok,n)) goto err;
	while (u_strcmp(tok[pos],"}")) {
		u_strcat(line,"+");
		u_strcat(line,tok[pos]);
		if (!next(&pos,tok,n)) goto err;
	}
	/* Now, we can build the dic entry */
	struct dela_entry* entry=tokenize_DELAF_line(line->str);
	if (entry==NULL) goto err;
	if (*n_entries==MAX_PARTS) {
		fatal_error("Too many dic entries in tokens_to_dela_entries\n");
	}
	entries[(*n_entries)++]=entry;
	if (entry->n_inflectional_codes!=0) {
		error("Error at line %d: %S\n",line_number,foo->str);
		error("Unexpected inflectional code(s)\n");
		goto err_no_msg;
	}
	if (!next(&pos,tok,n)) {
		/* If we have finished, we exit the loop */
		break;
	}
}
free_Ustring(line);
return 1;
err:
error("Error at line %d: %S\n",line_number,foo->str);
err_no_msg:
free_Ustring(line);
return 0;
}


/**
 * Builds and returns a string hash containing all grammatical/semantic codes
 * used in the .inf file
 */
struct string_hash* get_codes_from_inf(const struct INF_codes* inf) {
struct string_hash* h=new_string_hash(DONT_USE_VALUES);
struct list_ustring* l;
Ustring* tmp=new_Ustring(32);
unichar* s;
int pos;
for (int i=0;i<inf->N;i++) {
	l=inf->codes[i];
	while (l!=NULL) {
		s=l->string;
		pos=0;
		/* First, we locate the . */
		while (s[pos]!='.' && s[pos]!='\0') {
			if (s[pos]=='\\') {
				pos++;
				if (s[pos]=='\0') {
					fatal_error("Parse error in get_codes_from_inf for inf code %S\n",s);
				}
			}
			pos++;
		}
		if (s[pos]=='\0') {
			fatal_error("Parse error in get_codes_from_inf for inf code %S\n",s);
		}
		/* Then, we read all codes */
		while (s[pos]=='.' || s[pos]=='+') {
			empty(tmp);
			pos++;
			while (s[pos]!='+'  && s[pos]!=':' && s[pos]!='\0') {
				if (s[pos]=='\\') {
					pos++;
					if (s[pos]=='\0') {
						fatal_error("Parse error in get_codes_from_inf for inf code %S\n",s);
					}
				}
				u_strcat(tmp,s[pos]);
				pos++;
			}
			if (tmp->len==0) {
				fatal_error("Parse error in get_codes_from_inf for inf code %S\n",s);
			}
			get_value_index(tmp->str,h);
		}
		l=l->next;
	}
}
free_Ustring(tmp);
return h;
}


/**
 * Returns 1 if the given 'dic' entry is compatible with the 'e' one. The rules are:
 *
 * 1) e->semantic_code[0] must be a prefix of dic->semantic_code[0]
 * 2) all semantic codes of e that are in 'dic_codes' must also be in dic
 */
int are_compatible(struct dela_entry* e,struct dela_entry* dic,struct string_hash* dic_codes) {
if (!u_starts_with(dic->semantic_codes[0],e->semantic_codes[0])) {
	return 0;
}
unichar c=dic->semantic_codes[0][u_strlen(e->semantic_codes[0])];
if (c!='\0' && !u_is_digit(c)) {
	/* If the codes are not exactly the same, then we look for
	 * a digit to avoid that A may match ADV, for instance */
	return 0;
}
for (int i=1;i<e->n_semantic_codes;i++) {
	if (-1!=get_value_index(e->semantic_codes[i],dic_codes)) {
		if (!dic_entry_contain_gram_code(dic,e->semantic_codes[i])) {
			return 0;
		}
	}
}
return 1;
}


/**
 * Adds to e the codes of dic, replacing e's grammatical code by dic's one
 */
void upgrade_entry(struct dela_entry* e,struct dela_entry* dic) {
free(e->semantic_codes[0]);
e->semantic_codes[0]=u_strdup(dic->semantic_codes[0]);
for (int i=1;i<dic->n_semantic_codes;i++) {
	if (!dic_entry_contain_gram_code(e,dic->semantic_codes[i])) {
		e->semantic_codes[(e->n_semantic_codes)++]=u_strdup(dic->semantic_codes[i]);
	}
}
/* Finally, we have to test whether the grammatical code has been duplicated or not */
for (int i=1;i<e->n_semantic_codes;i++) {
	if (!u_strcmp(e->semantic_codes[0],e->semantic_codes[i])) {
		/* We remove the #i and shift the remaining codes */
		free(e->semantic_codes[i]);
		while (i<e->n_semantic_codes-1) {
			e->semantic_codes[i]=e->semantic_codes[i+1];
			i++;
		}
		(e->n_semantic_codes)--;
		break;
	}
}
}

/**
 * For each dic entry, this function looks up in the bin dictionary
 * to try to expand its codes. For instance, if we have an entry like
 *
 * 	ABC,.X+Y
 *
 * and if the binary dic contains:
 *
 * 	ABC,X08+W+Z
 *
 * then we want to obtain:
 *
 * 	ABC,X08+Y+W+Z
 *
 * Note that the grammatical code (the first one) may be replaced by a longer version if found
 * in the bin dictionary. To be considered, a binary entry must contains all codes of the
 * entry that are present in the binary (i.e. they are in 'dic_codes').
 *
 * If there is more than one binary entry that matches, then the function prints
 * an error message and returns 0;
 * otherwise it returns 1.
 */
int upgrade_entries(struct dela_entry** entries,int n_entries,Dictionary* d,
		            struct string_hash* dic_codes,
					Ustring* debug,int line_number) {
Ustring* line=new_Ustring(4096);
int OK=1;
for (int i=0;i<n_entries;i++) {
	struct dela_entry* e=entries[i];
	int inf_code=get_inf_code_exact_match(d,e->inflected);
	if (inf_code==-1) {
		/* No expansion to perform if the entry is not in the bin dictionary */
		continue;
	}
	/* Now, we uncompress all the entries found in the bin */
	vector_ptr* bin_entries=new_vector_ptr(8);
	struct list_ustring* l=d->inf->codes[inf_code];
	while (l!=NULL) {
		uncompress_entry(e->inflected,l->string,line);
		struct dela_entry* foo=tokenize_DELAF_line(line->str);
		if (foo==NULL) {
			error("Error at line %d: %S\n",line_number,debug->str);
			fatal_error("Internal error in upgrade_entries\n");
		}
		vector_ptr_add(bin_entries,foo);
		l=l->next;
	}
	/* Now, we count how many bin entries are compatible with the current one */
	unsigned int n_compatibles=0;
	for (int k=0;k<bin_entries->nbelems;k++) {
		if (are_compatible(e,(struct dela_entry*)(bin_entries->tab[k]),dic_codes)) {
			n_compatibles++;
		}
	}
	if (n_compatibles>1) {
		OK=0;
		error("Error at line %d: %S\n",line_number,debug->str);
		error("Cause: several possibilities for entry {");
		debug_print_entry(e);
		error("}:\n");
		for (int k=0;k<bin_entries->nbelems;k++) {
			if (are_compatible(e,(struct dela_entry*)(bin_entries->tab[k]),dic_codes)) {
				debug_println_entry((struct dela_entry*)(bin_entries->tab[k]));
			}
		}
	} else if (n_compatibles==1) {
		for (int k=0;k<bin_entries->nbelems;k++) {
			if (are_compatible(e,(struct dela_entry*)(bin_entries->tab[k]),dic_codes)) {
				/* We have found the only matching one, we expand it */
				upgrade_entry(e,(struct dela_entry*)(bin_entries->tab[k]));
			}
		}
	} else {
		/* If there is no compatible entry, we don't have to expand anything */
	}
	/* Don't forget to free bin entries */
	for (int k=0;k<bin_entries->nbelems;k++) {
		free_dela_entry((struct dela_entry*)(bin_entries->tab[k]));
	}
	free_vector_ptr(bin_entries,NULL);
}
free_Ustring(line);
return OK;
}
