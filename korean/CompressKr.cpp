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
// using namespace std;
#include "Unicode.h"
#include "File.h"
#include "Copyright.h"
#include "bin.h"
#include "etc.h"
#include "formation_dic_line.h"
#include "bin3.h"
#include "IOBuffer.h"
#include "Error.h"


const char* usage_CompressKr =
         "Usage: CompressKr [-s] [-g] [-o ofilename] [-l] <dictionary>\n" \
         "  -s  : suffixes dictionnary, indicate the type of dictionnary, Suffixe or Racine\n" \
         "  -g  : dicitionary from the graph\n" \
         "  -l  : <dictionnary> file has list of dictionnarys \n" \
         "   <dictionary> : any unicode DELAF or DELACF dictionary\n" \
         "  -o  : set output file\n" \
         "Compresses a dictionary into an finite state automaton. This automaton\n" \
         "is stored is a .bin file,  the associated flexional codes are\n" \
         "written in a .inf file and the list of automates at .aut file\n\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_CompressKr);
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

static int read_DELA_to_DICO(struct CompressKr_ctx* p_compressKr_ctx,class arbre_string3 &arbre,int a,char *fname,int);
//static void read_FST_to_DICO(class arbre_string3 &arbre,int a,char *fname); 



//
//	printf out list of initial states of the automate
//

//	the form of korean dico
//	flexion form,form orgin,canonical form, name of the suffix transductor, les information
//
#define MAX_LINE_ELEMENT_DELAKR	5
#define MAX_LINE_ELEMENT	1024


struct CompressKr_ctx {
    U_FILE *debugfile;
    unichar UtempBuff[512];

    unichar segs[MAX_LINE_ELEMENT_DELAKR+1][MAX_LINE_ELEMENT];
    unichar Etmp[4096];
    unichar Stmp[4096];


    class arbre_string0 MotEtr;
    char *cfilename;
    char templine[512];

    int suffixeMode;	// racine or suffixe
    int grapheMode;
    char *ofilename;
};


static void lineErrMess(struct CompressKr_ctx* p_compressKr_ctx,int lineCnt,const char *msg)
{
	error("%s file at line %d\n",p_compressKr_ctx->cfilename,lineCnt);
	error("<<%S>>\nhas syntax error",p_compressKr_ctx->UtempBuff);
	fatal_error("%s\n",msg);
}

static void make_compress_files(struct CompressKr_ctx* p_compressKr_ctx,Encoding encoding_output,int bom_output,
                                int mask_encoding_compatibility_input,
                                char *listFilename,int flag);
static int tokenize_entrees_Kr(struct CompressKr_ctx* p_compressKr_ctx,int lcnt,unsigned short *line,class dicLines *& head);



int main_CompressKr(int argc, char *argv[]) {

struct CompressKr_ctx compressKr_ctx;
memset(&compressKr_ctx,0,sizeof(struct CompressKr_ctx));

Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;

int argIdx = 1;
int listFormFlag = 0;	// filename is dictionnary
	compressKr_ctx.debugfile = 0;
 compressKr_ctx.grapheMode = 0;
compressKr_ctx.suffixeMode = 0;
	compressKr_ctx.ofilename  =0;
	if(argc == 1) {
       usage();
       return 0;
    }
//printf("%s\n",setlocale(LC_ALL,"Korean_korea.949"));
	while( argIdx < argc-1){
//printf("%s\n",argv[argIdx]);
		if(argv[argIdx][0] == '-'){
			switch(argv[argIdx][1]){
			case 'D': 
				compressKr_ctx.debugfile = u_fopen_creating_versatile_encoding(encoding_output,bom_output,"dd.txt",U_WRITE);
				break;
			case 'd': 
				compressKr_ctx.debugfile = U_STDOUT;
				break;
			case 'l': listFormFlag++;break;
			case 's':compressKr_ctx.suffixeMode++; break;
			case 'g':compressKr_ctx.grapheMode++; break;
			case 'o':argIdx++; compressKr_ctx.ofilename = argv[argIdx];break;
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
			default:
				usage();
				return 1;
			}
		}
		argIdx++;
	}
	
	if(argIdx != argc -1) {
	   usage();
	   return 1;
	}
	make_compress_files(&compressKr_ctx,encoding_output,bom_output,mask_encoding_compatibility_input,argv[argIdx],listFormFlag);
	if(compressKr_ctx.debugfile) u_fclose(compressKr_ctx.debugfile);
	return(0);
}
/*
 */
static void read_list_files(struct CompressKr_ctx* p_compressKr_ctx,simpleL<char *> &rd,char *filename)
{
	int sc;
	char fullPathName[2048];
	char pathName[1024];
	char *t;

	U_FILE *f=u_fopen(BINARY,filename,U_READ);
	
	if(!f) {
		fatal_error("file %s open fail\n",filename);
	}
	get_path(filename,pathName);
	while(af_fgets(p_compressKr_ctx->templine,256,f->f)){
		sc =0;
		while(p_compressKr_ctx->templine[sc]){
			if( (p_compressKr_ctx->templine[sc] == 0x0d) ||
				(p_compressKr_ctx->templine[sc] == 0x0a)||
				(p_compressKr_ctx->templine[sc] == '#') ){
				p_compressKr_ctx->templine[sc] = 0;
				break;
			}
			sc++;
		}
		if((p_compressKr_ctx->templine[0] == ' ') ||
		   (p_compressKr_ctx->templine[0]== 0) )
			continue;

		strcpy(fullPathName,pathName);
		strcat(fullPathName,p_compressKr_ctx->templine);
		
		t = new char[strlen(fullPathName)+1];
		strcpy(t,fullPathName);
		rd.put(t);
	}
	u_fclose(f);
}

//
//
static void 
make_compress_files(struct CompressKr_ctx* p_compressKr_ctx,
                    Encoding encoding_output,int bom_output,
                    int mask_encoding_compatibility_input,
                    char *listFileName,int listFormFlag)
{
	class arbre_string3 arbres;
	simpleL<char *> readFiles;
	char *fnamePtr;
	char extension[16];
	unichar *tmp;
	int autoStartIndex = 0;

    arbres.setEncoding(encoding_output,bom_output,mask_encoding_compatibility_input);
	
	int lineCnt = 0;
	if(listFormFlag){
		read_list_files(p_compressKr_ctx,readFiles,listFileName);
	} else {
		readFiles.put(listFileName);
	}
	readFiles.reset();
	if(!(p_compressKr_ctx->suffixeMode)) {	// racine
		u_strcpy(p_compressKr_ctx->segs[0],listFileName);
		getFileName(p_compressKr_ctx->segs[1],p_compressKr_ctx->segs[0]);
		tmp = new unichar[u_strlen(p_compressKr_ctx->segs[1])+1];
		u_strcpy(tmp,p_compressKr_ctx->segs[1]);
		autoStartIndex = arbres.new_arbre((unichar*)tmp);
	}
	// suffixe
	int readFileCnt=0;
	while( (fnamePtr = readFiles.getNext()) != 0) {
		if(p_compressKr_ctx->suffixeMode){	// suffixe
			u_strcpy(p_compressKr_ctx->segs[0],fnamePtr);
			getFileName(p_compressKr_ctx->segs[1],p_compressKr_ctx->segs[0]);
			tmp = new unichar[u_strlen(p_compressKr_ctx->segs[1])+1];
			u_strcpy(tmp,p_compressKr_ctx->segs[1]);
			autoStartIndex = arbres.new_arbre((unichar *)tmp);
		}
		get_extension(fnamePtr,extension);
      u_printf("\n%s load\n",fnamePtr);
        lineCnt += read_DELA_to_DICO(p_compressKr_ctx,arbres,autoStartIndex,fnamePtr,mask_encoding_compatibility_input);
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

	if(p_compressKr_ctx->debugfile) {
		int i;
		for( i = 0; i < arbres.getArbreCnt();i++){
			u_fprintf(p_compressKr_ctx->debugfile,"%i th===\n",i);
//			arbres.prNode(p_compressKr_ctx->debugfile,arbrelistOut);
		}
	}
	
	if(p_compressKr_ctx->ofilename)
	 remove_extension(p_compressKr_ctx->ofilename,p_compressKr_ctx->templine);
	else	 
	 remove_extension(listFileName,p_compressKr_ctx->templine);
	arbres.toBinTr(p_compressKr_ctx->templine,p_compressKr_ctx->suffixeMode);

	if(p_compressKr_ctx->debugfile) u_fclose(p_compressKr_ctx->debugfile);
}

//
//	
//
//
static int
read_DELA_to_DICO(struct CompressKr_ctx* p_compressKr_ctx,
                  class arbre_string3 &arbre,int curArbreIdx,char *fname,int mask_encoding_compatibility_input)
{
	U_FILE *f;
	int tokenCnt;
	int flineCnt = 0;
	class dicLines *heads,*wp;
	unichar RLine[2048];
	f=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,fname,U_READ);
	if(!f)	fopenErrMessage(fname);
	p_compressKr_ctx->cfilename = fname;
	u_printf("Read File %s\n",fname);
	while(EOF!=u_fgets(p_compressKr_ctx->UtempBuff,f)){
		if( (p_compressKr_ctx->UtempBuff[0] == '\0') ||(p_compressKr_ctx->UtempBuff[0] == ' '))
			continue; // comment line
		heads = 0; u_strcpy(RLine,p_compressKr_ctx->UtempBuff);
		tokenCnt = tokenize_entrees_Kr(p_compressKr_ctx,flineCnt,RLine,heads);

		if(!(p_compressKr_ctx->suffixeMode) && 
			(heads->EC_flechi == 0) && 
			(*heads->EC_flechi == '\0'))
			lineErrMess(p_compressKr_ctx,flineCnt,"not accept the data null at racine");
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

	u_fclose(f);
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
    tokenize_entrees_Kr(struct CompressKr_ctx* p_compressKr_ctx,int lineCnt,unsigned short *iline,class dicLines *&head)
{
	int scanIdx = 0;
	int openF = 0;
	int segIdx = 0;
	int saveIdx = 0;
	int i;
	dicLines *tail = 0;
	unichar *s = p_compressKr_ctx->Stmp;
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
				if(saveIdx > MAX_LINE_ELEMENT)	lineErrMess(p_compressKr_ctx,lineCnt,"linecount error");
				break;
			}
			if(segIdx > MAX_NUM_ENTREE_COMP_LINE) lineErrMess(p_compressKr_ctx,lineCnt,"line count error");
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
					     ((!(p_compressKr_ctx->suffixeMode)) && (elecount == 0)&&( sPtr[0][0]== 0)))
						lineErrMess(p_compressKr_ctx,lineCnt,"format error");
					p_compressKr_ctx->Etmp[0] = 0;

if(p_compressKr_ctx->debugfile){
for( i = 0; i <  MAX_NUM_ENTREE_COMP_LINE;i++)
u_fprintf(p_compressKr_ctx->debugfile,"%S,\n", sPtr[i]);
}
					if(*sPtr[4] == '-'){// control for suffixe 
						sPtr[4]++;
						sPtr[2] = u_zero_string;
					} else 
                    if(*sPtr[0] && *sPtr[2]){
					    if((*sPtr[2] == '-') || (*sPtr[2] == '+')){
					        p_compressKr_ctx->Etmp[0] = *sPtr[2];
					    	get_compressed_token(sPtr[0],&sPtr[2][1],&(p_compressKr_ctx->Etmp[1]));
					    } else {
					    	get_compressed_token(sPtr[0],sPtr[2],p_compressKr_ctx->Etmp);
						}
                        sPtr[2] = p_compressKr_ctx->Etmp;
						
					}
                    // flechi,origin,canonique,info,suff
if(p_compressKr_ctx->debugfile){
for( i = 0; i <  MAX_NUM_ENTREE_COMP_LINE;i++)
u_fprintf(p_compressKr_ctx->debugfile,"%S,\n", sPtr[i]);
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
			if(iline[scanIdx] == 0) lineErrMess(p_compressKr_ctx,lineCnt,"");
		default:
			s[saveIdx++] = iline[scanIdx];
		}
	} while( iline[scanIdx++]);

	return elecount;
}
