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

#include "VariableUtils.h"
#include "Ustring.h"
#include "UnusedParameter.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

Ustring* get_variable_content(const unichar* name,struct locate_parameters* p) {
struct transduction_variable* v=get_transduction_variable(p->input_variables,name);
if (v==NULL || v->start_in_tokens==UNDEF_VAR_BOUND || v->end_in_tokens==UNDEF_VAR_BOUND
    || v->start_in_tokens>v->end_in_tokens
    || (v->start_in_tokens==v->end_in_tokens && v->end_in_chars!=-1 && v->end_in_chars<v->start_in_chars)) {
 	return NULL;
}
Ustring* res=new_Ustring(128);
/* Case 1: start and end in the same token*/
if (v->start_in_tokens==v->end_in_tokens-1) {
   unichar* tok=p->tokens->value[p->buffer[v->start_in_tokens+p->current_origin]];
   int last=(v->end_in_chars!=-1) ? (v->end_in_chars) : (((int)u_strlen(tok))-1);
   for (int k=v->start_in_chars;k<=last;k++) {
	   u_strcat(res,tok[k]);
   }
} else {
	/* Case 2: first we deal with first token */
	unichar* tok=p->tokens->value[p->buffer[v->start_in_tokens+p->current_origin]];
	u_strcat(res,tok+v->start_in_chars);
	/* Then we copy all tokens until the last one */
	for (int k=v->start_in_tokens+1;k<v->end_in_tokens-1;k++) {
		u_strcat(res,p->tokens->value[p->buffer[k+p->current_origin]]);
	}
	/* Finally, we copy the last token */
	tok=p->tokens->value[p->buffer[v->end_in_tokens-1+p->current_origin]];
	int last=(v->end_in_chars!=-1) ? (v->end_in_chars) : (((int)u_strlen(tok))-1);
	for (int k=0;k<=last;k++) {
		u_strcat(res,tok[k]);
	}
}
return res;
}


Ustring* get_output_variable_content_Ustring(const unichar* name,struct locate_parameters* p) {
const Ustring* res=get_output_variable(p->output_variables,name);
if (res==NULL) return NULL;
return new_Ustring(res->str);
}


unichar* get_output_variable_content_str(const unichar* name, struct locate_parameters* p) {
	const Ustring* res = get_output_variable(p->output_variables, name);
	if (res == NULL) return NULL;
	return res->str;
}


Ustring* get_dic_variable_content_Ustring(const unichar* name,struct locate_parameters* p) {
struct dela_entry* entry=get_dic_variable(name,p->dic_variables);
if (entry==NULL) return NULL;
return new_Ustring(entry->inflected);
}


unichar* get_dic_variable_content_str(const unichar* name, struct locate_parameters* p) {
	struct dela_entry* entry = get_dic_variable(name, p->dic_variables);
	if (entry == NULL) return NULL;
	return entry->inflected;
}

/**
 * Returns the content associated to the given variable name. If
 * 'name' starts with '#', then we return the remaining string.
 */
Ustring* get_var_content(unichar* name,struct locate_parameters* p) {
if (name[0]=='#') {
	/* The user asked for a constant string */
	return new_Ustring(name+1);
}
Ustring* res=get_variable_content(name,p);
if (res!=NULL) return res;
res=get_output_variable_content_Ustring(name,p);
if (res!=NULL) return res;
return get_dic_variable_content_Ustring(name,p);
}


unichar* get_var_content_str(const unichar* name, struct locate_parameters* p, int*free_needed) {
	*free_needed = 0;
	if (name[0] == '#') {
		/* The user asked for a constant string */
		return (unichar*)name + 1;
	}
	Ustring* res = get_variable_content(name, p);
	if (res != NULL) {
		*free_needed = 1;
		return free_Ustring_get_str(res);
	}
	unichar* str = get_output_variable_content_str(name, p);
	if (str != NULL) return str;
	return get_dic_variable_content_str(name, p);
}


/**
 * This function compares two variables. Input and output variables are
 * considered as text content. For dictionary variables, it's the inflected
 * form that is taken into account.
 *
 * Note 1: you can compare variables of different kinds
 * Note 2: you can compare a variable to a constant string. To do that, the string
 *         must start with #
 */
int compare_variables(const unichar* var1,const unichar* var2,struct locate_parameters* p,int case_matters) {
int free_v1;
unichar* v1=get_var_content_str(var1,p,&free_v1);
if (!v1) {
	return VAR_CMP_ERROR;
}
int free_v2;
unichar* v2=get_var_content_str(var2,p,&free_v2);
if (!v2) {
	if (free_v1) free(v1);
	return VAR_CMP_ERROR;
}
int ret=case_matters?u_strcmp(v1,v2):u_strcmp_ignore_case(v1,v2);
if (free_v1) free(v1);
if (free_v2) free(v2);
if (ret==0) {
	return VAR_CMP_EQUAL;
}
return VAR_CMP_DIFF;
}


int compare_variables_substr(const unichar* var1,const unichar* var2,struct locate_parameters* p,int case_matters) {
DISCARD_UNUSED_PARAMETER(case_matters)
int free_v1;
unichar* v1 = get_var_content_str(var1, p, &free_v1);
if (!v1) {
	return VAR_CMP_ERROR;
}
int free_v2;
unichar* v2 = get_var_content_str(var2, p, &free_v2);
if (!v2) {
	if (free_v1) free(v1);
	return VAR_CMP_ERROR;
}
int ret=u_substr(v1,v2);
if (free_v1) free(v1);
if (free_v2) free(v2);
if (ret != 0) {
	return VAR_CMP_EQUAL;
}
return VAR_CMP_DIFF;
}


} // namespace unitex
