 /*
  * Unitex
  *
  * Copyright (C) 2001-2004 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

//---------------------------------------------------------------------------
#include <stdlib.h>

using namespace std;

#include "unicode.h"
#include "Fst2.h"
#include "LiberationFst2.h"
#include "Alphabet.h"
#include "Liste_num.h"
#include "UnicharTree.h"
#include "Copyright.h"
#include "FileName.h"
#include <locale.h>
#include "IOBuffer.h"



static void exitMessage(char *mes)
{
	fprintf(stderr,"ExitMessage:::%s\n",mes);
	exit(1);
}

void usage() {
      //012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
printf("%s",COPYRIGHT);
printf("Usage:\n");
printf(
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
" -r[s/l/x] \"L[,R]\"  : present recusive path(c0|...|cn) by Lc0|..|cnR : default null\r\n");
exitMessage("");
}
static char charBuffOut[1024];
static char *getUtoChar(unichar *s)
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
unichar changeStrTo[16][MAX_CHANGE_SYMBOL_SIZE];
int changeStrToIdx;

static int changeStrToVal(char *src)
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
		printf("the name of the variable too long %s",src);
		exitMessage("");
	}
	changeStrTo[changeStrToIdx][i++] = (unichar)'>';	
	changeStrTo[changeStrToIdx][i++] = (unichar)'\0';
	ptLoc = i;
	if(!*wp) usage();
	for(wp++;i <MAX_CHANGE_SYMBOL_SIZE && (*wp);i++)
		changeStrTo[changeStrToIdx][i] = (unsigned short)*wp++;
	if(*wp != (unichar)'\0') return(1);
	changeStrTo[changeStrToIdx][i++] = (unichar)'\0';
	uascToNum(&changeStrTo[changeStrToIdx][ptLoc],&i);
	changeStrTo[changeStrToIdx][0] = (unsigned short)i;

	printf("Change symbol %s --> %x\n",
        getUtoChar(&changeStrTo[changeStrToIdx][1]),
		changeStrTo[changeStrToIdx][0]);
	changeStrToIdx++;
	return(0);
}
static unichar u_null_string[]= {(unichar)'\0',(unichar)'\0'};
static unichar u_epsilon_string[] = {(unichar)'<',(unichar)'E'
,(unichar)'>',(unichar)'\0'};

static char *StrMemLack = "allocation of memory for cycle data is fail";

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
	
	struct automate_fst2 *a;
	FILE *foutput;
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
	void loadGraph(char *fname);
	void exploirerSubAuto(int startSubAutoNum);
	void getWordsFromGraph(char *fst2_file_name);
		void findCycleSubGraph(int autoNo,int autodep,int testEtat,int depthState);
			void outWordsOfGraph(int depth);	


	// the stack which keep the path and called sous-graphe	
	struct stackAuto {
		int aId;
		int next;
	} CautoQueue[2048];
	int CautoDepth;
	
	void CqueuePathPr(FILE *f,int dep);

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
	char ofdirName[1024];
	char ofExt[16];
	char ofnameOnly[512];
	char defaultIgnoreName[512]; // input filename
	void fileNameSet(char *ifn,char *ofn)
	{
	    char tmp[512];
		name_without_path(ifn,tmp);
		name_without_extension(tmp,defaultIgnoreName);
		if(!ofn){
		    get_filename_path(ifn,ofdirName);
		    name_without_path(ifn,tmp);
		    name_without_extension(tmp,ofnameOnly);
		    strcpy(ofExt,".txt");
		} else {
		    get_filename_path(ofn,ofdirName);
  		    name_without_path(ofn,tmp);
		    name_without_extension(tmp,ofnameOnly);
		    file_name_extension(tmp,ofExt);
		}
//fprintf(stderr,"%s %s %s",ofdirName,ofExt,ofnameOnly);
		if(ofnameOnly[0]== 0) exitMessage("ofile name not correct");
	}
	void makeOfileName(char *des,char *fn,char *ext){
	   strcpy(des,ofdirName);
	   if(fn) strcat(des,fn);
	   else strcat(des,ofnameOnly);
	   if(ext) strcat(des,ext);
	   else strcat(des,ofExt);
	}

	CFstApp(){
		a =0;
		sepL = u_null_string;	
		sepR = u_null_string;	
		sep1 = u_null_string;
		saveSep = u_null_string;
		stopSignal = u_null_string;
		saveEntre= u_null_string;
		entreGO = u_null_string;
		entreGF	= u_null_string;
		pathEtiQidx = 0;
	
		foutput = 0;
		ignoreTable = 0;
		numOfIgnore = 0;
		errPath =0;
verboseMode  = 0;
		outLineLimit = 0x10000000;
		numberOfOutLine = 0;
		niveau_traite_mot = 1;  // unit of box is word
		listOut = 0;
		
		headCyc = 0;
		cyclePathCnt = 0;
		headCycNodes = 0;
		cycNodeCnt = 0;

		prMode = PR_SEPARATION;
		traitAuto = SINGLE;
		display_control = FULL;
		recursiveMode = STOP;
		automateMode = AUTOMODE;
			arretSubListIdx = 0;
		initCallIdMap();
	};
	~CFstApp(){
	    arretExpoDel();
		cleanCyclePath();
		if(a) free_fst2(a);
        if(saveSep != u_null_string) delete saveSep;

		if(sep1 != u_null_string) delete sep1;
		if(stopSignal != u_null_string) delete stopSignal;
		if(saveEntre != u_null_string) delete saveEntre;
		if(ignoreTable) delete ignoreTable;
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
     Fst2Transition tran;
     int autoId;
    } *autoStackMap;
    struct callIdMap {
        int cnt;
        Fst2Transition *list;
        struct callIdMap *next;
    } *mapOfCallHead,*mapOfCallTail;
    
    
    int callIdentifyId(struct callStackMapSt *cmap,int count)
    {
        int id = 0;
        int i;
        struct callIdMap *fPtr;
        fPtr = mapOfCallHead;
//        for(i = 0;i < count;i++) fwprintf(stderr,L"0x%08x:",cmap[i].tran);
        while(fPtr){
            if(fPtr->cnt == count){
                for(i = 0; i < count;i++)
                    if(fPtr->list[i] != cmap[i].tran) 
                        break;
                if(i == count) {
//fwprintf(stdout,L"existed Id %d\n",id);
                    return id;
                }
            }
            id++;
            fPtr = fPtr->next;
        }
        fPtr = new struct callIdMap;
        fPtr->cnt = count;
        fPtr->next = 0;
        fPtr->list = new Fst2Transition [count];
        for(i = 0; i < count;i++) fPtr->list[i] = cmap[i].tran;
    
        if(mapOfCallTail){
                mapOfCallTail->next = fPtr;
                mapOfCallTail = fPtr;
        } else {
                mapOfCallTail = mapOfCallHead = fPtr;
        }
//fwprintf(stdout,L"get Id %d\n",id);
        return id;
    }
    void initCallIdMap()
    {
        autoStackMap = new struct callStackMapSt[1024];
        mapOfCallHead = mapOfCallTail = 0;
    }
    void finiCallIdMap()
    {
        delete autoStackMap;
        while(mapOfCallHead)
        {
           mapOfCallTail = mapOfCallHead;
            mapOfCallHead = mapOfCallHead->next;
            delete mapOfCallTail->list;
            delete mapOfCallTail;
        }
    }
    unichar aa[64];
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
    struct cycleNodeId {    
    	int index;
   	    int autoNo;
   	    int etatNo;
   	    int eti;
    	struct linkCycle {
    		struct cyclePathMark *cyc;
    		struct linkCycle *next;
    	} *cycInfos;
    	int flag;
    	struct cycleNodeId *next;
    };
    struct cyclePathMark *headCyc;
	int cyclePathCnt;
	struct cycleNodeId *headCycNodes;
	int cycNodeCnt;
	
	unichar *getLabelNumber(int numOfPath,int &flag,int curidx,int setflag)
	{
		struct cycleNodeId *cnode = headCycNodes;
		int i;
		int searchEtat = pathEtiQ[curidx].etatNo & PATHID_MASK;
		int searchEtatAuto = pathEtiQ[curidx].autoNo;
		int searchEti = pathEtiQ[curidx].eti;
//fwprintf(stdout,L"demande %08x,%08x::",searchEtatAuto,searchEtat);		
		while(cnode){
		    if(
              (searchEtatAuto ==  cnode->autoNo) &&
              (searchEtat ==  cnode->etatNo) &&
              (searchEti ==  cnode->eti)
             ) break;
			cnode = cnode->next;
		}
		if(!cnode){
		   fprintf(stderr,"%d/%d stack\n",numOfPath,curidx);
		   for(i = 0;i<pathEtiQidx;i++)
		   fprintf(stderr,"%d : (%08x:%08x) : %08x\n",
               i,pathEtiQ[i].autoNo,pathEtiQ[i].etatNo,pathEtiQ[i].eti);
		   
		   CqueuePathPr(stderr,numOfPath);
           exitMessage("eu~ak");
        }
        if(setflag){
		flag = cnode->flag;
		if(!cnode->flag){
			cnode->flag = 1;
		}
		}
//fwprintf(stdout,L"out %d\n",cnode->index);	 
		u_sprintf(aa,"Loc%d",cnode->index);
		return((unichar *)aa);
		
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
			if(!(*cnode)) exitMessage(StrMemLack);
			(*cnode)->next = 0;
			(*cnode)->cycInfos = 0;
			(*cnode)->index = cycNodeCnt++;
			(*cnode)->autoNo = cycEtatAutoNo;
			(*cnode)->etatNo = cycEtatNo;
			(*cnode)->eti = cycEtatEti;
			(*cnode)->flag = 0;
		}
		
		struct cyclePathMark *pCyc = getLoopId(offset,cntNode);
		struct cycleNodeId::linkCycle **a = &((*cnode)->cycInfos);
		while(*a){
			if(pCyc->index == (*a)->cyc->index) return;
			if((*a)->cyc->index < pCyc->index) break;
			a = &((*a)->next);
		}
		*a = new struct cycleNodeId::linkCycle;
		if(!(*a)) exitMessage(StrMemLack);
		(*a)->next = 0;
		(*a)->cyc = pCyc;

	}
	struct cyclePathMark *getLoopId(int offset,int noNode)
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
		if(!(*h)) exitMessage(StrMemLack);
		(*h)->pathEtiQueue = new struct pathAndEti [numOfPath];
		if(!((*h)->pathEtiQueue)) exitMessage(StrMemLack);
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
		struct cycleNodeId::linkCycle *inf,*tnf;
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
		for (i = 0; i < a->nombre_etats;i++){
			a->etat[i]->control &=0x7f;
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
    	            exitMessage("illegal execution");
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
	
	void outOneWord(unichar *suf){
		int i;
		int setOut;
		unichar *wp;
		EBuff[ePtrCnt]= TBuff[tPtrCnt] = 0;
		if(!ePtrCnt && !tPtrCnt && !suf) return;
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
            if(display_control ==  DEBUG)CqueuePathPr(foutput,depthDebug);
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
            printf("End by line limit %d\r\n", numberOfOutLine);
            exit(0);
       }

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
	void prOutCycle()
	{
		struct cyclePathMark *h = headCyc;
		int i;
		unichar *wp;
//        unichar *wwp;
		Fst2Tag Eti;		
		while(h){
		    if((foutput == stderr) || (foutput == stdout))
              fprintf(foutput,"C%d%s",h->index,getUtoChar(entreGO));
            else
             u_fprintf(foutput,"C%d%S",h->index,entreGO);
			ePtrCnt = tPtrCnt = 0;
			for(i = 0; i < h->pathCnt;i++){
//				putInt(0,h->pathEtiQueue[i].path);
				Eti = a->etiquette[h->pathEtiQueue[i].eti];
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
			outOneWord(entreGF);
			h = h->next;
		}
	}
	
	void prOutCycleAtNode(int autoNum,int nodeNum)
	{
		struct cyclePathMark *h = headCyc;
		int i,st,ed;
		int tmp;
		unichar *wp;
		Fst2Tag Eti;
		if(autoNum){
			st = a->debut_graphe_fst2[autoNum];
			ed =(autoNum  == a->nombre_graphes) ? a->nombre_etats :
					a->debut_graphe_fst2[autoNum+1];
		} else {
			st = nodeNum; ed = st+1;
		}

		while(h){
			tmp = h->pathEtiQueue[h->pathCnt -1].etatNo;
			if(tmp & FILE_PATH_MARK)
				tmp = a->debut_graphe_fst2[tmp & SUB_ID_MASK];
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
					tmp = a->debut_graphe_fst2[tmp & SUB_ID_MASK];
				if((tmp < st) || (tmp >= ed) )	break;

				Eti = a->etiquette[h->pathEtiQueue[i].eti];
				wp = (unichar *)	Eti->input;
				if(u_strcmp(wp,u_epsilon_string) && *wp){
					while(*wp)	EBuff[ePtrCnt++] = *wp++;
				}
				wp = (unichar *)Eti->output;
				if((automateMode == TRANMODE ) && wp && u_strcmp(wp,u_epsilon_string) && *wp ){
					while(*wp) TBuff[tPtrCnt++] = *wp++;
				}
			}
			if(i == h->pathCnt)
				outOneWord(u_null_string);
			else
				resetBuffs();
			h = h->next;
		}
	}
	//
	//
	//
	void prSubGrapheCycle()
	{
		int i;
		for(i = 0; i <= a->nombre_graphes;i++)
		{
			if(a->etat[a->debut_graphe_fst2[i]]->control & LOOP_NODE_MARK)
				fprintf(stderr,"the sub-graph %s has cycle path\n",
				  getUtoChar(a->nom_graphe[i]));
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
    		printf("too many igored sub-graph name igore%s\n",src);
    		return;
    	}
    	wp =new unichar [strlen(src)+1];
    	arretSubList[arretSubListIdx++] = wp;
    	while(*cp) *wp++ = (unichar)(*cp++ & 0xff);
    	*wp = (unichar)'\0';
    if(verboseMode) printf("IGNORE %s\n",getUtoChar(arretSubList[arretSubListIdx-1]));
    }
    char fileLine[1024];
     void arretExpoListFile(char *src)
    {
        int i;
        
        FILE *uf =fopen(src,"r");
        if(!uf){
         fprintf(stderr,"%s ",src);
         exitMessage("file access error");
         }
        while(fgets(fileLine,256,uf)){
//printf("list de arret %s\n",fileLine);
            if(fileLine[0] == ' ') continue;
            for(i  = 0; (i< 128) && (fileLine[i] != ' ') 
                    &&(fileLine[i] != '.') && (fileLine[i] != 0)
                    && (fileLine[i] != 0xa) && (fileLine[i] != 0xd) 
            ;i++) 
                        EBuff[i] = (unichar )(fileLine[i] & 0xff);
            EBuff[i++] = 0;
            
        	if(arretSubListIdx == MAX_IGONRE_SOUS_GRAPHE) {
        		printf("too many igored sub-graph name igore%s\n",src);
        		return;
        	}
        	arretSubList[arretSubListIdx] = new unichar [i];
        	for(i = 0;EBuff[i];i++) arretSubList[arretSubListIdx][i] = EBuff[i];
        	arretSubList[arretSubListIdx][i] = 0;
        	if(verboseMode) printf("IGNORE %s\n",getUtoChar(arretSubList[arretSubListIdx]));
    		arretSubListIdx++;
        }
        fclose(uf);
    }
    
     void arretExpoDel()
    {
    	for(int i = 0; i<arretSubListIdx;i++)
    		delete arretSubList[i];
    }
    //
//
//
void prAutoStack(int depStack)
{
    int i;
    FILE *ff = stdout;
    if((ff == stderr) || (ff == stdout))
        fprintf(ff,"===== AutoQueue\n");
    else
        u_fprintf(ff,"===== AutoQueue\n");
    for( i = 0; i <= CautoDepth;i++){
        if((ff == stderr) || (ff == stdout))
            fprintf(ff,"%d :: %d :: %d\n",i,CautoQueue[i].aId,CautoQueue[i].next);
        else
            u_fprintf(ff,"%d :: %d :: %d\n",i,CautoQueue[i].aId,CautoQueue[i].next);
    }
    if((ff == stderr) || (ff == stdout))
        fprintf(ff,"===== etatQueue\n");
    else
        u_fprintf(ff,"===== etatQueue\n");
    for(i = 0;i < pathEtiQidx;i++){
    if((ff == stderr) || (ff == stdout))
        fprintf(ff,"%d (%d ::%d)%d\n",i,pathEtiQ[i].autoNo,
            pathEtiQ[i].etatNo,pathEtiQ[i].eti);
    else
        u_fprintf(ff,"%d (%d ::%d)%d\n",i,pathEtiQ[i].autoNo,
            pathEtiQ[i].etatNo,pathEtiQ[i].eti);
    }
    if((ff == stderr) || (ff == stdout))
        fprintf(ff,"===== AutoStack\n");
    else
        u_fprintf(ff,"===== AutoStack\n");
    struct fst2Transition *k;
    for(i = 0;i < depStack;i++){
      k = autoStackMap[i].tran;
      if((ff == stderr) || (ff == stdout))
       fprintf(ff,"%d %d(%d ::%d)\n"
            ,i,autoStackMap[i].autoId,k->state_number,k->tag_number);
      else
       u_fprintf(ff,"%d %d(%d ::%d)\n"
            ,i,autoStackMap[i].autoId,k->state_number,k->tag_number);
    }
        
}
void prAutoStackOnly()
{
   for(int i = 0;i < pathEtiQidx;i++)
            u_fprintf(foutput,"%d (%d ::%d)%d\n",i,pathEtiQ[i].autoNo,
    pathEtiQ[i].etatNo,pathEtiQ[i].eti);
}

};    // end of fstApp


void CFstApp::loadGraph(char *fname)
{
	int i,j;
	struct fst2Transition *strans;
	
	a=load_fst2(fname,1);
	if (a==NULL) {
	  fprintf(stderr,"Cannot load graphe file %s\n",fname);
	  exit(1);
	}

	for( i = 0; i < a->nombre_etats;i++)
	{	
		strans = a->etat[i]->transitions;
		if(a->etat[i]->control & 0x80){
              exitMessage("Not vide control bit");
       }
		a->etat[i]->control &= 0x7f;	// clean for mark recusive
		while(strans){
			if(strans->tag_number < 0){
				strans->tag_number = 
					FILE_PATH_MARK | -strans->tag_number;
			}
			strans = strans->next;
		}
	}

	ignoreTable = new int [a->nombre_graphes+1];
    numOfIgnore = new int [a->nombre_graphes+1];
    for( i = 0 ; i<= a->nombre_graphes;i++){
    	ignoreTable[i] = 0;
    	numOfIgnore[i] = 0;
	}
	
	if(arretSubListIdx) {
	
		for(i = 0; i< arretSubListIdx;i++){
			for( j = 1; j <= a->nombre_graphes;j++){
				if(!u_strcmp((unichar *)a->nom_graphe[j],arretSubList[i]))
					break;
			}
			if( j > a->nombre_graphes){
				printf("Warning : Not exist the sub-graph %s\n",
					getUtoChar(arretSubList[i]));
				continue;
			}
			printf("%s %x graphe ignore the exploitation\n",
                     getUtoChar(a->nom_graphe[j]),j);
			ignoreTable[j] = 1;
		}
	}
	if(stopSignal){
	
		for( i = 0; i < a->nombre_etiquettes ;i++){
			if(u_strcmp((unichar *)a->etiquette[i]->input,stopSignal))
				continue;
			for(j = 0; j < a->nombre_etats;j++){
				strans = a->etat[j]->transitions;
				while(strans){
					if(strans->tag_number == i){
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
		for( i = 0; i < a->nombre_etiquettes ;i++){
			wp = (unichar *)a->etiquette[i]->input;
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
			wp = (unichar *)a->etiquette[i]->input;
			if(u_strcmp(wp,temp)){
			   printf("%0xth index, %s==>%s\n",i,
                        getUtoChar(wp),getUtoChar(temp));
            for(j = 0; temp[j];j++) *wp++ = temp[j];
			*wp=0;
			}
			
		}

	}
}
	char ofNameTmp[1024];
	char tmpchar[1024];
	char ttpchar[1024];
void CFstApp::getWordsFromGraph(char *fname)
{
	int i;
 	char *dp;
	// load fst2 file
	loadGraph(fname);
	
	CleanPathCounter();
    ofNameTmp[0] = 0;
	switch(display_control){
	case GRAPH:{
		listOut = 0;
        niveau_traite_mot =1;
		exploirerSubAuto(1);	// mark loop path start nodes
		prSubGrapheCycle();
		if(recursiveMode == LABEL) 
			fprintf(stderr,"warning:ignore the option -rl\r\n");
		recursiveMode = SYMBOL;
		strcpy(tmpchar,ofnameOnly);
		strcat(tmpchar,"autolst");
		makeOfileName(ofNameTmp,tmpchar,".txt");

		foutput = u_fopen(ofNameTmp,U_WRITE);
		if(!foutput) exitMessage("file open fail");
		
		listOut = 1;

		for( i = 1; i <= a->nombre_graphes;i++){
			u_fprintf(foutput,"[%d th automata %S]\n",i,a->nom_graphe[i]);
//			printf("[%d th automata %s]\n",i,getUtoChar(a->nom_graphe[i]));
			
			exploirerSubAuto(i);
			prOutCycleAtNode(i,0);
			u_fprintf(foutput,
		" the automate %S, %d path, %d path stopped by cycle, %d error path\n",
					a->nom_graphe[i],totalPath,totalLoop, errPath);
			CleanPathCounter();
		}

		if(recursiveMode == SYMBOL) prOutCycle();
		fclose(foutput);

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
				foutput = u_fopen(ofNameTmp,U_WRITE);
				if(!foutput) {
				    fprintf(stderr,"%s",ofNameTmp);
				    fflush(stderr);
                    exitMessage("file open fail");
                    }
				listOut = 1;
				exploirerSubAuto(1);
				if(verboseMode){
printf(" The automate %s : %d path, %d path stopped by cycle, %d error path\n"
            ,getUtoChar(a->nom_graphe[1])
            ,totalPath,totalLoop, errPath);
					if(stopPath){
					     for(int inx = 0; inx <= a->nombre_graphes;inx++){
                          if(numOfIgnore[inx]){
                              printf(" Sub call [%s] %d\n"
                                ,getUtoChar(a->nom_graphe[inx])
                                ,numOfIgnore[inx]);
                               numOfIgnore[inx] = 0;
                           }
                         }
					}
				}
 				if(recursiveMode == SYMBOL)	prOutCycle();
				
				fclose(foutput);
			}
			break;
		case MULTI:	// the first graph have only noms of the initials graphs
			{
			FILE *listFile;
			unichar *wp;
			struct fst2Transition *sui;
		strcpy(tmpchar,ofnameOnly);
		strcat(tmpchar,"lst");
			makeOfileName(ofNameTmp,tmpchar,".txt");
			if(!(listFile = fopen(ofNameTmp,"w")))
				exitMessage("list file open error");
			i = 0;

			for( sui = a->etat[0]->transitions;sui != 0 ; sui = sui->next){
				if(!(sui->tag_number & FILE_PATH_MARK))	continue;
				ignoreTable[sui->tag_number & SUB_ID_MASK] = 1;
				i++;
			}
			fprintf(listFile," %d\n",i);

			for( sui = a->etat[0]->transitions;sui != 0 ; sui = sui->next){
				if(!(sui->tag_number & FILE_PATH_MARK)) continue;
				cleanCyclePath();
				
				wp = (unichar *)a->nom_graphe[sui->tag_number & SUB_ID_MASK];
				dp = tmpchar;
				
				while(*wp){*dp++ = (char)(*wp&0xff);wp++;}
				*dp++ = '\0';
				makeOfileName(ofNameTmp,tmpchar,0);
                name_without_path(ofNameTmp,ttpchar);
				fprintf(listFile,"%s\r\n",ttpchar);


				if(!(foutput = u_fopen(ofNameTmp,U_WRITE))){
					fprintf(stderr,"%s",ofNameTmp);
					exitMessage("list file open fail");
				}
				listOut = 0;     // output disable
				
				exploirerSubAuto(sui->tag_number & SUB_ID_MASK);
				prSubGrapheCycle();			
				CleanPathCounter();				
				listOut = 1;    // output enable
				exploirerSubAuto(sui->tag_number & SUB_ID_MASK);
				
				if(recursiveMode == SYMBOL)	prOutCycle();
                if(verboseMode){
printf(" the automate %s %d path, %d path stopped by cycle, %d error path \n"
                    ,getUtoChar(a->nom_graphe[sui->tag_number & SUB_ID_MASK])
                    ,totalPath,totalLoop, errPath);
					
					if(stopPath){
					     for(int inx = 0; inx <= a->nombre_graphes;inx++){
                          if(numOfIgnore[inx]){
                              printf(" sub-call[%s] %d\n",
                                  getUtoChar(a->nom_graphe[inx]),
                                  numOfIgnore[inx]);
                              numOfIgnore[inx] = 0;
                           }
                         }
					}
				}

				fclose(foutput);
			}
			fclose(listFile);
			}
			break;
		}
	}
}
//
//
//
void CFstApp::exploirerSubAuto(int startAutoNo)
{
    struct fst2Transition startCallTr;
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
	pathEtiQ[pathEtiQidx].etatNo = a->debut_graphe_fst2[startAutoNo];
	pathEtiQ[pathEtiQidx].eti = 0;
	pathEtiQidx++;
	findCycleSubGraph(callSubId,1,a->debut_graphe_fst2[startAutoNo],0);
	pathEtiQidx--;
	if(pathEtiQidx)exitMessage("error in program");
}

//
//	find cycle path by node and call
//
void CFstApp::findCycleSubGraph(int automateNo,int autoDepth,int stateNo,int stateDepth)
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
				outWordsOfGraph(pathEtiQidx);
				pathEtiQidx--;
		} else
			fprintf(stderr,"warning:too many call");
		return;
	}
	if(IsCyclePath(stateDepth)){
		if(recursiveMode == STOP)	return;
//		if(!listOut){
//            a->etat[stateNo]->controle |= LOOP_NODE_MARK;
//		}
		return;
	}

	if( a->etat[stateNo]->control & FST2_FINAL_STATE_BIT_MASK) {	// terminal node 
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
			if( i == 0) exitMessage("not want state arrive");	

			int tauto = CautoQueue[i].aId;
			nEtat = CautoQueue[i].next;
			CautoDepth++;
			CautoQueue[CautoDepth].aId = -1;
			CautoQueue[CautoDepth].next = 0;
			
			pathEtiQ[pathEtiQidx].etatNo = nEtat;
			pathEtiQ[pathEtiQidx].eti = 0;
			pathEtiQ[pathEtiQidx].autoNo = tauto;
			pathEtiQidx++;
			findCycleSubGraph(tauto,autoDepth-1,nEtat,stateDepth+1);
			pathEtiQidx--;
			CautoDepth--;
		} else {	// stop condition
			if(listOut){
				totalPath++;
				pathEtiQ[pathEtiQidx].etatNo = STOP_PATH_MARK;
				pathEtiQ[pathEtiQidx].eti = 0;
				pathEtiQ[pathEtiQidx].autoNo = automateNo;
				pathEtiQidx++;
				outWordsOfGraph(pathEtiQidx);
				pathEtiQidx--;
			} else {
			   
			}
		}
	}
	for(struct fst2Transition *sui = a->etat[stateNo]->transitions;
	sui != 0 ; sui = sui->next){

		if(sui->tag_number & STOP_PATH_MARK){
			if(listOut){
				totalPath++;
				pathEtiQ[pathEtiQidx].autoNo = automateNo;
				pathEtiQ[pathEtiQidx].etatNo = STOP_PATH_MARK;
				pathEtiQ[pathEtiQidx].eti = sui->tag_number 
					& ~STOP_PATH_MARK;
				pathEtiQidx++;
				outWordsOfGraph(pathEtiQidx);
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
    			findCycleSubGraph(automateNo,autoDepth,sui->state_number,stateDepth+1);
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
					outWordsOfGraph(pathEtiQidx);
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
              pathEtiQ[pathEtiQidx].etatNo = a->debut_graphe_fst2[tmp]| LOOP_PATH_MARK;;
              ++pathEtiQidx;
              if(!IsCyclePath(stateDepth)) exitMessage("recusive find fail");
              --pathEtiQidx;
              continue;
            }
            pathEtiQ[pathEtiQidx].eti = 0;
            pathEtiQ[pathEtiQidx].autoNo = callId;
            pathEtiQ[pathEtiQidx].etatNo = a->debut_graphe_fst2[tmp];
            ++pathEtiQidx;
            autoStackMap[autoDepth].autoId = callId;
            
            CautoDepth++;
			CautoQueue[CautoDepth].aId = callId;
			CautoQueue[CautoDepth].next = sui->state_number;
			findCycleSubGraph(callId,autoDepth+1,
                  a->debut_graphe_fst2[tmp],stateDepth+1);
			--pathEtiQidx;
			--CautoDepth;
			continue;
		}
		pathEtiQ[pathEtiQidx].etatNo = sui->state_number;
		pathEtiQ[pathEtiQidx].eti = sui->tag_number;
		pathEtiQ[pathEtiQidx].autoNo = automateNo;
		++pathEtiQidx;
		findCycleSubGraph(automateNo,autoDepth,sui->state_number,stateDepth+1);
		pathEtiQidx--;
	}
}


//
//	for debugging, display all stack
//
void CFstApp::CqueuePathPr(FILE *f,int depth)
{
	int pidx = -1;
	int i;
	u_fprintf(f,"#");
	for(i = 0; i < pathEtiQidx;i++)
	{
		if(pathEtiQ[i].autoNo != pidx){ // skip the same value
			pidx = pathEtiQ[i].autoNo;
			u_fprintf(f,"%S>",a->nom_graphe[pidx]);
		}
	}
}


unichar *
uascToNum(unichar *uasc,int *val)
{
	unichar *wp = uasc;
	int base = 10;
	int sum = 0;
	if((*wp == (unichar)'0') && 
		(*(wp+1)==(unichar)'x') || 
		(*(wp+1) == (unichar)'X'))
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


void CFstApp::outWordsOfGraph(int depth)
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
				(unichar *)a->nom_graphe[pathEtiQ[s].eti & SUB_ID_MASK]:u_null_string;
			tp = u_null_string;
		} else {
			Eti = a->etiquette[pathEtiQ[s].eti & SUB_ID_MASK]; 
			ep = (u_strcmp((unichar *)Eti->input,u_epsilon_string)) ? 
				(unichar *)Eti->input : u_null_string;

            if((int)(Eti->output)){
			tp = (u_strcmp((unichar *)Eti->output,u_epsilon_string)) ? 
				(unichar *)Eti->output:u_null_string;
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
                    if(tPtrCnt) outOneWord(0);
                    if(markPreCtlChar && *ep) outOneWord(0);
           			while(*ep) EBuff[ePtrCnt++] = *ep++;
           			if(automateMode == TRANMODE)
                         while(*tp) TBuff[tPtrCnt++] = *tp++;
        
            		if(niveau_traite_mot) {
            			if(ePtrCnt|| tPtrCnt) outOneWord(0);            
            		} 
//                    else {		    
//            			if(control_char) outOneWord(0);
//            		}

				}
				if(pathEtiQ[s].etatNo & STOP_PATH_MARK){
//fwprintf(stdout,L"<%d:%d>",pathEtiQidx,s);
				    sp = getLabelNumber(depth,indicateFirstUsed,s,0);
//fwprintf(foutput,L"[%d]",indicateFirstUsed);
					outOneWord(sp);
					break;
				}
//fwprintf(stdout,L"<%d:%d>",pathEtiQidx,s);
				sp = getLabelNumber(s,indicateFirstUsed,s,1);
//fwprintf(foutput,L"[%d]",indicateFirstUsed);
				if(!indicateFirstUsed){	// first print out
					outOneWord(sp);
				}	else {
					resetBuffs();
				}
				while(*sp)	EOUTLINE[EOutCnt++] = *sp++;
				wp = entreGF;
                while(*wp) EOUTLINE[EOutCnt++] = *wp++;
                markPreCtlChar = markCtlChar;
				continue;
			} else if(recursiveMode == SYMBOL) {	// SYMBOL
				if(tPtrCnt && niveau_traite_mot) outOneWord(0);

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
				outOneWord(0);
				continue;
			} else { // STOP
                outOneWord(0);
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
    				outOneWord(u_null_string);
		        } else {
            		if(niveau_traite_mot) {
            			if(ePtrCnt|| tPtrCnt) outOneWord(0);        
            		} else {		    
            			if(markPreCtlChar) outOneWord(0);
            		}		        
                }
                markPreCtlChar = markCtlChar; 
				continue;		
			}
		}

		if(pathEtiQ[s].etatNo & STOP_PATH_MARK) {
//printf("stop %d\n",s);
            if(markPreCtlChar && markCtlChar) outOneWord(0);
			if((automateMode == TRANMODE)&& *tp){ // current 
			    if(tPtrCnt) outOneWord(0);
				while(*tp) TBuff[tPtrCnt++] = *tp++;				
			}
			if(pathEtiQ[s].eti & FILE_PATH_MARK)
				outOneWord((unichar *)a->nom_graphe[pathEtiQ[s].eti & SUB_ID_MASK]);
			else
                outOneWord(u_null_string);
			break;
		}				
		if(pathEtiQ[s].eti & FILE_PATH_MARK){
		    
			if(tPtrCnt ||(markPreCtlChar && *ep)) outOneWord(0);
			
			switch(display_control){
			case GRAPH:
				EBuff[ePtrCnt++] = (unichar)'{';
				while(*ep)	EBuff[ePtrCnt++] = *ep++;
				EBuff[ePtrCnt++] = (unichar)'}';
                outOneWord(0);
               break;
			case DEBUG:
			case FULL:
				exitMessage("???");
			}
			markPreCtlChar = markCtlChar;
			continue;
		}
		
// make a pair of (entre, sorti)
        if( (*ep == 0)  && ( *tp==0) ){
                continue;
          }
        
        if(tPtrCnt || (markPreCtlChar && *ep)) outOneWord(0);
        while(*ep) EBuff[ePtrCnt++] = *ep++;
        if(automateMode == TRANMODE)
			while(*tp) TBuff[tPtrCnt++] = *tp++;        
		if(niveau_traite_mot) {
			if(ePtrCnt|| tPtrCnt) outOneWord(0);
		}
//         else {		    
//			if(control_char) outOneWord(0);
//		}
		markPreCtlChar = markCtlChar;
	}
}

//
//
//
//


int main(int argc, char **argv) {
setBufferMode();
    
	CFstApp aa;
	char *ofilename =0;
	int iargIndex = 1;
	int i;


	changeStrToIdx = 0;

	char *wp;
	unichar *wp2,*wp3;
	while(iargIndex < argc){
//fprintf(stderr,"%s\n",argv[iargIndex]);
		if(*argv[iargIndex] != '-') break;
		switch(argv[iargIndex][1]){
		case 'f': iargIndex++;
			switch(argv[iargIndex][0]){
			case 's': aa.prMode = PR_SEPARATION; break;
			case 'a': aa.prMode = PR_TOGETHER;break;
			default: usage();
			}
			break;
		case 'o':iargIndex++; 
//fprintf(stderr,"%s\n",argv[iargIndex]);
			ofilename = new char [strlen(argv[iargIndex])+1];
			strcpy(ofilename,argv[iargIndex]);
			break;
		case 'l': iargIndex++;
//fprintf(stderr,"%s\n",argv[iargIndex]);
			aa.outLineLimit = atoi(argv[iargIndex]);
			break;
		case 'i': iargIndex++;	// stop the exploitation
//fprintf(stderr,"%s\n",argv[iargIndex]);
			aa.arretExpoList(argv[iargIndex]);
			break;
		case 'I': iargIndex++;	// stop the exploitation
//fprintf(stderr,"%s\n",argv[iargIndex]);
			aa.arretExpoListFile(argv[iargIndex]);
			break;
		case 'p':			iargIndex++;
//fprintf(stderr,"%s\n",argv[iargIndex]);
			switch(argv[iargIndex][0]){
			case 's': aa.display_control = GRAPH; break;
			case 'f': aa.display_control = FULL; break;
			case 'd': aa.display_control = DEBUG; break;
			default: usage();
			}
			break;
		case 'a':
		case 't':
			aa.automateMode = (argv[iargIndex][1] == 't') 
				? TRANMODE:AUTOMODE;
			iargIndex++;
//fprintf(stderr,"%s\n",argv[iargIndex]);
			switch(argv[iargIndex][0]){
			case 's': aa.traitAuto = SINGLE; break;
			case 'm': aa.traitAuto = MULTI; break;
			default: usage();
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
			}
			iargIndex++;
//fprintf(stderr,"%s\n",argv[iargIndex]);
		
			aa.saveEntre = new unichar[strlen(argv[iargIndex])+1];
			wp =  argv[iargIndex];
			wp2 = aa.saveEntre;
			wp3 = 0;
			while(*wp){
				if( (*wp < 0x20) ||	(*wp > 0x7e))
				{
					fprintf(stderr,"Warning:use a separator in ASC code\r\n");
					usage();
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
//fprintf(stderr,"%s\n",argv[iargIndex]);
			if(!changeStrToVal(argv[iargIndex])) break;
			usage();
		case 's':{
			i = 0;
			char cc= argv[iargIndex][2];
			iargIndex++;
//fprintf(stderr,"%s\n",argv[iargIndex]);
			wp = argv[iargIndex];
			wp3 = 0;
			switch(cc){
			case 0  :
				wp2 = aa.sep1 = new unichar [strlen(wp)+1];
				wp3 = 0;
				while(*wp){
					if( (*wp < 0x20) ||	(*wp > 0x7e))
					{
						fprintf(stderr,"Warning:use a separator in ASCII code\r\n");
						usage();
					}
					switch(*wp){
					case '\\':wp++;
                          if(*wp == '\0') usage();
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
						fprintf(stderr,"Warning:use a separator in ASC code\r\n");
						usage();
					}
					if(*wp == '\\'){
                          wp++;
                          if(*wp == '\0') usage();
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
			default:
				usage();
			}
			
				

		}
			break;
		default:
			usage();
		}
		iargIndex++;
	}
//printf("langue change %s\n",setlocale(LC_ALL,"Korean_Korea.949"));
	if(iargIndex != (argc -1 )) usage();
	aa.fileNameSet(argv[iargIndex],ofilename);
	aa.getWordsFromGraph(argv[iargIndex]);
	if(ofilename) delete ofilename;

	return 0;
}

