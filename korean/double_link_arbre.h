 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

struct double_link_state {
	   struct double_link_state *parent;
	   unsigned short c;
	   unsigned short depth;
       int final;
       struct double_link_trans* trans;
};
struct double_link_trans {
   struct double_link_state* arr;
   struct double_link_trans* suivant;
};
class terminalNoeud : public link_tables<struct double_link_state *>
{
public:
	terminalNoeud(){};
	~terminalNoeud(){};
};
class double_arbre_string0 {
	pageHandle nodes;
	pageHandle trans;
	terminalNoeud	terminal_noeuds;	// save terminal noeud
    int N;
	int limit;
    struct double_link_state* racine;
	int modeExplore;
	struct double_link_state **Ttable;
	FILE *tabOfile;
public:
	unsigned short **tab;
	double_arbre_string0(){
		nodes.setSz(sizeof(struct double_link_state)*4096,
			sizeof(struct double_link_state));
		trans.setSz(sizeof(struct double_link_trans)*4096,
			sizeof(struct double_link_trans));
		racine = 0;
		N=0;
		limit = 0x100000;
		tab = 0;
		racine=new_noeud();
		tabOfile = 0;

	};
	~double_arbre_string0(){
		if(tab){ for(int i = 0; i < N;i++) if(tab[i]) free(tab[i]);
		free(tab);}
		if(Ttable) delete Ttable;
	};
	int size(){return(N);};
	struct double_link_state *getRacine(){ return(racine);};
	struct double_link_state* new_noeud() {
		struct double_link_state* a;
		a=(struct double_link_state*)nodes.get();
		if(!a) {
			fatal_error("malloc fail to hash tree");
		}
		a->parent = 0;
		a->c = 0;
		a->depth = 0;
		a->final=-1;
		a->trans=NULL;

		return a;
	}


	struct double_link_trans* new_trans() {
		struct double_link_trans* a;
		a=(struct double_link_trans*)trans.get();
		if(!a){
			fatal_error("malloc fail to hash tree");
		}
		a->arr=NULL;
		a->suivant=NULL;
		return a;
	}


	void set_limit(int li) {
		limit  = li;
	}

	struct double_link_state* 
	insert(unsigned short* s,int pos,struct double_link_state* noeud) {
		if (noeud==NULL) {
		  fatal_error("Erreur dans fonction inserer\n");
		}
		if (!s[pos]) return(noeud);

		struct double_link_trans  **t= &noeud->trans;
		while(*t){
			if((*t)->arr->c == s[pos]) 
				return insert(s,pos+1,(*t)->arr);
			if( (*t)->arr->c > s[pos]) break;
			t = &((*t)->suivant);
		}
		if(modeExplore){
			struct double_link_trans *tt=new_trans();
			tt->arr=new_noeud();
			tt->arr->c = s[pos];
			tt->arr->depth = pos+1;
			tt->arr->parent = noeud;
			tt->suivant=*t;
			*t=tt;
			return insert(s,pos+1,tt->arr);
		}
		return (struct double_link_state*)-1;
	}


	int put(unsigned short* s) {
		modeExplore = 1;
		struct double_link_state* noeud = insert(s,0,racine);
		if(noeud->final == -1)	noeud->final=N++;
		if(N >= limit) fatal_error("Too many hash element");
      return noeud->final;
	}
	int putWithNum(unsigned short* s,int n) {
		modeExplore = 1;
		struct double_link_state* noeud = insert(s,0,racine);
		if(noeud->final == -1){
			noeud->final= n;
			terminal_noeuds.insertAtTable(noeud);
			N++;
		} 
		if(N >= limit) fatal_error("Too many hash elements\n");
      return noeud->final;
	}

	int check(unsigned short* s)
	{
		modeExplore = 0;
		struct double_link_state* noeud = insert(s,0,racine);
		if(noeud == (struct double_link_state *)-1)	
			return -1;
		return(noeud->final);
	}
#define CBUFF_SIZE	1024
	unsigned short cbuff[CBUFF_SIZE];

	int makeTableTNoeud()
	{
		int i;

		int tableSz = nodes.getSize();
		Ttable = new struct double_link_state *[N];
		if(!Ttable) fatal_error("mem alloc fail for terminal nodes\n");
		struct double_link_state *base;
		for( i = 0; i < tableSz;i++){
			base = (struct double_link_state *)nodes.getAddr(i);
			if(base->final == -1) continue;
			Ttable[base->final] = base;
		}
		return(N);	
	}
	
	unsigned short *getOneItemWithIndex(int num)
	{
		int pos;
		struct double_link_state *csp;
		if(num > N) fatal_error("illegal terminal index\n");
		pos = CBUFF_SIZE - 1;
		cbuff[pos] = 0;
		csp = Ttable[num];
		if(csp == 0) return(&cbuff[pos]);
		while(csp->parent){
			cbuff[--pos] = csp->c;
			csp = csp->parent;
		}
		return(&cbuff[pos]);
	}

	void releaseTable()
	{
		delete Ttable;
		Ttable = 0;
	}
	void explore_tout_leaf(release_f ppr)
	{
		struct double_link_trans  **t= &racine->trans;
		if(racine->final != -1){
			cbuff[0] = 0;
			cbuff[1] = 0;
			if(ppr) (*ppr)((void *)cbuff,(void *)racine->final,
				(void *)0);
		}
		while(*t){
			explore_leaf((*t)->arr,0,ppr);
			t = &((*t)->suivant);
		}
	}

	void explore_leaf(struct double_link_state* noeud,int pos,release_f ppr)
	{
		cbuff[pos] = noeud->c;
		if(noeud->final != -1){
			cbuff[0] = pos;
			cbuff[pos] = 0;
			if(ppr) (*ppr)((void *)cbuff,(void *)noeud->final,
				(void *)0);
		}
		struct double_link_trans  **t= &noeud->trans;
		while(*t){
			explore_leaf((*t)->arr,pos+1,ppr);
			t = &((*t)->suivant);
		}
	}
    //make string table 
	// after use this function , on doit call release_value()
	unsigned short **make_strPtr_table()
	{
		FILE *sf = tabOfile;
		tabOfile = 0;
		tab = (unsigned short **)malloc(N*sizeof(unsigned short *));
		if(!tab) fatal_error("memory alloc fail\n");
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
	void out_to_file(FILE *f)
	{
		unsigned short **stab = tab;
		tab = 0;
		tabOfile = f;
		u_fprintf(tabOfile,"%d\n",N);
		explore_to_get(racine,0);
		tabOfile = 0;
		tab = stab;
	}
	void out_to_file_data(FILE *f, int headOutFlag)
	{
		unsigned short **stab = tab;
		tab = 0;
		tabOfile = f;
		if(headOutFlag)
			strFileHeadLine(tabOfile,N);	// etc.cpp
		struct double_link_trans  **t= &racine->trans;
		while(*t){
			explore_to_get((*t)->arr,0);
			t = &((*t)->suivant);
		}
		tabOfile = 0;
		tab = stab;
	}
	void explore_to_get(struct double_link_state* noeud,int pos)
	{
	    int i;
		cbuff[pos] = noeud->c;
		if(noeud->final != -1){
			if(tab){
				cbuff[0] = pos;
				tab[noeud->final] = 
					(unsigned short *)malloc(
					(pos+1) * sizeof(unsigned short));
				for(i=0; i <= pos; i++)	tab[noeud->final][i]=cbuff[i];
				tab[noeud->final][i] = 0;
			}
			if(tabOfile)
			{
				cbuff[pos+1] = 0;
				u_fprintf(tabOfile,"%S\n",cbuff);
			}
		}
		struct double_link_trans  **t= &noeud->trans;
		while(*t){
			explore_to_get((*t)->arr,pos+1);
			t = &((*t)->suivant);
		}
	}

	//
	//	data out by sorted order and table which have not sorted order
	//
	unsigned int *sortedmap;
	int accessOrderCnt;
	void out_to_file_data1(FILE *f,unsigned int *sorted)
	{
		accessOrderCnt = 0;
		tabOfile = f;
		sortedmap = sorted;
		struct double_link_trans  **t= &racine->trans;
		while(*t){
			explore_to_get_avec_unsorted((*t)->arr,0);
			t = &((*t)->suivant);
		}
		tabOfile = 0;
	}

	void explore_to_get_avec_unsorted(struct double_link_state* noeud,int pos)
	{
		cbuff[pos] = noeud->c;
		if(noeud->final != -1){
			sortedmap[accessOrderCnt++] = noeud->final; 
			cbuff[pos+1] = 0;
			u_fprintf(tabOfile,"%S\n",cbuff);
		}
		struct double_link_trans  **t= &noeud->trans;
		while(*t){
			explore_to_get_avec_unsorted((*t)->arr,pos+1);
			t = &((*t)->suivant);
		}
	}
	int insertLink(struct simple_link *cur,
		struct double_link_state* noeud) 
	{
		if (noeud==NULL) {
		  fatal_error("Erreur dans fonction inserer\n");
		}
		if (!cur){
			if(noeud->final == -1)	noeud->final=N++;
			if(N >= limit) fatal_error("Too many hash element"); 
         return noeud->final;
		}

		struct double_link_trans  **t= &noeud->trans;
		unsigned short cc = (unsigned short)(cur->n);
		while(*t){
			if((*t)->arr->c == cc ) 
				return insertLink(cur->suivant,(*t)->arr);
			if( (*t)->arr->c > cc ) break;
			t = &((*t)->suivant);
		}
		if(modeExplore){
			struct double_link_trans *tt=new_trans();
			tt->arr=new_noeud();
			tt->arr->c = cc;
			tt->suivant=*t;
			*t=tt;
			return insertLink(cur->suivant,tt->arr);
		}
		return -1;
	}
	int putLink(struct simple_link *v)
	{
		modeExplore = 1;
		struct double_link_trans  **t= &racine->trans;
		unsigned short cc = (unsigned short)(v->n);
		while(*t){
			if((*t)->arr->c == cc ) 
				return insertLink(v->suivant,(*t)->arr);
			if( (*t)->arr->c > cc ) break;
			t = &((*t)->suivant);
		}
		struct double_link_trans *tt=new_trans();
		tt->arr=new_noeud();
		tt->arr->c = cc;
		tt->suivant=*t;
		*t=tt;
		return insertLink(v->suivant,tt->arr);
	}
	void
	tableLoadFromFile(FILE *a,int cnt,int szOfLoad)
	{
		unsigned char *map = new unsigned char [szOfLoad];
		if(!fread(map,szOfLoad,1,a)) fatal_error("read error\n");
		Ttable  = new struct double_link_state*[cnt];
		struct double_link_state* noeud;
		unsigned short *wp = (unsigned short *)map;
		unsigned short *curoffset = wp;
		unsigned short *end = (unsigned short *)(map + szOfLoad);
		modeExplore = 1;
		while(wp < end) {
			if(*wp == 0x0a){
				*wp++ = 0;
				if(*curoffset == 0) fatal_error("Format Error\n");
				noeud = insert(curoffset,0,racine);
				if(noeud->final == -1){
					Ttable[N] = noeud;
					noeud->final=N++;
				}
				else fatal_error("illegal value\n");
			
				curoffset = wp;
			} else  if(*wp == 0x0d) {
				*wp++ = 0;
			} else
				wp++;			
		};
		if( N != cnt) fatal_error("illegal table size\n");
		delete map;
	}
};
