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

#include <stdlib.h>
#include "GrfTest_lib.h"
#include "Error.h"
#include "Ustring.h"
#include "StringParsing.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

GrfUnitTest* new_GrfUnitTest(unichar* text,int start,int end,int must_match,
		OutputPolicy output_policy,MatchPolicy match_policy,unichar* output) {
GrfUnitTest* t=(GrfUnitTest*)malloc(sizeof(GrfUnitTest));
if (t==NULL) {
	fatal_alloc_error("new_GrfUnitTest");
}
t->text=u_strdup(text);
t->start=start;
t->end=end;
t->must_match=must_match;
t->output_policy=output_policy;
t->match_policy=match_policy;
t->expected_output=u_strdup(output);
return t;
}


void free_GrfUnitTest(GrfUnitTest* test) {
if (test==NULL) return;
free(test->text);
free(test->expected_output);
free(test);
}


static int valid_values(unichar a,unichar b,int *must_match,OutputPolicy *output_policy,
				MatchPolicy *match_policy) {
*must_match=1;
switch (a) {
case 'N': *must_match=0; *output_policy=IGNORE_OUTPUTS; break;
case 'I': *output_policy=IGNORE_OUTPUTS; break;
case 'M': *output_policy=MERGE_OUTPUTS; break;
case 'R': *output_policy=REPLACE_OUTPUTS; break;
default: return 0;
}
switch (b) {
case 'S': *match_policy=SHORTEST_MATCHES; break;
case 'L': *match_policy=LONGEST_MATCHES; break;
case 'A': *match_policy=ALL_MATCHES; break;
default: return 0;
}
return 1;
}

/**
 * If the given box content looks like a grf unit test,
 * it returns an object describing the test; NULL otherwise.
 * If the box content looks like a test but is not well-formed,
 * NULL is returned and a non empty message is stored in error_msg.
 */
static GrfUnitTest* get_grf_unit_test(unichar* box_content,Ustring* error_msg) {
empty(error_msg);
GrfUnitTest* res=NULL;
vector_ptr* lines=tokenize_box_content(box_content);
if (lines==NULL || lines->nbelems==0) {
	fatal_error("get_grf_unit_test: cannot tokenize line:%S\n",box_content);
}
unichar* tmp;
unichar a,b;
int n,start,end;
unichar* text=NULL;
unichar* output=NULL;
if (lines->nbelems==1) {
	/* A test box must contains at least 2 lines */
	goto end;
}
tmp=(unichar*)lines->tab[0];
if (!u_starts_with(tmp,GRF_UNIT_TEST_PFX)) goto end;
int must_match;
OutputPolicy output_policy;
MatchPolicy match_policy;
if (2!=u_sscanf(tmp,GRF_UNIT_TEST_PFX"%C:%C@%n",&a,&b,&n) || tmp[n]!='\0'
		|| !valid_values(a,b,&must_match,&output_policy,&match_policy)) {
	u_sprintf(error_msg,"Invalid unit test first line: __%S__\n",tmp);
	u_strcatf(error_msg,"It should be of the form %sx:y@ with x=[NIMR] and y=[SLA]\n",GRF_UNIT_TEST_PFX);
	goto end;
}
if (lines->nbelems!=3 && (output_policy==MERGE_OUTPUTS || output_policy==REPLACE_OUTPUTS)) {
	u_sprintf(error_msg,"Missing output line in unit test requiring output: %S %S\n",tmp,(unichar*)(lines->tab[1]));
	goto end;
}
tmp=(unichar*)lines->tab[1];
text=u_strdup(tmp);
n=0;
if (P_OK!=parse_string(tmp,&n,text,"<") || tmp[n]=='\0') {
	u_sprintf(error_msg,"Invalid unit test second line: __%S__\n",tmp);
	u_strcatf(error_msg,"It should be of the form xxx<yyy>zzz\n");
	goto end;
}
u_strcat(text," ");
start=u_strlen(text);
n++;
if (P_OK!=parse_string(tmp,&n,text+start,">") || text[start]=='\0') {
	u_sprintf(error_msg,"Invalid unit test second line: __%S__\n",tmp);
	u_strcatf(error_msg,"It should be of the form xxx<yyy>zzz\n");
	goto end;
}
n++;
end=u_strlen(text);
u_strcat(text," ");
if (tmp[n]!='\0' && P_OK!=parse_string(tmp,&n,text+end+1,P_EMPTY)) {
	u_sprintf(error_msg,"Invalid unit test second line: __%S__\n",tmp);
	u_strcatf(error_msg,"It should be of the form xxx<yyy>zzz\n");
	goto end;
}
if (lines->nbelems!=2) {
	if (lines->nbelems!=3) {
		u_sprintf(error_msg,"Invalid unit test with too many lines\n");
		goto end;
	}
	/* If there is an output */
	tmp=(unichar*)lines->tab[2];
	if (output_policy!=MERGE_OUTPUTS && output_policy!=REPLACE_OUTPUTS) {
		u_sprintf(error_msg,"Invalid unit test with unexpected output: __%S__\n",tmp);
		goto end;
	}
	output=u_strdup(tmp);
	n=0;
	if (P_OK!=parse_string(tmp,&n,output,P_EMPTY)) {
		u_sprintf(error_msg,"Invalid unit test output line: __%S__\n",tmp);
		goto end;
	}
}
res=new_GrfUnitTest(text,start,end,must_match,output_policy,match_policy,output);
end:
free_vector_ptr(lines,free);
free(text);
free(output);
return res;
}


/**
 * Returns a vector containing all the unit tests found in the given grf,
 * or NULL if none was found. If some errors are found, error
 * messages are printed to f_error.
 */
vector_ptr* get_grf_unit_tests(Grf* grf,const char* grf_name,U_FILE* f_error) {
if (grf==NULL) {
	fatal_alloc_error("Unexpected NULL grf in get_grf_unit_tests\n");
}
vector_ptr* tests=new_vector_ptr();
ReverseTransitions* reverse=compute_reverse_transitions(grf);
Ustring* msg=new_Ustring(128);
int first=1;
for (int i=2;i<grf->n_states;i++) {
	/* A test box must be a comment one */
	if (grf->states[i]->transitions->nbelems!=0 || reverse->t[i]->nbelems!=0) continue;
	GrfUnitTest* test=get_grf_unit_test(grf->states[i]->box_content,msg);
	if (msg->str[0]!='\0') {
		if (first) {
			u_fprintf(f_error,"Error in graph %s:\n",grf_name);
			first=0;
		}
		u_fprintf(f_error,"%S",msg->str);
	}
	if (test!=NULL) {
		vector_ptr_add(tests,test);
	}
}
free_Ustring(msg);
free_ReverseTransitions(reverse);
if (tests->nbelems==0) {
	free_vector_ptr(tests,(void(*)(void*))free_GrfUnitTest);
	return NULL;
}
return tests;
}


/**
 * This function compares the given test configuration to the actual
 * results stored in the given concord.txt file. Returns 1 if ok;
 * otherwise, returns 0 and prints an error message to f_error.
 */
int check_test_results(GrfUnitTest* t,const char* concord,const char* grf,U_FILE* f_error) {
U_FILE* f=u_fopen(UTF16_LE,concord,U_READ);
if (f==NULL) {
	u_fprintf(f_error,"Error for graph %s:\nCannot open concordance file %s\n",grf,concord);
	return 0;
}
int match=0,start,end,n,quite_match=0;
Ustring* line=new_Ustring(128);
unichar* bad=NULL;
while (EOF!=readline(line,f)) {
	u_sscanf(line->str,"%d%d%n",&start,&end,&n);
	if (t->must_match==0 && start==t->start && end==t->end) {
		/* If we have a match when we did not want, we are done */
		u_fprintf(f_error,"Test failed in graph %s:\n",grf);
		u_fprintf(f_error,"\"%S\" matched when it was not supposed to\n\n",t->text);
		match=1;
		break;
	}
	if (t->must_match && start==t->start && end==t->end) {
		quite_match=1;
		/* We have a match, we must test the output, if any */
		if (t->expected_output==NULL || !u_strcmp(t->expected_output,line->str+n)) {
			match=1;
			break;
		} else {
			free(bad);
			bad=u_strdup(line->str+n);
		}
	}
}
if (!match && t->must_match) {
	if (quite_match) {
		u_fprintf(f_error,"Test failed in graph %s:\n",grf);
		u_fprintf(f_error,"\"%S\" was matched but with bad output \"%S\" (expected=\"%S\")\n\n",
				t->text,bad,t->expected_output);
	} else {
		u_fprintf(f_error,"Test failed in graph %s:\n",grf);
		u_fprintf(f_error,"\"%S\" was not matched\n\n",t->text);
	}
}
free_Ustring(line);
free(bad);
u_fclose(f);
return match==t->must_match;
}

} // namespace unitex
