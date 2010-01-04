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

//using namespace std;

#include "Unicode.h"
#include "etc.h"
//#include "String_hash2.h"
#include "bin.h"
#include "bin3.h"
#include "Copyright.h"
#include "IOBuffer.h"


#define DEBUG_MER 1
const char* usage_MergeBin =
"MergeBin [-d] [-l lfilename] -o ofilename f0 f1 ... fn ] ] \n"\
"         : merge files which has the extension .bin \n"\
"  -d : debugMode set. all paths out to the file \"out.txt\"\n"\
"  -l lfilename : get files from list file\"\n"\
"  -o ofilename : set the output file name,at input files from command line\"\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_MergeBin);
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

struct MergeBin_ctx {
    int INF_offset;
    int REF_offset;
    int BIN_offset;
    int racTableIdx;
    int sufTableIdx;
    int infTableIdx;
    class arbre_string00 totSuf;
    class arbre_string0 unresolSuf;	// not resoled suffixe
    char *nameOfoutput;

    char path_of_locate[1024];
    char buff[1024];
    int racStateCounter;
    int fileListCounter;

    struct binFileList *tailFile,*headFiles;
#ifdef DEBUG_MER
    int dMode;
    unichar PrBuff[4096];
#endif
};

struct racTable {
	unsigned short *s;
	int pos;
	int offset;
};

static void initVar(struct MergeBin_ctx* p_mergeBin_ctx)
{
p_mergeBin_ctx->INF_offset = 0;
p_mergeBin_ctx->REF_offset =0;
p_mergeBin_ctx->BIN_offset =0;
p_mergeBin_ctx->racTableIdx =0 ;
p_mergeBin_ctx->sufTableIdx =0;
p_mergeBin_ctx->infTableIdx =0;
p_mergeBin_ctx->unresolSuf.put(assignUstring(u_epsilon_string));
#ifdef DEBUG_MER
p_mergeBin_ctx->dMode = 0;
#endif
}

static void
getOneFile(struct MergeBin_ctx* p_mergeBin_ctx,char *fn)
{
	struct binFileList *tmp;
    tmp = new struct binFileList;
	tmp->fname = new char[strlen(fn)+1];
	tmp->next = 0;
	tmp->newSufId = 0;
	strcpy(tmp->fname,fn);
	if(p_mergeBin_ctx->headFiles){
        p_mergeBin_ctx->tailFile->next = tmp;
        p_mergeBin_ctx->tailFile = p_mergeBin_ctx->tailFile->next;
	} else {
        p_mergeBin_ctx->tailFile = p_mergeBin_ctx->headFiles= tmp;
  	}
  	u_printf("Load file name :: <%s>\n",fn);
	p_mergeBin_ctx->fileListCounter++;
}
static void getListFile(struct MergeBin_ctx* p_mergeBin_ctx,char *fn)
{
	register char *wp;
	char pathName[1024];
	int pathLen;
struct binFileList *tmp;
	U_FILE *lstF;
	u_printf("Load file %s\n",fn);
    lstF = u_fopen(BINARY,fn,U_READ);
    if (!lstF)
    	fopenErrMessage(fn);
    get_path(fn,pathName);
    pathLen = (int)strlen(pathName);
    while(af_fgets(p_mergeBin_ctx->buff,1024,lstF->f)){
    	wp = p_mergeBin_ctx->buff;

    	while(*wp){
    		if(*wp == 0x0d){ *wp = 0; break;}
    		if(*wp == 0x0a){ *wp = 0; break;}
    		wp++;
    	}

    	switch(p_mergeBin_ctx->buff[0]){
    	case ' ':	// comment line
    	case 0:
    		continue;
    	}

    	tmp = new struct binFileList;
    	tmp->fname = new char[pathLen+strlen(p_mergeBin_ctx->buff)+1];
    	strcpy(tmp->fname,pathName);
    	strcat(tmp->fname,p_mergeBin_ctx->buff);
    	tmp->next = 0;
    	tmp->newSufId = 0;
    	if(p_mergeBin_ctx->headFiles){
           p_mergeBin_ctx->tailFile->next = tmp;
           p_mergeBin_ctx->tailFile = p_mergeBin_ctx->tailFile->next;
    	} else {
            p_mergeBin_ctx->tailFile = p_mergeBin_ctx->headFiles = tmp;	}
    	p_mergeBin_ctx->fileListCounter++;
    }
    u_fclose(lstF);
}

static void
load_bins(struct MergeBin_ctx* p_mergeBin_ctx,char *oFileName)
{
	p_mergeBin_ctx->path_of_locate[0] = 0;
	struct binFileList *tmp;
	struct binFileList *rhead,*rtail,*shead,*stail;


	p_mergeBin_ctx->racStateCounter = 0;
	rhead = shead = 0;
    rtail = stail = 0;
	tmp = p_mergeBin_ctx->headFiles;
	while(tmp){
      u_printf("%s\n",tmp->fname);
		tmp->image.loadBin(tmp->fname);
		tmp->newSufId = 0;

		if(tmp->image.isRacine()){
			(p_mergeBin_ctx->racStateCounter)++;
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

	p_mergeBin_ctx->headFiles = rhead;
	rtail->next = shead;
	if(!(p_mergeBin_ctx->headFiles))	fatal_error("null file read\n");
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
	if((p_mergeBin_ctx->racStateCounter) && ((p_mergeBin_ctx->racStateCounter) != 1)){
	    char charRootName[1024];
		relBin = (p_mergeBin_ctx->racStateCounter) * SIZE_ONE_TRANSITION_BIN + 2;
		remove_extension(oFileName,charRootName);
		unichar  *RootName = new unichar[strlen(charRootName)+1];
		u_strcpy(RootName,charRootName);
		p_mergeBin_ctx->totSuf.put((unichar *)RootName,0);
	}

	tmp = p_mergeBin_ctx->headFiles;
	while(tmp){
		tmp->relocateInfOffset = 0; // set value of relocated location
		tmp->relocateBinOffset = 0;

		for( i = 0; i <tmp->image.head.cnt_auto;i++){
		  u_printf("Initial state %s located at %d"
             ,getUtoChar(tmp->image.STR + tmp->image.AUT[i])
             ,tmp->image.autoffset[i]);
			tmp->image.autoffset[i] += relBin;
            u_printf("relocated at %d\n",tmp->image.autoffset[i]);
			nidx = p_mergeBin_ctx->totSuf.check(tmp->image.STR + tmp->image.AUT[i]);
			if(nidx != -1){
				fatal_error("at %s file,symbol %s:\ndupliate initial state name\n"
                 ,tmp->image.name
				 ,getUtoChar(tmp->image.STR + tmp->image.AUT[i]));
			}
			if(!nidx){
				fatal_error("at %s file,symbol\nnull initial state name\n",tmp->image.name);
			}
			nidx = p_mergeBin_ctx->totSuf.put(tmp->image.STR + tmp->image.AUT[i],
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

	tmp = p_mergeBin_ctx->headFiles;
	while(tmp){
		for( i = 1; i < tmp->image.head.cnt_suf;i++){// i = 0 : null
			nidx = p_mergeBin_ctx->totSuf.check(tmp->image.STR + tmp->image.SUF[i]);
			if(!nidx || nidx == -1){
				nidx = p_mergeBin_ctx->unresolSuf.put(tmp->image.STR + tmp->image.SUF[i]);
				unresolveSufCnt++;
				u_printf("the suffix, %s is not located\n",
				  getUtoChar(tmp->image.STR + tmp->image.SUF[i]));
				tmp->newSufId[i] = nidx;
			} else {
				nidx = (int)((intptr_t)p_mergeBin_ctx->totSuf.getCheckValue());
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
mergeFiles(struct MergeBin_ctx* p_mergeBin_ctx,char *ofn,struct binFileList *first)
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
	f = u_fopen(BINARY,ofilename,U_WRITE);
	if(!f)
		fopenErrMessage(ofilename);

	newHead.writeAtFile(f);
	newHead.cnt_auto = p_mergeBin_ctx->totSuf.size();
	newHead.cnt_suf = p_mergeBin_ctx->unresolSuf.size();

	//
	//	ref

	//	racine name table
	last = first;
	while(last){
		newHead.size_bin += last->image.head.size_bin;
		newHead.size_inf += last->image.head.size_inf;
		last = last->next;
	}
	unichar **a = p_mergeBin_ctx->totSuf.make_strPtr_table(&offsetMap);
	unichar **b = p_mergeBin_ctx->unresolSuf.make_strPtr_table();


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

	p_mergeBin_ctx->totSuf.release_value();

	for(i = 0; i < newHead.cnt_suf;i++){
		for(j = 1; j < b[i][0];j++)
			outbytes2((unsigned short)b[i][j],f);
		outbytes2(0,f);
		offsetStrSave += b[i][0];
	}

	p_mergeBin_ctx->unresolSuf.release_value();
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
	if((p_mergeBin_ctx->racStateCounter) && ((p_mergeBin_ctx->racStateCounter) != 1) ){ // dummy initial node
//printf("node  0 for racines");
		outbytes2((unichar)((unsigned int)(p_mergeBin_ctx->racStateCounter)),f);
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
			(p_mergeBin_ctx->racStateCounter) * SIZE_ONE_TRANSITION_BIN;
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
					info = (unsigned short)(last->relocateInfCnt + (info & 0x7fff));
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
	newHead.flag |= (p_mergeBin_ctx->racStateCounter) ? TYPE_BIN_RACINE:0;
	fseek(f,0,0);
	newHead.writeAtFile(f);
	u_fclose(f);
}
#ifdef DEBUG_MER
static void testLoad(struct MergeBin_ctx* p_mergeBin_ctx,char *fname)
{
	class explore_bin1 tbin;
    char filename[1024];
	U_FILE *t = u_fopen(UTF16_LE,"out.txt",U_WRITE);
	u_printf("result out\n");
	remove_extension(fname,filename);
	strcat(filename,".bin");
	tbin.loadBin(filename);
	tbin.exploreTree(t,p_mergeBin_ctx->PrBuff);
	u_fclose(t);
}
#endif



int main_MergeBin(int argc , char *argv[]) {

	int iargIndex=1;
    struct MergeBin_ctx mergeBin_ctx;
    memset(&mergeBin_ctx,0,sizeof(struct MergeBin_ctx));
	mergeBin_ctx.nameOfoutput = 0;
	mergeBin_ctx.headFiles = mergeBin_ctx.tailFile = 0;
	mergeBin_ctx.fileListCounter = 0;
	if(argc == 1) {
	   usage();
	   return 0;
	}
#ifdef DEBUG_MER
	mergeBin_ctx.dMode = 0;
#endif

	initVar(&mergeBin_ctx);

	iargIndex=1;
	while(iargIndex < argc){

		if(argv[iargIndex][0] != '-'){
           getOneFile(&mergeBin_ctx,argv[iargIndex]);
        } else {
        	switch(argv[iargIndex][1]){
#ifdef DEBUG_MER
        	case 'd': mergeBin_ctx.dMode  = 1;break;
#endif
            /* ignore -k and -q encoding parameter instead make error */
        	case 'k':iargIndex++;break;
        	case 'q':iargIndex++;break;
        	case 'o':iargIndex++;
        		mergeBin_ctx.nameOfoutput = new char [strlen(argv[iargIndex])+1];
        		strcpy(mergeBin_ctx.nameOfoutput,argv[iargIndex]);
        		break;
        	case 'l':iargIndex++;
                  getListFile(&mergeBin_ctx,argv[iargIndex]);
        		break;
        	default:
        	  usage();
        	  return 1;
            }
       	}
		iargIndex++;
	}
	if(!(mergeBin_ctx.fileListCounter)) fatal_error("null file read\n");
//printf("%d",fileListCounter);

	if(!(mergeBin_ctx.nameOfoutput)){
 		mergeBin_ctx.nameOfoutput = new char [strlen("merged")+1];
		strcpy(mergeBin_ctx.nameOfoutput,"merged");
	}
	if((mergeBin_ctx.headFiles) && (mergeBin_ctx.headFiles->next)) {
		load_bins(&mergeBin_ctx,mergeBin_ctx.nameOfoutput);
		mergeFiles(&mergeBin_ctx,mergeBin_ctx.nameOfoutput,mergeBin_ctx.headFiles);
	}
#ifdef DEBUG_MER
	struct binFileList *tail=	mergeBin_ctx.headFiles;
	while(tail){
		mergeBin_ctx.headFiles = tail;
		tail = tail->next;
		if(mergeBin_ctx.headFiles->newSufId)
			delete [] mergeBin_ctx.headFiles->newSufId;
		delete mergeBin_ctx.headFiles;
	}

	if(mergeBin_ctx.dMode)	testLoad(&mergeBin_ctx,argv[iargIndex]);
#endif
	if(mergeBin_ctx.nameOfoutput) delete [] mergeBin_ctx.nameOfoutput;
	u_printf("Done.\n");

	return(0);
}
