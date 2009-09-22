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

//
// sort morphemes after the consultation of dictionaries
//
// sortMorph
//     tokens file, morphems of a word file,  sequence de morpheme file

#include <stdio.h>
#include "Unicode.h"
#include "Copyright.h"
#include "File.h"
#include "String_hash2.h"
#include "etc.h"
#include "IOBuffer.h"



const char* usage_SortMorph =         
"SortMorph tfile mfile mcfile\r\n"\
" tfile : name of lists of tokens file\r\n"\
" sqfile : name of lists of sequences of morphemes in a word\r\n"\
" mcfile : name of lists of morphemes\r\n"\
"        : Output file DLC, DLF, ERR\r\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_SortMorph);
}


static     U_FILE *fout;
static char ftemp[4096];
static char pathNameSave[4096];
static void getPrOutWithValue(void * arg0,void *arg1,void *arg2)
{
    unichar *word = (unichar *)arg0;
    unichar *data = (unichar *)arg2;
    u_fprintf(fout,"%S %S\n",word+1,data);
}
static void getPrOut(void * arg0,void *arg1,void *arg2)
{
    unichar *word = (unichar *)arg0;

    u_fprintf(fout,"%S\n",word+1);
}


struct locdefi {
	 unichar *fl;
	 unichar *ca;
	 unichar *inf;
};
static locdefi **table;
static int *freqtable;
static void getPrOutMorpheme(void * arg0,void *arg1,void *arg2)
{
    intptr_t index = (intptr_t)arg2;
    u_fprintf(fout,"%S,%S.%S\t%d\n",table[index]->fl, table[index]->ca,
               table[index]->inf, freqtable[index]);
}

	static int partition_pour_quicksort_by_frequence(int m, int n)
	{
		int pivot;
		int tmp;
		struct locdefi* tmp_char;
		int i = m-1;
		int j = n+1;         // indice final du pivot
		pivot=freqtable[(m+n)/2];
		while (true) {
			do j--;while ((j>(m-1))&&(pivot>freqtable[j]));
			do i++;while ((i<n+1)&&(freqtable[i]>pivot));
			if (i<j) {
				tmp=freqtable[i];
				freqtable[i]=freqtable[j];
				freqtable[j]=tmp;

				tmp_char=table[i];
				table[i]=table[j];
				table[j]=tmp_char;
			} else
				return j;
		}
	}

	void static quicksort_by_frequence(int debut,int fin)
	{
		int p;
		if (debut<fin) {
			p=partition_pour_quicksort_by_frequence(debut,fin);
			quicksort_by_frequence(debut,p);
			quicksort_by_frequence(p+1,fin);
		}
	}
static void
sortMorphemTable(char *f)
{
	U_FILE *fptr;
	int count = 0;
	int tcount;
	unichar *wp,*mem;
class arbre_string00 parFlechi, parCano;

	if((fptr = u_fopen(BINARY,f,U_READ)) ==0 )	{
	   fopenErrMessage(f);
	}
	fseek(fptr,0,SEEK_END);
	int sizeFile =ftell(fptr);
	sizeFile /= 2;
	mem = new unichar[sizeFile];
	if(!mem) fatal_error("mem alloc fail\n");
	fseek(fptr,2,SEEK_SET);
	if(!u_fread_raw(mem,sizeFile-1,fptr)) fatal_error("Read morpheme table fail\n");
	mem[sizeFile-1] = 0;
   u_fclose(fptr);

	wp = mem;
	for(;(*wp >= L'0') && (*wp <= '9') ;wp++) count = count * 10 + *wp - L'0';

	while((*wp == L'\r') || (*wp == L'\n')) wp++;

	table = new struct locdefi *[count];
	freqtable = new int[count];
	if(!table) fatal_error("morphems table mem alloc fail\n");
	unichar *curoffset = wp;
	int index  = 0;
	while(*wp) {
	    if(*wp != L'{'){
	      if(*wp == '<'){
                 curoffset = wp;
	             while(*wp != L'>'){if(!*wp) fatal_error("illegal format 1\n");wp++;}
	             wp++;
	             *wp++ = 0;
                 while((*wp == L' ' ) || (*wp == '\t'))
                  {if(!*wp) fatal_error("illegal format 2\n");wp++;}

	             while((*wp == L'\r') && (*wp == L'\n')) wp++;

//	             table[index] = new struct locdefi;
//	             table[index]->fl = curoffset;
//	             table[index]->ca = curoffset;
//	             table[index]->inf = u_null_string;
        	    for(tcount = 0;(*wp >= L'0') && (*wp <= '9') ;wp++)
                   tcount = tcount * 10 + *wp - L'0';
//                freqtable[index] = tcount;
                while((*wp == L'\r') || (*wp == L'\n')) wp++;
//                parFlechi.put(curoffset,index);
//                parCano.put(curoffset,index);

//                index++;
	            continue;
          } else if ((*wp == L'\r') || (*wp == L'\n')) {
               wp++;continue;
               }
          fatal_error("illegal format 3\n");
        }
	    wp++;
	    table[index] = new struct locdefi;
        curoffset = wp;
	    while(*wp != L','){if(!*wp) fatal_error("illegal format\n");wp++;}
	    *wp++ = 0;
	    table[index]->fl = curoffset;

        curoffset = wp;
	    while(*wp != L'.'){if(!*wp) fatal_error("illegal format\n");wp++;}
	    *wp++ = 0;
	    table[index]->ca = curoffset;

	    curoffset = wp;
	    while(*wp != L'}'){if(!*wp) fatal_error("illegal format\n");wp++;}
	    *wp++ = 0;
	    table[index]->inf = curoffset;
	    while((*wp == L' ' ) || (*wp == '\t'))
          {if(!*wp) fatal_error("illegal format\n");wp++;}

	    curoffset = wp;
	    for(tcount = 0;(*wp >= L'0') && (*wp <= '9') ;wp++)
           tcount = tcount * 10 + *wp - L'0';
        freqtable[index] = tcount;
        while((*wp == L'\r') || (*wp == L'\n')) wp++;
        parFlechi.put(table[index]->fl,(void *)index);
        parCano.put(table[index]->ca,(void *)index);
        index++;

	};

    strcpy(ftemp,pathNameSave);
    strcat(ftemp,"morph_by_flei.txt");
    fout = u_fopen(UTF16_LE,ftemp,U_WRITE);
    if(!fout) fopenErrMessage(ftemp);
    parFlechi.explore_tout_leaf((release_f)getPrOutMorpheme);
    u_fclose(fout);

    strcpy(ftemp,pathNameSave);
    strcat(ftemp,"morph_by_cano.txt");
    fout = u_fopen(UTF16_LE,ftemp,U_WRITE);
    if(!fout) fopenErrMessage(ftemp);
    parCano.explore_tout_leaf((release_f)getPrOutMorpheme);
    u_fclose(fout);
    strcpy(ftemp,pathNameSave);
    strcat(ftemp,"morph_by_freq.txt");
    fout = u_fopen(UTF16_LE,ftemp,U_WRITE);
    if(!fout) fopenErrMessage(ftemp);

    quicksort_by_frequence(0,index-1);
    int i;

    for(i = 0; i < index;i++){
        u_fprintf(fout,"%S,%S.%S\t%d\n",table[i]->fl, table[i]->ca,
               table[i]->inf, freqtable[i]);
     }
    u_fclose(fout);
    delete table;
    delete freqtable;
    delete mem;
}



int main_SortMorph(int argc, char *argv[]) {

    if(argc != 4) {
       usage();
       return 0;
    }
    arbre_string00 tString,mString,sString,eString;

    unichar *tMem = 0;
    unichar **tTable = 0 ;
    int tcount;
    unichar *mMem = 0;
    unichar **mTable = 0;
    int mcount;
    unichar *sMem = 0;
    unichar **sTable = 0;


    get_path(argv[1],pathNameSave);
    tcount = getStringTableFile(argv[1],tMem,tTable);
    mcount = getStringTableFileAvecNull(argv[2],mMem,mTable);

    int i;
    for	(i = 0; i < tcount; i++){
        if(*mTable[i] != '\0'){
            tString.put(tTable[i],mTable[i]);
        } else {
            if(u_is_letter(tTable[i][0]))
                 eString.put(tTable[i],0);
        }
    }


    int outSize;
    outSize = tString.size();
    strcpy(ftemp,pathNameSave);
    strcat(ftemp,"mdlf.n");
    fout = u_fopen(UTF16_LE,ftemp,U_WRITE);
    if(!fout) fopenErrMessage(ftemp);
    strFileHeadLine(fout,outSize);
    u_fclose(fout);
    strcpy(ftemp,pathNameSave);
    strcat(ftemp,"mdlf");
    fout = u_fopen(UTF16_LE,ftemp,U_WRITE);
    if(!fout) fopenErrMessage(ftemp);
    tString.explore_tout_leaf((release_f)getPrOutWithValue);

    u_fclose(fout);

    outSize = 0;
    strcpy(ftemp,pathNameSave);
    strcat(ftemp,"dlf.n");
    fout = u_fopen(UTF16_LE,ftemp,U_WRITE);
    if(!fout) fopenErrMessage(ftemp);

    strFileHeadLine(fout,outSize);
    u_fclose(fout);
    strcpy(ftemp,pathNameSave);
    strcat(ftemp,"dlf");
    fout = u_fopen(UTF16_LE,ftemp,U_WRITE);
    if(!fout) fopenErrMessage(ftemp);

    u_fclose(fout);

    outSize = eString.size();
    strcpy(ftemp,pathNameSave);
    strcat(ftemp,"err.n");
    fout = u_fopen(UTF16_LE,ftemp,U_WRITE);
    if(!fout) fopenErrMessage(ftemp);
    strFileHeadLine(fout,outSize);
    u_fclose(fout);
    strcpy(ftemp,pathNameSave);
    strcat(ftemp,"err");
    fout = u_fopen(UTF16_LE,ftemp,U_WRITE);
    if(!fout) fopenErrMessage(ftemp);
    eString.explore_tout_leaf((release_f)getPrOut);

    u_fclose(fout);


    /* reserve these fields for sequences of morphems about multi segments
    */
    outSize = sString.size();
    strcpy(ftemp,pathNameSave);
    strcat(ftemp,"mdlc.n");
    fout = u_fopen(UTF16_LE,ftemp,U_WRITE);
    if(!fout) fopenErrMessage(ftemp);
    strFileHeadLine(fout,outSize);
    u_fclose(fout);
    strcpy(ftemp,pathNameSave);
    strcat(ftemp,"mdlc");
    fout = u_fopen(UTF16_LE,ftemp,U_WRITE);
    if(!fout) fopenErrMessage(ftemp);
    sString.explore_tout_leaf((release_f)getPrOut);
    u_fclose(fout);

    outSize = 0;
    strcpy(ftemp,pathNameSave);
    strcat(ftemp,"dlc.n");
    fout = u_fopen(UTF16_LE,ftemp,U_WRITE);
    if(!fout) fopenErrMessage(ftemp);
    strFileHeadLine(fout,outSize);
    u_fclose(fout);

    strcpy(ftemp,pathNameSave);
    strcat(ftemp,"dlc");
    fout = u_fopen(UTF16_LE,ftemp,U_WRITE);
    if(!fout) fopenErrMessage(ftemp);

    u_fclose(fout);

    if(tMem) delete [] tMem;
	if(mMem) delete [] mMem;
	if(sMem) delete [] sMem;
    if(tTable) delete [] tTable;
	if(mTable) delete [] mTable;
	if(sTable) delete [] sTable;
	//
	// load list of morphème file to sort by canonique form, forme flechi,
	// frequence of occurence
	//

	sortMorphemTable(argv[3]);

    return(0);
}
