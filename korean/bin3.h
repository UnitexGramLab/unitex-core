 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
//	STRUCTURE  for dictionnaire de coree by transducteur
//
#include "Alphabet.h"
#ifndef BIN3_DEF
#include "formation_dic_line.h"
#define BIN3_DEF
struct arbre_string3_node {
	int tcnt;
	int offset;
	struct avec_sorti_tran *trans;
};
#define SIZE_ONE_TRANSITION_BIN	7
#define FLAG_ABSL_PATH	0x8000
struct avec_sorti_tran {
	unichar c;
	unichar info;
	union {
		int suf;
		struct arbre_string3_node *node;
	} a;
	int depth;
	struct avec_sorti_tran *suivant;
};

//
//	class for save strucutre of dictionnainy under the compression
//
class arbre_string3
{
	class pageHandle nodes;
	class pageHandle avecTrans;
	
public:
	struct arbre_string3_node *racine[2048];
	int maxDepth;
	int init_node_cnt;
	int arbreCnt;

	class arbre_string0	 racName;
	class arbre_string0	 sufName;
	class arbre_string0	 baseInf;
	
	int checkDepth;

	arbre_string3(){
//		racName.put(u_epsilon_string);
		sufName.put(assignUstring(u_epsilon_string));		
		baseInf.put(assignUstring(u_epsilon_string));
		maxDepth = 0;
		arbreCnt = 0;
		nodes.setSz(sizeof(struct arbre_string3_node)*1024,
			sizeof(struct arbre_string3_node));
		avecTrans.setSz(
			sizeof(struct avec_sorti_tran)*1024
			,sizeof(struct avec_sorti_tran));

	};
	~arbre_string3()
	{
	};
		unsigned int getRacine(int idx){
		if(idx < arbreCnt){
			return((unsigned int)racine[idx]);
		}
		return(0);
	}
	int getArbreCnt(){return(arbreCnt);};
	int new_arbre(unichar *name){
		racName.put(name);
		racine[arbreCnt] = getNewNode();
		return(arbreCnt++);
	}

	struct arbre_string3_node *
	getNewNode()
	{
		struct arbre_string3_node* a;
		a=(struct arbre_string3_node *)nodes.get();
		a->tcnt = 0;
		a->trans = 0;
		a->offset = -2;
		return a;	
	}
	struct arbre_string3_node *put(struct arbre_string3_node *node,
		unichar c, unichar info,int suffixe)
	{

		struct avec_sorti_tran **pT;
		
		pT = &node->trans;
		if(c){
			if(info & 0x8000){ // terminal ou jmp
				while(*pT){
					if((info & 0x8000 ) == 0) break;
					if((*pT)->c){
						if( ((*pT)->c == c ) && 
							((*pT)->info == info) &&
							((*pT)->a.suf == suffixe))
							return(node);
						if( ( ((*pT)->c == c)  && 
							( (*pT)->info < info) )  || 
							( ((*pT)->c  < c) ) )
						break;
					}
					pT = &(*pT)->suivant;
				}
			} else { // normal node
				while(*pT){
					if((*pT)->c && ((info & 0x8000) == 0)){
						if(((*pT)->c == c) && 
							((*pT)->info == info))
								return((*pT)->a.node);
						if((*pT)->info < info) 	break;
					}
					pT = &(*pT)->suivant;
				}
			}
		} else {
			if(info & 0x8000){	// terminal
				while(*pT){
					if((*pT)->c) break;
					if(!((*pT)->info & 0x8000)) break;
					if( ((*pT)->a.suf == suffixe ) && 
						((*pT)->info == info))
						return(node);
					if( (*pT)->info < info) break;
					pT = &(*pT)->suivant;
				}
			} else {	// by pass
				while(*pT){
					if((*pT)->c) break; 
					if(((*pT)->info & 0x8000) == 0){
						if((*pT)->info == info)
							return((*pT)->a.node);
						if( (*pT)->info < info) break;
					}
					pT = &(*pT)->suivant;
				}
			}
		}
		node->tcnt++;
		struct avec_sorti_tran *t =
			(struct avec_sorti_tran *)avecTrans.get();
		t->c = c;
		t->info = info;
		t->suivant = (*pT);
		if(info & 0x8000){
			t->a.suf = suffixe;
		} else {
			t->a.node = getNewNode();
		}
		(*pT) = t;

		return(t->a.node );
	}
	void insert(int no_arbre,class dicLines *head)
	{
		if(no_arbre >= arbreCnt) fatal_error("Invalid No of the tree");

	    struct arbre_string3_node *scanNode = racine[no_arbre];
//		struct avec_sorti_tran *scanTran;
		
		unichar *wp;
		class dicLines *tdl;
		unichar infIdx;
		int sufIdx;

//printf("============\n");

		while(head){
			tdl = head;
			head = head->next;
			if(!scanNode) fatal_error("illegal setting ");
			wp = tdl->EC_flechi;
			unichar c;

			c = *wp;
			while(*wp){
			  c = *wp++;
			  if(!*wp) break;
#ifdef __DEBUG
printf("\nid (%x) =",scanNode);
#endif
			scanNode  = put(scanNode,c,0,0);
#ifdef __DEBUG
printf("\n=== %x scanNode ====\n",scanNode);
	  			prTreeId(no_arbre);
#endif 
			}
			
			//
			//	find
			//
			infIdx = 0;
			if(!tdl->EC_code_gramm)
				fatal_error("illegal null value;information");
			if(tdl->EC_code_gramm && (*(tdl->EC_code_gramm) != 0))
				infIdx = baseInf.put(tdl->EC_code_gramm);
			if(infIdx > 0x8000) fatal_error("too many information");
			sufIdx = 0;
			if(!head){
				infIdx |= 0x8000;
				if(tdl->EC_suffixe && (*(tdl->EC_suffixe) != 0))
					sufIdx = sufName.put(tdl->EC_suffixe);
			}
//printf("\nId(%x) =",scanNode);
			scanNode = put(scanNode,c,infIdx,sufIdx);
//printf("\nscanNode ====0x%x\n",scanNode);
//			prTreeId(no_arbre);
		}

	}

	void explore_for_access_check(
		struct arbre_string3_node * noeud)
	{
		noeud->offset = -1;
		struct avec_sorti_tran  *tt= noeud->trans;
		while(tt){
			tt->depth = 0;
			if(!(tt->info & 0x8000) )
				explore_for_access_check(tt->a.node);
			tt = tt->suivant;
		}
	}
#ifdef _DEBUG
	unichar prTreeBuff[2048];
	void prTree()
	{
		int prDepth = 0;
		for(int i = 0; i < arbreCnt;i++)
			prNode(prDepth,racine[i]);
	}
	void prTreeId(int i)
	{
		int prDepth = 0;
		prNode(prDepth,racine[i]);
	}
	void prNode(int prdepth,
		struct arbre_string3_node * noeud)
	{
		int t = 0;
		int cur;
		struct avec_sorti_tran  *tt= noeud->transitions;
		while(tt){
			if(tt->c){
			cur  = swprintf(&prTreeBuff[prdepth],
				L"<%c,%x,%x,%x>",tt->c,tt->info,tt->a.node,tt->depth);
			} else {
			cur  = swprintf(&prTreeBuff[prdepth],
				L"< ,%x,%x,%x>",tt->info,tt->a.node,tt->depth);
			}
			if(tt->info & 0x8000){
				printf("%s\n",getUtoChar(prTreeBuff));
			} else
				prNode(cur+prdepth,tt->a.node);
			t++;
			tt = tt->suivant;
		}
		if(t) return;
printf("%sT\n",getUtoChar(prTreeBuff));
	}
#endif	// __DEBUG
	int taille_de_sauve;
	// check exist node
	//	and calcul offset
	//
	void check_exist_sur_path()
	{
		struct arbre_string3_node* base;
		struct avec_sorti_tran *tPtr;
		int enCnt = 0;
		int i;
		int infoTranCnt = 0;	// a node which has informations
		int noInfoTranCnt =0;	// a node without informations

		for( i = 0;  i < arbreCnt;i++){
			explore_for_access_check(racine[i]);
		}
		taille_de_sauve = 0;
		for( i = 0; i < (int)nodes.counter ;i++){
			base = (struct arbre_string3_node*)
			(nodes.addrMap[ i/ nodes.pgEMcnt] +
			(i % nodes.pgEMcnt) * sizeof(struct arbre_string3_node));
			if(base->offset == -2 ){
				enCnt++; // not access noeud
				continue;
			}
			if(base->offset != -1)	fatal_error("illegal information on the tree link");
			
			// 
			//	calcule the size of a node
			//
			tPtr = base->trans;
			while(tPtr){
				if(tPtr->info)
					infoTranCnt++;
				else
					noInfoTranCnt++;
				tPtr = tPtr->suivant;
			}
			base->offset = taille_de_sauve;
			taille_de_sauve += base->tcnt*7 + 2;
		}
		u_printf("before compressing, %d nodes,",nodes.counter);
		u_printf("after %d nodes(%d deleted nodes )\n",nodes.counter-enCnt,enCnt);
		u_printf("trans with info %d, trans without info %d\n",
			infoTranCnt,noInfoTranCnt);
		u_printf("size of tree is 0x%08x(%d) bytes\n",taille_de_sauve,taille_de_sauve);
	}

	void minimize_tree() {
		int i;
		u_printf("Minimizing...                      \n");
		//
		//
		//init_tab_by_hauteur();
		nodes.getAddrMap();
		avecTrans.getAddrMap();
		
		int H;
		int t =-1;
		for(i = 0; i < arbreCnt;i++){
			H=sort_by_height(racine[i]);
			if( t < H ) t = H;
		}
		H = t;
		mfusionner(H);
		u_printf("Minimization done.                     \n");
	}
	int sort_by_height(struct arbre_string3_node* n) 
	{
		if (n==NULL) fatal_error("Problem in sort_by_height\n");
		
		if (n->trans==NULL)
			fatal_error("illegal transition\n"); // if the node is a leaf
		struct avec_sorti_tran * trans=n->trans;
		int maxD = -1;
		while (trans!=NULL) {
			if(trans->info & 0x8000)
				trans->depth = 1;
			else
				trans->depth = sort_by_height(trans->a.node);
		  if(maxD < trans->depth ) maxD = trans->depth ;
		 trans = trans->suivant;
		}
		return 1+maxD;
	}
	void mfusionner(int size) {
		struct avec_sorti_tran* base;
		struct avec_sorti_tran* scan;
		int etCnt = 0;
		int i,j;
		int totTrans = avecTrans.counter;
		int eleCnt = avecTrans.pgEMcnt;
      u_printf("total transition %d \n\n",totTrans);
		for(int depIdx = 1; depIdx < size; depIdx++)
		{
         u_printf("\r:the %d depth nodes",depIdx);

			for( i = 0; i < totTrans - 1;i++){
            if(!( i % 1000)) u_printf("\r:the %d depth nodes at %d",depIdx,i );
				base = (struct avec_sorti_tran*)(
					avecTrans.addrMap[i/eleCnt] +
				(i % eleCnt) * sizeof(struct avec_sorti_tran));
				if(base->depth != depIdx) continue;
				if(base->info & 0x8000) continue;
				for( j = i+1; j< totTrans;j++){
					scan = (struct avec_sorti_tran*)(
						avecTrans.addrMap[ j/eleCnt] +
					(j % eleCnt )* sizeof(struct avec_sorti_tran));
					if(scan->depth != depIdx) continue;
//					if(base->c   != scan->c) continue;
					if(scan->info & 0x8000) continue;
//					if(base->info != scan->info) continue;
					if(base->a.node->tcnt != scan->a.node->tcnt) continue;
					if (compare_nodes(base->a.node->trans,scan->a.node->trans)==0){
						scan->a.node=base->a.node;
						scan->depth = -1;	// mark delete
						etCnt++;
					}
				} // loop for compared trans

			} // loop for pivot trans
		}	
      u_printf("\n%d node delete\n",etCnt);
		check_exist_sur_path();
	}
	int 
	compare_nodes(struct avec_sorti_tran* a,struct avec_sorti_tran* b)
	{
		while((a!=NULL) && (b!=NULL)) {
   // if the unichars are different then nodes are different
			if(a->c != b->c) return (1);
			if(a->info != b->info) return(1);
   // if the unichars are equal and destination nodes are different...
			if (a->a.node != b->a.node) return (1);
			a=a->suivant;
			b=b->suivant;
		}
		// if the transition lists are equal
		if ((a==NULL) && (b==NULL)) return 0;
		fatal_error("illegal count of transitions\n");
		return 1;
	}
	unsigned int *positionOfInfo;
	int offsetStrSave;
	void toBinTr(char *fname,int racOrSuf)
	{
		U_FILE *bfile;
		U_FILE *displayfile;
		char inf[1024];
		int i,j;
		class binHead0 tmpH;


		tmpH.cnt_auto = racName.size();
		if(tmpH.cnt_auto != arbreCnt) fatal_error("tree count error\n");
		tmpH.cnt_suf = sufName.size();
		tmpH.cnt_inf = baseInf.size();

		strcpy(inf,fname);
		strcat(inf,".bin");
		bfile=u_fopen(BINARY,inf,U_WRITE);
		if (!bfile) {
			fatal_error("Cannot create the file %s\n",inf);
		}
		tmpH.writeAtFile(bfile);	// reserve space
		
		strcpy(inf,fname);
		strcat(inf,".aut");
		displayfile=u_fopen(UTF16_LE,inf,U_WRITE);
		if (!displayfile) {
			fatal_error("Cannot create the file %s\n",inf);
		}

		unichar **a = racName.make_strPtr_table();
		unichar **b = sufName.make_strPtr_table();

		u_fprintf(displayfile,"%d\n",tmpH.cnt_auto);
		for(i = 0; i < tmpH.cnt_auto;i++)
		{
			u_fprintf(displayfile,"%S %d\n",&a[i][1]
				,racine[racName.check(&a[i][1])]->offset);
//			outbytes2((unichar)offsetStrSave,bfile);	// offset of name of racine
			outbytes3((unsigned int)racine[racName.check(&a[i][1])]->offset
				,bfile);		// offset of postition of state
		}
		offsetStrSave = 0;
		for(i = 0; i < tmpH.cnt_auto;i++){
			for(j = 1; j < a[i][0];j++)
				outbytes2((unichar)a[i][j],bfile);
			outbytes2(0,bfile);
			offsetStrSave += a[i][0];
		}
		racName.release_value();

//		for(i = 0; i < tmpH.cnt_suf;i++){
//			outbytes2((unichar)offsetStrSave,bfile);	// offset of name of racine
//		}
		u_fclose(displayfile);

		
		strcpy(inf,fname);
		strcat(inf,".suf");
		displayfile=u_fopen(UTF16_LE,inf,U_WRITE);
		if (!displayfile) {
			fatal_error("Cannot create the file %s\n",inf);
		}
		
		u_fprintf(displayfile,"%d\n",tmpH.cnt_suf);
		for(i = 0; i < tmpH.cnt_suf;i++)
		{
			u_fprintf(displayfile,"%S\n",&b[i][1]);
			for(j = 1; j < b[i][0];j++)
				outbytes2((unichar)b[i][j],bfile);
			outbytes2(0,bfile);
			offsetStrSave += b[i][0];
		}
		sufName.release_value();
		tmpH.size_str = offsetStrSave;

		u_fclose(displayfile);


		strcpy(inf,fname);
		strcat(inf,".inf");
		displayfile=u_fopen(UTF16_LE,inf,U_WRITE);
		if (!displayfile) {
			fatal_error("Cannot create the file %s\n",inf);
		}

		unichar **c = baseInf.make_strPtr_table();
		
//		positionOfInfo = new unsigned int[tmpH.cnt_inf];
		u_fprintf(displayfile,"%d\n",tmpH.cnt_inf);

		offsetStrSave = 0;
		for(i = 0; i < tmpH.cnt_inf;i++){
//			positionOfInfo[i] = offsetStrSave;
			u_fprintf(displayfile,"%S\n",&c[i][1]);
			for(j = 1; j < c[i][0];j++)
				outbytes2((unichar)c[i][j],bfile);
			outbytes2(0,bfile);
			offsetStrSave += c[i][0];
		}
		tmpH.size_inf = offsetStrSave;	
		baseInf.release_value();
		u_fclose(displayfile);

		struct arbre_string3_node* nPtr;
		struct avec_sorti_tran*tmp; 
		for( i = 0; i < (int)nodes.counter ;i++){
			nPtr = (struct arbre_string3_node*)
			(nodes.addrMap[ i/nodes.pgEMcnt] +
			i % nodes.pgEMcnt * sizeof(struct arbre_string3_node));
			if(nPtr->offset == -2 ) continue;
			if(nPtr->tcnt > 0x10000) fatal_error("too many transitions\n");
			outbytes2(nPtr->tcnt,bfile);
			tmp= nPtr->trans;
			while (tmp!=NULL) {
				outbytes2((unichar)tmp->c,bfile);
				outbytes2((unichar)tmp->info,bfile);
				if(tmp->info & 0x8000)
					outbytes3((unsigned int)tmp->a.node,bfile);
				else
					outbytes3((unsigned int)tmp->a.node->offset,bfile);
				tmp=tmp->suivant;
			}
		
		}
		fseek(bfile,0,0);
		tmpH.flag |= (racOrSuf) ? 0: TYPE_BIN_RACINE;
		tmpH.size_bin = taille_de_sauve;
		tmpH.size_ref = racName.size() * 3;
		tmpH.writeAtFile(bfile);	// reserve space
		u_fclose(bfile);
	}

};


//
//	class for saved in file and handling
//
typedef int (*actDansTrBin)(unichar *arg0,int depth,int inf,int soff
							,int sIdx);
class explore_bin1 {

public:
	class binHead0 head;
	unsigned char  *REF;
	unichar *STR;
	unichar *INF;
	unsigned char  *BIN;
	int margin_offset;
	unsigned int *SUF;
	unsigned int *AUT;
	unsigned int *autoffset;
	unsigned int *sufoffset;
	unsigned int *infoffset;

	char *name;
//	int offsetINF;


	actDansTrBin actFuncForFinal;
	actDansTrBin actFuncForInfo;	// function called when we find the final state	
	Alphabet *AlphabetTable;
	explore_bin1(){
		name = 0;
		BIN = 0;
		actFuncForInfo = 0;
		actFuncForFinal = 0;
		INF = 0;
		REF = 0;
		STR = 0;
		margin_offset = 0;
		sufoffset = 0;
		autoffset = 0;
		infoffset = 0;
		AlphabetTable = 0;
	};

	~explore_bin1(){
		if(REF) free(REF);
		if(sufoffset) delete sufoffset;
		if(autoffset) delete autoffset;
		if(infoffset) delete infoffset;
		if(name) free( name);
	};
	int isRacine(){ return(head.flag & TYPE_BIN_RACINE);};
	void setAlphabetTable(Alphabet *intValue)
    {
        AlphabetTable = intValue;
    }
	void loadBin(char *fname)
	{
		U_FILE *f;

		if(!(f = u_fopen(BINARY,fname,U_READ)))
			fopenErrMessage(fname);
		name = (char *)malloc(strlen(fname)+1);
		strcpy(name,fname);

		head.readFromFile(f);

		int i = 
			head.size_bin+
			head.size_ref+
			head.size_str*2+
			head.size_inf*2;
		REF = (unsigned char *)malloc(i);
		if(!REF) fatal_error("malloc fail\n");
		if(!fread(REF,i,1,f)) freadError(fname);
//                debug("read bin ok size = %d\n", i);

		margin_offset = head.size_bin;
		STR = (unichar *)(REF + head.size_ref);
		INF = (unichar *)(STR + head.size_str);	// STR is short
		BIN = (unsigned char *)((unsigned char*)INF + head.size_inf*2);
//		if(protype.protype)// bin endian; 
//			return;
		autoffset = new unsigned int[head.cnt_auto+1];
		sufoffset = new unsigned int[head.cnt_suf+1];
		infoffset = new unsigned int[head.cnt_inf];
		AUT = new unsigned int [head.cnt_auto+1];
		SUF = new unsigned int [head.cnt_suf+1];

		unichar v;

		int offsetStr = 0;
		int refIdx =0;
		for( i = 0; i < head.cnt_auto ;i++){
			offsetStr = REF[refIdx++] << 16;
			offsetStr |= REF[refIdx++] << 8;
			offsetStr |= REF[refIdx++];
			autoffset[i] = offsetStr;
		}
                unsigned char *sp;
                
		refIdx = 0;
        sp = (unsigned char *)STR;
		for( i = 0; i < head.cnt_auto ;i++){
			AUT[i] = refIdx;
			while( STR[refIdx]){
                v = (*sp++ << 8) & 0xff00;
                v |= *sp++ & 0xff;
//				STR[refIdx] = (((v >> 8) & 0xff) | ((v << 8) & 0xff00));
                               STR[refIdx] = v;
                               refIdx++;
			}
			u_printf("Initial state %d %s at %d\n",i
               ,getUtoChar(STR+AUT[i]),autoffset[i]);
			refIdx++;sp+=2;
		}

		//
		//
//        sp = (unsigned char *) &STR[refIdx];
		for(i = 0; i < head.cnt_suf;i++){
			SUF[i] = refIdx;
			while(STR[refIdx]){
                v = (*sp++ << 8) & 0xff00;
                v |= *sp++ & 0xff;
                STR[refIdx] = v;
//				STR[refIdx]= (((v >> 8) & 0xff) | ((v << 8) & 0xff00));
				refIdx++;
			}
			refIdx++;sp+=2;
			sufoffset[i] = 0;
//printf("Demand suffixe 0x%x",i);printf(" %s\n",getUtoChar(STR+SUF[i]));
		}
		if((unsigned int)&STR[refIdx] != (unsigned int)INF)
			fatal_error("illegal size STR %d (expect %d Cnt suf %d(%d)\n ",
                        (unsigned int)(&STR[refIdx]),(unsigned int)INF,i,head.cnt_suf);
		refIdx = 0;
        sp = (unsigned char *)INF;
		for( i = 0; i < head.cnt_inf;i++)
		{
			infoffset[i] = refIdx;
			while( INF[refIdx]){
                v = (*sp++ << 8) & 0xff00;
                v |= *sp++ & 0xff;
                INF[refIdx] = v;
//				INF[refIdx] = (((v >> 8) & 0xff) | ((v << 8) & 0xff00));
				refIdx++;
			}
//printf("INF 0x%x",i);printf(" %s\n",getUtoChar(INF+infoffset[i]));
			refIdx++;sp+=2;
		}
		if((unsigned int)&INF[refIdx] != (unsigned int)BIN)
			fatal_error("illegal size INF %d (expected %d) %d( %d) %d\n",
                        (unsigned int)&INF[refIdx],(unsigned int)BIN,refIdx,i,head.cnt_inf);
		u_fclose(f);
	}
	void exploreTree(U_FILE *f,unichar *pBuff)
	{
		int offset = 0;
		if(isRacine()){
			explore_state(autoffset[0],f,pBuff,offset);
		} else {
			for( int i = 0; i < head.cnt_auto;i++){
				explore_state(autoffset[i],f,pBuff,offset);
			}
		}
	}
	unichar ttemp_buff[1024];
	void explore_state(int pos,U_FILE *f,unichar *buff,int soffset)
	{
		int tcnt;
		unichar c;
		unichar info;
		int npos;
		int coffset;

		tcnt = BIN[pos++] << 8;
		tcnt |=BIN[pos++];
		if(!tcnt) fatal_error("illegal tree\n");
		while(tcnt){
			coffset = soffset;
			c = BIN[pos++]<< 8;
			c |= BIN[pos++];
			info = BIN[pos++]<< 8;
			info |= BIN[pos++];
			if(c){
            coffset=coffset+u_sprintf(&buff[coffset],"<%c,%dx",c,info);
			} else {
			   coffset=coffset+u_sprintf(&buff[coffset],"< ,%d",info);
				}
			npos  = BIN[pos++]<< 16;
			npos |= BIN[pos++]<< 8;
			npos |= BIN[pos++];
			if(info & 0x7fff){
				coffset=coffset+u_sprintf(&buff[coffset],"(%S),",INF + infoffset[info & 0x7fff]);
			}
			if(info & 0x8000){
				if(!npos){
				   coffset=coffset+u_sprintf(&buff[coffset],",T>\n");
				} else {
               coffset=coffset+u_sprintf(&buff[coffset],",%s>\n",STR+SUF[npos]);
				}
				u_fprintf(f,"%S",buff);
			} else {
				 coffset=coffset+u_sprintf(&buff[coffset],",0x%x>",npos);
				explore_state(npos,f,buff,coffset);
			}
			--tcnt;
		}
	}
	
	void set_act_func(actDansTrBin ft,actDansTrBin fi){
		actFuncForFinal = ft;
		actFuncForInfo = fi;
	}
	void searchMotAtTree(unichar *word,int saveIdx)
	{
		if(isRacine()){
		    scanNodes(autoffset[0],word,0,saveIdx);
		} else {
			for( int i = 0; i < head.cnt_auto;i++){
				scanNodes(autoffset[i],word,0,saveIdx);
			}
		}
	}
	//
	//
	//
	void scanNodes(int spos,unichar *word,int scanPosition,int soffset)
	{
		int tcnt;
		int pos = spos;
		unichar c;
		unichar info,sinfo;
		int npos;
		int noff;	// data save index
		int findCnt = 0;
		tcnt = BIN[pos++] << 8;
		tcnt |=BIN[pos++];
		if(!tcnt) fatal_error("illegal tree\n");
//	printf("addr 0x%08x::---\n",spos);	
		for(int i = 0; i < tcnt;i++){
			noff = soffset;
			c = BIN[pos++]<< 8;
			c |= BIN[pos++];
			info = BIN[pos++]<< 8;
			info |= BIN[pos++];
			sinfo = info & 0x8000;
			info &= 0x7fff;
			npos  = BIN[pos++]<< 16;
			npos |= BIN[pos++]<< 8;
			npos |= BIN[pos++];
			
//printf("0x%08x:%04x %04x %x %x\n",pos,word[scanPosition],c,info | sinfo,npos);			
			if(!word[scanPosition]){
				if(c) continue;
				if(sinfo){
					findCnt++;
					if(info)
                      noff = (*actFuncForInfo)(word,scanPosition,(int)(INF + infoffset[info]),0,noff);
					(*actFuncForFinal)(word,scanPosition,0,(int)((npos) ? SUF+sufoffset[npos]:0),noff);
				}
				continue;
			}
            if(!c){	// c == 0
				if(sinfo){
					if(!npos){	// check terminal condition
						if(!word[scanPosition]){
							if(info) 
                                 noff = (*actFuncForInfo)(word,scanPosition+1,(int)(INF + infoffset[info]),0,noff);
							(*actFuncForFinal)(word,scanPosition+1,0,0,noff);
						}
					} else {
						if(info)
                           noff = (*actFuncForInfo)(word,0,(int)(INF + infoffset[info]),0,noff);
						(*actFuncForFinal)(word,scanPosition,0,(int)SUF+sufoffset[npos],noff);
					}
				} else { // sinfo = 0;	the node for passing
					if(npos){ 
						if(info){	// if there is info , reset pointer value
							noff = (*actFuncForInfo)
							(word,0,(int)(INF + infoffset[info]),0,noff);
							scanNodes(npos,&word[scanPosition],0,noff);
						} else { // simple jmp
							scanNodes(npos,word,scanPosition,noff);
						}
					}	else {	// not find case
						fatal_error("find zero transition\n");
					}
				}
				continue;
			}
			if(  (word[scanPosition] == c) ||
                (AlphabetTable && is_upper_of(c,word[scanPosition],AlphabetTable))
            ){  if(sinfo){	// check terminal
					if(npos){
						if(info) {
						   noff = (*actFuncForInfo)(word,scanPosition+1,(int)(INF + infoffset[info]),0,noff);
						}
                        (*actFuncForFinal)(word,scanPosition+1,0,(int)(SUF+sufoffset[npos]),noff);
					} else {// check stop condition
						if(!word[scanPosition+1]){
							if(info)noff = (*actFuncForInfo)
							(word,scanPosition+1,(int)(INF + infoffset[info]),0,noff);
							(*actFuncForFinal)(word,scanPosition+1,0,0,noff);
						}
					}
				} else {	// go to next node
					if(npos){
						if(info){noff = (*actFuncForInfo)
							(word,scanPosition+1,(int)(INF + infoffset[info]),0,noff);
							scanNodes(npos,&word[scanPosition+1],0,noff);
						} else {
							scanNodes(npos,word,scanPosition+1,noff);
						}
					} else {
						fatal_error("No transition found\n");			
					}
				}
			}
		}
		if(!word[scanPosition] && !findCnt){
			(*actFuncForFinal)(0,scanPosition,1,0,soffset);
		}

	}


};
#endif // BIN3_DEF
