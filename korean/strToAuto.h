 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
//	construction a automate from th sequence of words
//
#ifndef STR_TO_AUTO_DEF
#define STR_TO_AUTO_DEF
#include "bitmap.h"
#include "segtype.h"
#include "etcclass.h"
#include "double_link_arbre.h"
#include "Fst2.h"

//
//	structure for head of file which save automates and structures of morphemes
//
//
struct localhead {
	unsigned short uni_mark;
	unsigned short dummy;
	int count_of_sentences;
	int offset_of_sentence_init;
	int cnt_of_morpheme;
	
	int offset_morp_struct;
	int cnt_of_flechi;		// number of orgine form in text
	int offset_of_flechi;
	int cnt_of_canonique;	// number of canonique form
	
	int offset_of_canonique;	
	int cnt_of_info;		// number of etiquette
	int offset_of_info;
	int cnt_of_infoarray;

	int offset_of_infoarray;
	int col_of_infoarray;
	int cnt_of_variation_ecrit;
	int offset_of_variation_ecrit;
	int offset_of_org_var;
	int cnt_of_org_var;
};

//
//	classification of tokens
//	symbol/token org/morph/sub-graphe
//
// token + 1 control	sous-graphe/phrase/paragraphe
//       |
//		 + 0 word + 1 sym + 1 num_poc  + 1 num
//			      |       |            + 0 poctuation  ? ! . 
//		          |       + 0 gra_cnt  + 1 control
//                |                    + 0 graphe   mg / 
//                + 0 lex + 1 token    + 1orgine 
//                        |            + 0 inconnu  // in the automate of motif
//				  	      + 0 morpheme + 1 multi
//				 	                   + 0 single	==> sequences of morpheme
//
//
//
//


#define TT_CONTROL_		    0x80000000

#define TE_SYMBOL_NUM		0x70000000
#define TE_SYMBOL_CTL		0x50000000
#define TE_SYMBOL_GRA		0x40000000

#define TE_SEGMENT			0x30000000	// orgin segment
#define TE_NEW_TOKEN		0x20000000	// unknown token with dictinnary
#define TE_MUL_MORPHEMES	0x10000000
#define TE_WORD_MORPHEME	0x00000000
#define TE_TOKEN_VAR		0x08000000	// form variante

#define MASK_OF_TYPE		0xf8000000
#define MASK_OF_INDEX		0x07ffffff		// 64 M
#define MAX_INDEX_VALUE		0x08000000

//
//
//
//
struct text_eti_fst2 {
	unsigned int noEtiq;	// indentify
	unsigned int offset;	// 
};
struct text_transition_fst2 {
	struct text_eti_fst2	eti;
	struct text_etat_fst2 *arr;
	struct text_transition_fst2 *next;
};

struct text_etat_fst2 {
	unsigned int index;
	struct text_transition_fst2 *trans;
};

struct tmp_trans {
	struct text_eti_fst2 eti;
	struct text_etat_fst2 *src;
	struct text_etat_fst2 *des;
	struct tmp_trans *next;
};

struct in_file_trans {
	unsigned int noEtiq;			// indentify
	unsigned int offset;	// if segment == morphe then orgin identif segment
	unsigned int nextEtatOffset;
}; 
struct saveValueStack {
	struct tmp_trans *lastcond;
	struct tmp_trans *previous;
	struct tmp_trans *current;
	int avance;
};
struct simpleLinks {
	int v;
	struct simpleLinks *n;
};

//class controlCaracterName {
//	controlCaracterName(){};
//	~controlCaracterName(){};
//};
	
typedef unsigned short TypeChar;

//
//
//
//
class str2Auto {
	pageHandle nodes;
	pageHandle trans;
	pageHandle conds;
	pageHandle etis;
	struct saveValueStack aStack[1024];
	int cStackIdx;
	TypeChar *curScanP;
	TypeChar *epsilon;
	TypeChar coordination;
	TypeChar ordination;
	TypeChar entreOpen;
	TypeChar entreClose;
	TypeChar escapeChar;
	TypeChar escapeZoneOpen;
	TypeChar escapeZoneClose;
	unsigned int epsilonIndex;
	int autoEtatNumber;
	int etiOffset;	// offset for distingua 
	unsigned int passingNumber;
	int currentSegNumber;
public:

	arbre_string00 morphStruct;

	TypeChar *saveTokenP;
	int saveIdx;
	int blancMarkIndex;
#ifdef DEBUG
	FILE *debugf;
#endif
	str2Auto(){
		nodes.setSz(sizeof(struct text_etat_fst2)*1024,sizeof(struct text_etat_fst2));
		trans.setSz(sizeof(struct text_transition_fst2)*1024,sizeof(struct text_transition_fst2));
		conds.setSz(sizeof(struct tmp_trans)*1024,sizeof(struct tmp_trans));
		cStackIdx = 0;
		saveIdx = 0;
		saveTokenP = new unsigned short[1024];
		etiOffset = 0;
		epsilonIndex = morphStruct.insertWord(u_epsilon_string);

#ifdef DEBUG
		debugf = u_fopen("test.txt",U_WRITE);
#endif
	}
	~str2Auto(){
		delete saveTokenP;
#ifdef DEBUG
		fclose(debugf);
#endif
	}
	void resetMem(){
		nodes.resetAssign();
		trans.resetAssign();
		conds.resetAssign();
	}
	struct text_etat_fst2 *newEtat()
	{
		struct text_etat_fst2 *p = (struct text_etat_fst2 *)nodes.get();
		p->index = 0;
		p->trans = 0;
		return(p);
	}
	struct text_transition_fst2 *newTrans()
	{
		struct text_transition_fst2 *t = (struct text_transition_fst2 *)trans.get();
		t->next = 0;
		return(t);
	}
	struct tmp_trans *newCond(struct text_etat_fst2 *src,
						   struct text_etat_fst2 *des,
						   int seg,int off )
	{
		struct tmp_trans *s = (struct tmp_trans *)conds.get();
		s->src = src;
		s->des = des;
		s->eti.noEtiq = seg;
		s->eti.offset = off;
		s->next = 0;
		return(s);
	}
	void setCtrlChar(TypeChar *format,int offset)
	{

		ordination = *format++;
		coordination = *format++;
		entreOpen = *format++;
		entreClose = *format++;
		escapeZoneOpen = *format++;
		escapeZoneClose = *format++;
		blancMarkIndex = 0;
		etiOffset  = offset;
		escapeChar = '\\';
	}
	void setBlancSym(unsigned short *s)
	{
		blancMarkIndex = morphStruct.insertWord(s);
	}
	void setEspilonIndex(unsigned short *s)
	{
		epsilonIndex = morphStruct.insertWord(s);
	}
	void insBr(struct text_eti_fst2 *id,struct text_etat_fst2 *s,struct text_etat_fst2 *d)
	{
	struct text_transition_fst2 **t= &s->trans;
	while(*t){ 
		if( 
			((*t)->eti.noEtiq == id->noEtiq) && 
			((*t)->eti.offset == id->offset) && 
			((*t)->arr == d)
			)
			return; 
		t = &(*t)->next;}
	struct text_transition_fst2 *n = (struct text_transition_fst2 *)trans.get();
	n->arr = d;
	n->eti.noEtiq = id->noEtiq;
	n->eti.offset = id->offset;
	n->next = (*t);
	(*t) = n;
	}
    text_etat_fst2 * initStack()
    {
    	struct text_etat_fst2 *initEtat=newEtat();
    	saveIdx = 0;
    	cStackIdx = 0;
    	saveTokenP[0] = 0;
    	aStack[cStackIdx].previous = 0;
    	aStack[cStackIdx].current = newCond(initEtat,initEtat,epsilonIndex,0);
    	aStack[cStackIdx].avance = 1;
    	return(initEtat);
    }
    void saveToken()
    {
    	struct tmp_trans *stemp,*dtemp;
    	struct tmp_trans *t;
    	int mnum;
    	
    	if(aStack[cStackIdx].avance){
    		stemp = aStack[cStackIdx].previous;
    		while(stemp){
    			dtemp = aStack[cStackIdx].current;
    			while(dtemp){
    				insBr(&dtemp->eti,stemp->des,dtemp->src);
    				dtemp = dtemp->next;
    			}
    			stemp = stemp->next;
    		}
    		aStack[cStackIdx].previous = aStack[cStackIdx].current;
    		aStack[cStackIdx].current = 0;
    	}
    	if(saveIdx){
    		saveTokenP[saveIdx] = 0;
    		mnum = morphStruct.insertWord(saveTokenP) | TE_WORD_MORPHEME;
    		t= newCond(newEtat(),0,mnum,currentSegNumber);
    		t->des = t->src;
    	} else {
    		// autmate has only initial and terminal states.
    		if(!aStack[cStackIdx].lastcond)
    			return;
    		t = aStack[cStackIdx].lastcond;
    		aStack[cStackIdx].lastcond = 0;
    	}
    	t->next = aStack[cStackIdx].current;
    	aStack[cStackIdx].current = t;
    	saveIdx = 0;
    }
void controlStack(TypeChar curChar)
{
	struct text_etat_fst2 *ep;
	struct tmp_trans *t,*stemp,*dtemp;

	if(curChar == coordination){
		saveToken();
		aStack[cStackIdx].avance = 0;
		return;
	}
	if(curChar == ordination){
		saveToken();
		aStack[cStackIdx].avance = 1;
		return;
	}
	if(curChar == entreOpen){
//		saveToken();
		t = newCond(newEtat(),0,epsilonIndex,0);
		aStack[cStackIdx].lastcond = t;
//		t->next = aStack[cStackIdx].current;
//		aStack[cStackIdx].current = t;
		cStackIdx++;
		aStack[cStackIdx].lastcond = t;
		aStack[cStackIdx].avance = 1;
		aStack[cStackIdx].previous = 0;
		aStack[cStackIdx].current = newCond(t->src,t->src,epsilonIndex,0);		
	}
	if(curChar == entreClose){
		saveToken();
		aStack[cStackIdx].avance=1;
		stemp = aStack[cStackIdx].previous;
		while(stemp){
			dtemp = aStack[cStackIdx].current;
			while(dtemp){
				insBr(&dtemp->eti,stemp->des,dtemp->src);
				dtemp = dtemp->next;
			}
			stemp = stemp->next;
		}
		aStack[cStackIdx].previous = aStack[cStackIdx].current;
		aStack[cStackIdx].current = 0;
		ep = newEtat();
		t = aStack[cStackIdx].previous;
		struct text_eti_fst2 teti;
		teti.noEtiq = epsilonIndex;
		teti.offset = 0;
		while(t){
			insBr(&teti,t->des,ep);
			t = t->next;
		}
		if(cStackIdx){
			cStackIdx--;
			aStack[cStackIdx].lastcond->des = ep;
		}
	}
}
struct text_etat_fst2 *constructAutoFromStr(TypeChar *iString,int offset)
{
//	if(*iString != entreOpen) fatal_error("bad string for the construction text auto\n");
	curScanP = iString;
//	curScanP++;

	currentSegNumber = offset;
	int openFlag = 1;
	struct text_etat_fst2 *init=initStack();

	while(*curScanP){
		if((*curScanP == ordination) ||
		(*curScanP == coordination) ||
		(*curScanP == entreOpen) ||
		(*curScanP == entreClose) ){
			if(openFlag){
				controlStack(*curScanP++);
				continue;
			}
		}
		if(*curScanP == escapeZoneOpen) openFlag = 0;
		if(*curScanP == escapeZoneClose) openFlag = 1;
		if(*curScanP ==  escapeChar){
			curScanP++;
			if(!*curScanP) 
				fatal_error("bad escape character\n");
		}
		saveTokenP[saveIdx++] = *curScanP;
		curScanP++;
	}
	controlStack(entreClose);
	if(cStackIdx) fatal_error("bad string format\n");
	return(init);
}
    void setIndexNumber(struct text_etat_fst2 *s)
    {
    	int totEtat = nodes.getSize();
    	
    	struct text_etat_fst2 *lp;
    	int i;
    
    	for(i = 0; i < totEtat; i++){
    		lp = (struct text_etat_fst2 *)nodes.getAddr(i);
    		lp->index = 0xcacacaca;
    	}
    	lp = (struct text_etat_fst2 *)nodes.getAddr(0);	
    	if(s != lp) fatal_error("what\n");
    	autoEtatNumber = 0;
    	s->index = 0xfcfcfcfc;
    	autoEtatNumber++;
    	struct text_transition_fst2 *a=s->trans;
    	while(a){ markEtatNumber(a->arr); a = a->next;}
    
    }
	void markEtatNumber(struct text_etat_fst2 *s)
	{
		if(s->index == 0xfcfcfcfc) return;
		s->index = 0xfcfcfcfc;
		autoEtatNumber++;
		struct text_transition_fst2 *a=s->trans;
		while(a){ markEtatNumber(a->arr); a = a->next;}
	}
	
	int  outLesEtatToFile(FILE *f)
	{
		int totEtat = nodes.getSize();
		int sumOfOffset = 0;
		struct text_etat_fst2 *lp;
		struct text_transition_fst2 *s;
		int i;
		int tcnt;
		int indexCounter =0;
		int start_global_offset = ftell(f);
		int offset_start_at_file = start_global_offset;


		start_global_offset += 4; // for the count of states of a automata
		for(i = 0; i < totEtat; i++){
			lp = (struct text_etat_fst2 *)nodes.getAddr(i);
			if(lp->index == 0xfcfcfcfc){
				indexCounter++;
				lp->index = start_global_offset + sumOfOffset;
				s = lp->trans;
				tcnt = 0;
				while(s){
					tcnt++;
					s = s->next;
				};
				sumOfOffset += 4 + 
					(sizeof(struct text_eti_fst2)+sizeof(int))*tcnt;	// for couunter
			} else {
				lp->index = 0xcacacaca;
			}
		}

		if(indexCounter != autoEtatNumber){
#ifdef DEBUG
			prAuto(debugf,0);
#endif
			fatal_error("illegal etat count\n");
		}
		fwrite(&autoEtatNumber,4,1,f);	// number of state in a automate
		for(i = 0; i < totEtat; i++){
			lp = (struct text_etat_fst2 *)nodes.getAddr(i);
			if(lp->index == 0xcacacaca) continue;

			s = lp->trans;
			tcnt = 0;
//			if(!s){
//				if(lp->index == indexCounter -1 ){
//					// terminal etat
//					fwrite(&tcnt,sizeof(int),1,f);
//					continue;
//				} else
//					fatal_error("illegal state\n");
//			}			
			while(s){++tcnt;s = s->next;};
			fwrite(&tcnt,sizeof(int),1,f);
#ifdef PRDEBUG
error("%d:%d:%d",i,lp->index,tcnt);
#endif // PRDEBUG
			s = lp->trans;
			while(s){
#ifdef PRDEBUG
error("<%d:%d>",s->arr->index,s->eti.noEtiq);
#endif // PRDEBUG
				fwrite(&s->eti,sizeof(text_eti_fst2),1,f);
				fwrite(&s->arr->index,sizeof(int),1,f);
				s = s->next;
			};
#ifdef PRDEBUG
error("\n");
#endif // PRDEBUG
		}
		


		return(offset_start_at_file);
	}

	void cleanIndex(struct text_etat_fst2 *s)
	{
    	struct text_transition_fst2 *a;
    	s->index = 0;
    	a = s->trans;
		while(a){
			if(!a->arr) fatal_error("illegal\n");
			if(a->arr->index) cleanIndex(a->arr);
			a = a->next;
		}
    	
	}

	int getAutoEtatNumber()
	{
		return(autoEtatNumber);
	}
	void elimineSameEtiTrans(int setNum,struct text_etat_fst2 *base)
	{
		passingNumber = setNum;
		exploreEliminationOfSameEti(base);
	}
	void exploreEliminationOfSameEti(struct text_etat_fst2 *base)
	{
		struct text_transition_fst2 **pTrans,**sTrans,*cTrans;
		if(!base->trans) return;
		if(base->index == passingNumber ) return;
		pTrans = &base->trans;
		while(*pTrans){
			sTrans = &(*pTrans)->next;
		while(*sTrans){
					if(((*pTrans)->eti.noEtiq == (*sTrans)->eti.noEtiq) && 
					((*pTrans)->eti.offset == (*sTrans)->eti.offset)){
					cTrans = (*sTrans)->arr->trans;
					while(cTrans){
						insBr(&cTrans->eti,(*pTrans)->arr,cTrans->arr);
						cTrans = cTrans->next;
					}
					(*sTrans)=(*sTrans)->next;
					continue;
				}
				sTrans= &(*sTrans)->next;
			}
			pTrans = &(*pTrans)->next;
		}
		base->index = passingNumber;
		cTrans = base->trans;
		while(cTrans){
			if(cTrans->arr->trans)
				exploreEliminationOfSameEti(cTrans->arr);
			cTrans = cTrans->next;
		}
	}

	void elimineEpsilonTrans(int setNum,struct text_etat_fst2*st)
	{
		passingNumber = setNum;	// eviter rendre viste deux fois
		exploreForRemoveEspilon(st);
	}
	void exploreForRemoveEspilon(struct text_etat_fst2 *base)
	{
    	if(base->index == passingNumber) return;
    	if(!base->trans){
          base->index = passingNumber;return;
          }
	
    	struct text_transition_fst2 **Ptrans,*strans;
    	Ptrans = &base->trans;
    	while(*Ptrans){
    		if(((*Ptrans)->eti.noEtiq == epsilonIndex) 
    			&& ((*Ptrans)->arr->trans)){
    				if((*Ptrans)->arr == base) fatal_error("what is cycle???\n");
    				strans = (*Ptrans)->arr->trans;
    				while(strans){
    					insBr(&strans->eti,base,strans->arr);
    					strans = strans->next;
    				}
    				*Ptrans = (*Ptrans)->next;	// eliminer
    				continue;
    			}
    			Ptrans = &(*Ptrans)->next;
		}
		base->index = passingNumber;
		strans = base->trans;
		while(strans){
			exploreForRemoveEspilon(strans->arr);
			strans = strans->next;
		}
	}

	void setTerminalBranch(int setNum,struct text_etat_fst2 *s,struct text_etat_fst2 *t)
	{
		passingNumber = setNum;
		exploreSetTer(s,t);
	}
	void exploreSetTer(struct text_etat_fst2 *s,struct text_etat_fst2 *t)
	{
		if(s == t) return;
		if(s->index == passingNumber) return;
		s->index = passingNumber;
		struct text_transition_fst2 *scan = s->trans;
		while(scan){
			if(scan->arr->trans){
				exploreSetTer(scan->arr,t);
			} else {
				scan->arr = t;
			}
			scan = scan->next;
		}
	}


// debugging
	void prAuto(FILE *fde,unsigned int flag)
	{
		struct text_etat_fst2* base;
		struct text_transition_fst2 *strans;
		int i;
		int totNodes = nodes.counter;
		int eleCnt = nodes.pgEMcnt;
		unsigned int *addrmap= new unsigned int[(totNodes + eleCnt -1 )/eleCnt];
		fprintf(fde,"total node %d \n\n",totNodes);
		nodes.addMapOut(addrmap);
		for( i = 0; i < totNodes ;i++){
			base = (struct text_etat_fst2 *)(addrmap[i/eleCnt] +
			(i % eleCnt) * sizeof(struct text_etat_fst2));
			if(flag && (base->index != flag)) continue; 
			fprintf(fde,"<%08x:%08x:0x%08x>:",base->index,i,(unsigned int)base);
			strans = base->trans;
			while(strans){
				fprintf(fde,"(0x%08x:0x%08x:0x%08x)",strans->eti.noEtiq,
					strans->eti.offset,(unsigned int)strans->arr);
				strans = strans->next;
			}
			fprintf(fde,"\n");
		}
		fflush(fde);
		delete addrmap;
	}
	void prEtiquette(FILE *fout)
	{
	    int *c;
		unsigned short **m = morphStruct.make_strPtr_table(&c);
		int sz = morphStruct.size();
		for (int i = 0; i < sz;i++){
			m[i][0] = '%';
			u_fprintf(fout,"%S\n",m[i]);
		}
		morphStruct.release_value();
	}


};


//
//	morpheme 
//
//
//
//
//
	
class morpheme_info: public str2Auto ,public nameOfControlChars{
public:
	FILE *phraseAutoMap;
	unsigned short tbuff[4096];
	char pathNameStore[4096];
	char ftemp[2048];
	struct localhead head;
	int phraseMark;
	struct morpheme_struct {	// structure for save 
		int idx_f;
		int idx_c;
		int idx_i;
	} *morStructTable;

	unsigned short **flechi_table;
	unsigned short *flechi_map;
	unsigned short **canoni_table;
	unsigned short *canoni_map;
	unsigned short **linfos_table;
	unsigned short *linfos_map;

	unsigned int *infoArray_table;
	
	arbre_string0 tokenStruct;
	arbre_string0	flechi; 
	arbre_string0	canon;
	arbre_string0	info;
	arbre_string0	infoArr;

	double_arbre_string0	variation_ecriture;

	struct org_variation_table {
		int org;	// index of orgin
		int var;	
	} *orgConvTable;
	pageHandle  variation_table;
	


	//
	//	when creatation using for save structures of finded tokens
	//	when cosulation using for morphemses
	unsigned short **mophTable;
	int mophTableSz;
	unsigned short *mophMap;
	
	unsigned short **tokTable;
	int tokTableSz;
	unsigned short *tokMap;

	int info_col;
	class link_tables<int> sentenceInit;
	morpheme_info(){
		head.uni_mark=0;
		head.count_of_sentences = 0;

		phraseMark = -1;
		head.cnt_of_flechi=0;
		head.cnt_of_canonique=0;
		head.cnt_of_info=0;
		head.col_of_infoarray=0;

		flechi_table = 0;
		canoni_table = 0;
		linfos_table =0;
		flechi_map = 0;
		canoni_map = 0;
		linfos_map =0;
		infoArray_table = 0;
		morStructTable = 0;
		tokMap = 0;
		mophMap = 0;
		sentenceInit.setMaxSz(1024);
		variation_table.setSz(sizeof(struct org_variation_table)*1024,
			sizeof(struct org_variation_table));
	};
	~morpheme_info(){
		if(flechi_table) delete flechi_table;
		if(canoni_table) delete canoni_table;
		if(linfos_table) delete linfos_table;
		if(flechi_map) delete flechi_map;
		if(canoni_map) delete canoni_map;
		if(linfos_map) delete linfos_map;
		if(infoArray_table) delete infoArray_table;
		if(morStructTable) delete morStructTable;
		if(tokMap) delete tokMap;
		if(mophMap) delete mophMap;

	};
	void pathNameSet(char *f)
	{
	  get_path(f,pathNameStore);
	}
	FILE *
	openForWrite(char *f)
	{
		strcpy(ftemp,pathNameStore);
		strcat(ftemp,"phrase.cod");
		if((phraseAutoMap = fopen(ftemp,"wb")) == 0)
			fopenErrMessage(ftemp);
		fwrite(&head,sizeof(struct localhead),1,phraseAutoMap);
		return(phraseAutoMap);
	}
	void write_head()
	{
		fseek(phraseAutoMap,0,0);
                u_fputc_UTF16LE_raw(0xfeff,phraseAutoMap);
                u_fputc_UTF16LE_raw(0xfffe,phraseAutoMap);
                unsigned char b[4];
                unsigned int i= head.count_of_sentences;
                b[0] = i & 0xff;i = i >> 8;
                b[1] = i & 0xff;i = i >> 8;
                b[2] = i & 0xff;i = i >> 8;
                b[3] = i & 0xff;
                fwrite(b,4,1,phraseAutoMap);         
		fwrite(&head.offset_of_sentence_init,
                sizeof(struct localhead)-8,1,phraseAutoMap);			
	}
	void read_head()
	{
		fseek(phraseAutoMap,0,0);
               unichar a = u_fgetc_UTF16LE_raw(phraseAutoMap);
                a =u_fgetc_UTF16LE_raw(phraseAutoMap);
                
                unsigned char b[4];
                fread(b,4,1,phraseAutoMap);
                unsigned int i;
                i =  b[0]
                     | (( b[1] << 8 ) & 0xff00)
                     | (( b[2] << 16) & 0xff0000)
                     | (( b[3] << 24) & 0xff000000);
                head.count_of_sentences  = i;
		fread(&head.offset_of_sentence_init,
                sizeof(struct localhead)-8,1,phraseAutoMap);			
	}

	void loadTokensMap(char *f)
	{		
		FILE *fptr;
		strcpy(ftemp,pathNameStore);
		strcat(ftemp,"tokens.txt");
		u_printf("load Tokens file\n");
		if((fptr = u_fopen(ftemp,U_READ)) ==0 )	fopenErrMessage(f);
		fseek(fptr,0,SEEK_END);	
		int sizeFile =ftell(fptr)/2;
		tokMap = new unsigned short[sizeFile];
		if(!tokMap) fatal_error("mem alloc fail\n");
		fseek(fptr,2,SEEK_SET);
		if(!u_fread_raw(tokMap,sizeFile-1,fptr))
			fatal_error("Read Tokens fail\n");
		tokMap[sizeFile-1] = 0;
//		tokMap[sizeFile-2] = 0;
		loadStrTableFile(tokMap,tokTable,tokTableSz,1);
		fclose(fptr);
	}
#ifdef FFFFFFF
	void loadTokensStruct(char *f)
	{
		
		
		FILE *fptr;

		strcpy(temp,pathNameStore);
		strcat(temp,"tokens.txt");
		if((fptr = u_fopen(temp,U_READ)) ==0 )	fopenErrMessage(temp);
		fseek(fptr,0,SEEK_END);	
		int sizeFile =ftell(fptr);
		tokMap = new unsigned char[sizeFile];
		if(!tokMap) fatal_error("mem alloc fail\n");
		fseek(fptr,2,SEEK_SET);
		if(!fread(tokMap,sizeFile-2,1,fptr))
			fatal_error("Read Tokens fail\n");
		tokMap[sizeFile-1] = 0;
		tokMap[sizeFile-2] = 0;
		loadStrStructFile((unsigned short *)tokMap,tokenStruct);
		phraseMark = tokenStruct.check(u_phraseMark_string);
		delete tokMap;
		tokMap = 0;
		fclose(fptr);
	}
#endif //FFFFFFFFF
	void loadSousTokensMap(char *f)
	{
		FILE *fptr;

		strcpy(ftemp,pathNameStore);
		strcat(ftemp,"seqMorphs.txt");
		if((fptr = u_fopen(ftemp,U_READ)) ==0 )	fopenErrMessage(ftemp);
		fseek(fptr,0,SEEK_END);	
		int sizeFile =ftell(fptr)/2;
		mophMap = new unsigned short[sizeFile];
		if(!mophMap) fatal_error("mem alloc fail\n");
		fseek(fptr,2,SEEK_SET);
		if(!u_fread_raw(mophMap,sizeFile-1,fptr))
			fatal_error("Read Tokens fail\n");
		mophMap[sizeFile-1] = 0;
//		mophMap[sizeFile-2] = 0;
		loadStrTableFile(mophMap,mophTable,mophTableSz,1);		
		fclose(fptr);
	}
	void 
	loadStrTableFile(unsigned short *wp,unsigned short **&table,int &table_sz,int cflag)
	{
		table_sz = 0;
		for(;(*wp >= '0') && (*wp <= '9') ;wp++)
				table_sz = table_sz * 10 + *wp - '0';
		
		if(*wp == '\r') wp++;
		if(*wp == '\n') wp++;

		table = new unsigned short *[table_sz];
		if(!table) fatal_error("token table mem alloc fail\n");
		unsigned short *curoffset = wp;
		int index  = 0;
		while(*wp ) {
			if(*wp == 0x0a){
				*wp++ = 0;
				if(cflag && (*curoffset == '{')
                     && !u_strcmp(curoffset,u_phraseMark_string))
					phraseMark = index;
				table[index++] = curoffset;
				curoffset = wp;
			} else  if(*wp == 0x0d) {
				*wp++ = 0;
			} else
				wp++;			
		};
		if( index != table_sz) fatal_error("illegal table size\n");
	}
	
	void
	loadStrStructFile(unsigned short *wp,class arbre_string0 &strStr)
	{
		unsigned short *curoffset;
		int table_sz = 0;
		for(;(*wp >= '0') && (*wp <= '9') ;wp++)
				table_sz = table_sz * 10 + *wp - '0';
		
		if(*wp == '\r') wp++;
		if(*wp == '\n') wp++;

		while(*wp ) {
			if(*wp == 0x0a){
				*wp++ = 0;
				if(*curoffset == 0) fatal_error("Format Error\n");
				strStr.put(curoffset);
			} else  if(*wp == 0x0d) {
				*wp++ = 0;
			} else
				wp++;			
		};
		if(table_sz != strStr.size())
			fatal_error("illegal token table size\n");
	}
	void saveMorphems()
	{
		FILE *f = phraseAutoMap;			
        FILE *fmor;
		int count_eti; 
		unsigned short *wp;
		unsigned short *sp;
		int *frequenceMap;
		int sz = morphStruct.size();
		
        strcpy(ftemp,pathNameStore);
		strcat(ftemp,"Morphemes.txt");
		if((fmor = u_fopen(ftemp,U_WRITE)) ==0 ) fopenErrMessage(ftemp);
		
		strFileHeadLine(fmor,sz);
		morphStruct.save_tout_leaf(fmor,1);
		fclose(fmor);
		
		unsigned short **m = morphStruct.make_strPtr_table(&frequenceMap);
		
		unsigned short tmpEti[2048];
		
		head.offset_morp_struct = ftell(f);
		head.cnt_of_morpheme = sz;
        int i;
		morStructTable = new struct morpheme_struct [sz];
		if(!morStructTable) fatal_error("mem alloc fail\n");
		flechi.put(u_epsilon_string);
		canon.put(u_epsilon_string);
		info.put(u_epsilon_string);
		

		
		for (i = 0; i < sz;i++){

		    if(i < 2) continue;// skip for <E>,<BL>
			wp = m[i];
			wp++;   // skip count of characters

			if(*wp != '{') illegal_format(wp);
			sp = ++wp;
			while(*wp != ','){ 
				if(*wp == '\0') illegal_format(&m[i][1]);
				wp++;
			}
			*wp = '\0';
			if(sp == wp){
				morStructTable[i].idx_f = 0;
			} else {
				morStructTable[i].idx_f = flechi.put(sp);
			}
			wp++;
			sp = wp;
			while(*wp != '.'){ 
				if(*wp == '\0') illegal_format(&m[i][1]);
				wp++;
			}
			*wp++ = '\0';
			morStructTable[i].idx_c=canon.put(sp);
			count_eti = 0;
			sp = wp;

			while(*wp){
			switch(*wp){
			case '}':
			case '+':
			case '.':
				*wp++ = '\0';
//				if(!*sp) illegal_format(&m[i][1]);
//				if(!u_strcmp(sp,(unsigned short *)L"i<E")) exit(1);
				tmpEti[count_eti] =info.put(sp);
#ifdef DEBUG
u_fprintf(debugf,"<%S:%d>",sp,tmpEti[count_eti]);
#endif
				count_eti++;			
				sp =wp;
				break;
			default:
				wp++;
			}
		}
		if(!count_eti) illegal_format(&m[i][1]);
		sortArray(tmpEti,count_eti);
		tmpEti[count_eti] = 0;
		morStructTable[i].idx_i = infoArr.put(tmpEti);
#ifdef DEBUGG
for(int kk  = 0; kk<count_eti;kk++) fprintf(debugf,"%04\n",tmpEti[kk]);
#endif
		}
		fwrite(morStructTable,sizeof(struct morpheme_struct)*sz,1,f);
		morphStruct.release_value();
		
	//
	//	save 
	//
		unsigned short **tables;
		int tab_cnt;
		
		tab_cnt = flechi.size();
		head.cnt_of_flechi = tab_cnt;
		head.offset_of_flechi = ftell(f);
		tables = flechi.make_strPtr_table();
#ifdef DDEBUG
fprintf(debugf,"\nflechie form %d \n",tab_cnt);
#endif
		for(i = 0; i < tab_cnt;i++){
		u_fprintf(f,"%S\n",&tables[i][1]);
#ifdef DDEBUG
		fprintf(debugf,"%s\n",getUtoChar(&tables[i][1]));
#endif
		}
		flechi.release_value();

		tab_cnt =  canon.size();
		head.cnt_of_canonique = tab_cnt;
		head.offset_of_canonique = ftell(f);
		tables = canon.make_strPtr_table();
#ifdef DDEBUG
fprintf(debugf,"\ncannonique form %d \n",tab_cnt);
#endif 
		for(i = 0; i < tab_cnt;i++){
			u_fprintf(f,"%S\n",&tables[i][1]);
#ifdef DDEBUG
			fprintf(debugf,"%s\n",getUtoChar(&tables[i][1]));
#endif //DDEBUG
		}
		canon.release_value();

		tab_cnt =  info.size();
		head.cnt_of_info = tab_cnt;
		head.offset_of_info = ftell(f);
		tables = info.make_strPtr_table();
#ifdef DDEBUG
fprintf(debugf,"\ninfos form %d \n",tab_cnt);
#endif
		for(i = 0; i < tab_cnt;i++){
			u_fprintf(f,"%S\n",&tables[i][1]);
#ifdef DDEBUG
		fprintf(debugf,"%s\n",getUtoChar(&tables[i][1]));
#endif // DDEBUG
		}
		info.release_value();


	//
	//	compress linguistic informations 
	//
		
		head.offset_of_infoarray = ftell(f);
		head.cnt_of_infoarray = infoArr.size();
		head.col_of_infoarray=
		makeInfoTable(f,info.size(),infoArr);

		head.cnt_of_variation_ecrit = variation_ecriture.size();
		head.offset_of_variation_ecrit = ftell(f);

		
		int di = variation_ecriture.makeTableTNoeud();
		for( i = 0; i < di;i++)// out 
			u_fprintf(f,"%S\n",variation_ecriture.getOneItemWithIndex(i));

		head.offset_of_org_var = ftell(f);

		head.cnt_of_org_var = variation_table.tableOutToFile(f);

		{	// for debugging
			unsigned short *dp;
			struct org_variation_table *base;
			di =  variation_table.getSize();
			
			for( i = 0; i < di;i++){
				base = (struct org_variation_table *)variation_table.getAddr(i);
				dp = variation_ecriture.getOneItemWithIndex(base->var);

				if(*dp){
#ifdef DDEBUG
					fprintf(debugf,"%d %s ",base->org,getUtoChar(dp));
                    fprintf(debugf,"%s\n",getUtoChar(tokTable[base->org]));
#endif
				} else {
				    error("%d %s %s\n",base->org,getUtoChar(dp),getUtoChar(tokTable[base->org]));
					fatal_error("illegal\n");
				}
			}
			variation_ecriture.releaseTable();
		}
	}
//
//	make bit array for info
//	return : size of the table

	int
	makeInfoTable(FILE *f,int count_of_infos,arbre_string0 &infoArray)
	{
		unsigned short **tables;
		unsigned int *wi;
		int i,j;
		int row_cnt;
		int colon_size;
		row_cnt = infoArray.size();
		colon_size = (count_of_infos+31)/32 ; // number of data size
		infoArray_table = new unsigned int[row_cnt*colon_size];
	
		tables = infoArray.make_strPtr_table();
		wi = infoArray_table;
		for(i = 0; i < row_cnt;i++){
			for( j = 0; j < colon_size;wi[j++] = 0);
			for( j = 1; j < tables[i][0];j++){
//printf("%08x",tables[i][j]);
				wi[(tables[i][j]/32)%colon_size] |=
				bitSetL[tables[i][j]%32];
			}
//printf("|||");
//for(j = 0; j < colon_size;j++)	printf("%08x",wi[j]);
//printf("\n");
			wi += colon_size;
		}
		infoArray.release_value();
		fwrite(infoArray_table,row_cnt*colon_size*4,1,f);
		return(colon_size);
	}

	void illegal_format(unsigned short *m)
	{
		fatal_error("%s is Illegal format\n",getUtoChar(m));
	}
	void sortArray(unsigned short *a,int sz)
	{
	int i,j;
	unsigned short t;
	for( i = 0; i < sz ; i++)

		for( j = i +1 ; j < sz;j++)
			if(a[i] > a[j]){
				t = a[i]; a[i] = a[j]; a[j] = t;
			}
	}
	void 
	loadMorphemes(char *fname)
	{
			
		strcpy(ftemp,pathNameStore);
		strcat(ftemp,"phrase.cod");
		if((phraseAutoMap = fopen(ftemp,"rb")) ==0 ) fopenErrMessage(ftemp);

		read_head();

		fseek(phraseAutoMap,head.offset_morp_struct,SEEK_SET);
		morStructTable = new struct morpheme_struct[head.cnt_of_morpheme];
		if(!morStructTable) fatal_error("mem alloc fail\n");
		fread(morStructTable,sizeof(struct morpheme_struct)*
			head.cnt_of_morpheme,1,phraseAutoMap);

		flechi_map = 
		loadStrTableFromFile(phraseAutoMap,head.offset_of_flechi,
			head.offset_of_canonique - head.offset_of_flechi,
			head.cnt_of_flechi,	flechi_table);
		canoni_map =
		loadStrTableFromFile(phraseAutoMap,head.offset_of_canonique,
			head.offset_of_info - head.offset_of_canonique,
			head.cnt_of_canonique, 
				canoni_table);
		linfos_map =
		loadStrTableFromFile(phraseAutoMap,head.offset_of_info,
				head.offset_of_infoarray - head.offset_of_info,
				head.cnt_of_info, 
				linfos_table);
		int szinfos = head.col_of_infoarray*head.cnt_of_infoarray;
		infoArray_table = new unsigned int [szinfos];
		info_col = head.col_of_infoarray;
		fseek(phraseAutoMap,head.offset_of_infoarray,SEEK_SET);
		if(szinfos)
           if(!fread(infoArray_table,szinfos*4,1,phraseAutoMap))
			freadError("info array load error");
		fseek(phraseAutoMap,head.offset_of_sentence_init,SEEK_SET);
		sentenceInit.total = head.count_of_sentences;
		sentenceInit.readTablesFromFile(phraseAutoMap);
		if(head.cnt_of_variation_ecrit){
		    fseek(phraseAutoMap,head.offset_of_variation_ecrit,SEEK_SET);
            variation_ecriture.tableLoadFromFile(phraseAutoMap,
			     head.cnt_of_variation_ecrit,
			     head.offset_of_org_var - head.offset_of_variation_ecrit);
				
            fseek(phraseAutoMap,head.offset_of_org_var,SEEK_SET);
		    orgConvTable = new struct org_variation_table[head.cnt_of_org_var];
		    if(!fread(orgConvTable,head.cnt_of_org_var*
			   sizeof(struct org_variation_table),1,phraseAutoMap))
			      freadError("load conver map open error");
		}
	}

	unsigned short *
	loadStrTableFromFile(FILE *f,int offset,int iSz,int cnt,unsigned short **&table)
	{
		register unsigned short *wp;
                int readSz= iSz/2;
		unsigned short *map = new unsigned short[readSz];
		if(!map) fatal_error("mem alloc fail\n");
		fseek(f,offset,SEEK_SET);
		if(!u_fread_raw(map,readSz,f)) fatal_error("Read Tokens fail\n");
		table = new unsigned short *[cnt];
		if(!table) fatal_error("token table mem alloc fail\n");
		wp = map;
		unsigned short *curoffset = wp;
		int index  = 0;
		unsigned short *end = map + readSz;
		while(wp < end) {
			if(*wp == 0x0a){
				*wp++ = 0;
				table[index++] = curoffset;
				curoffset = wp;
			} else  if(*wp == 0x0d) {
				*wp++ = 0;
			} else
				wp++;			
		};
		if( index != cnt) fatal_error("illegal table size\n");
		return(map);
	}
	void 
	loadMorphemesStruct(char *fname)
	{
		strcpy(ftemp,pathNameStore);
		strcat(ftemp,"phrase.cod");
		if((phraseAutoMap = fopen(ftemp,"rb")) ==0 ) fopenErrMessage(ftemp);

		read_head();

		fseek(phraseAutoMap,head.offset_morp_struct,SEEK_SET);
		morStructTable = new struct morpheme_struct[head.cnt_of_morpheme];
		if(!morStructTable) fatal_error("mem alloc fail\n");
		if(!fread(morStructTable,sizeof(struct morpheme_struct)*
			head.cnt_of_morpheme,1,phraseAutoMap))
			freadError(ftemp);
		
		loadStrStructFromFile(phraseAutoMap,head.offset_of_flechi,
			head.offset_of_canonique - head.offset_of_flechi,
			flechi);

		loadStrStructFromFile(phraseAutoMap,head.offset_of_canonique,
			head.offset_of_info - head.offset_of_canonique,
				canon);
		loadStrStructFromFile(phraseAutoMap,head.offset_of_info,
				head.offset_of_infoarray - head.offset_of_info,
				info);

		int szinfos = head.col_of_infoarray*head.cnt_of_infoarray;
		infoArray_table = new unsigned int [szinfos];
		info_col = head.col_of_infoarray;
		fseek(phraseAutoMap,head.offset_of_infoarray,SEEK_SET);
		fread(infoArray_table,szinfos*4,1,phraseAutoMap);
	}
	void
	loadStrStructFromFile(FILE *f,int offset,int sz,class arbre_string0 &table)
	{
		register unsigned short *wp;
		unsigned char *map = new unsigned char[sz];
		if(!map) fatal_error("mem alloc fail\n");
		fseek(f,offset,SEEK_SET);
		if(!fread(map,sz,1,f)) fatal_error("Read Tokens fail\n");
		wp = (unsigned short *)map;
		unsigned short *curoffset = wp;
		unsigned short *end = (unsigned short *)(map + sz);
		int cnt = 0;
		while(wp < end) {
			if(*wp == 0x0a){
				*wp++ = 0;
				if(*curoffset == 0) fatal_error("Format Error\n");
				table.put(curoffset);
				cnt ++;
				curoffset = wp;
			} else  if(*wp == 0x0d) {
				*wp++ = 0;
			} else
				wp++;			
		};
		if( table.size() != cnt) fatal_error("illegal table size\n");
		delete map;
	}

	//
	//	convert a eti struct to string
	//
	unsigned short ucharTmp[1024];	// reserve for symbole
	unsigned short *getEtiquetteStr(struct text_eti_fst2 *e)
	{
		unsigned short *wp,*sp;
		int i;
		int index = e->noEtiq & MASK_OF_INDEX;
		int type = e->noEtiq & MASK_OF_TYPE;
		unsigned int *lp;
	    unsigned short sc;
		
		switch(type){
		case TE_NEW_TOKEN: fatal_error("Illegal value exist in the text automaton\n");
		case TE_SEGMENT:
			sp = tokTable[index];
			break;
		case TE_TOKEN_VAR:
			sp = ucharTmp;
			wp = variation_ecriture.getOneItemWithIndex(orgConvTable[index].var);
			while(*wp) *sp++ = *wp++;
			wp = tokTable[orgConvTable[index].org];
			*sp++ = '(';
			while(*wp) *sp++ = *wp++;
			*sp++ = ')';
			*sp = 0;
			sp = ucharTmp;
			break;
		case TE_WORD_MORPHEME:
		case TE_MUL_MORPHEMES:
			wp = tbuff;
			*wp++ = '{';
			sp = flechi_table[morStructTable[index].idx_f];
			while(*sp) *wp++ = *sp++;
			*wp++ = ',';
			sp = canoni_table[morStructTable[index].idx_c];
			while(*sp) *wp++ = *sp++;
			*wp++ = '.';
			lp = infoArray_table + (morStructTable[index].idx_i * info_col);
			for(i = 0; i < info_col * 32;i++){
				if( lp[ i / 32 ] & bitSetL[i%32]) {
					sp = linfos_table[i];
					while(*sp) *wp++ = *sp++;
					*wp++ = '+';
				}
			}
			--wp;
			*wp++ = '}';
			*wp++ = 0;
			sp = tbuff;
			break;
		case TE_SYMBOL_NUM:
		case TE_SYMBOL_GRA:
//			sp = tokTable[index];
            sc = (unsigned short )( index & 0xffff);
            switch(sc){
            case '"':
            case '\\':
               ucharTmp[0] = '\\';
               ucharTmp[1] =sc;
			   ucharTmp[2] = 0;
               break;
            case ' ':
               ucharTmp[0] = '<';
               ucharTmp[1] ='B';
			   ucharTmp[2] ='L';
			   ucharTmp[3] ='>';
			   ucharTmp[4] = 0;
               break;
               
            default:
			ucharTmp[0] =sc;
			ucharTmp[1] = 0;
			}
			sp = ucharTmp;
			break;
		case TE_SYMBOL_CTL:
			sp = ctl_Uchar_name_string[index];
			break;
		default:
			fatal_error("illegal type of morpheme\n");
		}
		return(sp);
	}
		//
	//	handling sentence table map
	//

	void
	saveSentencePosition()
	{
		
		head.offset_of_sentence_init= ftell(phraseAutoMap);
		head.count_of_sentences = sentenceInit.total+sentenceInit.curOffset;		
		sentenceInit.writeTableAtFile(phraseAutoMap);
	}
	//
	//
	//
	int variation_table_add(int org,unsigned short *dest)
	{
		int sz = variation_table.getSize();
        int i;
		struct org_variation_table *w;
		for(i = 0; i < sz;i++) {
			w =  (struct org_variation_table *)variation_table.getAddr(i);
			if(w->org == org) return(i);
		}
		w = (struct org_variation_table *)variation_table.get();
		w->org = org;
		w->var = (int)
          variation_ecriture.put(dest);
		return(sz);
	}


};



//
//
//
class mkTxt2Fst2Kr : public wideCharTable{
	int numberOfPhrase;
	#define MAX_NO_ELEMENT_PHASE	4094
	struct text_etat_fst2 *segOfPhrase[MAX_NO_ELEMENT_PHASE];
	int segOfPhraseIdx;
	Fst2 *loadFst2;
	FILE *fidx;	// index file
	class morpheme_info unePhraseAuto;

public:
	FILE *fout;
#ifdef DDEBUG
	FILE *fdebug;
#endif
	#define SEGMASK	0x80000000
	#define WORDONLY	0x1
	#define MORPHEMEONLY	0x2
	#define BOTHSET			0x3
	unsigned short *blancString;
	int mode;
	arbre_string0 fst2etiStr;

	mkTxt2Fst2Kr()
	{
		fout = 0;
		fidx = 0;
		mode = BOTHSET;
		blancString = 0;
#ifdef DDEBUG
		fdebug = u_fopen("debug.txt",U_WRITE);
#endif
  
    		fst2etiStr.put(u_epsilon_string);
		loadFst2 = 0;
	}
	~mkTxt2Fst2Kr()
	{
		if(blancString) delete blancString;
#ifdef DDEBUG
		if(fdebug){fflush(fdebug);fclose(fdebug);}
#endif
		if(fout){ fflush(fout); 	fclose(fout);}
		if(fidx){ fclose(fidx);}
		if(loadFst2) free_Fst2(loadFst2);
	}

	void modeSet(int i)
	{ mode = i;};
	void pathNameSet(char *f){
	unePhraseAuto.pathNameSet(f);
	}
	
	void getUnePhraseFst2(int demandNumberOfSentence,unsigned short *ctlCharString)
	{
	int idx;
	int offset;
	struct text_eti_fst2 teti;

		
	unePhraseAuto.setCtrlChar(ctlCharString,0);
	
	unePhraseAuto.setBlancSym(defaultSpaceStr);
	numberOfPhrase = 1;
	segOfPhraseIdx = 0;
	segOfPhrase[segOfPhraseIdx++] = unePhraseAuto.newEtat();
	teti.noEtiq = 0;
	teti.offset = 0;
	
	while(fread(&idx,sizeof(int),1,fidx))
	{
		if(idx > unePhraseAuto.tokTableSz) fatal_error("Illegal index of token\n");
		fread(&offset,sizeof(int),1,fidx);
		if(idx == unePhraseAuto.phraseMark){
			if(numberOfPhrase == demandNumberOfSentence){
				outOnePhrase();
				unePhraseAuto.resetMem();
				segOfPhraseIdx = 0;
				segOfPhrase[segOfPhraseIdx++] = unePhraseAuto.newEtat();
				break;
			}
			numberOfPhrase++;
		} else { 
			if(numberOfPhrase == demandNumberOfSentence){
				segOfPhrase[segOfPhraseIdx] = unePhraseAuto.newEtat();
				teti.noEtiq = idx;
				teti.offset = offset;
				unePhraseAuto.insBr(&teti,
					segOfPhrase[segOfPhraseIdx-1],
					segOfPhrase[segOfPhraseIdx]);
				if(segOfPhraseIdx >= MAX_NO_ELEMENT_PHASE){
					error("the %d sentence excede the number of elements",segOfPhraseIdx);
				}
				segOfPhraseIdx++;
			}
		}
	}
	}
	void outOnePhrase()
	{
	int i;
	int segNum;
	struct text_etat_fst2 *t;

	struct text_transition_fst2 *strans;
	switch(mode){
	case WORDONLY:
		break;
	case MORPHEMEONLY:
	case BOTHSET:
		for( i = 0; i < segOfPhraseIdx - 1;i++){
			segNum = segOfPhrase[i]->trans->eti.noEtiq;
			if(unePhraseAuto.mophTable[segNum][0])
			{// find in dictionnaire
				t = unePhraseAuto.constructAutoFromStr(
					unePhraseAuto.mophTable[segNum],
					segOfPhrase[i]->trans->eti.offset
				);
	
				if(mode == MORPHEMEONLY){
					segOfPhrase[i]->trans->arr = t;
				} else {
					strans = t->trans;
					while(strans){
						unePhraseAuto.insBr(&strans->eti,segOfPhrase[i],strans->arr);
						strans = strans->next;
					}
				}
//unePhraseAuto.prAuto(fdebug);
				unePhraseAuto.setTerminalBranch(1,t,segOfPhrase[i+1]);
			}
		}
		break;
	default:
		error("error in programm!!!\n");
	}
	u_fprintf(fout,"0000000001\n");
//unePhraseAuto.prAuto(fdebug);
	unePhraseAuto.elimineEpsilonTrans(2,segOfPhrase[0]);
	unePhraseAuto.elimineSameEtiTrans(3,segOfPhrase[0]);
//unePhraseAuto.prAuto(fdebug);
	unePhraseAuto.setTerminalBranch(4,segOfPhrase[0],unePhraseAuto.newEtat());
	unePhraseAuto.setIndexNumber(segOfPhrase[0]);
	
	u_fprintf(fout,"-1 %d\n",numberOfPhrase,numberOfPhrase);

	convStrAutoToFst2(segOfPhrase[0],1);
	u_fprintf(fout,"f \n");
	unePhraseAuto.prEtiquette(fout);
	u_fprintf(fout,"f \n");
	}
//
//	save a sentence to fst format
//
	void convStrAutoToFst2(struct text_etat_fst2 *node,int flag)
	{
	int tcnt = 0;
	if(node->index & 0x80000000) return;
	if(!node->trans){
		if(flag)
			u_fprintf(fout,"t \n");
		else
			fwrite(&tcnt,sizeof(int),1,fout);
		node->index |= 0x80000000;
	} else {
		if(flag){
			struct text_transition_fst2 *s=node->trans;
			u_fprintf(fout,": ");
			s = node->trans;
			while(s){
				u_fprintf(fout,"%d %d ",s->eti.noEtiq,
					s->arr->index & 0x7fffffff);
				s = s->next;
			};
			s = node->trans;
			u_fprintf(fout,"\n");
			node->index |= 0x80000000;

			while(s){
				convStrAutoToFst2(s->arr,flag);
				s = s->next;
			}
		} else {
			struct text_transition_fst2 *s=node->trans;
			s = node->trans;
			while(s){tcnt++;s = s->next;};
			s = node->trans;
			fwrite(&tcnt,sizeof(int),1,fout);
			s = node->trans;
			while(s){
				fwrite(&s->eti,sizeof(text_eti_fst2),1,fout);
				fwrite(&s->arr->index,1,sizeof(int),fout);
				s = s->next;
			};
			node->index |= 0x80000000;
			s = node->trans;
			while(s){
				convStrAutoToFst2(s->arr,flag);
				s = s->next;
			}

		}
	}
}
//
//	get a sentence from coded file text2.cod
//
	void getUnePhraseFst3(char *f,int lineNumber)
	{
	    char *ft = unePhraseAuto.ftemp;
		strcpy(ft,unePhraseAuto.pathNameStore);
		strcat(ft,"sentence.fst2");
		if((fout = u_fopen(ft,U_WRITE)) == 0) fopenErrMessage(ft);

		unePhraseAuto.loadTokensMap(f);
		unePhraseAuto.loadMorphemes(f);
		if(lineNumber < 0){
			for( int i = 0;
			i < unePhraseAuto.head.count_of_sentences;i++)
				extractUnePhrase(i+1);

		} else {
			extractUnePhrase(lineNumber);
		}
	
	}
	//
	// extract une phrase sous forme fst2 de code file
	// 
	void extractUnePhrase(int lineNum)
	{
		int off_automate =unePhraseAuto.sentenceInit.tableLoaded[lineNum-1];
		int cnt_etat;
		int tran_cnt;
		unsigned short tt[2];
		FILE *readFile = unePhraseAuto.phraseAutoMap;
		
		fseek(readFile,off_automate,SEEK_SET);
		int offset = off_automate;
		if(!fread(&cnt_etat,4,1,readFile))
			fatal_error("read error\n");
		offset += 4;
		int i,j;
		int *tab = new int[cnt_etat];
		for(i = 0; i < cnt_etat;i++)
		{
			tab[i] = offset;
			if(!fread(&tran_cnt,4,1,readFile))
				fatal_error("read error\n");
			offset += 4 + tran_cnt * 12;
			fseek(readFile,offset,SEEK_SET);
		}
		unsigned char *autoMap = new unsigned char [offset - off_automate];
		unsigned int *wp;
		fseek(readFile,off_automate,SEEK_SET);
		fread(autoMap,offset - off_automate,1,readFile);
		u_fprintf(fout,"0000000001\n");
		u_fprintf(fout,"-1 ");
		wp = (unsigned int *)autoMap;
		if((int)*wp++ != cnt_etat) fatal_error("mem error\n");

    	// get orgin sentence 
        for(i = 0; i < cnt_etat;i++)
		{
			tran_cnt = *wp++;
			if(!tran_cnt)	break;
			
			switch(*wp&MASK_OF_TYPE){
			case TT_CONTROL_:
			case TE_SYMBOL_NUM:
			case TE_SYMBOL_GRA:
			   tt[0] = *wp & 0xffff; 
			   u_fputc_UTF16LE_raw(tt[0],fout);
			   break;			
			case TE_SEGMENT:
				u_fprintf(fout,"%S",unePhraseAuto.tokTable[*wp&MASK_OF_INDEX]);
			   break;
			case TE_NEW_TOKEN:
			case TE_MUL_MORPHEMES:
			case TE_WORD_MORPHEME:
			case TE_TOKEN_VAR:
			   break;
			}
			wp += 3*tran_cnt;
		}		
		u_fprintf(fout,"\n");
    	int k;
		wp = (unsigned int *)autoMap;
		wp++;
		for(i = 0; i < cnt_etat;i++)
		{
			tran_cnt = *wp++;
			if(!tran_cnt){
				u_fprintf(fout,"t \n");
				continue;
			}
			u_fprintf(fout,": ");
			for(j = 0; j < tran_cnt;j++){
//u_fprintf(unePhraseAuto.debugf,"%d %d %d\n",wp[0],wp[1],wp[2]);
				if(((*wp&MASK_OF_TYPE) ==TE_SEGMENT)  && 
                    ((int)(*wp&MASK_OF_INDEX)==unePhraseAuto.phraseMark))
                    u_fprintf(fout,"0 ");
				else 
                    u_fprintf(fout,"%d ",getNewEtiq((struct text_eti_fst2 *)wp));
				if( (k = find_index_array(tab,cnt_etat,wp[2])) < 0) 
					fatal_error("illegal value at array index\n");
				u_fprintf(fout,"%d ",k);
				wp += 3;
			}
			u_fprintf(fout,"\n");
		}
		delete tab;
		delete autoMap;
		u_fprintf(fout,"f \n");
		prStrList(fout);
		u_fprintf(fout,"f \n");
	}

	int getNewEtiq(struct text_eti_fst2 *et)
	{
		unsigned short *wp =
			unePhraseAuto.getEtiquetteStr(et);
 
		int no = fst2etiStr.put(wp);
		return(no);
	}
	void prStrList(FILE *f)
	{
		int sz = fst2etiStr.size();
		int i;
		unsigned short **tab = fst2etiStr.make_strPtr_table();
		for( i = 0; i < sz;i++){
			tab[i][0] = '%';
			u_fprintf(f,"%S\n",tab[i]);

		}
		fst2etiStr.release_value();
	}
//
//	make sentence automate from idx file and tokens and sous-tokens
//	the idx file presente text file to index of tokens
//  sous tokens file presente a tokens to automata of morphems 
//
	void convertIdxFileToFst2(char *f,unsigned short *ctrString)
	{
		int idx;
		char *ft= unePhraseAuto.ftemp;
		struct text_eti_fst2 teti;

		strcpy(ft,unePhraseAuto.pathNameStore);
		strcat(ft,"text.cod");
		if((fidx = fopen(ft,"rb")) ==0 ) fopenErrMessage(ft);
		unePhraseAuto.loadTokensMap(f);
		if(unePhraseAuto.phraseMark < 0)
		fatal_error(" Sentence mark not exist!\n");
		unePhraseAuto.loadSousTokensMap(f);
		fout = unePhraseAuto.openForWrite(f);

	// space for total count
	    unichar defaultSpaceDef [] = {(unichar)'<',(unichar)'B',(unichar)'L'
                        ,(unichar)'>',(unichar)'\0'};
		unePhraseAuto.setCtrlChar(ctrString,unePhraseAuto.tokTableSz +1);
		unePhraseAuto.setBlancSym((blancString) ? blancString: defaultSpaceDef);
		numberOfPhrase = 0;
		segOfPhraseIdx = 0;
		teti.noEtiq  = 0;
		teti.offset = 0;

		segOfPhrase[segOfPhraseIdx++] = unePhraseAuto.newEtat();
		int textIndexCount = 0;
		while(fread(&idx,sizeof(int),1,fidx))
		{
			if(idx > unePhraseAuto.tokTableSz) fatal_error("Illegal index of token\n");
			if(idx == unePhraseAuto.phraseMark){
			    segOfPhrase[segOfPhraseIdx] = unePhraseAuto.newEtat();
				teti.noEtiq = idx | TE_SEGMENT;
				teti.offset = textIndexCount;
				unePhraseAuto.insBr(&teti,segOfPhrase[segOfPhraseIdx-1],
					segOfPhrase[segOfPhraseIdx]);
				numberOfPhrase++;

				savePhrase();

				if(!(numberOfPhrase % 100) ) 
					u_printf("\r %dth sentence handling",numberOfPhrase);
				unePhraseAuto.resetMem();
				segOfPhraseIdx = 0;
				segOfPhrase[segOfPhraseIdx++] = unePhraseAuto.newEtat();
			} else { 
				segOfPhrase[segOfPhraseIdx] = unePhraseAuto.newEtat();
				teti.noEtiq = idx | TE_SEGMENT;
				teti.offset = textIndexCount;

				unePhraseAuto.insBr(&teti,segOfPhrase[segOfPhraseIdx-1],
					segOfPhrase[segOfPhraseIdx]);
				if(segOfPhraseIdx > MAX_NO_ELEMENT_PHASE){
					fatal_error("the %d sentence excede the number of elements\n",numberOfPhrase);
				}
				segOfPhraseIdx++;
			}
			textIndexCount++;
		}
		if(segOfPhraseIdx){
			numberOfPhrase++;
			savePhrase();
			if(!(numberOfPhrase % 100) ) 
				error("\r %08dth sentence handling",numberOfPhrase);

			unePhraseAuto.resetMem();
			segOfPhraseIdx = 0;
			segOfPhrase[segOfPhraseIdx++] = unePhraseAuto.newEtat();
		}
		unePhraseAuto.saveMorphems();
		unePhraseAuto.saveSentencePosition();
		
		unePhraseAuto.write_head();


	}
//
//	add morphemes to the automate by only text tokens
//
	unsigned short tempArr[1024];
	void savePhrase()
	{
		int i,j;
		struct text_etat_fst2 *t;
		struct text_transition_fst2 *strans;
		int segOrg;
		unsigned short *tchar,*wp;
#ifdef PRDEBUG
unePhraseAuto.prAuto(stderr,0);
#endif // PRDEBUG
		for( i = 0; i < segOfPhraseIdx - 1;i++){
		    //
			//	first transition
			//
			segOrg = segOfPhrase[i]->trans->eti.noEtiq & MASK_OF_INDEX;
			tchar = unePhraseAuto.mophTable[segOrg];
			if(*tchar){
				wp = tempArr;
				if(*tchar == '\t'){ // mix with ideogramms
					tchar++;
					while(*tchar != '\t'){
						if(!*tchar) fatal_error("Illegal morphemes line\n");
						*wp++ = *tchar++;
					}
					*wp++ = 0;
					if(!tempArr[0]) fatal_error("illegal variation form\n");
					j = unePhraseAuto.variation_table_add(segOrg,tempArr);

					segOfPhrase[i]->trans->eti.noEtiq = j | TE_TOKEN_VAR;
					tchar++;
				}
				t = unePhraseAuto.constructAutoFromStr(
				tchar,
				segOfPhrase[i]->trans->eti.offset);
				// connect terminal to next state
				strans = t->trans;
				while(strans){
					unePhraseAuto.insBr(&strans->eti,
					segOfPhrase[i],
					strans->arr);
					strans = strans->next;
				}
				unePhraseAuto.setTerminalBranch(1,t,segOfPhrase[i]->trans->arr);
				
			} else {
				tchar = unePhraseAuto.tokTable[segOrg];
				if(check_range_character(*tchar) & 0x4){	// symbole
					// symbole, number, ctl
					if((*tchar == '<') && *(tchar+1) != 0){
						int ll = int (unePhraseAuto.checkName(tchar)& 0xffff);
						if( ll == -1)
							fatal_error("illegal control character\n");
						segOfPhrase[i]->trans->eti.noEtiq = ll | TE_SYMBOL_CTL;
					} else { // control character
						segOfPhrase[i]->trans->eti.noEtiq = *tchar | TE_SYMBOL_GRA;
					}
				}
			}
		}
#ifdef PRDEBUG		
unePhraseAuto.prAuto(stderr,0);
#endif // PRDEBUG
		unePhraseAuto.elimineEpsilonTrans(2,segOfPhrase[0]);
#ifdef PRDEBUG
unePhraseAuto.prAuto(stderr,2);
#endif // PRDEBUG

		unePhraseAuto.elimineSameEtiTrans(3,segOfPhrase[0]);

//		unePhraseAuto.setTerminalBranch(4,segOfPhrase[0],unePhraseAuto.newEtat());

		unePhraseAuto.setIndexNumber(segOfPhrase[0]);

		unePhraseAuto.sentenceInit.insertAtTable(
          unePhraseAuto.outLesEtatToFile(fout));
	}
	void addition(char *fstna,char *infile)
	{
		char pathName[2048];
		remove_extension(infile,pathName);
		
		
		loadFst2 = load_fst2(fstna,1);
		
		unePhraseAuto.loadTokensMap(infile);
		unePhraseAuto.loadMorphemes(infile);

		changeFst2TextAuto(0);
	}
	void modification(char *fstna,char *infile)
	{
	
		char pathName[2048];
		remove_extension(infile,pathName);
		
		
		loadFst2 = load_fst2(fstna,1);

		unePhraseAuto.loadTokensMap(infile);
		unePhraseAuto.loadMorphemes(infile);
		changeFst2TextAuto(1);
	}
	void changeFst2TextAuto(int flag)
	{
#ifdef NOTFini
		int lineNum = utoi(*loadFst2->graph_names);
		
		if( (lineNum < 1 ) || (lineNum > unePhraseAuto.head.count_of_sentences)){
			error("%s ",getUtoChar(loadFst2->graph_names));
			fatal_error("Illegal sentence number\n");
		}
		int off_automate =unePhraseAuto.sentenceInit.tableLoaded[lineNum-1];
		int cnt_etat;
		int tran_cnt;
		FILE *readFile = unePhraseAuto.phraseAutoMap;
		
		fseek(readFile,off_automate,SEEK_SET);
		int offset = off_automate;
		if(!fread(&cnt_etat,4,1,readFile))
			fatal_error("read error\n");
		offset += 4;
		if(cnt_etat != loadFst2->number_of_states)
		{
			error("%d < %d\n", cnt_etat,loadFst2->number_of_states);
			fatal_error("Count of state not match\n");
		}
		int i,j,k;
		int cnt_trans;
		Fst2Transition *t;
		unsigned int *wp;
		unsigned int *mapTrans  = new unsigned int [1024 * 3];
		if(!maptrans) fatal_error("Mem alloc fail\n");
		for(i = 0; i < cnt_etat;i++)
		{
			t = loadFst2->states[i]->transitions;
			fread(&k,4,1,readFile);
			fread(mapTrans,4*3,k,readFile);
			wp = mapTrans;
			while(t){
				etiPtr = loadFst2->tag_number[t->tag_number];
				switch(etiPtr->input[1]){
				case '{':
				case '"':
					if(etiPtr->input[1] == 0) break;
					for( j = 0; j < k;j++){

					}
					break;
				default:
					break;
				}
			}
		}
#endif // DDDDD
	}

};
#endif 
