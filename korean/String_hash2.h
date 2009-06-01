 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
  *
  */
#ifndef String_hash2H
#define String_hash2H

#include "Unicode.h"
#include "File.h"
#include "Error.h"



#define BIN_TYPE_SUFFIXE 1
#define BIN_TYPE_RACINE	0

//
//   class arbre_string0    structure for the string
//  class arbre_string00    strucure for the string with the value
template <class T>
class simpleL {
struct symLink {
	T v;
	struct symLink *next;
};
	struct symLink *head;
	struct symLink *tail;
	struct symLink *scan;
	int cnt;
public:
	simpleL(){ head = 0; tail = 0;cnt = 0;};
	~simpleL(){
		struct symLink *p;
		while(cnt){
			p = head;
			head = head->next;
			free(p);
			--cnt;
		};
	}
	void put(T v){
		struct symLink *p =
			(struct symLink *)
			malloc(sizeof(struct symLink));
		p->next = 0;
		p->v = v;
		if(cnt){
			tail->next = p;
			tail = p;
		} else {
			head = p;
			tail = p;
		}
		cnt++;
	}
	bool pop(T *v)
	{
		if(cnt){
			struct symLink *p;
			p = head;
			*v = head->v;
			head = head->next;
			free(p);
			cnt--;
			return(true);
		}
		return(false);
	}
	int size()	{return(cnt);};
	void reset(){
		scan = head;
	};
	T getNext(){
		if(scan){
			T v;
			v = scan->v;
			scan = scan->next;
			return(v);
		}
		return(0);
	}
	void freeAll()
	{
		struct symLink *scan = head;
		while(scan){
			free (scan->v);
			scan = scan->next;
		}
	}
	void del()
	{
		struct symLink *scan = head;
		while(scan){
			free(scan->v);
			scan = scan->next;
		}
	}
};
//class debugF {
//public:
//	FILE *f;
//	debugF(){
//		f = u_fopen("debug.txt",U_WRITE);
//	};
//	~debugF(){
//		fclose(f);
//	};
//} d;

typedef void (*release_f)(void * arg0,void *arg1,void *arg2);

typedef void (*release_ff)(void * arg0);

//
//
//

struct arbre_hash0 {
       int final;
       struct arbre_hash_trans0* trans;
};
struct arbre_hash_trans0 {
    unichar c;
    struct arbre_hash0* arr;
   struct arbre_hash_trans0* suivant;
};
struct simple_link {
	unsigned int n;
	struct simple_link *suivant;
};

class arbre_string0 {
    int N;
	int limit;
    struct arbre_hash0* racine;
	int modeExplore;
	int noeud_cnt;
	int trans_cnt;
	U_FILE *tabOfile;
public:
	unichar **tab;
	arbre_string0(){
		racine = 0;
		noeud_cnt=0;
		trans_cnt=0;
		N=0;
		limit = 0x100000;
		tab = 0;
		racine=new_noeud();
		tabOfile = 0;
	};
	~arbre_string0(){
		free_arbre_hash0(racine);
		if(tab){ for(int i = 0; i < N;i++) if(tab[i]) free(tab[i]);
		free(tab);}
	};
	int size(){return(N);};
	struct arbre_hash0 *getRacine(){ return(racine);};
	struct arbre_hash0* new_noeud() {
		struct arbre_hash0* a;
		a=(struct arbre_hash0*)malloc(sizeof(struct arbre_hash0));
		if(!a) {
			fatal_error("malloc fail to hash tree\n");
		}
		a->final=-1;
		a->trans=NULL;
		noeud_cnt++;
		return a;
	}


	struct arbre_hash_trans0* new_trans() {
		struct arbre_hash_trans0* a;
		a=(struct arbre_hash_trans0*)
			malloc(sizeof(struct arbre_hash_trans0));
		if(!a){
			fatal_error("malloc fail to hash tree\n");
		}
		a->c='\0';
		a->arr=NULL;
		a->suivant=NULL;
		trans_cnt++;
		return a;
	}


	void free_arbre_hash_trans0(struct arbre_hash_trans0* t) {
		struct arbre_hash_trans0* tmp;
		while (t!=NULL) {
			free_arbre_hash0(t->arr);
			tmp=t;
			t=t->suivant;
			free(tmp);
		}
	}


	void free_arbre_hash0(struct arbre_hash0* a) {
		if (a==NULL) return;
		free_arbre_hash_trans0(a->trans);
		free(a);
	}
	void set_limit(int li) {
		limit  = li;
	}

	struct arbre_hash0* insert(unichar* s,int pos,
		struct arbre_hash0* noeud) {
		if (noeud==NULL) {
		  fatal_error("Erreur dans fonction inserer\n");
		}
		if (!s[pos]) return(noeud);

		struct arbre_hash_trans0  **t= &noeud->trans;
		while(*t){
			if((*t)->c == s[pos])
				return insert(s,pos+1,(*t)->arr);
			if( (*t)->c > s[pos]) break;
			t = &((*t)->suivant);
		}
		if(modeExplore){
			struct arbre_hash_trans0 *tt=new_trans();
			tt->c=s[pos];
			tt->arr=new_noeud();
			tt->suivant=*t;
			*t=tt;
			return insert(s,pos+1,tt->arr);
		}
		return (struct arbre_hash0*)-1;
	}


	int put(unichar* s) {
		modeExplore = 1;
		struct arbre_hash0* noeud = insert(s,0,racine);
		if(noeud->final == -1)
			noeud->final=N++;
		if(N >= limit)	fatal_error("Too many hash elements\n");
      return noeud->final;
	}

	int check(unichar* s)
	{
		modeExplore = 0;
		struct arbre_hash0* noeud = insert(s,0,racine);
		if(noeud == (struct arbre_hash0 *)-1)
			return -1;
		return(noeud->final);
	}
	unichar cbuff[1024];
	void explore_tout_leaf(release_f ppr)
	{
		explore_leaf(racine,1,ppr);
	}
	void explore_leaf(struct arbre_hash0* noeud,int pos,release_f ppr)
	{
		if(noeud->final != -1){
			cbuff[0] = pos;
			cbuff[pos] = 0;
			if(ppr) (*ppr)((void *)cbuff,(void *)noeud->final,
				(void *)0);
		}
		struct arbre_hash_trans0  **t= &noeud->trans;
		while(*t){
			cbuff[pos] = (*t)->c;
			explore_leaf((*t)->arr,pos+1,ppr);
			t = &((*t)->suivant);
		}
	}
    //make string table
	// after use this function , on doit call release_value()
	unichar **make_strPtr_table()
	{
		U_FILE *sf = tabOfile;
		tabOfile = 0;
		tab = (unichar **)malloc(N*sizeof(unichar *));
		if(!tab) fatal_alloc_error("make_strPtr_table");
		explore_to_get(racine,1);
		tabOfile = sf;
		return(tab);
	}
	void release_value()
	{
		int i;
		for(i = 0; i< N;i++) free(tab[i]);
		free(tab);
		tab = 0;
	}
	void out_to_file(U_FILE *f)
	{
		unichar **stab = tab;
		tab = 0;
		tabOfile = f;
		u_fprintf(tabOfile,"%d\n",N);
		explore_to_get(racine,0);
		tabOfile = 0;
		tab = stab;
	}
	void out_to_file_data(U_FILE *f)
	{
		unichar **stab = tab;
		tab = 0;
		tabOfile = f;
		u_fprintf(tabOfile,"%d\n",N);
		explore_to_get(racine,0);
		tabOfile = 0;
		tab = stab;
	}
	void explore_to_get(struct arbre_hash0* noeud,int pos)
	{
	 int i;
		if(noeud->final != -1){
			if(tab){
				cbuff[0] = pos;
				tab[noeud->final] =
					(unichar *)malloc(
					(pos+2) * sizeof(unichar));
				for(i=0; i < pos; i++)	tab[noeud->final][i]=cbuff[i];
				tab[noeud->final][i] = 0;
			}
			if(tabOfile)
			{
				cbuff[pos] = 0;
				u_fprintf(tabOfile,"%S\n",cbuff);
			}
		}
		struct arbre_hash_trans0  **t= &noeud->trans;
		while(*t){
			cbuff[pos] = (*t)->c;
			explore_to_get((*t)->arr,pos+1);
			t = &((*t)->suivant);
		}
	}

	//
	//	data out by sorted order and table which have not sorted order
	//
	unsigned int *sortedmap;
	int accessOrderCnt;
	void out_to_file_data1(U_FILE *f,unsigned int *sorted)
	{
		accessOrderCnt = 0;
		tabOfile = f;
		sortedmap = sorted;
		explore_to_get_avec_unsorted(racine,0);
		tabOfile = 0;
	}

	void explore_to_get_avec_unsorted(struct arbre_hash0* noeud,int pos)
	{
		if(noeud->final != -1){
			sortedmap[accessOrderCnt++] = noeud->final;
			cbuff[pos] = 0;
			u_fprintf(tabOfile,"%S\n",cbuff);
		}
		struct arbre_hash_trans0  **t= &noeud->trans;
		while(*t){
			cbuff[pos] = (*t)->c;
			explore_to_get_avec_unsorted((*t)->arr,pos+1);
			t = &((*t)->suivant);
		}
	}
	int insertLink(struct simple_link *cur,
		struct arbre_hash0* noeud) {
		if (noeud==NULL) {
		  fatal_error("Erreur dans fonction inserer\n");
		}
		if (!cur){
			if(noeud->final == -1)	noeud->final=N++;
			if(N < limit)
				return noeud->final;

			{
				fatal_error("Too many hash elements\n");
			}
		}

		struct arbre_hash_trans0  **t= &noeud->trans;
		unichar cc = (unichar)(cur->n);
		while(*t){
			if((*t)->c == cc )
				return insertLink(cur->suivant,(*t)->arr);
			if( (*t)->c > cc ) break;
			t = &((*t)->suivant);
		}
		if(modeExplore){
			struct arbre_hash_trans0 *tt=new_trans();
			tt->c=cc;
			tt->arr=new_noeud();
			tt->suivant=*t;
			*t=tt;
			return insertLink(cur->suivant,tt->arr);
		}
		return -1;
	}
	int putLink(struct simple_link *v)
	{
		modeExplore = 1;
		return( insertLink(v,racine));
	}
};
/*
// string hash with value;
*/

struct arbre_hash00 {
       int final;
       uintptr_t value;
       struct arbre_hash_trans00* trans;
};
struct arbre_hash_trans00 {
    unichar c;
    struct arbre_hash00* arr;
   struct arbre_hash_trans00* suivant;
};

class arbre_string00 {
	int limit;
    struct arbre_hash00* racine;
    struct arbre_hash00* lastCheckNoeud;
	int noeud_cnt;
	int trans_cnt;
	unichar **tab;
	int *value_tab;
    int N;
	int modeExplore;
public:
	arbre_string00(){
		noeud_cnt=0;
		trans_cnt=0;
		N=0;
		limit = 0x100000;
		tab = 0;
		value_tab = 0;
		racine=new_noeud();
	};
	~arbre_string00(){
		free_arbre_hash00(racine);
//printf("arbre_string00 end\n");
	};
	int size(){return(N);};
	struct arbre_hash00* getRacine(){return(racine);};
	struct arbre_hash00* new_noeud() {
		struct arbre_hash00* a;
		a=(struct arbre_hash00*)malloc(sizeof(struct arbre_hash00));
		if(!a) {
			fatal_error("malloc fail to hash tree\n");
		}
		a->final= -1;
		a->value = 0;
		a->trans=0;
		noeud_cnt++;
		return a;
	}


	struct arbre_hash_trans00* new_trans() {
		struct arbre_hash_trans00* a;
		a=(struct arbre_hash_trans00*)
			malloc(sizeof(struct arbre_hash_trans00));
		if(!a){
			fatal_error("malloc fail to hash tree\n");
		}
		a->c='\0';
		a->arr=NULL;
		a->suivant=NULL;
		trans_cnt++;
		return a;
	}


	void free_arbre_hash_trans00(struct arbre_hash_trans00* t) {
		struct arbre_hash_trans00* tmp;
		while (t!=NULL) {
			free_arbre_hash00(t->arr);
			tmp=t;
			t=t->suivant;
			free(tmp);
		}
	}


	void free_arbre_hash00(struct arbre_hash00* a) {
		if (a==NULL) return;
		free_arbre_hash_trans00(a->trans);
		free(a);
	}
	void set_limit(int li) {
		limit  = li;
	}

	struct arbre_hash00* insert(unichar* s,int pos,
		struct arbre_hash00* noeud) {
		if (noeud==NULL) {
		  fatal_error("Erreur dans fonction inserer\n");
		}
		if (!s[pos]) return(noeud);

		struct arbre_hash_trans00  **t= &noeud->trans;
		while(*t){
			if((*t)->c == s[pos])
				return insert(s,pos+1,(*t)->arr);
			if( (*t)->c > s[pos]) break;
			t = &((*t)->suivant);
		}
		if(modeExplore){
			struct arbre_hash_trans00 *tt=new_trans();
			tt->c=s[pos];
			tt->arr=new_noeud();
			tt->suivant=*t;
			*t=tt;
			return insert(s,pos+1,tt->arr);
		}
		return ((struct arbre_hash00*)-1);
	}


	int put(unichar* s,void *value) {
		modeExplore = 1;
		struct arbre_hash00* noeud = insert(s,0,racine);
		if(noeud->final == -1){
			noeud->final=N++;
			noeud->value = (uintptr_t)value;
		}
		if(N >= limit)	fatal_error("Too many hash elements\n");
      return noeud->final;
	}

	int check(unichar* s)
	{
		modeExplore = 0;
		struct arbre_hash00* noeud = insert(s,0,racine);
		if(noeud == (struct arbre_hash00 *) -1) return(-1);
		lastCheckNoeud = noeud;
		return(noeud->final);
	}


	int insertWordAndInc(unichar *s,int sz)
	{
        int i;
		for(i = 0; i < sz;i++) cbuff[i] = s[i];
		cbuff[i] = 0;
		modeExplore = 1;
		struct arbre_hash00* noeud = insert(cbuff,0,racine);
		if(noeud->final == -1){
			noeud->final=N++;
			noeud->value = 0;
		}
		noeud->value++;
		if(N < limit) 	return noeud->final;
		fatal_error("Too many hash elements\n");
		return 0;
	}
     int insertWord(unichar *s)
     {
		modeExplore = 1;
		struct arbre_hash00* noeud = insert(s,0,racine);
		if(noeud->final == -1){
			noeud->final=N++;
			noeud->value = 0;
		}
		noeud->value++;
		if(N >= limit)	fatal_error("Too many hash elements\n");
      return noeud->final;
     }
	//
	//	must used after check
	//
	void *getCheckValue()
	{
		return((void *)lastCheckNoeud->value);
	}
	//
	// use carefully after check
	//
	void *getValue(unichar *s)
	{
		struct arbre_hash00* noeud = insert(s,0,racine);
		return (void *)noeud->value;
	}
	unsigned int getIndex(unichar *s)
	{
		struct arbre_hash00* noeud = insert(s,0,racine);
		return noeud->final;
	}
	unichar cbuff[1024];
	//
	//  get all word in alphabetic order
	//
	void explore_tout_leaf(release_f ppr)
	{
		explore_leaf(racine,1,ppr);
	}
	void explore_leaf(struct arbre_hash00* noeud,int pos,release_f ppr)
	{
		if(noeud->final != -1){
			cbuff[0] = pos;
			cbuff[pos] = 0;
			if(ppr) (*ppr)((void *)cbuff,(void *)noeud->final,(void *)noeud->value);
		}
		struct arbre_hash_trans00  **t= &noeud->trans;
		while(*t){
			cbuff[pos] = (*t)->c;
			explore_leaf((*t)->arr,pos+1,ppr);
			t = &((*t)->suivant);
		}
	}

	//
	//   get all word in alphabetic order
	//
	int flag_value_out;
	U_FILE *fileOut;
	void save_tout_leaf(U_FILE *f,int flag)
	{
	    flag_value_out = flag;
	    if(!f) return;
        fileOut = f;
		explore_to_save(racine,0);
	}
	void explore_to_save(struct arbre_hash00* noeud,int pos)
	{
		if(noeud->final != -1){
			cbuff[pos] = 0;
			if(flag_value_out)
			   u_fprintf(fileOut,"%S\t%d\n",cbuff,noeud->value);
			else
			   u_fprintf(fileOut,"%S\n",cbuff);

		}
		struct arbre_hash_trans00  **t= &noeud->trans;
		while(*t){
			cbuff[pos] = (*t)->c;
			explore_to_save((*t)->arr,pos+1);
			t = &((*t)->suivant);
		}
	}
	//
	//
	//

    //make string table
	// after use this function , on doit call release_value()
	unichar **make_strPtr_table(int **v)
	{
		tab = (unichar **)malloc(N*sizeof(unichar *));
		if (tab==NULL) {
		   fatal_alloc_error("make_strPtr_table");
		}
		value_tab = (int *)malloc(N*sizeof(int *));
		if (value_tab==NULL) {
		   fatal_alloc_error("make_strPtr_table");
		}
		(*v) = value_tab;
		explore_to_get(racine,1);
		return(tab);
	}


	void release_value()
	{
		int i;
		for(i = 0; i< N;i++) free(tab[i]);
		free(value_tab);
		free(tab);
		tab = 0;
		value_tab = 0;
	}

	void explore_to_get(struct arbre_hash00* noeud,int pos)
	{
	    int i;
		if(noeud->final != -1){
			cbuff[0] = pos;
			tab[noeud->final] =
				(unichar *)malloc(
				(pos+2) * sizeof(unichar));
			for(i=0; i < pos; i++)	tab[noeud->final][i]=cbuff[i];
			tab[noeud->final][i] = 0;
			value_tab[noeud->final] = (int)noeud->value;
		}
		struct arbre_hash_trans00  **t= &noeud->trans;
		while(*t){
			cbuff[pos] = (*t)->c;
			explore_to_get((*t)->arr,pos+1);
			t = &((*t)->suivant);
		}
	}
	int insertLink(struct simple_link *cur,
		struct arbre_hash00* noeud) {
		if (noeud==NULL) {
		  fatal_error("Erreur dans fonction inserer\n");
		}
		if (!cur){
			if(noeud->final == -1)	noeud->final=N++;
			if(N < limit)	return noeud->final;
			fatal_error("Too many hash elements\n");
		}

		struct arbre_hash_trans00  **t= &noeud->trans;
		unichar cc = (unichar)(cur->n);
		while(*t){
			if((*t)->c == cc )
				return insertLink(cur->suivant,(*t)->arr);
			if( (*t)->c > cc ) break;
			t = &((*t)->suivant);
		}
		if(modeExplore){
			struct arbre_hash_trans00 *tt=new_trans();
			tt->c=cc;
			tt->arr=new_noeud();
			tt->suivant=*t;
			*t=tt;
			return insertLink(cur->suivant,tt->arr);
		}
		return -1;
	}
	int putLink(struct simple_link *v)
	{
		modeExplore = 1;
		return( insertLink(v,racine));
	}
};

//
// string hash with value;
//
struct arbre_hash02 {
       int final;
	   uintptr_t value;
       struct arbre_hash_trans02* trans;
};
struct arbre_hash_trans02 {
    unsigned int c;
    struct arbre_hash02* arr;
   struct arbre_hash_trans02* suivant;
};

class arbre_string02 {
	int limit;
    struct arbre_hash02* racine;
    struct arbre_hash02* lastCheckNoeud;
	int noeud_cnt;
	int trans_cnt;
	unichar **tab;
	int *value_tab;
    int N;
	int modeExplore;
public:
	int sequenceMaxDepth;
	arbre_string02(){
		noeud_cnt=0;
		trans_cnt=0;
		N=0;
		limit = 0x100000;
		tab = 0;
		value_tab = 0;
		racine=new_noeud();
		sequenceMaxDepth =0;
	};
	~arbre_string02(){
		free_arbre_hash02(racine);
	};
	int size(){return(N);};
	struct arbre_hash02* getRacine(){return(racine);};
	struct arbre_hash02* new_noeud() {
		struct arbre_hash02* a;
		a=(struct arbre_hash02*)malloc(sizeof(struct arbre_hash02));
		if(!a) {
			fatal_error("malloc fail to hash tree\n");
		}
		a->final= -1;
		a->value = 0;
		a->trans=0;
		noeud_cnt++;
		return a;
	}


	struct arbre_hash_trans02* new_trans() {
		struct arbre_hash_trans02* a;
		a=(struct arbre_hash_trans02*)
			malloc(sizeof(struct arbre_hash_trans02));
		if(!a){
			fatal_error("malloc fail to hash tree\n");
		}
		a->c='\0';
		a->arr=NULL;
		a->suivant=NULL;
		trans_cnt++;
		return a;
	}


	void free_arbre_hash_trans02(struct arbre_hash_trans02* t) {
		struct arbre_hash_trans02* tmp;
		while (t!=NULL) {
			free_arbre_hash02(t->arr);
			tmp=t;
			t=t->suivant;
			free(tmp);
		}
	}


	void free_arbre_hash02(struct arbre_hash02* a) {
		if (a==NULL) return;
		free_arbre_hash_trans02(a->trans);
		free(a);
	}
	void set_limit(int li) {
		limit  = li;
	}

	struct arbre_hash02* insert(unsigned int* s,int pos,
		struct arbre_hash02* noeud) {
		if (noeud==NULL) {
		  fatal_error("Erreur dans fonction inserer\n");
		}
		if (!s[pos]){
            if (pos > sequenceMaxDepth ) sequenceMaxDepth = pos;
           return(noeud);
           }

		struct arbre_hash_trans02  **t= &noeud->trans;
		while(*t){
			if((*t)->c == s[pos])
				return insert(s,pos+1,(*t)->arr);
			if( (*t)->c > s[pos]) break;
			t = &((*t)->suivant);
		}
		if(modeExplore){
			struct arbre_hash_trans02 *tt=new_trans();
			tt->c=s[pos];
			tt->arr=new_noeud();
			tt->suivant=*t;
			*t=tt;
			return insert(s,pos+1,tt->arr);
		}
		return ((struct arbre_hash02*)-1);
	}


	int put(unsigned int* s,void *value) {
		modeExplore = 1;
		struct arbre_hash02* noeud = insert(s,0,racine);
		if(noeud->final == -1){
			noeud->final=N++;
			noeud->value = (uintptr_t)value;
		}
		if(N < limit) 	return noeud->final;
		fatal_error("Too many hash elements\n");
	}

	int find(unsigned int* s,int depth)
	{
		modeExplore = 0;
		struct arbre_hash02* noeud = scanAvecDepth(s,0,racine,depth);
		if(noeud == (struct arbre_hash02 *) -1) return(-1);
		lastCheckNoeud = noeud;
		return(noeud->final);
	}
	struct arbre_hash02* scanAvecDepth(unsigned int* s,int pos,
		struct arbre_hash02* noeud,int depth) {
		if (noeud==NULL) {
		  fatal_error("Erreur dans fonction inserer\n");
		}
		if(noeud->final) u_printf("find %d\n",noeud->final);
		if (!depth) return((struct arbre_hash02*)-1);

		struct arbre_hash_trans02  **t= &noeud->trans;
		while(*t){
			if((*t)->c == s[pos])
				return scanAvecDepth(s,pos+1,(*t)->arr,depth-1);
			if( (*t)->c > s[pos]) break;
			t = &((*t)->suivant);
		}
		return ((struct arbre_hash02*)-1);
	}



	int insertWordAndInc(unsigned int *s,int sz)
	{
        int i;
		for(i = 0; i < sz;i++) cbuff[i] = s[i];
		cbuff[i] = 0;
		modeExplore = 1;
		struct arbre_hash02* noeud = insert(cbuff,0,racine);
		if(noeud->final == -1){
			noeud->final=N++;
			noeud->value = 0;
		}
		noeud->value++;
		if(N < limit) 	return noeud->final;
		fatal_error("Too many hash elements\n");
	}
	//
	//	must used after check
	//
	void *getCheckValue()
	{
		return((void *)lastCheckNoeud->value);
	}
	//
	// use carefully after check
	//
	void *getValue(unsigned int *s)
	{
		struct arbre_hash02* noeud = insert(s,0,racine);
		return (void *)noeud->value;
	}
	unsigned int getIndex(unsigned int  *s)
	{
		struct arbre_hash02* noeud = insert(s,0,racine);
		return noeud->final;
	}
	unsigned int cbuff[1024];
	void explore_tout_leaf(release_f ppr)
	{
		explore_leaf(racine,1,ppr);
	}
	void explore_leaf(struct arbre_hash02* noeud,int pos,release_f ppr)
	{
		if(noeud->final != -1){
			cbuff[0] = pos;
			cbuff[pos] = 0;
			if(ppr) (*ppr)((void *)cbuff,(void *)noeud->final,(void *)noeud->value);
		}
		struct arbre_hash_trans02  **t= &noeud->trans;
		while(*t){
			cbuff[pos] = (*t)->c;
			explore_leaf((*t)->arr,pos+1,ppr);
			t = &((*t)->suivant);
		}
	}
	void save_tout_leaf(U_FILE *f)
	{
		explore_to_save(racine,0,f);
	}
	void explore_to_save(struct arbre_hash02* noeud,int pos,U_FILE *f)
	{
		if(noeud->final != -1){
			cbuff[pos] = 0;
			u_fprintf(f,"%S\n",cbuff);
		}
		struct arbre_hash_trans02  **t= &noeud->trans;
		while(*t){
			cbuff[pos] = (*t)->c;
			explore_to_save((*t)->arr,pos+1,f);
			t = &((*t)->suivant);
		}
	}
    //make string table
	// after use this function , on doit call release_value()
	unichar **make_strPtr_table(int **v)
	{
		tab = (unichar **)malloc(N*sizeof(unichar *));
		if (tab==NULL) {
		   fatal_alloc_error("make_strPtr_table");
		}
		value_tab = (int *)malloc(N*sizeof(int *));
		if (value_tab==NULL) {
		    fatal_alloc_error("make_strPtr_table");
		}

		(*v) = value_tab;
		explore_to_get(racine,1);
		return(tab);
	}


	void release_value()
	{
		int i;
		for(i = 0; i< N;i++) free(tab[i]);
		free(value_tab);
		free(tab);
		tab = 0;
		value_tab = 0;
	}

	void explore_to_get(struct arbre_hash02* noeud,int pos)
	{
	    int i;
		if(noeud->final != -1){
			cbuff[0] = pos;
			tab[noeud->final] =
				(unichar *)malloc(
				(pos+2) * sizeof(unichar));
			for(i=0; i < pos; i++)	tab[noeud->final][i]=cbuff[i];
			tab[noeud->final][i] = 0;
			value_tab[noeud->final] = (int)noeud->value;
		}
		struct arbre_hash_trans02  **t= &noeud->trans;
		while(*t){
			cbuff[pos] = (*t)->c;
			explore_to_get((*t)->arr,pos+1);
			t = &((*t)->suivant);
		}
	}
	int insertLink(struct simple_link *cur,
		struct arbre_hash02* noeud) {
		if (noeud==NULL) {
		  fatal_error("Erreur dans fonction inserer\n");
		}
		if (!cur){
			if(noeud->final == -1)	noeud->final=N++;
			if(N < limit)	return noeud->final;
			fatal_error("Too many hash elements\n");
		}

		struct arbre_hash_trans02  **t= &noeud->trans;
		unichar cc = (unichar)(cur->n);
		while(*t){
			if((*t)->c == cc )
				return insertLink(cur->suivant,(*t)->arr);
			if( (*t)->c > cc ) break;
			t = &((*t)->suivant);
		}
		if(modeExplore){
			struct arbre_hash_trans02 *tt=new_trans();
			tt->c=cc;
			tt->arr=new_noeud();
			tt->suivant=*t;
			*t=tt;
			return insertLink(cur->suivant,tt->arr);
		}
		return -1;
	}
	int putLink(struct simple_link *v)
	{
		modeExplore = 1;
		return( insertLink(v,racine));
	}
};

//
//	memory handler
//
//

struct pageElement {
	int pageNum;
	int cnt;
	unsigned char *pageAddr;
	unsigned int *used;
	struct pageElement *next;
};
class pageHandle {
private:
	struct pageElement *head;
	struct pageElement *tail;
	struct pageElement *curr;	// indicate current used page
	int pgSize;		// size of page
	int pgESize;	// size of elements
	unsigned int assignedCount;	// flag and count for repete using
	int pgCnt;		// assined page number
public:
	int pgEMcnt;	// max element cnt
	unsigned int counter;	// total assigned element count
	uintptr_t *addrMap;

	pageHandle(){
		head = 0;
		tail = 0;
		curr = 0;
		pgSize = 0;
		pgCnt = 0 ;
		counter=0;
		addrMap = 0;
		assignedCount = 0;
	};
	~pageHandle(){
		struct pageElement *tmp;
		while(head){
			tmp = head->next;
			if(head->pageAddr) free(head->pageAddr);
			if(head->used) free(head->used);
			head = tmp;
		}
		if(addrMap) free(addrMap);
	};

	void setSz(int pageSz,int elementSize)
	{
		if(!elementSize || !pageSz) {
		   fatal_error("Internal error in setSz\n");
		}
		pgSize = pageSz;
		pgEMcnt = pageSz/elementSize;
		pgESize = elementSize;
	}
	unsigned char *get()
	{
		if(assignedCount){
			if(assignedCount > counter){
				if(curr->cnt >= pgEMcnt)
					curr = curr->next;
				counter++;
				return(
				curr->pageAddr + curr->cnt++*pgESize);
			}
		} else {
			if(curr){
				if(curr->cnt < pgEMcnt){
					counter++;
					return(
					curr->pageAddr + curr->cnt++*pgESize);
				}
			}
		}
		struct pageElement *t = (struct pageElement *)
			malloc(sizeof(struct pageElement));
		t->cnt = 0;
		t->used = 0;
		t->next = 0;
		t->pageNum = pgCnt++;
		t->pageAddr = (unsigned char *)malloc(pgSize);
		if(!t->pageAddr) fatal_error("mem alloc for pagehandle fail\n");
		if(!curr){
			head = tail = curr = t;
		} else {
			tail->next = t;
			tail = t;
			curr = tail;
		}
		if(assignedCount){
			assignedCount += pgEMcnt;
			if(assignedCount > 0x7fffffff)
				fatal_error("too big\n");
		}
		return(pageHandle::get());
	};
	int getSize(){ return(counter);};
	void getAddrMap()
	{
		addrMap = (uintptr_t *)
			malloc(sizeof(uintptr_t)*pgCnt);
		struct pageElement *t = head;
		int pgIdx = 0;
		while(t){
			addrMap[pgIdx++] =(uintptr_t) t->pageAddr;
			t = t->next;
		}
	}
	void addMapOut(uintptr_t *addMap)
	{
		struct pageElement *t = head;
		int pgIdx = 0;
		while(t){
			addMap[pgIdx++] =(uintptr_t) t->pageAddr;
			t = t->next;
		}
	}
	unsigned char * getAddr(int idx){
		struct pageElement **scan = &head;

		while((*scan)){
			if(idx < (*scan)->cnt)
				return( (*scan)->pageAddr+
						pgESize * idx);
			idx -= pgEMcnt;
			scan = &(*scan)->next;
		}
		return(0);
	}
	int getIndex(unsigned char *addr)
	{
		struct pageElement **scan = &head;
		int offset;

		while((*scan)){
			if( addr >= (*scan)->pageAddr){
				offset = (int)(addr - (*scan)->pageAddr);
				if(offset < pgSize){
					return( (*scan)->pageNum * pgEMcnt+
						offset/pgESize);
				}
			}
			scan = & (*scan)->next;
		}
		return(0);
	}
	//
	//
	//
	void resetAssign()
	{
		if(!counter) return;
		if(!assignedCount){
			assignedCount =
				(counter + pgEMcnt -1) / pgEMcnt;
			assignedCount *= pgEMcnt;
		}
		for( curr = head; curr ; curr = curr->next)
			curr->cnt = 0;
		counter = 0;
		curr = head;
		pgCnt = 0;
	}
	//
	//
	//
	int tableOutToFile(U_FILE *f)
	{
		struct pageElement *wp = head;
		unsigned int sum = 0;
		while(wp){
			sum += wp->cnt;
			fwrite(wp->pageAddr,pgESize * wp->cnt,1,f);
			wp = wp->next;
		}
		if(sum != counter) fatal_error("illegal counter at page handler\n");
		return(counter);
	}
};



#endif
