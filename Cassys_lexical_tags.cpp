/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
/*
 * Cassys_tags.cpp
 *
 *  Created on: 8 oct. 2012
 *      Author: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */

#include "Cassys_lexical_tags.h"
#include "Cassys_io.h"

#include "File.h"
#include "StringParsing.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {



unichar *protect_lexical_tag(unichar *text, bool is_substring = false) {
	unichar *result = NULL;

	list_ustring *tokens = cassys_tokenize(text);


	int size = 0;
	result = (unichar*)malloc(sizeof(unichar)*(size+1));
	if(result==NULL){
		fatal_alloc_error("malloc");
	}
	result[size]='\0';

	for (list_ustring *ite = tokens; ite != NULL; ite = ite->next) {
		unichar *s=NULL;

		if (is_lexical_token(ite->string)) {
			cassys_pattern *cp = load_cassys_pattern(ite->string);

			unichar *protected_s = protect_lexical_tag( cp->form, true);
			free(cp->form);
			cp->form=(unichar*)malloc(sizeof(unichar)*(u_strlen(protected_s)+1));
			if(cp->form == NULL){
				fatal_alloc_error("malloc");
			}
			u_strcpy(cp->form, protected_s);

			s = cassys_pattern_2_lexical_tag(cp, is_substring);




			size += u_strlen(s);
			result = (unichar*)realloc(result, sizeof(unichar)*(size+1));
			if(result==NULL){
				fatal_alloc_error("malloc");
			}
			u_strcat(result,s);
			free(protected_s);
			free(s);
			free_cassys_pattern(cp);
		} else {
			size += + u_strlen(ite->string);
			result = (unichar*) realloc(result, sizeof(unichar) * (size+1));
			if (result == NULL) {
				fatal_alloc_error("malloc");
			}
			u_strcat(result, ite->string);
		}

	}

	free_list_ustring(tokens);

	return result;
}


/**
 * Return a list of alternative non lexical and lexical token. The first one is a non lexical token.
 */
list_ustring *cassys_tokenize(const unichar* text) {

	list_ustring *result = NULL;

	int position = 0;
	int last_position = 0;

	enum {TEXT_MODE, LEXICAL_TAG_MODE};

	int mode = TEXT_MODE;
	if(text[0] == '{'){
		mode = LEXICAL_TAG_MODE;
	}

	while(text[position]!='\0'){
		int offset=0;

		last_position = position;
		if(mode==TEXT_MODE){
			offset = begin_of_lexical_tag(text+position);
			position += offset;

			mode = LEXICAL_TAG_MODE;
		} else if(mode==LEXICAL_TAG_MODE){
			offset = end_of_lexical_tag(text+position);
			if(offset == -1){
				fatal_error("%S : Expected '}' character in cassys_tokenize", text+position);
			}
			offset++; // add the } character
			position+=offset;

			mode=TEXT_MODE;
		}

		int token_size = offset ;// We take also the closing bracket

		unichar *token = (unichar*)malloc(sizeof(unichar)*(token_size+1));
		if(token == NULL){
			fatal_error("malloc cassys_tokenize\n");
		}
		if(token_size > 0){
			u_strncpy(token,text+last_position, token_size);
		}
		token[token_size]='\0';
		result = insert_at_end_of_list(token, result);
		free(token);

	}

	return result;

}

/**
 * Return the position of the next lexical tag or the last character of the text if no lexical tag exists. In case of end
 * of the text the position returned is the position of the end character \0
 */
int begin_of_lexical_tag(const unichar *text){

	bool protected_character = false;
	int i=0;
	while(text[i]!='\0'){
		i++;
	}

	for(i=0;text[i]!='\0';i++){

		if(protected_character){
			protected_character = false;
			continue;
		}

		if (text[i] == '\\') {
			protected_character = true;
		}

		if (text[i] == '{') {
			return i;
		}
	}

	return i;

}


/**
 * Return the position the end of a lexical tag (the '}' character. The first character is supposed to be the opening round bracket '{'. Return -1
 * if no closing bracket is found
 */
int end_of_lexical_tag(const unichar *text){
	int i=0;

	if(text[0]!='{'){
		return -1;
	}

	bool protected_char = false;
	int brace_depth =1;
	for(i=1; text[i]!='\0' && brace_depth > 0;i++){

		if(protected_char){
			protected_char = false;
			continue;
		}
		if(text[i]=='\\'){
			protected_char=true;
		}
		if(text[i]=='}'){
			brace_depth--;
		}
		if(text[i]=='{'){
			brace_depth++;
		}
		if (brace_depth == 0) {
			return i;
		}
	}


	return -1;

}

/**
 * unprotect all protected characters. The 'text' argument is assumed to be a lexical tag. It has to begin with '{' and end with '}'.
 */
unichar *unprotect_lexical_tag(const unichar *text){

	int text_size = u_strlen(text);

	unichar *result = (unichar*)malloc(sizeof(unichar)*(text_size+1));
	if(result==NULL){
		fatal_alloc_error("unprotect_lexical_tag\n");
	}
	int i;
	for(i=0;i<=text_size;i++){
		result[i]='\0';
	}

	if(text[0]!='{'){
		fatal_error("lexical tag should begin with {\n");
	}


	i=0;
	int j=0;
	bool protected_char = false;
	for(i=0; text[i]!='\0';i++){
		if (protected_char) {
			result[j++] = text[i];
			protected_char = false;
			continue;
		}

		if(text[i]=='\\'){
			protected_char = true;
			continue;
		}

		result[j++] = text[i];

		if(text[i]=='}'){
			break;
		}
	}


	if(text[i]=='\0'){
		fatal_error("unprotect_lexical_tag : unexpected end of string\n");
	}

	return result;

}

/**
 * Return a string lexical tage format of the cassys pattern. If to_protect is set to true,
 * all special characters ('{','}',':','.',',') are protected with '\'
 */
unichar* cassys_pattern_2_lexical_tag(struct cassys_pattern *cp,
		bool to_protect) {

	unichar *result = NULL;

	// First we compute an upper bound on the needed size of unichar string and allocate the necessary memory
	int result_size = 4; // opening and closing bracket

	list_ustring *ite = cp->inflection;
	while (ite != NULL) {
		result_size += u_strlen(ite->string) +2 ; // size of string + a separator char
		ite = ite->next;
	}

	ite = cp->code;
	while (ite != NULL) {
		result_size += u_strlen(ite->string) + 2; // size of string + a separator char
		ite = ite->next;
	}

	result_size += u_strlen(cp->form) +2; // size of string + a separator char

	if(cp->lem != NULL){
		result_size += u_strlen(cp->lem) +2;
	}

	result = (unichar *)malloc(sizeof(unichar)*(result_size+1));
	if(result == NULL){
		fatal_alloc_error("cassys_pattern_2_lexical_tag");
	}
	int i;
	for(i=0; i<result_size+1;i++){
		result[i]='\0';
	}


	// Now we build the string

	int position = 0;
	if(to_protect){
		result[position++]='\\';
	}
	result[position++]='{';

	u_strcat(result, cp->form);
	position+= u_strlen(cp->form);

	if(cp->lem == NULL){
		return result;
	}

	if (cp->lem[0] != '\0' || length(cp->code) > 0 || length(cp->inflection)
			> 0) {
		if (to_protect) {
			result[position++] = '\\';
		}
		result[position++] = ',';

		u_strcat(result, cp->lem);
		position += u_strlen(cp->lem);

		if (length(cp->code) > 0 || length(cp->inflection) > 0) {
			if (to_protect) {
				result[position++] = '\\';
			}
			result[position++] = '.';

			// add the code
			ite = cp->code;
			while (ite != NULL) {
				u_strcat(result, ite->string);
				position += u_strlen(ite->string);
				ite = ite->next;

				if (ite != NULL) {
					if (to_protect) {
						result[position++] = '\\';
					}
					result[position++] = '+';
				}
			}

			// add the inflection
			ite = cp->inflection;
			while (ite != NULL) {
				if (to_protect) {
					result[position++] = '\\';
				}
				result[position++] = ':';

				u_strcat(result, ite->string);
				position += u_strlen(ite->string);
				ite = ite->next;
			}


		}
	}

	if (to_protect) {
		result[position++] = '\\';
	}
	result[position++] = '}';
	result[position++] = '\0';

	return result;
}



struct cassys_pattern* load_cassys_pattern(unichar *string){

	struct cassys_pattern *cp = NULL;

	cp = (struct cassys_pattern*) malloc(sizeof(struct cassys_pattern));
	if (cp == NULL) {
		fatal_alloc_error("malloc");
	}
	cp->form = NULL;
	cp->lem = NULL;
	cp->code=NULL;
	cp->inflection=NULL;

	unichar *result = (unichar *)malloc(sizeof(unichar)*(u_strlen(string)+1));
	if(result == NULL){
		fatal_alloc_error("malloc");
	}

	int string_size = u_strlen(string);
	int form_lemma_separator_position = get_form_lemma_separator_position(string);

	//u_printf("string = %S\n",string);

	if (form_lemma_separator_position == 0) {
		/*
		 * No comma so this lexical tag only contains form
		 */
		cp->form = (unichar*) malloc(sizeof(unichar) * (string_size - 2 + 1)); // -2 to exclude { and }
		if (cp->form == NULL) {
			fatal_alloc_error("malloc");
		}
		u_strncpy(cp->form, string + 1, string_size - 2); // we copy string without brackets so begin at position 1 and lenght -2
		cp->form[string_size - 2] = '\0';

		free(result);

		return cp;
	}

	int form_size = form_lemma_separator_position - 1; // -1 to exclude {
	cp->form = (unichar*) malloc(sizeof(unichar) * (form_size + 1));
	if (cp->form == NULL) {
		fatal_alloc_error("malloc");
	}
	u_strncpy(cp->form, string + 1, form_size); // we copy string without brackets so begin at position 1
	cp->form[form_size] = '\0';


	int position = 0;

	// put the annotation in the string with same name
	int annotation_size = string_size - form_size - 3; // -3 to exclude { and } and ,
	unichar *annotation = (unichar *) malloc(sizeof(unichar)
			* (annotation_size + 1));
	if (annotation == NULL) {
		fatal_alloc_error("malloc");
	}

	u_strncpy(annotation, string + form_lemma_separator_position + 1, annotation_size); // again +1 to exclude comma
	annotation[annotation_size]='\0';

	position = 0;
	if (parse_string(annotation, &position, result, P_DOT) != P_OK) {
		fatal_error("%S : malformed cassys pattern\n",annotation);
	}
	cp->lem = (unichar*) malloc(sizeof(unichar) * (u_strlen(result)+ 1));
	if (cp->lem == NULL) {
		fatal_alloc_error("malloc");
	}
	u_strcpy(cp->lem,result);

	enum {CODE,INFLEXION};
	int state = CODE;
	while(annotation[position] != '\0'){
		const unichar P_PLUS_CLOSING_ROUND_BRACE[]= { '+', '}', ':', 0 };
		position++;
		if (parse_string(annotation, &position, result,
				P_PLUS_CLOSING_ROUND_BRACE) != P_OK) {
			fatal_error("%S : malformed cassys pattern\n",annotation);
		}
		if(u_strlen(result) >0){
			unichar *token = (unichar *)malloc(sizeof(unichar)*(u_strlen(result)+1));
			if(token==NULL){
				fatal_error("malloc");
			}
			u_strcpy(token, result);
			if(state == CODE){
				cp->code = insert_at_end_of_list(token, cp->code);
			}
			if(state == INFLEXION){
				cp->inflection = insert_at_end_of_list(token, cp->inflection);
			}

			if(annotation[position] == ':'){
				state = INFLEXION;
			}
			free(token);
		}
	}
	free(annotation);
	free(result);

	return cp;
}


void protect_special_characters(const char *text, const VersatileEncodingConfig* vec){

	U_FILE *source;
	U_FILE *destination;

	char temp_name_file[FILENAME_MAX];
	char path[FILENAME_MAX];
	get_path(text,path);
	sprintf(temp_name_file,"%stemp",path);


	source = u_fopen(vec,text,U_READ);
	if( source == NULL){
		fatal_error("Cannot open file %s\n",text);
		exit(1);
	}

	fprintf(stdout,"pre File source loaded\n");

	unichar *source_text = read_file(source);

	fprintf(stdout,"File source loaded\n");

	destination = u_fopen(vec,temp_name_file,U_WRITE);
	if( destination == NULL){
		fatal_error("Cannot open file %s\n",temp_name_file);
		exit(1);
	}

	int position = 0;
	while(source_text[position] != '\0'){

		if(source_text[position]=='{'){

			unichar *bracket_string = get_braced_string(source_text, &position);
			unichar *protected_bracket_string = protect_braced_string(bracket_string);
			u_fprintf(destination,"%S",protected_bracket_string);
			free(bracket_string);
			free(protected_bracket_string);
		}

		position++;
	}

	u_fclose(source);
	u_fclose(destination);

	copy_file(text,temp_name_file);

	// should delete the 'temp' file
}







unichar *protect_lem_in_braced_string(const unichar *s){
	int length = u_strlen(s);
	//u_printf("%S = length = %d\n", s,length);
	int i;
	// find the lemm/label separator
	for (i = length-1; i >= 0; i--) {

		if (s[i] == '.') {
			break;
		}
	}

	if (i < 0) {
		fatal_error("protect_text error : no dots in string %S", s);
	}


	// nothing to do, just copy the lem
	unichar *result = (unichar*)malloc(sizeof(unichar)*(length+1));
	if(result == NULL){
		fatal_alloc_error("protect_lem_in_braced_string");
		exit(1);
	}
	i++;
	int j=0;
	while(i<length){
		result[j]=s[i];
		i++;j++;
	}
	result[j]='\0';
	return result;
}





unichar *get_braced_string( unichar *string, int *position){

	//u_printf("get_braced string = ");
	int brace_level = 0; // already one brace opened

	int origin_position = *position;

	bool protected_char = false;
	while (string[*position] != EOF) {
		if (protected_char) {
			protected_char = false;
		} else {
			if (string[*position] == '\\') {
				protected_char = true;
			} else {
				if (string[*position] == '}') {
					if (brace_level == 0) {
						break;
					}
					else {
						brace_level--;
					}
				}
				if(string[*position] =='{'){
					brace_level++;
				}

			}
		}
		*position = *position +1;
	}

	//u_printf("\n");


	if(string[*position] == '\0'){
		fatal_error("Unexpected end of file");
	}

	unichar *result;
	int length = *position - origin_position;
	result = (unichar*)malloc(sizeof(unichar)*(length+1));
	if(result == NULL){
		fatal_alloc_error("get_braced_string");
		exit(1);
	}

	for (int i = 0; i < length; ++i) {
		result[i]= string[*position + i];
	}
	result[length]='\0';

	return result;
}


unichar *protect_braced_string(const unichar *s){
	unichar *result;
	unichar *stop_sentence;

	stop_sentence = (unichar*) malloc(sizeof(unichar) * (1 + 1));
	if (stop_sentence == NULL) {
		fatal_alloc_error("protect_braced_string");
		exit(1);
	}
	u_sprintf(stop_sentence, "S");

	if (u_strcmp(stop_sentence, s) == 0) {
		return stop_sentence;

	} else {
		unichar* text = protect_text_in_braced_string(s);
		unichar* lem = protect_lem_in_braced_string(s);

		//u_printf("text / lem = %S --- %S\n",text, lem);

		int length_t = u_strlen(text);
		int length_l = u_strlen(lem);

		result = (unichar*) malloc(sizeof(unichar) * (length_t + length_l + 2
				+ 1));
		if (result == NULL) {
			fatal_alloc_error("protect_braced_string");
			exit(1);
		}

		u_sprintf(result, "%S,.%S", text, lem);

		free(lem);
		free(text);
		free(stop_sentence);
	}
	return result;
}


unichar *protect_text_in_braced_string(const unichar *s){
	int length = u_strlen(s);
	int i;
	// find the lemm/label separator
	for(i=length-1; i>=0; i--){
		if(s[i]=='.'){
			break;
		}
	}

	if(i<0){
		fatal_error("protect_text error : no dots in string %s",s);
	}

	i--;
	if(s[i] != ','){
		fatal_error("protect_text error : no comma in string %s",s);
	}


	unichar *result;
	// Alloc twice the memory of s to be sure to have enough space for escape chars.
	result = (unichar*)malloc(sizeof(unichar)*(length*2+1));
	if(result == NULL){
		fatal_alloc_error("protect_text_in_braced_string");
		exit(1);
	}

	// j for s and k for result
	int j,k;
	for(j=0,k=0; j<i; k++,j++){
		if(s[j] == '\\'){
			result[k]=s[j];
			j++;k++;
			result[k]=s[j];
			continue;
		}

		if(s[j] == '{'|| s[j] == '}' || s[j] == '+' || s[j] == ',' || s[j] == '.' || s[j] == ':'){
			result[k] = '\\';
			k++;
			result[k] = s[j];
			continue;
		}
		result[k] = s[j];
	}
	result[k]='\0';
	return result;

}

bool is_lexical_token(unichar *token){

	int size = u_strlen(token);

	if(token[0] == '{' && token[size-1] == '}'){
		return true;
	}
	return false;

}




list_ustring *cassys_tokenize_word_by_word(const unichar* text,const Alphabet* alphabet){

	list_ustring *result = NULL;
	unichar token[4096];
	unichar braced_token[4096];
	int i=0,j=0;


	int opened_bracket = 0;
	bool protected_char = false;
	bool token_found = false;

	//u_printf("Cassys_tokenize : \n");
	//u_printf("chaine = %S\n",text);

	while(text[i]!='\0'){
		if (!opened_bracket) {
			token[j++]=text[i];
			if (is_letter(text[i], alphabet)) {
				if (!is_letter(text[i + 1], alphabet)) {
					token_found = true;

				}
			}
			else {
				if(text[i]=='{'){
					opened_bracket ++;
					j=0;
				}
				else {
					token_found = true;
				}
			}

		}
		else {
			braced_token[j++] = text[i];

			if(protected_char){
				protected_char = false;
			}
			else {
				if(text[i]=='\\'){
					protected_char = true;
				}
				if (text[i] == '}'){
					opened_bracket --;
					if(!opened_bracket){
						token_found = true;
						braced_token[--j]='\0';
						//u_printf("protect string : \n");
						unichar *protected_braced_string = protect_braced_string(braced_token);
						u_sprintf(token,"{%S}",protected_braced_string);
						free(protected_braced_string);
						j=u_strlen(token);
					}
				}
				if (text[i] == '{'){
					opened_bracket ++;
				}
			}
		}
		if(token_found){
			token[j] = '\0';
			result = insert_at_end_of_list(token,result);
			j=0;
			token_found = false;
			//u_printf("token = %S\n",token);
		}
		i++;
	}

	return result;
}


/**
 * Text is supposed to be a well formed lexical tag
 */
int get_form_lemma_separator_position(unichar *text){

	/*
	 * The form lemma separator is the position of the last comma of a lexical tag
	 */

	int size = u_strlen(text);

	int i=size;
	int brace_level = 0;
	for(i=size-1; i>0; i--){

		/**
		 * Comma are only looked for outside of { }. It does not matter whether these brackets are protected
		 *
		 */
		if(text[i]=='}'){
			brace_level++;
		}
		if(text[i]=='{'){
			brace_level--;
		}

		// Since a lexical tag is supposed to be given as argument, a closing bracket should be the las character of
		// string. So brace_level is 1 and not 0.
		if(text[i]==',' && brace_level == 1){
			break;
		}
	}
	return i;

}


void free_cassys_pattern(cassys_pattern *cp){

	if(cp!=NULL){
		if(cp->form!=NULL){
			free(cp->form);
		}
		if(cp->lem!=NULL){
			free(cp->lem);
		}
		free_list_ustring(cp->code);
		free_list_ustring(cp->inflection);
		free(cp);
	}


}



}
