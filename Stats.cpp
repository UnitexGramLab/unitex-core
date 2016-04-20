/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <math.h>
#include "File.h"
#include "Copyright.h"
#include "Text_tokens.h"
#include "LocateMatches.h"
#include "HashTable.h"
#include "Vector.h"
#include "Alphabet.h"
#include "UnitexGetOpt.h"

#include "Stats.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

// main work functions

int concord_stats(const char* , int , const char *, const char* , const char* , const char*,
    const VersatileEncodingConfig*, int , int, int );
int build_counted_concord(match_list* , text_tokens* , U_FILE* , Alphabet*, int , int , int, vector_ptr** , hash_table** );
int build_counted_collocates(match_list* , text_tokens* , U_FILE* , Alphabet*, int , int , int, vector_int** , hash_table** , hash_table** , hash_table** );

// ...main work functions

// structs for keys that can be both case sensitive and case-insensitive

struct vec_CS_tag
{
  vector_int* vec;
  int CStag;
  text_tokens* tokens;
  Alphabet* alphabet;
};

struct int_CS_tag
{
  int tokenID;
  int CStag;
  text_tokens* tokens;
  Alphabet* alphabet;
} ;

struct counted_match_descriptor
{
  int countOfMatch;
  int leftEndsAt;
  int rightStartsAt;
};

// ... structs


// local helper functions
inline int min_int(int, int);
inline int max_int(int, int);
inline long min_long(long, long);
inline long max_long(long, long);
vector_int* get_string_in_context_as_token_list(match_list*, int, int, int**, long*, long*, long, text_tokens*, U_FILE*, int, counted_match_descriptor*);
void print_string_token_list_with_count(U_FILE*,vector_int*, text_tokens*, counted_match_descriptor*);
int get_buffer_around_token(U_FILE*, int**, long, long, long, long*, long*);
int count_collocates(U_FILE* , text_tokens* , Alphabet*, int, hash_table* , hash_table** , int* );
int is_appropriate_token(int tokenID, text_tokens* tokens);

 // sort helper functions
 void sort_matches_ptr(void *, int , int , void (*)(void*, int, int), int (*)(void*, int, int, void*, void*), void *, void*, int);
 int partition_ptr(void *, int , int , void (*)(void*, int, int), int (*)(void*, int, int, void*, void*), void *, void*, int );
 void swap_ptr(void *, int , int );
 int compare_ptr(void *, int , int , void*, void* );
 void swap_int(void *, int , int );
 int compare_int(void *, int , int , void *, void*);
 int compare_double(void *, int , int , void *, void*);
 //...sort helper functions

 // hash helper functions
 unsigned int hash_vector_int(const void *);
 unsigned int jenkins_one_at_a_time_hash(const unsigned char *, size_t);
 unsigned int jenkins_one_at_a_time_hash_string_uppercase(const unichar *, size_t, Alphabet* );
 int vectors_equal(const void*, const void*);
 void free_vec(void*);
 void* copy_vec(const void*);

 unsigned int hash_token_as_int(const void*);
 int tokens_as_int_equal(const void*, const void*);
 void* copy_token_as_int(const void*);
 void free_token_as_int(void*);

 vec_CS_tag* new_vec_CS_tag(vector_int*, int, text_tokens*, Alphabet*);
 int_CS_tag* new_int_CS_tag(int, int, text_tokens*, Alphabet*);
 void free_vec_CS_tag(vec_CS_tag*);
 void free_int_CS_tag(int_CS_tag*);
 int tokens_equal_ignore_case(const unichar*, const unichar*, const Alphabet*);

 // TODO this should probably be moved to Alphabet.cpp
 unichar alphabet_to_upper(unichar c, Alphabet*alph);
 // ...hash helper functions
// ...local helper functions


#define STATS_BUFFER_LENGTH 4096


const char* usage_Stats =
  "Usage: Stats [OPTIONS] <concord>\n"
  "\n"
  "  <concord>: a concord.ind file\n"
  "\n"
  "OPTIONS:\n"
  "  -m MODE/--mode=MODE: specifies mode of operation: \n"
  "                       0 = left + match + right count\n"
  "                       1 = collocate count\n"
  "                       2 = collocate count with z-score\n"
  "  -a ALPH/--alphabet=ALPH: path to the alphabet file\n"
  "  -o OUT/--output=OUT: output file\n"
  "  -l N/--left=N: length of left context in tokens\n"
  "  -r N/--right=N: length of right context in tokens\n"
  "  -c N/--case=N: 0=case insensitive, 1=case sensitive (default is 1)\n"
  "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
  "  -h/--help: this help\n"
  "\n"
  "Computes some statistics.\n";

static void usage() {
  display_copyright_notice();
  u_printf(usage_Stats);
}


const char* optstring_Stats=":m:a:l:r:c:o:Vhk:q:";

const struct option_TS lopts_Stats[]= {
  {"mode",required_argument_TS,NULL,'m'},
  {"alphabet",required_argument_TS,NULL,'a'},
  {"left",required_argument_TS,NULL,'l'},
  {"right",required_argument_TS,NULL,'r'},
  {"case",optional_argument_TS,NULL,'c'},
  {"output",optional_argument_TS,NULL,'o'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {0, 0, 0, 0 }
 } ;

int main_Stats(int argc,char* const argv[]) {
if (argc <= 1) {
  usage();
  return SUCCESS_RETURN_CODE;
}

int leftContext = 0,  rightContext = 0, mode=-1, caseSensitive = 1;
char concord_ind[FILENAME_MAX]="";
char tokens_txt[FILENAME_MAX]="";
char text_cod[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
char alphabet[FILENAME_MAX]="";
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
char foo;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_Stats,lopts_Stats,&index))) {
   switch(val) {
   case 'm': if (1!=sscanf(options.vars()->optarg,"%d%c",&mode,&foo) || mode<0 || mode>2) {
                error("Invalid mode %s: should be 0, 1 or 2\n",options.vars()->optarg);
                return USAGE_ERROR_CODE;
             }
             break;
   case 'a': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty alphabet file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(alphabet,options.vars()->optarg);
             break;
  case 'l': if (1!=sscanf(options.vars()->optarg,"%d%c",&leftContext,&foo) || leftContext<0) {
               error("Invalid left context %s: should >=0\n",options.vars()->optarg);
               return USAGE_ERROR_CODE;
            }
            break;
  case 'r': if (1!=sscanf(options.vars()->optarg,"%d%c",&rightContext,&foo) || rightContext<0) {
               error("Invalid right context %s: should >=0\n",options.vars()->optarg);
               return USAGE_ERROR_CODE;
            }
            break;
  case 'c': if (1!=sscanf(options.vars()->optarg,"%d%c",&caseSensitive,&foo) || caseSensitive<0 || caseSensitive>1) {
               error("Invalid case mode %s: should be 0 or 1\n",options.vars()->optarg);
               return USAGE_ERROR_CODE;
            }
            break;
  case 'o': if (options.vars()->optarg[0]=='\0') {
               error("You must specify a non empty output file name\n");
               return USAGE_ERROR_CODE;
            }
            strcpy(output,options.vars()->optarg);
            break;
  case 'k': if (options.vars()->optarg[0]=='\0') {
              error("Empty input_encoding argument\n");
              return USAGE_ERROR_CODE;
            }
            decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
            break;
  case 'q': if (options.vars()->optarg[0]=='\0') {
              error("Empty output_encoding argument\n");
              return USAGE_ERROR_CODE;
            }
            decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
            break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Stats[index].name);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;
   }
   index=-1;
}

if (options.vars()->optind!=argc-1) {
   error("You must specify a concordance file\n");
   return USAGE_ERROR_CODE;
}
if (output[0]=='\0') {
   error("You must specify a output file\n");
   return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  return SUCCESS_RETURN_CODE;
}

strcpy(concord_ind,argv[options.vars()->optind]);
get_path(concord_ind,tokens_txt);
strcat(tokens_txt,"tokens.txt");
get_path(concord_ind,text_cod);
strcat(text_cod,"text.cod");

int return_value = concord_stats(output,
                                 mode,
                                 concord_ind,
                                 tokens_txt,
                                 text_cod,
                                 alphabet,
                                 &vec,
                                 leftContext,
                                 rightContext,
                                 caseSensitive);

return return_value;
}

/**
 * This is the main function for making statistics on concordances. Parameter mode represents
 * mode of operation, as there are 3 modes: count of match surrounded with left and right context.
 * concordfname is path to concord.ind containing locations of matches, tokens_path is path to
 * tokens.txt, codname is path to text.cod containing text represented as sequence of token IDs.
 * leftContext and rightContext are number of non-space tokens to look to the left and right from the
 * match to build string for counting. The function is_appropriate_token makes distinction between
 * "space"-like and "regular" tokens to include.
 */
int concord_stats(const char* outfilename,int mode, const char *concordfname, const char* tokens_path, const char* codname,
                  const char* alphabetName,
                  const VersatileEncodingConfig* vec,
                  int leftContext, int rightContext, int caseSensitive) {
  U_FILE* concord = u_fopen(vec, concordfname, U_READ);
  U_FILE* outfile = (outfilename == NULL) ? U_STDOUT : u_fopen(vec, outfilename, U_WRITE);
  U_FILE* cod = u_fopen(BINARY, codname, U_READ);
  match_list* matches = load_match_list(concord,NULL,NULL);
  u_fclose(concord);

  text_tokens* tokens = load_text_tokens(vec,tokens_path);

  if (tokens == NULL) {
    error("Error in build_counted_concord, tokens cannot be loaded!");

    match_list* current_match = matches;
    struct match_list* next_match = NULL;

    while(current_match != NULL) {
      next_match = current_match->next;
      free_match_list_element(current_match);
      current_match = next_match;
    }

    u_fclose(cod);

    if (outfile != U_STDOUT) {
      u_fclose(outfile);
    }

    return DEFAULT_ERROR_CODE;
  }

  Alphabet* alphabet = NULL;
  if (alphabetName!=NULL && alphabetName[0]!='\0') {
     alphabet=load_alphabet(vec,alphabetName);
     if (alphabet == NULL) {
        error("Error in concord_stats, alphabet cannot be loaded!");

        free_text_tokens(tokens);
        match_list* current_match = matches;
        struct match_list* next_match = NULL;

        while(current_match != NULL) {
          next_match = current_match->next;
          free_match_list_element(current_match);
          current_match = next_match;
        }

        u_fclose(cod);

        if (outfile != U_STDOUT) {
          u_fclose(outfile);
        }
        return DEFAULT_ERROR_CODE;
     }
  }

  int i;
  int counted_concord_return_value     = SUCCESS_RETURN_CODE;
  int counted_collocates_return_value  = SUCCESS_RETURN_CODE;

  any* hash_val = NULL;
  counted_match_descriptor* descriptor = NULL;

  if (mode == 0) {
    vector_ptr* allMatches     = NULL;
    hash_table* countsPerMatch = NULL;

    counted_concord_return_value = build_counted_concord(matches,
                                                         tokens,
                                                         cod,
                                                         alphabet,
                                                         leftContext,
                                                         rightContext,
                                                         caseSensitive,
                                                         &allMatches,
                                                         &countsPerMatch);
   // return when build_counted_concord() fails
   if(counted_concord_return_value != SUCCESS_RETURN_CODE) {
      free_alphabet(alphabet);
      free_text_tokens(tokens);

      match_list* current_match     = matches;
      struct match_list* next_match = NULL;

      while(current_match != NULL) {
        next_match = current_match->next;
        free_match_list_element(current_match);
        current_match = next_match;
      }

      u_fclose(cod);

      if (outfile != U_STDOUT) {
        u_fclose(outfile);
      }

      return counted_concord_return_value;
    }

    // now we sort
    sort_matches_ptr(allMatches->tab, 0, allMatches->nbelems-1, swap_ptr, compare_ptr, countsPerMatch, NULL, -1);

    // and then print
    for (i = 0; i < allMatches->nbelems ; i++) {
      hash_val = get_value(countsPerMatch, allMatches->tab[i], HT_DONT_INSERT);
      descriptor = (counted_match_descriptor*)hash_val->_ptr;
      print_string_token_list_with_count(outfile, ((vec_CS_tag*)(allMatches->tab[i]))->vec, tokens, descriptor);
      // we free descriptors here
      free(descriptor);
      hash_val->_ptr = NULL;
    }

    free_hash_table(countsPerMatch);
    free_vector_ptr(allMatches, NULL);
  } else if (mode == 1) {
    vector_int* allMatches     = NULL;
    hash_table* countsPerMatch = NULL;

    counted_collocates_return_value = build_counted_collocates(matches,
                                                               tokens,
                                                               cod,
                                                               alphabet,
                                                               leftContext,
                                                               rightContext,
                                                               caseSensitive,
                                                               &allMatches,
                                                               &countsPerMatch,
                                                               NULL,
                                                               NULL);
    // return when build_counted_collocates() fails
    if(counted_concord_return_value != SUCCESS_RETURN_CODE) {
      free_alphabet(alphabet);
      free_text_tokens(tokens);

      match_list* current_match     = matches;
      struct match_list* next_match = NULL;

      while(current_match != NULL) {
        next_match = current_match->next;
        free_match_list_element(current_match);
        current_match = next_match;
      }

      u_fclose(cod);

      if (outfile != U_STDOUT) {
        u_fclose(outfile);
      }

      return counted_collocates_return_value;
    }

    // now we sort
    int_CS_tag* sampleKey = new_int_CS_tag(0, caseSensitive, tokens, alphabet);
    sort_matches_ptr(allMatches->tab, 0, allMatches->nbelems-1, swap_int, compare_int, countsPerMatch, sampleKey, -1);
    free_int_CS_tag(sampleKey);

    // and then print
    int K;

    int_CS_tag* currentKey = new_int_CS_tag(0, caseSensitive, tokens, alphabet);

    for (i = 0 ; i < allMatches->nbelems ; i++) {
      currentKey->tokenID = allMatches->tab[i];
      hash_val = get_value(countsPerMatch, currentKey, HT_DONT_INSERT);

      K = hash_val->_int;
      u_fprintf(outfile,"%S\t%d\n", tokens->token[allMatches->tab[i]], K);
    }

    free_int_CS_tag(currentKey);
    free_vector_int(allMatches);
    free_hash_table(countsPerMatch);
  } else if (mode == 2) {
    vector_int* allMatches      = NULL;
    hash_table* countsPerMatch  = NULL;
    hash_table* z_score         = NULL;
    hash_table* countInCorpora  = NULL;

    counted_collocates_return_value = build_counted_collocates(matches,
                                                               tokens,
                                                               cod,
                                                               alphabet,
                                                               leftContext,
                                                               rightContext,
                                                               caseSensitive,
                                                               &allMatches,
                                                               &countsPerMatch,
                                                               &z_score,
                                                               &countInCorpora);

    // return when build_counted_collocates() fails
    if(counted_concord_return_value != SUCCESS_RETURN_CODE) {
      free_alphabet(alphabet);
      free_text_tokens(tokens);

      match_list* current_match     = matches;
      struct match_list* next_match = NULL;

      while(current_match != NULL) {
        next_match = current_match->next;
        free_match_list_element(current_match);
        current_match = next_match;
      }

      u_fclose(cod);

      if (outfile != U_STDOUT) {
        u_fclose(outfile);
      }

      return counted_collocates_return_value;
    }

    // now we sort
    int_CS_tag* sampleKey = new_int_CS_tag(0, caseSensitive, tokens, alphabet);
    sort_matches_ptr(allMatches->tab, 0, allMatches->nbelems-1, swap_int, compare_double, z_score, sampleKey, -1);
    free_int_CS_tag(sampleKey);

    // and then print
    int Fc, K;
    double zScore;
    int_CS_tag* key = new_int_CS_tag(0, caseSensitive, tokens, alphabet);

    for (i = 0 ; i < allMatches->nbelems ; i++) {
      key->tokenID = allMatches->tab[i];

      hash_val = get_value(countsPerMatch, key, HT_DONT_INSERT);

      K = hash_val->_int;

      hash_val = get_value(countInCorpora, key, HT_DONT_INSERT);

      Fc = hash_val->_int;

      hash_val = get_value(z_score, key, HT_DONT_INSERT);

      zScore = *((double*)hash_val->_ptr);

      u_fprintf(outfile,"%S\t%d\t%d\t%f\n", tokens->token[allMatches->tab[i]], Fc, K, zScore);
    }


    // we have to deallocate pointers to doubles allocated for zscore
    /*for (i = 0 ; i < allMatches->nbelems; i++)
    {
      key->tokenID = allMatches->tab[i];
      hash_val = get_value(z_score, key, HT_DONT_INSERT);
      free(hash_val->_ptr);
    }*/

    free_int_CS_tag(key);
    free_vector_int(allMatches);
    free_hash_table(countsPerMatch);
    free_hash_table(z_score);
    free_hash_table(countInCorpora);
  }

  free_alphabet(alphabet);

  free_text_tokens(tokens);

  match_list* current_match     = matches;
  struct match_list* next_match = NULL;

  while(current_match != NULL) {
    next_match = current_match->next;
    free_match_list_element(current_match);
    current_match = next_match;
  }

  u_fclose(cod);

  if (outfile != U_STDOUT) {
    u_fclose(outfile);
  }

  return SUCCESS_RETURN_CODE;
}

/**
 * This function builds all strings that are based on matches found in original text surrounded with
 * left and right context. It outputs a vector and a hash table - vector contains distinct strings
 * found, and these strings are key to the hash table containing count per string in corpora. Strings
 * are represented by integer vector containing token IDs.
 */
int build_counted_concord(match_list* matches, text_tokens* tokens, U_FILE* cod, Alphabet* alphabet, int leftContext, int rightContext, int caseSensitive, vector_ptr** ret_vector, hash_table** ret_hash) {
  if (ret_vector == NULL) {
    error("Fatal error in build_counted_concord, ret_vector cannot be NULL!");
    return DEFAULT_ERROR_CODE;
  }

  if (ret_hash == NULL) {
    error("Fatal error in build_counted_concord, ret_hash cannot be NULL!");
    return DEFAULT_ERROR_CODE;
  }

  long codSize = get_file_size(cod) / sizeof(int);
  int* buffer = NULL;

  long buff_start, buff_end;

  // we initialize buffer to encompass 4096 tokens from starting position
  int get_buffer_return_value = get_buffer_around_token(cod,
                                                        &buffer,
                                                        0,
                                                        0,
                                                        STATS_BUFFER_LENGTH,
                                                        &buff_start,
                                                        &buff_end);
  if(get_buffer_return_value != SUCCESS_RETURN_CODE) {
    free(buffer);
    return get_buffer_return_value;
  }

  any* hash_val = NULL;
  int hash_ret;

  vector_ptr* allMatches       = new_vector_ptr();
  hash_table* countPerMatch    = new_hash_table(hash_vector_int, vectors_equal, free_vec, free,copy_vec);
  vector_int* currentMatchList = NULL;
  vec_CS_tag* currentKey       = NULL;

  match_list* current_match    = matches;

  counted_match_descriptor* descriptor = NULL;
  counted_match_descriptor tmpDescriptor;

  int i = 0;

  // for all matches, we form list of token IDs and check it against hash table
  while(current_match != NULL) {
    currentMatchList = get_string_in_context_as_token_list(current_match, leftContext, rightContext, &buffer, &buff_start, &buff_end, codSize, tokens, cod, 1, &tmpDescriptor);

    currentKey = new_vec_CS_tag(currentMatchList, caseSensitive, tokens, alphabet);
    if (!currentKey) {
      free_hash_table(countPerMatch);
      free_vector_ptr(allMatches, NULL);
      free(buffer);
      return ALLOC_ERROR_CODE;
    }

    hash_val = get_value(countPerMatch, currentKey, HT_INSERT_IF_NEEDED, &hash_ret);

    if (hash_ret == HT_KEY_ADDED) {
      // new value, we need to set descriptor
      descriptor = (counted_match_descriptor*)malloc(sizeof(counted_match_descriptor));
      if (descriptor == NULL) {
        alloc_error("build_counted_concord, counted_match_descriptor");
        free_vec_CS_tag(currentKey);
        free_hash_table(countPerMatch);
        free_vector_ptr(allMatches, NULL);
        free(buffer);
        return ALLOC_ERROR_CODE;
      }
      descriptor->countOfMatch  = 1;
      descriptor->leftEndsAt    = tmpDescriptor.leftEndsAt;
      descriptor->rightStartsAt = tmpDescriptor.rightStartsAt;
      hash_val->_ptr            = descriptor;

      vector_ptr_add(allMatches, currentKey);
    }
    else {
      descriptor = (counted_match_descriptor*)(hash_val->_ptr);
      descriptor->countOfMatch++;
      // we need to free, since it was not added to the table
      free_vec_CS_tag(currentKey);
    }

    current_match = current_match->next;
    i++;
  }

  free(buffer);

  *ret_vector = allMatches;
  *ret_hash   = countPerMatch;

  return SUCCESS_RETURN_CODE;
}

/**
 * This function performs collocates count. It has two modes of operation, corresponding to modes 1 and
 * 2 of main program. In mode 1, it only looks at tokens in left and right context and counts them
 * each time they appear in the context of a match. In this mode, it returns an int vector containing
 * all possible tokens found in left and right context of a match, as well as hash table containing
 * counts per tokens in context. In mode 2, it returns additional 2 hash tables, z_score hash table which
 * represents z-score of a collocate and countsInCorpora hash table which returns total count of a token
 * found in context of a match in the whole corpora.
 */
int build_counted_collocates(match_list* matches, text_tokens* tokens, U_FILE* cod, Alphabet* alphabet, int leftContext, int rightContext, int caseSensitive, vector_int** ret_vector, hash_table** ret_hash, hash_table** z_score, hash_table** countsInCorpora) {
  if (ret_vector == NULL) {
    error("Fatal error in build_counted_collocates, ret_vector cannot be NULL!");
    return DEFAULT_ERROR_CODE;
  }

  if (ret_hash == NULL) {
    error("Fatal error in build_counted_collocates, ret_hash cannot be NULL!");
    return DEFAULT_ERROR_CODE;
  }

  long codSize = get_file_size(cod) / sizeof(int);
  int* buffer = NULL;

  long buff_start, buff_end;

  // we initialize buffer to encompass 4096 tokens from starting position
  int get_buffer_return_value = get_buffer_around_token(cod,
                                                        &buffer,
                                                        0,
                                                        0,
                                                        STATS_BUFFER_LENGTH,
                                                        &buff_start,
                                                        &buff_end);

  if(get_buffer_return_value != SUCCESS_RETURN_CODE) {
    free(buffer);
    return get_buffer_return_value;
  }

  any* hash_val = NULL;
  int hash_ret;

  int i;

  vector_int* allMatches        = new_vector_int();
  hash_table* countPerCollocate = new_hash_table(hash_token_as_int,
                                                 tokens_as_int_equal,
                                                 free_token_as_int,
                                                 NULL,
                                                 copy_token_as_int);
  vector_int* currentMatchList;
  vector_int* tmpMatchList;

  match_list* current_match = matches;

  hash_table* collocateCountInCorpora = NULL;
  int totalMatches = 0;
  int corporaLength;
  int wordsTakenByMatches = 0;
  int totalWindow = 0;
  int_CS_tag* currentKey = NULL;

  // for all matches, we form list of token IDs and check it against hash table
  while(current_match != NULL) {
    currentMatchList = get_string_in_context_as_token_list(current_match,
                                                             leftContext,
                                                             rightContext,
                                                             &buffer,
                                                             &buff_start,
                                                             &buff_end,
                                                             codSize,
                                                             tokens,
                                                             cod,
                                                             0,
                                                             NULL);

    // now we don't just insert the whole match as we did in build_counted_concord, but
    // for each token in the left and right context we treat it as a possible entry to a hash
    // table

    for (i = 0 ; i < currentMatchList->nbelems ; i++) {
      // we don't want space, sentence or stop tokens in results

      if (!is_appropriate_token(currentMatchList->tab[i], tokens)) {
        continue;
      }

      currentKey = new_int_CS_tag(currentMatchList->tab[i], caseSensitive, tokens, alphabet);
      if (!currentKey) {
        free_hash_table(countPerCollocate);
        free_vector_int(allMatches, NULL);
        free(buffer);
        return ALLOC_ERROR_CODE;
      }

      hash_val = get_value(countPerCollocate, currentKey, HT_INSERT_IF_NEEDED, &hash_ret);

      if (hash_ret == HT_KEY_ADDED) {
        // new value, we need to set count to 1
        hash_val->_int = 1;
        vector_int_add(allMatches, currentMatchList->tab[i]);
      } else {
        hash_val->_int++;
      }
      free_int_CS_tag(currentKey);
    }

    // if we're calculating z-score as well, we have to account for totalWindow score
    // which represents total space in non-space tokens taken by matches and their
    // left and right contexts

    if (z_score != NULL && countsInCorpora != NULL) {
      // first we account for number of non-space tokens taken by the match itself
      tmpMatchList = get_string_in_context_as_token_list(current_match, 0, 0, &buffer, &buff_start, &buff_end, codSize, tokens, cod, 1, NULL);

      for (i = 0 ; i < tmpMatchList->nbelems ; i++)
      {
        if (is_appropriate_token(tmpMatchList->tab[i], tokens))
        {
          totalWindow++;
          wordsTakenByMatches++;
        }
      }

      free_vector_int(tmpMatchList);
      // then we account for number of non-space tokens taken by left and right context

      for (i = 0 ; i < currentMatchList->nbelems; i++) {
        if (is_appropriate_token(currentMatchList->tab[i], tokens)) {
          totalWindow++;
        }
      }
    }

    // in this method, we free anyway, since this is no longer needed
    free_vector_int(currentMatchList);
    current_match = current_match->next;
    totalMatches++;
  }

  free(buffer);
  *ret_vector = allMatches;
  *ret_hash   = countPerCollocate;


  // we don't proceed with calculating z-score unless it's required
  if (z_score == NULL || countsInCorpora == NULL) {
    return SUCCESS_RETURN_CODE;
  }

  // now we count all collocates in corpus
  int count_collocates_return_value = count_collocates(cod,
                                                       tokens,
                                                       alphabet,
                                                       caseSensitive,
                                                       countPerCollocate,
                                                       &collocateCountInCorpora,
                                                       &corporaLength);

  // return when count_collocates() fails
  if (count_collocates_return_value != SUCCESS_RETURN_CODE) {
    return count_collocates_return_value;
  }

  // now we build z_score hash per collocate
  hash_table* zret  = new_hash_table(hash_token_as_int, tokens_as_int_equal, free_token_as_int, free,
                copy_token_as_int);
  double* tmpZScore = NULL;
  int K, Fc;
  double p;
  double E;

  for (i = 0 ; i < allMatches->nbelems ; i++) {
    currentKey = new_int_CS_tag(allMatches->tab[i], caseSensitive, tokens, alphabet);
    hash_val   = get_value(countPerCollocate, currentKey, HT_DONT_INSERT);

    K = hash_val->_int;

    hash_val = get_value(collocateCountInCorpora, currentKey, HT_DONT_INSERT);

    Fc = hash_val->_int;

    tmpZScore = (double*)malloc(sizeof(double));

    if (tmpZScore == NULL) {
      alloc_error("build_counted_collocates");
      free_hash_table(zret);
      free_hash_table(collocateCountInCorpora);
      free_int_CS_tag(currentKey);
      free_hash_table(countPerCollocate);
      free_vector_int(allMatches, NULL);
      return ALLOC_ERROR_CODE;
    }

    p = (double)Fc / (corporaLength - wordsTakenByMatches);
    E = p * totalWindow;

    *tmpZScore = (K - E) / sqrt(E * (1 - p));

    hash_val = get_value(zret, currentKey, HT_INSERT_IF_NEEDED);
    hash_val->_ptr = tmpZScore;

    free_int_CS_tag(currentKey);
  }

  *countsInCorpora = collocateCountInCorpora;
  *z_score         = zret;
  return SUCCESS_RETURN_CODE;
}

/**
 * This is a helper function for build_counted_collocates. It counts all collocates found for a specific
 * match list in corpora and returns the result as a hash table. Additional result - corpora_length, returns
 * total length of corpora in non-space tokens. Non-space tokens are determined by the result
 * of is_appropriate_token function.
 */
int count_collocates(U_FILE* cod, text_tokens* tokens, Alphabet* alphabet, int caseSensitive, hash_table* collocates, hash_table** ret_hash, int* corpora_length) {
  if (ret_hash == NULL) {
    error("Error in count_collocates, ret_hash cannot be null!");
    return DEFAULT_ERROR_CODE;
  }

  int* buffer   = NULL;
  long bufferStart;
  long bufferEnd;

  int get_buffer_return_value = get_buffer_around_token(cod,
                                                        &buffer,
                                                        0,
                                                        0,
                                                        STATS_BUFFER_LENGTH,
                                                        &bufferStart,
                                                        &bufferEnd);

  if(get_buffer_return_value != SUCCESS_RETURN_CODE) {
    free(buffer);
    return get_buffer_return_value;
  }

  hash_table* ret = new_hash_table(hash_token_as_int,
                                   tokens_as_int_equal,
                                   free_token_as_int,
                                   NULL,
                                   copy_token_as_int);

  *corpora_length = 0;
  int_CS_tag* currentKey = NULL;
  int hash_ret;
  any* hash_val = NULL;
  int i ;
  long codSize  = get_file_size(cod) / sizeof(int);
  for (i = 0 ; i < codSize ; i++) {
    if (i < bufferStart || i > bufferEnd) {
      get_buffer_return_value = get_buffer_around_token(cod,
                                                        &buffer,
                                                        i,
                                                        0,
                                                        STATS_BUFFER_LENGTH,
                                                        &bufferStart,
                                                        &bufferEnd);
      if(get_buffer_return_value != SUCCESS_RETURN_CODE) {
        free(buffer);
        return get_buffer_return_value;
      }
    }

    if (is_appropriate_token(buffer[i - bufferStart], tokens)) {
      (*corpora_length)++;
    }

    currentKey = new_int_CS_tag(buffer[i - bufferStart], caseSensitive, tokens, alphabet);

    hash_val = get_value(collocates, currentKey, HT_DONT_INSERT, &hash_ret);

    if (hash_val!=NULL && hash_ret == HT_KEY_ALREADY_THERE) {
      // this means that we can count this collocate in another hash

      int_CS_tag* insertKey = new_int_CS_tag(buffer[i - bufferStart], caseSensitive, tokens, alphabet);

      hash_val = get_value(ret, insertKey, HT_INSERT_IF_NEEDED, &hash_ret);

      if (hash_ret == HT_KEY_ADDED) {
        hash_val->_int = 1;
      }
      else {
        hash_val->_int++;
      }
      free_int_CS_tag(insertKey);
    }

    free_int_CS_tag(currentKey);
  }

  *ret_hash = ret;

  free(buffer);

  return SUCCESS_RETURN_CODE;
}

inline int min_int(int a, int b) {
  return (a < b) ? a : b;
}

inline int max_int(int a, int b) {
  return (a > b) ? a : b;
}

inline long min_long(long a, long b) {
  return (a < b) ? a : b;
}

inline long max_long(long a, long b) {
  return (a > b) ? a : b;
}

/**
 * This is a helper function which detects left and right context of a particular match and returns it
 * as an int vector. It expands match to the left and right by number of "appropriate" non-space tokens.
 * These tokens are determined by is_appropriate_token function.
 */
vector_int* get_string_in_context_as_token_list(match_list* match, int leftContext, int rightContext, int** buffer, long* bufferStart, long* bufferEnd, long totalSize, text_tokens* tokens, U_FILE* source, int includeMatch, counted_match_descriptor* descriptor) {
  long i;

  if (match == NULL) {
    error("Error in get_string_as_token_list, match cannot be NULL!");
    return NULL;
  }

  if (buffer == NULL) {
    error("Error in get_string_as_token_list, buffer cannot be NULL");
    return NULL;
  }

  if (*bufferStart > *bufferEnd) {
    error("Error in get_string_as_token_list, bufferStart must be <= bufferEnd!");
    return NULL;
  }

  vector_int* res = new_vector_int();

  long startFrom = match->m.start_pos_in_token - 1;
  long endAt = match->m.end_pos_in_token + 1;
  int foundLeft = 0;
  int foundRight = 0;

  // first we skip sentence and space tokens to the left and right until we have
  // enough left and right context to work with
  int get_buffer_return_value = SUCCESS_RETURN_CODE;
  while(startFrom >= 0 && foundLeft < leftContext) {
    if (startFrom < *bufferStart || startFrom > *bufferEnd) {
      // request new buffer
      get_buffer_return_value = get_buffer_around_token(source,
                                                        buffer,
                                                        startFrom,
                                                        STATS_BUFFER_LENGTH,
                                                        0,
                                                        bufferStart,
                                                        bufferEnd);
      if(get_buffer_return_value != SUCCESS_RETURN_CODE) {
        return NULL;
      }
    }

    if (is_appropriate_token((*buffer)[startFrom - *bufferStart], tokens)) {
      foundLeft++;
    }
    startFrom--;
  }

  if (descriptor != NULL) {
    // we set where left context ends (last token of left context)
    descriptor->leftEndsAt = (int)(match->m.start_pos_in_token - 1 - (startFrom + 1));
    // we set where right context starts (first token of right context)
    descriptor->rightStartsAt = (int)(match->m.end_pos_in_token + 1 - (startFrom + 1));
  }

  while(endAt <= totalSize-1 && foundRight < rightContext) {
    if (endAt > *bufferEnd || endAt < *bufferStart) {
      // request new buffer
       get_buffer_return_value = get_buffer_around_token(source,
                                                         buffer,
                                                         endAt,
                                                         0,
                                                         STATS_BUFFER_LENGTH,
                                                         bufferStart,
                                                         bufferEnd);
      if(get_buffer_return_value != SUCCESS_RETURN_CODE) {
        return NULL;
      }
    }

    if (is_appropriate_token((*buffer)[endAt - *bufferStart], tokens)) {
      foundRight++;
    }
    endAt++;
  }

  for (i = startFrom + 1 ; i <= endAt - 1 ; i++) {
    if (!includeMatch && i >= match->m.start_pos_in_token && i <= match->m.end_pos_in_token) {
      // we don't want the match, just the context
      continue;
    }

    if (i < *bufferStart || i > *bufferEnd) {
      int get_buffer_around_token_return_value = get_buffer_around_token(source,
                                                            buffer,
                                                            i,
                                                            0,
                                                            STATS_BUFFER_LENGTH,
                                                            bufferStart,
                                                            bufferEnd);
      if(get_buffer_around_token_return_value != SUCCESS_RETURN_CODE) {
        return NULL;
      }
    }

    vector_int_add(res, (*buffer)[i - *bufferStart]);
  }
  return res;
}

unsigned int hash_vector_int(const void* vec) {
  const vec_CS_tag* v = (const vec_CS_tag*)vec;
  const vector_int* vector = v->vec;
  if (v->CStag) {
    return jenkins_one_at_a_time_hash((const unsigned char*)&vector->tab[0], sizeof(int) * vector->nbelems);
  } else {
    // since we're in case-insensitive mode, we have to look at the contents of a token

    int i = 0;
    int hash = 0;

    for (i = 0 ; i < vector->nbelems ; i++) {
      hash ^= jenkins_one_at_a_time_hash_string_uppercase(v->tokens->token[vector->tab[i]], u_strlen(v->tokens->token[vector->tab[i]]), v->alphabet);
    }

    return hash;
  }
}

unsigned int jenkins_one_at_a_time_hash(const unsigned char *key, size_t key_len) {
    unsigned int hash = 0;
    size_t i;

    for (i = 0; i < key_len; i++) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

unsigned int jenkins_one_at_a_time_hash_string_uppercase(const unichar *key, size_t key_len, Alphabet* alphabet) {
    unsigned int hash = 0;
    size_t i;

    for (i = 0; i < key_len; i++) {
        hash +=  alphabet_to_upper(key[i], alphabet);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}


void print_string_token_list_with_count(U_FILE* outfile,vector_int* list, text_tokens* tokens, counted_match_descriptor* descriptor) {
  int i;

  for (i = 0 ; i < list->nbelems ; i++) {
    if (i == descriptor->leftEndsAt + 1) {
      u_fprintf(outfile, "\t");
    }
    u_fputs(tokens->token[list->tab[i]],outfile);
    if (i == descriptor->rightStartsAt - 1) {
      u_fprintf(outfile, "\t");
    }
  }
  u_fprintf(outfile,"\t%d", descriptor->countOfMatch);
  u_fprintf(outfile,"\n");
}

/**
 * This function fills a buffer of token IDs starting from a particular token to the left and/or right.
 */
int get_buffer_around_token(U_FILE* inputFile, int** buffer, long tokenPosition, long leftSize, long rightSize, long* buffer_start_from, long* buffer_ends_at) {
  // free old buffer first
  if (*buffer != NULL) {
    free(*buffer);
    *buffer_start_from = -1;
    *buffer_ends_at = -1;
  }

  long fileSize = get_file_size(inputFile);
  long seekTo = max_long(0, tokenPosition - leftSize);
  long endReadingAt = min_long(fileSize/sizeof(int) - 1, tokenPosition + rightSize);

  int bufferSize = (int)(endReadingAt - seekTo + 1);

  *buffer = (int*)malloc(sizeof(int) * bufferSize);

  if (*buffer == NULL) {
    alloc_error("get_buffer_around_token");
    return ALLOC_ERROR_CODE;
  }

  fseek(inputFile, seekTo * sizeof(int), SEEK_SET);

  long numRead = (long)fread(*buffer, sizeof(int), bufferSize, inputFile);

  if (numRead != bufferSize) {
    error("Error reading file in get_buffer_around_token!");
    return DEFAULT_ERROR_CODE;
  }

  *buffer_start_from = seekTo;
  *buffer_ends_at = endReadingAt;

  return SUCCESS_RETURN_CODE;
}

int vectors_equal(const void* v1, const void* v2) {
  int cs = ((const vec_CS_tag*)v1)->CStag;
  text_tokens* tokens = ((vec_CS_tag*)v1)->tokens;
  const Alphabet* a = ((vec_CS_tag*)v1)->alphabet;

  const vector_int* vec1 = ((const vec_CS_tag*)v1)->vec;
  const vector_int* vec2 = ((const vec_CS_tag*)v2)->vec;

  if (vec1->nbelems != vec2->nbelems) {
    return 0;
  }

  int i;

  for (i = 0 ; i < vec1->nbelems ; i++) {
    if (vec1->tab[i] != vec2->tab[i]) {
      if (!cs) {
        if (!tokens_equal_ignore_case(tokens->token[vec1->tab[i]], tokens->token[vec2->tab[i]], a)) {
          return 0;
        }
      } else {
        return 0;
      }
    }
  }
  return 1;
}


void free_vec(void* vec) {
  free_vec_CS_tag((vec_CS_tag*)vec);
}

void* copy_vec(const void* vec) {
  const vector_int* const input = ((const vec_CS_tag*)vec)->vec;
  vector_int* output = new_vector_int();
  vec_CS_tag* ret    = new_vec_CS_tag(output, ((const vec_CS_tag*)vec)->CStag, ((const vec_CS_tag*)vec)->tokens, ((const vec_CS_tag*)vec)->alphabet);

  int i;

  for (i = 0 ; i < input->nbelems ; i++) {
    vector_int_add(output, input->tab[i]);
  }

  return ret;
}

void sort_matches_ptr(void *array, int left, int right, void (*swap)(void*, int, int), int (*compare)(void*, int, int, void*, void*), void* objectForCompare, void* objectForCompare2, int direction) {
  if (left >= right) {
    return;
  }

  int partitionIndex = partition_ptr(array, left, right, swap, compare, objectForCompare, objectForCompare2, direction);
  if (partitionIndex != -1) {
    sort_matches_ptr(array, partitionIndex+1, right, swap, compare, objectForCompare, objectForCompare2, direction);
    sort_matches_ptr(array, left, partitionIndex-1, swap, compare, objectForCompare, objectForCompare2, direction);
  }
}

int partition_ptr(void *array, int left, int right, void (*swap)(void*, int, int), int (*compare)(void*, int, int, void*, void*), void* objectForCompare, void* objectForCompare2, int direction) {
  swap(array, left, right);

  // frequently, all counts in partition are equal (lots of items with count 1)
  // we can speed things up dramatically if we check, in one pass, that not all counts are equal

  int allEqual = 1; int i;

  for (i = left ; i < right ; i++) {
    if (compare(array, i, right, objectForCompare, objectForCompare2) != 0) {
      allEqual = 0;
      break;
    }
  }

  if (allEqual) {
    return -1;
  }

  int storeIndex = left;

  for (i = left ; i < right ; i++) {
    if (direction * compare(array, i, right, objectForCompare, objectForCompare2) <= 0) {
      swap(array, i, storeIndex);
      storeIndex++;
    }
  }
  swap(array, storeIndex, right);

  return storeIndex;
}

void swap_ptr(void *array, int idx1, int idx2) {
  void **real_arr = (void**)array;
  void *tmp = real_arr[idx1];
  real_arr[idx1] = real_arr[idx2];
  real_arr[idx2] = tmp;
}

int compare_ptr(void *array, int idx1, int idx2, void* hash, void* /*unused*/) {
  void **real_arr = (void**)array;
  hash_table* hsh = (hash_table*)hash;
  any* hash_val1, *hash_val2;

  counted_match_descriptor* desc1, *desc2;

  hash_val1 = get_value(hsh, real_arr[idx1], HT_DONT_INSERT);
  hash_val2 = get_value(hsh, real_arr[idx2], HT_DONT_INSERT);

  desc1 = (counted_match_descriptor*)hash_val1->_ptr;
  desc2 = (counted_match_descriptor*)hash_val2->_ptr;

  if (desc1->countOfMatch < desc2->countOfMatch){
    return -1;
  } else if (desc1->countOfMatch > desc2->countOfMatch) {
    return 1;
  } else {
    return 0;
  }
}

void swap_int(void *array, int idx1, int idx2) {
  int* real_arr = (int*)array;
  int tmp = real_arr[idx1];
  real_arr[idx1] = real_arr[idx2];
  real_arr[idx2] = tmp;
}

int compare_int(void *array, int idx1, int idx2, void *hash, void* int_CS_sample) {
  int *real_arr = (int*)array;
  hash_table* hsh = (hash_table*)hash;
  any* hash_val1, *hash_val2;
  int_CS_tag* tag = (int_CS_tag*)int_CS_sample;

  tag->tokenID = real_arr[idx1];
  hash_val1 = get_value(hsh, tag, HT_DONT_INSERT);
  tag->tokenID = real_arr[idx2];
  hash_val2 = get_value(hsh, tag, HT_DONT_INSERT);

  if (hash_val1->_int < hash_val2->_int) {
    return -1;
  } else if (hash_val1->_int > hash_val2->_int) {
    return 1;
  } else {
    return 0;
  }
}

int compare_double(void *array, int idx1, int idx2, void *hash, void* int_CS_sample) {
  int *real_arr = (int*)array;
  hash_table* hsh = (hash_table*)hash;
  any* hash_val1, *hash_val2;
  int_CS_tag* tag = (int_CS_tag*)int_CS_sample;

  tag->tokenID = real_arr[idx1];
  hash_val1 = get_value(hsh, tag, HT_DONT_INSERT);
  tag->tokenID = real_arr[idx2];
  hash_val2 = get_value(hsh, tag, HT_DONT_INSERT);

  if (*((double*)hash_val1->_ptr) < *((double*)hash_val2->_ptr)) {
    return -1;
  } else if (*((double*)hash_val1->_ptr) > *((double*)hash_val2->_ptr)) {
    return 1;
  } else {
    return 0;
  }
}

int is_appropriate_token(int tokenID, text_tokens* tokens) {
  if (tokenID != tokens->SPACE && tokenID != tokens->SENTENCE_MARKER && tokenID != tokens->STOP_MARKER) {
    return 1;
  } else {
    return 0;
  }
}

vec_CS_tag* new_vec_CS_tag(vector_int* vec, int CS, text_tokens* tokens, Alphabet* alphabet) {
  vec_CS_tag* ret = (vec_CS_tag*)malloc(sizeof(vec_CS_tag));
  if (ret == NULL) {
    alloc_error("new_vec_CS_tag");
    return NULL;
  }

  ret->CStag    = CS;
  ret->vec      = vec;
  ret->tokens   = tokens;
  ret->alphabet = alphabet;

  return ret;
}


int_CS_tag* new_int_CS_tag(int value, int CS, text_tokens* tokens, Alphabet* alphabet) {
  int_CS_tag* ret = (int_CS_tag*)malloc(sizeof(int_CS_tag));

  if (ret == NULL) {
    alloc_error("new_int_CS_tag");
    return NULL;
  }

  ret->CStag = CS;
  ret->tokenID = value;
  ret->tokens = tokens;
  ret->alphabet = alphabet;

  return ret;
}

void free_vec_CS_tag(vec_CS_tag* vec) {
  free_vector_int(vec->vec);
  free(vec);
}

void free_int_CS_tag(int_CS_tag* inttag) {
  free(inttag);
}

int tokens_equal_ignore_case(const unichar* s1, const unichar* s2, const Alphabet* a) {
  int i = 0;

  while(s1[i] && is_equal_ignore_case(s1[i], s2[i], a)) {
    i++;
  }

  return (s1[i] == '\0' && s2[i] == '\0');

}

unichar alphabet_to_upper(unichar c, Alphabet* alphabet) {
  const unichar* t = NULL;
  int i_pos_in_array_of_string = alphabet->pos_in_represent_list[c];
  t = (i_pos_in_array_of_string == 0) ? NULL :
      (alphabet->t_array_collection[i_pos_in_array_of_string]);
  return (t == NULL) ? c : (((*t)=='\0') ? c : (*t));
}

unsigned int hash_token_as_int(const void* t) {
  const int_CS_tag* token = (const int_CS_tag*)t;

  if (!(token->CStag)) {
    return jenkins_one_at_a_time_hash_string_uppercase(token->tokens->token[token->tokenID], u_strlen(token->tokens->token[token->tokenID]), token->alphabet);
  }

  return token->tokenID;
}

int tokens_as_int_equal(const void* t1, const void* t2) {
  const int_CS_tag* T1 = (int_CS_tag*)t1;
  const int_CS_tag* T2 = (int_CS_tag*)t2;

  if (!(T1->CStag)) {
    if (tokens_equal_ignore_case(T1->tokens->token[T1->tokenID], T2->tokens->token[T2->tokenID], T1->alphabet)) {
      return 1;
    } else {
      return 0;
    }
  } else {
    return T1->tokenID == T2->tokenID;
  }
}

void* copy_token_as_int(const void* t) {
  const int_CS_tag* source = (const int_CS_tag*)t;
  int_CS_tag* ret = new_int_CS_tag(source->tokenID, source->CStag, source->tokens, source->alphabet);
  return ret;
}

void free_token_as_int(void* t) {
  free_int_CS_tag((int_CS_tag*)t);
}

} // namespace unitex
