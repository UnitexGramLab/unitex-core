 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef STATE_MACHINE_FST2_H
#define STATE_MACHINE_FST2_H

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "Unicode.h"
#include "Fst2.h"
#include "bitmap.h"
#include "etc.h"
#include "Error.h"
#include "Transitions.h"


//
//
// state macheine used by fst2
//


#define SZ64K	64*1024
#define CNT_SZ_MAX_ARRAY	256
#define SZ64K	64*1024

#define FLOW_CONTROL_INIT    0x1
#define FLOW_CONTROL_FINI   0x2

class state_machine {
    unichar *terSymbol;
    unsigned int Null_intValue;
struct link 
{
	unsigned int val;
	struct link *next;
};
	Fst2* a;
	int controlFlag;
public:
	state_machine(){
	    int i;
		changeStrToIdx = 0;
		a = 0;
		terSymbol = u_null_string;
		Null_intValue = 0;
		HeadVari = 0;
		HeadStacks = 0;
		smemIdx = 0;
		dmemIdx = 0;
		for(int i=0; i< 2048;i++){
			IncludeMap[i] = 0;
		}
		for(i = 0; i< CNT_SZ_MAX_ARRAY;i++)
			smemArray[i] = dmemArray[i]= 0;
		outCnt = 0;
	
	};
	~state_machine(){
		struct link *sp,*ntp;
		sp  = HeadVari;
		while(sp){
			ntp = sp->next;
			delete (unichar *)sp->val;
			delete sp;
			sp  = ntp;
		}
		if(saveTransductionTable){
			for( int i = 1; i < a->number_of_tags;i++)
				if(saveTransductionTable[i])
					a->tags[i]->output=(unichar*)saveTransductionTable[i];
			delete saveTransductionTable;
		}
		if(a)free_Fst2(a);

	};
	int isReady(){ return( a ? 1:0);};
	//
	//	load fst2 file and set variables
	//
	void init_machine(char *mapfilename,int sz)
	{
		int i;
		unichar wt;
		unichar *wp;

		a=load_fst2(mapfilename,1);
		if (a==NULL) {
		   fatal_error("Cannot load %s in 'init_machine'\n",mapfilename);
		}
		outSize = sz;
		outCnt = 0;
	
		saveTransductionTable = new unichar *[a->number_of_tags];
		for(i = 1; i < a->number_of_tags;i++){
			saveTransductionTable[i] = 0;
		}	

		for(i = 1; i < a->number_of_tags;i++){
			saveTransductionTable[i]=a->tags[i]->output;
			if(a->tags[i]->output){
//		wprintf(L" %d %s\n",i,a->etiquette[i]->transduction);
			 a->tags[i]->output=(unichar*)ajouteTransValue(a->tags[i]->output);
			}
			wp=a->tags[i]->input;
	 		if(*wp == '<'){
	           if( (*(wp+1) ==  'E') && (*(wp+2) ==  '>'))
	          	 fatal_error("do not accept the transition with null\n");
		       if((*(wp+1) == '!') && (*(wp+2) ==  '>') )
                 continue;  		
		       if((*(wp+1) == '#') && (*(wp+2) == '>') )
         		  *wp = (unichar)*terSymbol;
               else if(findChangeStr(wp,&wt)){
				    *wp=(unichar)wt;
		       } else {
				     fatal_error("un define %s\n",getUtoChar(a->tags[i]->input));
		       }
	         }
		//
		// distingue between element et non element
		//
		      wt = *wp;
             IncludeMap[wt/32] |= bitSetL[wt%32];
            
       }
	}
	
	void cleanMachine(){
		curEtat = 0;
		cleardmem();
	};
	void cleardmem()
	{
		for(int i=0; i< dmemIdx;i++)
			dmemArray[i] = 0;
	};
	struct cmdInst {
		int op;
		unsigned int *op1;
		unsigned int *op2;
		unsigned int *src;
		struct cmdInst *next;
	};
	unsigned uByte[CNT_SZ_MAX_ARRAY];
	unsigned uWord[CNT_SZ_MAX_ARRAY];
	unsigned uLong[CNT_SZ_MAX_ARRAY];
	int outSize;
	int outCnt;
	unichar **saveTransductionTable;


	int curEtat;
	unsigned int IncludeMap[2048]; // bit 
	struct link *HeadVari;
	struct link *HeadStacks;
	struct link *memArray;
	struct link **ss;

	int smemArray[CNT_SZ_MAX_ARRAY];
	int smemIdx;
	int dmemArray[CNT_SZ_MAX_ARRAY];
	int dmemIdx;
#define MARK_CMD	0x80000000
#define MARK_DMEM	0x20000000
#define MARK_SMEM	0x40000000
#define MASK_ID		0x0fffffff
#define MASK_FUNC	0xf0000000
#define CMD_ADD	1
#define CMD_MIN	2
#define CMD_MUL	3
#define CMD_DIV 4
#define CMD_INIT	5
#define CMD_COUT	6
#define CMD_FINI	7

#define CHECK_MEM_INDEX	0

	void traiteAct(struct cmdInst *map)
	{
	
		while(map){
if(debugPrFlag){	
//wprintf(L"<%x,",map->op);
switch(map->op){
case CMD_ADD:
case CMD_MIN:
case CMD_MUL:
case CMD_DIV:;
//wprintf(L"%x(0x%x),",map->op1,*(map->op1));
//wprintf(L"%x(0x%x),",map->op2,*(map->op2));
//wprintf(L"%x(0x%x)=>",map->src,*(map->src));
}
}



		switch(map->op){
		case CMD_ADD:
			*(map->src) = *(map->op1) + *(map->op2); break;
		case CMD_MIN:
			(*map->src) = *(map->op1) - *(map->op2); break;
		case CMD_MUL:
			(*map->src) = *(map->op1) * *(map->op2); break;
		case CMD_DIV:
			(*map->src) = *(map->op1) / *(map->op2); break;
		case CMD_COUT:outValue();break;
		case CMD_INIT:cleardmem();controlFlag = FLOW_CONTROL_INIT; break;
		case CMD_FINI:cleardmem();controlFlag = FLOW_CONTROL_FINI; break;
		default:
			fatal_error("syntax error\n");
		}
if(debugPrFlag){
switch(map->op){
case CMD_ADD:
case CMD_MIN:
case CMD_MUL:
case CMD_DIV:;
//printf("0x%x",*(map->src));
}
//printf(">\n");
}
		map = map->next;
		}

	}
	void outValue()
	{
		switch(outSize){
		case 1:	uByte[outCnt++] = (unsigned char)(dmemArray[0] & 0xff); break;
		case 2:	uWord[outCnt++] = (unichar)(dmemArray[0] & 0xffff);break;
		case 4:	uLong[outCnt++] = (unsigned int)dmemArray[0];break;
		}
		if(outCnt == CNT_SZ_MAX_ARRAY){
			outCnt = CNT_SZ_MAX_ARRAY;
			error("warning buffer full");
		}
	};

void curSMvalue(unichar cval)
{
	Transition *cT = a->states[curEtat]->transitions;
	struct cmdInst *cmdPtr;
	unichar *wp;
	int nextArr = 0;

	while(cT){
        wp = (unichar *)a->tags[cT->tag_number]->input;
        if(*wp == '<'){
           wp++;
           if(*wp == '!'){
		    nextArr = cT->state_number;
		    cmdPtr = (struct cmdInst *)a->tags[cT->tag_number]->output;
           }
        } else if(*wp == cval){
		    nextArr = cT->state_number;
		    cmdPtr = (struct cmdInst *)a->tags[cT->tag_number]->output;
		    break;
		}
		cT = cT->next;
	}
	curEtat = 0;
	if(nextArr){
	    controlFlag = 0;
		traiteAct(cmdPtr);

		if(controlFlag & FLOW_CONTROL_INIT){
			curSMvalue(cval);
			return;
		} else if(controlFlag & FLOW_CONTROL_FINI){
			return;
		}
		curEtat = nextArr;
		return;
	}
cleardmem();
}



#define LOCAL_STACK_MAX	256
unsigned int *
ajouteTransValue(unichar *istr)
{
	unsigned int strSz;
	unichar valName[16];
	unichar saveCmdLine[64];
	int ii;

	unichar *wp,*sp= istr ;
	int i;
	struct cmdInst *lhead =0;
	struct cmdInst **p;
	struct cmdInst *tcmd;
	unsigned int lstack[16];
	int lstackIdx = 0;
	int sum;
//printf("%s\n",getUtoChar(istr));
	
do {
	i =0;
	while(*sp) {
		if(*sp == (unichar)',') break;
		if((*sp == (unichar)' ') || (*sp == (unichar)'\t')) continue;
		saveCmdLine[i++] = *sp++;
		if(i < 64 ) continue;
		fatal_error("too long line %s\n",istr);
	}
	if(!*sp &&(!i) ) break;
	if(*sp) sp++;
	saveCmdLine[i]= '\0';

	wp = saveCmdLine;
	if(!u_strcmp(wp,"INIT")){
		tcmd = new struct cmdInst;
		tcmd->op = CMD_INIT;
		tcmd->next = 0;
	}else if(!u_strcmp(wp,"COUT")){
		tcmd = new struct cmdInst;
		tcmd->op = CMD_COUT;
		tcmd->next = 0;
	}else if(!u_strcmp(wp,"FINI")){
		tcmd = new struct cmdInst;
		tcmd->op = CMD_FINI;
		tcmd->next = 0;
	}else if(!u_strcmp(wp,"<E>")){
		tcmd = 0;
	}else if(!*wp){
		tcmd = 0;	
	} else {
		int endFlag = 1;
		do {
		switch(*wp){
		case '%':
				strSz = 0;
				wp++;
				while(*wp != '%'){
					if(!*wp) fatal_error("syntax error\n");
					valName[strSz++] = *wp++;
					if(strSz == 16) fatal_error("Too long name\n");
				}
				wp++;
				valName[strSz] = '\0';
				if(!strSz) fatal_error("synTax error\n");

				if(u_strcmp(valName,"ret")){
					ss = &HeadVari;
					i = 1;
					while(*ss){
						if(!u_strcmp((unichar *)(*ss)->val,valName))
							break;
						i++;
						ss = &(*ss)->next;
					}
					if(!*ss){
						*ss = new struct link;
						(*ss)->val = (int)new unichar[strSz+1];
						u_strcpy((unichar *)(*ss)->val,valName);
						(*ss)->next = 0;
					}
				} else 
					i = 0;
				lstack[lstackIdx++] = (unsigned int)&dmemArray[i];
				break;
			case '+':
			case '-':
			case '*':
			case '/':
			case '=':
				lstack[lstackIdx++] = MARK_CMD | *wp++; break;
			case ',':
				fatal_error("syntax error\n");
			case '\0':
				tcmd = new 	struct cmdInst ;
				tcmd->src = (unsigned int *)lstack[0];
				tcmd->op1 = (unsigned int *)lstack[2];
				if(lstackIdx == 5){
					switch(lstack[3] & MASK_ID){
					case (unichar)'+': tcmd->op = CMD_ADD;break;
					case (unichar)'-': tcmd->op = CMD_MIN;break;
					case (unichar)'*': tcmd->op = CMD_MUL;break;
					case (unichar)'/': tcmd->op = CMD_DIV;break;
					default: fatal_error("syntax error\n");
					}
					tcmd->op2 = (unsigned int *) lstack[4];
				} else if(lstackIdx == 3){
					tcmd->op = CMD_ADD;
					tcmd->op2 = &Null_intValue;
				} else {
					fatal_error("syntax error\n");
				}
			
	
				endFlag = 0;
				break;
			case (unichar)' ':
			case (unichar)'\t': wp++; break;
			default:
				if(lstackIdx < 2) fatal_error("syntax error\n");
				wp = uascToNum(wp,&sum);

				for(ii=0;(ii< smemIdx) && (ii<CNT_SZ_MAX_ARRAY);ii++)
                        if( smemArray[ii] == sum)
                         break;
				if(ii == smemIdx) // registe new constant variable
					smemArray[smemIdx++] = sum;
				if(smemIdx >= CNT_SZ_MAX_ARRAY) fatal_error("too many constants\n");
				lstack[lstackIdx++] = (unsigned int)&smemArray[ii];
			}
			if(lstackIdx >5) fatal_error("too much operations\n");
		} while(endFlag);
	}
	if(tcmd){	// ajoute cmd link
		p = &lhead;
		while(*p){
			p = &(*p)->next;
		}
		(*p) = tcmd;
		tcmd->next = 0;
		lstackIdx = 0;
if(debugPrFlag) {
//printf("%x,",tcmd->op);
//printf("%x,",tcmd->op1);
//printf("%x,",tcmd->op2);
//printf("%x\n",tcmd->src);
}
		}
	}	while(*sp);
	return((unsigned int *)lhead);
}

	int convStr(unichar *iBuff,int sz,unichar *obuff)
	{
		int isPreviousJamo =0;	// flag indicate previous character
		int offset;
		int outputCnt = 0;
		int i;
		unichar wt;
		for( offset = 0; offset < sz;offset++){
			wt = iBuff[offset];
			if(IncludeMap[wt/32] & bitSetL[wt%32]){
				if(!isPreviousJamo) cleanMachine();
				curSMvalue(wt);
				for( i = 0; i < outCnt;i++)	obuff[outputCnt++] = uWord[i];
				outCnt = 0;
				isPreviousJamo = 1;
			} else {
				if(isPreviousJamo){
					curSMvalue(*terSymbol);
					for( i = 0; i < outCnt;i++)	obuff[outputCnt++] = uWord[i];
					outCnt = 0;
				}
				obuff[outputCnt++] = wt;
				isPreviousJamo = 0;
			}
		}
		obuff[outputCnt] = 0;
		return(outputCnt);
	}
	int convJamoStrToSyl(unichar *wt,unichar *obuff)
	{
		int isPreviousJamo =0;	// flag indicate previous character
		int outputCnt = 0;
		int i;

		while(*wt){
			if(IncludeMap[*wt/32] & bitSetL[*wt%32]){
				if(!isPreviousJamo) cleanMachine();
					curSMvalue(*wt);
				for( i = 0; i < outCnt;i++)
					obuff[outputCnt++] = uWord[i];
				outCnt = 0;
				isPreviousJamo = 1;
			} else {
				if(isPreviousJamo){
					curSMvalue(*terSymbol);
					for( i = 0; i < outCnt;i++)
						obuff[outputCnt++] = uWord[i];
					outCnt = 0;
				}
				obuff[outputCnt++] = *wt;
				isPreviousJamo = 0;
			}
			wt++;
		}
		if(isPreviousJamo){
		curSMvalue(*terSymbol);
		for( i = 0; i < outCnt;i++)
				obuff[outputCnt++] = uWord[i];
		outCnt = 0;
		}
		obuff[outputCnt] = 0;
		return(outputCnt);
	}
	int convFile(char *ifname,char *ofname)
	{
		U_FILE *fi,*fo;
		fi = u_fopen(UTF16_LE,ifname,U_READ);
		fo = u_fopen(UTF16_LE,ofname,U_WRITE);
		if(!fi || !fo) return(0);
	//
	//	scan for get the order of character
	//
		if(!a) return(0);
		unichar iBuff[SZ64K];
		unichar oBuff[SZ64K];
		cleanMachine();
		int rsz,osz;
		int totalRead = 0;
	
		do { 
//			if((rsz = fread(iBuff,2,SZ64K,fi)) > 0){
			if((rsz = u_fread_raw(iBuff,SZ64K,fi)) > 0){
				totalRead+= rsz;
				osz = convStr(iBuff,rsz,oBuff);
                u_fwrite_raw(oBuff,osz,fo);//fwrite(oBuff,2*osz,1,fo);
			}
			u_printf("\rTotal  %d byte read",totalRead*2);
		} while(rsz == SZ64K);
		u_printf("\n");
		u_fclose(fi);
		u_fclose(fo);
		return(totalRead);
	}
};
#endif	// STATE_MACHINE_FST2_H
