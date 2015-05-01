/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Offsets.h"
#include "Unicode.h"
#include "Error.h"
#include "Overlap.h"
#include "File.h"
#include "Ustring.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Loads the given offset file. Returns NULL in case of error.
 */
vector_offset* load_offsets(const VersatileEncodingConfig* vec,const char* name) {
U_FILE* f=u_fopen(vec,name,U_READ);
if (f==NULL) return NULL;
int a,b,c,d,n;
vector_offset* res=new_vector_offset();
while ((n=u_fscanf(f,"%d%d%d%d",&a,&b,&c,&d))!=EOF) {
	if (n!=4) {
		fatal_error("Corrupted offset file %s\n",name);
	}
	vector_offset_add(res,a,b,c,d);
}
u_fclose(f);
return res;
}


void save_offsets(U_FILE* f, int a, int b, int c, int d) {
u_fprintf(f, "%d %d %d %d\n", a, b, c, d);
//error("save: %d %d %d %d\n", a, b, c, d);
}


/*static void save_offsets(U_FILE* f,vector_offset* v) {
if (v==NULL) return;
for (int i=0;i<v->nbelems;i++) {
	Offsets o=v->tab[i];
	save_offsets(f,o.old_start,o.old_end,o.new_start,o.new_end);
}
}*/


/*static const char* N(Overlap o) {
switch (o) {
case A_BEFORE_B: return "A_BEFORE_B";
case A_BEFORE_B_OVERLAP: return "A_BEFORE_B_OVERLAP";
case A_INCLUDES_B: return "A_INCLUDES_B";
case A_EQUALS_B: return "A_EQUALS_B";
case B_INCLUDES_A: return "B_INCLUDES_A";
case A_AFTER_B_OVERLAP: return "A_AFTER_B_OVERLAP";
case A_AFTER_B: return "A_AFTER_B";
case -1: return "[-1]";
default: return "OOOOOOOOPS";
}
}*/


/**
 * This function takes two offset arrays:
 *
 * old_offsets=original text => input text
 * new_offsets=input text => output text
 *
 * and it computes and prints in the given file the new offset file:
 *
 * original text => output text
 *
 * If old_offsets is NULL, new offsets are saved in the output file
 * without any modification.
 */
void process_offsets(vector_offset* old_offsets, vector_offset* new_offsets,U_FILE* f) {
if (f==NULL || new_offsets==NULL) return;
if (old_offsets==NULL) {
	/* If there are no old offsets, we just have to save the new ones */
	Offsets x;
	for (int i=0;i<new_offsets->nbelems;i++) {
		x = new_offsets->tab[i];
		save_offsets(f,x.old_start,x.old_end,x.new_start,x.new_end);
	}
	return;
}
int i = 0, j = 0;
/* shift_A is the current shift between original text and input text. It is
 * to be used when B is before A, in order to adjust B's old coordinates */
int shift_A = 0;
/* shift_B is the current shift between input text and output text. It is
 * to be used when A is before B, in order to adjust A's new coordinates */
int shift_B = 0;
int A_includes_B_shift = 0;
int B_includes_A_shift = 0;
Offsets x, y;
Overlap o;
while (j < new_offsets->nbelems) {
	y = x = new_offsets->tab[j++];
	o = (Overlap) -1;
	for (; i < old_offsets->nbelems; i++) {
		x = old_offsets->tab[i];
		/* Note: below, A stands for x's new interval and B stands for y's old interval */
		/*error("\n\nTEST OVERLAP entre %d;%d->%d;%d et %d;%d->%d;%d\n",
				x.old_start,x.old_end,x.new_start,x.new_end,
				y.old_start,y.old_end,y.new_start,y.new_end);*/
		o = overlap(x.new_start, x.new_end, y.old_start, y.old_end);
		//error("o==%s\n",N(o));
		if (o == A_BEFORE_B) {
			save_offsets(f,
						x.old_start,
						x.old_end,
						x.new_start + shift_B,
						x.new_end + shift_B + A_includes_B_shift);
			A_includes_B_shift = 0;
			shift_A = x.old_end - x.new_end;
		} else {
			break;
		}
	}
	if (i < old_offsets->nbelems) {
		/* There may be an overlap */
		if (o == A_INCLUDES_B) {
			/* If A includes B, then we just have to stay on A, note
			 * the shift induced by B and then ignore B */
			//error("A includes B entre %d;%d et %d;%d\n",x.new_start,x.new_end,y.old_start,y.old_end);
			int b_old_length = y.old_end - y.old_start;
			int b_new_length = y.new_end - y.new_start;
			A_includes_B_shift += b_new_length - b_old_length;
			continue;
		}
		if (o == B_INCLUDES_A) {
			/* If B includes A, then we just have to stay on B, note
			 * the shift induced by A and then ignore A */
			//error("B includes A entre %d;%d et %d;%d\n",x.new_start,x.new_end,y.old_start,y.old_end);
			int a_old_length = x.old_end - x.old_start;
			int a_new_length = x.new_end - x.new_start;
			/* Note: it is normal this shift is not computed in the same way than A_includes_B_shift */
			B_includes_A_shift += a_old_length - a_new_length;
			/* j--: we want to stay on B
			 * i++: we want to skip A */
			j--;
			i++;
			continue;
		}
		if (o == A_BEFORE_B_OVERLAP) {
			/* If A overlaps B starting before B, then we have to produce an output */
			//error("A before B overlap entre %d;%d et %d;%d\n",x.new_start,x.new_end,y.old_start,y.old_end);
			int new_shift_A = x.old_end - x.new_end;
			save_offsets(f,
						x.old_start + shift_A,
						y.old_end + shift_A + B_includes_A_shift + new_shift_A,
						x.new_start + shift_B,
						y.new_end + shift_B + A_includes_B_shift);
			A_includes_B_shift = 0;
			B_includes_A_shift = 0;
			shift_A += new_shift_A;
			shift_B = y.new_end - y.old_end;
			/* We skip A */
			i++;
			continue;
		}
		if (o == A_AFTER_B_OVERLAP) {
			/* If A overlaps B starting after B, then we have to produce an output */
			//error("A after B overlap entre %d;%d et %d;%d\n",x.new_start,x.new_end,y.old_start,y.old_end);
			int new_shift_B = y.new_end - y.old_end;
			//error("new shift B = %d\n",new_shift_B);
			
// Bugfix 2015/05/01, svn revision 3828
// remove the + shift_B on the two last parameters of save_offsets
// do a #define IGNORE_BUGFIX to revert and compare if needed
#ifdef IGNORE_BUGFIX
			save_offsets(f,
						y.old_start + shift_A,
						x.old_end + shift_A + B_includes_A_shift,
						y.new_start + shift_B,
						x.new_end + shift_B + A_includes_B_shift + new_shift_B);
#else
			save_offsets(f,
						y.old_start + shift_A,
						x.old_end + shift_A + B_includes_A_shift,
						y.new_start,
						x.new_end + A_includes_B_shift + new_shift_B);
#endif

			A_includes_B_shift = 0;
			B_includes_A_shift = 0;
			shift_B += new_shift_B;
			shift_A = x.old_end - x.new_end;
			/* We skip A */
			i++;
			continue;
		}
		if (o == A_EQUALS_B) {
			/* If A equals B starting after B, then we have to produce an output */
			//error("A equals B entre %d;%d et %d;%d\n",x.new_start,x.new_end,y.old_start,y.old_end);
			save_offsets(f,
						x.old_start + shift_A, 
						x.old_end + shift_A + B_includes_A_shift,
						y.new_start + shift_B,
						y.new_end + shift_B + A_includes_B_shift);
			A_includes_B_shift = 0;
			B_includes_A_shift = 0;
			shift_A = x.old_end - x.new_end;
			shift_B = y.new_end - y.old_end;
			/* We skip A */
			i++;
			continue;
		}
	}
	if (o != (Overlap) -1 && o != A_AFTER_B && i != old_offsets->nbelems)
		fatal_error("o==%d\n", o);
	/* Default case: A_AFTER_B */
	save_offsets(f,
					y.old_start + shift_A,
					y.old_end + shift_A + B_includes_A_shift,
					y.new_start,
					y.new_end);
	/* The following line was introduced as a bug fix in r2771, but
	 * it caused several problems. Since commenting it out solve
	 * all those issues, let's do. If someone steps again on the bug
	 * that this line was fixing, we will check this out again.
	 */
	//shift_A+=B_includes_A_shift;
	B_includes_A_shift = 0;
	shift_B = y.new_end - y.old_end;
}
while (i < old_offsets->nbelems) {
	x = old_offsets->tab[i++];
	save_offsets(f,
					x.old_start,
					x.old_end, 
					x.new_start + shift_B,
					x.new_end + shift_B + A_includes_B_shift);
	A_includes_B_shift = 0;
}
}


/**
 * Saves snt offsets to the given file, as a binary file containing integers.
 * Returns 1 in case of success; 0 otherwise.
 */
int save_snt_offsets(vector_int* snt_offsets,const char* name) {
if (snt_offsets==NULL) {
	fatal_error("Unexpected NULL offsets in save_snt_offsets\n");
}
if (snt_offsets->nbelems%3 != 0) {
	fatal_error("Invalid offsets in save_snt_offsets\n");
}
U_FILE* f=u_fopen(BINARY,name,U_WRITE);
if (f==NULL) return 0;
int ret=(int)(fwrite(snt_offsets->tab,sizeof(int),snt_offsets->nbelems,f));
u_fclose(f);
return (ret==snt_offsets->nbelems);
}


/**
 * Loads snt offsets from the given binary file.
 */
vector_int* load_snt_offsets(const char* name) {
U_FILE* f=u_fopen(BINARY,name,U_READ);
if (f==NULL) return NULL;
long size=get_file_size(f);
if (size%(3*sizeof(int))!=0) {
	u_fclose(f);
	return NULL;
}
vector_int* v=new_vector_int((int)(size/sizeof(int)));
if (size!=0) {
	int n=(int)fread(v->tab,sizeof(int),size/sizeof(int),f);
	u_fclose(f);
	if (n!=(int)(size/sizeof(int))) {
		free_vector_int(v);
		return NULL;
	}
	v->nbelems=v->size;
}
return v;
}


/**
 * This function adds a new token shift to the given snt offsets.
 */
void add_snt_offsets(vector_int* snt_offsets,int token_pos,int shift_before,int shift_after) {
vector_int_add(snt_offsets,token_pos);
vector_int_add(snt_offsets,shift_before);
vector_int_add(snt_offsets,shift_after);
}


/**
 * Reads the start and end positions of each token stored in the file
 * produced by Tokenize's --output_offsets option.
 */
vector_int* load_uima_offsets(const VersatileEncodingConfig* vec,const char* name) {
U_FILE* f;
f=u_fopen(vec,name,U_READ);
if (f==NULL) {
   return NULL;
}
vector_int* v=new_vector_int();
Ustring* line=new_Ustring();
int a,b,c;
while (EOF!=readline(line,f)) {
	u_sscanf(line->str,"%d%d%d",&a,&b,&c);
	vector_int_add(v,b);
	vector_int_add(v,c);
}
free_Ustring(line);
u_fclose(f);
return v;
}

} // namespace unitex
