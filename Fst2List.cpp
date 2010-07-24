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

#include <stdlib.h>


#include <locale.h>
#include "Unicode.h"
#include "Fst2.h"
#include "AbstractFst2Load.h"
#include "Alphabet.h"
#include "Copyright.h"
#include "File.h"
#include "Error.h"
#include "Transitions.h"



const char* usage_Fst2List =
"Usage:\n" \
"Fst2List [-o outFile][-p s/f/d][-[a/t] s/m] [-m] [-f s/a][-s[0s] \"Str\"] [-r[s/l] \"Str\"] [-l line#] [-i subname]* [-c SS=0xxxx]* fname\n"\
" fname : name of the input file name with extension \".fst2\"\r\n"\
" -o outFile : if this option not exist, save paths at \"file\"lst.txt\r\n"\
" -[a/t] s/m : mode de automata or transducteur, s=single initial, m = multi-inital\r\n"\
"              by default \"-a s\"\r\n"\
" -l line#  :  max number of line to save[decimal].\r\n"\
" -i sname : stop call of the exploitation at this sub-graphe \"sname\"\r\n"\
" -p s/f/d : mode de extrait word, s=each sub graphs, f=full path(default),\r\n"\
"            d= debugging. default is 'f'\r\n"\
" -c SS=0xXXXX: change the symbol string between symbols < and >,\"<SS>\" \r\n"\
"                to a unicode character(0xXXXX)\r\n"\
" -s \"L[,R]\" : use two strings L, R as the separator each item\r\n"\
"                   default null\r\n"\
" -s0 \"Str\" : if transductor mode,set \"str\" as the separator between input and out\r\n"\
"                   default null\r\n"\
" -f  a/s :  if the mode is transductor,the format of output line i0i1SOS1(:s) or i0S0i1S1(:a),i0,i1: input, S0,S1:out\r\n"\
"       default value is \'s\'\r\n"\
" -ss \"stop\" : set \"str\" as the mark of stop exploitation at \"<stop>\" \r\n"\
"                    default null\r\n"\
" -m  : mode special for description with alphabet\r\n"\
" -v : verbose mode  default null\r\n"\
" -r[s/l/x] \"L[,R]\"  : present recusive path(c0|...|cn) by Lc0|..|cnR : default null\r\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Fst2List);
}


static char *getUtoChar(char charBuffOut[],unichar *s)
{
    int i;
    for(i = 0; (i < 1024 ) && s[i] ;i++)
     charBuffOut[i] = (char)s[i];
    charBuffOut[i] = 0;
    return(charBuffOut);
}
enum ModeOut {PR_SEPARATION,PR_TOGETHER};	// make word and output separe ou together
enum printOutType {GRAPH,FULL,DEBUG};
enum initialType { SINGLE,MULTI};
enum presentCycleValue {SYMBOL,LABEL,STOP};
enum autoType {AUTOMODE,TRANMODE};

static unichar *uascToNum(unichar *uasc,int *val);
#define FILE_PATH_MARK	0x10000000
#define LOOP_PATH_MARK	0x20000000
#define STOP_PATH_MARK	0x40000000
#define PATHID_MASK		0x1FFFFFFF
#define SUB_ID_MASK		0x0fffffff
#define CTL_MASK		0xe0000000
#define LOOP_NODE_MARK	0x80
#define DIS_LINE_LIMIT_MAX	4096




#define MAX_CHANGE_SYMBOL_SIZE 32

static int changeStrToVal(int &changeStrToIdx,unichar changeStrTo[][MAX_CHANGE_SYMBOL_SIZE],char *src)
{
	char *wp = src;

	int ptLoc = 0;
	int i;

	i = 1;
	changeStrTo[changeStrToIdx][i++] = (unichar)'<';
	for(wp = src; i <MAX_CHANGE_SYMBOL_SIZE && (*wp) ;i++){
		if(*wp == (unichar)'=') break;
		changeStrTo[changeStrToIdx][i] = (unsigned short)*wp++;
	}
	if(*wp != (unichar)'=') return(1);
	if(i > (MAX_CHANGE_SYMBOL_SIZE - 2)) {
		u_printf("the name of the variable too long %s",src);
		return 1;
	}
	changeStrTo[changeStrToIdx][i++] = (unichar)'>';
	changeStrTo[changeStrToIdx][i++] = (unichar)'\0';
	ptLoc = i;
	if(!*wp) {
      usage();
      return 1;
   }
	for(wp++;i <MAX_CHANGE_SYMBOL_SIZE && (*wp);i++)
		changeStrTo[changeStrToIdx][i] = (unsigned short)*wp++;
	if(*wp != (unichar)'\0') return(1);
	changeStrTo[changeStrToIdx][i++] = (unichar)'\0';
	uascToNum(&changeStrTo[changeStrToIdx][ptLoc],&i);
	changeStrTo[changeStrToIdx][0] = (unsigned short)i;

	{
		char charBuffOut[1024];
		u_printf("Change symbol %s --> %x\n",
			getUtoChar(charBuffOut,&changeStrTo[changeStrToIdx][1]),
			changeStrTo[changeStrToIdx][0]);
	}
	changeStrToIdx++;
	return(0);
}
static unichar u_null_string[]= {(unichar)'\0',(unichar)'\0'};
static unichar u_epsilon_string[] = {(unichar)'<',(unichar)'E'
,(unichar)'>',(unichar)'\0'};

static const char *StrMemLack = "allocation of memory for cycle data is fail";

struct pathAndEti {
    int autoNo;
	int etatNo;
	int eti;
};


//
//
//

class CFstApp {


public:

	struct fst2 *a;
	struct FST2_free_info fst2_free;
	U_FILE* foutput;
	ModeOut prMode;
	autoType automateMode;
	int listOut;
	int verboseMode;
//	int control_char; // control the output for control_chars <>
#define	PATH_QUEUE_MAX	1024
	struct pathAndEti pathEtiQ[PATH_QUEUE_MAX];
	int pathEtiQidx;
#define ETIQ_MAX		1024

	//
	// print out all path in the .fst2
	//	  first : check and mark all close paths in each sous-automate
	//			  print out to the file [filename]LstAuto.txt
	//	 seconde: print out all open paths
	//			  at [filename]L.txt
	//
	void loadGraph(int &changeStrToIdx,unichar changeStrTo[][MAX_CHANGE_SYMBOL_SIZE],char *fname);
	int exploirerSubAuto(int startSubAutoNum);
	int getWordsFromGraph(int &changeStrToIdx,unichar changeStrTo[][MAX_CHANGE_SYMBOL_SIZE],char *fst2_file_name);
		int findCycleSubGraph(int autoNo,int autodep,int testEtat,int depthState);
			int outWordsOfGraph(int depth);


	// the stack which keep the path and called sous-graphe
	struct stackAuto {
		int aId;
		int next;
	} CautoQueue[2048];
	int CautoDepth;

	void CqueuePathPr(U_FILE *f);

	int *ignoreTable;
	int *numOfIgnore;


	int outLineLimit;
	int numberOfOutLine;
	int count_in_line; // number of out per line


	int totalPath;
	int totalLoop;
	int stopPath;
	int errPath;

	presentCycleValue recursiveMode;
	printOutType display_control;
	initialType traitAuto;	// single or multi initial state
	int niveau_traite_mot;
	int depthDebug;

	unichar *saveSep,*sepL;	// for input and output
	unichar *sepR;	// for input and output
	unichar *sep1;	// for each input/output
	unichar *stopSignal;
	unichar *saveEntre,*entreGO,*entreGF;
	Encoding encoding_output;
	int bom_output;
	int mask_encoding_compatibility_input;
	char ofdirName[1024];
	char ofExt[16];
	char ofnameOnly[512];
	char defaultIgnoreName[512]; // input filename
	void fileNameSet(char *ifn,char *ofn)
	{
	    char tmp[512];
		remove_path(ifn,tmp);
		remove_extension(tmp,defaultIgnoreName);
		if(!ofn){
		    get_path(ifn,ofdirName);
		    remove_path(ifn,tmp);
		    remove_extension(tmp,ofnameOnly);
		    strcpy(ofExt,".txt");
		} else {
		    get_path(ofn,ofdirName);
  		    remove_path(ofn,tmp);
		    remove_extension(tmp,ofnameOnly);
		    get_extension(tmp,ofExt);
		}
		if(ofnameOnly[0]== 0) fatal_error("ofile name not correct");
	}
    void fileEncodingSet(Encoding encoding_output_set,int bom_output_set,int mask_encoding_compatibility_input_set)
	{
        encoding_output = encoding_output_set;
        bom_output = bom_output_set;
        mask_encoding_compatibility_input = mask_encoding_compatibility_input_set;
	}
	void makeOfileName(char *des,const char *fn,const char *ext){
	   strcpy(des,ofdirName);
	   if(fn) strcat(des,fn);
	   else strcat(des,ofnameOnly);
	   if(ext) strcat(des,ext);
	   else strcat(des,ofExt);
	}

	CFstApp():
		a (0),
        fst2_free(FST2_free_info_init),
		foutput(0),
		prMode(PR_SEPARATION),
		automateMode(AUTOMODE),
		listOut(0),
		verboseMode(0),
		pathEtiQidx(0),

        CautoDepth(0),

		ignoreTable(0),
		numOfIgnore(0),

		outLineLimit(0x10000000),
		numberOfOutLine(0),
        count_in_line(0),

        totalPath(0),
        totalLoop(0),
        stopPath(0),
		errPath(0),

		recursiveMode(STOP),
		display_control(FULL),
		traitAuto(SINGLE),
		niveau_traite_mot(1),  // unit of box is word
        depthDebug(0),

		saveSep (u_null_string),
		sepL (u_null_string),
		sepR (u_null_string),
		sep1 (u_null_string),
		stopSignal (u_null_string),
		saveEntre(u_null_string),
		entreGO(u_null_string),
		entreGF(u_null_string),

        encoding_output(DEFAULT_ENCODING_OUTPUT),
        bom_output(DEFAULT_BOM_OUTPUT),
        mask_encoding_compatibility_input(DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT),

        autoStackMap(NULL),
        mapOfCallHead(NULL),
        mapOfCallTail(NULL),

        cycInfos(NULL),

		headCyc(0),
		cyclePathCnt(0),
		headCycNodes(0),
		cycNodeCnt(0),

        ePtrCnt(0),
        tPtrCnt(0),
        EOutCnt(0),
        SOutCnt(0),

        arretSubListIdx(0)
		{
		initCallIdMap();
	};
	~CFstApp(){
	   arretExpoDel();
		cleanCyclePath();
		free_abstract_Fst2(a,&fst2_free);
      if(saveSep != u_null_string) delete saveSep;
      if(sep1 != u_null_string) delete sep1;
		if(stopSignal != u_null_string) delete stopSignal;
		if(saveEntre != u_null_string) delete saveEntre;
		if(ignoreTable) delete [] ignoreTable;
        if(numOfIgnore) delete [] numOfIgnore;
		finiCallIdMap();
	};
	void CleanPathCounter()
	{
	 totalPath = totalLoop = errPath = stopPath = 0;
	}
	    //
    //    identify sub calls with transition and call id
    //

    struct callStackMapSt {
     Transition* tran;
     int autoId;
    } *autoStackMap;
    struct callIdMap {
        int cnt;
        Transition** list;
        struct callIdMap *next;
    } *mapOfCallHead,*mapOfCallTail;


    int callIdentifyId(struct callStackMapSt *cmap,int count)
    {
        int id = 0;
        int i;
        struct callIdMap *fPtr;
        fPtr = mapOfCallHead;
        while(fPtr){
            if(fPtr->cnt == count){
                for(i = 0; i < count;i++)
                    if(fPtr->list[i] != cmap[i].tran)
                        break;
                if(i == count) {
                    return id;
                }
            }
            id++;
            fPtr = fPtr->next;
        }
        fPtr = new struct callIdMap;
        fPtr->cnt = count;
        fPtr->next = 0;
        fPtr->list = new Transition*[count];
        for(i = 0; i < count;i++) fPtr->list[i] = cmap[i].tran;

        if(mapOfCallTail){
                mapOfCallTail->next = fPtr;
                mapOfCallTail = fPtr;
        } else {
                mapOfCallTail = mapOfCallHead = fPtr;
        }
        return id;
    }
    void initCallIdMap()
    {
        autoStackMap = new struct callStackMapSt[1024];
        mapOfCallHead = mapOfCallTail = 0;
    }
    void finiCallIdMap()
    {
        delete [] autoStackMap;
        while(mapOfCallHead)
        {
           mapOfCallTail = mapOfCallHead;
            mapOfCallHead = mapOfCallHead->next;
            delete [] mapOfCallTail->list;
            delete mapOfCallTail;
        }
    }
    
	//
	// for level
	//
    //
    //	save all cycle identify
    //
    struct cyclePathMark {
    	int index;		// nomber of identify
    	struct pathAndEti  *pathEtiQueue;
    	int pathCnt;
    	int flag;
    	struct cyclePathMark *next;
    };
    //
    //	save all node which has the cycle path
    //	the path from initial to the node is used to identify the node
    //	in sous graphe
    //
    struct linkCycle {
    	struct cyclePathMark *cyc;
    	struct linkCycle *next;
    } *cycInfos;

    struct cycleNodeId {
    	int index;
   	    int autoNo;
   	    int etatNo;
   	    int eti;
    	struct linkCycle *cycInfos;
    	int flag;
    	struct cycleNodeId *next;
    };
    struct cyclePathMark *headCyc;
	int cyclePathCnt;
	struct cycleNodeId *headCycNodes;
	int cycNodeCnt;

	unichar *getLabelNumber(unichar*aa,int numOfPath,int &flag,int curidx,int setflag)
	{
		struct cycleNodeId *cnode = headCycNodes;
		int i;
		int searchEtat = pathEtiQ[curidx].etatNo & PATHID_MASK;
		int searchEtatAuto = pathEtiQ[curidx].autoNo;
		int searchEti = pathEtiQ[curidx].eti;
		while(cnode){
		    if(
              (searchEtatAuto ==  cnode->autoNo) &&
              (searchEtat ==  cnode->etatNo) &&
              (searchEti ==  cnode->eti)
             ) break;
			cnode = cnode->next;
		}
		if(!cnode){
		   error("%d/%d stack\n",numOfPath,curidx);
		   for(i = 0;i<pathEtiQidx;i++)
		   error("%d : (%08x:%08x) : %08x\n",
               i,pathEtiQ[i].autoNo,pathEtiQ[i].etatNo,pathEtiQ[i].eti);

		   CqueuePathPr(U_STDERR);
           fatal_error("eu~ak\n");
        }
        if(setflag){
		flag = cnode->flag;
		if(!cnode->flag){
			cnode->flag = 1;
		}
		}

		{
			//unichar aa[64];
			u_sprintf(aa,"Loc%d",cnode->index);
			return((unichar *)aa);
		}
	}

	void setIdentifyValue(int offset,int cntNode)
	{
		struct cycleNodeId **cnode = &headCycNodes;

 	    int cycEtatNo = pathEtiQ[cntNode-1].etatNo & PATHID_MASK;
    	int cycEtatAutoNo = pathEtiQ[cntNode-1].autoNo;
	    int cycEtatEti = pathEtiQ[cntNode-1].eti;
		while(*cnode){
			if(((*cnode)->autoNo == cycEtatAutoNo) &&
               ((*cnode)->etatNo == cycEtatNo) &&
               ((*cnode)->eti == cycEtatEti)
               ){
				break;
			}
			cnode = &((*cnode)->next);
		}
		if(!*cnode){
			*cnode = new struct cycleNodeId;
			if(!(*cnode)) fatal_error(StrMemLack);
			(*cnode)->next = 0;
			(*cnode)->cycInfos = 0;
			(*cnode)->index = cycNodeCnt++;
			(*cnode)->autoNo = cycEtatAutoNo;
			(*cnode)->etatNo = cycEtatNo;
			(*cnode)->eti = cycEtatEti;
			(*cnode)->flag = 0;
		}

		struct cyclePathMark *pCyc = getLoopId(offset);
		struct linkCycle **alc = &((*cnode)->cycInfos);
		while(*alc){
			if(pCyc->index == (*alc)->cyc->index) return;
			if((*alc)->cyc->index < pCyc->index) break;
			alc = &((*alc)->next);
		}
		*alc = new struct linkCycle;
		if(!(*alc)) fatal_error(StrMemLack);
		(*alc)->next = 0;
		(*alc)->cyc = pCyc;

	}
	struct cyclePathMark *getLoopId(int offset)
	{
		struct cyclePathMark **h = &headCyc;
		int numOfPath;
		int i,j;
		offset++;
		numOfPath = pathEtiQidx - offset;
		while(*h){
			if((*h)->pathCnt == numOfPath){
				for(i = 0; i < numOfPath;i++){
					if((pathEtiQ[offset].autoNo
                         == (*h)->pathEtiQueue[i].autoNo) &&
                         ((pathEtiQ[offset].etatNo & PATHID_MASK)
                         == (*h)->pathEtiQueue[i].etatNo) &&
						(pathEtiQ[offset].eti
                        == (*h)->pathEtiQueue[i].eti))
						break;
				}
				if(i != numOfPath){	// find first the position in cycle ring
					for(j = 0; j < numOfPath;j++){
						if(((pathEtiQ[offset+j].etatNo & PATHID_MASK)
                              != (*h)->pathEtiQueue[i].etatNo) ||
						   (pathEtiQ[offset+j].eti
                           != (*h)->pathEtiQueue[i].eti) ||
                           (pathEtiQ[offset+j].autoNo
                           != (*h)->pathEtiQueue[i].autoNo))
								break;
						if(++i >= numOfPath) i = 0;
					}
					if(j == numOfPath)
						return((*h));
				}
			}
			if((*h)->pathCnt < numOfPath) break;
			h = &((*h)->next);
		}
		struct cyclePathMark *tmp = new struct cyclePathMark;
		tmp->flag =0;
		tmp->index = 0;
		tmp->pathCnt =0;
		tmp->next = *h;
		*h = tmp;
		if(!(*h)) fatal_error(StrMemLack);
		(*h)->pathEtiQueue = new struct pathAndEti [numOfPath];
		if(!((*h)->pathEtiQueue)) fatal_error(StrMemLack);
		for( i = 0; i < numOfPath;i++){
			(*h)->pathEtiQueue[i].etatNo = pathEtiQ[i+offset].etatNo & PATHID_MASK;
			(*h)->pathEtiQueue[i].eti = pathEtiQ[i+offset].eti;
			(*h)->pathEtiQueue[i].autoNo = pathEtiQ[i+offset].autoNo;

		}
		(*h)->pathCnt = numOfPath;
		(*h)->index = cyclePathCnt++;

		return((*h));
	}
	void cleanCyclePath()
	{
		struct cycleNodeId *cnode = headCycNodes;
		struct cycleNodeId *tnode;
		struct linkCycle *inf,*tnf;
		struct cyclePathMark *tc,*cp;
		int i;
      while(cnode){
			tnode = cnode->next;
			inf =  cnode->cycInfos;
			while(inf){
				tnf = inf->next;
				delete inf;
				inf = tnf;
			}
			delete cnode;
			cnode = tnode;
		}
		headCycNodes = 0;
		cycNodeCnt = 0;
		cp = headCyc;
		while(cp){
			tc = cp->next;
			delete cp->pathEtiQueue;
			delete cp;
			cp = tc;
		}
		headCyc = 0;
		cyclePathCnt = 0;
      if (a!=NULL) {
		   for (i = 0; i < a->number_of_states;i++){
			   a->states[i]->control &=0x7f;
		   }
      }
	}
	void prCycleNode()
	{
  		struct cycleNodeId *cnode = headCycNodes;
  		u_fprintf(foutput,"cycle Nodes %d\n",cyclePathCnt);
  		while(cnode){
            u_fprintf(foutput,"%d (%d:%d)\n"
            ,cnode->index,cnode->autoNo,cnode->etatNo);
  		    cnode = cnode->next;
        }
	}
    int WasCycleNode(int cauto,int cetat)
    {
  		struct cycleNodeId **cnode = &headCycNodes;

		while(*cnode){
			if(((*cnode)->autoNo == cauto) &&
               ((*cnode)->etatNo == cetat) ){
				return 1;
			}
			cnode = &((*cnode)->next);
		}
		return 0;
    }
	int IsCyclePath(int depth)
    {
    	int scanner;
    	int curId = pathEtiQ[pathEtiQidx-1].etatNo & PATHID_MASK;
    	int curAutoId = pathEtiQ[pathEtiQidx-1].autoNo;

    	for(scanner = 0; scanner < pathEtiQidx-1;scanner++){
    		if(((pathEtiQ[scanner].etatNo & PATHID_MASK) == curId ) &&
                (pathEtiQ[scanner].autoNo == curAutoId )) { // find recusive path
    			switch(recursiveMode){
                case LABEL:
                    if(listOut){
    					pathEtiQ[pathEtiQidx-1].etatNo
    						|= STOP_PATH_MARK;
    					outWordsOfGraph(scanner);
    				} else {
    //				  saveNodeIndex(scanner);
                       setIdentifyValue(scanner,pathEtiQidx);
                   }
    				break;
                case SYMBOL:
                    if(!listOut)
                        setIdentifyValue(scanner,pathEtiQidx);
                    break;
    			case  STOP:
    			    if(listOut){
    					pathEtiQ[scanner].etatNo |= LOOP_PATH_MARK;
    					pathEtiQ[pathEtiQidx-1].etatNo|= LOOP_PATH_MARK|STOP_PATH_MARK;
    					outWordsOfGraph(depth);
    					pathEtiQ[scanner].etatNo &= ~LOOP_PATH_MARK;
    				}
    			    break;
    	        default:
    	            fatal_error("illegal execution");
    			}
    			totalLoop++;
    			return(1);
    		}
    	}
    	return(0);
    }


	unichar EBuff[128];
	unichar TBuff[128];
	int ePtrCnt;
	int tPtrCnt;
	unichar EOUTLINE[4096];
	unichar SOUTLINE[4096];
	int EOutCnt;
	int SOutCnt;

	void resetBuffs()
	{
		ePtrCnt = tPtrCnt = EOutCnt= SOutCnt = 0;
	}

	int outOneWord(unichar *suf){
		int i;
		int setOut;
		unichar *wp;
		EBuff[ePtrCnt]= TBuff[tPtrCnt] = 0;
		if(!ePtrCnt && !tPtrCnt && !suf) return 0;
		if(suf){
		  setOut = 0;
//printf("%d %d %d %d \n",ePtrCnt,tPtrCnt,*suf,count_in_line);
          if(ePtrCnt || tPtrCnt || *suf ||(count_in_line==0)){
            setOut = 1;
            if(prMode == PR_SEPARATION){
    			wp = sepL;
    			while(*wp) {
      				EOUTLINE[EOutCnt++] = *wp;
			        if(automateMode == TRANMODE)SOUTLINE[SOutCnt++] = *wp;
    				wp++;
    			}
    			for(i = 0; i < ePtrCnt;i++) EOUTLINE[EOutCnt++] = EBuff[i];
   				if(automateMode == TRANMODE)
                     for(i = 0; i < tPtrCnt;i++)
                      SOUTLINE[SOutCnt++] = TBuff[i];
    			wp = sepR;
    			while(*wp) {
    				if(ePtrCnt) EOUTLINE[EOutCnt++] = *wp;
			        if(automateMode == TRANMODE)SOUTLINE[SOutCnt++] = *wp;
    				wp++;
    			}
            } else {
    			wp = sepL;while(*wp) EOUTLINE[EOutCnt++] = *wp++;
    			for(i = 0; i < ePtrCnt;i++) EOUTLINE[EOutCnt++] = EBuff[i];
				wp = saveSep;while(*wp) EOUTLINE[EOutCnt++] = *wp++;
				if(automateMode == TRANMODE)
                    for(i = 0; i < tPtrCnt;i++)
    					EOUTLINE[EOutCnt++] = TBuff[i];
    			wp = sepR; while(*wp) EOUTLINE[EOutCnt++] = *wp++;
            }
		  } // condition de out
			if((recursiveMode == LABEL) && setOut){
			    if((automateMode == TRANMODE)&&(prMode == PR_SEPARATION)){
				wp = entreGO;while(*wp)	SOUTLINE[SOutCnt++] = *wp++;
				wp = suf;while(*wp)	SOUTLINE[SOutCnt++] = *wp++;
			    } else {
				wp = entreGO;while(*wp)	EOUTLINE[EOutCnt++] = *wp++;
				wp = suf;while(*wp)	EOUTLINE[EOutCnt++] = *wp++;
				}
			}
			EOUTLINE[EOutCnt] = 0; SOUTLINE[SOutCnt] = 0;
			u_fprintf(foutput,"%S",EOUTLINE);
            if((automateMode == TRANMODE) && SOutCnt){
                  u_fprintf(foutput,"%S%S",saveSep,SOUTLINE);
            }
            if(display_control ==  DEBUG)CqueuePathPr(foutput);
            u_fprintf(foutput,"\n");
            numberOfOutLine++;
		    EOutCnt = SOutCnt = 0;
		} else { // suf == 0
          if(ePtrCnt || tPtrCnt){
            if(prMode == PR_SEPARATION){
    			wp = sepL;
    			while(*wp) {
    				EOUTLINE[EOutCnt++] = *wp;
                    if(automateMode == TRANMODE) SOUTLINE[SOutCnt++] = *wp;
    				wp++;
    			}
    			for(i = 0; i < ePtrCnt;i++) EOUTLINE[EOutCnt++] = EBuff[i];
				if(automateMode == TRANMODE)
	   				for(i = 0; i < tPtrCnt;i++)
                        SOUTLINE[SOutCnt++] = TBuff[i];
//				if(recursiveMode == LABEL){
//				  wp = entreGO;while(*wp)	EOUTLINE[EOutCnt++] = *wp++;
//			    }
    			wp = sepR;
    			while(*wp) {
    				EOUTLINE[EOutCnt++] = *wp;
                    if(automateMode == TRANMODE) SOUTLINE[SOutCnt++] = *wp;
    				wp++;
    			}
            } else {
    			wp = sepL;while(*wp) EOUTLINE[EOutCnt++] = *wp++;
    			for(i = 0; i < ePtrCnt;i++) EOUTLINE[EOutCnt++] = EBuff[i];
				wp = saveSep;
				while(*wp) EOUTLINE[EOutCnt++] = *wp++;
				if(automateMode == TRANMODE)
                    for(i = 0; i < tPtrCnt;i++) EOUTLINE[EOutCnt++] = TBuff[i];
				if(recursiveMode == LABEL){
				  wp = entreGO;while(*wp) EOUTLINE[EOutCnt++] = *wp++;
			    }
    			wp = sepR; while(*wp) EOUTLINE[EOutCnt++] = *wp++;
            }
            count_in_line++;
		  }
		}

		ePtrCnt = tPtrCnt = 0;

		if(outLineLimit <= numberOfOutLine ){
            error("End by line limit %d\r\n", numberOfOutLine);
            return 1;
       }
		return 0;
	}

	void dummyWordOut()
	{
	    unichar*wp;
		if((recursiveMode == LABEL) && !count_in_line ){
   			wp = sepL;   while(*wp) EOUTLINE[EOutCnt++] = *wp++;
			wp = saveSep;while(*wp) EOUTLINE[EOutCnt++] = *wp++;
			wp = sepR;   while(*wp) EOUTLINE[EOutCnt++] = *wp++;
		    wp = entreGO;while(*wp) EOUTLINE[EOutCnt++] = *wp++;
	    }
	    EOUTLINE[EOutCnt++] = 0;
	    u_fprintf(foutput,"%S\n",EOUTLINE);
		numberOfOutLine++;
	}
	void putInt(int flag,int v)
	{
		if(v < 10){
			EBuff[ePtrCnt] =v % 10 + (unichar)'0';
			if(flag) TBuff[tPtrCnt++] = EBuff[ePtrCnt];
			ePtrCnt++;
			return;
		}
		putInt(flag,v / 10);
		EBuff[ePtrCnt] =v % 10 + (unichar)'0';
		if(flag) TBuff[tPtrCnt++] = EBuff[ePtrCnt];
		ePtrCnt++;
	}
	int prOutCycle()
	{
		struct cyclePathMark *h = headCyc;
		int i;
		unichar *wp;
//        unichar *wwp;
		Fst2Tag Eti;
		while(h){
		    u_fprintf(foutput,"C%d%S",h->index,entreGO);
			ePtrCnt = tPtrCnt = 0;
			for(i = 0; i < h->pathCnt;i++){
//				putInt(0,h->pathEtiQueue[i].path);
				Eti = a->tags[h->pathEtiQueue[i].eti];
				wp = 	(unichar *)Eti->input;

				if(u_strcmp(wp,u_epsilon_string)){
//					wwp = saveSep;while(*wwp) EBuff[ePtrCnt++] = *wwp++;
					while(*wp)	EBuff[ePtrCnt++] = *wp++;
				}
				wp = (unichar *)Eti->output;
				if((automateMode == TRANMODE) && wp && u_strcmp(wp,u_epsilon_string)){
//				    wwp = saveSep;while(*wwp) EBuff[ePtrCnt++] = *wwp++;
					while(*wp) TBuff[tPtrCnt++] = *wp++;
				}
			}
			if (outOneWord(entreGF) != 0)
                return 1;
			h = h->next;
		}
        return 0;
	}

	int prOutCycleAtNode(int autoNum,int nodeNum)
	{
		struct cyclePathMark *h = headCyc;
		int i,st,ed;
		int tmp;
		unichar *wp;
		Fst2Tag Eti;
		if(autoNum){
			st = a->initial_states[autoNum];
			ed =(autoNum  == a->number_of_graphs) ? a->number_of_states :
					a->initial_states[autoNum+1];
		} else {
			st = nodeNum; ed = st+1;
		}

		while(h){
			tmp = h->pathEtiQueue[h->pathCnt -1].etatNo;
			if(tmp & FILE_PATH_MARK)
				tmp = a->initial_states[tmp & SUB_ID_MASK];
			if( ( tmp < st) || (tmp >= ed)){
					h = h->next;continue;
			}

			ePtrCnt = tPtrCnt = 0;
			EBuff[ePtrCnt++] = (unichar)'C';
			putInt(0,h->index);
			EBuff[ePtrCnt++] = (unichar)':';

			for(i = 0; i < h->pathCnt;i++){
				tmp = h->pathEtiQueue[i].etatNo;
				if(tmp & FILE_PATH_MARK)
					tmp = a->initial_states[tmp & SUB_ID_MASK];
				if((tmp < st) || (tmp >= ed) )	break;

				Eti = a->tags[h->pathEtiQueue[i].eti];
				wp = (unichar *)	Eti->input;
				if(u_strcmp(wp,u_epsilon_string) && *wp){
					while(*wp)	EBuff[ePtrCnt++] = *wp++;
				}
				wp = (unichar *)Eti->output;
				if((automateMode == TRANMODE ) && wp && u_strcmp(wp,u_epsilon_string) && *wp ){
					while(*wp) TBuff[tPtrCnt++] = *wp++;
				}
			}
			if(i == h->pathCnt) {
				if (outOneWord(u_null_string) != 0)
                    return 1;
            }
			else
				resetBuffs();
			h = h->next;
		}
        return 0;
	}
	//
	//
	//
	void prSubGrapheCycle()
	{
		int i;
		for(i = 1; i <= a->number_of_graphs;i++)
		{
			char charBuffOut[1024];
			if(a->states[a->initial_states[i]]->control & LOOP_NODE_MARK)
				error("the sub-graph %s has cycle path\n",
				  getUtoChar(charBuffOut,a->graph_names[i]));
		}
	}

#define MAX_IGONRE_SOUS_GRAPHE	256
     unichar *arretSubList[MAX_IGONRE_SOUS_GRAPHE];
     int arretSubListIdx;

     void arretExpoList(char *src)
    {
    	char *cp = src;
    	unichar *wp;
    	if(arretSubListIdx == MAX_IGONRE_SOUS_GRAPHE) {
    		u_printf("too many igored sub-graph name igore%s\n",src);
    		return;
    	}
    	wp =new unichar [strlen(src)+1];
    	arretSubList[arretSubListIdx++] = wp;
    	while(*cp) *wp++ = (unichar)(*cp++ & 0xff);
    	*wp = (unichar)'\0';
		if(verboseMode) {
			char charBuffOut[1024];
			u_printf("IGNORE %s\n",getUtoChar(charBuffOut,arretSubList[arretSubListIdx-1]));
		}
    }
    char fileLine[1024];
     void arretExpoListFile(char *src)
    {
        int i;

        U_FILE* uf=u_fopen(ASCII,src,U_READ);
        if(!uf){
           fatal_error("Cannot open file %s\n",src);
        }
        while(af_fgets(fileLine,256,uf->f)){
            if(fileLine[0] == ' ') continue;
            for(i  = 0; (i< 128) && (fileLine[i] != ' ')
                    &&(fileLine[i] != '.') && (fileLine[i] != 0)
                    && (fileLine[i] != 0xa) && (fileLine[i] != 0xd)
            ;i++)
                        EBuff[i] = (unichar )(fileLine[i] & 0xff);
            EBuff[i++] = 0;

        	if(arretSubListIdx == MAX_IGONRE_SOUS_GRAPHE) {
        		u_printf("too many igored sub-graph name igore%s\n",src);
        		return;
        	}
        	arretSubList[arretSubListIdx] = new unichar [i];
        	for(i = 0;EBuff[i];i++) arretSubList[arretSubListIdx][i] = EBuff[i];
        	arretSubList[arretSubListIdx][i] = 0;
			if(verboseMode) {
				char charBuffOut[1024];
				u_printf("IGNORE %s\n",getUtoChar(charBuffOut,arretSubList[arretSubListIdx]));
			}
    		arretSubListIdx++;
        }
        u_fclose(uf);
    }

     void arretExpoDel()
    {
    	for(int i = 0; i<arretSubListIdx;i++)
    		delete arretSubList[i];
    }
    //
//
//
void prAutoStack(int depStack) {
    int i;
    u_printf("===== AutoQueue\n");
    for( i = 0; i <= CautoDepth;i++){
        u_printf("%d :: %d :: %d\n",i,CautoQueue[i].aId,CautoQueue[i].next);
    }
    u_printf("===== etatQueue\n");
    for(i = 0;i < pathEtiQidx;i++){
       u_printf("%d (%d ::%d)%d\n",i,pathEtiQ[i].autoNo,pathEtiQ[i].etatNo,pathEtiQ[i].eti);
    }
    u_printf("===== AutoStack\n");
    Transition *k;
    for(i = 0;i < depStack;i++){
      k = autoStackMap[i].tran;
      u_printf("%d %d(%d ::%d)\n",i,autoStackMap[i].autoId,k->state_number,k->tag_number);
    }

}
void prAutoStackOnly()
{
   for(int i = 0;i < pathEtiQidx;i++)
            u_fprintf(foutput,"%d (%d ::%d)%d\n",i,pathEtiQ[i].autoNo,
    pathEtiQ[i].etatNo,pathEtiQ[i].eti);
}

private:
   /* prevent GCC warning */

   CFstApp(const CFstApp&) :
		a (0),
        fst2_free(FST2_free_info_init),
		foutput(0),
		prMode(PR_SEPARATION),
		automateMode(AUTOMODE),
		listOut(0),
		verboseMode(0),
		pathEtiQidx(0),

        CautoDepth(0),

		ignoreTable(0),
		numOfIgnore(0),

		outLineLimit(0x10000000),
		numberOfOutLine(0),
        count_in_line(0),

        totalPath(0),
        totalLoop(0),
        stopPath(0),
		errPath(0),

		recursiveMode(STOP),
		display_control(FULL),
		traitAuto(SINGLE),
		niveau_traite_mot(1),  // unit of box is word
        depthDebug(0),

		saveSep (u_null_string),
		sepL (u_null_string),
		sepR (u_null_string),
		sep1 (u_null_string),
		stopSignal (u_null_string),
		saveEntre(u_null_string),
		entreGO(u_null_string),
		entreGF(u_null_string),

        encoding_output(DEFAULT_ENCODING_OUTPUT),
        bom_output(DEFAULT_BOM_OUTPUT),
        mask_encoding_compatibility_input(DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT),

        autoStackMap(NULL),
        mapOfCallHead(NULL),
        mapOfCallTail(NULL),

        cycInfos(NULL),

		headCyc(0),
		cyclePathCnt(0),
		headCycNodes(0),
		cycNodeCnt(0),

        ePtrCnt(0),
        tPtrCnt(0),
        EOutCnt(0),
        SOutCnt(0),

        arretSubListIdx(0)
   {
       fatal_error("Unexpected copy constructor for CFstApp\n");
   }

   CFstApp& operator =(const CFstApp&) {
                fatal_error("Unexpected = operator for CFstApp\n");
           return *this;
   }
};    // end of fstApp


void CFstApp::loadGraph(int& changeStrToIdx,unichar changeStrTo[][MAX_CHANGE_SYMBOL_SIZE],char *fname)
{
	int i_1,j_1;
	Transition *strans;

	a=load_abstract_fst2(fname,1,&fst2_free);
	if (a==NULL) {
      fatal_error("Cannot load graph file %s\n",fname);
	}

	for( i_1 = 0; i_1 < a->number_of_states;i_1++)
	{
		strans = a->states[i_1]->transitions;
		if(a->states[i_1]->control & 0x80){
              fatal_error("Not null control bit");
       }
		a->states[i_1]->control &= 0x7f;	// clean for mark recusive
		while(strans){
			if(strans->tag_number < 0){
				strans->tag_number =
					FILE_PATH_MARK | -strans->tag_number;
			}
			strans = strans->next;
		}
	}

	ignoreTable = new int [a->number_of_graphs+1];
    numOfIgnore = new int [a->number_of_graphs+1];
    for( i_1 = 1 ; i_1<= a->number_of_graphs;i_1++){
    	ignoreTable[i_1] = 0;
    	numOfIgnore[i_1] = 0;
	}

	if(arretSubListIdx) {

		for(i_1 = 0; i_1< arretSubListIdx;i_1++){
			for( j_1 = 1; j_1 <= a->number_of_graphs;j_1++){
				if(!u_strcmp((unichar *)a->graph_names[j_1],arretSubList[i_1]))
					break;
			}
			if( j_1 > a->number_of_graphs){
				char charBuffOut[1024];
				u_printf("Warning : Not exist the sub-graph %s\n",
					getUtoChar(charBuffOut,arretSubList[i_1]));
				continue;
			}
			{
				char charBuffOut[1024];
				u_printf("%s %d graphe ignore the exploitation\n",
						 getUtoChar(charBuffOut,a->graph_names[j_1]),j_1);
			}
			ignoreTable[j_1] = 1;
		}
	}
	if(stopSignal){

		for( i_1 = 0; i_1 < a->number_of_tags ;i_1++){
			if(u_strcmp((unichar *)a->tags[i_1]->input,stopSignal))
				continue;
			for(j_1 = 0; j_1 < a->number_of_states;j_1++){
				strans = a->states[j_1]->transitions;
				while(strans){
					if(strans->tag_number == i_1){
						strans->tag_number |= STOP_PATH_MARK;
					}
					strans = strans->next;
				}

			}
		}
	}
	if(changeStrToIdx){
		unichar *wp;
		int i,j,k,l,m;
		unichar temp[256];
		for( i = 0; i < a->number_of_tags ;i++){
			wp = (unichar *)a->tags[i]->input;
			for(j = 0; wp[j] ;j++) 	temp[j] = wp[j];
			temp[j] = 0;
			for(j = 0 ; j < changeStrToIdx;j++){
				wp = changeStrTo[j]+1;
				for(k = 0; temp[k];k++){
					for(l = 0; wp[l] ; l++){
						if(!temp[k+l] || (temp[k+l] != wp[l]))
							break;
					}
					if(wp[l]) continue;
					temp[k] = changeStrTo[j][0];
					l--;
					for(m= k+1 ; temp[m+l];m++)
						temp[m] = temp[m+l];
					temp[m] = 0;
				}
			}
			wp = (unichar *)a->tags[i]->input;
			if(u_strcmp(wp,temp)){
				char charBuffOut1[1024];
				char charBuffOut2[1024];
			   u_printf("%dth index, %s==>%s\n",i,
                        getUtoChar(charBuffOut1,wp),getUtoChar(charBuffOut2,temp));
            for(j = 0; temp[j];j++) *wp++ = temp[j];
			*wp=0;
			}

		}

	}
}

int CFstApp::getWordsFromGraph(int &changeStrToIdx,unichar changeStrTo[][MAX_CHANGE_SYMBOL_SIZE],char *fname)
{
	int i;
 	char *dp;
	char ofNameTmp[1024];
	char tmpchar[1024];
	char ttpchar[1024];
	// load fst2 file
	loadGraph(changeStrToIdx,changeStrTo,fname);
   CleanPathCounter();
   ofNameTmp[0] = 0;
	switch(display_control){
	case GRAPH:{
		listOut = 0;
        niveau_traite_mot =1;
		exploirerSubAuto(1);	// mark loop path start nodes
		prSubGrapheCycle();
		if (recursiveMode == LABEL) {
         error("warning:ignore the option -rl\r\n");
      }
		recursiveMode = SYMBOL;
		strcpy(tmpchar,ofnameOnly);
		strcat(tmpchar,"autolst");
		makeOfileName(ofNameTmp,tmpchar,".txt");

		foutput = u_fopen_creating_versatile_encoding(encoding_output,bom_output,ofNameTmp,U_WRITE);
		if(!foutput) {
         fatal_error("Cannot open file %s\n",ofNameTmp);
      }

		listOut = 1;

		for( i = 1; i <= a->number_of_graphs;i++){
			u_fprintf(foutput,"[%d th automata %S]\n",i,a->graph_names[i]);
//			printf("[%d th automata %s]\n",i,getUtoChar(a->nom_graphe[i]));

			if (exploirerSubAuto(i) != 0)
            {
                u_fclose(foutput);
                return 1;
            }

			if (prOutCycleAtNode(i,0) != 0)
            {
                u_fclose(foutput);
                return 1;
            }
			u_fprintf(foutput,
		" the automate %S, %d path, %d path stopped by cycle, %d error path\n",
					a->graph_names[i],totalPath,totalLoop, errPath);
			CleanPathCounter();
		}

		if(recursiveMode == SYMBOL) {
            if (prOutCycle() != 0) {
                u_fclose(foutput);
                return 1;
            }
        }
		u_fclose(foutput);

		break;
			   }
	case DEBUG: break;
	case FULL:
		switch(traitAuto){
		case SINGLE:
			{
            listOut = 0;
				exploirerSubAuto(1);	// mark loop path start nodes
				prSubGrapheCycle();
				makeOfileName(ofNameTmp,0,0);
				foutput = u_fopen_creating_versatile_encoding(encoding_output,bom_output,ofNameTmp,U_WRITE);
				if(!foutput) {
				    fatal_error("Cannot open file %s\n",ofNameTmp);
            }
				listOut = 1;
				exploirerSubAuto(1);
				if(verboseMode){
					char charBuffOut[1024];
					u_printf(" The automate %s : %d path, %d path stopped by cycle, %d error path\n",
					          getUtoChar(charBuffOut,a->graph_names[1]),
					          totalPath,totalLoop, errPath);
					if(stopPath){
					     for(int inx = 1; inx <= a->number_of_graphs;inx++){
                          if(numOfIgnore[inx]){
                              u_printf(" Sub call [%s] %d\n"
                                ,getUtoChar(charBuffOut,a->graph_names[inx])
                                ,numOfIgnore[inx]);
                               numOfIgnore[inx] = 0;
                           }
                         }
					}
				}
 				if(recursiveMode == SYMBOL) {
                    if (prOutCycle() != 0) {
                        u_fclose(foutput);
                        return 1;
                    }
                }
				u_fclose(foutput);
			}
			break;
		case MULTI:	// the first graph have only noms of the initials graphs
			{
			U_FILE* listFile;
			unichar *wp;
			Transition *sui;
		strcpy(tmpchar,ofnameOnly);
		strcat(tmpchar,"lst");
			makeOfileName(ofNameTmp,tmpchar,".txt");
			listFile = u_fopen(ASCII,ofNameTmp,U_WRITE);
			if(!(listFile))
				fatal_error("list file open error");
			i = 0;

			for( sui = a->states[0]->transitions;sui != 0 ; sui = sui->next){
				if(!(sui->tag_number & FILE_PATH_MARK))	continue;
				ignoreTable[sui->tag_number & SUB_ID_MASK] = 1;
				i++;
			}
			u_fprintf(listFile," %d\n",i);
			for( sui = a->states[0]->transitions;sui != 0 ; sui = sui->next){
				if(!(sui->tag_number & FILE_PATH_MARK)) continue;
				cleanCyclePath();

				wp = (unichar *)a->graph_names[sui->tag_number & SUB_ID_MASK];
				dp = tmpchar;

				while(*wp){*dp++ = (char)(*wp&0xff);wp++;}
				*dp++ = '\0';
				makeOfileName(ofNameTmp,tmpchar,0);
                remove_path(ofNameTmp,ttpchar);
				u_fprintf(listFile,"%s\r\n",ttpchar);


				foutput = u_fopen_creating_versatile_encoding(encoding_output,bom_output,ofNameTmp,U_WRITE);
				if (!foutput){
				  fatal_error("Cannot open file %s\n",ofNameTmp);
				}
				listOut = 0;     // output disable

				exploirerSubAuto(sui->tag_number & SUB_ID_MASK);
				prSubGrapheCycle();
				CleanPathCounter();
				listOut = 1;    // output enable
				exploirerSubAuto(sui->tag_number & SUB_ID_MASK);

				if(recursiveMode == SYMBOL)	 {
                    if (prOutCycle() != 0) {
                        u_fclose(foutput);
                        return 1;
                    }
                }
                if(verboseMode){
					char charBuffOut[1024];
					u_printf(" the automate %s %d path, %d path stopped by cycle, %d error path \n",
					           getUtoChar(charBuffOut,a->graph_names[sui->tag_number & SUB_ID_MASK]),
                               totalPath,totalLoop, errPath);

					if(stopPath){
					     for(int inx = 1; inx <= a->number_of_graphs;inx++){
                          if(numOfIgnore[inx]){
                              u_printf(" sub-call[%s] %d\n",
                                  getUtoChar(charBuffOut,a->graph_names[inx]),
                                  numOfIgnore[inx]);
                              numOfIgnore[inx] = 0;
                           }
                         }
					}
				}

				u_fclose(foutput);
			}
			u_fclose(listFile);
			}
			break;
		}
	}
    return 0;
}
//
//
//
int CFstApp::exploirerSubAuto(int startAutoNo)
{
    Transition startCallTr;
//if(listOut) prCycleNode();
    startCallTr.tag_number  = startAutoNo |FILE_PATH_MARK;
    startCallTr.state_number        = 0;
    numberOfOutLine = 0;    // reset output lines


    autoStackMap[0].tran = &startCallTr;
    int callSubId = callIdentifyId(autoStackMap,1);
    autoStackMap[0].autoId = callSubId;

    CautoDepth = 0;
	CautoQueue[CautoDepth].aId = callSubId;
	CautoQueue[CautoDepth].next = 0;
    pathEtiQidx = 0;
	pathEtiQ[pathEtiQidx].autoNo = callSubId;
	pathEtiQ[pathEtiQidx].etatNo = a->initial_states[startAutoNo];
	pathEtiQ[pathEtiQidx].eti = 0;
	pathEtiQidx++;
	if (findCycleSubGraph(callSubId,1,a->initial_states[startAutoNo],0) != 0)
        return 1;
	pathEtiQidx--;
	if(pathEtiQidx) fatal_error("error in program");
    return 0;
}

//
//	find cycle path by node and call
//
int CFstApp::findCycleSubGraph(int automateNo,int autoDepth,int stateNo,int stateDepth)
{
	int skipCnt = 0;
	int i;
	int tmp;
	int nEtat;
    int callId;
    int scanner;
//	prAutoStack(autoDepth);
	if(listOut && WasCycleNode(automateNo,stateNo)){
		pathEtiQ[pathEtiQidx-1].etatNo |= LOOP_PATH_MARK;
	}
	if( pathEtiQidx > PATH_QUEUE_MAX){
		if(listOut){
				errPath++;
				pathEtiQ[pathEtiQidx].etatNo = STOP_PATH_MARK;
				pathEtiQ[pathEtiQidx].eti = 0;
				pathEtiQidx++;
				if (outWordsOfGraph(pathEtiQidx) != 0)
                    return 1;
				pathEtiQidx--;
		} else
			error("Warning:too many calls\n");
		return 0;
	}
	if(IsCyclePath(stateDepth)){
		if(recursiveMode == STOP)	return 0;
//		if(!listOut){
//            a->etat[stateNo]->controle |= LOOP_NODE_MARK;
//		}
		return 0;
	}

	if (is_final_state(a->states[stateNo])) {	// terminal node
		if(autoDepth != 1){		// check continue  condition
			skipCnt = 0;	// find next state
			for(i = CautoDepth;i>=0; --i){
				if(CautoQueue[i].aId == -1)
					skipCnt++;
				else {
					if(skipCnt)
						skipCnt--;
					else
						break;
				}
			}

			// ?
			if( i == 0) {
                //fatal_error("not want state arrive");
                error("not want state arrive");
                return 1;
            }
			int tauto = CautoQueue[i].aId;
			nEtat = CautoQueue[i].next;
			CautoDepth++;
			CautoQueue[CautoDepth].aId = -1;
			CautoQueue[CautoDepth].next = 0;

			pathEtiQ[pathEtiQidx].etatNo = nEtat;
			pathEtiQ[pathEtiQidx].eti = 0;
			pathEtiQ[pathEtiQidx].autoNo = tauto;
			pathEtiQidx++;
			if (findCycleSubGraph(tauto,autoDepth-1,nEtat,stateDepth+1) != 0)
                return 1;
			pathEtiQidx--;
			CautoDepth--;
		} else {	// stop condition
			if(listOut){
				totalPath++;
				pathEtiQ[pathEtiQidx].etatNo = STOP_PATH_MARK;
				pathEtiQ[pathEtiQidx].eti = 0;
				pathEtiQ[pathEtiQidx].autoNo = automateNo;
				pathEtiQidx++;
				if (outWordsOfGraph(pathEtiQidx) != 0)
                    return 1;
				pathEtiQidx--;
			} else {

			}
		}
	}
	for(Transition *sui = a->states[stateNo]->transitions;
	sui != 0 ; sui = sui->next){

		if(sui->tag_number & STOP_PATH_MARK){
			if(listOut){
				totalPath++;
				pathEtiQ[pathEtiQidx].autoNo = automateNo;
				pathEtiQ[pathEtiQidx].etatNo = STOP_PATH_MARK;
				pathEtiQ[pathEtiQidx].eti = sui->tag_number
					& ~STOP_PATH_MARK;
				pathEtiQidx++;
				if (outWordsOfGraph(pathEtiQidx) != 0)
                    return 1;
				pathEtiQidx--;
			}
			continue;
		}
		if(display_control == GRAPH)
		{
			if(listOut){
			    pathEtiQ[pathEtiQidx].autoNo = automateNo;
    			pathEtiQ[pathEtiQidx].etatNo = sui->state_number;
    			pathEtiQ[pathEtiQidx].eti = sui->tag_number;
    			pathEtiQidx++;
    			if (findCycleSubGraph(automateNo,autoDepth,sui->state_number,stateDepth+1) != 0)
                    return 1;
    			pathEtiQidx--;
			}
			continue;
		}
		if(sui->tag_number & FILE_PATH_MARK ) {	// handling sub call
			if(ignoreTable[sui->tag_number & SUB_ID_MASK]){
			   // find stop condition path
				if(listOut){
					totalPath++;
					stopPath++;

					numOfIgnore[sui->tag_number & SUB_ID_MASK]++;

					pathEtiQ[pathEtiQidx].autoNo = automateNo;
					pathEtiQ[pathEtiQidx].eti = sui->tag_number;
					pathEtiQ[pathEtiQidx].etatNo = STOP_PATH_MARK;
					pathEtiQidx++;
					if (outWordsOfGraph(pathEtiQidx) != 0)
                        return 1;
					pathEtiQidx--;
				}
				continue;
			}
			//
			//    find cycle call
			//
			tmp = sui->tag_number & SUB_ID_MASK;

    	    for(scanner = 0;scanner < autoDepth;scanner++)
        	    if(autoStackMap[scanner].tran->tag_number == sui->tag_number)
        	       break;
            autoStackMap[autoDepth].tran = sui;
            if(scanner == autoDepth)
			{
			   callId =  callIdentifyId(autoStackMap,autoDepth+1);
           	} else { // find recusive call
              pathEtiQ[pathEtiQidx].eti = 0;
              pathEtiQ[pathEtiQidx].autoNo = autoStackMap[scanner].autoId;;
              pathEtiQ[pathEtiQidx].etatNo = a->initial_states[tmp]| LOOP_PATH_MARK;;
              ++pathEtiQidx;
              if(!IsCyclePath(stateDepth)) fatal_error("recursive find fail");
              --pathEtiQidx;
              continue;
            }
            pathEtiQ[pathEtiQidx].eti = 0;
            pathEtiQ[pathEtiQidx].autoNo = callId;
            pathEtiQ[pathEtiQidx].etatNo = a->initial_states[tmp];
            ++pathEtiQidx;
            autoStackMap[autoDepth].autoId = callId;

            CautoDepth++;
			CautoQueue[CautoDepth].aId = callId;
			CautoQueue[CautoDepth].next = sui->state_number;
			if (findCycleSubGraph(callId,autoDepth+1,
                  a->initial_states[tmp],stateDepth+1) != 0)
                  return 1;
			--pathEtiQidx;
			--CautoDepth;
			continue;
		}
		pathEtiQ[pathEtiQidx].etatNo = sui->state_number;
		pathEtiQ[pathEtiQidx].eti = sui->tag_number;
		pathEtiQ[pathEtiQidx].autoNo = automateNo;
		++pathEtiQidx;
		if (findCycleSubGraph(automateNo,autoDepth,sui->state_number,stateDepth+1) != 0)
            return 1;
		pathEtiQidx--;
	}
    return 0;
}


//
//	for debugging, display all stack
//
void CFstApp::CqueuePathPr(U_FILE *f)
{
	int pidx = -1;
	int i;
	u_fprintf(f,"#");
	for(i = 0; i < pathEtiQidx;i++)
	{
		if(pathEtiQ[i].autoNo != pidx){ // skip the same value
			pidx = pathEtiQ[i].autoNo;
			u_fprintf(f,"%S>",a->graph_names[pidx]);
		}
	}
}


unichar *
uascToNum(unichar *uasc,int *val)
{
	unichar *wp = uasc;
	int base = 10;
	int sum = 0;
	if((*wp == (unichar)'0') && ((*(wp+1)==(unichar)'x') || (*(wp+1) == (unichar)'X')))
	{
		base = 16;
		wp+=2;
	}
	do {
		if( (*wp >= (unichar)'0') && (*wp <= (unichar)'9')){
			sum = sum*base + *wp - (unichar)'0';
				wp++;
		} else if ((base == 16) &&
				(*wp >= (unichar)'a') && (*wp <= (unichar)'f')){
				sum = sum *base + *wp - (unichar)'a'+10;
				wp++;
		} else if ((base == 16) &&
				(*wp >= (unichar)'A') && (*wp <= (unichar)'F')){
				sum = sum*base + *wp - (unichar)'A'+10;
				wp++;
		} else {
			break;
		}
	} while(*wp);
	*val = sum;
	return(wp);
}


int CFstApp::outWordsOfGraph(int depth)
{
	int s;
	Fst2Tag Eti;
	unichar *sp;
	unichar *wp;
	unichar *ep;
	unichar *tp;
	unichar *chp;
	int indicateFirstUsed;
	int i;
	int markCtlChar,markPreCtlChar;
	depthDebug = pathEtiQidx;
	EOutCnt = SOutCnt = 0;
	ePtrCnt = tPtrCnt = 0;
	unichar aaBuffer_for_getLabelNumber[64];

	//	fini = (etiQ[etiQidx - 1] & (FILE_PATH_MARK | LOOP_PATH_MARK)) ?
	//		etiQ[etiQidx -1 ]:0;
	//
	//	elimine the value signified repete
	//
	markCtlChar= markPreCtlChar = 0;
	indicateFirstUsed = 0;
	count_in_line = 0;
//prAutoStackOnly();
	for(s = 0;s < pathEtiQidx;	s++){
		EBuff[ePtrCnt] = TBuff[tPtrCnt] = 0;
		if(!pathEtiQ[s].eti) {
			ep = tp = u_null_string;
		} else if(pathEtiQ[s].eti & FILE_PATH_MARK){
			ep = (display_control == GRAPH) ?
				(unichar *)a->graph_names[pathEtiQ[s].eti & SUB_ID_MASK]:u_null_string;
			tp = u_null_string;
		} else {
			Eti = a->tags[pathEtiQ[s].eti & SUB_ID_MASK];
			ep = (u_strcmp(Eti->input,u_epsilon_string)) ?
				Eti->input : u_null_string;

            if(Eti->output!=NULL){
			tp = (u_strcmp(Eti->output,u_epsilon_string)) ?
				Eti->output:u_null_string;
				} else tp = u_null_string;
		}
//wprintf(L"{%d,%x,%x,%s,%s}",s,pathEtiQ[s].etatNo,pathEtiQ[s].eti,ep,tp);
        markCtlChar = 0;
		if(!(pathEtiQ[s].etatNo & STOP_PATH_MARK) &&
             !niveau_traite_mot && (*ep == '<')){
            chp = ep+1;
            while(*chp) chp++; --chp;
		    if(*chp == (unichar)'>'){
                  markCtlChar = 1;
//		           if(ePtrCnt || tPtrCnt) outOneWord(0);
//		           else if(control_char) outOneWord(0);
//                   control_char = 1;
//  				   while(*ep) EBuff[ePtrCnt++] = *ep++;
//				   while(*tp) TBuff[tPtrCnt++] = *tp++;
//				   continue;
             }

        }
//wprintf(L"\n");

		if(pathEtiQ[s].etatNo & LOOP_PATH_MARK){
			if(recursiveMode == LABEL){
	        	if( *ep  || *tp ){ // current
                    if(tPtrCnt) {
                        if (outOneWord(0) != 0)
                            return 1;
                    }
                    if(markPreCtlChar && *ep) {
                        if (outOneWord(0) != 0)
                            return 1;
                    }
           			while(*ep) EBuff[ePtrCnt++] = *ep++;
           			if(automateMode == TRANMODE)
                         while(*tp) TBuff[tPtrCnt++] = *tp++;

            		if(niveau_traite_mot) {
            			if(ePtrCnt|| tPtrCnt) {
                            if (outOneWord(0) != 0)
                                return 1;
                        }
            		}
//                    else {
//            			if(control_char) outOneWord(0);
//            		}

				}
				if(pathEtiQ[s].etatNo & STOP_PATH_MARK){
				    sp = getLabelNumber(aaBuffer_for_getLabelNumber,depth,indicateFirstUsed,s,0);
					if (outOneWord(sp) != 0)
                        return 1;
					break;
				}
				sp = getLabelNumber(aaBuffer_for_getLabelNumber,s,indicateFirstUsed,s,1);
				if(!indicateFirstUsed){	// first print out
					if (outOneWord(sp) != 0)
                        return 1;
				}	else {
					resetBuffs();
				}
				while(*sp)	EOUTLINE[EOutCnt++] = *sp++;
				wp = entreGF;
                while(*wp) EOUTLINE[EOutCnt++] = *wp++;
                markPreCtlChar = markCtlChar;
				continue;
			} else if(recursiveMode == SYMBOL) {	// SYMBOL
				if(tPtrCnt && niveau_traite_mot) {
                            if (outOneWord(0) != 0)
                                return 1;
                        }
				wp = entreGO;
				while(*wp){
					if(automateMode == TRANMODE)  TBuff[tPtrCnt++] = *wp;
					EBuff[ePtrCnt++] = *wp;
					wp++;
				}
				while(*ep)	EBuff[ePtrCnt++] = *ep++;
				EBuff[ePtrCnt++] = (unichar)'|';
				if(automateMode == TRANMODE){
					while(*tp) TBuff[tPtrCnt++] = *tp++;
					TBuff[tPtrCnt++] = (unichar)'|';
				}
				struct cyclePathMark *h = headCyc;
				int findId = pathEtiQ[s].etatNo & PATHID_MASK;
				while(h){
					for(i = 0; i < h->pathCnt;i++){
						if(h->pathEtiQueue[i].etatNo == findId)
							break;
					}
					if(i != h->pathCnt){
						if(automateMode == TRANMODE){
							EBuff[ePtrCnt++] = (unichar)'C';
                            TBuff[tPtrCnt++] = (unichar)'C';
							putInt(1,h->index);
							EBuff[ePtrCnt++] = (unichar)'|';
                            TBuff[tPtrCnt++] = (unichar)'|';
						} else {
							EBuff[ePtrCnt++] =(unichar)'C';
							putInt(0,h->index);
							EBuff[ePtrCnt++] = (unichar)'|';
						}
					}
					h = h->next;
				}
				if(automateMode == TRANMODE) --tPtrCnt;
                --ePtrCnt;
				wp = entreGF;
				while(*wp){
					if(automateMode == TRANMODE) TBuff[tPtrCnt++] = *wp;
					EBuff[ePtrCnt++] = *wp;
					wp++;
				}
                if (outOneWord(0) != 0)
                    return 1;

				continue;
			} else { // STOP
                if (outOneWord(0) != 0)
                    return 1;
				if(!(pathEtiQ[s].etatNo & STOP_PATH_MARK)){
                    // mark the stop
                    wp = entreGO;
                    while(*wp){
                    EOUTLINE[EOutCnt++] = *wp;
   					if((automateMode == TRANMODE) && (prMode == PR_SEPARATION))
                    SOUTLINE[SOutCnt++] = *wp;
                    wp++;
                    }
				}
    			while(*ep) EBuff[ePtrCnt++] = *ep++;
    			if(automateMode == TRANMODE)
                     while(*tp) TBuff[tPtrCnt++] = *tp++;
                if(pathEtiQ[s].etatNo & STOP_PATH_MARK) {
                    wp = entreGF;
                    while(*wp){
                        if(automateMode == TRANMODE){
                            TBuff[tPtrCnt++] = *wp;
                            if(prMode == PR_SEPARATION)
                            EBuff[ePtrCnt++] = *wp;
                        } else
                            EBuff[ePtrCnt++] = *wp;

                        wp++;
                    }
                    if (outOneWord(u_null_string) != 0)
                        return 1;
		        } else {
            		if(niveau_traite_mot) {
            			if(ePtrCnt|| tPtrCnt)
                        {
                            if (outOneWord(0) != 0)
                                return 1;
                        }
            		} else {
            			if(markPreCtlChar)
                        {
                            if (outOneWord(0) != 0)
                                return 1;
                        }

            		}
                }
                markPreCtlChar = markCtlChar;
				continue;
			}
		}

		if(pathEtiQ[s].etatNo & STOP_PATH_MARK) {
//printf("stop %d\n",s);
            if(markPreCtlChar && markCtlChar)
            {
                if (outOneWord(0) != 0)
                    return 1;
            }
			if((automateMode == TRANMODE)&& *tp){ // current
			    if(tPtrCnt) {
                    if (outOneWord(0) != 0)
                        return 1;
                }
				while(*tp) TBuff[tPtrCnt++] = *tp++;
			}
			if(pathEtiQ[s].eti & FILE_PATH_MARK) {
                    if (outOneWord((unichar *)a->graph_names[pathEtiQ[s].eti & SUB_ID_MASK]) != 0)
                        return 1;
                }
			else {
                    if (outOneWord(u_null_string) != 0)
                        return 1;
                }
			break;
		}
		if(pathEtiQ[s].eti & FILE_PATH_MARK){

			if(tPtrCnt ||(markPreCtlChar && *ep))  {
                    if (outOneWord(0) != 0)
                        return 1;
                }

			switch(display_control){
			case GRAPH:
				EBuff[ePtrCnt++] = (unichar)'{';
				while(*ep)	EBuff[ePtrCnt++] = *ep++;
				EBuff[ePtrCnt++] = (unichar)'}';
                if (outOneWord(0) != 0)
                        return 1;
               break;
			case DEBUG:
			case FULL:
				fatal_error("???");
			}
			markPreCtlChar = markCtlChar;
			continue;
		}

// make a pair of (entre, sorti)
        if( (*ep == 0)  && ( *tp==0) ){
                continue;
          }

        if(tPtrCnt || (markPreCtlChar && *ep))  {
                    if (outOneWord(0) != 0)
                        return 1;
                }
        while(*ep) EBuff[ePtrCnt++] = *ep++;
        if(automateMode == TRANMODE)
			while(*tp) TBuff[tPtrCnt++] = *tp++;
		if(niveau_traite_mot) {
			if(ePtrCnt|| tPtrCnt)   {
                    if (outOneWord(0) != 0)
                        return 1;
                }
		}
//         else {
//			if(control_char) outOneWord(0);
//		}
		markPreCtlChar = markCtlChar;
	}
    return 0;
}

//
//
//
//


int main_Fst2List(int argc,char* const argv[]) {

	char *ofilename =0;
	int iargIndex = 1;
	int i;

	unichar changeStrTo[16][MAX_CHANGE_SYMBOL_SIZE];
	int changeStrToIdx;

   CFstApp aa;

	changeStrToIdx = 0;

	char *wp;
	unichar *wp2,*wp3;

    Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
    int bom_output = DEFAULT_BOM_OUTPUT;
    int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;

	while(iargIndex < argc){
		if(*argv[iargIndex] != '-') break;
		switch(argv[iargIndex][1]){
		case 'f': iargIndex++;
			switch(argv[iargIndex][0]){
			case 's': aa.prMode = PR_SEPARATION; break;
			case 'a': aa.prMode = PR_TOGETHER;break;
			default: usage();
                  return 1;
			}
			break;
		case 'o':iargIndex++;
			ofilename = new char [strlen(argv[iargIndex])+1];
			strcpy(ofilename,argv[iargIndex]);
			break;
		case 'l': iargIndex++;
			aa.outLineLimit = atoi(argv[iargIndex]);
			break;
		case 'i': iargIndex++;	// stop the exploitation
			aa.arretExpoList(argv[iargIndex]);
			break;
		case 'I': iargIndex++;	// stop the exploitation
			aa.arretExpoListFile(argv[iargIndex]);
			break;
		case 'p':			iargIndex++;
			switch(argv[iargIndex][0]){
			case 's': aa.display_control = GRAPH; break;
			case 'f': aa.display_control = FULL; break;
			case 'd': aa.display_control = DEBUG; break;
			default: usage();
                  return 1;
			}
			break;
		case 'a':
		case 't':
			aa.automateMode = (argv[iargIndex][1] == 't')
				? TRANMODE:AUTOMODE;
			iargIndex++;
			switch(argv[iargIndex][0]){
			case 's': aa.traitAuto = SINGLE; break;
			case 'm': aa.traitAuto = MULTI; break;
			default: usage();
                  return 1;
			}
			break;
		case 'v':
			aa.verboseMode = 1;
			break;
        case 'm':
            aa.niveau_traite_mot = 0; break;
		case 'r':
			switch(argv[iargIndex][2]){
			case 's': aa.recursiveMode = SYMBOL; break;
			case 'l': aa.recursiveMode = LABEL; break;
			case 'x': aa.recursiveMode = STOP; break;
			default: usage();
                  return 1;
			}
			iargIndex++;
			aa.saveEntre = new unichar[strlen(argv[iargIndex])+1];
			wp =  argv[iargIndex];
			wp2 = aa.saveEntre;
			wp3 = 0;
			while(*wp){
				if( (*wp < 0x20) ||	(*wp > 0x7e))
				{
					error("Warning:use a separator in ASC code\r\n");
					usage();
               return 1;
				}
				if(*wp == '\\') wp++;
				else if(*wp == ',') wp3 = wp2;
				if(*wp != '"') 	*wp2++ = (unichar)*wp++;
				else wp++;
			}
			*wp2 = 0;

			aa.entreGO = aa.saveEntre;
			if(wp3){
				*wp3++ = 0;
				aa.entreGF = wp3;
			} else
				aa.entreGF = wp2;
			break;
		case 'c':			iargIndex++;
			if(!changeStrToVal(changeStrToIdx,changeStrTo,argv[iargIndex])) break;
			usage();
         return 1;
		case 's':{
			i = 0;
			char cc= argv[iargIndex][2];
			iargIndex++;
			wp = argv[iargIndex];
			wp3 = 0;
			switch(cc){
			case 0  :
				wp2 = aa.sep1 = new unichar [strlen(wp)+1];
				wp3 = 0;
				while(*wp){
					if( (*wp < 0x20) ||	(*wp > 0x7e))
					{
						error("Warning:use a separator in ASCII code\r\n");
						usage();
                  return 1;
					}
					switch(*wp){
					case '\\':wp++;
                          if(*wp == '\0') {usage();
                        return 1;}
                          if(*wp != '"')  break;
                    case '"': wp++; continue;
					case ',': wp3 = wp2; break;
					default: break;
					}
                    *wp2++ = (unichar)*wp++ ;
				}
				*wp2 = 0;
				aa.sepL = aa.sep1;
				if(wp3){
					*wp3++ = 0;
					aa.sepR = wp3;
				} else {
					aa.sepR = wp2;
				}

				break;
			case '0':
				wp2 = aa.saveSep = new unichar [strlen(wp)+1];
				while(*wp){
					if( (*wp < 0x20) ||	(*wp > 0x7e))
					{
						error("Warning:use a separator in ASC code\r\n");
						usage();
                  return 1;
					}
					if(*wp == '\\'){
                          wp++;
                          if(*wp == '\0') {usage();
      return 1;}
                          }
                    if(*wp == '"') continue;
					*wp2++ = (unichar)*wp++;
				}
				*wp2 = 0;
				break;
			case 's':
				wp2 = aa.stopSignal= new unichar [strlen(wp)+3];;
				*wp2++ = (unichar)'<';
				while(*wp){
					if(*wp == '"') continue;
                    *wp2++ = (unichar)*wp++;
				}
				*wp2++= (unichar)'>';
				*wp2= 0;
				break;
            case 'k': iargIndex++;
                     if (argv[iargIndex][0]=='\0') {
                        fatal_error("Empty input_encoding argument\n");
                     }
                     decode_reading_encoding_parameter(&mask_encoding_compatibility_input,argv[iargIndex]);
                     break;
            case 'q': iargIndex++;
                     if (argv[iargIndex][0]=='\0') {
                        fatal_error("Empty output_encoding argument\n");
                     }
                     decode_writing_encoding_parameter(&encoding_output,&bom_output,argv[iargIndex]);
                     break;

			default:
				usage();
      return 1;
			}



		}
			break;
		default:
			usage();
      return 1;
		}
		iargIndex++;
	}
	if(iargIndex != (argc -1 )) {
      usage();
      return 1;
   }
	aa.fileNameSet(argv[iargIndex],ofilename);
    aa.fileEncodingSet(encoding_output,bom_output,mask_encoding_compatibility_input);
	aa.getWordsFromGraph(changeStrToIdx,changeStrTo,argv[iargIndex]);
	if(ofilename) delete ofilename;
	return 0;
}
