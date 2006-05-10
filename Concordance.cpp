 /*
  * Unitex
  *
  * Copyright (C) 2001-2005 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

int create_raw_text_concordance(FILE*,FILE*,FILE*,struct text_tokens*,int,int,int,int,
								int*,int,int,int*);
void compute_token_length(int*,struct text_tokens*);

void create_modified_text_file(FILE*,FILE*,struct text_tokens*,char*,int,int*);
void write_HTML_header(FILE*,int,char*,char*);
void write_HTML_end(FILE*);
void reverse_initial_vowels_thai(unichar*);
int get_decalage(int,int*,int);





//int buffer[BUFFER_SIZE];
//int BUFFER_LENGTH;
//int N_UNITS_ALLREADY_READ;
int open_bracket=-1;
int close_bracket=-1;
//int origine_courante_char=0;
//int phrase_courante=1;




/**
 * This function builds a concordance from a 'concord.ind' file described
 * by the 'concordance' parameter. 'text' is supposed to represent the 'text.cod'
 * file from which the concordance index was computed. 'tokens' represents the
 * associated 'tokens.txt' file. 'sort_mode' is an integer that represents the sort
 * mode to be used for creating the concordance. This parameter will be ignored
 * if the function must modify the text instead of building a concordance.
 * 'left_context' and 'right_context' specify the length of the contexts in visible
 * characters (for Thai, this number is different from the number of unicode
 * characters because of diacritics). 'fontname' and 'fontsize' are used to
 * set the font that will be used if the output is an HTML file (if not, these
 * parameters will be ignored). 'directory' represents the working directory.
 * 'result_mode' indicates the kind of output that is expected. If it value if
 * "html" or "text", the function will build an HTML or text concordance. If
 * its value is of the form "glossanet=xxx", the result will be an HTML
 * concordance designed for the GlossaNet system (http://glossa.fltr.ucl.ac.be),
 * and "xxx" will be taken as a parameter given to GlossaNet. Any other value
 * will be considered as a file name to use for producing a modified version of
 * the text. 'sort_alphabet' is the name of the "Alphabet_sort.txt" file to use
 * for sorting the lines of the concordance. This parameter will be ignored
 * if the output is a modified text file or if the sort mode is TEXT_ORDER.
 * 'n_enter_char' is the number of new lines in the text, and 'enter_pos' is
 * an array that contains the positions of these new lines. If 'thai_mode' is set
 * to a non zero value, it indicates that the concordance is a Thai one. This
 * information is used to compute correctly the context sizes.
 */
void create_concordance(FILE* concordance,FILE* text,struct text_tokens* tokens,
						int sort_mode,int left_context,int right_context,
						char* fontname,char* fontsize,
						char* directory,char* result_mode,char* sort_alphabet,
						int n_enter_char,int* enter_pos,int thai_mode) {
FILE* out;
FILE* f;
char temp_file_name[2000];
char output_file_name[2000];
char* script_glossanet=NULL;
struct string_hash* glossa_hash=NULL;
int RES;
/* We compute the length of each token */
int* token_length=(int*)malloc(sizeof(int)*tokens->N);
if (token_length==NULL) {fatal_error("Not enough memory in create_concordance\n");}
compute_token_length(token_length,tokens);
/* Then, we see which kind of output is expected */
script_glossanet=strstr(result_mode,"glossanet=");
if (script_glossanet!=NULL) {
	/* If the result parameter starts with "glossanet=", then we make
	 * start the GlossaNet script after the equal sign. */
	script_glossanet=script_glossanet+strlen("glossanet=");
}
if (strcmp(result_mode,"html") && strcmp(result_mode,"text") && script_glossanet==NULL) {
	/* If we have to produced a modified version of the original text, we
	 * do it and return. */
	create_modified_text_file(concordance,text,tokens,result_mode,n_enter_char,enter_pos);
	return;
}
/* If the expected result is a concordance */
if (!strcmp(result_mode,"html")) RES=HTML_;
else if (!strcmp(result_mode,"text")) RES=TEXT_;
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
strcpy(temp_file_name,directory);
strcat(temp_file_name,"concord_.txt");
strcpy(output_file_name,directory);
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
N_MATCHES=create_raw_text_concordance(f,concordance,text,tokens,left_context,right_context,
										RES,n_enter_char,enter_pos,thai_mode,sort_mode,
										token_length);
u_fclose(f);
free(token_length);
/* If necessary, we sort it by invoking the main function of the SortTxt program */
if (sort_mode!=TEXT_ORDER) {
	char** argv;
	argv=(char**)malloc(6*sizeof(char*));
	argv[0]=strdup(" ");
	argv[1]=strdup(temp_file_name);
	argv[2]=strdup("-n");
	int i=3;
	if (strcmp(sort_alphabet,"NULL")) {
		argv[i++]=strdup("-o");
		argv[i++]=strdup(sort_alphabet);
	}
	if (thai_mode) {
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
if (RES!=TEXT_) write_HTML_header(out,N_MATCHES,fontname,fontsize);
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
	switch(sort_mode) {
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
		if (sort_mode!=TEXT_ORDER) {
			/* If the concordance was sorted, the left sequence was reversed, and
			 * then, we have to reverse it again. However, the Thai sort algorithm
			 * requires to modify some vowels. That's why we must apply a special
			 * procedure if we have a Thai sorted concordance. */
			if (thai_mode) reverse_initial_vowels_thai(left);
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
void write_HTML_header(FILE* f,int number_of_matches,char* fontname,char* fontsize) {
fprintf(f,"<html lang=en>\n");
fprintf(f,"<head>\n");
fprintf(f,"   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
fprintf(f,"   <title>%d match%s</title>\n",number_of_matches,(number_of_matches>1)?"es":"");
fprintf(f,"</head>\n");
fprintf(f,"<body>\n<table border=\"0\" cellpadding=\"0\" width=\"100%%\" style=\"font-family: %s; font-size: %s\">\n",fontname,fontsize);
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
void block_change(int start_pos,FILE* text,struct text_tokens* tokens,int* token_length,
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
 * 
 */
void extract_left_context(int pos,unichar* left,struct text_tokens* tok,
                          int LEFT_CONTEXT_LENGTH,int thai_mode,int* token_length,
                          struct buffer* buffer) {
int i=LEFT_CONTEXT_LENGTH-1;
if (thai_mode) {
   // if we are in thai mode, we must ignore diacritic sign to count "real" printed chars
   i=0;
   int count=0;
   left[0]='\0';
   pos--;
   int l=token_length[buffer->buffer[pos]]-1;
   unichar* s=tok->token[buffer->buffer[pos]];
   while (pos>=0 && count<LEFT_CONTEXT_LENGTH) {
     left[i]=s[l--];
     if (!u_is_to_be_ignored_thai(left[i])) {
        // we increase the char count only we don't have a diacritic mark
        count++;
     }
     i++;
     if (l<0) {
        // if we must change of token
        pos--;
        if (pos>=0) {
           // and if we can
           l=token_length[buffer->buffer[pos]]-1;
           s=tok->token[buffer->buffer[pos]];
        }
     }
   }
   if (count!=LEFT_CONTEXT_LENGTH) {
     while (count++!=LEFT_CONTEXT_LENGTH) {
       left[i++]=' ';
     }
   }
   left[i]='\0';
   u_reverse_string(left);
   return;
}

left[LEFT_CONTEXT_LENGTH]='\0';
pos--;  // we must start on the left of the match
int l=token_length[buffer->buffer[pos]]-1;
unichar* s=tok->token[buffer->buffer[pos]];
while (pos>=0 && i>=0) {
  left[i--]=s[l--];
  if (l<0) {
     // if we must change of token
     pos--;
     if (pos>=0) {
        // and if we can
        l=token_length[buffer->buffer[pos]]-1;
        s=tok->token[buffer->buffer[pos]];
     }
  }
}
if (i>=0) {
   // if we have not completed the string because we are at the beginning of the buffer
   do {
     left[i--]=' ';
   } while (i>=0);
}
}



void extract_match(int start_pos,int end_pos,unichar* output,unichar* middle,
					struct text_tokens* tok,struct buffer* buffer) {
int i,j,k;
unichar* s;
if (output==NULL) {
   // if there is no output, we compute the match from the text
   j=0;
   for (i=start_pos;i<=end_pos;i++) {
     k=0;
     s=tok->token[buffer->buffer[i]];
     while (s[k]!='\0') {
       middle[j++]=s[k++];
     }
   }
   middle[j]='\0';
} else {
   // if there is an output, then the match is the output
   u_strcpy(middle,output);
}
}



void extract_right_context(int pos,unichar* right,struct text_tokens* tok,
                          int RIGHT_CONTEXT_LENGTH,int MATCH_LENGTH,int thai_mode,
                          struct buffer* buffer) {
right[0]='\0';
if (MATCH_LENGTH>=RIGHT_CONTEXT_LENGTH) {
   // we return if we all allready overpassed the right context length with the match
   return;
}
RIGHT_CONTEXT_LENGTH=RIGHT_CONTEXT_LENGTH-MATCH_LENGTH;
int i=0;
int count=0;
pos++;  // we must start on the right of the match
int l=0;
unichar* s=tok->token[buffer->buffer[pos]];
while (pos<buffer->size && count<RIGHT_CONTEXT_LENGTH) {
  right[i]=s[l++];
  if (thai_mode) {
     if (!u_is_to_be_ignored_thai(right[i])) count++;
  } else count++;
  i++;
  if (s[l]=='\0') {
     // if we must change of token
     pos++;
     if (pos<buffer->size) {
        // and if we can
        l=0;
        s=tok->token[buffer->buffer[pos]];
     }
  }
}
if (count<RIGHT_CONTEXT_LENGTH) {
   // if we have not completed the string because we are at the beginning of the buffer
   do {
     right[i++]=' ';
     count++;
   } while (count<RIGHT_CONTEXT_LENGTH);
}
right[i]='\0';
}



int extract_href(int end_pos,unichar* href,struct text_tokens* tok,struct buffer* buffer) {
href[0]='\0';
if (open_bracket==-1 || close_bracket==-1) {
   // if there are no both open and close bracket, there
   // is no chance to find any href
   return 1;
}
int i=end_pos;
int op=0;
int cl=0;
while (i<buffer->size && op!=2 && cl!=2) {
      if (buffer->buffer[i]==open_bracket) {op++;cl=0;}
      else if (buffer->buffer[i]==close_bracket) {cl++;op=0;}
      else {op=0;cl=0;}
      i++;
}
if (cl==2) {
   // if we have found ]], it means that the matches is part of a href
   return 0;
}
if (op!=2) return 1;
while (i<buffer->size && buffer->buffer[i]!=close_bracket) {
   u_strcat(href,tok->token[buffer->buffer[i]]);
   i++;
}
return 1;
}



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



int create_raw_text_concordance(FILE* f,FILE* concor,FILE* text,struct text_tokens* tok,
                                int left_context_length,int right_context_length,int RES,
                                int n_enter_char,int* enter_pos,int thai_mode,
                                int sort_mode,int* token_length) {
struct liste_matches* l;
struct liste_matches* l_tmp;
unichar left[5000];
unichar middle[5000];
unichar right[5000];
unichar href[5000];
int TRANSDUCTION_MODE;
int N=0;
int GOOD_MATCH=1;
int start_pos,end_pos;
int N_UNITS_ALLREADY_READ=0;
int current_origin_in_chars=0;
struct buffer* buffer=new_buffer(11000/*1000000*/);
printf("Loading concordance index...\n");
l=load_match_list(concor,&TRANSDUCTION_MODE);
buffer->size=fread(buffer->buffer,sizeof(int),buffer->MAXIMUM_BUFFER_SIZE,text);
int debut_char;
int fin_char;
int current_sentence=0;
int locale_phrase_courante=0;
int char_pos=0;
int int_pos=0;
printf("Constructing concordance...\n");
while (l!=NULL) {
   if (buffer->size==buffer->MAXIMUM_BUFFER_SIZE && ((l->debut-N_UNITS_ALLREADY_READ)+MAX_CONTEXT_IN_UNITS/*MAX_HTML_CONTEXT*/)>buffer->size) {
      // if we must change of block...
      block_change(l->debut,text,tok,token_length,buffer,&N_UNITS_ALLREADY_READ,&current_origin_in_chars,&current_sentence);
      // on reinitialise char_pos et int_pos
      char_pos=current_origin_in_chars;
      int_pos=0;
      locale_phrase_courante=current_sentence;
   }
   // now, we are sure that the buffer contains all we want
   start_pos=l->debut-N_UNITS_ALLREADY_READ;
   end_pos=l->fin-N_UNITS_ALLREADY_READ;
   // we extract the 3 parts of the concordance
   debut_char=char_pos;
   for (int z=int_pos;z<start_pos;z++) {
      debut_char=debut_char+token_length[buffer->buffer[z]];
      if (buffer->buffer[z]==tok->SENTENCE_MARKER) locale_phrase_courante++;
   }
   char_pos=debut_char;
   int_pos=start_pos;
   fin_char=debut_char;
   for (int z=start_pos;z<=end_pos;z++) {
      fin_char=fin_char+token_length[buffer->buffer[z]];
   }
   extract_left_context(start_pos,left,tok,left_context_length,thai_mode,token_length,buffer);
   extract_match(start_pos,end_pos,l->output,middle,tok,buffer);
   unichar indice[100];
   char ch[100];
   // ce bloc sert à calculer le decalage du aux retours à la ligne qui compte 2 caracteres
   int decalage=get_decalage(n_enter_char,enter_pos,start_pos);
   debut_char=debut_char+decalage;
   fin_char=fin_char+decalage;
   // fin decalage
   sprintf(ch,"\t%d %d %d",debut_char,fin_char,locale_phrase_courante);
   u_strcpy_char(indice,ch);
   int MATCH_WIDTH;
   if (thai_mode) MATCH_WIDTH=u_strlen_thai_without_diacritic(middle);
   else MATCH_WIDTH=u_strlen(middle);
   extract_right_context(end_pos,right,tok,right_context_length,MATCH_WIDTH,thai_mode,buffer);
   if (RES==GLOSSANET_) GOOD_MATCH=extract_href(end_pos,href,tok,buffer);
   if (GOOD_MATCH) {
      if (sort_mode!=TEXT_ORDER) {
         // if we must reverse the left context in thai mode,
         // we must reverse initial vowels with their following consonants
         if (thai_mode) reverse_initial_vowels_thai(left);
      }
      // and we save them according to the sort mode
      switch(sort_mode) {
        case TEXT_ORDER: {u_fprints(left,f);u_fprints_char("\t",f);
                          u_fprints(middle,f);u_fprints_char("\t",f);
                          u_fprints(right,f);u_fprints(indice,f);
                          if (RES==GLOSSANET_) {
                             u_fprints_char("\t",f);u_fprints(href,f);
                          }
                          u_fprints_char("\n",f);} break;
        case LEFT_CENTER: {u_fprints_reverse(left,f);u_fprints_char("\t",f);
                           u_fprints(middle,f);u_fprints_char("\t",f);
                           u_fprints(right,f);u_fprints(indice,f);
                           if (RES==GLOSSANET_) {
                              u_fprints_char("\t",f);u_fprints(href,f);
                           }
                           u_fprints_char("\n",f);} break;
        case LEFT_RIGHT: {u_fprints_reverse(left,f);u_fprints_char("\t",f);
                          u_fprints(right,f);u_fprints_char("\t",f);
                          u_fprints(middle,f);u_fprints(indice,f);
                          if (RES==GLOSSANET_) {
                             u_fprints_char("\t",f);u_fprints(href,f);
                          }
                          u_fprints_char("\n",f);} break;
        case CENTER_LEFT: {u_fprints(middle,f);u_fprints_char("\t",f);
                           u_fprints_reverse(left,f);u_fprints_char("\t",f);
                           u_fprints(right,f);u_fprints(indice,f);
                           if (RES==GLOSSANET_) {
                              u_fprints_char("\t",f);u_fprints(href,f);
                           }
                           u_fprints_char("\n",f);} break;
        case CENTER_RIGHT: {u_fprints(middle,f);u_fprints_char("\t",f);
                            u_fprints(right,f);u_fprints_char("\t",f);
                            u_fprints_reverse(left,f);u_fprints(indice,f);
                            if (RES==GLOSSANET_) {
                               u_fprints_char("\t",f);u_fprints(href,f);
                            }
                            u_fprints_char("\n",f);} break;
        case RIGHT_LEFT: {u_fprints(right,f);u_fprints_char("\t",f);
                          u_fprints_reverse(left,f);u_fprints_char("\t",f);
                          u_fprints(middle,f);u_fprints(indice,f);
                          if (RES==GLOSSANET_) {
                             u_fprints_char("\t",f);u_fprints(href,f);
                          }
                          u_fprints_char("\n",f);} break;
        case RIGHT_CENTER: {u_fprints(right,f);u_fprints_char("\t",f);
                            u_fprints(middle,f);u_fprints_char("\t",f);
                            u_fprints_reverse(left,f);u_fprints(indice,f);
                            if (RES==GLOSSANET_) {
                               u_fprints_char("\t",f);u_fprints(href,f);
                            }
                            u_fprints_char("\n",f);} break;
      }
      // counting matches
      N++;
   }
   // and skipping to the next element of the list
   l_tmp=l;
   l=l->suivant;
   free_liste_matches(l_tmp);
}
return N;
}


//
// prints a token to res
// if the token is a space or a line break, which are the same in struct text_tokens,
// enter_pos is used to decide, whether a space or a line break has to be printed
//
inline int fprint_token (FILE* res,
                        struct text_tokens* tok, long int adresse, int current_global_position, 
                        int n_enter_char, int* enter_pos, int pos_in_enter_pos,
                        struct buffer* buffer) {
  while (pos_in_enter_pos < n_enter_char) {
    if ((current_global_position+adresse) < enter_pos[pos_in_enter_pos]) {
      break;
    }
    else if ((current_global_position+adresse) > enter_pos[pos_in_enter_pos]) {
      pos_in_enter_pos++;
      continue;
    }
    else if ((current_global_position+adresse) == enter_pos[pos_in_enter_pos]) {
      pos_in_enter_pos++;
      u_fputc((unichar)'\n',res);
      return pos_in_enter_pos;
    }
  }
  u_fprints(tok->token[buffer->buffer[adresse]],res);
  return pos_in_enter_pos;
}



//
// this function saves the text from the token n° current_global_position to the n° match_start
//
int move_in_text_with_writing(int match_start, FILE* text, struct text_tokens* tok,
                              int current_global_position, FILE* res,
                              int n_enter_char, int* enter_pos, int pos_in_enter_pos,
                              struct buffer* buffer) {
  long int adresse = (current_global_position)*sizeof(int);
  fseek(text, adresse, SEEK_SET);
  while ((match_start-current_global_position) > buffer->MAXIMUM_BUFFER_SIZE) {
    // if the distance between current position and final position is larger than
    // buffer size, we read a full buffer
    fread(buffer->buffer,sizeof(int),buffer->MAXIMUM_BUFFER_SIZE,text);
    for (adresse = 0; adresse < buffer->MAXIMUM_BUFFER_SIZE; adresse++) {
      pos_in_enter_pos = fprint_token(res, tok, adresse, current_global_position,
                                      n_enter_char, enter_pos, pos_in_enter_pos,
                                      buffer);
    }
    current_global_position = current_global_position+buffer->MAXIMUM_BUFFER_SIZE;
  }
  buffer->size = fread(buffer->buffer, sizeof(int), (match_start-current_global_position), text);
  for (adresse=0; adresse < buffer->size; adresse++) {
    pos_in_enter_pos = fprint_token(res, tok, adresse, current_global_position,
                                    n_enter_char, enter_pos, pos_in_enter_pos,
                                    buffer);
  }
  return pos_in_enter_pos;
}



//
// this function saves the text from the token n° current_global_position to the end
//
int move_to_end_of_text_with_writing(FILE* text, struct text_tokens* tok,
                                     int current_global_position, FILE* res,
                                     int n_enter_char, int* enter_pos, int pos_in_enter_pos,
                                     struct buffer* buffer) {
  long int adresse=(current_global_position)*sizeof(int);
  fseek(text,adresse,SEEK_SET);
  while (0 != (buffer->size = fread(buffer->buffer,sizeof(int),buffer->MAXIMUM_BUFFER_SIZE,text))) {
    for (adresse=0; adresse < buffer->size; adresse++) {
      pos_in_enter_pos = fprint_token(res, tok, adresse, current_global_position,
                                      n_enter_char, enter_pos, pos_in_enter_pos,buffer);
      
    }
  }
  return pos_in_enter_pos;
}




void create_modified_text_file(FILE* concord, FILE* text,
                          struct text_tokens* tok, char* result_name,
                          int n_enter_char, int* enter_pos) {
  FILE* res=u_fopen(result_name,U_WRITE);
  if (res==NULL) {
    fprintf(stderr,"Cannot write file %s\n",result_name);
    u_fclose(concord);
    fclose(text);
    exit(1);
  }
  struct liste_matches* l;
  struct liste_matches* l_tmp;
  int current_global_position;
  struct buffer* buffer=new_buffer(1000000);
  l=load_match_list(concord,&current_global_position);
  current_global_position=0;
  int pos_in_enter_pos=0;
  printf("Merging outputs with text...\n");
  while (l!=NULL) {
    while (l!=NULL && l->debut<current_global_position) {
      // if we must ignore this match
      l_tmp=l;
      l=l->suivant;
     free_liste_matches(l_tmp);
    }
    if (l!=NULL) {
      // there, we are sure that we have a valid match to process
      pos_in_enter_pos = move_in_text_with_writing(l->debut,text,tok,
                                                   current_global_position,res,
                                                   n_enter_char,enter_pos,pos_in_enter_pos,
                                                   buffer);
      // now, we are sure that the buffer contains all we want
      if (l->output!=NULL) {
        u_fprints(l->output,res);
      }
      current_global_position=l->fin+1;
      // and skipping to the next element of the list
      l_tmp=l;
      l=l->suivant;
      free_liste_matches(l_tmp);
    }
  }
  pos_in_enter_pos = move_to_end_of_text_with_writing(text,tok,
                                                      current_global_position,res,
                                                      n_enter_char,enter_pos,pos_in_enter_pos,
                                                      buffer);
  free_buffer(buffer);
  printf("Done.\n");
}




int trouver_par_dichotomie(int a,int* t,int n) {
int debut,milieu;
if (n==0||t==NULL) return 0;
if (a<t[0]) return 0;
if (a>t[n-1]) return n;
n=n-1;
debut=0;
while (debut<=n) {
  milieu=(debut+n)/2;
  if (t[milieu]==a) return milieu;
  if (t[milieu]<a) {
    debut=milieu+1;
  }
  else {
    n=milieu-1;
  }
}
return n+1;
}



//
// cette fonction prend le nombre de retours à la ligne, le tableau des positions
// des retours à la ligne et une position; elle renvoie le nombre de retours à la ligne
// se trouvant avant cette position
//
int get_decalage(int n_enter_char,int* enter_pos,int start_pos) {
int res=trouver_par_dichotomie(start_pos,enter_pos,n_enter_char);
return res;
}

