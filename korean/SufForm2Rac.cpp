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
#include "Unicode.h"
#include "etc.h"
#include "Copyright.h"
#include "File.h"
#include "IOBuffer.h"
#include "Error.h"

using namespace std;

//
// change the format which is get from graph to format of list form
//
static unichar saveOrg[2][1024];
static unichar canonique[1024];
static unichar flechi[1024];
//static unichar suffixe[1024];


static void usage()
{
    u_printf("%s",COPYRIGHT);
    u_printf("Usage:\r\n");
    u_printf(
    "SufForm2Rac [-m converTable] [-l] [-o ofilename] fname "\
    " fname : name of the input file name with extension \".fst2\"\r\n"\
    " -l : input file is liste of input files \r\n"\
    " -m convTable : change characters\r\n"\
    " \r\n");
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
                delete headFileList->filename;
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
        while(fgets(line,1024,f->f->fin)){
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

static    void changeFile(char *,char *);
static char desFileName[2048];
static char tfn[2048];
static char ttfn[2048];
static int    converMapExistFlag;
static char inputFilePath[2048];



int main_SufForm2Rac(int argc, char **argv) {

    int iargIndex = 1;
    int listFileFlag = 0;
    char *ofilename =0;
    class fileLinkListe fileStock;
    int remFlag = 0;
    converMapExistFlag = 0;
    if(argc == 1) {
       usage();
       return 0;
    }

    converTableInit();
    while(iargIndex < argc-1){
    	if(*argv[iargIndex] != '-') break;
    	switch(argv[iargIndex][1]){
    	case 'm': iargIndex++;
    	    loadChangeFileToTable(argv[iargIndex]);
    	    converMapExistFlag = 1;
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
    	default:
    	   usage();
    	   return 1;
    	}
    	iargIndex++;
    }
    fileStock.fileNameSet(argv[iargIndex],ofilename);
    
    if(listFileFlag){
        fileStock.ajouteFromFile(argv[iargIndex]);
        get_path(argv[iargIndex],inputFilePath);
    } else {
        get_path(argv[iargIndex],inputFilePath);
        remove_path(argv[iargIndex],tfn);
        fileStock.ajouteList(tfn);    
    }
    struct fileListe *wpointer = fileStock.getHead();
    while(wpointer){
        remove_path(wpointer->filename,tfn);
        u_printf("[%s][%s]\n",tfn,wpointer->filename);
        remove_extension(tfn,ttfn);
        u_printf("[%s][%s]\n",tfn,ttfn);
        fileStock.makeOfileName(desFileName,ttfn,0);
        strcpy(tfn,inputFilePath);
        u_printf("[%s][%s]\n",tfn,desFileName);
        strcat(tfn,wpointer->filename);
        u_printf("[%s][%s]\n",tfn,desFileName);
        changeFile(tfn,desFileName);
        wpointer = wpointer->next;
    }
  u_printf("Done\n");
  return 0;
     
}
//static  char ofileNameBuff[1024];
static void changeFile(char  *ifname,char *ofname)
{
U_FILE *ifile,*ofile;
  int c;
  int countComma;
  int openFlag;
  int index;
  int offset;
  int opened;
  int changeFlag;
  int flechiIndex;
  unichar *wp,*cwp,*twp;
  
  openFlag = 0;
  countComma = 0;
  offset = 0;
  opened = 0;
  if(!(ifile = u_fopen(UTF16_LE,ifname,U_READ)))
    fopenErrMessage(ifname);
  if(!(ofile = u_fopen(UTF16_LE,ofname,U_WRITE)))
    fatal_error("Cannot open %s\n",ofname);
//printf("%s %s\n",ifname,ofname);
  while((c = u_fgetc(ifile)) != EOF){
    switch(c){
    case L',':
        if(openFlag){
           saveOrg[index][offset] = 0;
         index++;
         offset = 0;
           if(index >= 2) fatal_error("format error\n");
           break;
         }
         switch(countComma){
         case 3:
                u_fputc(c,ofile);                  
                if(!opened){u_fputc(c,ofile);u_fputc(c,ofile);u_fputc(c,ofile);};                
                break;
         case 0:
                  flechi[flechiIndex] = 0;
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
        saveOrg[index][offset] =0;
        break;
    case L'>':
        saveOrg[index][offset++] = 0;
        changeFlag = 0;
        wp = saveOrg[0];
        switch(*wp){
        case L'+':
        case L'-':
        case L'=':
                wp++;
                if(*wp != 0) break;
        case L'\0':
                u_strcpy(wp,flechi);
                break;
        default:
                break;
        }
        if(converMapExistFlag){
            twp = canonique;
            wp = saveOrg[0];
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
                u_fprintf(ofile,",%S,%S,%S",saveOrg[0],canonique,saveOrg[1]);
            } else {
                u_fprintf(ofile,",,%S,%S",saveOrg[0],saveOrg[1]);
            }
        } else 
            u_fprintf(ofile,",,%S,%S",saveOrg[0],saveOrg[1]);
        openFlag = 0;
        break;
    case L';':
        opened = 0;
        countComma = 0;
        flechiIndex = 0;
        flechi[flechiIndex] = 0;
        u_fputc(c,ofile);
        break;
    default:
        if(openFlag)
                saveOrg[index][offset++] = c;
        else {
           if(!countComma)flechi[flechiIndex++] = c;
           u_fputc(c,ofile);
        }
    }
  }
  u_fclose(ifile);
  u_fclose(ofile);
}
