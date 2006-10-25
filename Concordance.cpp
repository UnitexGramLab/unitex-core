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
  
#include "Concordance.h"
#include "unicode.h"
#include "Matches.h"
#include "SortTxtMain.h"
#include "Error.h"
#include "Buffer.h"

#define HTML_ 0
#define TEXT_ 1
#define GLOSSANET_ 2

int create_raw_text_concordance(FILE*,FILE*,FILE*,struct text_tokens*,int,int,
                                int*,int*,int,int,struct conc_opt);
void compute_token_length(int*,struct text_tokens*);

void create_modified_text_file(FILE*,FILE*,struct text_tokens*,char*,int,int*);
void write_HTML_header(FILE*,int,struct conc_opt);
void write_HTML_end(FILE*);
void reverse_initial_vowels_thai(unichar*);
int get_shift(int,int*,int);


/**
 * This function builds a concordance from a 'concord.ind' file
 * described by the 'concordance' parameter. 'text' is supposed to
 * represent the 'text.cod' file from which the concordance index was
 * computed. 'tokens' represents the associated 'tokens.txt'
 * file. 'option.sort_mode' is an integer that represents the sort mode to be
 * used for creating the concordance. This parameter will be ignored
 * if the function must modify the text instead of building a
 * concordance.  'option.left_context' and 'option.right_context' specify the length
 * of the contexts in visible characters (for Thai, this number is
 * different from the number of unicode characters because of
 * diacritics). 'option.fontname' and 'fontsize' are used to set the font
 * that will be used if the output is an HTML file (if not, these
 * parameters will be ignored). 'option.directory' represents the
 * working directory.  'option.result_mode' indicates the kind of
 * output that is expected. If it value if "html" or "text", the
 * function will build an HTML or text concordance. If its value is of
 * the form "glossanet=xxx", the result will be an HTML concordance
 * designed for the GlossaNet system (http://glossa.fltr.ucl.ac.be),
 * and "xxx" will be taken as a parameter given to GlossaNet. Any
 * other value will be considered as a file name to use for producing
 * a modified version of the text. 'option.sort_alphabet' is the name of the
 * "Alphabet_sort.txt" file to use for sorting the lines of the
 * concordance. This parameter will be ignored if the output is a
 * modified text file or if the sort mode is TEXT_ORDER.
 * 'n_enter_char' is the number of new lines in the text, and
 * 'enter_pos' is an array that contains the positions of these new
 * lines. If 'option.thai_mode' is set to a non zero value, it indicates that
 * the concordance is a Thai one. This information is used to compute
 * correctly the context sizes.
 */
void create_concordance(FILE* concordance,FILE* text,struct text_tokens* tokens,
                        int n_enter_char,int* enter_pos,struct conc_opt option) {
FILE* out;
FILE* f;
char temp_file_name[2000];
char output_file_name[2000];
char* script_glossanet=NULL;
struct string_hash* glossa_hash=NULL;
int RES;
int open_bracket=-1;
int close_bracket=-1;
/* We compute the length of each token */
int* token_length=(int*)malloc(sizeof(int)*tokens->N);
if (token_length==NULL) {fatal_error("Not enough memory in create_concordance\n");}
compute_token_length(token_length,tokens);
/* Then, we see which kind of output is expected */
script_glossanet=strstr(option.result_mode,"glossanet=");
if (script_glossanet!=NULL) {
	/* If the result parameter starts with "glossanet=", then we make
	 * start the GlossaNet script after the equal sign. */
	script_glossanet=script_glossanet+strlen("glossanet=");
}
if (strcmp(option.result_mode,"html") && strcmp(option.result_mode,"text") && script_glossanet==NULL) {
	/* If we have to produced a modified version of the original text, we
	 * do it and return. */
	create_modified_text_file(concordance,text,tokens,option.result_mode,n_enter_char,enter_pos);
	return;
}
/* If the expected result is a concordance */
if (!strcmp(option.result_mode,"html")) RES=HTML_;
else if (!strcmp(option.result_mode,"text")) RES=TEXT_;
else {
	RES=GLOSSANET_;
	/* The structure glossa_hash will be used to ignore duplicate lines
	 * without sorting */
	glossa_hash=new_string_hash();
	/* Building GlossaNet concordances requires to locate square brackets in the
	 * text. That's why we compute the token numbers associated to '[' and ']' */
	unichar r[2];
	r[0]='[';
	r[1]='\0';
	open_bracket=get_token_number(r,tokens);
	r[0]=']';
	close_bracket=get_token_number(r,tokens);
}
/* We set temporary and final file names */
strcpy(temp_file_name,option.working_directory);
strcat(temp_file_name,"concord_.txt");
strcpy(output_file_name,option.working_directory);
if (RES==TEXT_)
	strcat(output_file_name,"concord.txt");
else
	strcat(output_file_name,"concord.html");
int N_MATCHES;
f=u_fopen(temp_file_name,U_WRITE);
if (f==NULL) {
	error("Cannot write %s\n",temp_file_name);
	return;
}
/* First, we create a raw text concordance. 
 * NOTE: columns may have been reordered according to the sort mode. See the
 * comments of the 'create_raw_text_concordance' function for more details. */
N_MATCHES=create_raw_text_concordance(f,concordance,text,tokens,
                                      RES,n_enter_char,enter_pos,
                                      token_length,open_bracket,close_bracket,
                                      option);
u_fclose(f);
free(token_length);
/* If necessary, we sort it by invoking the main function of the SortTxt program */
if (option.sort_mode!=TEXT_ORDER) {
	char** argv;
	argv=(char**)malloc(6*sizeof(char*));
	argv[0]=strdup(" ");
	argv[1]=strdup(temp_file_name);
	argv[2]=strdup("-n");
	int i=3;
	if (strcmp(option.sort_alphabet,"NULL")) {
		argv[i++]=strdup("-o");
		argv[i++]=strdup(option.sort_alphabet);
	}
	if (option.thai_mode) {
		argv[i++]=strdup("-thai");
	}
	main_SortTxt(i,argv);
	for (int j=0;j<i;j++) {
		free(argv[j]);
	}
	free(argv);
}
/* Now, we will take the sorted raw text concordance and we will:
 * 1) reorder the columns
 * 2) insert HTML info if needed
 */

f=u_fopen(temp_file_name,U_READ);
if (f==NULL) {
	error("Cannot read %s\n",temp_file_name);
	return;
}
if (RES!=TEXT_) {
	out=fopen(output_file_name,"w");
} else {
	out=u_fopen(output_file_name,U_WRITE);
}
if (out==NULL) {
	error("Cannot write %s\n",output_file_name);
	u_fclose(f);
	return;
}
/* If we have an HTML or a GlossaNet concordance, we must write an HTML
 * file header. */
if (RES!=TEXT_) write_HTML_header(out,N_MATCHES,option);
unichar A[3000];
unichar B[3000];
unichar C[3000];
unichar href[3000];
unichar indices[100];
unichar* left=NULL;
unichar* middle=NULL;
unichar* right=NULL;
int j;
int c;
/* Now we process each line of the sorted raw text concordance */
while ((c=u_fgetc(f))!=EOF) {
	j=0;
	/* We save the first column in A... */
	while (c!=0x09) {
		A[j++]=(unichar)c;
		c=u_fgetc(f);
	}
	A[j]='\0';
	c=u_fgetc(f);
	j=0;
	/* ...the second in B... */
	while (c!=0x09) {
		B[j++]=(unichar)c;
		c=u_fgetc(f);
	}
	B[j]='\0';
	c=u_fgetc(f);
	j=0;
	/* ...and the third in C */
	while (c!='\n' && c!='\t') {
		C[j++]=(unichar)c;
		c=u_fgetc(f);
	}
	C[j]='\0';
	indices[0]='\0';
	/* If there are indices to be read like "15 17 1", we read them */
	if (c=='\t') {
		c=u_fgetc(f);
		j=0;
		while (c!='\t' && c!='\n') {
			indices[j++]=(unichar)c;
			c=u_fgetc(f);
		}
		indices[j]='\0';
		/*------------begin GlossaNet-------------------*/
		/* If we are in GlossaNet mode, we extract the url at the end of the line */
		if (RES==GLOSSANET_) {
			if (c!='\t') {
				error("ERROR in GlossaNet concordance: no URL found\n");
				href[0]='\0';
			} else {
				j=0;
				while ((c=u_fgetc(f))!='\n') {
					href[j++]=(unichar)c;
				}
				href[j]='\0';
			}
		}
		/*------------end GlossaNet-------------------*/
	}
	/* Now we will reorder the columns according to the sort mode */
	switch(option.sort_mode) {
		case TEXT_ORDER: left=A; middle=B; right=C; break;
		case LEFT_CENTER: left=A; middle=B; right=C; break;
		case LEFT_RIGHT: left=A; right=B; middle=C; break;
		case CENTER_LEFT: middle=A; left=B; right=C; break;
		case CENTER_RIGHT: middle=A; right=B; left=C; break;
		case RIGHT_LEFT: right=A; left=B; middle=C; break;
		case RIGHT_CENTER: right=A; middle=B; left=C; break;
	}
	/* We use 'can_print_line' to decide if the concordance line must be
	 * printed, because in GlossaNet mode, duplicates must be removed. */
	int can_print_line=1;
	if (RES==GLOSSANET_) {
		unichar line[4000];
		u_strcpy(line,left);
		u_strcat_char(line,"\t");
		u_strcat(line,middle);
		u_strcat_char(line,"\t");
		u_strcat(line,right);
		/* We test if the line was already seen */
		if (-1 == get_token_number(line,glossa_hash)) {
			can_print_line=1;
			get_hash_number(line,glossa_hash);
		} else {
			can_print_line=0;
		}
	}
	/* If we can print the line */
	if (can_print_line) {
		if (option.sort_mode!=TEXT_ORDER) {
			/* If the concordance was sorted, the left sequence was reversed, and
			 * then, we have to reverse it again. However, the Thai sort algorithm
			 * requires to modify some vowels. That's why we must apply a special
			 * procedure if we have a Thai sorted concordance. */
			if (option.thai_mode) reverse_initial_vowels_thai(left);
			/* Now we revert and print the left context */
			if (RES!=TEXT_) {
				fprintf(out,"<tr><td nowrap>");
				u_fprints_html_reverse(left,out);
			} else {u_fprints_reverse(left,out);}
		} else {
			/* If the concordance is not sorted, we do not need to revert the
			 * left context. */
			if (RES!=TEXT_) {
				fprintf(out,"<tr><td nowrap>");
				u_fprints_html(left,out);
			} else {u_fprints(left,out);}
		}
		/* If we must produce an HTML concordance, then we surround the
		 * located sequence by HTML tags in order to make it an hyperlink.
		 * This hyperlink will contain a fake URL of the form "X Y Z", where
		 * X and Y are the starting and ending position of the sequence (in
		 * tokens) and Z is the number of the sentence that contains the
		 * sequence. */
		if (RES==HTML_) {
			char tmp[100];
			u_to_char(tmp,indices);
			fprintf(out,"<a href=\"%s\">",tmp);
			u_fprints_html(middle,out);
			fprintf(out,"</a>");
			u_fprints_html(right,out);
			fprintf(out,"%s</td></tr>\n",NBSP);
		}
		/* If we must produce a GlossaNet concordance, we turn the sequence
		 * into an URL, using the given GlossaNet script. */
		else if (RES==GLOSSANET_) {
			fprintf(out,"<A HREF=\"%s?rec=",script_glossanet);
			u_fprints_html(middle,out);
			fprintf(out,"&adr=");
			u_fprints_html(href,out);
			fprintf(out,"\" style=\"color: rgb(0,0,128)\">");
			u_fprints_html(middle,out);
			fprintf(out,"</A>");
			u_fprints_html(right,out);
			fprintf(out,"</td></tr>\n");
		}
		/* If must must produce a text concordance */
		else if (RES==TEXT_) {
			u_fprints_char("\t",out);
			u_fprints(middle,out);
			u_fprints_char("\t",out);
			u_fprints(right,out);
			u_fprints_char("\n",out);
		}
	}
}
/* If we have an HTML or a GlossaNet concordance, we must write some
 * HTML closing tags. */
if (RES!=TEXT_) write_HTML_end(out);
u_fclose(f);
remove(temp_file_name);
fclose(out);
if (RES==GLOSSANET_) {
	free_string_hash(glossa_hash);
}
}


/**
 * This function computes the length in unicode characters of
 * all the given tokens.
 */
void compute_token_length(int* token_length,struct text_tokens* tokens) {
int i;
for (i=0;i<tokens->N;i++) {
  token_length[i]=u_strlen(tokens->token[i]);
}
}


/**
 * This function writes the HTML header for an HTML or a GlossaNet concordance.
 */
void write_HTML_header(FILE* f,int number_of_matches,struct conc_opt option) {
fprintf(f,"<html lang=en>\n");
fprintf(f,"<head>\n");
fprintf(f,"   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
fprintf(f,"   <title>%d match%s</title>\n",number_of_matches,(number_of_matches>1)?"es":"");
fprintf(f,"</head>\n");
fprintf(f,"<body>\n<table border=\"0\" cellpadding=\"0\" width=\"100%%\" style=\"font-family: %s; font-size: %s\">\n",option.fontname,option.fontsize);
}


/**
 * This function write the HTML closing tags for an HTML or a GlossaNet concordance.
 */
void write_HTML_end(FILE* f) {
fprintf(f,"</table></body>\n");
fprintf(f,"</html>\n");
}


/**
 * This function moves in the file 'text' until the buffer 'buffer' contains the sequence
 * that begins at the global position 'start_pos', but with a large enough left context:
 * 
 * buffer:
 * 0                              X                                buffer->MAXIMUM_BUFFER_SIZE
 * ------------------------------------------------------------------------------
 * |                              |                                             |
 * | <-- MAX_CONTEXT_IN_UNITS --> |                                             |
 * |                              |                                             |
 * ------------------------------------------------------------------------------
 * 
 * buffer[X] will contain the token number 'start_pos'.
 * 
 * 
 * The function updates the '*n_units_already_read' parameter that counts the total
 * number of tokens that have been read since the beginning of the file 'text'. It also
 * updates the parameter '*current_origin_in_chars' that counts the length in chars of the
 * tokens that have been read since the beginning of the file 'text'. This counter is used
 * for computing the offset in characters for a given sequence. This information will
 * be used by the graphical interface to highlight a match in the text. 'token_length'
 * is an array that associates to each token its length in characters.
 * '*current_sentence' is used to count the number of '{S}' in order to know
 * what is the current sentence number.
 */
void move_buffer_to_position(int start_pos,FILE* text,struct text_tokens* tokens,int* token_length,
					struct buffer* buffer,int *n_units_already_read,
					int *current_origin_in_chars,int *current_sentence) {
/* Before moving in the file, buffer[0] contains the token number '*n_units_already_read'.
 * After moving, we want it to contain the token number 'start_pos'-'MAX_CONTEXT_IN_UNITS',
 * so we move of ((start_pos-MAX_CONTEXT_IN_UNITS)-*n_units_already_read) int. */
int jump_size=((start_pos-MAX_CONTEXT_IN_UNITS)-(*n_units_already_read));
/* We go to '*n_units_already_read' * sizeof(int) */
fseek(text,(*n_units_already_read)*sizeof(int),SEEK_SET);
/* And we read ((start_pos-MAX_CONTEXT_IN_UNITS)-*n_units_already_read) integers */
while (jump_size!=0) {
	buffer->size=fread(buffer->buffer,sizeof(int),(jump_size>=buffer->MAXIMUM_BUFFER_SIZE)?buffer->MAXIMUM_BUFFER_SIZE:jump_size,text);
	jump_size=jump_size-buffer->size;
	for (int i=0;i<buffer->size;i++) {
		/* We update the current position in characters */
		current_origin_in_chars=current_origin_in_chars+token_length[buffer->buffer[i]];
		/* And the current sentence number */
		if (buffer->buffer[i]==tokens->SENTENCE_MARKER) {(*current_sentence)++;}
	}
}
/* Finally, we update '*n_units_already_read' */
(*n_units_already_read)=start_pos-MAX_CONTEXT_IN_UNITS;
/* And we fill the buffer with the integers that we need */
buffer->size=fread(buffer->buffer,sizeof(int),buffer->MAXIMUM_BUFFER_SIZE,text);
}


/**
 * This function fills the string 'left' with the mirror of the string of length
 * 'option.left_context' corresponding to the tokens located before the token number
 * 'pos'. 'token_length' is an array that gives the lengthes of the tokens.
 * 'buffer' contains the token numbers to work on. 'option.thai_mode' indicates by a non
 * zero value that we deal with a Thai sequence; in that case, we must do a special
 * operation in order to count correctly displayable characters.
 * 
 * Note that extra spaces will be used to fill 'left' if there no left context enough,
 * in order to preserve alignment at display time.
 */
void extract_left_context(int pos,unichar* left,struct text_tokens* tokens,
                          struct conc_opt option,int* token_length,
                          struct buffer* buffer) {
int i;
/* If there is no left context at all, we fill 'left' with spaces. */
if (pos==0) {
	for (i=0;i<option.left_context;i++) {
		left[i]=' ';
	}
	left[i]='\0';
	return;
}
i=0;
int count=0;
left[option.left_context]='\0';
/* We must start on the left of the match */
pos--;
int l=token_length[buffer->buffer[pos]]-1;
unichar* s=tokens->token[buffer->buffer[pos]];
/* We look for every token, until we have the correct number of displayable
 * characters. */
while (pos>=0 && count<option.left_context) {
	left[i]=s[l--];
	if (!option.thai_mode || !u_is_to_be_ignored_thai(left[i])) {
		/* We increase the character count only we don't have a diacritic mark */
		count++;
	}
	i++;
	if (l<0) {
		/* If we must change of token */
		if (option.left_context_until_eos
                    && !u_strcmp_char(tokens->token[buffer->buffer[pos]],"{S}"))
                  break; /* token was "{S}" */
		pos--;
		if (pos>=0) {
			/* And if we can, i.e. we are not at the beginning of the text */
			l=token_length[buffer->buffer[pos]]-1;
			s=tokens->token[buffer->buffer[pos]];
		}
	}
}
/* If it was not possible to get to correct number of characters because
 * the sequence was too close to the beginning of the text, we fill
 * 'left' with spaces. */ 
if (count!=option.left_context) {
	while (count++!=option.left_context) {
		left[i++]=' ';
	}
}
left[i]='\0';
/* Finally, we reverse the string because we want the mirror of the left context.
 * Note that we cannot fill the buffer from the end because of Thai diacritics that
 * can make the length of left in characters greater than 'LEFT_CONTEXT_LENGTH'. */
u_reverse_string(left);
}


/**
 * This function fills 'middle' with the matched sequence represented by the token
 * range [start_pos,end_pos]. 'output' is the output sequence that was computed
 * during the locate operation. If not NULL, we ignore the original text and copy
 * this value to 'middle'; otherwise we concatenate the tokens that compose
 * the matched sequence. Note that we allways take the whole sequence, so that
 * 'middle' must have been carefully allocated. 'buffer' contains the token numbers
 * to work on.
 */
void extract_match(int start_pos,int end_pos,unichar* output,unichar* middle,
					struct text_tokens* tokens,struct buffer* buffer) {
int j,k;
unichar* s;
if (output==NULL) {
	/* If there is no output, we compute the match from the text */
	j=0;
	for (int i=start_pos;i<=end_pos;i++) {
		k=0;
		s=tokens->token[buffer->buffer[i]];
		while (s[k]!='\0') {
			middle[j++]=s[k++];
		}
	}
	middle[j]='\0';
} else {
	/* If there is an output, then the match is the output */
	u_strcpy(middle,output);
}
}


/**
 * This function fills the string 'right' with the string of length
 * 'option.right_context'-'match_length' corresponding to the tokens located
 * after the token number 'pos'. Note that the 'right' may be empty if the match
 * was already greater or equal to 'option.right_context'.
 * 
 * 'token_length' is an array that gives the lengthes of the tokens.
 * 'buffer' contains the token numbers to work on. 'option.thai_mode' indicates by a non
 * zero value that we deal with a Thai sequence; in that case, we must do a special
 * operation in order to count correctly displayable characters.
 */
void extract_right_context(int pos,unichar* right,struct text_tokens* tokens,
                           int match_length,struct conc_opt option,
                           struct buffer* buffer) {
right[0]='\0';
if (match_length>=option.right_context || pos+1>=buffer->size) {
   /* We return if we have allready overpassed the right context length
    * with the matched sequence, or if there is no right context because
    * we are at the end of the text. */
    return;
}
int right_context_length=option.right_context-match_length;
int i=0;
int count=0;
/* We must start after the last token of the matched sequence */
pos++;
int l=0;
unichar* s=tokens->token[buffer->buffer[pos]];
while (pos<buffer->size && count<right_context_length) {
	right[i]=s[l++];
	if (!option.thai_mode || !u_is_to_be_ignored_thai(right[i])) count++;
	i++;
	if (s[l]=='\0') {
		/* If we must change of token */
		if (option.right_context_until_eos
                    && !u_strcmp_char(tokens->token[buffer->buffer[pos]],"{S}"))
                  break; /* token was "{S}" */
		pos++;
		if (pos<buffer->size) {
			/* And if we can */
			l=0;
			s=tokens->token[buffer->buffer[pos]];
		}
	}
}
/* We don't fill 'right' with spaces if we have reached the end of the text, because
 * there is no alignment problem on the right side of concordance. */
right[i]='\0';
}


/**
 * A GlossaNet concordance is supposed to have been computed on a text of the
 * following form:
 * 
 * document1
 * [[url1]]
 * document2
 * [[url2]]
 * ...
 * 
 * This function tries to find the URL between [[ and ]] that follows the matched sequence
 * in the text (i.e. after the token number 'end_pos'). If there is one, it is copied in
 * 'href'; if not, 'href' is empty. 1 is returned except if the function finds out that
 * the matched sequence is a part of an URL. In that case, 0 is returned in order to
 * indicate that this is not a real matched sequence.
 * 
 * The function assumes that 'buffer' is large enough to find the URL. It may
 * not work anymore if the buffer size is too small.
 */
int extract_href(int end_pos,unichar* href,struct text_tokens* tokens,struct buffer* buffer,
				int open_bracket,int close_bracket) {
href[0]='\0';
if (open_bracket==-1 || close_bracket==-1) {
	/* If there are no both open and close square brackets, there
	 * is no chance to find any URL. */
	return 1;
}
int i=end_pos+1;
int op=0;
int cl=0;
/* First, we look for [[ or ]] */
while (i<buffer->size && op!=2 && cl!=2) {
	if (buffer->buffer[i]==open_bracket) {op++;cl=0;}
	else if (buffer->buffer[i]==close_bracket) {cl++;op=0;}
	else {op=0;cl=0;}
	i++;
}
if (cl==2) {
	/* If we have found ]], it means that the matched sequence is part of
	 * an URL. */
	return 0;
}
if (op!=2) {
	/* If we have reached the end of the buffer without finding [[ */
	return 1;
}
/* We concatenate all the tokens we find before ]] */
while (i+1<buffer->size && (buffer->buffer[i]!=close_bracket || buffer->buffer[i+1]!=close_bracket)) {
	u_strcat(href,tokens->token[buffer->buffer[i]]);
	i++;
}
if (buffer->buffer[i]!=close_bracket || buffer->buffer[i+1]!=close_bracket) {
	/* If we don't find ]], we empty href */
	href[0]='\0';
}
return 1;
}


/**
 * This function takes a string 's' that is the mirror of a Thai left context.
 * For sorting reasons, we must invert s[i] and s[i+1] when s[i] is
 * an initial vowel, because the Thai sort algorithm would behave strangely
 * when applied on raw reversed text. For more information (written in French),
 * see chapter 3.1 in:
 * 
 * Paumier Sébastien, 2003. De la reconnaissance de formes linguistiques à
 * l'analyse syntaxique, Ph.D., Université de Marne-la-Vallée. Downloadable
 * at: http://igm.univ-mlv.fr/LabInfo/theses/
 * 
 * You can also consult (in French too):
 * 
 * Kosawat Krit, 2003. Méthodes de segmentation et d'analyse automatique de
 * textes thaï, Ph.D., Université de Marne-la-Vallée. Downloadable
 * at: http://igm.univ-mlv.fr/LabInfo/theses/
 */
void reverse_initial_vowels_thai(unichar* s) {
int i=0;
unichar c;
while (s[i]!='\0') {
	if (u_is_vowel_thai(s[i]) && s[i+1]!='\0') {
		c=s[i+1];
		s[i+1]=s[i];
		s[i]=c;
		i++;
	}
	i++;
}
}


/**
 * This function reads a concordance index from the file 'concordance' and produces a
 * text file that is stored in 'output'. This file contains the lines of the concordance,
 * but the columns may have been moved according to the sort mode, and the left
 * context is reversed. For instance, if we have a concordance line like:
 * 
 * ABC DEF GHI
 * 
 * where ABC is the left context, DEF is the matched sequence and GHI is the right
 * context. If the sort mode is "CENTER_LEFT", the output will contain the following
 * line:
 * 
 * DEF^CBA^GHI
 * 
 * where ^ stands for the tabulation character. If there are extra information like
 * positions (HTML concordance) or URL (GlossaNet concordance), they are stored at the end
 * of the line:
 * 
 * DEF^CBA^GHI^120 124 5
 * 
 * 
 * 'text' is the "text.cod" file. 'tokens' contains the text tokens.
 * 'option.left_context' and 'option.right_context' specify the lengthes of the
 * contexts to extract. 'expected_result' is used to know if the output is
 * a GlossaNet concordance. 'n_enter_char' is the number of new lines in the text,
 * and 'enter_pos' is an array that contains the positions of these new lines.
 * If 'option.thai_mode' is set to a non zero value, it indicates that the concordance
 * is a Thai one. This information is used to compute correctly the context sizes.
 * 
 * The function returns the number of matches actually written to the output file.
 */
int create_raw_text_concordance(FILE* output,FILE* concordance,FILE* text,struct text_tokens* tokens,
                                int expected_result,
                                int n_enter_char,int* enter_pos,
                                int* token_length,int open_bracket,int close_bracket,
                                struct conc_opt option) {
struct liste_matches* matches;
struct liste_matches* matches_tmp;
unichar left[MAX_CONTEXT_IN_UNITS+1];
unichar middle[MAX_CONTEXT_IN_UNITS+1];
unichar right[MAX_CONTEXT_IN_UNITS+1];
unichar href[MAX_CONTEXT_IN_UNITS+1];
int number_of_matches=0;
int is_a_good_match=1;
int start_pos,end_pos;
int n_units_already_read=0;
int current_origin_in_chars=0;
/* First, we allocate a buffer to read the "text.cod" file */
struct buffer* buffer=new_buffer(1000000);
printf("Loading concordance index...\n");
/* Then we load the concordance index. NULL means that the kind of output
 * doesn't matter. */
matches=load_match_list(concordance,NULL);
/* Then we fill the buffer with the beginning of the text */
buffer->size=fread(buffer->buffer,sizeof(int),buffer->MAXIMUM_BUFFER_SIZE,text);
int start_pos_char;
int end_pos_char;
int current_sentence=0;
int current_sentence_local=0;
int position_in_chars=0;
int position_in_tokens=0;
/* Now we can proceed all the matches, assuming that they are sorted by starting
 * position */
printf("Constructing concordance...\n");
while (matches!=NULL) {
	if (buffer->size==buffer->MAXIMUM_BUFFER_SIZE
		&& ((matches->debut-n_units_already_read)+MAX_CONTEXT_IN_UNITS)>buffer->size) {
		/* If we must change of block... */
		move_buffer_to_position(matches->debut,text,tokens,token_length,buffer,&n_units_already_read,
						&current_origin_in_chars,&current_sentence);
		/* We update the position in characters so that we know how
		 * many characters there are before buffer[0]. We update
		 * the sentence number in the same way. */
		position_in_chars=current_origin_in_chars;
		position_in_tokens=0;
		current_sentence_local=current_sentence;
	}
	/* Here, we are sure that the buffer contains all the tokens we need.
	 * We adjust 'start_pos' and 'end_pos' so that the tokens that compose
	 * the current match are between buffer[start_pos] and buffer[end_pos]. */
	start_pos=matches->debut-n_units_already_read;
	end_pos=matches->fin-n_units_already_read;
	start_pos_char=position_in_chars;
	/* We update the position in characters so that we know how
	 * many characters there are before buffer[start_pos]. We update
	 * the sentence number in the same way. */
	for (int z=position_in_tokens;z<start_pos;z++) {
		start_pos_char=start_pos_char+token_length[buffer->buffer[z]];
		if (buffer->buffer[z]==tokens->SENTENCE_MARKER) {
			current_sentence_local++;
		}
	}
	position_in_chars=start_pos_char;
	position_in_tokens=start_pos;
	end_pos_char=start_pos_char;
	/* We update 'end_pos_char' in the same way */
	for (int z=start_pos;z<=end_pos;z++) {
		end_pos_char=end_pos_char+token_length[buffer->buffer[z]];
	}
	/* Now we extract the 3 parts of the concordance */
	extract_left_context(start_pos,left,tokens,option,token_length,buffer);
	extract_match(start_pos,end_pos,matches->output,middle,tokens,buffer);
	/* To compute the 3rd part (right context), we need to know the length of
	 * the matched sequence in displayable characters. */
	int match_length_in_displayable_chars;
	if (option.thai_mode) {match_length_in_displayable_chars=u_strlen_thai_without_diacritic(middle);}
	else {match_length_in_displayable_chars=u_strlen(middle);}
	/* Then we can compute the right context */
	extract_right_context(end_pos,right,tokens,match_length_in_displayable_chars,
                              option,buffer);
	/* If we must produce a GlossaNet concordance, we look for a URL. After the
	 * function call, 'is_a_good_match' can be set to 0 if the match
	 * was a part of a URL instead of a valid match. */
	if (expected_result==GLOSSANET_) {
		is_a_good_match=extract_href(end_pos,href,tokens,buffer,open_bracket,close_bracket);
	}
	/* We compute the shift due to the new lines that count for 2 characters */
	unichar positions[100];
	char tmp_chars[100];
	/* And we use it to compute the bounds of the matched sequence in characters
	 * from the beginning of the text file. */
	int shift=get_shift(n_enter_char,enter_pos,start_pos);
	start_pos_char=start_pos_char+shift;
	/* The shift value can be different at the end of the match since new lines
	 * can occur inside a match. */
	shift=get_shift(n_enter_char,enter_pos,end_pos);
	end_pos_char=end_pos_char+shift;
	sprintf(tmp_chars,"\t%d %d %d",start_pos_char,end_pos_char,current_sentence_local);
	/* Finally, we copy the sequence bounds and the sentence number into 'positions'. */
	u_strcpy_char(positions,tmp_chars);
	/* Now we save the concordance line to the output file, but only if
	 * it's a valid match. */
	if (is_a_good_match) {
		if (option.sort_mode!=TEXT_ORDER) {
			/* If we must reverse the left context in thai mode,
			 * we must reverse initial vowels with their following consonants. */
			if (option.thai_mode) {
				reverse_initial_vowels_thai(left);
			}
		}
		/* We save the 3 parts of the concordance line according to the sort mode */
		switch(option.sort_mode) {
			case TEXT_ORDER: u_fprints(left,output);u_fprints_char("\t",output);
							u_fprints(middle,output);u_fprints_char("\t",output);
							u_fprints(right,output);
							break;
			case LEFT_CENTER: u_fprints_reverse(left,output);u_fprints_char("\t",output);
							u_fprints(middle,output);u_fprints_char("\t",output);
							u_fprints(right,output);
							break;
			case LEFT_RIGHT: u_fprints_reverse(left,output);u_fprints_char("\t",output);
							u_fprints(right,output);u_fprints_char("\t",output);
							u_fprints(middle,output);
							break;
			case CENTER_LEFT: u_fprints(middle,output);u_fprints_char("\t",output);
							u_fprints_reverse(left,output);u_fprints_char("\t",output);
							u_fprints(right,output);
							break;
			case CENTER_RIGHT: u_fprints(middle,output);u_fprints_char("\t",output);
							u_fprints(right,output);u_fprints_char("\t",output);
							u_fprints_reverse(left,output);
							break;
			case RIGHT_LEFT: u_fprints(right,output);u_fprints_char("\t",output);
							u_fprints_reverse(left,output);u_fprints_char("\t",output);
							u_fprints(middle,output);
							break;
			case RIGHT_CENTER: u_fprints(right,output);u_fprints_char("\t",output);
							u_fprints(middle,output);u_fprints_char("\t",output);
							u_fprints_reverse(left,output);
							break;
		}
		/* And we add the position information */
		u_fprints(positions,output);
		/* And the GlossaNet URL if needed */
		if (expected_result==GLOSSANET_) {
			u_fprints_char("\t",output);
			u_fprints(href,output);
		}
		u_fprints_char("\n",output);
		/* We increase the number of matches actually written to the output */
		number_of_matches++;
	}
	/* Finally, we go on the next match */
	matches_tmp=matches;
	matches=matches->suivant;
	free_liste_matches(matches_tmp);
}
return number_of_matches;
}


/**
 * This function prints the token 'buffer[offset_in_buffer]' to the output.
 * If the token is a space or a line break, which are the same in 'tokens',
 * the 'enter_pos' array is used to decide, whether a space or a line break
 * has to be printed. 'n_enter_char' is the length of the 'enter_pos' array.
 * 'pos_in_enter_pos' is the current position in this array. The function
 * returns the updated current position in the 'pos_in_enter_pos' array.
 */
int fprint_token(FILE* output,struct text_tokens* tokens,long int offset_in_buffer,
				int current_global_position,int n_enter_char,int* enter_pos,
				int pos_in_enter_pos,struct buffer* buffer) {
/* We look for the new line that is closer (but after) to the token to print */
while (pos_in_enter_pos < n_enter_char) {
	if ((current_global_position+offset_in_buffer) < enter_pos[pos_in_enter_pos]) {
		/* We have found the new line that follows the token to print, so
		 * we can stop. */
		break;
	}
	else if ((current_global_position+offset_in_buffer) > enter_pos[pos_in_enter_pos]) {
		/* The current new line is still before the token to print, so we go on */
		pos_in_enter_pos++;
		continue;
	}
	else if ((current_global_position+offset_in_buffer) == enter_pos[pos_in_enter_pos]) {
		/* The token to print is a new line, so we print it and return */
		pos_in_enter_pos++;
		u_fputc((unichar)'\n',output);
		return pos_in_enter_pos;
	}
}
/* The token to print is not a new line, so we print it and return */
u_fprints(tokens->token[buffer->buffer[offset_in_buffer]],output);
return pos_in_enter_pos;
}


/**
 * This function saves the text from the token n° 'current_global_position'
 * to the token n° 'match_start'. The text is printed in the file 'output'.
 * The function returns the updated current position in the 'pos_in_enter_pos'
 * array.
 */
int move_in_text_with_writing(int match_start,FILE* text,struct text_tokens* tokens,
								int current_global_position,FILE* output,
								int n_enter_char,int* enter_pos,int pos_in_enter_pos,
								struct buffer* buffer) {
long int address=current_global_position*sizeof(int);
fseek(text,address,SEEK_SET);
while ((match_start-current_global_position) > buffer->MAXIMUM_BUFFER_SIZE) {
	/* If the distance between current position and final position is larger than
	 * the buffer size, then we read a full buffer. */
	fread(buffer->buffer,sizeof(int),buffer->MAXIMUM_BUFFER_SIZE,text);
	for (address=0;address<buffer->MAXIMUM_BUFFER_SIZE;address++) {
		pos_in_enter_pos=fprint_token(output,tokens,address,current_global_position,
										n_enter_char,enter_pos,pos_in_enter_pos,
										buffer);
	}
	current_global_position=current_global_position+buffer->MAXIMUM_BUFFER_SIZE;
}
buffer->size=fread(buffer->buffer,sizeof(int),(match_start-current_global_position),text);
for (address=0;address<buffer->size;address++) {
	pos_in_enter_pos=fprint_token(output,tokens,address,current_global_position,
									n_enter_char,enter_pos,pos_in_enter_pos,
									buffer);
}
return pos_in_enter_pos;
}


/**
 * This function saves all the text from the token n° 'current_global_position' to
 * the end.
 */
int move_to_end_of_text_with_writing(FILE* text,struct text_tokens* tokens,
									int current_global_position,FILE* output,
									int n_enter_char,int* enter_pos,int pos_in_enter_pos,
									struct buffer* buffer) {
long int address=current_global_position*sizeof(int);
fseek(text,address,SEEK_SET);
while (0!=(buffer->size = fread(buffer->buffer,sizeof(int),buffer->MAXIMUM_BUFFER_SIZE,text))) {
	for (address=0;address<buffer->size;address++) {
		pos_in_enter_pos=fprint_token(output,tokens,address,current_global_position,
										n_enter_char,enter_pos,pos_in_enter_pos,buffer);
	}
}
return pos_in_enter_pos;
}


/**
 * This function loads the "concord.ind" file 'concordance' and uses it
 * to produce a modified version of the original text. The output is saved
 * in a file named 'output_name'. The output is obtained by replacing
 * matched sequences by their associated outputs (note that if there is no
 * output, the matched sequence is deleted). In case of overlapping matches,
 * priority is given to left most one. If 2 matches start at the same position,
 * the longest is preferred. If 2 matches start and end at the same positions,
 * then the first one is arbitrarily preferred.
 */
void create_modified_text_file(FILE* concordance,FILE* text,
                               struct text_tokens* tokens,char* output_name,
                               int n_enter_char,int* enter_pos) {
FILE* output=u_fopen(output_name,U_WRITE);
if (output==NULL) {
	u_fclose(concordance);
	fclose(text);
	fatal_error("Cannot write file %s\n",output_name);
}
struct liste_matches* matches;
struct liste_matches* matches_tmp;
int current_global_position=0;
/* We allocate a buffer to read the tokens of the text */
struct buffer* buffer=new_buffer(1000000);
/* We load the match list */
matches=load_match_list(concordance,NULL);
int pos_in_enter_pos=0;
printf("Merging outputs with text...\n");
while (matches!=NULL) {
	while (matches!=NULL && matches->debut<current_global_position) {
		/* If we must ignore this match because it is overlapping a previous
		 * match */
		matches_tmp=matches;
		matches=matches->suivant;
		free_liste_matches(matches_tmp);
	}
	if (matches!=NULL) {
		/* There, we are sure that we have a valid match to process */
		pos_in_enter_pos=move_in_text_with_writing(matches->debut,text,tokens,
													current_global_position,output,
													n_enter_char,enter_pos,pos_in_enter_pos,
													buffer);
		/* Now, we are sure that the buffer contains all we want */
		if (matches->output!=NULL) {
			u_fprints(matches->output,output);
		}
		current_global_position=matches->fin+1;
		/* We skip to the next match of the list */
		matches_tmp=matches;
		matches=matches->suivant;
		free_liste_matches(matches_tmp);
	}
}
/* Finally, we don't forget to dump all the text that may remain after the
 * last match. */
move_to_end_of_text_with_writing(text,tokens,current_global_position,output,
								n_enter_char,enter_pos,pos_in_enter_pos,buffer);
free_buffer(buffer);
printf("Done.\n");
}


/**
 * This function takes an integer 'a' and an array 't' of size 'n'.
 * It returns the greatest value x so that t[x]<=a.
 */
int find_by_dichotomy(int a,int* t,int n) {
int start_position,middle_position;
if (n==0||t==NULL) {
	error("Empty or NULL array in find_by_dichotomy\n");
	return 0;
}
if (a<t[0]) return 0;
if (a>t[n-1]) return n;
n=n-1;
start_position=0;
while (start_position<=n) {
	middle_position=(start_position+n)/2;
	if (t[middle_position]==a) return middle_position;
	if (t[middle_position]<a) {
		start_position=middle_position+1;
	} else {
		n=middle_position-1;
	}
}
return n+1;
}


/**
 * This function takes the number of new lines in the text ('n_enter_char'),
 * the array 'enter_pos' that contains their positions in tokens and a position
 * 'pos'. It returns the number of new lines that occur before 'pos'.
 */
int get_shift(int n_enter_char,int* enter_pos,int pos) {
int res=find_by_dichotomy(pos,enter_pos,n_enter_char);
return res;
}

