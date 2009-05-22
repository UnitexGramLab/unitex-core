 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
#include "Matches.h"
#include "HashTable.h"
#include "Vector.h"
#include "Alphabet.h"
#include "getopt.h"

#include "Stats.h"

// main work functions

void concord_stats(const char* , int , const char *, const char* , const char* , const char*, int , int, int );
void build_counted_concord(match_list* , text_tokens* , U_FILE* , Alphabet*, int , int , int, vector_ptr** , hash_table** );
void build_counted_collocates(match_list* , text_tokens* , U_FILE* , Alphabet*, int , int , int, vector_int** , hash_table** , hash_table** , hash_table** );

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

// ... structs


// local helper functions
inline int min_int(int, int);
inline int max_int(int, int);
inline long min_long(long, long);
inline long max_long(long, long);
vector_int* get_string_in_context_as_token_list(match_list*, int, int, int**, long*, long*, long, text_tokens*, U_FILE*, int);
void print_string_token_list_with_count(U_FILE*,vector_int*, text_tokens*, int);
void get_buffer_around_token(U_FILE*, int**, long, long, long, long*, long*);
void count_collocates(U_FILE* , text_tokens* , Alphabet*, int, hash_table* , hash_table** , int* );
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
 unsigned int hash_vector_int(void *);
 unsigned int jenkins_one_at_a_time_hash(unsigned char *, size_t);
 unsigned int jenkins_one_at_a_time_hash_string_uppercase(unichar *, size_t, Alphabet* );
 int vectors_equal(void*, void*);
 void free_vec(void*);
 void* copy_vec(void*);

 unsigned int hash_token_as_int(void*);
 int tokens_as_int_equal(void*, void*);
 void* copy_token_as_int(void*);
 void free_token_as_int(void*);

 vec_CS_tag* new_vec_CS_tag(vector_int*, int, text_tokens*, Alphabet*);
 int_CS_tag* new_int_CS_tag(int, int, text_tokens*, Alphabet*);
 void free_vec_CS_tag(vec_CS_tag*);
 void free_int_CS_tag(int_CS_tag*);
 int tokens_equal_ignore_case(unichar*, unichar*, Alphabet*);

 // TODO this should probably be moved to Alphabet.cpp
 unichar alphabet_to_upper(unichar c, Alphabet*alph);
 // ...hash helper functions
// ...local helper functions


#define STATS_BUFFER_LENGTH 4096

static void usage()
{
	u_printf("%S",COPYRIGHT);
	u_printf("Usage: Stats [ARGUMENTS]\n"
			 "\n"
			 "ARGUMENTS:\n"
			"-m/--mode <mode>: specifies mode of operation: 0 - left + match + right count, 1 - collocate count, 2 - collocate count with z-score\n"
			"-c/--concord <concord.ind>: path to concord.ind\n"
			"-t/--tokens <tokens.txt>: path to tokens.txt\n"
			"-x/--text <text.cod>: path to text.cod\n"
			"-a/--alpha <alphabet.txt>: path to alphabet.txt\n"
			"-o/--output <output.txt>: path to output.txt file to create with result\n"
			"-l/--left <length>: length of left context\n"
			"-r/--right <length>: length of right context\n"
			"-s/--case <0/1>: 0 - case insensitive, 1 - case sensitive, default is 1!\n");

	return;
}

int main_Stats(int argc,char *argv[]) {

	if (argc <= 1)
	{
		usage();
		return 0;
	}

	int leftContext = 0,  rightContext = 0, mode=-1, caseSensitive = 1;
	const char *concord = NULL, *text = NULL, *tokens = NULL, *alpha = NULL, *output_file = NULL;
	const char* optstring=":m:c:t:x:a:l:r:s:o:";

	struct option_TS lopts[]= {
	      {"mode",required_argument_TS,NULL,'m'},
	      {"concord",required_argument_TS,NULL,'c'},
	      {"tokens",required_argument_TS,NULL,'t'},
	      {"text",required_argument_TS,NULL,'x'},
	      {"alpha",required_argument_TS,NULL,'a'},
	      {"left",required_argument_TS,NULL,'l'},
	      {"right",required_argument_TS,NULL,'r'},
	      {"case",optional_argument_TS,NULL,'s'},
		  {"output",optional_argument_TS,NULL,'o'},
	      {0, 0, 0, 0 }
	 } ;


	int val,index=-1;
	struct OptVars* vars=new_OptVars();
	while (EOF!=(val=getopt_long_TS(argc,argv,optstring,lopts,&index,vars))) {
	   switch(val) {
	   case 'm': mode = atoi(vars->optarg); break;
	   case 'c': concord = vars->optarg; break;
	   case 't': tokens = vars->optarg; break;
	   case 'x': text = vars->optarg; break;
	   case 'a': alpha = vars->optarg; break;
	   case 'l': leftContext = atoi(vars->optarg); break;
	   case 'r': rightContext = atoi(vars->optarg); break;
	   case 's': caseSensitive = atoi(vars->optarg); break;
	   case 'o': output_file = vars->optarg; break;
	   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
	             else fatal_error("Missing argument for option --%s\n",lopts[index].name);
	   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
	             else fatal_error("Invalid option --%s\n",vars->optarg);
	             break;
	   }
	   index=-1;
	}

	if (concord == NULL)
	{
		fatal_error("Stats: path to concord.ind not specified!");
	}

	if (tokens == NULL)
	{
		fatal_error("Stats: path to tokens.txt not specified!");
	}

	if (text == NULL)
	{
		fatal_error("Stats: path to text.cod not specified!");
	}

	if (alpha == NULL)
	{
		fatal_error("Stats: path to alphabet.txt not specified!");
	}


	if (mode == 0 || mode == 1 || mode == 2)
	{
		concord_stats(output_file, mode, concord, tokens, text, alpha, leftContext, rightContext, caseSensitive);
	}
	else
	{
		fatal_error("Unknown mode!");
	}

	return 0;
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
void concord_stats(const char* outfilename,int mode, const char *concordfname, const char* tokens_path, const char* codname,
				   const char* alphabetName, int leftContext, int rightContext, int caseSensitive)
{
	U_FILE* concord = u_fopen(UTF16_LE, concordfname, U_READ);
	U_FILE* outfile = (outfilename == NULL) ? U_STDOUT : u_fopen(UTF16_LE, outfilename, U_WRITE);
	U_FILE* cod = u_fopen(BINARY, codname, U_READ);
	match_list* matches = load_match_list(concord, NULL);
	u_fclose(concord);


	text_tokens* tokens = load_text_tokens(tokens_path);

	if (tokens == NULL)
	{
		fatal_error("Error in build_counted_concord, tokens cannot be loaded!");
	}

	Alphabet* alphabet = load_alphabet(alphabetName);

	if (alphabet == NULL)
	{
		fatal_error("Error in concord_stats, alphabet cannot be loaded!");
	}


	int i;
	any* hash_val;


	if (mode == 0)
	{
		vector_ptr* allMatches;
		hash_table* countsPerMatch;

		build_counted_concord(matches, tokens, cod, alphabet, leftContext, rightContext, caseSensitive, &allMatches, &countsPerMatch);
		// now we sort

		sort_matches_ptr(allMatches->tab, 0, allMatches->nbelems-1, swap_ptr, compare_ptr, countsPerMatch, NULL, -1);

		// and then print


		for (i = 0; i < allMatches->nbelems ; i++)
		{
			hash_val = get_value(countsPerMatch, allMatches->tab[i], HT_DONT_INSERT);
			print_string_token_list_with_count(outfile, ((vec_CS_tag*)(allMatches->tab[i]))->vec, tokens, hash_val->_int);
		}

		free_vector_ptr(allMatches, NULL);
		free_hash_table(countsPerMatch);
	}
	else if (mode == 1)
	{
		vector_int* allMatches;
		hash_table* countsPerMatch;

		build_counted_collocates(matches, tokens, cod, alphabet, leftContext, rightContext, caseSensitive, &allMatches, &countsPerMatch, NULL, NULL);

		// now we sort

		int_CS_tag* sampleKey = new_int_CS_tag(0, caseSensitive, tokens, alphabet);
		sort_matches_ptr(allMatches->tab, 0, allMatches->nbelems-1, swap_int, compare_int, countsPerMatch, sampleKey, -1);
		free_int_CS_tag(sampleKey);

		// and then print

		int K;

		int_CS_tag* currentKey;

		currentKey = new_int_CS_tag(0, caseSensitive, tokens, alphabet);

		for (i = 0 ; i < allMatches->nbelems ; i++)
		{
			currentKey->tokenID = allMatches->tab[i];
			hash_val = get_value(countsPerMatch, currentKey, HT_DONT_INSERT);

			K = hash_val->_int;
			u_fprintf(outfile,"%S\t%d\n", tokens->token[allMatches->tab[i]], K);
		}

		free_int_CS_tag(currentKey);
		free_vector_int(allMatches);
		free_hash_table(countsPerMatch);
	}
	else if (mode == 2)
	{
		vector_int* allMatches;
		hash_table* countsPerMatch;
		hash_table* z_score;
		hash_table* countInCorpora;

		build_counted_collocates(matches, tokens, cod, alphabet, leftContext, rightContext, caseSensitive, &allMatches, &countsPerMatch, &z_score, &countInCorpora);

		// now we sort

		int_CS_tag* sampleKey = new_int_CS_tag(0, caseSensitive, tokens, alphabet);
		sort_matches_ptr(allMatches->tab, 0, allMatches->nbelems-1, swap_int, compare_double, z_score, sampleKey, -1);
		free_int_CS_tag(sampleKey);

		// and then print

		int Fc, K;
		double zScore;
		int_CS_tag* key = new_int_CS_tag(0, caseSensitive, tokens, alphabet);


		for (i = 0 ; i < allMatches->nbelems ; i++)
		{
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
		for (i = 0 ; i < allMatches->nbelems; i++)
		{
			key->tokenID = allMatches->tab[i];
			hash_val = get_value(z_score, key, HT_DONT_INSERT);
			free(hash_val->_ptr);
		}

		free_int_CS_tag(key);
		free_vector_int(allMatches);
		free_hash_table(countsPerMatch);
		free_hash_table(z_score);
		free_hash_table(countInCorpora);
	}


	u_fclose(cod);
	if (outfilename != NULL)
		u_fclose(outfile);
	free_text_tokens(tokens);
	free_alphabet(alphabet);
	match_list* current_match = matches;
	struct match_list* next_match;

	while(current_match != NULL)
	{
		next_match = current_match->next;
		free_match_list_element(current_match);
		current_match = next_match;
	}
}

/**
 * This function builds all strings that are based on matches found in original text surrounded with
 * left and right context. It outputs a vector and a hash table - vector contains distinct strings
 * found, and these strings are key to the hash table containing count per string in corpora. Strings
 * are represented by integer vector containing token IDs. NOTE: function is case-sensitive, so matches
 * that are different in case are counted separatly. This can easily be corrected using different
 * function instead of vectors_equal.
 */
void build_counted_concord(match_list* matches, text_tokens* tokens, U_FILE* cod, Alphabet* alphabet, int leftContext, int rightContext, int caseSensitive, vector_ptr** ret_vector, hash_table** ret_hash)
{
	if (ret_vector == NULL)
	{
		fatal_error("Fatal error in build_counted_concord, ret_vector cannot be NULL!");
	}

	if (ret_hash == NULL)
	{
		fatal_error("Fatal error in build_counted_concord, ret_hash cannot be NULL!");
	}

	long codSize = get_file_size(cod) / sizeof(int);
	int *buffer = NULL;

	long buff_start, buff_end;

	// we initialize buffer to encompass 4096 tokens from starting position
	get_buffer_around_token(cod, &buffer, 0, 0, STATS_BUFFER_LENGTH, &buff_start, &buff_end);

	any* hash_val;
	int hash_ret;

	vector_ptr* allMatches = new_vector_ptr();
	hash_table* countPerMatch = new_hash_table(hash_vector_int, vectors_equal, free_vec, copy_vec);
	vector_int* currentMatchList;
	vec_CS_tag* currentKey;

	match_list* current_match = matches;

	int i = 0;

	// for all matches, we form list of token IDs and check it against hash table
	while(current_match != NULL)
	{
		currentMatchList = get_string_in_context_as_token_list(current_match, leftContext, rightContext, &buffer, &buff_start, &buff_end, codSize, tokens, cod, 1);

		currentKey = new_vec_CS_tag(currentMatchList, caseSensitive, tokens, alphabet);

		hash_val = get_value(countPerMatch, currentKey, HT_INSERT_IF_NEEDED, &hash_ret);

		if (hash_ret == HT_KEY_ADDED)
		{
			// new value, we need to set count to 1
			hash_val->_int = 1;
			vector_ptr_add(allMatches, currentKey);
		}
		else
		{
			hash_val->_int++;
			// we need to free, since it was not added to the table
			free_vec_CS_tag(currentKey);
		}

		current_match = current_match->next;
		i++;
	}

	free(buffer);

	*ret_vector = allMatches;
	*ret_hash = countPerMatch;
}

/**
 * This function performs collocates count. It has two modes of operation, corresponding to modes 1 and
 * 2 of main program. In mode 1, it only looks at tokens in left and right context and counts them
 * each time they appear in the context of a match. In this mode, it returns an int vector containing
 * all possible tokens found in left and right context of a match, as well as hash table containing
 * counts per tokens in context. In mode 2, it returns additional 2 hash tables, z_score hash table which
 * represents z-score of a collocate and countsInCorpora hash table which returns total count of a token
 * found in context of a match in the whole corpora. NOTE: Collocates are case-sensitive, as they are
 * identified by token IDs - there are ways around this, but they are not as straightforward as it was
 * in case of build_counted_concord.
 */
void build_counted_collocates(match_list* matches, text_tokens* tokens, U_FILE* cod, Alphabet* alphabet, int leftContext, int rightContext, int caseSensitive, vector_int** ret_vector, hash_table** ret_hash, hash_table** z_score, hash_table** countsInCorpora)
{
	if (ret_vector == NULL)
	{
		fatal_error("Fatal error in build_counted_collocates, ret_vector cannot be NULL!");
	}

	if (ret_hash == NULL)
	{
		fatal_error("Fatal error in build_counted_collocates, ret_hash cannot be NULL!");
	}

	long codSize = get_file_size(cod) / sizeof(int);
	int *buffer = NULL;

	long buff_start, buff_end;

	// we initialize buffer to encompass 4096 tokens from starting position
	get_buffer_around_token(cod, &buffer, 0, 0, STATS_BUFFER_LENGTH, &buff_start, &buff_end);

	any* hash_val;
	int hash_ret;

	int i;

	vector_int* allMatches = new_vector_int();
	hash_table* countPerCollocate = new_hash_table(hash_token_as_int, tokens_as_int_equal, free_token_as_int, copy_token_as_int);
	vector_int* currentMatchList;
	vector_int* tmpMatchList;

	match_list* current_match = matches;

	hash_table* collocateCountInCorpora;
	int totalMatches = 0;
	int corporaLength;
	int wordsTakenByMatches = 0;
	int totalWindow = 0;
	int_CS_tag* currentKey;

	// for all matches, we form list of token IDs and check it against hash table
	while(current_match != NULL)
	{
		currentMatchList = get_string_in_context_as_token_list(current_match, leftContext, rightContext, &buffer, &buff_start, &buff_end, codSize, tokens, cod, 0);

		// now we don't just insert the whole match as we did in build_counted_concord, but
		// for each token in the left and right context we treat it as a possible entry to a hash
		// table

		for (i = 0 ; i < currentMatchList->nbelems ; i++)
		{
			// we don't want space, sentence or stop tokens in results

			if (!is_appropriate_token(currentMatchList->tab[i], tokens))
			{
				continue;
			}

			currentKey = new_int_CS_tag(currentMatchList->tab[i], caseSensitive, tokens, alphabet);

			hash_val = get_value(countPerCollocate, currentKey, HT_INSERT_IF_NEEDED, &hash_ret);

			if (hash_ret == HT_KEY_ADDED)
			{
				// new value, we need to set count to 1
				hash_val->_int = 1;
				vector_int_add(allMatches, currentMatchList->tab[i]);
			}
			else
			{
				hash_val->_int++;
				free_int_CS_tag(currentKey);
			}
		}

		// if we're calculating z-score as well, we have to account for totalWindow score
		// which represents total space in non-space tokens taken by matches and their
		// left and right contexts

		if (z_score != NULL && countsInCorpora != NULL)
		{
			// first we account for number of non-space tokens taken by the match itself
			tmpMatchList = get_string_in_context_as_token_list(current_match, 0, 0, &buffer, &buff_start, &buff_end, codSize, tokens, cod, 1);

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

			for (i = 0 ; i < currentMatchList->nbelems; i++)
			{
				if (is_appropriate_token(currentMatchList->tab[i], tokens))
				{
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
	*ret_hash = countPerCollocate;


	// we don't proceed with calculating z-score unless it's required
	if (z_score == NULL || countsInCorpora == NULL)
	{
		return;
	}

	// now we count all collocates in corpus

	count_collocates(cod, tokens, alphabet, caseSensitive, countPerCollocate, &collocateCountInCorpora, &corporaLength);

	// now we build z_score hash per collocate

	hash_table* zret = new_hash_table(hash_token_as_int, tokens_as_int_equal, free_token_as_int, copy_token_as_int);
	double *tmpZScore;
	int K, Fc;
	double p;
	double E;

	for (i = 0 ; i < allMatches->nbelems ; i++)
	{
		currentKey = new_int_CS_tag(allMatches->tab[i], caseSensitive, tokens, alphabet);
		hash_val = get_value(countPerCollocate, currentKey, HT_DONT_INSERT);

		K = hash_val->_int;

		hash_val = get_value(collocateCountInCorpora, currentKey, HT_DONT_INSERT);

		Fc = hash_val->_int;

		tmpZScore = (double*)malloc(sizeof(double*));

		if (tmpZScore == NULL)
		{
			fatal_alloc_error("build_counted_collocates");
		}

		p = (double)Fc / (corporaLength - wordsTakenByMatches);
		E = p * totalWindow;

		*tmpZScore = (K - E) / sqrt(E * (1 - p));

		hash_val = get_value(zret, currentKey, HT_INSERT_IF_NEEDED);
		hash_val->_ptr = tmpZScore;

		// no need to free currentKey, it is added to the hash table!
	}

	*countsInCorpora = collocateCountInCorpora;
	*z_score = zret;

}

/**
 * This is a helper function for build_counted_collocates. It counts all collocates found for a specific
 * match list in corpora and returns the result as a hash table. Additional result - corpora_length, returns
 * total length of corpora in non-space tokens. Non-space tokens are determined by the result
 * of is_appropriate_token function.
 */
void count_collocates(U_FILE* cod, text_tokens* tokens, Alphabet* alphabet, int caseSensitive, hash_table* collocates, hash_table** ret_hash, int* corpora_length)
{
	if (ret_hash == NULL)
	{
		fatal_error("Error in count_collocates, ret_hash cannot be null!");
	}

	hash_table* ret = new_hash_table(hash_token_as_int, tokens_as_int_equal, free_token_as_int, copy_token_as_int);
	int* buffer = NULL;
	long bufferStart, bufferEnd;
	long codSize = get_file_size(cod) / sizeof(int);
	any* hash_val;
	int hash_ret;
	int_CS_tag* currentKey;

	get_buffer_around_token(cod, &buffer, 0, 0, STATS_BUFFER_LENGTH, &bufferStart, &bufferEnd);

	int i ;

	*corpora_length = 0;

	for (i = 0 ; i < codSize ; i++)
	{
		if (i < bufferStart || i > bufferEnd)
		{
			get_buffer_around_token(cod, &buffer, i, 0, STATS_BUFFER_LENGTH, &bufferStart, &bufferEnd);
		}

		if (is_appropriate_token(buffer[i - bufferStart], tokens))
		{
			(*corpora_length)++;
		}

		currentKey = new_int_CS_tag(buffer[i - bufferStart], caseSensitive, tokens, alphabet);

		hash_val = get_value(collocates, currentKey, HT_DONT_INSERT, &hash_ret);

		if (hash_ret == HT_KEY_ALREADY_THERE)
		{
			// this means that we can count this collocate in another hash

			int_CS_tag* insertKey = new_int_CS_tag(buffer[i - bufferStart], caseSensitive, tokens, alphabet);

			hash_val = get_value(ret, insertKey, HT_INSERT_IF_NEEDED, &hash_ret);

			if (hash_ret == HT_KEY_ADDED)
			{
				hash_val->_int = 1;
			}
			else
			{
				hash_val->_int++;
				free_int_CS_tag(insertKey);
			}
		}

		free_int_CS_tag(currentKey);
	}

	*ret_hash = ret;

	free(buffer);
}


inline int min_int(int a, int b)
{
	return (a < b) ? a : b;
}

inline int max_int(int a, int b)
{
	return (a > b) ? a : b;
}

inline long min_long(long a, long b)
{
	return (a < b) ? a : b;
}

inline long max_long(long a, long b)
{
	return (a > b) ? a : b;
}

/**
 * This is a helper function which detects left and right context of a particular match and returns it
 * as an int vector. It expands match to the left and right by number of "appropriate" non-space tokens.
 * These tokens are determined by is_appropriate_token function.
 */
vector_int* get_string_in_context_as_token_list(match_list* match, int leftContext, int rightContext, int** buffer, long* bufferStart, long* bufferEnd, long totalSize, text_tokens* tokens, U_FILE* source, int includeMatch)
{
	long i;

	if (match == NULL)
	{
		fatal_error("Error in get_string_as_token_list, match cannot be NULL!");
		return NULL;
	}

	if (buffer == NULL)
	{
		fatal_error("Error in get_string_as_token_list, buffer cannot be NULL");
		return NULL;
	}

	if (*bufferStart > *bufferEnd)
	{
		fatal_error("Error in get_string_as_token_list, bufferStart must be <= bufferEnd!");
		return NULL;
	}

	vector_int* res = new_vector_int();

	long startFrom = match->start - 1;
	long endAt = match->end + 1;
	int foundLeft = 0;
	int foundRight = 0;

	// first we skip sentence and space tokens to the left and right until we have
	// enough left and right context to work with

	while(startFrom >= 0 && foundLeft < leftContext)
	{
		if (startFrom < *bufferStart || startFrom > *bufferEnd)
		{
			// request new buffer
			get_buffer_around_token(source, buffer, startFrom, STATS_BUFFER_LENGTH, 0, bufferStart, bufferEnd);
		}

		if (is_appropriate_token((*buffer)[startFrom - *bufferStart], tokens))
		{
			foundLeft++;
		}
		startFrom--;
	}
	while(endAt <= totalSize-1 && foundRight < rightContext)
	{
		if (endAt > *bufferEnd || endAt < *bufferStart)
		{
			// request new buffer
			get_buffer_around_token(source, buffer, endAt, 0, STATS_BUFFER_LENGTH, bufferStart, bufferEnd);
		}

		if (is_appropriate_token((*buffer)[endAt - *bufferStart], tokens))
		{
			foundRight++;
		}
		endAt++;
	}

	for (i = startFrom + 1 ; i <= endAt - 1 ; i++)
	{
		if (!includeMatch && i >= match->start && i <= match->end)
		{
			// we don't want the match, just the context
			continue;
		}

		if (i < *bufferStart || i > *bufferEnd)
		{
			get_buffer_around_token(source, buffer, i, 0, STATS_BUFFER_LENGTH, bufferStart, bufferEnd);
		}

		vector_int_add(res, (*buffer)[i - *bufferStart]);
	}
	return res;
}

unsigned int hash_vector_int(void* vec)
{
	vec_CS_tag* v = (vec_CS_tag*)vec;
	vector_int* vector = v->vec;
	if (!v->CStag)
	{
		return jenkins_one_at_a_time_hash((unsigned char*)&vector->tab[0], sizeof(int) * vector->nbelems);
	}
	else
	{
		// since we're in case-sensitive mode, we have to look at the contents of a token

		int i = 0;
		int hash = 0;

		for (i = 0 ; i < vector->nbelems ; i++)
		{
			hash ^= jenkins_one_at_a_time_hash_string_uppercase(v->tokens->token[vector->tab[i]], u_strlen(v->tokens->token[vector->tab[i]]), v->alphabet);
		}

		return hash;
	}
}

unsigned int jenkins_one_at_a_time_hash(unsigned char *key, size_t key_len)
{
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

unsigned int jenkins_one_at_a_time_hash_string_uppercase(unichar *key, size_t key_len, Alphabet* alphabet)
{
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


void print_string_token_list_with_count(U_FILE* outfile,vector_int* list, text_tokens* tokens, int count)
{
	int i;

	for (i = 0 ; i < list->nbelems ; i++)
	{
		u_fprintf(outfile,"%S", tokens->token[list->tab[i]]);
	}
	u_fprintf(outfile,"\t%d", count);
	u_fprintf(outfile,"\n");
}

/**
 * This function fills a buffer of token IDs starting from a particular token to the left and/or right.
 */
void get_buffer_around_token(U_FILE* inputFile, int** buffer, long tokenPosition, long leftSize, long rightSize, long* buffer_start_from, long* buffer_ends_at)
{
	// free old buffer first
	if (*buffer != NULL)
	{
		free(*buffer);
		*buffer_start_from = -1;
		*buffer_ends_at = -1;
	}

	long fileSize = get_file_size(inputFile);
	long seekTo = max_long(0, tokenPosition - leftSize);
	long endReadingAt = min_long(fileSize/sizeof(int) - 1, tokenPosition + rightSize);

	int bufferSize = endReadingAt - seekTo + 1;

	*buffer = (int*)malloc(sizeof(int) * bufferSize);

	if (*buffer == NULL)
	{
		fatal_alloc_error("get_buffer_around_token");
	}

	fseek(inputFile, seekTo * sizeof(int), SEEK_SET);

	long numRead = (long)fread(*buffer, sizeof(int), bufferSize, inputFile);

	if (numRead != bufferSize)
	{
		fatal_error("Error reading file in get_buffer_around_token!");
	}

	*buffer_start_from = seekTo;
	*buffer_ends_at = endReadingAt;
}

int vectors_equal(void* v1, void* v2)
{
	int cs = ((vec_CS_tag*)v1)->CStag;
	text_tokens* tokens = ((vec_CS_tag*)v1)->tokens;
	Alphabet* a = ((vec_CS_tag*)v1)->alphabet;

	vector_int* vec1 = ((vec_CS_tag*)v1)->vec;
	vector_int* vec2 = ((vec_CS_tag*)v2)->vec;

	if (vec1->nbelems != vec2->nbelems)
	{
		return 0;
	}

	int i;

	for (i = 0 ; i < vec1->nbelems ; i++)
	{
		if (vec1->tab[i] != vec2->tab[i])
		{
			if (!cs)
			{
				if (!tokens_equal_ignore_case(tokens->token[vec1->tab[i]], tokens->token[vec2->tab[i]], a))
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}
	}
	return 1;
}


void free_vec(void* vec)
{
	free_vec_CS_tag((vec_CS_tag*)vec);
}

void* copy_vec(void* vec)
{
	vector_int* input = ((vec_CS_tag*)vec)->vec;
	vector_int* output = new_vector_int();
	vec_CS_tag* ret = new_vec_CS_tag(output, ((vec_CS_tag*)vec)->CStag, ((vec_CS_tag*)vec)->tokens, ((vec_CS_tag*)vec)->alphabet);

	int i;

	for (i = 0 ; i < input->nbelems ; i++)
	{
		vector_int_add(output, input->tab[i]);
	}

	return ret;
}

void sort_matches_ptr(void *array, int left, int right, void (*swap)(void*, int, int), int (*compare)(void*, int, int, void*, void*), void* objectForCompare, void* objectForCompare2, int direction)
{
	if (left >= right)
		return;

	int partitionIndex = partition_ptr(array, left, right, swap, compare, objectForCompare, objectForCompare2, direction);
	if (partitionIndex != -1)
	{
		sort_matches_ptr(array, partitionIndex+1, right, swap, compare, objectForCompare, objectForCompare2, direction);
		sort_matches_ptr(array, left, partitionIndex-1, swap, compare, objectForCompare, objectForCompare2, direction);
	}
}

int partition_ptr(void *array, int left, int right, void (*swap)(void*, int, int), int (*compare)(void*, int, int, void*, void*), void* objectForCompare, void* objectForCompare2, int direction)
{
	swap(array, left, right);

	// frequently, all counts in partition are equal (lots of items with count 1)
	// we can speed things up dramatically if we check, in one pass, that not all counts are equal

	int allEqual = 1; int i;

	for (i = left ; i < right ; i++)
	{
		if (compare(array, i, right, objectForCompare, objectForCompare2) != 0)
		{
			allEqual = 0;
			break;
		}
	}

	if (allEqual)
	{
		return -1;
	}

	int storeIndex = left;

	for (i = left ; i < right ; i++)
	{
		if (direction * compare(array, i, right, objectForCompare, objectForCompare2) <= 0)
		{
			swap(array, i, storeIndex);
			storeIndex++;
		}
	}
	swap(array, storeIndex, right);

	return storeIndex;
}

void swap_ptr(void *array, int idx1, int idx2)
{
	void **real_arr = (void**)array;
	void *tmp = real_arr[idx1];
	real_arr[idx1] = real_arr[idx2];
	real_arr[idx2] = tmp;
}

int compare_ptr(void *array, int idx1, int idx2, void* hash, void* /*unused*/)
{
	void **real_arr = (void**)array;
	hash_table* hsh = (hash_table*)hash;
	any* hash_val1, *hash_val2;

	hash_val1 = get_value(hsh, real_arr[idx1], HT_DONT_INSERT);
	hash_val2 = get_value(hsh, real_arr[idx2], HT_DONT_INSERT);

	if (hash_val1->_int < hash_val2->_int)
		return -1;
	else if (hash_val1->_int > hash_val2->_int)
		return 1;
	else
		return 0;
}

void swap_int(void *array, int idx1, int idx2)
{
	int* real_arr = (int*)array;
	int tmp = real_arr[idx1];
	real_arr[idx1] = real_arr[idx2];
	real_arr[idx2] = tmp;
}

int compare_int(void *array, int idx1, int idx2, void *hash, void* int_CS_sample)
{
	int *real_arr = (int*)array;
	hash_table* hsh = (hash_table*)hash;
	any* hash_val1, *hash_val2;
	int_CS_tag* tag = (int_CS_tag*)int_CS_sample;

	tag->tokenID = real_arr[idx1];
	hash_val1 = get_value(hsh, tag, HT_DONT_INSERT);
	tag->tokenID = real_arr[idx2];
	hash_val2 = get_value(hsh, tag, HT_DONT_INSERT);

	if (hash_val1->_int < hash_val2->_int)
		return -1;
	else if (hash_val1->_int > hash_val2->_int)
		return 1;
	else
		return 0;
}

int compare_double(void *array, int idx1, int idx2, void *hash, void* int_CS_sample)
{
	int *real_arr = (int*)array;
	hash_table* hsh = (hash_table*)hash;
	any* hash_val1, *hash_val2;
	int_CS_tag* tag = (int_CS_tag*)int_CS_sample;

	tag->tokenID = real_arr[idx1];
	hash_val1 = get_value(hsh, tag, HT_DONT_INSERT);
	tag->tokenID = real_arr[idx2];
	hash_val2 = get_value(hsh, tag, HT_DONT_INSERT);

	if (*((double*)hash_val1->_ptr) < *((double*)hash_val2->_ptr))
		return -1;
	else if (*((double*)hash_val1->_ptr) > *((double*)hash_val2->_ptr))
		return 1;
	else
		return 0;
}

int is_appropriate_token(int tokenID, text_tokens* tokens)
{
	if (tokenID != tokens->SPACE && tokenID != tokens->SENTENCE_MARKER && tokenID != tokens->STOP_MARKER)
		return 1;
	else
		return 0;
}

vec_CS_tag* new_vec_CS_tag(vector_int* vec, int CS, text_tokens* tokens, Alphabet* alphabet)
{
	vec_CS_tag* ret = (vec_CS_tag*)malloc(sizeof(vec_CS_tag));
	if (ret == NULL)
	{
		fatal_alloc_error("new_vec_CS_tag");
	}

	ret->CStag = CS;
	ret->vec = vec;
	ret->tokens = tokens;
	ret->alphabet = alphabet;

	return ret;
}


int_CS_tag* new_int_CS_tag(int value, int CS, text_tokens* tokens, Alphabet* alphabet)
{
	int_CS_tag* ret = (int_CS_tag*)malloc(sizeof(int_CS_tag));

	if (ret == NULL)
	{
		fatal_alloc_error("new_int_CS_tag");
	}

	ret->CStag = CS;
	ret->tokenID = value;
	ret->tokens = tokens;
	ret->alphabet = alphabet;

	return ret;
}

void free_vec_CS_tag(vec_CS_tag* vec)
{
	free_vector_int(vec->vec);
	free(vec);
}

void free_int_CS_tag(int_CS_tag* inttag)
{
	free(inttag);
}

int tokens_equal_ignore_case(unichar* s1, unichar* s2, Alphabet* a)
{
	int i = 0;

	while(s1[i] && is_equal_ignore_case(s1[i], s2[i], a))
		i++;

	return (s1[i] == '\0' && s2[i] == '\0');

}

unichar alphabet_to_upper(unichar c, Alphabet* alphabet)
{
	if (alphabet->t[c]==NULL) return c;
	if (alphabet->t[c][0] == '\0')
		return c;
	else
		return alphabet->t[c][0];
}

unsigned int hash_token_as_int(void* t)
{
	int_CS_tag* token = (int_CS_tag*)t;

	if (!(token->CStag))
	{
		return jenkins_one_at_a_time_hash_string_uppercase(token->tokens->token[token->tokenID], u_strlen(token->tokens->token[token->tokenID]), token->alphabet);
	}
	else
	{
		return token->tokenID;
	}
}

int tokens_as_int_equal(void* t1, void* t2)
{
	int_CS_tag* T1 = (int_CS_tag*)t1;
	int_CS_tag* T2 = (int_CS_tag*)t2;

	if (!(T1->CStag))
	{
		if (tokens_equal_ignore_case(T1->tokens->token[T1->tokenID], T2->tokens->token[T2->tokenID], T1->alphabet))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return T1->tokenID == T2->tokenID;
	}
}

void* copy_token_as_int(void* t)
{
	int_CS_tag* source = (int_CS_tag*)t;
	int_CS_tag* ret = new_int_CS_tag(source->tokenID, source->CStag, source->tokens, source->alphabet);
	return ret;
}

void free_token_as_int(void* t)
{
	free_int_CS_tag((int_CS_tag*)t);
}
