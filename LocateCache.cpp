/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdio.h>
#include <stdlib.h>
#include "LocateCache.h"
#include "Error.h"
#include "Match.h"


/**
 * Builds, initializes and returns a new LocateCache.
 */
LocateCache new_LocateCache(int token,struct match_list* matches,Abstract_allocator prv_alloc) {
LocateCache c=(LocateCache)malloc_cb(sizeof(struct locate_cache),prv_alloc);
if (c==NULL) {
	fatal_alloc_error("new_LocateCache");
}
c->left=NULL;
c->middle=NULL;
c->right=NULL;
c->token=token;
c->matches=matches;
return c;
}


/**
 * Frees all the memory associated to the given LocateCache, including
 * its match list, if any.
 */
void free_LocateCache(LocateCache c,Abstract_allocator prv_alloc) {
if (c==NULL) return;
free_LocateCache(c->left,prv_alloc);
free_LocateCache(c->middle,prv_alloc);
free_LocateCache(c->right,prv_alloc);
free_match_list(c->matches,prv_alloc);
free_cb(c,prv_alloc);
}


/**
 * Caches the given token sequence in the given cache. Note that
 * match is supposed to contain a single match, not a match list.
 */
static void cache_match_internal(struct match_list* match,const int* tab,int start,int end,LocateCache *c,Abstract_allocator prv_alloc) {
int token=-1;
struct match_list* m=match;
if (start<=end) {
	token=tab[start];
	m=NULL;
}
/* No node */
if (*c==NULL) {
	*c=new_LocateCache(token,m,prv_alloc);
	if (token!=-1) {
		cache_match_internal(match,tab,start+1,end,&((*c)->middle),prv_alloc);
	}
	return;
}
/* There is a node */
if (token<(*c)->token) {
	/* If we have to move on the left */
	return cache_match_internal(match,tab,start,end,&((*c)->left),prv_alloc);
}
if (token>(*c)->token) {
	/* If we have to move on the right */
	return cache_match_internal(match,tab,start,end,&((*c)->right),prv_alloc);
}
/* We have the correct token */
if (token==-1) {
	/* If we are in a final node that already existed, we just add
	 * the new match at the end of the match list to get the same match order as
	 * if the cache system had not been used, but only if the match is not already present */
	struct match_list* *ptr=&((*c)->matches);
	struct match_list* z;
	match->next=NULL;
	while ((*ptr)!=NULL) {
		z=*ptr;
		if (compare_matches(&(z->m),&(match->m))==A_EQUALS_B &&
				!u_strcmp(z->output,match->output)) {
			/* We discard a match that was already in cache */
			free_match_list_element(match,prv_alloc);
			return;
		}
		ptr=&((*ptr)->next);
	}
	(*ptr)=match;
	return;
}
cache_match_internal(match,tab,start+1,end,&((*c)->middle),prv_alloc);
}


/**
 * Caches the given token sequence in the given cache. Note that
 * match is supposed to contain a single match, not a match list.
 * There is no need to save the first token, since caches are stored
 * in an array indexed on first tokens.
 */
void cache_match(struct match_list* match,const int* tab,int start,int end,LocateCache *c, Abstract_allocator prv_alloc) {
cache_match_internal(match,tab,start+1,end,c,prv_alloc);
}


/**
 * Explores a given node of the cache tree.
 */
void explore_cache_node(const int* tab,int pos,int tab_size,LocateCache c,vector_ptr* res) {
if (pos==tab_size || c==NULL) return;
if (c->token==-1) {
	/* If we have found a token sequence end mark, then we have matches to
	 * store before trying the right node to look for longer matches. As -1 is
	 * lower than any token value, we don't need to explore the left side */
	vector_ptr_add(res,c->matches);
	explore_cache_node(tab,pos,tab_size,c->right,res);
	return;
}
int token=tab[pos];
if (token==c->token) {
	explore_cache_node(tab,pos+1,tab_size,c->middle,res);
	return;
}
if (token<c->token) {
	explore_cache_node(tab,pos,tab_size,c->left,res);
	return;
}
explore_cache_node(tab,pos,tab_size,c->right,res);
}


/**
 * Consults the cache to find matches. If some are found, the match list pointers
 * associated to token sequences are stored in 'res'. Returns 1 if matches
 * were found; 0 otherwise.
 */
int consult_cache(const int* tab,int start,int tab_size,LocateCache* caches,vector_ptr* res) {
res->nbelems=0;
int first_token=tab[start];
if (first_token==-1) {
	return 0;
}
explore_cache_node(tab,start+1,tab_size,caches[first_token],res);
return res->nbelems!=0;
}

