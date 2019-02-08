/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "DutchCompounds.h"
#include "Error.h"
#include "List_ustring.h"
#include "Ustring.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {


/* this define the maximum nomber of item in dutch_word_decomposition_list before abort an exploration */
#ifndef MAX_NB_WORD_DECOMPOSITION_LIST_POSSIBLE
#define MAX_NB_WORD_DECOMPOSITION_LIST_POSSIBLE (10000)
#endif


/* define LARGE_STACK_USAGE_DUTCH_COMPOUNDS if you have a big stack option set on linker (a bit faster) */

#ifndef STANDARD_STRING_BUFFER_SIZE
 #ifdef LARGE_STACK_USAGE_DUTCH_COMPOUNDS
  #define STANDARD_STRING_BUFFER_SIZE   256
 #else
  #define STANDARD_STRING_BUFFER_SIZE   32
 #endif
#endif



#ifndef DEC_TEMP_STRING_BUFFER_SIZE
#ifdef LARGE_STACK_USAGE_DUTCH_COMPOUNDS
#define DEC_TEMP_STRING_BUFFER_SIZE     1024
#else
#define DEC_TEMP_STRING_BUFFER_SIZE     4 // we will always does malloc
#endif
#endif


#define is_N 0
#define is_A 1
#define is_VP1s 2
#define is_ADV 3
#define INVALID_LEFT_COMPONENT 4


    /**
    * This structure is used to englobe settings for the analysis of
    * Dutch unknown words.
    */
    struct dutch_infos {
        /* The Dutch alphabet */
        const Alphabet* alphabet;
        Dictionary* d;
        /* The file where to read the words to analyze from */
        U_FILE* unknown_word_list;
        /* The file where new dictionary lines will be written */
        U_FILE* output;
        /* The file where to print information about the analysis */
        U_FILE* info_output;
        /* The file where to write words that cannot be analyzed as
        * compound words */
        U_FILE* new_unknown_word_list;
        /* The words that cannot appear in a decomposition, like single letters */
        struct string_hash* forbidden_words;
        /* These arrays indicates for each INF code if it can be
        * viewed as a valid code for a left/right component of a
        * compound word. */
        char* valid_left_component;
        char* valid_right_component;
    };


    struct dutch_word_decomposition {
        int n_parts;
        const unichar* decomposition;
        const unichar* dela_line;
    };


    struct dutch_word_decomposition_list {
        struct dutch_word_decomposition* element;
        struct dutch_word_decomposition_list* next;
    };

    // must be at last 4096
    #define PERMANENT_STRING_BUFFER_SIZE            8192
    #define PERMANENT_DEC_STRING_BUFFER_SIZE        PERMANENT_STRING_BUFFER_SIZE
    #define PERMANENT_STRING_TOKENIZE_BUFFER_SIZE   PERMANENT_STRING_BUFFER_SIZE

    struct original_tokenize_DELAF_line_buffer {
        unichar buffer[PERMANENT_STRING_TOKENIZE_BUFFER_SIZE];
    };

    struct analyse_dutch_permanent {
        unichar original_dec_buffer[PERMANENT_DEC_STRING_BUFFER_SIZE];
        unichar original_siacode_buffer[PERMANENT_STRING_BUFFER_SIZE];
        unichar original_new_dela_line_buffer[PERMANENT_STRING_BUFFER_SIZE];
        struct original_tokenize_DELAF_line_buffer tok_buffer;

        unichar decomposition[PERMANENT_STRING_BUFFER_SIZE];
        unichar dela_line[PERMANENT_STRING_BUFFER_SIZE];
        unichar correct_word[PERMANENT_STRING_BUFFER_SIZE];
        long nb_word_decomposition_list_possible;
    };


    static void analyse_dutch_unknown_words(struct dutch_infos*);
    static int analyse_dutch_word(const unichar* word, struct dutch_infos*);
    static void explore_state_dutch(int offset, unichar* current_component, int pos_in_current_component,
        const unichar* word_to_analyze, int pos_in_word_to_analyze, const unichar* analysis,
        const unichar* output_dela_line, struct dutch_word_decomposition_list** L, struct analyse_dutch_permanent * adp,
        int number_of_components, const struct dutch_infos* infos, Ustring*, int base);
    static void check_valid_right_component_dutch(char*, const struct INF_codes*, struct original_tokenize_DELAF_line_buffer*);
    static char check_valid_right_component_for_an_INF_line_dutch(const struct list_ustring*, struct original_tokenize_DELAF_line_buffer*);
    static char check_valid_right_component_for_one_INF_code_dutch(const unichar*, struct original_tokenize_DELAF_line_buffer*);
    static void check_valid_left_component_dutch(char*, const struct INF_codes*, struct original_tokenize_DELAF_line_buffer*);
    static char check_valid_left_component_for_an_INF_line_dutch(const struct list_ustring*, struct original_tokenize_DELAF_line_buffer*);
    static char check_valid_left_component_for_one_INF_code_dutch(const unichar*, struct original_tokenize_DELAF_line_buffer*);

    static struct dutch_word_decomposition* new_word_decomposition_dutch(int n_parts, const unichar* decomposition, const unichar* dela_line);
    static void free_word_decomposition_dutch(struct dutch_word_decomposition*);
    static struct dutch_word_decomposition_list* new_word_decomposition_list_dutch();
    static void free_word_decomposition_list_dutch(struct dutch_word_decomposition_list*);


    /**
    * This function analyzes a list of unknown Dutch words.
    */
    void analyse_dutch_unknown_words(const Alphabet* alphabet, Dictionary* d,
        U_FILE* unknown_word_list, U_FILE* output, U_FILE* info_output,
        U_FILE* new_unknown_word_list, struct string_hash* forbidden_words) {
        /* We create a structure that will contain all settings */
        struct dutch_infos infos;
        infos.alphabet = alphabet;
        infos.d = d;
        infos.unknown_word_list = unknown_word_list;
        infos.forbidden_words = forbidden_words;
        infos.output = output;
        infos.info_output = info_output;
        infos.new_unknown_word_list = new_unknown_word_list;
        infos.valid_left_component = (char*)malloc(sizeof(char)*(d->inf->N));
        if (infos.valid_left_component == NULL) {
            fatal_alloc_error("analyse_dutch_unknown_words");
        }
        infos.valid_right_component = (char*)malloc(sizeof(char)*(d->inf->N));
        if (infos.valid_right_component == NULL) {
            fatal_alloc_error("analyse_dutch_unknown_words");
        }
        /* We look for all INF codes if they correspond to valid left/right
        * components of compounds words. */

        struct original_tokenize_DELAF_line_buffer* tokenize_buffer = (struct original_tokenize_DELAF_line_buffer*)malloc(sizeof(struct original_tokenize_DELAF_line_buffer));
        check_valid_left_component_dutch(infos.valid_left_component, d->inf, tokenize_buffer);
        check_valid_right_component_dutch(infos.valid_right_component, d->inf, tokenize_buffer);
        free(tokenize_buffer);
        /* Now we are ready to analyse the given word list */
        analyse_dutch_unknown_words(&infos);
        free(infos.valid_left_component);
        free(infos.valid_right_component);
    }


    /**
    * This function checks all the INF codes of 'inf' and sets 'valid_left_component[i]'
    * to 1 if the i-th INF line contains at least one INF code that contains
    * the "N" grammatical code.
    */
    void check_valid_right_component_dutch(char* valid_left_component, const struct INF_codes* inf, struct original_tokenize_DELAF_line_buffer* tokenize_buffer) {
        u_printf("Check valid right components...\n");
        for (int i = 0; i<inf->N; i++) {
            valid_left_component[i] = check_valid_right_component_for_an_INF_line_dutch(inf->codes[i], tokenize_buffer);
        }
    }


    /**
    * Returns 1 if at least one of the INF codes of 'INF_codes' is a valid
    * right component, 0 otherwise.
    */
    char check_valid_right_component_for_an_INF_line_dutch(const struct list_ustring* INF_codes, struct original_tokenize_DELAF_line_buffer* tokenize_buffer) {
        while (INF_codes != NULL) {
            if (check_valid_right_component_for_one_INF_code_dutch(INF_codes->string, tokenize_buffer)) {
                return 1;
            }
            INF_codes = INF_codes->next;
        }
        return 0;
    }


    /**
    * Returns 1 if the given dictionary entry is a "N" one.
    */
    char check_N_dutch(struct dela_entry* d) {
        unichar t1[2];
        u_strcpy(t1, "N");
        return (char)dic_entry_contain_gram_code(d, t1);
    }



    /**
    * Returns 1 if the given dictionary entry is a "A" one.
    */
    char check_A_dutch(struct dela_entry* d) {
        unichar t1[2];
        u_strcpy(t1, "A");
        return (char)dic_entry_contain_gram_code(d, t1);
    }


    /**
    * Returns 1 if the given dictionary entry is a "V:P1s" one.
    */
    char check_VP1s_dutch(struct dela_entry* d) {
        unichar t1[2];
        u_strcpy(t1, "V");
        unichar t2[4];
        u_strcpy(t2, "P1s");
        return dic_entry_contain_gram_code(d, t1) && dic_entry_contain_inflectional_code(d, t2);
    }


    /**
    * Returns 1 if the given dictionary entry is a "ADV" one.
    */
    char check_ADV_dutch(struct dela_entry* d) {
        unichar t1[4];
        u_strcpy(t1, "ADV");
        return (char)dic_entry_contain_gram_code(d, t1);
    }



    /**
    * This function checks all the INF codes of 'inf' and sets 'valid_right_component[i]'
    * to 1 if the i-th INF line contains at least one INF code that
    * one of the following codes: "N:sia", "A:sio", "V:W" or "ADV".
    */
    void check_valid_left_component_dutch(char* valid_right_component, const struct INF_codes* inf, struct original_tokenize_DELAF_line_buffer* tokenize_buffer) {
        u_printf("Check valid left components...\n");
        for (int i = 0; i<inf->N; i++) {
            valid_right_component[i] = check_valid_left_component_for_an_INF_line_dutch(inf->codes[i], tokenize_buffer);
        }
    }


    /**
    * Returns 1 if at least one of the INF codes of 'INF_codes' is a valid
    * left component, 0 otherwise.
    */
    char check_valid_left_component_for_an_INF_line_dutch(const struct list_ustring* INF_codes, struct original_tokenize_DELAF_line_buffer* tokenize_buffer) {
        while (INF_codes != NULL) {
            if (check_valid_left_component_for_one_INF_code_dutch(INF_codes->string, tokenize_buffer)) {
                return 1;
            }
            INF_codes = INF_codes->next;
        }
        return 0;
    }


    /**
    * This function analyzes an INF code and returns a value that indicates
    * if it is a valid left component or not.
    */
    int get_valid_left_component_type_for_one_INF_code_dutch(unichar* INF_code, struct original_tokenize_DELAF_line_buffer* tokenize_buffer) {
        /* We produce an artifical dictionary entry with the given INF code,
        * and then, we tokenize it in order to get grammatical and inflectional
        * codes in a structured way. */
        u_strcpy(tokenize_buffer->buffer, "x,");
        u_strcat(tokenize_buffer->buffer, INF_code);
        struct dela_entry* d = tokenize_DELAF_line(tokenize_buffer->buffer, 0);
        int res;
        /* Now we can test if the INF code corresponds to a valid left component */
        if (check_N_dutch(d)) res = is_N;
        else if (check_A_dutch(d)) res = is_A;
        else if (check_VP1s_dutch(d)) res = is_VP1s;
        else if (check_ADV_dutch(d)) res = is_ADV;
        else res = INVALID_LEFT_COMPONENT;
        /* Finally we free the artifical dictionary entry */
        free_dela_entry(d);
        return res;
    }


    /**
    * This function looks in the INF line number 'n' for the first INF code that
    * contains a valid left component.
    * 'code' is a string that will contains the selected code.
    **/
    void get_first_valid_left_component_dutch(struct list_ustring* INF_codes, unichar* code, struct original_tokenize_DELAF_line_buffer* tokenize_buffer) {
        int tmp;
        code[0] = '\0';
        while (INF_codes != NULL) {
            tmp = get_valid_left_component_type_for_one_INF_code_dutch(INF_codes->string, tokenize_buffer);
            if (tmp != INVALID_LEFT_COMPONENT) {
                /* If we find a valid left component, then we return it */
                u_strcpy(code, INF_codes->string);
                return;
            }
            INF_codes = INF_codes->next;
        }
    }


    /**
    * This function looks in the INF line number 'n' for the first INF code that
    * contains a valid left component.
    * 'code' is a string that will contains the selected code.
    **/
    unichar* get_first_valid_left_component_dutch(struct list_ustring* INF_codes, unichar * original_buffer_code, size_t original_buffer_code_size,
        unichar**allocated_buffer_code, struct original_tokenize_DELAF_line_buffer* tokenize_buffer) {
        int tmp;
        while (INF_codes != NULL) {
            tmp = get_valid_left_component_type_for_one_INF_code_dutch(INF_codes->string, tokenize_buffer);
            if (tmp != INVALID_LEFT_COMPONENT) {
                /* If we find a valid left component, then we return it */
                return u_strcpy_optional_buffer(original_buffer_code, original_buffer_code_size,allocated_buffer_code, INF_codes->string);

            }
            INF_codes = INF_codes->next;
        }
        return u_strcpy_optional_buffer(original_buffer_code, original_buffer_code_size, allocated_buffer_code, "");
    }



    /**
    * Returns 1 if the INF code refers to a valid left component, 0 otherwise.
    */
    char check_valid_left_component_for_one_INF_code_dutch(const unichar* INF_code, struct original_tokenize_DELAF_line_buffer* tokenize_buffer) {
        /* We produce an artifical dictionary entry with the given INF code,
        * and then, we tokenize it in order to get grammatical and inflectional
        * codes in a structured way. */
        u_strcpy(tokenize_buffer->buffer, "x,");
        u_strcat(tokenize_buffer->buffer, INF_code);
        struct dela_entry* d = tokenize_DELAF_line(tokenize_buffer->buffer, 0);
        /* Now, we can use this structured representation to check if the INF code
        * corresponds to a valid left component. */
        char res = check_N_dutch(d) || check_A_dutch(d) || check_VP1s_dutch(d) || check_ADV_dutch(d);
        /* Finally, we free the artificial dictionary entry */
        free_dela_entry(d);
        return res;
    }



    /**
    * Returns 1 if the given INF code is a "N" one.
    */
    char check_N_dutch(unichar* INF_code, struct original_tokenize_DELAF_line_buffer* tokenize_buffer) {
        /* We produce an artifical dictionary entry with the given INF code,
        * and then, we tokenize it in order to get grammatical and inflectional
        * codes in a structured way. */
        u_strcpy(tokenize_buffer->buffer, "x,");
        u_strcat(tokenize_buffer->buffer, INF_code);
        struct dela_entry* d = tokenize_DELAF_line(tokenize_buffer->buffer, 0);
        char res = check_N_dutch(d);
        /* We free the artifical dictionary entry */
        free_dela_entry(d);
        return res;
    }




    /**
    * Returns 1 if the INF code refers to a valid left component, 0 otherwise.
    */
    char check_valid_right_component_for_one_INF_code_dutch(const unichar* INF_code, struct original_tokenize_DELAF_line_buffer* tokenize_buffer) {
        /* We produce an artifical dictionary entry with the given INF code,
        * and then, we tokenize it in order to get grammatical and inflectional
        * codes in a structured way. */
        u_strcpy(tokenize_buffer->buffer, "x,");
        u_strcat(tokenize_buffer->buffer, INF_code);
        struct dela_entry* d = tokenize_DELAF_line(tokenize_buffer->buffer, 0);
        char res = check_N_dutch(d);
        /* We free the artifical dictionary entry */
        free_dela_entry(d);
        return res;
    }




    /**
    * This function reads words from the unknown word file and tries to
    * analyse them. The unknown word file is supposed to contain one word
    * per line. If a word cannot be analyzed, we print it to the new
    * unknown word list file.
    */
    void analyse_dutch_unknown_words(struct dutch_infos* infos) {
        u_printf("Analysing Dutch unknown words...\n");
        int n = 0;
        /* We read each line of the unknown word list and we try to analyze it */
        Ustring* line = new_Ustring(1024);
        while (EOF != readline(line, infos->unknown_word_list)) {
            if (!analyse_dutch_word(line->str, infos)) {
                /* If the analysis has failed, we store the word in the
                * new unknown word file */
                u_fprintf(infos->new_unknown_word_list, "%S\n", line->str);
            } else {
                /* Otherwise, we increase the number of analyzed words */
                n++;
            }
        }
        free_Ustring(line);
        u_printf("%d words decomposed as compound word%s\n", n, (n>1) ? "s" : "");
    }


    /**
    * This function tries to analyse an unknown Dutch word. If OK,
    * it returns 1 and print the dictionary entry to the output (and
    * information if an information file has been specified in 'infos');
    * returns 0 otherwise.
    */
    int analyse_dutch_word(const unichar* word, struct dutch_infos* infos) {
        /*
        unichar decomposition[4096];
        unichar dela_line[4096];
        unichar correct_word[4096];*/
        struct dutch_word_decomposition_list* l = NULL;
        /* We look if there are decompositions for this word */
        struct analyse_dutch_permanent * adp = (struct analyse_dutch_permanent *)malloc(sizeof(struct analyse_dutch_permanent));
        if (adp == NULL) {
            fatal_alloc_error("analyse_dutch_word");
        }
        adp->decomposition[0] = '\0';
        adp->dela_line[0] = '\0';
        adp->correct_word[0] = '\0';

        Ustring* ustr = new_Ustring();
        adp->nb_word_decomposition_list_possible = MAX_NB_WORD_DECOMPOSITION_LIST_POSSIBLE;
        explore_state_dutch(infos->d->initial_state_offset, adp->correct_word, 0, word, 0, adp->decomposition, adp->dela_line, &l, adp, 1, infos, ustr, 0);
        free_Ustring(ustr);
        if (l == NULL) {
            /* If there is no decomposition, we return */
            free(adp);
            return 0;
        }
        /* Otherwise, we will choose the one to keep */
        struct dutch_word_decomposition_list* tmp = l;
        int n = 1000;
        /* First, we count the minimal number of components, because
        * we want to give priority to analysis with smallest number
        * of components. By the way, we note if there is a minimal
        * analysis ending by a noun or an adjective. */
        while (tmp != NULL) {
            if (tmp->element->n_parts <= n) {
                n = tmp->element->n_parts;
            }
            tmp = tmp->next;
        }
        tmp = l;
        while (tmp != NULL) {
            if (n == tmp->element->n_parts) {
                /* We only consider the words that have shortest decompositions.
                * The test (tmp->element->n_parts==1) is used to
                * match simple words that would have been wrongly considered
                * as unknown words. */
                if (infos->info_output != NULL) {
                    u_fprintf(infos->info_output, "%S = %S\n", word, tmp->element->decomposition);
                }
                u_fprintf(infos->output, "%S\n", tmp->element->dela_line);
            }
            tmp = tmp->next;
        }
        free_word_decomposition_list_dutch(l);
        free(adp);
        return 1;
    }


    /**
    * Allocates, initializes and returns a word decomposition structure.
    */
    static struct dutch_word_decomposition* new_word_decomposition_dutch(int n_parts, const unichar* decomposition, const unichar* dela_line) {

        #define AroundSizeBorder(x) (((x + 0xf) / 0x10) * 0x10)

        unsigned int len_decomposition_string_in_bytes = (u_strlen(decomposition) + 1) * sizeof(unichar);
        unsigned int len_dela_line_in_bytes = (u_strlen(dela_line) + 1) * sizeof(unichar);
        size_t pos_decomposition = sizeof(struct dutch_word_decomposition);
        size_t pos_dela_line = sizeof(struct dutch_word_decomposition) + AroundSizeBorder(len_decomposition_string_in_bytes);
        size_t size_allocation = pos_dela_line + AroundSizeBorder(len_dela_line_in_bytes);
        struct dutch_word_decomposition* tmp;
        tmp = (struct dutch_word_decomposition*)malloc(size_allocation);
        if (tmp == NULL) {
            fatal_alloc_error("new_word_decomposition_dutch");
        }

        unichar* buffer_decomposition = (unichar*)(((unsigned char*)tmp) + pos_decomposition);
        unichar* buffer_dela_line = (unichar*)(((unsigned char*)tmp) + pos_dela_line);
        u_strcpy(buffer_decomposition, decomposition);
        u_strcpy(buffer_dela_line, dela_line);

        tmp->n_parts = n_parts;
        tmp->decomposition = buffer_decomposition;
        tmp->dela_line = buffer_dela_line;

        return tmp;
    }


    /**
    * Frees a word decomposition structure.
    */
    static void free_word_decomposition_dutch(struct dutch_word_decomposition* t) {
        if (t == NULL) return;
        free(t);
    }


    /**
    * Allocates, initializes and returns a word decomposition list structure.
    */
    static struct dutch_word_decomposition_list* new_word_decomposition_list_dutch() {
        struct dutch_word_decomposition_list* tmp;
        tmp = (struct dutch_word_decomposition_list*)malloc(sizeof(struct dutch_word_decomposition_list));
        if (tmp == NULL) {
            fatal_alloc_error("new_word_decomposition_list_dutch");
        }
        tmp->element = NULL;
        tmp->next = NULL;
        return tmp;
    }


    /**
    * Frees a word decomposition list.
    */
    static void free_word_decomposition_list_dutch(struct dutch_word_decomposition_list* l) {
        struct dutch_word_decomposition_list* tmp;
        while (l != NULL) {
            free_word_decomposition_dutch(l->element);
            tmp = l->next;
            free(l);
            l = tmp;
        }
    }


    /**
    * This explores the dictionary in order decompose the given word into a valid sequence
    * of simple words. For instance, if we have the word "Sommervarmt", we will first
    * explore the dictionary and find that "sommer" is a valid left component that
    * corresponds to the dictionary entry "sommer,.N:msia". Then we will
    * look if the following word "varmt" is in the dictionary. It is
    * the case, with the entry "varmt,varm.A:nsio". As we are at the end of the word to
    * analyze and as "varmt" is a valid rightmost component, we will generate an entry
    * according to the following things:
    *
    * 'output_dela_line'="sommervarmt,sommervarm.A:nsio"
    * 'analysis'="sommer,.N:msia +++ varmt,varm.A:nsio"
    * 'number_of_components'=2
    *
    * Note that the initial "S" was put in lowercase, because the dictionary
    * contains "sommer" and not "Sommer". The lemma is obtained with
    * the lemma of the rightmost component (here "varm"), and the word inherits
    * from the grammatical information of its rightmost component.
    *
    * 'offset': offset of the current node in the binary array 'infos->bin'
    * 'current_component': string that represents the current simple word
    * 'pos_in_current_component': position in the string 'current_component'
    * 'word_to_analyze': the word to analyze
    * 'pos_in_word_to_analyze': position in the string 'word_to_analyze'
    * 'analysis': string that represents the analysis as a concatenation like
    *             "sommer,.N:msia +++ varmt,varm.A:nsio"
    * 'output_dela_line': string that contains the final DELA line. The lemma is
    *                     obtained by replacing the rightmost term of
    *                     the word to analyze by its lemma.
    * 'L': list of all analysis for the given word
    * 'number_of_components': number of components that compose the word.
    * 'infos': global settings.
    */
    static void explore_state_dutch(int offset, unichar* current_component, int pos_in_current_component,
        const unichar* word_to_analyze, int pos_in_word_to_analyze, const unichar* analysis,
        const unichar* output_dela_line, struct dutch_word_decomposition_list** L, struct analyse_dutch_permanent * adp,
        int number_of_components, const struct dutch_infos* infos, Ustring* ustr, int base) {

        /* abort if excess list size */
        if ((adp->nb_word_decomposition_list_possible) <= 0)
            return;

        int final, n_transitions, inf_number;
        int z = save_output(ustr);
        offset = read_dictionary_state(infos->d, offset, &final, &n_transitions, &inf_number);
        if (final) {
            /* If we are in a final state, we can set the end of our current component */
            current_component[pos_in_current_component] = '\0';
            /* We do not consider forbidden words */
            if (infos->forbidden_words == NULL || NO_VALUE_INDEX == get_value_index(current_component, infos->forbidden_words, DONT_INSERT)) {
                /* We don't consider components with a length of 1 */
                if (word_to_analyze[pos_in_word_to_analyze] == '\0') {
                    /* If we have explored the entire original word */
                    /* And if we do not have forbidden word in last position */
                    struct list_ustring* l = infos->d->inf->codes[inf_number];
                    /* We will look at all the INF codes of the last component in order
                    * to produce analysis */
                    while (l != NULL) {
                        unichar* allocated_dec = NULL; const unichar* dec;
                        dec = u_strcpy_optional_buffer(adp->original_dec_buffer, PERMANENT_DEC_STRING_BUFFER_SIZE, &allocated_dec, analysis);
                        //u_strcpy(dec,analysis);
                        if (dec[0] != '\0') {
                            /* If we have already something in the analysis (i.e. if
                            * we have not a simple word), we insert the concatenation
                            * mark before the entry to come */
                            dec = u_strcat_optional_buffer(adp->original_dec_buffer, PERMANENT_DEC_STRING_BUFFER_SIZE, &allocated_dec, " +++ ");
                        }
                        Ustring* entry = new_Ustring(4096);
                        /* We get the dictionary line that corresponds to the current INF code */
                        uncompress_entry(current_component, l->string, entry);
                        /* And we add it to the analysis */
                        dec = u_strcat_optional_buffer(adp->original_dec_buffer, PERMANENT_DEC_STRING_BUFFER_SIZE, &allocated_dec, entry->str);
                        unichar* allocated_new_dela_line = NULL; const unichar* new_dela_line;
                        /* We copy the current output DELA line that contains
                        * the concatenation of the previous components */
                        new_dela_line = u_strcpy_optional_buffer(adp->original_new_dela_line_buffer, PERMANENT_STRING_BUFFER_SIZE, &allocated_new_dela_line, output_dela_line);
                        /* Then we tokenize the DELA line that corresponds the current INF
                        * code in order to obtain its lemma and grammatical/inflectional
                        * information */
                        struct dela_entry* tmp_entry = tokenize_DELAF_line(entry->str, 1);
                        free_Ustring(entry);
                        /* We concatenate the inflected form of the last component to
                        * the output DELA line */
                        new_dela_line = u_strcat_optional_buffer(adp->original_new_dela_line_buffer, PERMANENT_STRING_BUFFER_SIZE, &allocated_new_dela_line, tmp_entry->inflected);
                        /* We put the comma that separates the inflected form and the lemma */
                        new_dela_line = u_strcat_optional_buffer(adp->original_new_dela_line_buffer, PERMANENT_STRING_BUFFER_SIZE, &allocated_new_dela_line, ",");
                        /* And we build the lemma in the same way than the inflected form */
                        new_dela_line = u_strcat_optional_buffer(adp->original_new_dela_line_buffer, PERMANENT_STRING_BUFFER_SIZE, &allocated_new_dela_line, output_dela_line);
                        new_dela_line = u_strcat_optional_buffer(adp->original_new_dela_line_buffer, PERMANENT_STRING_BUFFER_SIZE, &allocated_new_dela_line, tmp_entry->lemma);
                        /* We put the dot that separates the the lemma and the grammatical/inflectional
                        * information */
                        new_dela_line = u_strcat_optional_buffer(adp->original_new_dela_line_buffer, PERMANENT_STRING_BUFFER_SIZE, &allocated_new_dela_line, ".");
                        /* And finally we put the grammatical/inflectional information */
                        new_dela_line = u_strcat_optional_buffer(adp->original_new_dela_line_buffer, PERMANENT_STRING_BUFFER_SIZE, &allocated_new_dela_line, tmp_entry->semantic_codes[0]);
                        int k;
                        for (k = 1; k<tmp_entry->n_semantic_codes; k++) {
                            new_dela_line = u_strcat_optional_buffer(adp->original_new_dela_line_buffer, PERMANENT_STRING_BUFFER_SIZE, &allocated_new_dela_line, "+");
                            new_dela_line = u_strcat_optional_buffer(adp->original_new_dela_line_buffer, PERMANENT_STRING_BUFFER_SIZE, &allocated_new_dela_line, tmp_entry->semantic_codes[k]);
                        }
                        for (k = 0; k<tmp_entry->n_inflectional_codes; k++) {
                            new_dela_line = u_strcat_optional_buffer(adp->original_new_dela_line_buffer, PERMANENT_STRING_BUFFER_SIZE, &allocated_new_dela_line, ":");
                            new_dela_line = u_strcat_optional_buffer(adp->original_new_dela_line_buffer, PERMANENT_STRING_BUFFER_SIZE, &allocated_new_dela_line, tmp_entry->inflectional_codes[k]);
                        }
                        /*
                        * Now we can build an analysis in the form of a word decomposition
                        * structure, but only if the last component is a valid
                        * right one or if it is a verb long enough, or if we find out
                        * that the word to analyze was in fact a simple word
                        * in the dictionary */
                        if (check_valid_right_component_for_one_INF_code_dutch(l->string, &(adp->tok_buffer))
                            || number_of_components == 1) {
                            /*
                            * We set the number of components, the analysis, the actual
                            * DELA line and information about
                            */
                            struct dutch_word_decomposition* wd = new_word_decomposition_dutch(number_of_components, dec, new_dela_line);
                            /* Then we add the decomposition word structure to the list that
                            * contains all the analysis for the word to analyze */
                            struct dutch_word_decomposition_list* wdl = new_word_decomposition_list_dutch();
                            wdl->element = wd;
                            wdl->next = (*L);
                            (*L) = wdl;
                            (adp->nb_word_decomposition_list_possible)--;

                            struct dutch_word_decomposition_list* wdlbrowse = wdl;
                            while (wdlbrowse != NULL)
                            {
                                wdlbrowse = wdlbrowse->next;
                            }
                        }
                        free_dela_entry(tmp_entry);
                        free_string_optional_buffer(&allocated_dec);
                        free_string_optional_buffer(&allocated_new_dela_line);
                        /* We go on with the next INF code of the last component */
                        l = l->next;
                    }
                    /* If are at the end of the word to analyze, we have nothing more to do */
                    return;
                }
                else {
                    /* If we are not at the end of the word to analyze, we must
                    * 1) look if the current component is a valid left one
                    * 2) explore the rest of the original word
                    */
                    if (infos->valid_left_component[inf_number]) {
                        /* If we have a valid component, we look first if we are
                        * in the case of a word followed by a "s" */
                        if (word_to_analyze[pos_in_word_to_analyze] == 's') {
                            /* If we have such a word, we add it to the current analysis,
                            * putting "+++ s +++*/

                            unichar* allocated_dec = NULL; const unichar* dec;
                            dec = u_strcpy_optional_buffer(adp->original_dec_buffer, PERMANENT_DEC_STRING_BUFFER_SIZE, &allocated_dec, analysis);
                            if (dec[0] != '\0') {
                                u_strcat_optional_buffer(adp->original_dec_buffer, PERMANENT_DEC_STRING_BUFFER_SIZE, &allocated_dec, " +++");
                            }
                            /* In order to print the component in the analysis, we arbitrary
                            * take a valid left component among all those that are available
                            * for the current component */
                            unichar* allocated_sia_code = NULL; const unichar* sia_code;
                            unichar original_line[STANDARD_STRING_BUFFER_SIZE]; unichar* allocated_line = NULL; unichar* line;

                            Ustring* entry = new_Ustring(4096);
                            sia_code = get_first_valid_left_component_dutch(infos->d->inf->codes[inf_number], adp->original_siacode_buffer, PERMANENT_STRING_BUFFER_SIZE, &allocated_sia_code, &(adp->tok_buffer));
                            uncompress_entry(current_component, sia_code, entry);
                            dec = u_strcat_optional_buffer(adp->original_dec_buffer, PERMANENT_DEC_STRING_BUFFER_SIZE, &allocated_dec, entry->str);
                            free_Ustring(entry);
                            line = u_strcpy_optional_buffer(original_line, STANDARD_STRING_BUFFER_SIZE, &allocated_line, output_dela_line);
                            line = u_strcat_optional_buffer(original_line, STANDARD_STRING_BUFFER_SIZE, &allocated_line, current_component);
                            /* As we have a double letter at the end of the word,
                            * we must remove a character */
                            line[u_strlen(line) - 1] = '\0';

                            unichar *temp = (unichar*)malloc(4096 * sizeof(unichar));
                            if (temp == NULL){
                                fatal_alloc_error("explore_state_dutch");
                            }
                            unichar original_dec_temp[DEC_TEMP_STRING_BUFFER_SIZE]; unichar* allocated_dec_temp = NULL; const unichar* dec_temp;

                            dec_temp = u_strcpy_optional_buffer(original_dec_temp, DEC_TEMP_STRING_BUFFER_SIZE, &allocated_dec_temp, dec);
                            dec_temp = u_strcat_optional_buffer(original_dec_temp, DEC_TEMP_STRING_BUFFER_SIZE, &allocated_dec_temp, " +++ s");

                            free_string_optional_buffer(&allocated_dec);
                            free_string_optional_buffer(&allocated_sia_code);

                            /* Then, we explore the dictionary in order to analyze the
                            * next component. We start at the root of the dictionary
                            * and we go back one position in the word to analyze */
                            Ustring* foo = new_Ustring();
                            explore_state_dutch (infos->d->initial_state_offset, temp, 0, word_to_analyze, pos_in_word_to_analyze + 1,
                                dec_temp, line, L, adp, number_of_components + 1, infos, foo, 0);
                            free_Ustring(foo);

                            free(temp);
                            free_string_optional_buffer(&allocated_dec_temp);
                            free_string_optional_buffer(&allocated_line);

                        }
                        /* Now, we try to analyze the component normally */
                        unichar* allocated_dec = NULL; const unichar* dec;
                        unichar original_line[STANDARD_STRING_BUFFER_SIZE]; unichar* allocated_line = NULL; const unichar* line;
                        dec = u_strcpy_optional_buffer(adp->original_dec_buffer, PERMANENT_DEC_STRING_BUFFER_SIZE, &allocated_dec, analysis);
                        if (dec[0] != '\0') {
                            /* We add the "+++" mark if the current component is not the first one */
                            dec = u_strcat_optional_buffer(adp->original_dec_buffer, PERMANENT_DEC_STRING_BUFFER_SIZE, &allocated_dec, " +++ ");
                        }
                        unichar original_sia_code[STANDARD_STRING_BUFFER_SIZE]; unichar* allocated_sia_code = NULL; const unichar* sia_code;
                        Ustring* entry = new_Ustring(4096);
                        /* In order to print the component in the analysis, we arbitrary
                        * take a valid left component among all those that are available
                        * for the current component */
                        sia_code = get_first_valid_left_component_dutch(infos->d->inf->codes[inf_number], original_sia_code, STANDARD_STRING_BUFFER_SIZE, &allocated_sia_code, &(adp->tok_buffer));
                        uncompress_entry(current_component, sia_code, entry);
                        free_string_optional_buffer(&allocated_sia_code);
                        dec = u_strcat_optional_buffer(adp->original_dec_buffer, PERMANENT_DEC_STRING_BUFFER_SIZE, &allocated_dec, entry->str);
                        free_Ustring(entry);
                        line = u_strcpy_optional_buffer(original_line, STANDARD_STRING_BUFFER_SIZE, &allocated_line, output_dela_line);
                        line = u_strcat_optional_buffer(original_line, STANDARD_STRING_BUFFER_SIZE, &allocated_line, current_component);

                        unichar *temp = (unichar*)malloc(4096*sizeof(unichar));
                        if (temp == NULL){
                            fatal_alloc_error("explore_state_dutch");
                        }
                        unichar original_dec_temp[DEC_TEMP_STRING_BUFFER_SIZE]; unichar* allocated_dec_temp = NULL; const unichar* dec_temp;
                        dec_temp = u_strcpy_optional_buffer(original_dec_temp, DEC_TEMP_STRING_BUFFER_SIZE, &allocated_dec_temp, dec);
                        free_string_optional_buffer(&allocated_dec);
                        /* Then, we explore the dictionary in order to analyze the
                        * next component. We start at the root of the dictionary */
                        Ustring* foo = new_Ustring();
                        explore_state_dutch (infos->d->initial_state_offset, temp, 0, word_to_analyze, pos_in_word_to_analyze,
                            dec_temp, line, L, adp, number_of_components + 1, infos, foo, 0);
                        free_string_optional_buffer(&allocated_dec_temp);

                        free_string_optional_buffer(&allocated_line);

                        free(temp);
                        free_Ustring(foo);
                    }
                }
            }
            /* Once we have finished to deal with the current final dictionary node,
            * we go on because we may match a longer word */
            base = ustr->len;
        }
        /* We examine each transition that goes out from the node */
        unichar c;
        int adr;
        for (int i = 0; i<n_transitions; i++) {
            offset = read_dictionary_transition(infos->d, offset, &c, &adr, ustr);
            if (is_equal_or_uppercase(c, word_to_analyze[pos_in_word_to_analyze], infos->alphabet)) {
                /* If the transition's letter is case compatible with the current letter of the
                * word to analyze, we follow it */
                current_component[pos_in_current_component] = c;
                explore_state_dutch (adr, current_component, pos_in_current_component + 1, word_to_analyze, pos_in_word_to_analyze + 1,
                    analysis, output_dela_line, L, adp, number_of_components, infos, ustr, base);
            }
            restore_output(z, ustr);
        }
    }

} // namespace unitex
