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
#include <stdlib.h>

//using namespace std;

#include <locale.h>
#include <stdlib.h>
#include "UnusedParameter.h"
#include "Unicode.h"
#include "Copyright.h"
#include "File.h"
#include "Alphabet.h"
#include "etc.h"
#include "bin.h"
#include "bin3.h"
#include "IOBuffer.h"




static const unichar closeSymbol=(unichar)')';
static const unichar openSymbol=(unichar)'(';
static const unichar newLineSymbol=(unichar)'\n';
//static const unichar blancChar=L' ';
static const unichar ordinationChar=(unichar)' ';
//static const unichar tabsymbole = L'\t';
static const unichar coordinationChar=(unichar)'+';

//
//	save data for passing
//
struct savePassingData {
	unichar *word;
	unichar *info;
	int current_index;
};



const char* usage_ConsultDic =
"\nConsultDic [-d dicfile] [-l diclistfile] [-a alphabetfile] textTokenfile"\
"\n diclistfile : list of dictionaries to be applied"\
"\n divide a word to rac+ suf+ form"\
"\n output file : seqMorphs.txt"\
;

static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_ConsultDic);
}


//
//	line for accumule all words on the path
//
#define SEGMAX	5
#define SEG_LINE_MAX	2048

struct binFileList {
	char *fname;
	class explore_bin1 dic;
	struct binFileList *next;
};

struct ConsultDic_ctx {
    U_FILE *findFile;

    struct savePassingData aLine[4096];

    unichar lsegs[SEGMAX][SEG_LINE_MAX];
    unichar info_flechi[SEG_LINE_MAX];
    unichar info_cano[SEG_LINE_MAX];
    unichar info_org[SEG_LINE_MAX];
    unichar info_inf[SEG_LINE_MAX];
    							
    unichar workBuff[1024*5];


    struct sufptr **sufoffset;

    int findCnt;
    int notFindCnt;
    unichar save_flechi[SEG_LINE_MAX];
    unichar save_cano[SEG_LINE_MAX];
    unichar save_org[SEG_LINE_MAX];
    unichar save_inf[SEG_LINE_MAX];
    unichar save_tmp[SEG_LINE_MAX];

    int findWordCnt;
    char tmpBuff0[2048];

    struct binFileList *tailFile,*headFiles;
    int fileListCounter;

    char buff[2048];
};

static int
convertInfosToGrf(struct ConsultDic_ctx * p_consultDic_ctx,int index)
{
    int signal = 0;
	int i,cdepth;
	struct savePassingData *sp = &(p_consultDic_ctx->aLine[index]);
	unichar *infos = sp->info;
	unichar *wp;
	wp = p_consultDic_ctx->info_flechi;
	for( i = 0; i < sp->current_index;i++) 	*wp++ = sp->word[i];
	*wp = 0;
	
	wp = p_consultDic_ctx->info_cano;
	if((*infos == '+') ||(*infos == '-'))infos++;
	if( ( *infos >= '0') && (*infos <= '9')){
		i = 0;
		while( ( *infos >= '0') && (*infos <= '9'))
			i = i * 10 + *infos++ - '0';
		cdepth = sp->current_index-i;
		for(i = 0; i < cdepth;i++)	*wp++ = sp->word[i];
	}
	while(*infos != '.'){
		if(!(*infos))  {
//                  die("bad infos canon: %S\n", info_cano);
                fatal_error("illegal info pas de .\n");
                }
		*wp++ = *infos++;
	}
	*wp = 0;
	infos++;
	wp = p_consultDic_ctx->info_org;
	while(*infos != '.'){
		if(!(*infos)) {
//                  die("bad infos orig : %S\n", info_org);

                fatal_error("illegal info\n");
                }
		*wp++ = *infos++;
	}
	*wp=0;
	infos++;
	wp = p_consultDic_ctx->info_inf;
	while((*infos == ' ') || (*infos == '\t')) infos++;
	
	switch(*infos){
    case '-' : if(*(infos+1))*wp++ = '+';
                infos++;
                signal = -1; break;
    case '+' :  signal = 1;
                if(*(infos+1)) break;
                infos++; break;
    default : signal = 0;
    }
 	while(*infos) *wp++ = *infos++;
	*wp = 0;
	return(signal);
}
static void saveAsequnceMorpheme(struct ConsultDic_ctx * p_consultDic_ctx,int &saveCnt)
{
   if( (!(p_consultDic_ctx->save_flechi[0])) &&
	(!(p_consultDic_ctx->save_cano[0]))&&
	(!(p_consultDic_ctx->save_org[0]))&&
	(!(p_consultDic_ctx->save_inf[0]))) return;
    unichar *iPtr = p_consultDic_ctx->workBuff;
    unichar *wp;
	if(saveCnt){
		*iPtr++ = ordinationChar;
	} else {
		*iPtr++ = openSymbol;
	}
	*iPtr++ = '{';

	wp = p_consultDic_ctx->save_flechi;while(*wp) *iPtr++ = *wp++;
	*iPtr++ = ',';
	wp = p_consultDic_ctx->save_cano;while(*wp) *iPtr++ = *wp++;
	
	if(p_consultDic_ctx->lsegs[2][0]){
		*iPtr++ = '(';
		wp = p_consultDic_ctx->save_org;while(*wp) *iPtr++ = *wp++;
		*iPtr++ = ')';
	}
	*iPtr++ = '.';
	wp = p_consultDic_ctx->save_inf;while(*wp) *iPtr++ = *wp++;
	*iPtr++ = '}';
	saveCnt++;
	*iPtr=0;
	u_fwrite_raw(p_consultDic_ctx->workBuff,(int)(iPtr-(p_consultDic_ctx->workBuff)),p_consultDic_ctx->findFile);
	p_consultDic_ctx->save_flechi[0]= 0;
	p_consultDic_ctx->save_cano[0]= 0;
	p_consultDic_ctx->save_org[0]= 0;
	p_consultDic_ctx->save_inf[0]= 0;
}

static void ajouteBack(struct ConsultDic_ctx * p_consultDic_ctx)
{
    u_strcat(p_consultDic_ctx->save_flechi,p_consultDic_ctx->info_flechi);
    u_strcat(p_consultDic_ctx->save_cano,p_consultDic_ctx->info_cano);
    u_strcat(p_consultDic_ctx->save_org,p_consultDic_ctx->info_org);
    u_strcat(p_consultDic_ctx->save_inf,p_consultDic_ctx->info_inf);
}
static void ajouteForward(struct ConsultDic_ctx * p_consultDic_ctx)
{
    u_strcat(p_consultDic_ctx->save_flechi,p_consultDic_ctx->info_flechi);
    u_strcat(p_consultDic_ctx->save_cano,p_consultDic_ctx->info_cano);
    u_strcat(p_consultDic_ctx->save_org,p_consultDic_ctx->info_org);
    u_strcpy(p_consultDic_ctx->save_tmp,p_consultDic_ctx->save_inf);
    u_strcpy(p_consultDic_ctx->save_inf,p_consultDic_ctx->info_inf);
    u_strcat(p_consultDic_ctx->save_inf,p_consultDic_ctx->save_tmp);    
}
static int actForFinal(unichar *cc,int depth,intptr_t infoT,intptr_t suf,int sdepth,void* private_ptr)
{
    DISCARD_UNUSED_PARAMETER(depth)
    DISCARD_UNUSED_PARAMETER(cc)
    DISCARD_UNUSED_PARAMETER(suf)

    struct ConsultDic_ctx * p_consultDic_ctx=(struct ConsultDic_ctx *)private_ptr;

	if(suf || infoT){ // not find
		(p_consultDic_ctx->notFindCnt)++;
		return(0);
	}
	int index;
	unichar *wp;
	int saveCnt = 0;

	if(p_consultDic_ctx->findCnt)
		u_fputc(coordinationChar,p_consultDic_ctx->findFile);

	int lastSign = 0;
	for( index = 0; index < sdepth;index++){
		if((p_consultDic_ctx->aLine[index].info) == 0) fatal_error("illegal info value\n");
		wp = (p_consultDic_ctx->aLine[index].info);
		if((wp[0] == '.') &&
			(wp[1] == '.') &&
			(wp[2] == 0)){	// info for branch the transition of path
			continue;
		}        
		switch(convertInfosToGrf(p_consultDic_ctx,index)){	// call next item and concation
        case 1:     
             switch(lastSign){
             case 1: 
                     ajouteBack(p_consultDic_ctx); break;
             case 0: saveAsequnceMorpheme(p_consultDic_ctx,saveCnt); 
                     ajouteBack(p_consultDic_ctx); break;
             case -1:
             default: 
                   fatal_error("%s::%d::+1\nInacceptable format\n",getUtoChar(p_consultDic_ctx->aLine[index].info),lastSign);
             }
             lastSign = 1;
             break;
		case -1: 
			 switch(lastSign){
             case -1: 
             case  0: ajouteBack(p_consultDic_ctx); break;
             case  1:
             default: 
                   fatal_error("%s::%d::-1\nInacceptable format\n",getUtoChar(p_consultDic_ctx->aLine[index].info),lastSign);
             }// call next item and concation with info	
     	   lastSign = -1;
     	   break;
		case 0:
			 switch(lastSign){
             case -1:
             case  0: saveAsequnceMorpheme(p_consultDic_ctx,saveCnt); 
                      ajouteBack(p_consultDic_ctx);break;  
             case  1:
                      ajouteForward(p_consultDic_ctx);
                      break;
             default: 
                   fatal_error("%s::%d\nInacceptable format\n",getUtoChar(p_consultDic_ctx->aLine[index].info),lastSign);
             }// call next item and concation with info	
           lastSign = 0;
		}
	}
    saveAsequnceMorpheme(p_consultDic_ctx,saveCnt);
	
	if(saveCnt)
	    u_fputc(closeSymbol,p_consultDic_ctx->findFile);
//		fwrite(&closeSymbol,2,1,findFile);
	else
		fatal_error("Illegal dictionary value\n");
	(p_consultDic_ctx->findCnt)++;
	return(0);
}
static int actForInfo(unichar *c,int depth,intptr_t info,intptr_t suf,int sdepth,void* private_ptr)
{
    DISCARD_UNUSED_PARAMETER(suf)
    struct ConsultDic_ctx * p_consultDic_ctx=(struct ConsultDic_ctx *)private_ptr;
	p_consultDic_ctx->aLine[sdepth].word = c;
	p_consultDic_ctx->aLine[sdepth].current_index = depth;
	p_consultDic_ctx->aLine[sdepth].info = (unichar *)info;
	return(++sdepth);
}


static void
getOneFile(struct ConsultDic_ctx * p_consultDic_ctx,char *filename)
{
	struct binFileList *tmp;	
    tmp = new struct binFileList;
	tmp->fname = new char[strlen(filename)+1];
	tmp->next = 0;

	strcpy(tmp->fname,filename);
	if(p_consultDic_ctx->headFiles){
        p_consultDic_ctx->tailFile->next = tmp;
        p_consultDic_ctx->tailFile = p_consultDic_ctx->tailFile->next;
	} else {
        p_consultDic_ctx->tailFile = p_consultDic_ctx->headFiles= tmp;
  	}
	(p_consultDic_ctx->fileListCounter)++;
}

static void getListFile(struct ConsultDic_ctx * p_consultDic_ctx,char *filename)
{
	register char *wp;
	char pathName[1024];
	int pathLen;
	struct binFileList *tmp;
	U_FILE *lstF;
    lstF = u_fopen(BINARY,filename,U_READ);
    if (!lstF)
    	fopenErrMessage(filename);	
    get_path(filename,pathName);
    pathLen = (int)strlen(pathName);
    while(af_fgets(p_consultDic_ctx->buff,1024,lstF->f)){
    	wp = p_consultDic_ctx->buff;

    	while(*wp){
    		if(*wp == 0x0d){ *wp = 0; break;}
    		if(*wp == 0x0a){ *wp = 0; break;}
    		wp++;
    	}
    	
    	switch(p_consultDic_ctx->buff[0]){
    	case ' ':	// comment line
    	case 0:
    		continue;
    	}

    	tmp = new struct binFileList;
    	tmp->fname = new char[pathLen+strlen(p_consultDic_ctx->buff)+1];
    	strcpy(tmp->fname,pathName);
    	strcat(tmp->fname,p_consultDic_ctx->buff);
    	tmp->next = 0;
    	if(p_consultDic_ctx->headFiles){	
           p_consultDic_ctx->tailFile->next = tmp; 
           p_consultDic_ctx->tailFile = p_consultDic_ctx->tailFile->next;
    	} else {
            p_consultDic_ctx->tailFile = p_consultDic_ctx->headFiles= tmp;	}
    	(p_consultDic_ctx->fileListCounter)++;
    }
    u_fclose(lstF);
}


void consultationLesTokens(struct ConsultDic_ctx * p_consultDic_ctx,Encoding encoding_output,int bom_output,
                           char *textfile,Alphabet *PtrAlphabet)
{
	

	struct binFileList *tmp;
	
	tmp = p_consultDic_ctx->headFiles;
	while(tmp){
	     tmp->dic.loadBin(tmp->fname);
	     tmp->dic.set_act_func(actForFinal,(void*)p_consultDic_ctx,actForInfo,(void*)p_consultDic_ctx);
	     if(PtrAlphabet) tmp->dic.setAlphabetTable(PtrAlphabet);
	    tmp= tmp->next;                
    }
	
	
	unsigned short *tokensmap;
	unichar **tokensTable;
	int tokensCnt = getStringTableFile(textfile,tokensmap,
		tokensTable);


	get_path(textfile,p_consultDic_ctx->tmpBuff0);
	strcat(p_consultDic_ctx->tmpBuff0,"seqMorphs.txt");
	p_consultDic_ctx->findFile = u_fopen_creating_versatile_encoding(encoding_output,bom_output,p_consultDic_ctx->tmpBuff0,U_WRITE);
	if(!(p_consultDic_ctx->findFile)) fatal_error("Save file \"seqMorphs.txt\" open fail\n");

	int wordCnt = 0;

	strFileHeadLine(p_consultDic_ctx->findFile,tokensCnt);
    tmp = p_consultDic_ctx->headFiles;	    
	while(tmp){
	  u_printf("load dictionnaire %s\n",tmp->fname);
      tmp= tmp->next;                
    }
		
	int sidx;
	for(wordCnt = 0; wordCnt<tokensCnt;wordCnt++){

		if(!(wordCnt % 10000)) u_printf("\rread token %d",wordCnt);
		sidx = 0;
	    tmp = p_consultDic_ctx->headFiles;	    
		while(tmp){
        	p_consultDic_ctx->findCnt = 0;
            tmp->dic.searchMotAtTree(tokensTable[wordCnt],0);
            /*if (findCnt) {
               u_printf("%S",tokensTable[wordCnt]);
            }*/        
            if(p_consultDic_ctx->findCnt) (p_consultDic_ctx->findWordCnt)++;
    	    tmp= tmp->next;                
        }
//		fwrite(&newLineSymbol,2,1,p_consultDic_ctx->findFile);
        u_fputc(newLineSymbol,p_consultDic_ctx->findFile);
	}
	u_fclose(p_consultDic_ctx->findFile);
}


int main_ConsultDic(int argc,char *argv[]) {

	int argIdx= 1;
//	char *decodageMap = 0;
//	char *hanjaFileName;
 	int debugFlag =0;

    struct ConsultDic_ctx * p_consultDic_ctx = new struct ConsultDic_ctx;
    if (p_consultDic_ctx == NULL)
        fatal_error("mem alloc fail\n");
    memset(p_consultDic_ctx,0,sizeof(struct ConsultDic_ctx));

 	p_consultDic_ctx->fileListCounter = 0;
 	p_consultDic_ctx->tailFile = p_consultDic_ctx->headFiles = NULL;
 	Alphabet *saveAlphabet = 0;

    Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
    int bom_output = DEFAULT_BOM_OUTPUT;
    int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;

	if(argc == 1) {
	   usage();
       delete p_consultDic_ctx;
	   return 9;
	}
	
    while(argIdx < argc -1 ){
		if(argv[argIdx][0] != '-'){
			getOneFile(p_consultDic_ctx,argv[argIdx]);
			argIdx++;
   			continue;
		}
		switch(argv[argIdx][1]){
//		case 't':
//			decodageMap = argv[++argIdx];
//			break;
//		case 'h':
//		    hanjaFileName = argv[++argIdx];
//			break;
        case 'a':
             ++argIdx; saveAlphabet =load_alphabet(argv[argIdx] );
             if(!saveAlphabet) {
                usage();
                delete p_consultDic_ctx;
                return 0;
             }
             break; 
		case 'l':
			++argIdx;
            getListFile(p_consultDic_ctx,argv[argIdx]);
			break;
        case 'k': argIdx++;
                 if (argv[argIdx][0]=='\0') {
                    fatal_error("Empty input_encoding argument\n");
                 }
                 decode_reading_encoding_parameter(&mask_encoding_compatibility_input,argv[argIdx]);
                 break;
        case 'q': argIdx++;
                 if (argv[argIdx][0]=='\0') {
                    fatal_error("Empty output_encoding argument\n");
                 }
                 decode_writing_encoding_parameter(&encoding_output,&bom_output,argv[argIdx]);
                 break;
		case 'd':
			debugFlag = 1; break;
		default:
			usage();
            delete p_consultDic_ctx;
			return 1;
		}
		argIdx++;
	}
	if(argc -1  != argIdx) {
	   usage();
       delete p_consultDic_ctx;
	   return 1;
	}
	if(!(p_consultDic_ctx->fileListCounter)) {
	   usage();
       delete p_consultDic_ctx;
	   return 1;
	}
	consultationLesTokens(p_consultDic_ctx,encoding_output,bom_output,argv[argIdx],saveAlphabet);
	if(saveAlphabet) free_alphabet(saveAlphabet);
    delete p_consultDic_ctx;
	return 0;
}
