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

#include <stdlib.h>

using namespace std;

#include "Unicode.h"
#include "etc.h"
//#include "String_hash2.h"
#include "bin.h"
#include "bin3.h"
#include "Copyright.h"
#include "IOBuffer.h"




static void usage()
{
u_printf("%S",COPYRIGHT);
u_printf(
"MergeBin [-d] [-l lfilename] -o ofilename f0 f1 ... fn ] ] \n"\
"         : merge files which has the extension .bin \n"\
"  -d : debugMode set. all paths out to the file \"out.txt\"\n"\
"  -l lfilename : get files from list file\"\n"\
"  -o ofilename : set the output file name,at input files from command line\"\n"\
);
}

struct binFileList {
	char *fname;
	class explore_bin1  image;
	int *newSufId;
	struct binFileList *next;
	int relocateInfOffset;
	int relocateBinOffset;
	int relocateInfCnt;
};


static int INF_offset;
static int REF_offset;
static int BIN_offset;
static int racTableIdx;
static int sufTableIdx;
static int infTableIdx;
class arbre_string00 totSuf;
class arbre_string0 unresolSuf;	// not resoled suffixe
char *nameOfoutput;
struct racTable {
	unsigned short *s;
	int pos;
	int offset;
};
#ifdef DEBUG_MER
static int dMode;
#endif
static void initVar()
{
INF_offset = 0;
REF_offset =0;
BIN_offset =0;
racTableIdx =0 ;
sufTableIdx =0;
infTableIdx =0;
unresolSuf.put(assignUstring(u_epsilon_string));
#ifdef DEBUG_MER
dMode = 0;
#endif
}

static		char path_of_locate[1024];
static		char buff[1024];
static		int racStateCounter;
static 		int fileListCounter;

static struct binFileList *tailFile,*headFiles;
static void
getOneFile(char *fn)
{
	struct binFileList *tmp;	
    tmp = new struct binFileList;
	tmp->fname = new char[strlen(fn)+1];
	tmp->next = 0;
	tmp->newSufId = 0;
	strcpy(tmp->fname,fn);
	if(headFiles){
        tailFile->next = tmp;
        tailFile = tailFile->next;
	} else {
        tailFile = headFiles= tmp;
  	}
  	u_printf("Load file name :: <%s>\n",fn);
	fileListCounter++;
}
static void getListFile(char *fn)
{
	register char *wp;
	char pathName[1024];
	int pathLen;
struct binFileList *tmp;
	U_FILE *lstF;
	u_printf("Load file %s\n",fn);
    if(!(lstF = u_fopen(BINARY,fn,U_READ)))
    	fopenErrMessage(fn);	
    get_path(fn,pathName);
    pathLen = strlen(pathName);
    while(fgets(buff,1024,lstF->f->fin)){
    	wp = buff;

    	while(*wp){
    		if(*wp == 0x0d){ *wp = 0; break;}
    		if(*wp == 0x0a){ *wp = 0; break;}
    		wp++;
    	}
    	
    	switch(buff[0]){
    	case ' ':	// comment line
    	case 0:
    		continue;
    	}

    	tmp = new struct binFileList;
    	tmp->fname = new char[pathLen+strlen(buff)+1];
    	strcpy(tmp->fname,pathName);
    	strcat(tmp->fname,buff);
    	tmp->next = 0;
    	tmp->newSufId = 0;
    	if(headFiles){	
           tailFile->next = tmp; 
           tailFile = tailFile->next;
    	} else {
            tailFile = headFiles= tmp;	}
    	fileListCounter++;
    }
    u_fclose(lstF);
}

static void 
load_bins(char *oFileName)
{
	path_of_locate[0] = 0;
	struct binFileList *tmp;
	struct binFileList *rhead,*rtail,*shead,*stail;


	racStateCounter = 0;
	rhead = shead = 0; 
    rtail = stail = 0;
	tmp = headFiles;	
	while(tmp){
      u_printf("%s\n",tmp->fname);
		tmp->image.loadBin(tmp->fname);
		tmp->newSufId = 0;
		
		if(tmp->image.isRacine()){
			racStateCounter++;
         u_printf("is racine %s\n",tmp->fname);
  			if(rhead){	
                rtail->next = tmp; 
                rtail = rtail->next;
			} else {
                rtail =rhead= tmp;
            }
            tmp = tmp->next;
			rtail->next = 0;
		} else {
         u_printf("Load suffix %s\n",tmp->fname);
  			if(shead){	
                stail->next = tmp;
                stail = stail->next;
			} else {
                stail = shead= tmp;
            }
            tmp = tmp->next;
			stail->next = 0;
		}
	}
	
	headFiles = rhead;
	rtail->next = shead;
	if(!headFiles)	fatal_error("null file read\n");
	//
	//	calcule the decalage by add dummy node for racines
	//  in the image of the bin
	
	//
	//	get value of addresss relocatable  
	//
	int i =0;
	int nidx;
	int relBin,relInf;
	
	relBin = 0;
	relInf = 0;
	if(racStateCounter && (racStateCounter != 1)){
	    char charRootName[1024];
		relBin = racStateCounter * SIZE_ONE_TRANSITION_BIN + 2;
		remove_extension(oFileName,charRootName);
		unichar  *RootName = new unichar[strlen(charRootName)+1];
		u_strcpy(RootName,charRootName);
		totSuf.put((unichar *)RootName,0);
	}

	tmp = headFiles;
	while(tmp){
		tmp->relocateInfOffset = 0; // set value of relocated location
		tmp->relocateBinOffset = 0;
		
		for( i = 0; i <tmp->image.head.cnt_auto;i++){
		  u_printf("Initial state %s located at %d"
             ,getUtoChar(tmp->image.STR + tmp->image.AUT[i])
             ,tmp->image.autoffset[i]);
			tmp->image.autoffset[i] += relBin;
            u_printf("relocated at %d\n",tmp->image.autoffset[i]);
			nidx = totSuf.check(tmp->image.STR + tmp->image.AUT[i]);
			if(nidx != -1){
				fatal_error("at %s file,symbol %s:\ndupliate initial state name\n"
                 ,tmp->image.name
				 ,getUtoChar(tmp->image.STR + tmp->image.AUT[i]));
			}
			if(!nidx){
				fatal_error("at %s file,symbol\nnull initial state name\n",tmp->image.name);
			}
			nidx = totSuf.put(tmp->image.STR + tmp->image.AUT[i],
				  (void *)(tmp->image.autoffset[i]));
		}
		tmp->relocateInfOffset = relInf;
		tmp->relocateBinOffset = relBin;
		tmp->newSufId = new int [tmp->image.head.cnt_suf];
		for( i = 0; i < tmp->image.head.cnt_suf;i++)
			tmp->newSufId[i] = 0;
		relInf += tmp->image.head.size_inf;
		relBin += tmp->image.head.size_bin;		
		tmp = tmp->next;
	}
	int unresolveSufCnt = 0;
	//
	//	set the value of the position of jmping suffixe 
	//
	
	tmp = headFiles;
	while(tmp){
		for( i = 1; i < tmp->image.head.cnt_suf;i++){// i = 0 : null
			nidx = totSuf.check(tmp->image.STR + tmp->image.SUF[i]);
			if(!nidx || nidx == -1){
				nidx = unresolSuf.put(tmp->image.STR + tmp->image.SUF[i]);
				unresolveSufCnt++;
				u_printf("the suffix, %s is not located\n",
				  getUtoChar(tmp->image.STR + tmp->image.SUF[i]));
				tmp->newSufId[i] = nidx;
			} else {
				nidx = (int)totSuf.getCheckValue();
				tmp->image.sufoffset[i] = nidx;	
	            u_printf("the suffix, %s is set %d\n",
				  getUtoChar(tmp->image.STR + tmp->image.SUF[i]),nidx);
			}
		}
		tmp = tmp->next;
	}
	

}

static
void
mergeFiles(char *ofn,struct binFileList *first)
{
	struct binFileList *last;
	int i;
	class binHead0 newHead;
	int *offsetMap;
	int		offsetStrSave = 0;
	U_FILE *f;
		unsigned char *wp,*limitBin;
	unsigned short info,sinfo;
	int trCnt;
	unsigned int sufid;
	
	last = first;
	
	char ofilename[1024];
	remove_extension(ofn,ofilename);
	strcat(ofilename,".mtb");
	if(!(f = u_fopen(BINARY,ofilename,U_WRITE)))
		fopenErrMessage(ofilename);
		
	newHead.writeAtFile(f);
	newHead.cnt_auto = totSuf.size();
	newHead.cnt_suf = unresolSuf.size();

	//
	//	ref 

	//	racine name table
	last = first;
	while(last){
		newHead.size_bin += last->image.head.size_bin;
		newHead.size_inf += last->image.head.size_inf;
		last = last->next;
	}
	unichar **a = totSuf.make_strPtr_table(&offsetMap);
	unichar **b = unresolSuf.make_strPtr_table();

	
	for(i = 0; i < newHead.cnt_auto;i++)
	{
	  u_printf("%s %d\n",getUtoChar(a[i]+1),offsetMap[i]);
		outbytes3((unsigned int)offsetMap[i],f);		// offset of postition of state
	}
	
	int j;
	newHead.size_ref = newHead.cnt_auto * 3;
	offsetStrSave = 0;

	for(i = 0; i < newHead.cnt_auto;i++)
		{
			for(j = 1; j < a[i][0];j++)
				outbytes2((unsigned short)a[i][j],f);
			outbytes2(0,f);
			offsetStrSave += a[i][0];
		}

	totSuf.release_value();

	for(i = 0; i < newHead.cnt_suf;i++){
		for(j = 1; j < b[i][0];j++)
			outbytes2((unsigned short)b[i][j],f);
		outbytes2(0,f);
		offsetStrSave += b[i][0];
	}

	unresolSuf.release_value();
	newHead.size_str = offsetStrSave;
	
	//
	//	inf table
	//
	offsetStrSave = 0;
	int cntInf = 0;
//	unsigned short v;
	unichar *INF;
        unsigned char *sp;
	last = first;
	while(last){
		last->relocateInfCnt  = cntInf;
		if(offsetStrSave != last->relocateInfOffset)
			fatal_error("illegal value\n");
		INF = last->image.INF;
        sp = (unsigned char *)INF;
		for( i = 0; i < last->image.head.size_inf;i++){	// swap data
// printf("%x %x %x\n",v,*sp,*(sp+1));
            outbytes2(INF[i],f);
//            *sp++ =  (v >> 8 ) & 0xff;
//            *sp++ = v & 0xff;
		}

//		fwrite(last->image.INF,1,last->image.head.size_inf*2,f);
		
		cntInf += last->image.head.cnt_inf;
		offsetStrSave += last->image.head.size_inf;
		last = last->next;
	}
	newHead.size_inf = offsetStrSave;
	newHead.cnt_inf = cntInf;


	offsetStrSave = 0;
	if(racStateCounter && (racStateCounter != 1) ){ // dummy initial node
//printf("node  0 for racines");
		outbytes2((unsigned int)racStateCounter,f);
		last = first;
		while(last){
			if(last->image.isRacine()){
				outbytes2(0,f);                        // char 0
				outbytes2(0x0000,f);                   // info 0
				outbytes3(last->image.autoffset[0],f); // next
//printf("dummy node for racines %s 0x%x\n",last->image.name,last->image.autoffset[0]);
			}
			last = last->next;
		}
		offsetStrSave = 2 + 
			racStateCounter * SIZE_ONE_TRANSITION_BIN;
	}
	last = first;
	while(last){
		if(offsetStrSave != last->relocateBinOffset)
			fatal_error("illegal value\n");
		offsetStrSave += last->image.head.size_bin;
		wp = last->image.BIN;
		limitBin = wp + last->image.head.size_bin;
		do {
			trCnt  = *wp++ << 8;
			trCnt |= *wp++ ;
			if(!trCnt)
				fatal_error("illegal bin value\n");
			while(trCnt){
				wp+= 2;	
				info   = wp[0] << 8 ;
				info  |= wp[1];
				sufid  = wp[2] << 16;
				sufid |= wp[3] << 8;
				sufid |= wp[4];
				sinfo  = info & 0x8000;
				if(info & 0x7fff){
					info = last->relocateInfCnt + (info & 0x7fff);
					info |= sinfo;
				}
				if(sinfo){
					if(sufid){
						if(last->image.sufoffset[sufid]){
							sufid  = last->image.sufoffset[sufid];
							info &= 0x7fff;
						} else {
							if(!last->newSufId)
								fatal_error("suffix array a illegal value\n");
							sufid = last->newSufId[sufid];
						}
					}
				} else {
					sufid = sufid + last->relocateBinOffset;
				}
				*wp++ = (unsigned char)((info >> 8 ) & 0xff);
				*wp++ = (unsigned char)( info        & 0xff);
				*wp++ = (unsigned char)((sufid >> 16)& 0xff);
				*wp++ = (unsigned char)((sufid >> 8 )& 0xff);
				*wp++ = (unsigned char)( sufid       & 0xff);
				trCnt--; 
			}
		} while(wp < limitBin);
		if(wp != limitBin) fatal_error("size not match\n");

		
		if(fwrite(last->image.BIN,last->image.head.size_bin,1,f) != 1)
			fatal_error("write fail\n");
		last = last->next;
	}

	newHead.size_bin= offsetStrSave;
	newHead.flag |= (racStateCounter) ? TYPE_BIN_RACINE:0;
	fseek(f,0,0);
	newHead.writeAtFile(f);	
	u_fclose(f);
}
#ifdef DEBUG_MER
static unichar PrBuff[4096];
static void testLoad(char *fname)
{
	class explore_bin1 tbin;
	U_FILE *t = u_fopen(UTF16_LE,"out.txt",U_WRITE);
	u_printf("result out\n");
	remove_extension(fname,filename);
	strcat(filename,".bin");
	tbin.loadBin(filename);
	tbin.exploreTree(t,PrBuff);
	fclose(t);
}
#endif



int main_MergeBin(int argc , char **argv) {

	int iargIndex=1;
	nameOfoutput = 0;
	headFiles = tailFile = 0;
	fileListCounter = 0;
	if(argc == 1) {
	   usage();
	   return 0;
	}
#ifdef DEBUG_MER
	dMode = 0;
#endif

	initVar();
	
	iargIndex=1;
	while(iargIndex < argc){

		if(argv[iargIndex][0] != '-'){
           getOneFile(argv[iargIndex]);
        } else {
        	switch(argv[iargIndex][1]){
#ifdef DEBUG_MER
        	case 'd':dMode  = 1;break;
#endif
        	case 'o':iargIndex++; 
        		nameOfoutput = new char [strlen(argv[iargIndex])+1];
        		strcpy(nameOfoutput,argv[iargIndex]);
        		break;
        	case 'l':iargIndex++; 
                  getListFile(argv[iargIndex]);
        		break;
        	default:
        	  usage();
        	  return 1;
            }
       	}
		iargIndex++;
	}
	if(!fileListCounter) fatal_error("null file read\n");
//printf("%d",fileListCounter);

	if(!nameOfoutput){
 		nameOfoutput = new char [strlen("merged")+1];
		strcpy(nameOfoutput,"merged");
	}
	if(headFiles && headFiles->next){
		load_bins(nameOfoutput);
		mergeFiles(nameOfoutput,headFiles);
	}
#ifdef DEBUG_MER
	struct binFileList *tail=	headFiles;
	while(tail){
		headFiles = tail;
		tail = tail->next;
		if(headFiles->newSufId)
			delete headFiles->newSufId;
		delete headFiles;
	}

	if(dMode)	testLoad(argv[iargIndex]);
#endif
	if(nameOfoutput) delete nameOfoutput;
	u_printf("Done.\n");

	return(0);
}
