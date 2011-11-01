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

#include <stdlib.h>
#include "Grf_lib.h"
#include "Error.h"
#include "Ustring.h"


/**
 * Creates an empty grf object.
 */
Grf* new_Grf() {
Grf* grf=(Grf*)malloc(sizeof(Grf));
if (grf==NULL) fatal_alloc_error("new_Grf");
grf->size[0]='\0';
grf->font[0]='\0';
grf->ofont[0]='\0';
grf->bcolor[0]='\0';
grf->fcolor[0]='\0';
grf->acolor[0]='\0';
grf->scolor[0]='\0';
grf->ccolor[0]='\0';
grf->dboxes[0]='\0';
grf->dframe[0]='\0';
grf->ddate[0]='\0';
grf->dfile[0]='\0';
grf->ddir[0]='\0';
grf->drig[0]='\0';
grf->drst[0]='\0';
grf->fits[0]='\0';
grf->porient[0]='\0';
grf->metadata=new_vector_ptr(1);
grf->n_states=0;
grf->states=NULL;
return grf;
}


/**
 * Creates an empty grf state.
 */
GrfState* new_GrfState() {
GrfState* s=(GrfState*)malloc(sizeof(GrfState));
if (s==NULL) fatal_alloc_error("new_GrfState");
s->box_content=NULL;
s->transitions=new_vector_int();
s->rank=-1;
s->box_number=-1;
return s;
}


/**
 * Creates a grf state.
 */
GrfState* new_GrfState(const unichar* content,int x,int y,int rank,int box_number) {
GrfState* s=new_GrfState();
s->box_content=u_strdup(content);
s->x=x;
s->y=y;
s->box_number=box_number;
s->rank=rank;
return s;
}


/**
 * Creates a grf state.
 */
GrfState* new_GrfState(const char* content,int x,int y,int rank,int box_number) {
GrfState* s=new_GrfState();
s->box_content=u_strdup(content);
s->x=x;
s->y=y;
s->box_number=box_number;
s->rank=rank;
return s;
}


/**
 * Frees all the memory associated to the given grf state.
 */
void free_GrfState(GrfState* s) {
if (s==NULL) return;
free(s->box_content);
free_vector_int(s->transitions);
free(s);
}


/**
 * Frees all the memory associated to the given grf.
 */
void free_Grf(Grf* grf) {
if (grf==NULL) return;
for (int i=0;i<grf->n_states;i++) {
	free_GrfState(grf->states[i]);
}
free_vector_ptr(grf->metadata,free);
free(grf->states);
free(grf);
}


/**
 * Adds the given state to the given grf, returning the state index.
 */
int add_GrfState(Grf* grf,GrfState* s) {
if (grf==NULL || s==NULL) {
	fatal_error("Unexpected NULL error in add_GrfState\n");
}
int n=grf->n_states;
(grf->n_states)++;
grf->states=(GrfState**)realloc(grf->states,grf->n_states*sizeof(GrfState*));
if (grf->states==NULL) {
	fatal_alloc_error("add_GrfState");
}
grf->states[n]=s;
return n;
}


/**
 * Reads a line and stores it into the destination string. Returns 1
 * in case of success; 0 otherwise.
 */
static int read_grf_header_line(U_FILE* f,Ustring* line,unichar* dest) {
if (EOF==readline(line,f)) return 0;
if (line->len > GRF_HEADER_LINE_SIZE) return 0;
u_strcpy(dest,line->str);
return 1;
}


/**
 * Reads a line containing a number and stores it into *n. Returns 1
 * in case of success; 0 otherwise.
 */
static int read_grf_n_states(U_FILE* f,Ustring* line,int *n) {
if (EOF==readline(line,f)) return 0;
return 1==u_sscanf(line->str,"%d",n);
}


/**
 * Reads a line containing a grf state. Returns 1 in case of success;
 * 0 otherwise.
 */
static int read_grf_state(U_FILE* f,Ustring* line,int n,Grf* grf) {
if (EOF==readline(line,f) || line->len < 2) return 0;
unsigned int pos=0;
if (line->str[pos]=='s') {
	/* Old stuff: s at line start used to mean that the box was selected.
	 * We can ignore this */
	pos++;
}
if (line->str[pos]!='"') return 0;
unsigned int start_pos=pos;
pos++;
while (pos<line->len && line->str[pos]!='"') {
	if (line->str[pos]=='\\') pos++;
	pos++;
}
if (++pos>=line->len) {
	/* Reached the end of line ? It's an error. We use a ++ since
	 * a valid grf line should always have a space after the box content */
	return 0;
}
unichar c=line->str[pos];
if (c!=' ') return 0;
line->str[pos]='\0';
grf->states[n]=new_GrfState();
grf->states[n]->box_number=n;
grf->states[n]->box_content=u_strdup(line->str+start_pos);
pos++;
int shift;
int n_transitions;
if (3!=u_sscanf(line->str+pos,"%d%d%d%n",
		&(grf->states[n]->x),
		&(grf->states[n]->y),
		&n_transitions,
		&shift)) return 0;
pos=pos+shift;
int dest;
for (int i=0;i<n_transitions;i++) {
	if (1!=u_sscanf(line->str+pos,"%d%n",
			&dest,
			&shift)) return 0;
	vector_int_add(grf->states[n]->transitions,dest);
	pos=pos+shift;
}
return 1;
}


/**
 * Reads lines until a # line is found. Each line is supposed to be of the form:
 *
 *   key=value
 *
 * Those key/value pairs are stored in the grf's metadata. No interpretation
 * is made of key/value contents.
 * Returns 1 in case of success, 0 otherwise (no # line is found,
 * line with no =, or several lines with the same key).
 */
static int read_metadata(Ustring* line,U_FILE* f,Grf* grf) {
while (EOF!=readline(line,f)) {
	if (!u_strcmp(line->str,"#")) return 1;
	unichar* value=u_strchr(line->str,'=');
	if (value==NULL) return 0;
	*value='\0';
	value++;
	for (int i=0;i<grf->metadata->nbelems;i=i+2) {
		if (!u_strcmp((unichar*)(grf->metadata->tab[i]),line->str)) {
			return 0;
		}
	}
	vector_ptr_add(grf->metadata,u_strdup(line->str));
	vector_ptr_add(grf->metadata,u_strdup(value));
}
return 0;
}


/**
 * Loads and returns a grf file, or NULL in case of error.
 */
Grf* load_Grf(const VersatileEncodingConfig* vec,const char* name) {
U_FILE* f=u_fopen(vec,name,U_READ);
if (f==NULL) return NULL;
Ustring* line=new_Ustring();
Grf* grf=new_Grf();
if (EOF==readline(line,f)/* || u_strcmp(line->str,"#Unigraph")*/) {
	/* The test has been removed because of backward compatibility problems
	 * with old graphs starting with old header lines */
	goto error;
}
if (!read_grf_header_line(f,line,grf->size)) goto error;
if (!read_grf_header_line(f,line,grf->font)) goto error;
if (!read_grf_header_line(f,line,grf->ofont)) goto error;
if (!read_grf_header_line(f,line,grf->bcolor)) goto error;
if (!read_grf_header_line(f,line,grf->fcolor)) goto error;
if (!read_grf_header_line(f,line,grf->acolor)) goto error;
if (!read_grf_header_line(f,line,grf->scolor)) goto error;
if (!read_grf_header_line(f,line,grf->ccolor)) goto error;
if (!read_grf_header_line(f,line,grf->dboxes)) goto error;
if (!read_grf_header_line(f,line,grf->dframe)) goto error;
if (!read_grf_header_line(f,line,grf->ddate)) goto error;
if (!read_grf_header_line(f,line,grf->dfile)) goto error;
if (!read_grf_header_line(f,line,grf->ddir)) goto error;
if (!read_grf_header_line(f,line,grf->drig)) goto error;
if (!read_grf_header_line(f,line,grf->drst)) goto error;
if (!read_grf_header_line(f,line,grf->fits)) goto error;
if (!read_grf_header_line(f,line,grf->porient)) goto error;
if (!read_metadata(line,f,grf)) goto error;
if (!read_grf_n_states(f,line,&(grf->n_states))) goto error;
grf->states=(GrfState**)calloc(grf->n_states,sizeof(GrfState*));
if (grf->states==NULL) {
	fatal_alloc_error("load_Grf");
}
for (int i=0;i<grf->n_states;i++) {
	if (!read_grf_state(f,line,i,grf)) {
		goto error;
	}
}
goto end;
error:
free_Grf(grf);
grf=NULL;
end:
free_Ustring(line);
u_fclose(f);
return grf;
}


/**
 * Saves the given grf to the given file.
 */
void save_Grf(U_FILE* f,Grf* grf) {
u_fprintf(f,"#Unigraph\n");
u_fprintf(f,"%S\n",grf->size);
u_fprintf(f,"%S\n",grf->font);
u_fprintf(f,"%S\n",grf->ofont);
u_fprintf(f,"%S\n",grf->bcolor);
u_fprintf(f,"%S\n",grf->fcolor);
u_fprintf(f,"%S\n",grf->acolor);
u_fprintf(f,"%S\n",grf->scolor);
u_fprintf(f,"%S\n",grf->ccolor);
u_fprintf(f,"%S\n",grf->dboxes);
u_fprintf(f,"%S\n",grf->dframe);
u_fprintf(f,"%S\n",grf->ddate);
u_fprintf(f,"%S\n",grf->dfile);
u_fprintf(f,"%S\n",grf->ddir);
u_fprintf(f,"%S\n",grf->drig);
u_fprintf(f,"%S\n",grf->drst);
u_fprintf(f,"%S\n",grf->fits);
u_fprintf(f,"%S\n",grf->porient);
for (int i=0;i<grf->metadata->nbelems;i=i+2) {
	u_fprintf(f,"%S=%S\n",grf->metadata->tab[i],grf->metadata->tab[i+1]);
}
u_fprintf(f,"#\n");
u_fprintf(f,"%d\n",grf->n_states);
for (int i=0;i<grf->n_states;i++) {
	u_fprintf(f,"%S %d %d %d ",grf->states[i]->box_content,grf->states[i]->x,grf->states[i]->y,
			grf->states[i]->transitions->nbelems);
	for (int j=0;j<grf->states[i]->transitions->nbelems;j++) {
		u_fprintf(f,"%d ",grf->states[i]->transitions->tab[j]);
	}
	u_fprintf(f,"\n");
}
}


/**
 * Returns a copy of the given grf state.
 */
GrfState* cpy_grf_state(GrfState* s) {
if (s==NULL) return NULL;
GrfState* res=new_GrfState();
res->box_content=u_strdup(s->box_content);
res->x=s->x;
res->y=s->y;
res->box_number=s->box_number;
vector_int_copy(res->transitions,s->transitions);
return res;
}


/**
 * Copies src header into dst.
 */
void cpy_grf_header(Grf* dst,Grf* src) {
u_strcpy(dst->size,src->size);
u_strcpy(dst->font,src->font);
u_strcpy(dst->ofont,src->ofont);
u_strcpy(dst->bcolor,src->bcolor);
u_strcpy(dst->fcolor,src->fcolor);
u_strcpy(dst->acolor,src->acolor);
u_strcpy(dst->scolor,src->scolor);
u_strcpy(dst->ccolor,src->ccolor);
u_strcpy(dst->dboxes,src->dboxes);
u_strcpy(dst->dframe,src->dframe);
u_strcpy(dst->ddate,src->ddate);
u_strcpy(dst->dfile,src->dfile);
u_strcpy(dst->ddir,src->ddir);
u_strcpy(dst->drig,src->drig);
u_strcpy(dst->drst,src->drst);
u_strcpy(dst->fits,src->fits);
u_strcpy(dst->porient,src->porient);
for (int i=0;i<src->metadata->nbelems;i++) {
	vector_ptr_add(dst->metadata,u_strdup((unichar*)(src->metadata->tab[i])));
}
}


/**
 * Copies src graph states into dst, freeing previous state array, if any.
 */
void cpy_grf_states(Grf* dst,Grf* src) {
if (dst->states!=NULL)  {
	for (int i=0;i<dst->n_states;i++) {
		free_GrfState(dst->states[i]);
	}
	free(dst->states);
}
dst->n_states=src->n_states;
dst->states=(GrfState**)malloc(dst->n_states*sizeof(GrfState*));
if (dst->states==NULL) fatal_alloc_error("cmp_grf_states");
for (int i=0;i<dst->n_states;i++) {
	dst->states[i]=cpy_grf_state(src->states[i]);
}
}


/**
 * Returns a copy of the given grf.
 */
Grf* dup_Grf(Grf* src) {
Grf* dst=new_Grf();
cpy_grf_header(dst,src);
cpy_grf_states(dst,src);
return dst;
}


/**
 * Reads everything from content until the stop char is found and copies it
 * into tmp. Returns 1 in case of success, 0 if the end of string is found.
 */
static int read_sequence(unichar stop,unichar* content,int *pos,Ustring* tmp) {
while (content[*pos]!='\0' && content[*pos]!=stop) {
	if (content[*pos]=='\\') {
		u_strcat(tmp,'\\');
		(*pos)++;
		if (content[*pos]=='\0') return 0;
	}
	u_strcat(tmp,content[*pos]);
	(*pos)++;
}
if (content[*pos]==stop) {
	(*pos)++;
	u_strcat(tmp,stop);
	return 1;
}
return 0;
}


/**
 * Reads everything from content until \" is found and copies it
 * into tmp. Returns 1 in case of success, 0 if the end of string is found.
 */
static int read_quoted_sequence(unichar* content,int *pos,Ustring* tmp) {
int state=0;
while (content[*pos]!='\0') {
	unichar c=content[*pos];
	u_strcat(tmp,c);
	switch(state) {
	case 0: {
		if (c=='\\') state=1;
		break;
	}
	case 1: {
		if (c=='"') {
			(*pos)++;
			return 1;
		}
		if (c=='\\') state=2;
		else state=0;
		break;
	}
	case 2: {
		if (c=='\\') state=3;
		else state=0;
		break;
	}
	case 3: {
		state=0;
		break;
	}
	}
	(*pos)++;
}
return 0;
}


/**
 * Reads everything until a / or a box line separator (+) has been found.
 * The read string is added to the given vector and *pos is updated.
 * Returns 1 in case of success, or 0 if any syntax error in found in
 * the content string.
 */
static int read_box_line(vector_ptr* v,unichar* content,int *pos,Ustring* tmp) {
empty(tmp);
while (content[*pos]!='\0' && content[*pos]!='/' && content[*pos]!='+') {
	switch (content[*pos]) {
	case '<': if (!read_sequence('>',content,pos,tmp)) return 0; break;
	case '{': if (!read_sequence('}',content,pos,tmp)) return 0; break;
	case '\\': {
		u_strcat(tmp,'\\');
		(*pos)++;
		if (content[*pos]=='\0') {
			return 0;
		}
		u_strcat(tmp,content[*pos]);
		(*pos)++;
		if (content[(*pos)-1]=='"' && ((*pos)-3<0 || content[(*pos)-3]!='\\')) {
			/* If we have just read \" (and not \\\") then the box line contains
			 * a double quoted sequence that we have to read */
			if (!read_quoted_sequence(content,pos,tmp)) return 0;
		}
		break;
	}
	default: u_strcat(tmp,content[*pos]); (*pos)++; break;
	}
}
vector_ptr_add(v,u_strdup(tmp->str));
if (content[*pos]=='+') {
	(*pos)++;
}
return 1;
}


static int is_variable_char(unichar c) {
return ((c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9') || c=='_');
}


/**
 * Returns 1 if s is a start or end of variable
 */
static int is_variable_box(unichar* s)  {
int pos=0;
if (s[pos++]!='$') return 0;
if (s[pos]=='|') pos++;
while (is_variable_char(s[pos])) pos++;
if (!is_variable_char(s[pos-1])) {
	/* Empty variable  name ? */
	return 0;
}
if (s[pos]!='(' && s[pos]!=')') return 0;
return s[pos+1]=='\0';
}


/**
 * Returns a unichar* vector corresponding to raw box line contents.
 * No protected character has been unprotected, so that the
 * concatenation of lines with + separation produces exactly
 * the same string as content, once the surrounding double quotes removed. If there is a box output
 * introduced by /, it is stored like a normal box line, but with
 * its starting / that will indicate that it is an output.
 *
 * Returns NULL in case of error.
 */
vector_ptr* tokenize_box_content(unichar* content) {
if (content==NULL || content[0]!='"') return NULL;
vector_ptr* v=new_vector_ptr();
int l=u_strlen(content);
/* We remove the ending double quote */
content[l-1]='\0';
if (!u_strcmp(content+1,"$<") || !u_strcmp(content+1,"$>")
	|| !u_strcmp(content+1,"$[") || !u_strcmp(content+1,"$![") || !u_strcmp(content+1,"$]")
	|| is_variable_box(content+1)) {
	vector_ptr_add(v,u_strdup(content+1));
	content[l-1]='"';
	return v;
}
Ustring* s=new_Ustring();
int pos=1;
while (content[pos]!='\0') {
	if (content[pos]=='/') {
		/* If we have found the output, we can copy it rawly and finish */
		vector_ptr_add(v,u_strdup(content+pos));
		break;
	}
	if (!read_box_line(v,content,&pos,s)) {
		free_Ustring(s);
		free_vector_ptr(v,free);
		content[l-1]='"';
		return NULL;
	}
}
content[l-1]='"';
free_Ustring(s);
return v;
}



/**
 * Computes and returns g's reverse transitions.
 */
ReverseTransitions* compute_reverse_transitions(Grf* g) {
ReverseTransitions* reverse=(ReverseTransitions*)malloc(sizeof(ReverseTransitions));
if (reverse==NULL) {
	fatal_alloc_error("compute_reverse_transitions");
}
reverse->n=g->n_states;
reverse->t=(vector_int**)malloc(g->n_states*sizeof(vector_int*));
if (reverse->t==NULL) {
	fatal_alloc_error("compute_reverse_transitions");
}
for (int i=0;i<g->n_states;i++) {
	reverse->t[i]=new_vector_int(1);
}
for (int i=0;i<g->n_states;i++) {
	GrfState* s=g->states[i];
	for (int j=0;j<s->transitions->nbelems;j++) {
		vector_int_add(reverse->t[s->transitions->tab[j]],i);
	}
}
return reverse;
}


void free_ReverseTransitions(ReverseTransitions* r) {
if (r==NULL) return;
for (int i=0;i<r->n;i++) {
	free_vector_int(r->t[i]);
}
free(r->t);
free(r);
}


/**
 * Returns 1 if a and b have same outgoing transitions; 0 otherwise.
 */
int have_same_outgoing_transitions(GrfState* a,GrfState* b) {
return vector_int_equals_disorder(a->transitions,b->transitions);
}


/**
 * Returns 1 if states #a and #b have same outgoing transitions; 0 otherwise.
 */
int have_same_outgoing_transitions(Grf* grf,int a,int b) {
return have_same_outgoing_transitions(grf->states[a],grf->states[b]);
}


/**
 * Returns 1 if states #a and #b have same incoming transitions; 0 otherwise.
 */
int have_same_incoming_transitions(int a,int b,ReverseTransitions* r) {
return vector_int_equals_disorder(r->t[a],r->t[b]);
}


/**
 * Returns 1 if states #a and #b have same incoming and outgoing transitions;
 * 0 otherwise.
 */
int have_same_transitions(Grf* grf,int a,int b,ReverseTransitions* r) {
return have_same_outgoing_transitions(grf,a,b)
	&& have_same_incoming_transitions(a,b,r);
}


