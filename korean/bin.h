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
//
//	compress files .bin
//	.ref	structure for linked
//	.inf	infomation of lingustics
//	.aut	initial states with nom and offset of start point
//

#ifndef RELOCATABLE_BIN_DEF
#define RELOCATABLE_BIN_DEF
#include "UnusedParameter.h"
#include "String_hash2.h"
#include "etc.h"
#include "pro_type.h"
#include "Error.h"



#define TYPE_BIN_RACINE		0x8000	// racine or suffixe
#define TYPE_BIN_LINKED		0x4000	// linked or not linked with suffixe
#define TYPE_DECODE_CHAR	0x2000	// decode by the unite of char or syllabe
#define TYPE_BIN_CONT		0x1000	// exist divided part

//
//	structure for convert the form of INF from string to structure
//	exist .ref file
//
#define INF_ENCODED				0x80	// codage

#define INF_CONTROL_MASK		0x70
#define INF_CTL_NEXTTOK			0x10	// read next token
#define INF_CTL_CHK_A_NEXT		0x20 // if current char is blanc then read next token

#define INF_IGNO_INF			0x08	// not use the value of inf
#define INF_IGNO_SUF			0x04	// not use the suffixe offset
#define INF_NOABS_SUF			0x02	// not linked suf
#define INF_NAME_STR			0x01	// use for save a string
//#pragma pack(push, temp_pragma_save)

#ifdef _MSC_VER
# pragma pack( push, packing )
# pragma pack( 1 )
# define PACK_STRUCT_BH
#else
#if defined( __GNUC__ )
# define PACK_STRUCT_BH __attribute__((packed))
#else
# error you must byte-align these structures with the appropriate compiler directives
#endif
#endif

struct INF_raw {
	unsigned char flag;
	uintptr_t  sufIdx;
	uintptr_t infIdx;	// lingusitic informations
}PACK_STRUCT_BH;
//#pragma pack(1)
/*
		bin structure
		head
		table of initial states
		table of string for initial state and name of demanded state
		table of infomation
		data structure for transductor
 */

class binHead0 {
public:
	unichar markUnicode;
	unichar flag;
	int size_ref;	// if flag & TYPE_BIN_LINKED
	int size_str;
	int size_inf;
	int size_bin;

	int cnt_auto;	// num of initial
	int cnt_suf;	// num of demanded suffixe not yet located value
	int cnt_inf;
#define IMAGE_HEAD_SZ		0x18

	binHead0()
	{
		flag = 0;
		size_bin  = 0;
		size_ref = 0;
		size_inf = 0;
		size_str = 0;
		cnt_auto = 0;
		cnt_suf = 0;
		cnt_inf  = 0;
	};
	~binHead0(){};
	void writeAtFile(U_FILE *f)
	{

		outbytes2(0xfeff,f);
		outbytes2(flag,f);
		outbytes4(size_ref,f);
		outbytes4(size_inf,f);
		outbytes4(size_bin,f);
		outbytes2((unichar)size_str,f);
		outbytes2((unichar)cnt_auto,f);
		outbytes2((unichar)cnt_suf,f);
		outbytes2((unichar)cnt_inf,f);
	}
	void readFromFile(U_FILE *f)
	{
		unsigned short markUnicode = inbytes2(f);
		if(markUnicode != 0xfeff){
		    error("Marked 0x%x (0x%x)\n",markUnicode, proType.uniMark);
			 fatal_error("not bin file");
		}
		flag		= inbytes2(f);
		size_ref	= inbytes4(f);
		size_inf	= inbytes4(f);
		size_bin	= inbytes4(f);
		size_str	= inbytes2(f);
		cnt_auto	= inbytes2(f);
		cnt_suf		= inbytes2(f);
		cnt_inf		= inbytes2(f);
//debug("readfromfil:flag=%d, cntAuto=%d, cntSuf=%d cntinf=%d\n",flag, cnt_auto,cnt_suf, cnt_inf);
//debug("readfromfil:sizeRef=%d, sizeInf=%d, sizeBin=%d\n",size_ref, size_inf, size_bin);
	}
} PACK_STRUCT_BH;
// #pragma pack(pop,temp_pragma_save)

#ifdef _MSC_VER
# pragma pack( pop, packing )
#endif


struct arbre_dico_with_depth {
	   struct simple_link* arr;
       struct arbre_dico_trans_with_depth* trans;
       int offset; // this value will be used to give to this node an adress in the .BIN file
       int n_trans;
       int hash_number;
};
struct arbre_dico_trans_with_depth {
       unichar c;
       struct arbre_dico_with_depth* noeud;
       struct arbre_dico_trans_with_depth* suivant;
	   unichar depth;
};
#define INSERT_MODE  1
#define MAX_ARBRES	1024
class arbre_string2 {
	class pageHandle transitions;
	class pageHandle nodes;
public:
	struct arbre_dico_with_depth *racine[MAX_ARBRES];

	int search_arbre_mode;
//	int nodeCnt[MAX_ARBRES];
//	int transCnt[MAX_ARBRES];
	int arbreCnt;
	int maxDepth;
	int cArbreIdx;
	class arbre_string0	 racName;
	class arbre_string0	 sufName;
	class arbre_string0	 baseInf;
	class arbre_string0  sequence_info;

    Encoding encoding_output ;
    int bom_output ;
    int mask_encoding_compatibility_input ;

	arbre_string2(){

		maxDepth = 0;
		arbreCnt = 0;
//		for(int i = 0; i < MAX_ARBRES;i++) nodeCnt[i] = 0;
//		for(i = 0; i < MAX_ARBRES;i++) transCnt[i] = 0;
		nodes.setSz(sizeof(struct arbre_dico_with_depth)*1024,
			sizeof(struct arbre_dico_with_depth));
		transitions.setSz(
			sizeof(struct arbre_dico_trans_with_depth)*1024
			,sizeof(struct arbre_dico_trans_with_depth));
        encoding_output = DEFAULT_ENCODING_OUTPUT;
        bom_output = DEFAULT_BOM_OUTPUT;
        mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;

	};
	~arbre_string2(){
		struct arbre_dico_with_depth* base;
		unsigned int i;
		if(!nodes.addrMap){
			nodes.getAddrMap();
			for( i = 0; i < nodes.counter ;i++){
				base = (struct arbre_dico_with_depth*)
				(nodes.addrMap[ i/ nodes.pgEMcnt] +
				(i % nodes.pgEMcnt) *
				sizeof(struct arbre_dico_with_depth));
				if(!base->arr) continue;
				struct simple_link* tmp , *l = base->arr;
					while (l!=NULL) {
						tmp=l;
						l=l->suivant;
						free(tmp);
					}

			}
		}
	};

    void setEncoding(Encoding encoding_outputSet,int bom_outputSet,int mask_encoding_compatibility_inputSet){
        encoding_output = encoding_outputSet;
        bom_output = bom_outputSet;
        mask_encoding_compatibility_input = mask_encoding_compatibility_inputSet;
    }

	uintptr_t getRacine(int idx){
		if(idx < arbreCnt){
			return((uintptr_t)racine[idx]);
		}
		return(0);
	}
	int getArbreCnt(){return(arbreCnt);};
	int new_arbre(unichar *name){
		racName.put(name);
		racine[arbreCnt] =
			(struct arbre_dico_with_depth *)new_node();
		return(arbreCnt++);
	}
	struct arbre_dico_with_depth* new_node() {
		struct arbre_dico_with_depth* a;
		a=(struct arbre_dico_with_depth *)nodes.get();
		a->arr=NULL;
		a->offset=-2;
		a->trans=NULL;
		a->n_trans = 0;
		return a;
	}


	struct arbre_dico_trans_with_depth* new_trans() {
		struct arbre_dico_trans_with_depth* t;
		t=(struct arbre_dico_trans_with_depth*)transitions.get();
		t->c='\0';
		t->noeud=NULL;
		t->suivant=NULL;
		t->depth = (unichar)-1;
		return t;
	}
	unichar *scaningBuff;

	//
	// for racine or suffixe with following suffixe indicate
	//
	struct arbre_dico_with_depth* put(int pos,struct arbre_dico_with_depth* noeud)
	{
		if (scaningBuff[pos]=='\0'){
			if(maxDepth < pos) maxDepth = pos;
			return noeud;
		}

		unichar c = scaningBuff[pos];
		struct arbre_dico_trans_with_depth**t=&noeud->trans;
		while((*t)){
			if((*t)->c == c) return(put(pos+1,(*t)->noeud));
			if((*t)->c > c) break;
			t = &(*t)->suivant;
		}
		struct arbre_dico_trans_with_depth* ptr =new_trans();
		ptr->c=c;
		ptr->suivant=(*t);
		ptr->noeud=new_node();
		(*t) =ptr;
		(noeud->n_trans)++;
		return(put(pos+1,ptr->noeud));
	}
	//
	//	construction transducter for suffixe in the middle of word
	//
	struct arbre_dico_with_depth* putTr(int pos,struct arbre_dico_with_depth* noeud)
	{
		unichar c = scaningBuff[pos];
		struct arbre_dico_trans_with_depth**t=&noeud->trans;
		while((*t)){
			if((*t)->c == c){
				if(c == 0){
					if(maxDepth < (pos +1)) maxDepth = pos +1;
					return((*t)->noeud);
				}
				return(putTr(pos+1,(*t)->noeud));
			}
			if((*t)->c > c) break;
			t = &(*t)->suivant;
		}
		struct arbre_dico_trans_with_depth* ptr =new_trans();
		ptr->c=c;
		ptr->suivant=(*t);
		ptr->noeud=new_node();
		(*t) =ptr;
		(noeud->n_trans)++;
		if(c == 0){
			if(maxDepth < (pos +1)) maxDepth = pos +1;
			return(ptr->noeud);
		}
		return(putTr(pos+1,ptr->noeud));
	}
	unichar ccbuf[1024];
	void insert(unichar *contenu,
		unichar *suff,
		unichar *info,int no_arbre)
	{
		if(no_arbre >= arbreCnt) fatal_error("error : bad request tree Id\n");

		cArbreIdx = no_arbre;
		scaningBuff = contenu;

		struct arbre_dico_with_depth *noeud;

		if(*contenu)
			noeud = put(0,racine[no_arbre]);
		else
			noeud = racine[no_arbre];
		if(suff && (*suff != 0) )	sufName.put(suff);

		ccbuf[0] = 0;
		u_strcat(ccbuf,suff);
		u_strcat(ccbuf,".");
		u_strcat(ccbuf,info);
		unsigned int N= baseInf.put(ccbuf);

		struct simple_link **tarr = &noeud->arr;
		while(*tarr) {
			   if((*tarr)->n == N) return;
			   if((*tarr)->n > N) break;
		   tarr = &((*tarr)->suivant);
		}
		struct simple_link* l_tmp=
			(struct simple_link *)malloc(sizeof(struct simple_link));
		l_tmp->n = N;
		l_tmp->suivant = (*tarr);
		(*tarr) = l_tmp;
	}
	//
	//	insert states from the state indicated
	//
	struct arbre_dico_with_depth *
	insertSerialElements(unichar *contenu,
		unichar *suff,
		unichar *info,int no_arbre,
		struct arbre_dico_with_depth *st_node)
	{
		if(no_arbre >= arbreCnt) fatal_error("error in tree\n");
		cArbreIdx = no_arbre;
		scaningBuff = contenu;

		struct arbre_dico_with_depth *noeud;

		if(!contenu ||(*contenu == 0) ){
			noeud = (st_node) ? st_node: racine[no_arbre];
		} else {
			if(st_node)
				noeud = put(0,st_node);
			else
				noeud = put(0,racine[no_arbre]);
		}
		ccbuf[0] = 0;
		if(suff && (*suff != 0) ){
			if(u_strcmp(suff,"null"))
				sufName.put(suff);
			u_strcat(ccbuf,suff);
		}
		u_strcat(ccbuf,".");
		u_strcat(ccbuf,info);
		unsigned int N= baseInf.put(ccbuf);

		struct simple_link **tarr = &noeud->arr;
		while(*tarr) {
			   if((*tarr)->n == N) return(noeud);
			   if((*tarr)->n > N) break;
		   tarr = &((*tarr)->suivant);
		}
		struct simple_link* l_tmp=
			(struct simple_link *)malloc(sizeof(struct simple_link));
		l_tmp->n = N;
		l_tmp->suivant = (*tarr);
		(*tarr) = l_tmp;
		return(noeud);
	}

	void explore_tout_leaf(int idx,release_f ppr)
	{
		if(idx < arbreCnt){
			explore_leaf(0,racine[idx],ppr);
			return;
		}
		fatal_error("illegal auto index demand\n");
	}
	uintptr_t cbuff[1024];
	void explore_leaf(int depth,struct arbre_dico_with_depth* noeud,release_f ppr)
	{
		if(depth > 1024) return;
		struct arbre_dico_trans_with_depth  *t= noeud->trans;
		if( !t || noeud->arr) {
			cbuff[depth] = 0;
			if(ppr) (*ppr)((void *)depth,(void *)cbuff,(void *)depth);
		}
		while(t){
			cbuff[depth] = (uintptr_t)t;
			explore_leaf(depth+1,t->noeud,ppr);
			t = t->suivant;
		}
	}
	void explore_for_access_check(
		struct arbre_dico_with_depth* noeud)
	{
		noeud->offset = -1;
		struct arbre_dico_trans_with_depth  *t= noeud->trans;
		while(t){
			explore_for_access_check(t->noeud);
			t = t->suivant;
		}
	}

	int taille_de_sauve;
	// check exist node
	//	and calcul offset
	//
	void check_exist_sur_path()
	{
		struct arbre_dico_with_depth* base;
		int enCnt = 0;
		int i;
		int infoNodeCnt = 0;	// a node which has informations
		int noInfoNodeCnt =0;	// a node without informations

		u_printf("total %d node ==>",nodes.counter);
		for( i = 0;  i < arbreCnt;i++)
			explore_for_access_check(racine[i]);
		taille_de_sauve = 0;
		for( i = 0; i < (int)nodes.counter ;i++){
			base = (struct arbre_dico_with_depth*)
			(nodes.addrMap[ i/ nodes.pgEMcnt] +
			(i % nodes.pgEMcnt) * sizeof(struct arbre_dico_with_depth));
			if(base->offset == -2 ){
				enCnt++; // not access noeud
				continue;
			}
			if(base->offset != -1)
				fatal_error("illegal information on the tree link\n");
			//
			//	calcule the size of a node
			//
			base->offset = taille_de_sauve;
			if(base->arr){
				infoNodeCnt++;
				taille_de_sauve+=3;
				base->hash_number = sequence_info.putLink(base->arr);
			} else
				noInfoNodeCnt++;
			taille_de_sauve += base->n_trans*5 + 2;
		}
		u_printf("%d node delete\n",enCnt);
		u_printf("node with info %d, node without info %d\n",infoNodeCnt,noInfoNodeCnt);
		u_printf("size of tree is 0x%08x(%d)\n",taille_de_sauve,taille_de_sauve);
	}

	void minimize_tree() {
		int i;
		u_printf("Minimizing...                      \n");
		//
		//
		//init_tab_by_hauteur();
		nodes.getAddrMap();
		transitions.getAddrMap();
		int H;
		int t =-1;
		for(i = 0; i < arbreCnt;i++){
			H=sort_by_height(racine[i]);
			if( t < H ) t = H;
		}
		H = t;
		fill_hash_number();
//		float z;
//		for (int k=0;k<=H;k++) {
//			int size=convert_list_to_array(k);
//			quicksort(0,size-1);
//			mfusionner(size);
//			z=(float)(100.0*(float)(k)/(float)H);
//			if (z>100.0) z=100.0;
//			printf("%2.0f%% completed...    %s",z,CR);
//		}
		mfusionner(H);
		u_printf("Minimization done.                     \n");
	}



	int sort_by_height(struct arbre_dico_with_depth* n)
	{
		if (n==NULL) fatal_error("NULL error in sort_by_height\n");
		if (n->trans==NULL) return(0); // if the node is a leaf
		struct arbre_dico_trans_with_depth * trans=n->trans;
		int maxD = -1;
		while (trans!=NULL) {
			trans->depth = (unichar)sort_by_height(trans->noeud);
		  if(maxD < trans->depth ) maxD = trans->depth ;
		 trans = trans->suivant;
		}
		return 1+maxD;
	}
	void fill_hash_number()
	{
		struct arbre_dico_with_depth* base;
		unsigned int i;
		u_printf("Fill hash number for information\n");
		for( i = 0; i < nodes.counter ;i++){
			base = (struct arbre_dico_with_depth*)
			(nodes.addrMap[ i/nodes.pgEMcnt] +
			(i % nodes.pgEMcnt) * sizeof(struct arbre_dico_with_depth));
			if(base->arr){
				base->hash_number = sequence_info.putLink(base->arr);
			}
		}
	}
	void mfusionner(int size) {
		struct arbre_dico_trans_with_depth* base;
		struct arbre_dico_trans_with_depth* scan;
		int etCnt = 0;
		int i,j;
		int totTrans = transitions.counter;
		int eleCnt = transitions.pgEMcnt;
      u_printf("total transition %d \n\n",totTrans);
		for(int depIdx = 0; depIdx < size; depIdx++)
		{
         u_printf("\rtraiting the %d depth nodes",depIdx);

			for( i = 0; i < totTrans - 1;i++){
				base = (struct arbre_dico_trans_with_depth*)(
					transitions.addrMap[i/eleCnt] +
				(i % eleCnt) * sizeof(struct arbre_dico_trans_with_depth));
				if(base->depth != depIdx) continue;
				for( j = i+1; j< totTrans;j++){
					scan = (struct arbre_dico_trans_with_depth*)(
						transitions.addrMap[ j/eleCnt] +
					(j % eleCnt )* sizeof(struct arbre_dico_trans_with_depth));
					if(scan->depth != depIdx) continue;
					if (compare_nodes(base,scan)==0){
						scan->noeud=base->noeud;
						scan->depth = (unichar)-1;	// mark delete
						etCnt++;
					}
				} // loop for compared trans
			} // loop for pivot trans
		}
      u_printf("%d transition delete\n",etCnt);
		check_exist_sur_path();
	}
	int
	compare_nodes(struct arbre_dico_trans_with_depth* a,
		struct arbre_dico_trans_with_depth* b) {
		if (a==NULL || b==NULL || a->noeud==NULL || b->noeud==NULL)
		fatal_error("Probleme dans compares_nodes\n");
// then, the hash numbers
if (a->noeud->arr!=NULL && b->noeud->arr==NULL) return -1;
if (a->noeud->arr==NULL && b->noeud->arr!=NULL) return 1;

if (a->noeud->arr!=NULL && b->noeud->arr!=NULL &&
/*if (*/a->noeud->hash_number != b->noeud->hash_number)
   return (a->noeud->hash_number - b->noeud->hash_number);
// and finally the transitions
a=a->noeud->trans;
b=b->noeud->trans;
while(a!=NULL && b!=NULL) {
   // if the unichars are different then nodes are different
   if (a->c != b->c) return (a->c - b->c);
   // if the unichars are equal and destination nodes are different...
   if (a->noeud != b->noeud) return (int)(a->noeud - b->noeud);
   a=a->suivant;
   b=b->suivant;
}
if (a==NULL && b==NULL) {
   // if the transition lists are equal
   return 0;
}
if (a==NULL) {
   // if the first list is shorter than the second
   return -1;
}
// if the first list is longuer then the second
return 1;
}


	//
	//	 head : unicode mark
	//   number of auto
	//	 number of request suffixe
	//	 array for the initial state of autos
	//	 array for the request suffixe
//	static int tailleKr;
	void toBinTr(char *fname,int racOrSuf)
	{
		U_FILE *bfile;

		char inf[1024];
		int i;
		class binHead0 tmpH;

		strcpy(inf,fname);
		strcat(inf,".aut");
		bfile=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,inf,U_WRITE);
		if (!bfile) {
			fatal_error("Cannot create the file %s\n",inf);
		}
		int racCount = racName.size();
		u_fprintf(bfile,"%d\n",racCount);
		unichar **a = racName.make_strPtr_table();
		for(i = 0; i < racCount;i++)
		{
			u_fprintf(bfile,"%S %d\n",&a[i][1]
				,racine[racName.check(&a[i][1])]->offset);
		}
		racName.release_value();
		u_fclose(bfile);

		strcpy(inf,fname);
		strcat(inf,".bin");
		bfile = u_fopen(BINARY,inf,U_WRITE);	// for distingue
		if(!bfile) fatal_error("binary file open error\n");

		tmpH.size_bin = taille_de_sauve;
		tmpH.flag |= (racOrSuf) ? 0: TYPE_BIN_RACINE;
		tmpH.writeAtFile(bfile);
		saveBin(bfile);
		u_fclose(bfile);
	}
	void saveBin(U_FILE *f)
	{
		struct arbre_dico_with_depth* base;
		unsigned int i;


		for( i = 0; i < nodes.counter ;i++){
			base = (struct arbre_dico_with_depth*)
			(nodes.addrMap[ i/nodes.pgEMcnt] +
			i % nodes.pgEMcnt * sizeof(struct arbre_dico_with_depth));
			if(base->offset == -2 ) continue;
			if(base->n_trans > 0x8000) fatal_error("Too many transitions\n");
				outbytes2(
				(base->arr==NULL) ?(unichar)(base->n_trans|0x8000)
						:(unichar)base->n_trans
					,f);

// we write the 2 bytes info about the node
			if (base->arr!=NULL)
				outbytes3((unsigned int)base->hash_number,f);

			struct arbre_dico_trans_with_depth* tmp= base->trans;
			while (tmp!=NULL) {
				outbytes2((unichar)tmp->c,f);
				outbytes3((unsigned int)tmp->noeud->offset,f);
				tmp=tmp->suivant;
			}
		}
	}



};

//
//	load .bin .inf .suf .aut
//
//
//
struct sufptr {
	class explore_bin0 *bin;
	int offset;
	unichar *w;
};
typedef int (*actDansBin)(class explore_bin0 *,
						   unichar *arg0,int depth,int inf,int sIdx);
class explore_bin0 {
public:
	class binHead0 head;
	unichar *INF;
	unsigned char  *BIN;
	unsigned char  *REF;
	int margin_offset;
	unichar **SUF;
	unichar **AUT;
	unsigned int *autoffset;
	unsigned int *sufoffset;
	char *name;
//	int offsetINF;


	actDansBin actFuncForFinal;
	actDansBin actFuncForInfo;	// fonction when we find the final etat
	explore_bin0(){
		name = 0;
		BIN = 0;
		actFuncForInfo = 0;
		actFuncForFinal = 0;
		INF = 0;
		REF = 0;
		margin_offset = 0;
	};
	~explore_bin0(){
		if(BIN) free(BIN);
		if(sufoffset) delete [] sufoffset;
		if(name) free(name);
	};
	int isRacine(){ return(head.flag & TYPE_BIN_RACINE);};
	void loadbin(char *fname){
		U_FILE *f;
		if(!(f = u_fopen(BINARY,fname,U_READ)))
			fopenErrMessage(fname);
		name = (char *)malloc(strlen(fname)+1);
		if (name==NULL) {
		   fatal_alloc_error("loadbin");
		}
		strcpy(name,fname);
		head.readFromFile(f);
		int i =
			head.size_bin+
			head.size_ref+
			head.size_inf*2;
		BIN = (unsigned char *)malloc(i);
		if(!BIN) fatal_error("malloc fail\n");
		if(!fread(BIN,i,1,f))
			freadError(fname);
		margin_offset = head.size_bin;
		REF = (unsigned char *)(BIN + head.size_bin);
		INF = (unichar *)(REF + head.size_ref);


		autoffset = new unsigned int[head.cnt_auto+1];
		sufoffset = new unsigned int[head.cnt_suf+1];
		AUT = new unichar *[head.cnt_auto+1];
		SUF = new unichar *[head.cnt_suf+1];

		int sz;
		unsigned char *wp;
		unichar v;
		unichar *uc;

		wp = (unsigned char *)INF;
		uc = INF;
		for ( i = 0; i < head.size_inf; i++)
		{
			v =  ((*wp++ <<  8) & 0xff00);
			v |= ( *wp++        & 0x00ff);
			uc[i] = v ;
		}

		int offsetStr = 0;
		AUT[0] = assignUstring(u_null_string);
		for( i = 1; i <= head.cnt_auto ;i++){
			AUT[i] = &INF[offsetStr];
			while(INF[offsetStr]) offsetStr++;
			offsetStr++;

		}
		//
		//
		sufoffset[0] = 0;
		SUF[0] = assignUstring(u_null_string);
		for(i = 1; i <= head.cnt_suf;i++){
			SUF[i] = &INF[offsetStr];
			while(INF[offsetStr]) offsetStr++;
			offsetStr++;
			sufoffset[i] = 0;
			u_printf("%d %S\n",i,SUF[i]);
		}

		wp = (unsigned char *)REF;
		uc = (unichar *)REF;
		sz  = *wp++ << 8;
		sz |= *wp++;

		if(sz != head.cnt_auto)
			fatal_error("illegal bin\n");
		int infIdx;
		for(i = 1; i <= sz;i++){
			wp++; // flag skip
			autoffset[i]  = *wp++ << 16;
			autoffset[i] |= *wp++ << 8;
			autoffset[i] |= *wp++;
			infIdx		  = *wp++ << 16;
			infIdx		 |= *wp++ << 8;
			infIdx		 |= *wp++;
			if(u_strcmp(&INF[infIdx],AUT[i])){
				fatal_error("illegal value in the bin\n");
			}
         u_printf("%S start offset is %d\n",AUT[i],autoffset[i]);
		}
		u_fclose(f);
	}


	void
	reloc_BIN(int pos,int decalage_bin,int decalage_ref)
	{
		int i;
	int ref;
	int n_transitions;
	do {
		n_transitions  =BIN[pos++]*256;
		n_transitions +=BIN[pos++];

			if(n_transitions & 0x8000){
				n_transitions &= 0x7fff;
			} else {
				ref = BIN[pos];
				ref = ref * 256 + BIN[pos+1];
				ref = ref * 256 + BIN[pos+2];
				ref += decalage_ref;
				BIN[pos++] =  (unsigned char) (((ref >> 16) & 0xff));
				BIN[pos++] =  (unsigned char) (((ref >> 8) & 0xff));
				BIN[pos++] =  (unsigned char) ((ref & 0xff));
			}
			for( i = 0; i <n_transitions;i++){
				pos++; pos++;	// skip character
				ref =    (BIN[pos] << 16) & 0xff0000;
				ref |= 	(BIN[pos+1] << 8)  & 0xff00;
				ref |=	BIN[pos+2] & 0xff ;
				ref += decalage_bin;
				BIN[pos++] =  (unsigned char) (((ref >> 16) & 0xff));
				BIN[pos++] =  (unsigned char) (((ref >> 8) & 0xff));
				BIN[pos++] =  (unsigned char) ((ref & 0xff));
			}
		} while( pos < head.size_bin);
	}
//
//	relocate ref zone
//	input:
//	output : size of skip offset which for inital states
//
	void
	relocate_ref(int pos,int decalage_inf)
	{
	// delete first reference
	int sz,suf,inf;
	unsigned char flag;
	int i;

	sz  = REF[pos++] << 8;
	sz |= REF[pos++];
	pos += sz*7;
	do {
		sz  = REF[pos++] << 8;
		sz |= REF[pos++];
		for( i = 0; i < sz;i++)
		{
			flag =   REF[pos++];
			suf  =  (REF[pos]   << 16) & 0xff0000;
			suf |= 	(REF[pos+1] << 8)  & 0xff00;
			suf |=	 REF[pos+2]        & 0xff ;
			suf = sufoffset[suf];
			REF[pos++] =  (unsigned char) ((suf >> 16) & 0xff);
			REF[pos++] =  (unsigned char) ((suf >>  8) & 0xff);
			REF[pos++] =  (unsigned char) ( suf        & 0xff);
			inf =    (REF[pos] << 16) & 0xff0000;
			inf |= 	(REF[pos+1] << 8)  & 0xff00;
			inf |=	REF[pos+2] & 0xff ;
			inf  += decalage_inf;
			REF[pos++] =  (unsigned char) ((inf >> 16) & 0xff);
			REF[pos++] =  (unsigned char) ((inf >>  8) & 0xff);
			REF[pos++] =  (unsigned char) ( inf        & 0xff);
		}

	} while(pos < head.size_ref);
	if(pos != head.size_ref) fatal_error("ah\n");
	}
	void setfunc(actDansBin a, actDansBin b)
	{
		actFuncForInfo = a;
		actFuncForFinal = b;
	}
	void search_mot( unichar *cc,int offset,int sidx)
	{
		explorer_bin_avec_tokens(cc,offset,0,sidx);
	}
	void get_mots(int offset,unichar *cc,int idx)
	{
		explorer_bin_sans_tokens(cc,offset,idx);
	}
	void
	explorer_bin_sans_tokens(unichar *contenu,int pos,int depth)
	{
		int n_transitions;

		if(pos >= margin_offset) fatal_error("illegal bin value\n");
		n_transitions=BIN[pos++] << 8;
		n_transitions |=BIN[pos++];
		if(n_transitions & 0x8000){
			n_transitions &= 0x7fff;
			 // if we are in a normal node, we remove the control bit to
		  // have the good number of transitions
			next_tran_sans_token(n_transitions,contenu,pos,depth);
			return;
		}
		register int index;
		int infIdx;
		unsigned char flag;
		int sufIdx;
		unichar *wp;
		unsigned char *scanRef;
		int refidx;
		int sdepth;
		int suffixe_jmp_case = 0;

		refidx  = BIN[pos++] << 16;
		refidx |= BIN[pos++] << 8;
		refidx |= BIN[pos++];

//fwprintf(d.f,L"<%s:%s:%d>\n",c,inf,depth);
		scanRef = &REF[refidx];
		int cnt = (*scanRef++ << 8);
		cnt |=  *scanRef++;
		if(!cnt) fatal_error("illegal reference value\n");
		for(index = 0 ; index < cnt;index++){
			sdepth  = depth;
			flag    = *scanRef++;
			sufIdx  = *scanRef++ << 16;
			sufIdx |= *scanRef++ << 8;
			sufIdx |= *scanRef++;
			infIdx  = *scanRef++ << 16;
			infIdx |= *scanRef++ << 8;
			infIdx |= *scanRef++;
			contenu[sdepth++] = '[';
			if(flag & (INF_IGNO_INF | INF_NAME_STR)){
			   unichar nameArr[] = {'n','a','m','e','\0'};
				wp = assignUstring(nameArr);
			} else {
				wp = assignUstring(u_null_string);
			}
			while(*wp) contenu[sdepth++] = *wp++;
			wp = &INF[infIdx];
			while(*wp) contenu[sdepth++] = *wp++;

			if(flag & INF_IGNO_SUF){
				contenu[sdepth++] = ']';
				next_tran_sans_token(
				n_transitions,contenu,pos,sdepth);
			} else {
				suffixe_jmp_case++;
				if(sufIdx){ // the condition stop
					if(flag & INF_IGNO_INF){
						contenu[sdepth++] = ']';
						explorer_bin_sans_tokens(contenu,sufIdx,sdepth);
					}else if(flag & INF_NOABS_SUF) {

						contenu[sdepth++] = ']';
						(*actFuncForInfo)(this,contenu,pos,sdepth,sufIdx);
					} else {
						contenu[sdepth++] = ']';
						explorer_bin_sans_tokens(contenu,sufIdx,sdepth);
					}
				} else {
					(*actFuncForFinal)(this,contenu,pos,sdepth,0);

				}
			}
		}
		if(suffixe_jmp_case){
			next_tran_sans_token(
			n_transitions,contenu,pos,depth);
		}
	}

	void
	next_tran_sans_token(int n_transitions,unichar *contenu,int pos,int depth)
	{
		int ref;
		unichar c;
		for (int i=0;i<n_transitions;i++) {
			c=BIN[pos++];
			c = c*256+BIN[pos++];
			ref = BIN[pos++];
			ref = ref * 256 + BIN[pos++];
			ref = ref * 256 + BIN[pos++];
			contenu[depth] = c;
			explorer_bin_sans_tokens(contenu,ref,depth+1);
		}
	}

	void
	explorer_bin_avec_tokens(unichar *word,int pos,int depth,int saveIdx)
	{
		int n_transitions;
		if(BIN[pos] & 0x80){
			n_transitions  =(BIN[pos++] & 0x7f)<< 8;
			n_transitions = n_transitions + BIN[pos++];
		  // if we are in a normal node, we remove the control bit to
		  // have the good number of transitions
			n_transitions=n_transitions & 0x7fff;
			next_tran_avec_token(n_transitions,word,pos,depth,saveIdx);
			return;
		}
		// if we are in a control node, we must jump after the reference to the INF line number
		(*actFuncForInfo)(this,word,depth,pos,saveIdx);
	}

	void
	next_tran_avec_token(int n_transitions,unichar *word,int pos,int depth,int saveIdx)
	{
		int ref;
		unichar c,testc;
		testc = word[depth];
		if(testc == '\0'){
			if(actFuncForFinal)	(*actFuncForFinal)(this,0,depth,0,saveIdx);
			return;
		}
// fwprintf(d.f,L"[%c:",testc);
		for (int i=0;i<n_transitions;i++) {
			c=BIN[pos++];
			c = c*256+BIN[pos++];
// fwprintf(d.f,L"%c",c);
			if(c == testc){
				ref = BIN[pos++];
				ref = ref * 256 + BIN[pos++];
				ref = ref * 256 + BIN[pos++];
//fwprintf(d.f,L"%06x]\n",ref);
				explorer_bin_avec_tokens(word,ref,depth+1,saveIdx);
				return;
			} else {
//fwprintf(d.f,L"=>");
				pos+=3;
			}
		}
//fwprintf(d.f,L"xxx\n");
	}

	int
	explorer_bin_pour_obtenir_value(unichar *contenu,int pos,int depth,int saveIdx)
	{
		int n_transitions;
		int ref;
		unichar c,testc;
		testc = contenu[depth];
		n_transitions  =BIN[pos++]*256;
		n_transitions +=BIN[pos++];

		if(testc == '\0'){
			if(n_transitions & 32768){
				return(-1);
			}
		  // if we are in a final node, we must jump after the reference
		  // to the INF line number
			ref = BIN[pos++];
			ref = ref * 256 + BIN[pos++];
			ref = ref * 256 + BIN[pos++];
		}
		n_transitions=n_transitions & 0x7fff;
		for (int i=0;i<n_transitions;i++) {
			c=BIN[pos++];
			c = c*256+BIN[pos++];
			if(c == testc){
				ref = BIN[pos++];
				ref = ref * 256 + BIN[pos++];
				ref = ref * 256 + BIN[pos++];
				return(
				explorer_bin_pour_obtenir_value(contenu,ref,depth+1,saveIdx));
			}
			pos+=3;
		}
		return(-1);
	}


};
//
//
//
class load_consultation_dics
{
	class explore_bin0 **loaded_map;
	arbre_string00 SUF;
	unichar **SUF_tmp_unichar;
public:
	struct sufptr **offsetCommon;

	int offsetCommonCnt;
	class explore_bin0 **AUT;
	int racineCnt;
	int loadMapCnt;
	load_consultation_dics()
	{
		racineCnt = 0;
		offsetCommonCnt = 0;
		loadMapCnt = 0;
		AUT = 0;
		loaded_map = 0;
		offsetCommon = 0;
		SUF_tmp_unichar = 0;
	};
	~load_consultation_dics()
	{
		int i;
		if(loaded_map){
			for( i = 0; i < loadMapCnt;i++)
				if(loaded_map[i]) delete loaded_map[i];
			delete [] loaded_map;
		}
		if(AUT) delete [] AUT;
		if(offsetCommon){
			for( i = 0; i < offsetCommonCnt;i++)
				if(offsetCommon[i]) delete offsetCommon[i];
			free(offsetCommon);
		}
		if(SUF_tmp_unichar){
			for( i = 0; i < offsetCommonCnt;i++)
				if(SUF_tmp_unichar[i]) free(SUF_tmp_unichar[i]);
			free(SUF_tmp_unichar);
		}
	};

	struct sufptr **load_bins(char *listFileName)
	{
		U_FILE *lstF;
		if(!(lstF = u_fopen(BINARY,listFileName,U_READ)))
			fopenErrMessage(listFileName);
		char path_of_locate[1024];
		char buff[1024];
		char filename[1024];
		path_of_locate[0] = 0;
		register char *wp;
		simpleL<char *> fileLists;
		char *tmp;


		while(af_fgets(buff,1024,lstF->f)){
			wp = buff;
			while(*wp){
				if(*wp == 0x0d){ *wp = 0; break;}
				if(*wp == 0x0a){ *wp = 0; break;}
				wp++;
			}
			switch(buff[0]){
			case ':':
				strcpy(path_of_locate,&buff[1]);
			case ' ':	// comment line
			case 0:
				continue;
			}
			if(path_of_locate[0]){
				strcpy(filename,path_of_locate);
				strcat(filename,buff);
			} else {
				strcpy(filename,buff);
			}
			tmp = new char[strlen(filename)+1];
			strcpy(tmp,filename);
			fileLists.put(tmp);
		}
		u_fclose(lstF);
		//
		//	construction aut<->image map
		//
		loadMapCnt = fileLists.size();
		loaded_map = new class explore_bin0 *[ loadMapCnt];
		int idx = 0;
		int i;
		struct sufptr *tp;
		char *fnPtr;
		int nidx;

		fileLists.reset();
		racineCnt = 0;
		SUF.put(assignUstring(u_epsilon_string),0);

		while((fnPtr = fileLists.getNext())){
			loaded_map[idx] = new class explore_bin0;
			loaded_map[idx]->loadbin(fnPtr);

			if(loaded_map[idx]->isRacine()){
				racineCnt++;
			} else {
				for( i = 1; i <= loaded_map[idx]->head.cnt_auto;i++){
					tp = new struct sufptr;
					tp->bin = loaded_map[idx];
					tp->offset =loaded_map[idx]->autoffset[i];
					tp->w = loaded_map[idx]->AUT[i];
					nidx = SUF.put(loaded_map[idx]->AUT[i],tp);
               u_printf("suffixe %S %dth image %d  array %d offset\n",
						tp->w,idx,nidx, tp->offset);
				}
			}
			idx++;
		}

		//
		//	linking map point
		//
		AUT = new class explore_bin0 *[racineCnt];
		offsetCommonCnt = SUF.size();

		SUF_tmp_unichar = SUF.make_strPtr_table((int **)&offsetCommon);
      if(!offsetCommon) {
         fatal_error("mem alloc fail for offset array\n");
      }
		for( i = 1; i < offsetCommonCnt;i++){
			error("%d %S %d",
				i,&SUF_tmp_unichar[i][1],offsetCommon[i]->offset);
			error("%s\n",offsetCommon[i]->bin->name);

		}
		racineCnt = 0;

		for( idx = 0; idx < loadMapCnt;idx++){
#ifdef DDDDDDD
			for( i = 1;i <= loaded_map[idx]->head.cnt_suf;i++){
				if(!u_strcmp(loaded_map[idx]->SUF[i],"null"))
					fatal_error("nom de suffixe violation, do not use 'null'\n");
				nidx =SUF.check(loaded_map[idx]->SUF[i]);
				if( nidx == -1){
					error("%s: ",getUtoChar(loaded_map[idx]->SUF[i]));
					fatal_error("not solved suffixe link existe");
				}
				if(nidx == 0) fatal_error("kki ak\n");

				loaded_map[idx]->sufoffset[i] = nidx;

				offsetCommon[nidx] = new struct sufptr;
				tp= (struct sufptr *)SUF.getCheckValue();
				if(!tp) fatal_error("illegal value of the suffix pointer\n");
				offsetCommon[nidx]->bin = tp->bin;
				offsetCommon[nidx]->offset = tp->offset;
				offsetCommon[nidx]->w = tp->w;

			}
#endif
			if(loaded_map[idx]->isRacine())
				AUT[racineCnt++] = loaded_map[idx];
		}
		//
		//
		//
		for( idx = 0 ; idx < loadMapCnt;idx++){
			relocalize_suffixe_offset(loaded_map[idx]);
		}
		return(offsetCommon);
	}
	void relocalize_suffixe_offset(class explore_bin0 *bp)
	{
		unsigned char *wp;
		int i;
		int sz;
		int sufIdx;
		unsigned char flag;

		wp = (unsigned char *)bp->REF;
		while(wp < (unsigned char *)bp->INF){
			sz  = *wp++ << 8;
			sz |= *wp++;
			if(!sz) fatal_error("illegal reference value\n");
			for( i = 0; i < sz; i++){
				flag = *wp;
				if(flag & INF_NOABS_SUF){
					*wp++;
					sufIdx  = (wp[0] << 16);
					sufIdx |= (wp[1] << 8);
					sufIdx |= wp[2];
               error("%s",getUtoChar((unichar *)&bp->INF[sufIdx]));
					sufIdx = SUF.check(&bp->INF[sufIdx]);
					if(!sufIdx ||(sufIdx == -1)){
						fatal_error("%s\n",getUtoChar((unichar *)&bp->INF[sufIdx]));
					}
               error("%d\n",sufIdx);
					*wp++ = (sufIdx >> 16 ) & 0xff;
					*wp++ = (sufIdx >>  8 ) & 0xff;
					*wp++ =  sufIdx & 0xff;
					wp += 3;
				} else
					wp += 7;
			}
		}
	}
	void set_act_func(actDansBin a, actDansBin b)
	{
		for( int i = 0; i < loadMapCnt;i++)
			loaded_map[i]->setfunc(a,b);
	}
};
//
//
//
class unification_des_bins
{
	class binHead0	head;
	class explore_bin0 **loaded_map;
	arbre_string00 SUF;
	unichar **SUF_tmp_uni_char;
public:
	struct sufptr **offsetCommon;
	int offsetCommonCnt;
	class explore_bin0 **AUT;
	int racineCnt;
	int loadMapCnt;
	unification_des_bins()
	{
		racineCnt = 0;
		offsetCommonCnt = 0;
		loadMapCnt = 0;
		AUT = 0;
		loaded_map = 0;
		offsetCommon = 0;
		SUF_tmp_uni_char = 0;

	};
	~unification_des_bins()
	{
		int i;
		if(loaded_map){
			for( i = 0; i < loadMapCnt;i++)
				if(loaded_map[i]) delete loaded_map[i];
			delete loaded_map;
		}
		if(AUT) delete [] AUT;
		if(offsetCommon){
			for( i = 0; i < offsetCommonCnt;i++)
				if(offsetCommon[i]) delete offsetCommon[i];
			free(offsetCommon);
		}
		if(SUF_tmp_uni_char){
			for( i = 0; i < offsetCommonCnt;i++)
				if(SUF_tmp_uni_char[i]) free(SUF_tmp_uni_char[i]);
			free(SUF_tmp_uni_char);
		}
	};

	void load_bins(char *listFileName)
	{
		U_FILE *lstF;
		if(!(lstF = u_fopen(BINARY,listFileName,U_READ)))
			fopenErrMessage(listFileName);
		char path_of_locate[1024];
		char buff[1024];
		char filename[1024];
		path_of_locate[0] = 0;
		register char *wp;
		simpleL<char *> fileLists;
		char *tmp;

		while(af_fgets(buff,1024,lstF->f)){
			wp = buff;
			while(*wp){
				if(*wp == 0x0d){ *wp = 0; break;}
				if(*wp == 0x0a){ *wp = 0; break;}
				wp++;
			}
			switch(buff[0]){
			case ':':
				strcpy(path_of_locate,&buff[1]);
			case ' ':	// comment line
			case 0:
				continue;
			}
			if(path_of_locate[0]){
				strcpy(filename,path_of_locate);
				strcat(filename,buff);
			} else {
				strcpy(filename,buff);
			}
			tmp = new char[strlen(filename)+1];
			strcpy(tmp,filename);
			fileLists.put(tmp);
		}
		u_fclose(lstF);
		//
		//	construction aut<->image map
		//
		loadMapCnt = fileLists.size();
		loaded_map = new class explore_bin0 *[ loadMapCnt];
		int idx = 0;
		int i;
		struct sufptr *tp;
		char *fnPtr;
		fileLists.reset();
		racineCnt = 0;

        SUF.put(assignUstring(u_epsilon_string),0);
		while((fnPtr = fileLists.getNext())){
			loaded_map[idx] = new class explore_bin0;
			loaded_map[idx]->loadbin(fnPtr);

			if(loaded_map[idx]->isRacine()){
				racineCnt++;
			} else {
				for( i = 1; i <= loaded_map[idx]->head.cnt_auto;i++){
					tp = new struct sufptr;
					tp->bin = loaded_map[idx];
					tp->offset =loaded_map[idx]->autoffset[i];
					tp->w = loaded_map[idx]->AUT[i];
					SUF.put(loaded_map[idx]->AUT[i],tp);
				}
			}
			idx++;
		}

		//
		//	linking map point
		//
		AUT = new class explore_bin0 *[racineCnt];
		offsetCommonCnt = SUF.size();

		SUF_tmp_uni_char = SUF.make_strPtr_table((int **)&offsetCommon);
		for( i = 1; i < offsetCommonCnt;i++){
			error("%d %s %d %d\n",i
                     ,getUtoChar(&SUF_tmp_uni_char[i][1])
            , (uintptr_t)offsetCommon[i]->bin,offsetCommon[i]->offset);
		}
		if(!offsetCommon)
			fatal_error("mem alloc fail for offset array\n");
		racineCnt = 0;

		int nidx;
		for( idx = 0; idx < loadMapCnt;idx++){
			for( i = 1;i <= loaded_map[idx]->head.cnt_suf;i++){
				nidx =SUF.check(loaded_map[idx]->SUF[i]);
				if( nidx == -1){
					fatal_error("%s: not solved suffixe link existe\n",getUtoChar(loaded_map[idx]->SUF[i]));
				}
				if(nidx == 0) fatal_error("kki ak\n");
				loaded_map[idx]->sufoffset[i] = nidx;
				offsetCommon[nidx] = new struct sufptr;
				tp= (struct sufptr *)SUF.getCheckValue();
				if(!tp) fatal_error("illegal value of the suffix pointer\n");
				offsetCommon[nidx]->bin		= tp->bin;
				offsetCommon[nidx]->offset  = tp->offset;
				offsetCommon[nidx]->w		= tp->w;
			}
			if(loaded_map[idx]->isRacine())
				AUT[racineCnt++] = loaded_map[idx];
		}
		if(offsetCommon[0])	// null value
			fatal_error("illegal value\n");
		int unsolved_suffixe = 0;
		for( i = 1; i < offsetCommonCnt;i++){
			if(!offsetCommon[i]) unsolved_suffixe++;
		}
		//
		//	calcul head
		//
/*
		for( idx = 0; idx < loadMapCnt;idx++){
			head.size_bin += loaded_map[idx]->head.size_bin - 5;
			head.size_ref += loaded_map[idx]->head.size_ref
				- loaded_map[idx].offset_org_REF;
			head.size_inf += loaded_map[idx]->head.size_int
				- loaded_map[idx].offset_org_INF;
		}
		*/
		//
		//
		//



	}
	void set_act_func(actDansBin a, actDansBin b)
	{
		for( int i = 0; i < loadMapCnt;i++)
			loaded_map[i]->setfunc(a,b);
	}
};

//
//	load relocatabl bins
//

class union_bin_file {
	U_FILE *inf;
	int *newInfTable;
	char sansExtension[1024];
	class binHead0 imageHead;
	struct simpleTmp {
		unichar *name;
		int szStr;
		int offset;
	};
	class arbre_string0 AUT;
	struct simpleTmp *AUT_tmp;
	struct simpleTmp *SUF_tmp_simpleTmp;
	class arbre_string00	 NINF;
	class arbre_string00 SUF;
	int save_inf_offset;
	int save_ref_offset;
	int save_str_offset;

	unichar tmpBuff[4096];
	unichar svBuff[4096];

    Encoding encoding_output ;
    int bom_output ;
    int mask_encoding_compatibility_input ;

public:
	union_bin_file()
	{
		newInfTable = 0;
		save_ref_offset = 0;
		AUT_tmp = 0;
		SUF_tmp_simpleTmp = 0;

        encoding_output = DEFAULT_ENCODING_OUTPUT;
        bom_output = DEFAULT_BOM_OUTPUT;
        mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
	};
	~union_bin_file()
	{
		int i;
		if(newInfTable) delete [] newInfTable;

		if(AUT_tmp){
			for( i = 0; i < imageHead.cnt_auto;i++)
				if(AUT_tmp[i].name) delete [] AUT_tmp[i].name;
		}
		if(SUF_tmp_simpleTmp){
			for( i = 0; i < imageHead.cnt_suf;i++)
				if(SUF_tmp_simpleTmp[i].name) delete [] SUF_tmp_simpleTmp[i].name;
		}
	};

    void setEncoding(Encoding encoding_outputSet,int bom_outputSet,int mask_encoding_compatibility_inputSet){
        encoding_output = encoding_outputSet;
        bom_output = bom_outputSet;
        mask_encoding_compatibility_input = mask_encoding_compatibility_inputSet;
    }

	//
	//	reconstruction of the bin file
	//
	void ajouteRef(char *binfilename)
	{
		if(!(inf = u_fopen(BINARY,binfilename,U_MODIFY)))
			fopenErrMessage(binfilename);

		imageHead.readFromFile(inf);
		remove_extension(binfilename,sansExtension);
		getAutoArray();
		load_ref();
		load_bin_for_change();
		save_inf_image();

		fseek(inf,0,SEEK_SET);
		imageHead.flag |= TYPE_BIN_LINKED;
		imageHead.writeAtFile(inf);
		u_fclose(inf);
	}
	void getAutoArray()
	{
		U_FILE *lf;
		char openfilename[1024];
		unichar UtempBuff[4096];
		int i;

		strcpy(openfilename,sansExtension);
		strcat(openfilename,".aut");
		if(!(lf = u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,openfilename,U_READ)))
			fopenErrMessage(openfilename);
		u_fgets(UtempBuff,4096,lf);
		imageHead.cnt_auto = utoi(UtempBuff);
		AUT_tmp = new struct
			union_bin_file::simpleTmp[imageHead.cnt_auto +1];
		for( i = 0; i <= imageHead.cnt_auto;i++)
		{
			AUT_tmp[i].name = 0;
			AUT_tmp[i].szStr = 0;
			AUT_tmp[i].offset = 0;
		}
		//	first value 0
		AUT.put(assignUstring(u_epsilon_string));
		int lidx;
		unichar *num;
		lidx = 1;
		while(EOF!=u_fgets_limit2(UtempBuff,4096,lf)){
			num = UtempBuff;
			while(*num != ' '){
				if(*num == 0) fatal_error("Illegal autolist file\n");
				num++;
			}
			*num++ = 0;
			AUT_tmp[lidx].szStr = u_strlen(UtempBuff) +1;
			AUT_tmp[lidx].name = new unichar[AUT_tmp[lidx].szStr+1];
			u_strcpy(AUT_tmp[lidx].name,UtempBuff);
			AUT_tmp[lidx].offset = utoi(num);
			AUT.put(UtempBuff);
			lidx++;
		}
		if((lidx-1) != imageHead.cnt_auto)
			fatal_error("suffix count is mismatch\n");

		u_fclose(lf);
		strcpy(openfilename,sansExtension);
		strcat(openfilename,".suf");
		if(!(lf = u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,openfilename,U_READ)))
			fopenErrMessage(openfilename);
		u_fgets_limit2(UtempBuff,4096,lf);
		imageHead.cnt_suf = utoi(UtempBuff);
		SUF_tmp_simpleTmp =
			new struct union_bin_file::simpleTmp[imageHead.cnt_suf+1];
		for( i = 0; i <= imageHead.cnt_suf;i++)
		{
			SUF_tmp_simpleTmp[i].name = 0;
			SUF_tmp_simpleTmp[i].szStr = 0;
			SUF_tmp_simpleTmp[i].offset = 0;
		}
		SUF.put(assignUstring(u_epsilon_string),0);
		lidx = 1;
		while(EOF!=u_fgets_limit2(UtempBuff,4096,lf)){
			SUF_tmp_simpleTmp[lidx].szStr = u_strlen(UtempBuff) +1;
			SUF_tmp_simpleTmp[lidx].name = new unichar[SUF_tmp_simpleTmp[lidx].szStr+1];
			u_strcpy(SUF_tmp_simpleTmp[lidx].name,UtempBuff);
			SUF_tmp_simpleTmp[lidx].offset = 0;
//			SUF.put(UtempBuff,0);
			lidx++;
		}
		if((lidx - 1 ) != imageHead.cnt_suf)
			fatal_error("suffix count is mismatch\n");
		u_fclose(lf);
	}
#define DECALAGE_NEW_BIN	5
	void load_ref()
	{
		int i;
		U_FILE *lf;
		fseek(inf,0,SEEK_END);
		// get space 5 for initial state
		outbytes2(0,inf);
		outbytes3(0,inf);
		// make inf 0 for automate
		save_ref_offset = 0;
		save_inf_offset = 0;
		outbytes2((unichar)imageHead.cnt_auto,inf);
		save_ref_offset +=2;
		int infIdx;
		int sufIdx;
		unsigned char flag;
		for( i = 1; i <= imageHead.cnt_auto;i++){
			flag  = INF_IGNO_INF;
			sufIdx = AUT_tmp[i].offset + DECALAGE_NEW_BIN;
			if(sufIdx > 0x1000000)
				fatal_error("too big suffixe reference\n");
			infIdx = save_inf_offset;
			fwrite(&flag,1,1,inf);
			outbytes3(sufIdx,inf);
			save_ref_offset +=4;
			outbytes3(infIdx,inf);
			save_ref_offset +=3;
			save_inf_offset += AUT_tmp[i].szStr;
         u_printf("%s %d\n",getUtoChar(AUT_tmp[i].name),save_inf_offset);
		}
		for( i = 1; i <= imageHead.cnt_suf;i++){
			SUF.put(SUF_tmp_simpleTmp[i].name,(void *)save_inf_offset);
			save_inf_offset += SUF_tmp_simpleTmp[i].szStr;
         u_printf("%s %d\n",getUtoChar(SUF_tmp_simpleTmp[i].name),save_inf_offset);
		}

		char fname_sans_extension[1024];
		int rsFlag = ((imageHead.flag & TYPE_BIN_RACINE) == TYPE_BIN_RACINE) ;

		strcpy(fname_sans_extension,sansExtension);
		strcat(fname_sans_extension,".inf");
		if(!(lf = u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,fname_sans_extension,U_READ)))
			fopenErrMessage(fname_sans_extension);
		int rdInfCnt;// =u_read_int(lf);
      u_fscanf(lf,"%d\n",&rdInfCnt);
		newInfTable = new int[rdInfCnt];
		if(!newInfTable) fatal_error("mem alloc fail for inf\n");
		unichar UtempBuff[4096];

		NINF.put(assignUstring(u_epsilon_string),(void *)save_inf_offset);
		save_inf_offset += 4;	// length of epsilon
		int cidx = 0;
		while(EOF!=u_fgets_limit2(UtempBuff,4096,lf)){
			newInfTable[cidx] = makeNewINF(UtempBuff,rsFlag);
			if(!newInfTable[cidx]) fatal_error("illegal reference value\n");
			cidx++;
		}
		u_fclose(lf);
		if(cidx != rdInfCnt) fatal_error("not match read count\n");

		imageHead.size_ref = save_ref_offset;
		imageHead.size_inf = save_inf_offset;
	}
	int
	makeNewINF(unichar *in,int racine_suffixe)
	{
		DISCARD_UNUSED_PARAMETER(racine_suffixe)

		simpleL<struct INF_raw *> links;
		int tidx;
		struct INF_raw *p;
		int index;
		int saveT;
//		int sindex;
//		int segcnt = 0;
		int openF = 0;
//		int sz;
		unichar info[4096];
		unichar *sPtr;
		unichar *iPtr;

//		unichar *lsegs[3];
//		lsegs[0] = lsegs[1] = lsegs[2] = 0;
//		sindex = 0;
//u_fprintf(d.f,"<%S:%S:%d>\n",c,inf,depth);
//		lsegs[segcnt++] = tmpBuff;
//		lsegs[segcnt] = tmpBuff;
//		tmpBuff[sindex] = 0;
		sPtr = info;
		saveT = 1;
		index = 0;
      u_printf("%s=>", getUtoChar(in));
		while(saveT){
			switch(info[index] = in[index]){
			case '<': openF = 1;
//				tmpBuff[sindex++]=info[index];
				break;
			case '>':
//				openF = 0;tmpBuff[sindex++]=info[index];
				break;
			case ',':
				if(openF) {
//					tmpBuff[sindex++] = '/';
					break;
				}
			case 0:
//				tmpBuff[sindex] = 0;
//				if(segcnt !=3 ) fatal_error("illegal INF format\n");
				if(!info[index]) saveT = 0;
				else info[index] = 0;
				iPtr = sPtr;
				while(*iPtr != '.'){
					if(!*iPtr){
						fatal_error(":%s: illegal INF format\n",getUtoChar(in));
					}
					iPtr++;
				}
				*iPtr++ = 0;
				p = new struct INF_raw;
				p->flag = 0;
				p->sufIdx = 0;



//				if(!wcscmp(lsegs[0],epsilon)) lsegs[0][0] = 0;
//				if(lsegs[0][0] != 0){
//					if( (lsegs[0][0] >= '0') &&
//						(lsegs[0][0] >= '9')){
//						int sc = 0;
//						while(lsegs[0][sc]){
//							p->sufIdx = p->sufIdx*10
//								+ lsegs[0][sc] - '0';
//							sc++;
//						}
//					} else {
//						p->flag |= INF_NOABS_SUF;
//						tidx = SUF.check(lsegs[0]);
//						if(tidx == -1) fatal_error("illegal suffixe value\n");
//						p->sufIdx = (int)SUF.getCheckValue();
//					}
//				}
//				if(!wcscmp(lsegs[1],epsilon)) lsegs[1][0] = 0;
//				if(!wcscmp(lsegs[2],epsilon)) lsegs[2][0] = 0;

//				u_strcpy(svBuff,lsegs[1]);
//				u_strcat(svBuff,L".");
//				u_strcat(svBuff,lsegs[2]);

				if(*sPtr != 0){
					if( (*sPtr >= '0') &&
						(*sPtr <= '9')){	// check internal jmp
						int sc = 0;
						while(sPtr[sc]){
							p->sufIdx = p->sufIdx*10
								+ sPtr[sc] - '0';
							sc++;
						}
					} else if (!u_strcmp(sPtr,"null")){
						p->flag |= INF_IGNO_SUF;

					} else {
						p->flag |= INF_NOABS_SUF;
						tidx = SUF.check(sPtr);
						if(tidx == -1){
							fatal_error(":%s :%s illegal suffixe value\n",getUtoChar(in),getUtoChar(sPtr));
						}
						p->sufIdx = (uintptr_t)SUF.getCheckValue();
					}
				}

				if(NINF.check(iPtr) == -1){
					p->infIdx = save_inf_offset;
					NINF.put(iPtr,(void *)save_inf_offset);
					save_inf_offset+= u_strlen((unichar*)iPtr)+1;
				} else {
					p->infIdx =(uintptr_t)NINF.getCheckValue();
				}
				links.put(p);
            u_printf("<%s",getUtoChar(sPtr));
            u_printf("|%s:%d,%d,%d>",getUtoChar(iPtr),p->flag,p->sufIdx,p->infIdx);
				if(saveT){
					sPtr = &info[index+1];
				}

//				segcnt = 0;
//				sindex = 0;
//				tmpBuff[segcnt] = 0;
//				lsegs[segcnt++] = tmpBuff;
//				lsegs[2] = 0;
				break;
//			case '.':	// separateur
//				if(!openF) {
//					tmpBuff[sindex++] = 0;
//					lsegs[segcnt++] = &tmpBuff[sindex];
//					if(segcnt >= 4) fatal_error("too many inf\n");
//					break;
//				}
//			default:
//				tmpBuff[sindex++] = info[index];
			}
			index++;
		};
      u_printf("\n");
		int retval;
		int retPos = save_ref_offset;
		retval = retPos;
		tidx = links.size();
		outbytes2((unichar)tidx,inf);
		retPos+=2;
		links.reset();
		while((p = links.getNext())){
			fwrite(&p->flag,1,1,inf);
			outbytes3(p->sufIdx,inf);
			if(p->infIdx > 0x1000000) fatal_error("too big inf index\n");
			outbytes3(p->infIdx,inf);
			delete p;
			retPos+=7;
		}
		save_ref_offset = retPos;
		return(retval);
	}
	void load_bin_for_change()
	{

		int pos = 0;
		int ref;
		int n_transitions;
		int i;

		unsigned char *BIN = new unsigned char[imageHead.size_bin];
		fseek(inf,IMAGE_HEAD_SZ,SEEK_SET);
		fread(BIN,imageHead.size_bin,1,inf);
		do {
			n_transitions  =BIN[pos++]*256;
			n_transitions +=BIN[pos++];

			if(n_transitions & 0x8000){
				n_transitions &= 0x7fff;
			} else {
				ref = BIN[pos];
				ref = ref * 256 + BIN[pos+1];
				ref = ref * 256 + BIN[pos+2];
				ref = newInfTable[ref];
				BIN[pos++] =  (unsigned char) (((ref >> 16) & 0xff));
				BIN[pos++] =  (unsigned char) (((ref >> 8) & 0xff));
				BIN[pos++] =  (unsigned char) ((ref & 0xff));
			}
			for( i = 0; i <n_transitions;i++){
				pos++; pos++;	// skip character
				ref =    (BIN[pos] << 16) & 0xff0000;
				ref |= 	(BIN[pos+1] << 8)  & 0xff00;
				ref |=	BIN[pos+2] & 0xff ;
				ref = ref + DECALAGE_NEW_BIN;
				BIN[pos++] =  (unsigned char) (((ref >> 16) & 0xff));
				BIN[pos++] =  (unsigned char) (((ref >> 8) & 0xff));
				BIN[pos++] =  (unsigned char) ((ref & 0xff));
			}
		} while( pos < imageHead.size_bin);

		fseek(inf,IMAGE_HEAD_SZ,SEEK_SET);
		outbytes2(0,inf);
		outbytes3(0,inf);
		fwrite(BIN,imageHead.size_bin,1,inf);
		imageHead.size_bin += DECALAGE_NEW_BIN;
		delete [] BIN;
		delete [] newInfTable;
		newInfTable = 0;
	}

	void save_inf_image()
	{
		int i,j;
		int sz = 0;
		int inf_offset = 0;
		fseek(inf,0,SEEK_END);
		for( i = 1; i <= imageHead.cnt_auto;i++){
			for(j = 0; AUT_tmp[i].name[j];j++)
				outbytes2(AUT_tmp[i].name[j],inf);
			outbytes2(0,inf);
			inf_offset += AUT_tmp[i].szStr;
		}
		for( i = 1; i <= imageHead.cnt_suf;i++){
			for(j = 0; SUF_tmp_simpleTmp[i].name[j];j++)
				outbytes2(SUF_tmp_simpleTmp[i].name[j],inf);
			outbytes2(0,inf);
			inf_offset += SUF_tmp_simpleTmp[i].szStr;
		}
		sz  = NINF.size();
		int *s;
		unichar **strTable = NINF.make_strPtr_table(&s);
		for(i = 0; i < sz;i++)
		{
			if(s[i] != inf_offset)
				fatal_error("illegal offset value\n");
			for(j = 1; strTable[i][j];j++)
				outbytes2(strTable[i][j],inf);
			outbytes2(0,inf);
			inf_offset += strTable[i][0];
		}
		NINF.release_value();
		if(	imageHead.size_inf != inf_offset)
			fatal_error("the size of saved information is mismatch\n");
	}
};

//
//	consultation of bins files
//
class tableForMap {
	unsigned char *BIN;
	unsigned char *ref;
	arbre_string00 NINF;
public:
	tableForMap(){};
	~tableForMap(){};

};

#endif
