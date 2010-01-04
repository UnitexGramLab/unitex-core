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

//
// sort morphemes after the consultation of dictionaries
//
// sortMorph
//     tokens file, morphems of a word file,  sequence de morpheme file

#include <stdio.h>
#include "UnusedParameter.h"
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


    struct locdefi {
	     unichar *fl;
	     unichar *ca;
	     unichar *inf;
    };

struct SortMorph_ctx {
 U_FILE *fout;
 char ftemp[4096];
 char pathNameSave[4096];

 locdefi **table;
 int *freqtable;
} ;

static void getPrOutWithValue(struct SortMorph_ctx * p_sortMorph_ctx,void * arg0,void *arg1,void *arg2)
{
    DISCARD_UNUSED_PARAMETER(arg1)
    unichar *word = (unichar *)arg0;
    unichar *data = (unichar *)arg2;
    u_fprintf(p_sortMorph_ctx->fout,"%S %S\n",word+1,data);
}


static void getPrOut(struct SortMorph_ctx * p_sortMorph_ctx,void * arg0,void *arg1,void *arg2)
{
    DISCARD_UNUSED_PARAMETER(arg1)
    DISCARD_UNUSED_PARAMETER(arg2)
    unichar *word = (unichar *)arg0;

    u_fprintf(p_sortMorph_ctx->fout,"%S\n",word+1);
}


static void getPrOutMorpheme(struct SortMorph_ctx * p_sortMorph_ctx,void * arg0,void *arg1,void *arg2)
{
    DISCARD_UNUSED_PARAMETER(arg0)
    DISCARD_UNUSED_PARAMETER(arg1)
    intptr_t index = (intptr_t)arg2;
    u_fprintf(p_sortMorph_ctx->fout,"%S,%S.%S\t%d\n",
        p_sortMorph_ctx->table[index]->fl, 
               p_sortMorph_ctx->table[index]->ca,
               p_sortMorph_ctx->table[index]->inf, 
               p_sortMorph_ctx->freqtable[index]);
}

	static int partition_pour_quicksort_by_frequence(struct SortMorph_ctx * p_sortMorph_ctx,int m, int n)
	{
		int pivot;
		int tmp;
		struct locdefi* tmp_char;
		int i = m-1;
		int j = n+1;         // indice final du pivot
		pivot=p_sortMorph_ctx->freqtable[(m+n)/2];
		for (;;) {
			do j--;while ((j>(m-1))&&(pivot>(p_sortMorph_ctx->freqtable[j])));
			do i++;while ((i<n+1)&&((p_sortMorph_ctx->freqtable[i])>pivot));
			if (i<j) {
				tmp=p_sortMorph_ctx->freqtable[i];
				p_sortMorph_ctx->freqtable[i]=p_sortMorph_ctx->freqtable[j];
				p_sortMorph_ctx->freqtable[j]=tmp;

				tmp_char=p_sortMorph_ctx->table[i];
				p_sortMorph_ctx->table[i]=p_sortMorph_ctx->table[j];
				p_sortMorph_ctx->table[j]=tmp_char;
			} else
				return j;
		}
	}

	void static quicksort_by_frequence(struct SortMorph_ctx * p_sortMorph_ctx,int debut,int fin)
	{
		int p;
		if (debut<fin) {
			p=partition_pour_quicksort_by_frequence(p_sortMorph_ctx,debut,fin);
			quicksort_by_frequence(p_sortMorph_ctx,debut,p);
			quicksort_by_frequence(p_sortMorph_ctx,p+1,fin);
		}
	}
static void
sortMorphemTable(struct SortMorph_ctx * p_sortMorph_ctx,char *f,Encoding encoding_output,int bom_output)
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

	p_sortMorph_ctx->table = new struct locdefi *[count];
	p_sortMorph_ctx->freqtable = new int[count];
	if(!(p_sortMorph_ctx->table)) fatal_error("morphems table mem alloc fail\n");
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
	    p_sortMorph_ctx->table[index] = new struct locdefi;
        curoffset = wp;
	    while(*wp != L','){if(!*wp) fatal_error("illegal format\n");wp++;}
	    *wp++ = 0;
	    p_sortMorph_ctx->table[index]->fl = curoffset;

        curoffset = wp;
	    while(*wp != L'.'){if(!*wp) fatal_error("illegal format\n");wp++;}
	    *wp++ = 0;
	    p_sortMorph_ctx->table[index]->ca = curoffset;

	    curoffset = wp;
	    while(*wp != L'}'){if(!*wp) fatal_error("illegal format\n");wp++;}
	    *wp++ = 0;
	    p_sortMorph_ctx->table[index]->inf = curoffset;
	    while((*wp == L' ' ) || (*wp == '\t'))
          {if(!*wp) fatal_error("illegal format\n");wp++;}

	    curoffset = wp;
	    for(tcount = 0;(*wp >= L'0') && (*wp <= '9') ;wp++)
           tcount = tcount * 10 + *wp - L'0';
        p_sortMorph_ctx->freqtable[index] = tcount;
        while((*wp == L'\r') || (*wp == L'\n')) wp++;
        parFlechi.put(p_sortMorph_ctx->table[index]->fl,(void *)index);
        parCano.put(p_sortMorph_ctx->table[index]->ca,(void *)index);
        index++;

	};

    strcpy(p_sortMorph_ctx->ftemp,p_sortMorph_ctx->pathNameSave);
    strcat(p_sortMorph_ctx->ftemp,"morph_by_flei.txt");
    p_sortMorph_ctx->fout = u_fopen_creating_versatile_encoding(encoding_output,bom_output,
         p_sortMorph_ctx->ftemp,U_WRITE);
    if(!(p_sortMorph_ctx->fout)) fopenErrMessage(p_sortMorph_ctx->ftemp);
    parFlechi.explore_tout_leaf((release_f)getPrOutMorpheme);
    u_fclose(p_sortMorph_ctx->fout);

    strcpy(p_sortMorph_ctx->ftemp,p_sortMorph_ctx->pathNameSave);
    strcat(p_sortMorph_ctx->ftemp,"morph_by_cano.txt");
    p_sortMorph_ctx->fout = u_fopen_creating_versatile_encoding(encoding_output,bom_output,
          p_sortMorph_ctx->ftemp,U_WRITE);
    if(!(p_sortMorph_ctx->fout)) fopenErrMessage(p_sortMorph_ctx->ftemp);
    parCano.explore_tout_leaf((release_f)getPrOutMorpheme);
    u_fclose(p_sortMorph_ctx->fout);
    strcpy(p_sortMorph_ctx->ftemp,p_sortMorph_ctx->pathNameSave);
    strcat(p_sortMorph_ctx->ftemp,"morph_by_freq.txt");
    p_sortMorph_ctx->fout = u_fopen_creating_versatile_encoding(encoding_output,bom_output,
          p_sortMorph_ctx->ftemp,U_WRITE);
    if(!(p_sortMorph_ctx->fout)) fopenErrMessage(p_sortMorph_ctx->ftemp);

    quicksort_by_frequence(p_sortMorph_ctx,0,index-1);
    int i;

    for(i = 0; i < index;i++){
        u_fprintf(p_sortMorph_ctx->fout,"%S,%S.%S\t%d\n",
                  p_sortMorph_ctx->table[i]->fl, 
                  p_sortMorph_ctx->table[i]->ca,
                  p_sortMorph_ctx->table[i]->inf, 
                  p_sortMorph_ctx->freqtable[i]);
     }
    u_fclose(p_sortMorph_ctx->fout);
    delete p_sortMorph_ctx->table;
    delete p_sortMorph_ctx->freqtable;
    delete mem;
}



int main_SortMorph(int argc, char *argv[]) {
    int nb_skip_arg=0;

    Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
    int bom_output = DEFAULT_BOM_OUTPUT;
    int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;

    struct SortMorph_ctx sortMorph_ctx;
    memset(&sortMorph_ctx,0,sizeof(struct SortMorph_ctx));

    while ((argc-nb_skip_arg)>4)
    {
        if(*argv[1+nb_skip_arg] != '-') break;
        switch(argv[1+nb_skip_arg][1]){
            case 'k': 
                     if (argv[1+1+nb_skip_arg][0]=='\0') {
                        fatal_error("Empty input_encoding argument\n");
                     }
                     decode_reading_encoding_parameter(&mask_encoding_compatibility_input,argv[1+1+nb_skip_arg]);
                     nb_skip_arg+=2;
                     break;
            case 'q':
                     if (argv[1+1+nb_skip_arg][0]=='\0') {
                        fatal_error("Empty output_encoding argument\n");
                     }
                     decode_writing_encoding_parameter(&encoding_output,&bom_output,argv[1+1+nb_skip_arg]);
                     nb_skip_arg+=2;
                     break;
            default:
                     usage();
                     return 1;
        }
    }

    if(argc-nb_skip_arg != 4) {
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


    get_path(argv[1+nb_skip_arg],sortMorph_ctx.pathNameSave);
    tcount = getStringTableFile(argv[1+nb_skip_arg],tMem,tTable);
    mcount = getStringTableFileAvecNull(argv[2+nb_skip_arg],mMem,mTable);

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
    strcpy(sortMorph_ctx.ftemp,sortMorph_ctx.pathNameSave);
    strcat(sortMorph_ctx.ftemp,"mdlf.n");
    sortMorph_ctx.fout = u_fopen_creating_versatile_encoding(encoding_output,bom_output,sortMorph_ctx.ftemp,U_WRITE);
    if(!(sortMorph_ctx.fout)) fopenErrMessage(sortMorph_ctx.ftemp);
    strFileHeadLine(sortMorph_ctx.fout,outSize);
    u_fclose(sortMorph_ctx.fout);
    strcpy(sortMorph_ctx.ftemp,sortMorph_ctx.pathNameSave);
    strcat(sortMorph_ctx.ftemp,"mdlf");
    sortMorph_ctx.fout = u_fopen_creating_versatile_encoding(encoding_output,bom_output,sortMorph_ctx.ftemp,U_WRITE);
    if(!sortMorph_ctx.fout) fopenErrMessage(sortMorph_ctx.ftemp);
    tString.explore_tout_leaf((release_f)getPrOutWithValue);

    u_fclose(sortMorph_ctx.fout);

    outSize = 0;
    strcpy(sortMorph_ctx.ftemp,sortMorph_ctx.pathNameSave);
    strcat(sortMorph_ctx.ftemp,"dlf.n");
    sortMorph_ctx.fout = u_fopen_creating_versatile_encoding(encoding_output,bom_output,sortMorph_ctx.ftemp,U_WRITE);
    if(!(sortMorph_ctx.fout)) fopenErrMessage(sortMorph_ctx.ftemp);

    strFileHeadLine(sortMorph_ctx.fout,outSize);
    u_fclose(sortMorph_ctx.fout);
    strcpy(sortMorph_ctx.ftemp,sortMorph_ctx.pathNameSave);
    strcat(sortMorph_ctx.ftemp,"dlf");
    sortMorph_ctx.fout = u_fopen_creating_versatile_encoding(encoding_output,bom_output,
          sortMorph_ctx.ftemp,U_WRITE);
    if(!(sortMorph_ctx.fout)) fopenErrMessage(sortMorph_ctx.ftemp);

    u_fclose(sortMorph_ctx.fout);

    outSize = eString.size();
    strcpy(sortMorph_ctx.ftemp,sortMorph_ctx.pathNameSave);
    strcat(sortMorph_ctx.ftemp,"err.n");
    sortMorph_ctx.fout = u_fopen_creating_versatile_encoding(encoding_output,bom_output,sortMorph_ctx.ftemp,U_WRITE);
    if(!(sortMorph_ctx.fout)) fopenErrMessage(sortMorph_ctx.ftemp);
    strFileHeadLine(sortMorph_ctx.fout,outSize);
    u_fclose(sortMorph_ctx.fout);
    strcpy(sortMorph_ctx.ftemp,sortMorph_ctx.pathNameSave);
    strcat(sortMorph_ctx.ftemp,"err");
    sortMorph_ctx.fout = u_fopen_creating_versatile_encoding(encoding_output,bom_output,sortMorph_ctx.ftemp,U_WRITE);
    if(!(sortMorph_ctx.fout)) fopenErrMessage(sortMorph_ctx.ftemp);
    eString.explore_tout_leaf((release_f)getPrOut);

    u_fclose(sortMorph_ctx.fout);


    /* reserve these fields for sequences of morphems about multi segments
    */
    outSize = sString.size();
    strcpy(sortMorph_ctx.ftemp,sortMorph_ctx.pathNameSave);
    strcat(sortMorph_ctx.ftemp,"mdlc.n");
    sortMorph_ctx.fout = u_fopen_creating_versatile_encoding(encoding_output,bom_output,sortMorph_ctx.ftemp,U_WRITE);
    if(!(sortMorph_ctx.fout)) fopenErrMessage(sortMorph_ctx.ftemp);
    strFileHeadLine(sortMorph_ctx.fout,outSize);
    u_fclose(sortMorph_ctx.fout);
    strcpy(sortMorph_ctx.ftemp,sortMorph_ctx.pathNameSave);
    strcat(sortMorph_ctx.ftemp,"mdlc");
    sortMorph_ctx.fout = u_fopen_creating_versatile_encoding(encoding_output,bom_output,sortMorph_ctx.ftemp,U_WRITE);
    if(!(sortMorph_ctx.fout)) fopenErrMessage(sortMorph_ctx.ftemp);
    sString.explore_tout_leaf((release_f)getPrOut);
    u_fclose(sortMorph_ctx.fout);

    outSize = 0;
    strcpy(sortMorph_ctx.ftemp,sortMorph_ctx.pathNameSave);
    strcat(sortMorph_ctx.ftemp,"dlc.n");
    sortMorph_ctx.fout = u_fopen_creating_versatile_encoding(encoding_output,bom_output,sortMorph_ctx.ftemp,U_WRITE);
    if(!(sortMorph_ctx.fout)) fopenErrMessage(sortMorph_ctx.ftemp);
    strFileHeadLine(sortMorph_ctx.fout,outSize);
    u_fclose(sortMorph_ctx.fout);

    strcpy(sortMorph_ctx.ftemp,sortMorph_ctx.pathNameSave);
    strcat(sortMorph_ctx.ftemp,"dlc");
    sortMorph_ctx.fout = u_fopen_creating_versatile_encoding(encoding_output,bom_output,sortMorph_ctx.ftemp,U_WRITE);
    if(!(sortMorph_ctx.fout)) fopenErrMessage(sortMorph_ctx.ftemp);

    u_fclose(sortMorph_ctx.fout);

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

	sortMorphemTable(&sortMorph_ctx,argv[3+nb_skip_arg],encoding_output,bom_output);

    return(0);
}
