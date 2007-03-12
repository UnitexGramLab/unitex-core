 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Fst2.h"
#include "Copyright.h"
#include "FileName.h"
#include "String_hash2.h"
#include <locale.h>
#include "etc.h"
#include "formation_dic_line.h"
#include "IOBuffer.h"



#define N_FST2 3000 // maximum number of flexional transducers
#define MAX_CHARS_IN_STACK 1024
static int debugFlag = 0;
static char *dervRep;
static char *flexRep;
static char ofilename[1024];
static char ofilename1[1024];
static int auto_courant;
static int startAutoNum;
static char cur_fstName[1024];
static unichar curFstName[1024];

char repertoire[MAX_CHARS_IN_STACK];
#define CONTENT_RACINE  0x01
#define CONTENT_SUFFIX  0x02
#define CONTENT_RACSUF  0x03
static FILE *f_out;
int line=0;
//
// loads an fst2 and returns its representation in an Automate_fst2 structure
// same as load_fst2 but no message for the not exist file
// hhuh
static Fst2* load_fst22(char* file,int noms) {
FILE *f;
f=u_fopen(file,U_READ);
if (f==NULL) {
  return NULL;
}
u_fclose(f);
return load_fst2(file,noms);
}
//
//
//
class fst_array {
	class arbre_string0 a;
	struct fst2* fst2[N_FST2];
	int referenceCnt[N_FST2];
	public:
		fst_array(){
		    int i;
			for(i =0; i < N_FST2;i++){
				fst2[i] = (struct fst2 *)-1;
				referenceCnt[i] = 0;
			}

			{
			unichar tt='\0';
			i = a.put(&tt);
			}
			fst2[i] = 0;
		};
		~fst_array(){
		};
	struct fst2 * loadfst2name(char *rep,unichar *nomFst)
	{
		u_strcpy((unichar *)curFstName,(unichar *)nomFst);
		char *wp = cur_fstName;
		unichar *fn = nomFst;
		while(*rep) *wp++ = *rep++;
		if(*(rep-1) != '\\' && *(rep-1) != '/') *wp++ = '/';
		while(*fn) {*wp++ = (char)(*fn & 0x7f); fn++;}
		for(char *k=".fst2";*k;k++)
			*wp++ = *k;
		*wp = 0;

		int fstId;
		fstId = a.put((unichar *)nomFst);
		if(fst2[fstId] == (struct fst2 *)-1){
			fst2[fstId] = load_fst22(cur_fstName,1);
		}
		return(fst2[fstId]);
	}

} suf,dev;

int ADD_TWO_POINTS=0;
int REMOVE_DIGITS_FROM_GRAMM_CODE=1;


static int inflect_kr(struct fst2 *,unichar*,int mode);

static void explore_state(int);
#define MAX_DEPTH_AUTO	2048
static int etiQueue[MAX_DEPTH_AUTO];
static int curEtiCnt;
//
//	mode for handling graphs of the flexion or the derivation
//
#define FLEXION_MODE	0
#define DERIVATION_MODE	1
int grapheTraiteMode;

void usage(int flag) {
u_printf("%S",COPYRIGHT);
u_printf("Usage: InflectKr -d d_dir -v v_dir [-o ofile] <dictionnary>\n");
u_printf("     <delas> : the unicode delas file to be inflected\n");
u_printf("    -v v_dir : the directory of inflectional graphs.\n");
u_printf("    -d d_dir : the directory of graphs of the derivation.\n");
u_printf("    -o ofile : the inflected result file name  \n");
u_printf("             : the default output file name is <delas>IF  \n");
u_printf("    -c SS=0xNNNN : change a sting to character \n");
u_printf("    -m 0xNNNN : skipMark value set\n");
u_printf("    -x file   : load a file to change a character to sting\n");
u_printf("    -r : the content is racines\n");
u_printf("    -s : the content is suffixs\n");
u_printf("\nInflects a korean DELAS.\n");
if(flag) exit(1);
exit(0);
}
static char *cfilename;
static unichar *curLineTemp;
static int lineCnt;
static void lineErrMess()
{
	error("%s file at line %d\n",cfilename,lineCnt);
	fatal_error("Line <<%S>>\nhas syntax error",curLineTemp);
}

static Fst2* Ptr_cAuto;

void cleanData()
{
	if(dervRep) free(dervRep);
	if(flexRep) free(flexRep);
}


static void fst_err()
{
	fatal_error("%S fst2 file error\n",curFstName);
}

class arbre_string0 suffixeAuto;
class arbre_string0 notTraiteSuff;

int reference[1024];
int skipMark;
void prSuffixeString(void *a,void *b,void *c)
{

	unichar *obuf = (unichar *)a;
	int i = (int)b;
	//
	//	the first character of the line is a blanc for indicate comment line
	//
	u_fprintf(f_out," %S %d\n",&obuf[1],reference[i]);
}
void prSuffixeString0(void *a,void *b,void *c)
{

	unichar *obuf = (unichar *)a;
	//
	//	the first character of the line is a blanc for indicate comment line
	//
	u_fprintf(f_out," %S\n",&obuf[1]);
}

static void get_flexion_form(FILE *ifile,FILE *ofile);
static void trait_renouvelle_lign(FILE *ofile,unichar *readLine);
static int get_forms_variant(unichar *l,int *s,class dicElements *e);
static void outFileRac(char *ofileName);
static class dicLines *orgWord;
static class dicElements *curDicElements;
static unichar tempBuff[1096];
static int flagRacSuf;



int main(int argc, char **argv) {
setBufferMode();

	FILE *f;
	unichar *tt;
	int argIdx;
	skipMark= -1;
	argIdx = 1;
	char fNameSansExt[1024];
	
	if(argc == 1) usage(0);
	memset(reference,0,1024*4);
//	printf("%s\n",	setlocale(LC_ALL,"Korean_Korea.949"));
    flagRacSuf = CONTENT_RACINE;    
	dervRep =0;
	flexRep = 0;
	ofilename[0] = 0;
	ofilename1[0] = 0;
 	while(argIdx < argc -1 ){

		switch(*argv[argIdx]){
		case '-':
			switch(argv[argIdx][1]){
			case 'c':
			   argIdx++;

			   tt = new unichar[strlen(argv[argIdx])+1];
			   u_strcpy((unichar *)tt,argv[argIdx]);
			   changeStrToVal(tt);
			   break;
			case 'd':
				argIdx++;
				dervRep = (char *)malloc(strlen(argv[argIdx])+1);
				strcpy(dervRep,argv[argIdx]);break;
			case 'v':
				argIdx++;
				flexRep = (char *)malloc(strlen(argv[argIdx])+1);
				strcpy(flexRep,argv[argIdx]);break;
			case 'o':
				argIdx++;
				strcpy(ofilename,argv[argIdx]);break;
			case 'm':
				argIdx++;
			   tt = new unichar[strlen(argv[argIdx])+1];
			   u_strcpy((unichar *)tt,argv[argIdx]);
				skipMark = (unichar)(uniToInt(tt) & 0xffff);
               break;
			case 'r':
			   flagRacSuf |= CONTENT_RACINE;
               break;
			case 's':
			   flagRacSuf |= CONTENT_SUFFIX;
               break;
			case 'x':
			   argIdx++;
			   if(loadChangeFileToTable(argv[argIdx]))  break;
       
			default:
				usage(1);
			}
			break;
		default:
			usage(1);
		}
		argIdx++;
	}
	if(!dervRep || !flexRep) usage(1);

	cfilename = argv[argIdx];
	if(!ofilename[0]){
		remove_extension(cfilename,fNameSansExt);
		switch(flagRacSuf){
		case CONTENT_RACINE:
          sprintf(ofilename,"%s.ric",fNameSansExt);break;
		case CONTENT_SUFFIX:
            sprintf(ofilename,"%s.sic",fNameSansExt);break;
		case CONTENT_RACSUF:
            sprintf(ofilename,"%s.sic",fNameSansExt);
            sprintf(ofilename1,"%s.ric",fNameSansExt);
            break;
		default:
		   fatal_error("Error");
        }
	} else {
	   if(flagRacSuf == CONTENT_RACSUF){
            remove_extension(ofilename,fNameSansExt);
            sprintf(ofilename1,"%s.ric",fNameSansExt);
       }
	}
	f=u_fopen(argv[argIdx],U_READ);
	if (f==NULL) {
		error("Cannot open %s\n",argv[argIdx]);
		return 1;
	}
 	f_out=u_fopen(ofilename,U_WRITE);
	if (f_out==NULL) {
		error("Cannot open %s\n",ofilename);
		return 1;
	}
	//
	//
	get_flexion_form(f,f_out);
	cleanData();
	u_fclose(f);
	u_fclose(f_out);	//
	//
	//
    if(ofilename1[0]) outFileRac(ofilename1);	
	u_fprintf(f_out," suffixes list\n");
	suffixeAuto.explore_tout_leaf((release_f )prSuffixeString);
	u_fprintf(f_out," not handled suffixes list\n");
	notTraiteSuff.explore_tout_leaf((release_f )prSuffixeString0);


	u_printf("Done.\n");
	return 0;

}
static void
get_flexion_form(FILE *f,FILE *fout)
{
//
// reads the DELAS and produces the DELAF
//
// single,orgin,suffixe,information 
	#define MAX_LINE_NUMBER	4096
	unichar readLine[MAX_LINE_NUMBER];
	unichar workLine[MAX_LINE_NUMBER];

	unichar *s,*ss;
	

	int scanIdx;
	int saveIdx;
	unichar *fPtr[MAX_NUM_ELEMENT_BY_UNIT];
	readLine[0] = 0;
	lineCnt = 0;
	curLineTemp = readLine;
	while (EOF!=u_fgets(readLine,f)){
		lineCnt++;


		scanIdx = 0;
		if((readLine[scanIdx] == '#') ||
		(readLine[scanIdx] == '\0') ||
		(readLine[scanIdx] == ' ')
		) continue;
	//
	//	
	//
		for( saveIdx = 0; saveIdx < MAX_NUM_ELEMENT_BY_UNIT; saveIdx++)
				fPtr[saveIdx] = 0;
		saveIdx = 0;
		fPtr[saveIdx++] = readLine;
		scanIdx = 0;
		while(readLine[scanIdx]) {
			switch(readLine[scanIdx]){
			case ',':
				readLine[scanIdx++] = 0;
				fPtr[saveIdx++] = &readLine[scanIdx];
			break;
			case '\\':
				if(readLine[scanIdx] == 0)
					lineErrMess();
			default:
				scanIdx++;
			}
			
		}
		if(saveIdx != MAX_NUM_ELEMENT_BY_UNIT) lineErrMess();

		scanIdx = 0;
		int curColume = 0;
		int i;
		s = fPtr[curColume];
		saveIdx = 0;
		workLine[saveIdx++] = ';';
		do {
		switch(s[scanIdx]){
		case '|':	// separator in the fild of information
		case ' ':	// separer by a space
		case '^':	// separe or soud
		case '-':	// always soud
			workLine[saveIdx++] = ',';
			fPtr[curColume] = &s[scanIdx];
			if(curColume ==(MAX_NUM_ELEMENT_BY_UNIT-2)){
				workLine[saveIdx++] = ',';	// following suffixe
				workLine[saveIdx++] = ';';
				curColume = 0;
			} else {
				curColume++;
			}
			scanIdx = 0;
			s  = fPtr[curColume];
			if(s[scanIdx]){
				workLine[saveIdx++] = s[scanIdx];
				break;
			}
		case '\0':
			if(curColume) lineErrMess();
			for(i = 1; i < MAX_NUM_ELEMENT_BY_UNIT; i++)
			{
				workLine[saveIdx++] = ',';
				ss = fPtr[i];
				while(*ss) workLine[saveIdx++] = *ss++;
			}
			break;
		case '\\':
			workLine[saveIdx++] = s[scanIdx];
		default:
			workLine[saveIdx++] = s[scanIdx];
		}
		} while (s[scanIdx++]);
		workLine[saveIdx] = 0;
		
		trait_renouvelle_lign(f_out,workLine);

	}

}
static void
trait_renouvelle_lign(FILE *f,unichar *readLine)
{
	int scanIdx = 0;
	int saveIdx = 0;
	int segs[8];
	int serialElementCnt = 0;
	class dicElements *head_chaine = 0;
	class dicElements *tail_chaine = 0;
	
	int segCnt;

	unichar workLine[MAX_LINE_NUMBER];

	segCnt = 0;
	if(readLine[0] != ';') fatal_error("line error\n");
	do {
		switch(readLine[scanIdx]){
		case ',':
			if(segCnt >= 4){
				u_printf("error: line %d:%S is illegal\n",lineCnt,readLine);
				return;		
			}
			workLine[saveIdx++] = '\0';
			segs[segCnt++] = saveIdx;
			break;
		case '\0':
		case ';':
			if(head_chaine){
				workLine[saveIdx] = 0;

				if((segCnt != 4) || !workLine[segs[0]] || 
				!workLine[segs[2]]) {
					fatal_error("%d:%S line syntax error\n",lineCnt,readLine);
				}
				if(serialElementCnt){
					unichar c = workLine[segs[0]];
					if( (workLine[segs[2]] != c) ||
						((workLine[segs[1]] != 0) && (workLine[1] != c)) ||
						(workLine[segs[3]] != c))
					fatal_error("%d:%S line syntax error\n",lineCnt,readLine);
				}
				get_forms_variant(workLine,segs,tail_chaine);
				if(readLine[scanIdx] != '\0')
				{
					tail_chaine->next = new class dicElements;
					tail_chaine = tail_chaine->next;
				}
				serialElementCnt++;
			} else {
				tail_chaine = head_chaine = new class dicElements ;
			}

			for(segCnt = 0; segCnt < 4 ; segCnt++)
				segs[segCnt] = 0;
			segCnt = 0;
			saveIdx = 0;
			segs[segCnt++] = saveIdx;
			break;
		case '\t': 
			workLine[saveIdx++]= ' ';
			break;
		case '\\':
			workLine[saveIdx++] = readLine[scanIdx++];
			if(!readLine[scanIdx]){
				error("error: line %d:%S is illegal\n",lineCnt,readLine);
				return;		
			}
		default:
			workLine[saveIdx++]=readLine[scanIdx];
		}
		if(scanIdx > MAX_LINE_NUMBER){
			error("error: line %d:%S is illegal",lineCnt,readLine);
			return;		
		}
	} while (readLine[scanIdx++]);

	head_chaine->prEle(f);
}
//
//  get the form of derivation with  /d /D
//  and the form of flection
//

static int
get_forms_variant(unichar *workLine,int *segIndex,class dicElements *ele)
{
	int lineIdx ;
	int saveIdx;
	int scanIdx;
	unichar workLine1[MAX_LINE_NUMBER];
if(debugFlag){
for ( lineIdx = 0; lineIdx < 4;lineIdx++) {
   u_printf("==%S",&workLine[segIndex[lineIdx]]);
}
u_printf("\n");
}
	
	// find derivation information
	// in the lingustic information
	// extrait lingustic informations

	lineIdx = 0;
	scanIdx = segIndex[2];
	saveIdx = scanIdx;
	simpleL<unichar *> cmds;
	do {
		if((workLine[scanIdx] == '+') &&
			(workLine[scanIdx+1] == '/') && 
			((workLine[scanIdx+2] == 'd') ||  
			(workLine[scanIdx+2] == 'D') )
            ){
			
			workLine[saveIdx++] = workLine[scanIdx];
			scanIdx+=3;
			cmds.put(&workLine1[lineIdx]);
			while(
				(workLine[scanIdx] != '+')
				&& (workLine[scanIdx] !='\0')){
				workLine[saveIdx++] = workLine[scanIdx];
				workLine1[lineIdx++] = workLine[scanIdx++];
			}
			workLine1[lineIdx++]='\0';
		} else {
			do {
				workLine[saveIdx++] = workLine[scanIdx++];
			}while(
				(workLine[scanIdx] != '+')
				&& (workLine[scanIdx] !='\0'));
		}
		if(scanIdx > MAX_LINE_NUMBER) {
			error("error: line %d:%S is illegal",lineCnt,workLine);
			return 0;
		}
	} while(workLine[scanIdx]);
	workLine[saveIdx] = 0;

	//
	//	set first word
	//
	orgWord = new class dicLines; 

	orgWord->set(u_null_string,
        &workLine[segIndex[0]],
		&workLine[segIndex[1]],
		&workLine[segIndex[2]],
		&workLine[segIndex[3]]	);
	//
	//	handling the derivation word
	//  get derived words
	class dicElements local;
	struct fst2 *fstAuto;
	unichar *wp;
	local.put(orgWord);
	curDicElements = &local;

	if(cmds.size()){
		cmds.reset();
		for( scanIdx = 0; scanIdx < cmds.size() ;scanIdx++){
			wp = cmds.getNext();
			if(!wp){
				fatal_error("Warning: line %d:%S is illegal\n",lineCnt,workLine);
			}
			if((fstAuto = dev.loadfst2name(dervRep,wp))
				!= (struct fst2 *)0 ){
				inflect_kr(fstAuto,orgWord->EC_canonique,DERIVATION_MODE);
			}else {
				fatal_error("error: derivation file not exist\n");
			}
		}
	}

	//
	//	get form flexion
	//
	unichar *variation_code;

	class dicLines *wEle= local.hPtr;
	curDicElements = ele;
	while(wEle){
		orgWord = wEle;
		variation_code = orgWord->EC_code;
		switch(*variation_code){
		case '-':
		case '+':
			variation_code++;
		}
		if( (fstAuto = suf.loadfst2name(flexRep,variation_code))
			!= (struct fst2 *)0){
			inflect_kr(fstAuto,orgWord->EC_canonique,FLEXION_MODE);
			wEle = wEle->next;
		} else{
			notTraiteSuff.put(orgWord->EC_code);
			orgWord = new class dicLines;
			orgWord->set(
               wEle->EC_canonique,
				wEle->EC_canonique,
				wEle->EC_orgin,
				wEle->EC_code_gramm,
				wEle->EC_code);
			curDicElements->put(orgWord);
			wEle = wEle->next;
		}
		
	} while(wEle);
	orgWord = curDicElements->hPtr;
	do {
		variation_code = orgWord->EC_code;
		switch(*variation_code){
		case '-':
		case '+':
			variation_code++;
		}
		reference[suffixeAuto.put(variation_code)]++;
		orgWord = orgWord->next;
	} while(orgWord);
	return 1;
}







//
// inflect the lemma 'lemme', using the flexional transducer 'flex', and
// taking 'code' as the basic grammatical code to be written in the DELAF
//
static int inflect_kr(Fst2*fstAuto,unichar* flex,int modeflex)
{
	grapheTraiteMode = modeflex;
	auto_courant = 1;
	startAutoNum = auto_courant;
	curEtiCnt = 0;
	Ptr_cAuto= fstAuto;
	explore_state(Ptr_cAuto->initial_states[auto_courant]);
	return(1);
}



void explore_state_recursion(unichar*,unichar*,unichar*,Fst2*,int,struct couple_string**,unichar*);

//
// Shifts all the stack from the position pos
//
void shift_stack(unichar* stack,int pos) {
if (pos==0) {
   // this case should never happen
   return;
}
for (int i=(MAX_CHARS_IN_STACK-1);i>=pos;i--) {
   stack[i]=stack[i-1];
}
}
static unichar Fpile[MAX_CHARS_IN_STACK];
static unichar Opile[MAX_CHARS_IN_STACK];

static unichar FF[MAX_CHARS_IN_STACK];	// form of the flexion
static unichar OF[MAX_CHARS_IN_STACK];  // form of orgin
static 	unichar SuF[1024];
static	unichar InF[1024];


static void traiteEttiques()
{
	Fst2Tag et;

	int wp,dp,ip,op;
	int FpileIdx = 0;
	int OpileIdx = 0;
	unichar Ctmp;
	unichar temp[4];
	int transFlag = 0;
	int orgFlag = 0;

	unichar *swp;
    int FIdx = 0;
    int OIdx = 0;
	dp =0; ip = 0; op = 0;
	
// copy a orginal word before handling
	if(grapheTraiteMode == FLEXION_MODE){ 
 		swp = orgWord->EC_canonique;
        while(*swp) FF[FIdx++] = *swp++; 
        if(orgWord->EC_orgin == u_null_string){
 		        swp = orgWord->EC_canonique;
        } else {
 		        swp = orgWord->EC_orgin;
 		        orgFlag = 1;
 		}
        while(*swp) OF[OIdx++] = *swp++;     			
	} 
	FF[FIdx] = 0;OF[OIdx] = 0;    
	
	for(int i = 0; i < curEtiCnt;i++){
		et=Ptr_cAuto->tags[etiQueue[i]];
if(debugFlag)
	u_printf("%S %S\n",et->output,et->input);
		//
		//	gether informations
		//
		if (et->output && 
            *(et->output) && 
                u_strcmp(et->output,"<E>")) {
				// if we are in a final state, we save the computed things
			wp = 0;
			while(et->output[wp]){
				if(et->output[wp] == '<') break;
				wp++;
			}			
			if(!et->output[wp]) fst_err();
			
			wp++;
			while(et->output[wp]){
				if(et->output[wp]==',') break;	
				SuF[dp++] = et->output[wp++];
			}
			if(et->output[wp]!=',') fst_err();
			wp++;
			while(et->output[wp]){
				if(et->output[wp] == '>') break;
				InF[ip++] = et->output[wp++];
			}
		
		}
		if(et->input[0] == '<'){
		  if(!u_strcmp(et->input,"<$>")){ 
			// copy org to working stack
			swp = orgWord->EC_canonique;
			while(*swp)  FF[FIdx++] = *swp++;
		    if(orgWord->EC_orgin == u_null_string){
 		        swp = orgWord->EC_canonique;
            } else {
 		        swp = orgWord->EC_orgin;
 		        orgFlag = 1;
 		    }
            while(*swp) OF[OIdx++] = *swp++;
            continue;
		  } else if (!u_strcmp(et->input,"<E>")) {
		     continue;
		  } else if (findChangeStr((unichar*)et->input,temp)) {
				FF[FIdx++] = temp[0];
				OF[OIdx++] = temp[0];
				continue;
		  }
        }
		
	   // if the tag is not <E>, we process it
        for (wp=0; et->input[wp] !='\0';wp++) {
            if( (swp = getConvTable(et->input[wp])) != 0){
			     while(*swp) FF[FIdx++] = *swp++;
                 OF[OIdx++] = et->input[wp];
                 transFlag = 1;
                 continue; 
			}
			switch (et->input[wp]) {
			case '[':
			      if(skipMark == -1 ){
                       fatal_error("skipMark is not defined\n");
                  } 
                  while((FIdx != 0) &&(Fpile[FpileIdx++] = FF[--FIdx])
					!= skipMark);
					
                  Ctmp = OF[OIdx-1];
                  if(u_is_CJK_Unified_Ideographs(Ctmp)
	                         || u_is_cjk_compatibility_ideographs(Ctmp)){
                      if(OIdx != 0)	Opile[OpileIdx++]= Ctmp;
                      --OIdx;
                      break;
                  }
                  while((OIdx != 0) &&(Opile[OpileIdx++] = OF[--OIdx])
					!= skipMark);				  	
                  break;
			case 'L': 
                  Ctmp = OF[OIdx-1];
                  if(u_is_CJK_Unified_Ideographs(Ctmp)
	                         || u_is_cjk_compatibility_ideographs(Ctmp)){
                      if(OIdx != 0)	Opile[OpileIdx++]= Ctmp;
                      --OIdx;
                     while((FIdx != 0) &&(Fpile[FpileIdx++] = FF[--FIdx])
					  != skipMark);
                      break;
                  }
               if(FIdx!= 0)	Fpile[FpileIdx++]= FF[--FIdx];
               if(OIdx!= 0)	Opile[OpileIdx++]= OF[--OIdx];
               break;
			case 'X':	// delete a sylable
		        if(skipMark == -1 ){
                    fatal_error("skipMark is not defined\n");
                } 
                  Ctmp = OF[OIdx-1];
                  if(u_is_CJK_Unified_Ideographs(Ctmp)
	                         || u_is_cjk_compatibility_ideographs(Ctmp)){
                      if(OIdx != 0)	Opile[OpileIdx++]= Ctmp;
                      --OIdx;
                     while((FIdx != 0) &&(Fpile[FpileIdx++] = FF[--FIdx])
					  != skipMark);
                      break;
                  }
             	while((FpileIdx != 0) && (Fpile[--FpileIdx] != skipMark));
             	while((OpileIdx != 0) && (Opile[--OpileIdx] != skipMark));
             	
				break;
			case 'R': 
               if(FpileIdx != 0) FpileIdx--; 
               if(OpileIdx != 0) OpileIdx--; 
               break;
			case ']':
			    if(skipMark == -1 ) fatal_error("skipMark is not defined\n");
				if(FpileIdx == 0) break;
				--FpileIdx;
				if(OpileIdx == 0) break;
				--OpileIdx;
				
				while(FpileIdx != 0 ){
				    FF[FIdx++] = Fpile[FpileIdx++];
					if(Fpile[FpileIdx] == skipMark) break;
				}
				
				break;
			case 'C':
				if(OpileIdx != 0) { FF[FIdx++] = Opile[--OpileIdx];}
				break;
			default:
		       FF[FIdx++] = et->input[wp];
		       OF[OIdx++] = et->input[wp];
			}
if(debugFlag){ FF[FIdx] = 0;  u_printf("%S >>>>\n",FF);}
		}
	} // parcours
	FF[FIdx] = 0;
	OF[OIdx] = 0;
	SuF[dp]=0;
	InF[ip] = 0;
	
	class dicLines *wWord = new class  dicLines;
	if(!wWord) fatal_error("mem alloc fail\n");
	switch(grapheTraiteMode){
    case FLEXION_MODE: // copy a orginal word before handling
          wWord->set(FF,orgWord->EC_canonique,
                      (transFlag || orgFlag) ?OF:u_null_string,
                      orgWord->EC_code_gramm,SuF);
		break;
	case  DERIVATION_MODE :// derivation
		if(InF[0]== '+'){
			u_strcpy((unichar*)tempBuff,(unichar *)orgWord->EC_code_gramm);
			u_strcat((unichar *)tempBuff,(unichar*)InF);
		} else {
  			u_strcpy((unichar*)tempBuff,(unichar *)InF);
		}
 	    wWord->set(u_null_string,FF,u_null_string,tempBuff,u_null_string);

 	    if(transFlag||orgFlag){
 	     	    wWord->set(u_null_string,u_null_string
                     ,OF,u_null_string,u_null_string);
 	    }
    
 	    if((SuF[0] == '\0') || !u_strcmp((unichar*)SuF,"<E>"))
 	    wWord->set(u_null_string,FF,u_null_string,tempBuff,
                  orgWord->EC_code);
 	    else 
 	    wWord->set(u_null_string,FF,u_null_string,tempBuff,SuF);
	}
	curDicElements->put(wWord);
}

//
// explore the transducer a
//
void explore_state(int etat_courant)
{
	int save_auto;
	Fst2State e=Ptr_cAuto->states[etat_courant];
	if (is_final_state(e)) {
		if(auto_courant == startAutoNum){
			traiteEttiques();
		}
	}
	struct fst2Transition* t=e->transitions;
	while (t!=NULL) {
		if (t->tag_number < 0) {
			save_auto = auto_courant;
			auto_courant = -(t->tag_number);
		    explore_state(Ptr_cAuto->initial_states[-(t->tag_number)]);
		    auto_courant = save_auto;
		}

		etiQueue[curEtiCnt++] = t->tag_number;
		explore_state(t->state_number);
		curEtiCnt--;
		t = t->next;
   }
   return;
}
static void outFileRac(char *ofileName)
{
    FILE *of = u_fopen(ofileName,U_WRITE);
    char nameOfSuf[1024],temp[1024];
    remove_path(ofileName,temp);
    remove_extension(temp,nameOfSuf);
    u_fprintf(of,",,,,%s\n",nameOfSuf);
    u_fclose(of);
}
