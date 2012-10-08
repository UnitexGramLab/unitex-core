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

#include "File.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {



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

	//display_list_ustring(result);
	return result;
}




void protect_special_characters(const char *text, const VersatileEncodingConfig* vec){

	U_FILE *source;
	U_FILE *destination;

	//fprintf(stdout,"protect special character\n");

	char temp_name_file[FILENAME_MAX];
	char path[FILENAME_MAX];
	get_path(text,path);
	sprintf(temp_name_file,"%stemp",path);


	source = u_fopen(vec,text,U_READ);
	if( source == NULL){
		fatal_error("Cannot open file %s\n",text);
		exit(1);
	}

	destination = u_fopen(vec,temp_name_file,U_WRITE);
	if( destination == NULL){
		fatal_error("Cannot open file %s\n",temp_name_file);
		exit(1);
	}

	int a;
	a = u_fgetc(source);
	while(a!=EOF){
		u_fputc((unichar)a,destination);
		if(a=='{'){
			//fprintf(stdout,"opening bracket found\n");


			unichar *bracket_string = get_braced_string(source);
			unichar *protected_bracket_string = protect_braced_string(bracket_string);
			//u_fprints(protected_bracket_string,destination);
			u_fprintf(destination,"%S",protected_bracket_string);
			//u_printf("%S --- ",bracket_string);
			//u_printf("%S\n",protected_bracket_string);
			free(bracket_string);
			free(protected_bracket_string);
		}

		a = u_fgetc(source);
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





unichar *get_braced_string(U_FILE *u){

	//u_printf("get_braced string = ");
	int brace_level = 0; // already one brace opened

	long origin_position = ftell(u);
	if (origin_position == -1) {
		fatal_error("ftell");
	}

	int length = 0;
	int a = u_fgetc(u);
	bool protected_char = false;
	while (a != EOF) {
		//u_printf("%C",(unichar)a);
		unichar c = (unichar)a;
		if (protected_char) {
			protected_char = false;
		} else {
			if (c == '\\') {
				protected_char = true;
			} else {
				if (c == '}') {
					if (brace_level == 0) {
						break;
					}
					else {
						brace_level--;
					}
				}
				if(c=='{'){
					brace_level++;
				}

			}
		}
		length++;
		a = u_fgetc(u);
	}

	//u_printf("\n");


	if(a == EOF){
		fatal_error("Unexpected end of file");
	}

	unichar *result;
	result = (unichar*)malloc(sizeof(unichar)*(length+1));
	if(result == NULL){
		fatal_alloc_error("get_braced_string");
		exit(1);
	}

	int fseek_result = fseek(u,origin_position,SEEK_SET);
	if(fseek_result==-1){
		fatal_error("fseek");
	}

	for (int i = 0; i < length; ++i) {
		result[i]=(unichar)u_fgetc(u);
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




}
