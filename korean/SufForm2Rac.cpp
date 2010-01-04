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
#include "Unicode.h"
#include "etc.h"
#include "Copyright.h"
#include "File.h"
#include "IOBuffer.h"
#include "Error.h"

// using namespace std;

//
// change the format which is get from graph to format of list form
//
struct SufForm2Rac_ctx {
    unichar saveOrg[2][1024];
    unichar canonique[1024];
    unichar flechi[1024];
//    unichar suffixe[1024];
    char desFileName[2048];
    char tfn[2048];
    char ttfn[2048];
    int    converMapExistFlag;
    char inputFilePath[2048];
};

const char* usage_SufForm2Rac =
    "SufForm2Rac [-m converTable] [-l] [-o ofilename] fname "\
    " fname : name of the input file name with extension \".fst2\"\r\n"\
    " -l : input file is liste of input files \r\n"\
    " -m convTable : change characters\r\n"\
    " \r\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_SufForm2Rac);
}

struct fileListe {
char *filename;
struct fileListe *next;
};

class fileLinkListe {
    struct fileListe *headFileList,*tailFileList;
public:
    fileLinkListe(){
        headFileList = tailFileList = 0;
    };
    ~fileLinkListe(){
        struct fileListe *wp;
        while(headFileList){
                wp = headFileList->next;
				delete [] headFileList->filename;
                delete headFileList;
                headFileList = wp;
        }
    };
    struct fileListe *getHead(){return(headFileList);};
    char ofdirName[1024];
    char ofExt[16];
    char ofnameOnly[512];
    void fileNameSet(char *ifn,char *ofn)
    {
        char tmp[512];
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
    	if(ofnameOnly[0]== 0) fatal_error("ofile name not correct\n");
    }
    void makeOfileName(char *des,char *fn,char *ext)
    {
        strcpy(des,ofdirName);
        if(fn) strcat(des,fn);
        else strcat(des,ofnameOnly);
        if(ext) strcat(des,ext);
        else strcat(des,ofExt);
    }
                
    
    void ajouteList(char *fname)
    {
        if(!headFileList){
            headFileList = tailFileList = new struct fileListe;
        } else {
            tailFileList->next = new struct fileListe;
            tailFileList = tailFileList->next;               
        }            
        tailFileList->filename = new char[strlen(fname)+1];
        tailFileList->next = 0;
        strcpy(tailFileList->filename,fname);
    }
    void ajouteFromFile(char *fa)
    {
        U_FILE *f = u_fopen(BINARY,fa,U_READ);
        if(!f) fopenErrMessage(fa);
        char line[1024];
        char *wp;
        while(af_fgets(line,1024,f->f)){
                if(line[0] == ' ') continue;
            wp = line;
            while(*wp){
                if(*wp == ' ') break;
                if(*wp == '\n') break;
                if(*wp == 0xa) break;
                if(*wp == 0xd) break;
                wp++;
            }
            *wp = 0;
            if(wp == line) fatal_error("list file format error\n");
            ajouteList(line);
            u_printf("line[%s]\n",line);
        }
        u_fclose(f);
    }

};

static    void changeFile(struct SufForm2Rac_ctx*,Encoding encoding,int write_bom,int MASK_ENCODING_COMPATIBILITY,char *,char *);




int main_SufForm2Rac(int argc, char *argv[]) {

    int iargIndex = 1;
    int listFileFlag = 0;
    char *ofilename =0;
    class fileLinkListe fileStock;

    struct SufForm2Rac_ctx sufForm2Rac_ctx;
    memset(&sufForm2Rac_ctx,0,sizeof(struct SufForm2Rac_ctx));

    Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
    int bom_output = DEFAULT_BOM_OUTPUT;
    int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;

    int remFlag = 0;
    sufForm2Rac_ctx.converMapExistFlag = 0;
    if(argc == 1) {
       usage();
       return 0;
    }

    converTableInit();
    while(iargIndex < argc-1){
    	if(*argv[iargIndex] != '-') break;
    	switch(argv[iargIndex][1]){
    	case 'm': iargIndex++;
    	    loadChangeFileToTable(argv[iargIndex],mask_encoding_compatibility_input);
    	    sufForm2Rac_ctx.converMapExistFlag = 1;
    		break;
    	case 'l':listFileFlag = 1; break;
    	case 'o': 
               iargIndex++; 
               ofilename = argv[iargIndex];
               if(remFlag) remFlag = 0;
           break;
        case 'r': // replace
                if(!ofilename) remFlag = 1;
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
    	iargIndex++;
    }
    fileStock.fileNameSet(argv[iargIndex],ofilename);
    
    if(listFileFlag){
        fileStock.ajouteFromFile(argv[iargIndex]);
        get_path(argv[iargIndex],sufForm2Rac_ctx.inputFilePath);
    } else {
        get_path(argv[iargIndex],sufForm2Rac_ctx.inputFilePath);
        remove_path(argv[iargIndex],sufForm2Rac_ctx.tfn);
        fileStock.ajouteList(sufForm2Rac_ctx.tfn);    
    }
    struct fileListe *wpointer = fileStock.getHead();
    while(wpointer){
        remove_path(wpointer->filename,sufForm2Rac_ctx.tfn);
        u_printf("[%s][%s]\n",sufForm2Rac_ctx.tfn,wpointer->filename);
        remove_extension(sufForm2Rac_ctx.tfn,sufForm2Rac_ctx.ttfn);
        u_printf("[%s][%s]\n",sufForm2Rac_ctx.tfn,sufForm2Rac_ctx.ttfn);
        fileStock.makeOfileName(sufForm2Rac_ctx.desFileName,sufForm2Rac_ctx.ttfn,0);
        strcpy(sufForm2Rac_ctx.tfn,sufForm2Rac_ctx.inputFilePath);
        u_printf("[%s][%s]\n",sufForm2Rac_ctx.tfn,sufForm2Rac_ctx.desFileName);
        strcat(sufForm2Rac_ctx.tfn,wpointer->filename);
        u_printf("[%s][%s]\n",sufForm2Rac_ctx.tfn,sufForm2Rac_ctx.desFileName);
        changeFile(&sufForm2Rac_ctx,encoding_output,bom_output,mask_encoding_compatibility_input,sufForm2Rac_ctx.tfn,sufForm2Rac_ctx.desFileName);
        wpointer = wpointer->next;
    }
  u_printf("Done\n");
  return 0;
}

//static  char ofileNameBuff[1024];
static void changeFile(struct SufForm2Rac_ctx* p_sufForm2Rac_ctx, Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input,char  *ifname,char *ofname)
{
U_FILE *ifile,*ofile;
  int c;
  int countComma;
  int openFlag;
  int index = 0;
  int offset;
  int opened;
  int changeFlag;
  int flechiIndex = 0;
  unichar *wp,*cwp,*twp;
  
  openFlag = 0;
  countComma = 0;
  offset = 0;
  opened = 0;
  ifile = u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,ifname,U_READ);
  if(!ifile)
    fopenErrMessage(ifname);
  ofile = u_fopen_creating_versatile_encoding(encoding_output,bom_output,ofname,U_WRITE);
  if(!ofile)
    fatal_error("Cannot open %s\n",ofname);
//printf("%s %s\n",ifname,ofname);
  while((c = u_fgetc(ifile)) != EOF){
    switch(c){
    case L',':
        if(openFlag){
           p_sufForm2Rac_ctx->saveOrg[index][offset] = 0;
         index++;
         offset = 0;
           if(index >= 2) fatal_error("format error\n");
           break;
         }
         switch(countComma){
         case 3:
                u_fputc((unichar)c,ofile);                  
                if(!opened){u_fputc((unichar)c,ofile);u_fputc((unichar)c,ofile);u_fputc((unichar)c,ofile);};                
                break;
         case 0:
                  p_sufForm2Rac_ctx->flechi[flechiIndex] = 0;
         case 1:
         case 2:
                  break;
         default:
                  fatal_error("format error\n");
         }
          countComma++;
        break;
    case L'<':
        opened = 1;
        openFlag = 1;
        index = 0;
        offset = 0;
        p_sufForm2Rac_ctx->saveOrg[index][offset] =0;
        break;
    case L'>':
        p_sufForm2Rac_ctx->saveOrg[index][offset++] = 0;
        changeFlag = 0;
        wp = p_sufForm2Rac_ctx->saveOrg[0];
        switch(*wp){
        case L'+':
        case L'-':
        case L'=':
                wp++;
                if(*wp != 0) break;
        case L'\0':
                u_strcpy(wp,p_sufForm2Rac_ctx->flechi);
                break;
        default:
                break;
        }
        if(p_sufForm2Rac_ctx->converMapExistFlag){
            twp = p_sufForm2Rac_ctx->canonique;
            wp = p_sufForm2Rac_ctx->saveOrg[0];
            while(*wp){
                cwp = getConvTable(*wp);
                if(cwp)
                {
                     changeFlag++;
                     while(*cwp) *twp++ = *cwp++;
                } else {
                    *twp++ = *wp;
                }
                wp++;
            }
            *twp=0;
            
            if(changeFlag){
                u_fprintf(ofile,",%S,%S,%S",p_sufForm2Rac_ctx->saveOrg[0],p_sufForm2Rac_ctx->canonique,p_sufForm2Rac_ctx->saveOrg[1]);
            } else {
                u_fprintf(ofile,",,%S,%S",p_sufForm2Rac_ctx->saveOrg[0],p_sufForm2Rac_ctx->saveOrg[1]);
            }
        } else 
            u_fprintf(ofile,",,%S,%S",p_sufForm2Rac_ctx->saveOrg[0],p_sufForm2Rac_ctx->saveOrg[1]);
        openFlag = 0;
        break;
    case L';':
        opened = 0;
        countComma = 0;
        flechiIndex = 0;
        p_sufForm2Rac_ctx->flechi[flechiIndex] = 0;
        u_fputc((unichar)c,ofile);
        break;
    default:
        if(openFlag)
                p_sufForm2Rac_ctx->saveOrg[index][offset++] = (unichar)c;
        else {
           if(!countComma) p_sufForm2Rac_ctx->flechi[flechiIndex++] = (unichar)c;
           u_fputc((unichar)c,ofile);
        }
    }
  }
  u_fclose(ifile);
  u_fclose(ofile);
}
