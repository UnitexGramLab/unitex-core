/*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "File.h"
#include "Copyright.h"
#include "bin.h"
#include "etc.h"
#include "formation_dic_line.h"
#include "bin3.h"
#include "IOBuffer.h"
#include "Error.h"


void usage(int flag) {
u_printf("%S",COPYRIGHT);
u_printf("Usage: CompressKr [-s] [-g] [-o ofilename] [-l] <dictionary>\n");
u_printf("  -s  : suffixes dictionnary, indicate the type of dictionnary, Suffixe or Racine\n");
u_printf("  -g  : dicitionary from the graph\n");
u_printf("  -l  : <dictionnary> file has list of dictionnarys \n");
u_printf("   <dictionary> : any unicode DELAF or DELACF dictionary\n");
u_printf("  -o  : set output file\n");
u_printf("Compresses a dictionary into an finite state automaton. This automaton\n");
u_printf("is stored is a .bin file,  the associated flexional codes are\n");
u_printf("written in a .inf file and the list of automates at .aut file\n\n");
if(flag) exit(1);
}

static void 
getFileName(unichar *des,unichar *src)
{
	register unichar *dPtr = 0;
	register unichar *ePtr = 0;
	register unichar *wp = src;
	while(*wp) {
		switch(*wp){
		case '\\': dPtr = wp;break;
		case '/': dPtr = wp;break;		
		case '.': ePtr = wp; break;
		}
		wp++;
	}
	wp = (dPtr) ? dPtr+1:src;
	dPtr = des;
	while(*wp) {
		if(wp == ePtr) break;
		*dPtr++ = *wp++;
	}
	*dPtr = 0;
}
//
//
//

static int read_DELA_to_DICO(class arbre_string3 &arbre,int a,char *fname);
//static void read_FST_to_DICO(class arbre_string3 &arbre,int a,char *fname); 
FILE *debugfile;
static 	unichar UtempBuff[512];


//
//	printf out list of initial states of the automate
//

//	the form of korean dico
//	flexion form,form orgin,canonical form, name of the suffix transductor, les information
//
#define MAX_LINE_ELEMENT_DELAKR	5
#define MAX_LINE_ELEMENT	1024
static unichar segs[MAX_LINE_ELEMENT_DELAKR+1][MAX_LINE_ELEMENT];
static unichar Etmp[4096];
static unichar Stmp[4096];



class arbre_string0 MotEtr;
static char *cfilename;
static 	char templine[512];

static void lineErrMess(int lineCnt,char *msg)
{
	error("%s file at line %d\n",cfilename,lineCnt);
	error("<<%S>>\nhas syntax error",UtempBuff);
	fatal_error("%s\n",msg);
}

static void make_compress_files(char *listFilename,int flag);
static int tokenize_entrees_Kr(int lcnt,unsigned short *line,class dicLines *& head);
static int suffixeMode;	// racine or suffixe
static int grapheMode;
static char *ofilename;

int main(int argc, char **argv) {
setBufferMode();

int argIdx = 1;
int listFormFlag = 0;	// filename is dictionnary
	debugfile = 0;
 grapheMode = 0;
suffixeMode = 0;
	ofilename  =0;
	if(argc == 1) {
       usage(0);
       return 0;
    }
//printf("%s\n",setlocale(LC_ALL,"Korean_korea.949"));
	while( argIdx < argc-1){
//printf("%s\n",argv[argIdx]);
		if(argv[argIdx][0] == '-'){
			switch(argv[argIdx][1]){
			case 'D': 
				debugfile = u_fopen("dd.txt",U_WRITE);
				break;
			case 'd': 
				debugfile = stdout;
				break;
			case 'l': listFormFlag++;break;
			case 's':suffixeMode++; break;
			case 'g':grapheMode++; break;
			case 'o':argIdx++;ofilename = argv[argIdx];break;
			default:
				usage(1);
			}
		}
		argIdx++;
	}
	
	if(argIdx != argc -1) usage(1);
	make_compress_files(argv[argIdx],listFormFlag);
	if(debugfile) fclose(debugfile);
	return(0);
}
/*
 */
static void read_list_files(simpleL<char *> &rd,char *filename)
{
	int sc;
	char fullPathName[2048];
	char pathName[1024];
	char *t;

	FILE *f=fopen(filename,"r");
	
	if(!f) {
		fatal_error("file %s open fail\n",filename);
	}
	get_path(filename,pathName);
	while(fgets(templine,256,f)){
		sc =0;
		while(templine[sc]){
			if( (templine[sc] == 0x0d) ||
				(templine[sc] == 0x0a)||
				(templine[sc] == '#') ){
				templine[sc] = 0;
				break;
			}
			sc++;
		}
		if((templine[0] == ' ') ||
		   (templine[0]== 0) )
			continue;

		strcpy(fullPathName,pathName);
		strcat(fullPathName,templine);
		
		t = new char[strlen(fullPathName)+1];
		strcpy(t,fullPathName);
		rd.put(t);
	}
	fclose(f);
}

//
//
static void 
make_compress_files(char *listFileName,int listFormFlag)
{
	class arbre_string3 arbres;
	simpleL<char *> readFiles;
	char *fnamePtr;
	char extension[16];
	unichar *tmp;
	int autoStartIndex = 0;
	
	int lineCnt = 0;
	if(listFormFlag){
		read_list_files(readFiles,listFileName);
	} else {
		readFiles.put(listFileName);
	}
	readFiles.reset();
	if(!suffixeMode){	// racine
		u_strcpy(segs[0],listFileName);
		getFileName(segs[1],segs[0]);
		tmp = new unichar[u_strlen(segs[1])+1];
		u_strcpy(tmp,segs[1]);
		autoStartIndex = arbres.new_arbre((unichar*)tmp);
	}
	// suffixe
	int readFileCnt=0;
	while( (fnamePtr = readFiles.getNext())){
		if(suffixeMode){	// suffixe
			u_strcpy(segs[0],fnamePtr);
			getFileName(segs[1],segs[0]);
			tmp = new unichar[u_strlen(segs[1])+1];
			u_strcpy(tmp,segs[1]);
			autoStartIndex = arbres.new_arbre((unichar *)tmp);
		}
		get_extension(fnamePtr,extension);
      u_printf("\n%s load\n",fnamePtr);
        lineCnt += read_DELA_to_DICO(arbres,autoStartIndex,fnamePtr);
		readFileCnt++;
	}
	if(readFileCnt > 1)
	     u_printf("\ntotal read is %d line\n",lineCnt);
	//
	//	minimize tree
	//
	//
	u_printf("Compressing... \n\n");
	arbres.minimize_tree();

	if(debugfile) {
		int i;
		for( i = 0; i < arbres.getArbreCnt();i++){
			u_fprintf(debugfile,"%i th===\n",i);
//			arbres.prNode(debugfile,arbrelistOut);
		}
	}
	
	if(ofilename)
	 remove_extension(ofilename,templine);
	else	 
	 remove_extension(listFileName,templine);
	arbres.toBinTr(templine,suffixeMode);

	if(debugfile) fclose(debugfile);
}

//
//	
//
//
static int
read_DELA_to_DICO(class arbre_string3 &arbre,int curArbreIdx,char *fname)
{
	FILE *f;
	int tokenCnt;
	int flineCnt = 0;
	class dicLines *heads,*wp;
	unichar RLine[2048];
	if(!(f=u_fopen(fname,U_READ)))	fopenErrMessage(fname);
	cfilename = fname;
	u_printf("Read File %s\n",fname);
	while(EOF!=u_fgets(UtempBuff,f)){
		if( (UtempBuff[0] == '\0') ||( UtempBuff[0] == ' '))
			continue; // comment line
		heads = 0; u_strcpy(RLine,UtempBuff);
		tokenCnt = tokenize_entrees_Kr(flineCnt,RLine,heads);

		if(!suffixeMode && 
			(heads->EC_flechi == 0) && 
			(*heads->EC_flechi == '\0'))
			lineErrMess(flineCnt,"not accept the data null at racine");
		arbre.insert(curArbreIdx,heads);
		for(;tokenCnt > 0;tokenCnt--){
			wp = heads;
			heads=heads->next;
			delete wp;
		}
		
		flineCnt++;
		if(!(flineCnt % 1000))
			u_printf("\r%d line read",flineCnt);
	}
	u_printf("\n%s: %d line read\n",fname,flineCnt);

	fclose(f);
	return(flineCnt);
}



static void  get_compressed_token(unichar* inflected,unichar* lemma,unichar* res) 
{
int prefix=get_longuest_prefix(inflected,lemma);
int a_effacer=u_strlen(inflected)-prefix;
int l_lemma=u_strlen(lemma);
if (l_lemma==1 && ((lemma[0]==' ') || (lemma[0]== '-')) &&
    u_strlen(inflected)==1 && (inflected[0]==' ' || inflected[0]=='-')) {
    // if we have 2 separators, we write it rawly to make the INF file visible
    // ex: "jean-pierre,jean-pierre.N" => "0-0.N" instead of "000.N"
    res[0]=lemma[0];
    res[1]='\0';
    return;
}
int l_NB=u_sprintf(res,"%d",a_effacer);
int i;
int j=0;
for (i=0;i<(l_lemma-prefix);i++) {
    if ((lemma[i+prefix]>= '0' && lemma[i+prefix]<= '9')
        || lemma[i+prefix]==',' || lemma[i+prefix]=='.'
        || lemma[i+prefix]== '\\') {
       res[j+l_NB]='\\';
       j++;
    }
    res[j+l_NB]=lemma[i+prefix];
    j++;
}
res[j+l_NB]= '\0';
}
//
//	the line with the form ;,,,,,;...;,,,,,
//	convert struct morph 
//
static unichar u_zero_string[] = {(unichar)'0',(unichar)'\0'};
static int
tokenize_entrees_Kr(int lineCnt,unsigned short *iline,class dicLines *&head)
{
	int scanIdx = 0;
	int openF = 0;
	int segIdx = 0;
	int saveIdx;
	int i;
	dicLines *tail = 0;
	unichar *s = Stmp;
	unichar *sPtr[MAX_NUM_ENTREE_COMP_LINE];// 0:flechi 
											  // 1:org
											  // 2:canonique
											  // 3:info
											  // 4:suf
	int elecount = 0;


	if(iline[0] != ';') return 0;
	do {
		
		switch(iline[scanIdx]){
		case ',': 
			if(openF){
				s[saveIdx++] = iline[scanIdx];
				if(saveIdx > MAX_LINE_ELEMENT)	lineErrMess(lineCnt,"linecount error");
				break;
			}
			if(segIdx > MAX_NUM_ENTREE_COMP_LINE) lineErrMess(lineCnt,"line count error");
			s[saveIdx++] = '\0';
			sPtr[segIdx++] = &s[saveIdx];
            break;

		case '<': openF = 1; 
			s[saveIdx++] = iline[scanIdx];
			break;
		case '>': openF = 0; 
			s[saveIdx++] = iline[scanIdx];
			break;
		case ';':
		case '\0':
			{
				if(head){
					s[saveIdx] = 0;
					if((segIdx != MAX_LINE_ELEMENT_DELAKR) ||
					     ((!suffixeMode) && (elecount == 0)&&( sPtr[0][0]== 0)))
						lineErrMess(lineCnt,"format error");
					Etmp[0] = 0;

if(debugfile){
for( i = 0; i <  MAX_NUM_ENTREE_COMP_LINE;i++)
u_fprintf(debugfile,"%S,\n", sPtr[i]);
}
					if(*sPtr[4] == '-'){// control for suffixe 
						sPtr[4]++;
						sPtr[2] = u_zero_string;
					} else 
                    if(*sPtr[0] && *sPtr[2]){
					    if((*sPtr[2] == '-') || (*sPtr[2] == '+')){
					        Etmp[0] = *sPtr[2];
					    	get_compressed_token(sPtr[0],&sPtr[2][1],&Etmp[1]);
					    } else {
					    	get_compressed_token(sPtr[0],sPtr[2],Etmp);
						}
                        sPtr[2] = Etmp;
						
					}
                    // flechi,origin,canonique,info,suff
if(debugfile){
for( i = 0; i <  MAX_NUM_ENTREE_COMP_LINE;i++)
u_fprintf(debugfile,"%S,\n", sPtr[i]);
}
					tail->makeInfField((unichar*)sPtr[0],
                              (unichar*)sPtr[1],
                              (unichar*)sPtr[2],
                              (unichar*)sPtr[3],
                              (unichar*)sPtr[4]);
					if(iline[scanIdx]){
						tail->next = new class dicLines;
						tail = tail->next;
						elecount++;
					}
				} else {
					head = tail = new class dicLines;
					elecount++;
				}
				for( i = 0; i <  MAX_NUM_ENTREE_COMP_LINE;i++) sPtr[i] =0;
				openF = 0;
				segIdx = 0;
				saveIdx = 0;
				sPtr[segIdx++] = &s[saveIdx];
			}
			break;
		case '\\':
			s[saveIdx++] = iline[scanIdx++];
			if(iline[scanIdx] == 0) lineErrMess(lineCnt,"");
		default:
			s[saveIdx++] = iline[scanIdx];
		}
	} while( iline[scanIdx++]);

	return elecount;
}




