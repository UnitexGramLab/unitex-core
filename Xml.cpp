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


#include "Xml.h"
#include "HTMLCharacters.h"
#include "Alphabet.h"
#include "Ustring.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

int skip_tag(U_FILE* f,U_FILE* f_out,int *pos,int *new_pos,vector_offset* offsets,
		UnxmlizeOpts* options,unichar* bastien[],U_FILE* f_bastien);
int decode_html_char(U_FILE* f,U_FILE* f_out,int *pos,int *new_pos,vector_offset* offsets,
		void* html_ctx);


/**
 * Produces a version of the input file removing all xml tags.
 * Returns 1 in case of success; 0 otherwise.
 *
 * If 'html' is non null, special HTML filtering is applied
 * (i.e. skipping script code, replacing any tag by a space).
 */
int unxmlize(U_FILE* input,U_FILE* output,vector_offset* offsets,UnxmlizeOpts* options,
		unichar* bastien[],U_FILE* f_bastien) {
int c;
int pos=0,new_pos=0;
void* html_ctx=init_HTML_character_context();
while ((c=u_fgetc_raw(input))!=EOF) {
	pos++;
	if (c=='<') {
		if (!skip_tag(input,output,&pos,&new_pos,offsets,options,bastien,f_bastien)) {
			free_HTML_character_context(html_ctx);
			return 0;
		}
	}
	else if (c=='&') {
		if (!decode_html_char(input,output,&pos,&new_pos,offsets,html_ctx)) {
			free_HTML_character_context(html_ctx);
			return 0;
		}
	} else {
		u_fputc_raw((unichar)c,output);
		new_pos++;
	}
}
free_HTML_character_context(html_ctx);
return 1;
}


void write_offsets(vector_offset* offsets,int a,int b,int c,int d) {
if (offsets!=NULL) {
	vector_offset_add(offsets,a,b,c,d);
}
}


/**
 * This function is called when <!-- was read and it skips everything
 * until --> has been read.
 */
int skip_comment(U_FILE* f,int *pos) {
int c,state=0;
while (state!=3) {
	c=u_fgetc_raw(f);
	if (c==EOF) return 0;
	(*pos)++;
	switch(state) {
	case 0: {
		if (c=='-') state=1;
		break;
	}
	case 1: {
		if (c=='-') state=2;
		else state=0;
		break;
	}
	case 2: {
		if (c=='-') state=2;
		else if (c=='>') state=3;
		else state=0;
		break;
	}
	}
}
return 1;
}


/**
 * Returns 1 if the string seq can be read from the given file;
 * 0 otherwise.
 */
int read(U_FILE* f,const char* seq) {
while (*seq) {
	if (u_fgetc_raw(f)!=*seq) return 0;
	seq++;
}
return 1;
}


/**
 * Same as 'read', but ignoring case.
 */
int read2(U_FILE* f,const char* seq) {
int c;
while (*seq) {
	c=u_fgetc_raw(f);
	if (u_toupper((unichar)c)!=u_toupper(*seq)) return 0;
	seq++;
}
return 1;
}


/**
 * This function assumes that <![CDATA[ has been read. It will consider
 * all text until ]]> is read. This text will be taken as is, except for
 * &gt; that will be turned into a >.
 */
int skip_cdata(U_FILE* f,U_FILE* f_out,int *pos,int *new_pos,vector_offset* offsets) {
int c,state=0;
while (state!=3) {
	c=u_fgetc_raw(f);
	if (c==EOF) return 0;
	(*pos)++;
	switch (state) {
	case 0: {
		if (c==']') state=1;
		else if (c=='&') state=4;
		else {
			u_fputc_raw((unichar)c,f_out);
			(*new_pos)++;
			state=0;
		}
		break;
	}
	case 1: {
		if (c==']') state=2;
		else if (c=='&') {
			/* To come here, we had read a ] that will not be used as a
			 * part of ]]> so we have to dump this char in the output file */
			state=4;
			u_fputc_raw(']',f_out);
			(*new_pos)++;
		}
		else {
			/* We have to save ]c */
			u_fputc_raw(']',f_out);
			u_fputc_raw((unichar)c,f_out);
			(*new_pos)+=2;
			state=0;
		}
		break;
	}
	case 2: {
		if (c==']') {
			/* This is the third ], we have to save one */
			u_fputc_raw(']',f_out);
			(*new_pos)++;
			state=2;
		}
		else if (c=='&') {
			/* We have ]] to save */
			u_fputc_raw(']',f_out);
			u_fputc_raw(']',f_out);
			(*new_pos)+=2;
			state=4;
		}
		else if (c=='>') state=3;
		else {
			/* We have ]]c to save */
			u_fputc_raw(']',f_out);
			u_fputc_raw(']',f_out);
			u_fputc_raw((unichar)c,f_out);
			(*new_pos)+=3;
			state=0;
		}
		break;
	}
	case 4: {
		if (c==']') {
			/* We have & to save */
			u_fputc_raw('&',f_out);
			(*new_pos)++;
			state=1;
		}
		else if (c=='&') {
			/* We have & to save */
			u_fputc_raw('&',f_out);
			(*new_pos)++;
			state=4;
		}
		else if (c=='g') state=5;
		else {
			/* We have &c to save */
			u_fputc_raw('&',f_out);
			u_fputc_raw((unichar)c,f_out);
			(*new_pos)+=2;
			state=0;
		}
		break;
	}
	case 5: {
		if (c==']') {
			/* We have &g to save */
			u_fputc_raw('&',f_out);
			u_fputc_raw('g',f_out);
			(*new_pos)+=2;
			state=1;
		} else if (c=='&') {
			/* We have &g to save */
			u_fputc_raw('&',f_out);
			u_fputc_raw('g',f_out);
			(*new_pos)+=2;
			state=4;
		} else if (c=='t') state=6;
		else {
			/* We have &g to save */
			u_fputc_raw('&',f_out);
			u_fputc_raw('g',f_out);
			(*new_pos)+=2;
			state=0;
		}
		break;
	}
	case 6: {
		if (c==']') {
			/* We have &gt to save */
			u_fputc_raw('&',f_out);
			u_fputc_raw('g',f_out);
			u_fputc_raw('t',f_out);
			(*new_pos)+=3;
			state=1;
		}
		else if (c=='&') {
			/* We have &gt to save */
			u_fputc_raw('&',f_out);
			u_fputc_raw('g',f_out);
			u_fputc_raw('t',f_out);
			(*new_pos)+=3;
			state=4;
		} else if (c==';') {
			/* We have to replace &gt; by > */
			u_fputc_raw('>',f_out);
			write_offsets(offsets,(*pos)-4,*pos,*new_pos,(*new_pos)+1);
			(*new_pos)++;
			state=0;
		}
		else {
			/* We have &gt to save */
			u_fputc_raw('&',f_out);
			u_fputc_raw('g',f_out);
			u_fputc_raw('t',f_out);
			(*new_pos)+=3;
			state=0;
		}
		break;
	}
	}
}
/* We have to write ]]> => nothing in the offsets */
write_offsets(offsets,(*pos)-3,*pos,*new_pos,(*new_pos));
return 1;
}


/**
 * This function is called when < was read and it skips everything
 * until > has been read.
 */
int skip_normal_tag(U_FILE* f,int *pos,unichar* bastien[],U_FILE* f_bastien) {
int c;
Ustring* ustr=new_Ustring();
int tag_name_found=(bastien!=NULL)?0:-1;
int tag_index=-1;
int old_pos=*pos;
while ((c=u_fgetc_raw(f))!='>') {
	if (c==EOF) goto err;
	(*pos)++;
	if (c=='"') {
		/* If we have to skip an attribute between double quotes */
		empty(ustr);
		while ((c=u_fgetc_raw(f))!='"') {
			if (c==EOF) goto err;
			u_strcat(ustr,(unichar)c);
			(*pos)++;
		}
		if (tag_name_found==2) {
			tag_name_found=3;
			for (int i=tag_index;i<10;i++) {
				free(bastien[i]);
				bastien[i]=NULL;
			}
			bastien[tag_index]=u_strdup(ustr->str);
			empty(ustr);
			for (int i=0;i<10;i++) {
				if (bastien[i]!=NULL) {
					if (ustr->len!=0) {
						u_strcat(ustr,'-');
					}
					u_strcat(ustr,bastien[i]);
				}
			}
			u_fprintf(f_bastien,"%d %S\n",old_pos,ustr->str);
		}
		(*pos)++;
		continue;
	}
	if (c=='\'') {
		/* If we have to skip an attribute between single quotes */
		while ((c=u_fgetc_raw(f))!='\'') {
			if (c==EOF) goto err;
			(*pos)++;
		}
		(*pos)++;
		continue;
	}
	if (c!=' ' && c!='=') {
		u_strcat(ustr,(unichar)c);
	}
	if (c==' ') {
		if (tag_name_found==0) {
			tag_name_found=1;
			unichar z,foo;
			if (1==u_sscanf(ustr->str,"R%C%C",&z,&foo) && z>='0' && z<='9') {
				tag_index=z-'0';
			}
		}
		empty(ustr);
	}
	if (c=='=') {
		if (tag_name_found==1) {
			if (!u_strcmp(ustr->str,"utxShort")) {
				tag_name_found=2;
			}
		}
		empty(ustr);
	}
}
(*pos)++;
free_Ustring(ustr);
return 1;
err:
free_Ustring(ustr);
return 0;
}


/**
 * This function is called when '<script ' or '<script>' was read and it skips everything
 * until </script> has been read.
 *
 * NOTE: I (S.P.) know that this may bug if a script contains the pattern
 * </script> in a comment or a string, but I don't want to waste time
 * taking into account such a silly situation with a full and painful
 * analysis of script codes.
 */
int skip_script(U_FILE* f,int *pos) {
int c,state=0;
while (state!=9) {
	c=u_fgetc_raw(f);
	if (c==EOF) return 0;
	(*pos)++;
	switch(state) {
		case 0: {
		if (c=='<') state=1;
		break;
	}
	case 1: {
		if (c=='/') state=2;
		else state=0;
		break;
	}
	case 2: {
		if (c=='s' || c=='S') state=3;
		else state=0;
		break;
	}
	case 3: {
		if (c=='c' || c=='C') state=4;
		else state=0;
		break;
	}
	case 4: {
		if (c=='r' || c=='R') state=5;
		else state=0;
		break;
	}
	case 5: {
		if (c=='i' || c=='I') state=6;
		else state=0;
		break;
	}
	case 6: {
		if (c=='p' || c=='P') state=7;
		else state=0;
		break;
	}
	case 7: {
		if (c=='t' || c=='T') state=8;
		else state=0;
		break;
	}
	case 8: {
		if (c=='>') state=9;
		else state=0;
		break;
	}
}
}
return 1;
}


/**
 * This function is called when a '<' has just been read. It is
 * then supposed to skip the tag. If offsets is not NULL, we
 * save the offsets shifts.
 * Returns 1 in case of success; 0 if the tag is malformed.
 */
int skip_tag(U_FILE* f,U_FILE* f_out,int *pos,int *new_pos,vector_offset* offsets,
		UnxmlizeOpts* options,unichar* bastien[],U_FILE* f_bastien) {
int old_pos=(*pos)-1;
long current=ftell(f);
/* We may read a comment */
if (read(f,"!--")) {
	(*pos)+=3;
	if (!skip_comment(f,pos)) {
		error("Invalid comment\n");
		return 0;
	}
	if (options->comments==UNXMLIZE_IGNORE) {
		write_offsets(offsets,old_pos,*pos,*new_pos,*new_pos);
	} else {
		/* We may have to replace comments by a space */
		write_offsets(offsets,old_pos,*pos,*new_pos,(*new_pos)+1);
		(*new_pos)++;
		u_fputc_raw(' ',f_out);
	}
	return 1;
}
fseek(f,current,SEEK_SET);
/* Or a CDATA */
if (read(f,"![CDATA[")) {
	(*pos)+=8;
	write_offsets(offsets,old_pos,*pos,*new_pos,*new_pos);
	if (!skip_cdata(f,f_out,pos,new_pos,offsets)) {
		error("Invalid CDATA\n");
		return 0;
	}
	return 1;
}
fseek(f,current,SEEK_SET);
/* Or a html script code */
if (options->scripts!=UNXMLIZE_DO_NOTHING) {
	int ok=read2(f,"script ");
	if (!ok) {
		fseek(f,current,SEEK_SET);
		ok=read2(f,"script>");
	}
	if (ok) {
		(*pos)+=7;
		if (!skip_script(f,pos)) {
			error("Invalid script code\n");
			return 0;
		}
		if (options->scripts==UNXMLIZE_IGNORE) {
			write_offsets(offsets,old_pos,*pos,*new_pos,*new_pos);
		} else {
			/* We replace script sections by a space */
			write_offsets(offsets,old_pos,*pos,*new_pos,(*new_pos)+1);
			(*new_pos)++;
			u_fputc_raw(' ',f_out);
		}
		return 1;
	}
}
fseek(f,current,SEEK_SET);
/* Or a normal tag */
if (!skip_normal_tag(f,pos,bastien,f_bastien)) {
	error("Invalid xml tag\n");
	return 0;
}
if (options->normal_tags==UNXMLIZE_IGNORE) {
	write_offsets(offsets,old_pos,*pos,*new_pos,*new_pos);
} else {
	/* We replace tags by a space */
	write_offsets(offsets,old_pos,*pos,*new_pos,(*new_pos)+1);
	(*new_pos)++;
	u_fputc_raw(' ',f_out);
}
return 1;
}


/**
 * This function is called when & has been read. It reads an html char
 * like &gt; or &#206;
 */
int decode_html_char(U_FILE* f,U_FILE* f_out,int *pos,int *new_pos,vector_offset* offsets,
		void* html_ctx) {
char tmp[32];
int c,i=0;
while (i<32 && (c=u_fgetc_raw(f))!=';') {
	if (c>255) {
		/* Should not happen with valid html chars */
		tmp[i]='\0';
		error("Invalid html char: &%s%C;\n",tmp,c);
		return 0;
	}
	tmp[i++]=(char)c;
	(*pos)++;
}
if (i==32) {
	/* Should not happen with valid html chars */
	tmp[31]='\0';
	error("Too long HTML character: %s\n",tmp);
	error("This may come from an invalid & found in text instead of &amp;\n");
	return 0;
}
(*pos)++;
tmp[i]='\0';
c=get_HTML_character(html_ctx,tmp,1);
if (c<0) {
	error("Invalid html character: &%s;\n",tmp);
	return 0;
}
u_fputc_raw((unichar)c,f_out);
write_offsets(offsets,(*pos)-(2+i),*pos,*new_pos,(*new_pos)+1);
(*new_pos)++;
return 1;
}

} // namespace unitex
