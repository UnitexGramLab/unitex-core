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
#include "Unicode.h"
#include "Copyright.h"
#include "File.h"
#include "Alphabet.h"
#include "etc.h"
#include "bin.h"
#include "bin3.h"
#include "IOBuffer.h"



static U_FILE *findFile;
static unichar closeSymbol=(unichar)')';
static unichar openSymbol=(unichar)'(';
static unichar newLineSymbol=(unichar)'\n';
//static unichar blancChar=L' ';
static unichar ordinationChar=(unichar)' ';
//static unichar tabsymbole = L'\t';
static unichar coordinationChar=(unichar)'+';

//
//	save data for passing
//
struct savePassingData {
	unichar *word;
	unichar *info;
	int current_index;
};
struct savePassingData aLine[4096];


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

static unichar lsegs[SEGMAX][SEG_LINE_MAX];
static unichar info_flechi[SEG_LINE_MAX];
static unichar info_cano[SEG_LINE_MAX];
static unichar info_org[SEG_LINE_MAX];
static unichar info_inf[SEG_LINE_MAX];
							
static unichar workBuff[1024*5];


struct sufptr **sufoffset;

static int findCnt;
static int notFindCnt;
static unichar save_flechi[SEG_LINE_MAX];
static unichar save_cano[SEG_LINE_MAX];
static unichar save_org[SEG_LINE_MAX];
static unichar save_inf[SEG_LINE_MAX];
static unichar save_tmp[SEG_LINE_MAX];


static int
convertInfosToGrf(int index)
{
    int signal = 0;
	int i,cdepth;
	struct savePassingData *sp = &aLine[index];
	unichar *infos = sp->info;
	unichar *wp;
	wp = info_flechi;
	for( i = 0; i < sp->current_index;i++) 	*wp++ = sp->word[i];
	*wp = 0;
	
	wp = info_cano;
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
	wp = info_org;
	while(*infos != '.'){
		if(!(*infos)) {
//                  die("bad infos orig : %S\n", info_org);

                fatal_error("illegal info\n");
                }
		*wp++ = *infos++;
	}
	*wp=0;
	infos++;
	wp = info_inf;
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
static void saveAsequnceMorpheme(int &saveCnt)
{
   if( (!save_flechi[0]) &&
	(!save_cano[0])&&
	(!save_org[0])&&
	(!save_inf[0])) return;
    unichar *iPtr = workBuff;
    unichar *wp;
	if(saveCnt){
		*iPtr++ = ordinationChar;
	} else {
		*iPtr++ = openSymbol;
	}
	*iPtr++ = '{';

	wp = save_flechi;while(*wp) *iPtr++ = *wp++;
	*iPtr++ = ',';
	wp = save_cano;while(*wp) *iPtr++ = *wp++;
	
	if(lsegs[2][0]){
		*iPtr++ = '(';
		wp = save_org;while(*wp) *iPtr++ = *wp++;
		*iPtr++ = ')';
	}
	*iPtr++ = '.';
	wp = save_inf;while(*wp) *iPtr++ = *wp++;
	*iPtr++ = '}';
	saveCnt++;
	*iPtr=0;
	u_fwrite_raw(workBuff,(int)(iPtr-workBuff),findFile);
	save_flechi[0]= 0;
	save_cano[0]= 0;
	save_org[0]= 0;
	save_inf[0]= 0;
}

static void ajouteBack()
{
    u_strcat(save_flechi,info_flechi);
    u_strcat(save_cano,info_cano);
    u_strcat(save_org,info_org);
    u_strcat(save_inf,info_inf);
}
static void ajouteForward()
{
    u_strcat(save_flechi,info_flechi);
    u_strcat(save_cano,info_cano);
    u_strcat(save_org,info_org);
    u_strcpy(save_tmp,save_inf);
    u_strcpy(save_inf,info_inf);
    u_strcat(save_inf,save_tmp);    
}
static int actForFinal(unichar *cc,int depth,intptr_t infoT,intptr_t suf,int sdepth)
{

	if(suf || infoT){ // not find
		notFindCnt++;
		return(0);
	}
	int index;
	unichar *wp;
	int saveCnt = 0;

	if(findCnt)
		u_fputc(coordinationChar,findFile);

	int lastSign = 0;
	for( index = 0; index < sdepth;index++){
		if(aLine[index].info == 0) fatal_error("illegal info value\n");
		wp = aLine[index].info;
		if((wp[0] == '.') &&
			(wp[1] == '.') &&
			(wp[2] == 0)){	// info for branch the transition of path
			continue;
		}        
		switch(convertInfosToGrf(index)){	// call next item and concation
        case 1:     
             switch(lastSign){
             case 1: 
                     ajouteBack(); break;
             case 0: saveAsequnceMorpheme(saveCnt); 
                     ajouteBack(); break;
             case -1:
             default: 
                   fatal_error("%s::%d::+1\nInacceptable format\n",getUtoChar(aLine[index].info),lastSign);
             }
             lastSign = 1;
             break;
		case -1: 
			 switch(lastSign){
             case -1: 
             case  0: ajouteBack(); break;
             case  1:
             default: 
                   fatal_error("%s::%d::-1\nInacceptable format\n",getUtoChar(aLine[index].info),lastSign);
             }// call next item and concation with info	
     	   lastSign = -1;
     	   break;
		case 0:
			 switch(lastSign){
             case -1:
             case  0: saveAsequnceMorpheme(saveCnt); 
                      ajouteBack();break;  
             case  1:
                      ajouteForward();
                      break;
             default: 
                   fatal_error("%s::%d\nInacceptable format\n",getUtoChar(aLine[index].info),lastSign);
             }// call next item and concation with info	
           lastSign = 0;
		}
	}
    saveAsequnceMorpheme(saveCnt);
	
	if(saveCnt)
	    u_fputc(closeSymbol,findFile);
//		fwrite(&closeSymbol,2,1,findFile);
	else
		fatal_error("Illegal dictionary value\n");
	findCnt++;
	return(0);
}
static int actForInfo(unichar *c,int depth,intptr_t info,intptr_t suf,int sdepth)
{
	aLine[sdepth].word = c;
	aLine[sdepth].current_index = depth;
	aLine[sdepth].info = (unichar *)info;
	return(++sdepth);
}

static 	int findWordCnt;
static char tmpBuff0[2048];

struct binFileList {
	char *fname;
	class explore_bin1 dic;
	struct binFileList *next;
};

static struct binFileList *tailFile,*headFiles;
int fileListCounter;
static void
getOneFile(char *filename)
{
	struct binFileList *tmp;	
    tmp = new struct binFileList;
	tmp->fname = new char[strlen(filename)+1];
	tmp->next = 0;

	strcpy(tmp->fname,filename);
	if(headFiles){
        tailFile->next = tmp;
        tailFile = tailFile->next;
	} else {
        tailFile = headFiles= tmp;
  	}
	fileListCounter++;
}
static char buff[2048];
static void getListFile(char *filename)
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
    while(af_fgets(buff,1024,lstF->f)){
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
    	if(headFiles){	
           tailFile->next = tmp; 
           tailFile = tailFile->next;
    	} else {
            tailFile = headFiles= tmp;	}
    	fileListCounter++;
    }
    u_fclose(lstF);
}


void consultationLesTokens(Encoding encoding_output,int bom_output,
                           char *textfile,Alphabet *PtrAlphabet)
{
	

	struct binFileList *tmp;
	
	tmp = headFiles;
	while(tmp){
	     tmp->dic.loadBin(tmp->fname);
	     tmp->dic.set_act_func(actForFinal,actForInfo);
	     if(PtrAlphabet) tmp->dic.setAlphabetTable(PtrAlphabet);
	    tmp= tmp->next;                
    }
	
	
	unsigned short *tokensmap;
	unichar **tokensTable;
	int tokensCnt = getStringTableFile(textfile,tokensmap,
		tokensTable);


	get_path(textfile,tmpBuff0);
	strcat(tmpBuff0,"seqMorphs.txt");
	findFile = u_fopen_creating_versatile_encoding(encoding_output,bom_output,tmpBuff0,U_WRITE);
	if(!findFile) fatal_error("Save file \"seqMorphs.txt\" open fail\n");

	int wordCnt = 0;

	strFileHeadLine(findFile,tokensCnt);
    tmp = headFiles;	    
	while(tmp){
	  u_printf("load dictionnaire %s\n",tmp->fname);
      tmp= tmp->next;                
    }
		
	int sidx;
	for(wordCnt = 0; wordCnt<tokensCnt;wordCnt++){

		if(!(wordCnt % 10000)) u_printf("\rread token %d",wordCnt);
		sidx = 0;
	    tmp = headFiles;	    
		while(tmp){
        	findCnt = 0;
            tmp->dic.searchMotAtTree(tokensTable[wordCnt],0);
            /*if (findCnt) {
               u_printf("%S",tokensTable[wordCnt]);
            }*/        
            if(findCnt) findWordCnt++;
    	    tmp= tmp->next;                
        }
//		fwrite(&newLineSymbol,2,1,findFile);
        u_fputc(newLineSymbol,findFile);
	}
	u_fclose(findFile);
}


int main_ConsultDic(int argc,char *argv[]) {

	int argIdx= 1;
//	char *decodageMap = 0;
//	char *hanjaFileName;
 	int debugFlag =0;
 	fileListCounter = 0;
 	Alphabet *saveAlphabet =0;

    Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
    int bom_output = DEFAULT_BOM_OUTPUT;
    int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;

	if(argc == 1) {
	   usage();
	   return 9;
	}
	
    while(argIdx < argc -1 ){
		if(argv[argIdx][0] != '-'){
			getOneFile(argv[argIdx]);
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
                return 0;
             }
             break; 
		case 'l':
			++argIdx;
            getListFile(argv[argIdx]);
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
			return 1;
		}
		argIdx++;
	}
	if(argc -1  != argIdx) {
	   usage();
	   return 1;
	}
	if(!fileListCounter) {
	   usage();
	   return 1;
	}
	consultationLesTokens(encoding_output,bom_output,argv[argIdx],saveAlphabet);
	if(saveAlphabet) free_alphabet(saveAlphabet);
	return 0;
}
