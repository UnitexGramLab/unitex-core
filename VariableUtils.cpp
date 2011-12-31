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

#include "VariableUtils.h"
#include "Ustring.h"


Ustring* get_variable_content(unichar* name,struct locate_parameters* p) {
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


Ustring* get_output_variable_content(unichar* name,struct locate_parameters* p) {
Ustring* res=get_output_variable(p->output_variables,name);
if (res==NULL) return NULL;
return new_Ustring(res->str);
}


Ustring* get_dic_variable_content(unichar* name,struct locate_parameters* p) {
struct dela_entry* entry=get_dic_variable(name,p->dic_variables);
if (entry==NULL) return NULL;
return new_Ustring(entry->inflected);
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
res=get_output_variable_content(name,p);
if (res!=NULL) return res;
return get_dic_variable_content(name,p);
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
int compare_variables(unichar* var1,unichar* var2,struct locate_parameters* p,int case_matters) {
Ustring* v1=get_var_content(var1,p);
if (!v1) {
	return VAR_CMP_ERROR;
}
Ustring* v2=get_var_content(var2,p);
if (!v2) {
	free_Ustring(v1);
	return VAR_CMP_ERROR;
}
int ret=case_matters?u_strcmp(v1->str,v2->str):u_strcmp_ignore_case(v1->str,v2->str);
free_Ustring(v1);
free_Ustring(v2);
if (ret==0) {
	return VAR_CMP_EQUAL;
}
return VAR_CMP_DIFF;
}
