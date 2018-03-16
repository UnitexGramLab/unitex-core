/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "MorphologicalFilters.h"
#include "Error.h"
#include "DELA.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#ifdef REGEX_FACADE_ENGINE

#define HASH_FILTERS_DIM 1024


static void free_FilterSet(FilterSet*,int);
static void w_extract_inflected(const unichar* tag_token, unichar_regex** inflected, size_t *buffer_size);
static void split_filter(const unichar*,unichar*,char*);


/**
 * This function creates and returns a filter set representing all the
 * morphological filters contained in the tags of the given fst2.
 * NULL is returned in case of malformed filter.
 */
FilterSet* new_FilterSet(Fst2* fst2,Alphabet* alph) {
struct string_hash* filters=new_string_hash(HASH_FILTERS_DIM);
Fst2Tag* tag=fst2->tags;
int i,ccode;
/* For each tag, we insert its filter, if any, in the filter set */
for (i=0;i<fst2->number_of_tags;i++) {
   if (tag[i]->morphological_filter!=NULL) {
      tag[i]->filter_number=get_value_index(tag[i]->morphological_filter,filters);
   } else {
      tag[i]->filter_number=-1;
   }
}
/* Then, */
FilterSet* filter_set=(FilterSet*)malloc(sizeof(FilterSet));
if (filter_set==NULL) {
   fatal_alloc_error("new_FilterSet");
}
if (filters->size>0) {
   unichar filterContent[2048];
   char filterOptions[2048];
   int regBasic,cflags;
   filter_set->size=filters->size;
   filter_set->filter=(MorphoFilter*)malloc(sizeof(MorphoFilter)*filters->size);
   if (filter_set->filter==NULL) {
      fatal_alloc_error("new_FilterSet");
   }
   for (i=0;i<filters->size;i++) {
      /* For each filter */
      filter_set->filter[i].content=NULL;
      filter_set->filter[i].options=NULL;
      filter_set->filter[i].matcher=NULL;
      /* We split the filter into a regular and the options, if any. For instance,
       * "<<able$>>_f_" will be turned into "able$" and "f". */
      split_filter(filters->value[i],filterContent,filterOptions);
      filter_set->filter[i].options=strdup(filterOptions);
      if (filter_set->filter[i].options==NULL) {
         fatal_alloc_error("new_FilterSet");
      }
      regBasic=0;
      int replaceLetters=1;
      for (int j=0;filterOptions[j]!='\0';j++) {
         switch (filterOptions[j]) {
            case 'f':
               replaceLetters=0;
               break;
            case 'b':
               regBasic=1;
               break;
            default:
               error("Morphological filter '%S' : ",filterContent);
               error("Invalid option(s) : '%s'\n",filterOptions);
               free_string_hash(filters);
               free_FilterSet(filter_set,i);
               return NULL;
         }
      }
      if (replaceLetters==1) {
         if(alph==NULL){
           error("\nWARNING: If you want to use a custom case-insensitive matching in morphological filters\n");
           error("you must define an alphabet file.\n");
         }
         /* If we must replace letters by the set of their case variants
          * like".+e" -> ".+[eE]" */
         unichar temp[2048];
         replace_letter_by_letter_set(alph,temp,filterContent);
         u_strcpy(filterContent,temp);
      }
      filter_set->filter[i].content=u_strdup(filterContent);
      cflags=REGEX_FACADE_REG_NOSUB;
      if (!regBasic) {
         cflags|=REGEX_FACADE_REG_EXTENDED;
      }
      filter_set->filter[i].matcher=(regex_facade_regex_t*)malloc(sizeof(regex_facade_regex_t));
      if (filter_set->filter[i].matcher==NULL) {
         fatal_alloc_error("new_FilterSet");
      }
      /* As the TRE library manipulates unichar_regex* strings, we must convert our unichar* one */
      /* we add 0x10 to prevent valgrind warning */
      unichar_regex* warray=(unichar_regex*)malloc(0x20+(sizeof(unichar_regex)*(u_strlen(filter_set->filter[i].content)+1)*UNICHAR_REGEX_ALLOC_FACTOR));
      if (warray==NULL) {
          fatal_alloc_error("new_FilterSet");
      }
      regex_facade_strcpy(warray,filter_set->filter[i].content);
      /* Then, we build the regular expression matcher associated to our filter */
      ccode=regex_facade_regcomp(filter_set->filter[i].matcher,warray,cflags);
      free(warray);
      if (ccode!=0) {
         error("Morphological filter '%S' : ",filter_set->filter[i].content);
         char errbuf[512];
         regex_facade_regerror(ccode,filter_set->filter[i].matcher,errbuf,512);
         error("Syntax error : %s\n",errbuf);
         free_string_hash(filters);
         free_FilterSet(filter_set,i);
         return NULL;
      }
   }
} else {
   /* No need to allocate an array if there is no filter */
   filter_set->size=0;
   filter_set->filter=NULL;
}
free_string_hash(filters);
return filter_set;
}


/**
 * Frees the memory associated to the given filter set. Only the first
 * 'n' filters are freed. This function should only be used when an
 * error occurs during the construction of a filter set.
 */
static void free_FilterSet(FilterSet* filters,int n) {
if (filters==NULL) return;
if (filters->filter==NULL) {
   free(filters);
   return;
}
for (int i=0;i<n;i++) {
   if (filters->filter[i].options!= NULL) free(filters->filter[i].options);
   if (filters->filter[i].content!= NULL) free(filters->filter[i].content);
   if (filters->filter[i].matcher!= NULL) {
       regex_facade_regfree(filters->filter[i].matcher);
       free(filters->filter[i].matcher);
   }
}
free(filters->filter);
free(filters);
}


/**
 * Frees all the memory associated to the given filter set.
 */
void free_FilterSet(FilterSet* filters){
if (filters==NULL) return;
free_FilterSet(filters,filters->size);
}


/**
 * Allocates, initializes and returns a structure that indicates for each token
 * which of the given filters it matches.
 */
FilterMatchIndex* new_FilterMatchIndex(FilterSet* filters,struct string_hash* tokens) {
int i,k;
unichar_regex *inflected=NULL;
size_t inflected_buffer_size = 0;
unichar* current_token;
FilterMatchIndex* index=(FilterMatchIndex*)malloc(sizeof(FilterMatchIndex));
if (index==NULL) {
   fatal_alloc_error("new_FilterMatchIndex");
}
if (filters->size>0) {
   index->size=filters->size;
   index->matching_tokens=(struct bit_array**)malloc(sizeof(struct bit_array*)*index->size);
   if (index->matching_tokens==NULL) {
      fatal_alloc_error("new_FilterMatchIndex");
   }
   /* We initialize the bit arrays */
   for (i=0;i<index->size;i++) {
      index->matching_tokens[i]=NULL;
   }
   /* Then, we look all the tokens */
   for (i=0;i<tokens->size;i++) {
      current_token=tokens->value[i];
      if (current_token[0]=='{' && u_strcmp(current_token,"{S}")
          && u_strcmp(current_token,"{STOP}")) {
         /* If we have a tag token like "{today,.ADV}", we extract its inflected form */
         w_extract_inflected(current_token,&inflected,&inflected_buffer_size);
      } else {
         /* Otherwise, we just convert the unichar* token into a unichar_regex* string */
         w_strcpy(&inflected, &inflected_buffer_size, current_token);
      }
      for (k=0;k<filters->size;k++) {
         if (regex_facade_regexec(filters->filter[k].matcher,inflected,0,NULL,0)==0) {
            /* If the current token matches the filter k */
            if (index->matching_tokens[k]==NULL) {
               /* If necessary, we allocate the bit array of the filter k */
               index->matching_tokens[k]=new_bit_array(tokens->size,ONE_BIT);
            }
            /* Then, we mark the token as being matched by the filter */
            set_value(index->matching_tokens[k],i,1);
         }
      }
   }
} else {
   /* If there is no filter */
   index->size=0;
   index->matching_tokens=NULL;
}
if (inflected != NULL) {
  free(inflected);
}
return index;
}


/**
 * Frees all the memory associated to the given FilterMatchIndex.
 */
void free_FilterMatchIndex(FilterMatchIndex* index) {
if (index==NULL) return;
for (int i=0;i<index->size;i++) {
   free_bit_array(index->matching_tokens[i]);
}
free(index->matching_tokens);
free(index);
}


/**
 * Returns 1 if the given string matches the given filter.
 * with a user provided buffer
 */
int string_match_filter(const FilterSet* filters, const unichar* s, int filter_number, unichar_regex* original_temp, size_t size_temp) {
unichar_regex* allocated_temp = NULL; const unichar_regex* temp;
temp = w_strcpy_optional_buffer(original_temp, size_temp, &allocated_temp, s, NULL, NULL);
int ret_value = !regex_facade_regexec(filters->filter[filter_number].matcher,temp,0,NULL,0);
free_wstring_optional_buffer(&allocated_temp, NULL);
return ret_value;
}


/**
 * Returns 1 if the given string matches the given filter.
 */
int string_match_filter(const FilterSet* filters,const unichar* s,int filter_number) {
#define STRING_MATCH_STRING_BUFFER_SIZE 8
unichar_regex original_temp[STRING_MATCH_STRING_BUFFER_SIZE]; unichar_regex* allocated_temp = NULL; const unichar_regex* temp;
temp = w_strcpy_optional_buffer(original_temp, STRING_MATCH_STRING_BUFFER_SIZE, &allocated_temp, s, NULL, NULL);
int ret_value = !regex_facade_regexec(filters->filter[filter_number].matcher,temp,0,NULL,0);
free_wstring_optional_buffer(&allocated_temp, NULL);
return ret_value;
}


/**
 * Returns 1 if the given filter can match the given token; 0 otherwise.
 */
int token_match_filter(FilterMatchIndex* index,int token,int filter_number) {
if (index==NULL) {
   fatal_error("NULL filter match index in token_match_filter\n");
}
if (index->matching_tokens==NULL || index->matching_tokens[filter_number]==NULL) {
   /* If there is no filter index array, it means that no token can match
    * this filter. */
   return 0;
}
return get_value(index->matching_tokens[filter_number],token);
}


/**
 * This function extracts the inflected form of the given tag token and
 * stores it in the given unichar_regex* string.
 */
static void w_extract_inflected(const unichar* tag_token,unichar_regex** inflected, size_t *buffer_size) {
struct dela_entry* entry=tokenize_tag_token(tag_token,1);
if (entry==NULL) {
   fatal_error("Invalid tag token in w_extract_inflected\n");
}
w_strcpy(inflected, buffer_size, entry->inflected);
free_dela_entry(entry);
}


/**
 * Splits a filter like "<<able$>>_f_" into its content "able$" and its options "f".
 */
static void split_filter(const unichar* filter_all,unichar* filter_content,char* filter_options) {
int i=2;
int j=0;
while (filter_all[i]!='\0' && (filter_all[i]!='>' || filter_all[i+1]!='>')) {
   filter_content[j++]=filter_all[i++];
}
if (j==0) {
   fatal_error("Empty filter in split_filter\n");
}
filter_content[j]='\0';
j=0;
if (filter_all[i]!='\0' && filter_all[i+2]=='_') {
   i=i+3;
   while (filter_all[i]!='\0' && filter_all[i]!='_') {
      filter_options[j++]=(char)filter_all[i++];
   }
}
filter_options[j]='\0';
}


#endif

} // namespace unitex
