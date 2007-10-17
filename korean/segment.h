 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/*
 * segment.h
 *	make a structure from a plaintext for untilise for ananlyses the text
 *
 */
#ifndef SEGMENT_KR
#define SEGMENT_KR

#include "String_hash2.h"
#include "etcclass.h"

#define MAX_UNIT_FILE_SIZE	0x00100000		// 1M
//#define MAX_UNIT_FILE_SIZE	0x80
#define MAX_FILE_SIZE		0x10000000	// 256M
struct tLink {
	int v;
	struct tLink *next;
};
class segmentation:wideCharTable,nameOfControlChars
{
	class arbre_string00 segments;
	char pathName[2048];
	FILE *textFile;
	FILE *tokenFile;
	FILE *writeFile;
public:

	int segmentCount;
	int sentenceCnt;
	int grSegmentCnt[8];	// count of segments in each group
	int grSegmentSum[8];
	int lengthMax;
	int lengthMin;
	
	int segmentStartOffet;
#ifdef AVEC_STRUCT
	struct tmplettre *saveTmps;
#else
	int *saveTmps;
#endif
	int saveTmps_cnt;
	unsigned short *mot_buff;	// buffer for file 
	int lastOffset;
	int currentOffset;
	int connection;	// flag for utilise the indication of
                    // of the connection with next segment
	int *indexStartOffset;
	int startFileOffset;
	int lastFileOffset;
	int maxUnitSize;

	int nomOftraitSection;
	int currentOfileIndex;
	unsigned short  cbuff[1024];
	int spaceCnt;
	unsigned short retnew[2];	// new line return
	FILE *sort_token;
	int *n_occur;
	unsigned short  **tokTable;
	time_t debug_time_point;
#define MAX_SAVETMPS_CNT	2048
	segmentation(){
		retnew[0] = 0x0d;
		retnew[1] = 0x0a;
		segmentCount = 0;
		sentenceCnt = 0;
		spaceCnt = 0;
		int i;
		startFileOffset = 1;	// unicode file mark skip
		mot_buff = 0;
		preTypeChar = 0xffffffff;
		for(i = 0; i< NUM_OF_GROUP_SEG;i++)
			grSegmentCnt[i] = 0;
#ifdef AVEC_STRUCT
		saveTmps = new struct tmplettre[MAX_SAVETMPS_CNT];
#else
		saveTmps = new int[MAX_SAVETMPS_CNT*2];
#endif
		saveTmps_cnt = 0;
	}
	~segmentation()
	{
		if(mot_buff)	
			delete mot_buff;
		if(textFile) fclose(textFile);
		if(saveTmps_cnt){
			fwrite(saveTmps,sizeof(struct tmplettre)*
				MAX_SAVETMPS_CNT,1,writeFile);
		}
		if(writeFile) fclose(writeFile);
	}

	unsigned char preTypeChar;

	int segmentFile(char *,char *,int sizeBuffer);
		void filleCntLine(FILE *,int);
		void fillIntAtArray(int,unsigned short *,int);
	void getSegments(unsigned short  *wp);



#ifdef AVEC_STRUCT
	void saveWord(unsigned char typeDef,unsigned short  *strbuff,int size)
	{
		struct tmplettre *lettre;
		struct tmpSymbole *nonLettre;

		switch(typeDef){
		case  HAN_CHAR:
		case  ASC_CHAR:
			lettre = &saveTmps[saveTmps_cnt++];
			lettre->index = segments.insertWordAndInc(
			strbuff,size);
			lettre->type = typeDef;
			lettre->Offset = lastOffset;

		break;
		case  SYM_CHAR:
			break;
		default:
			fatal_error("Segment Type Error\n");
		}
		if(saveTmps_cnt >= MAX_SAVETMPS_CNT){
			fwrite(saveTmps,sizeof(struct tmplettre)*
				MAX_SAVETMPS_CNT,1,writeFile);
			saveTmps_cnt = 0;
		}

	}
#endif		// AVEC_STRUCT
	void openFiles(char *ifn,char *ofn)
	{


		if(!(textFile = u_fopen(ifn,U_READ)))
			fopenErrMessage(ifn);
	
		if(ofn)
			get_snt_path(ofn,pathName);
		else
			get_snt_path(ifn,pathName);


		char *tFileName = "text2.cod";
		char *idFileName  = new char
			[strlen(pathName)+strlen(tFileName)+1];
		strcpy(idFileName,pathName);
		strcat(idFileName,tFileName);
		if(!(writeFile = fopen(idFileName,"wb")))
			fopenErrMessage(idFileName);


		delete idFileName;
	}

	void
	static_data()
	{
		char *tFileName = "tokens.txt";
		char *tokenFileName  = new char 
			[strlen(pathName)+strlen(tFileName) +1];
		
  			strcpy(tokenFileName,pathName);
		strcat(tokenFileName,tFileName);
		
		if(!(tokenFile = u_fopen(tokenFileName,U_WRITE)))
			fopenErrMessage(tokenFileName);
		segmentCount = segments.size();
		
        
        
		tokTable =segments.make_strPtr_table(&n_occur);
		filleCntLine(tokenFile,segments.size());
		int i;		
      for(i = 0; i < 8;i++){
			grSegmentCnt[i] = 0;
			grSegmentSum[i]=  0;
		}
		int typeWord;
		for(i = 0; i < segmentCount;i++)
		{
			typeWord =check_range_character(
				tokTable[i][1])& TYPE_MASK ;
			grSegmentCnt[typeWord]++;
			grSegmentSum[typeWord]+= n_occur[i];
			a_token_out(tokenFile,&tokTable[i][1]);
			fwrite(&retnew,4,1,tokenFile);
		}
		fclose(tokenFile);
		
		tFileName = "tok_by_alph.txt";
		tokenFileName  = new char 
			[strlen(pathName)+strlen(tFileName) +1];
		strcpy(tokenFileName,pathName);
		strcat(tokenFileName,tFileName);
		if(!(sort_token = u_fopen(tokenFileName,U_WRITE)))
			fopenErrMessage(tokenFileName);
		filleCntLine(sort_token,segments.size());	
		explore_leaf(segments.getRacine(),0);
		fclose(sort_token);

		FILE *tmpf;
		tFileName = "tok_by_freq.txt";
		tokenFileName  = new char 
			[strlen(pathName)+strlen(tFileName) +1];
		strcpy(tokenFileName,pathName);
		strcat(tokenFileName,tFileName);
		if(!(tmpf = u_fopen(tokenFileName,U_WRITE)))
			fopenErrMessage(tokenFileName);
		filleCntLine(tmpf,segments.size());	
		quicksort_by_frequence(0,segmentCount);
		for(i = 0; i < segmentCount;i++){
			fwprintf(tmpf,L"%d\t\t",n_occur[i]);
			a_token_out(tmpf,&tokTable[i][1]);
			fwrite(&retnew,4,1,tokenFile);
		}
		
		fclose(sort_token);
		segments.release_value();
		delete tokenFileName;
		fclose(tmpf);
			
		tFileName = "stats.n";
		tokenFileName  = new char 
			[strlen(pathName)+strlen(tFileName) +1];
		strcpy(tokenFileName,pathName);
		strcat(tokenFileName,tFileName);
		if(!(tmpf = u_fopen(tokenFileName,U_WRITE)))
			fopenErrMessage(tokenFileName);
		
	
		fwprintf(tmpf,L"%d sentence delimiters",sentenceCnt);
		fwprintf(tmpf,L" %d (%d diff) tokens,",
			grSegmentSum[0]+grSegmentSum[1]+grSegmentSum[2]+grSegmentSum[3],
			grSegmentCnt[0]+grSegmentCnt[1]+grSegmentCnt[2]+grSegmentCnt[3]
			);
		fwprintf(tmpf,L" %d (%d) simple forms, %d (%d) digits\n",			
			grSegmentSum[4]+grSegmentSum[5]+grSegmentSum[6]+spaceCnt,
			grSegmentCnt[4]+grSegmentCnt[5]+grSegmentCnt[6]+1,
			grSegmentSum[7],		
			grSegmentCnt[7]);
		fwprintf(tmpf,L"%d text size, %d space\n",currentOffset,spaceCnt);
		fclose(tmpf);
		delete tokenFileName;
	}
	void explore_leaf(struct arbre_hash00* noeud,int pos)
	{
		if(noeud->final != -1){
			cbuff[pos] = 0;
			a_token_out(sort_token,cbuff);
			fwprintf(sort_token,L"\t\t%d\n",noeud->value);
		}
		struct arbre_hash_trans00  **t= &noeud->transitions;
		while(*t){
			cbuff[pos] = (*t)->c;
			explore_leaf((*t)->arr,pos+1);
			t = &((*t)->suivant);
		}
	}
	int partition_pour_quicksort_by_frequence(int m, int n)
	{
		int pivot;
		int tmp;
		unsigned short * tmp_char;
		int i = m-1;
		int j = n+1;         // indice final du pivot
		pivot=n_occur[(m+n)/2];
		while (true) {
			do j--;while ((j>(m-1))&&(pivot>n_occur[j]));
			do i++;while ((i<n+1)&&(n_occur[i]>pivot));
			if (i<j) {
				tmp=n_occur[i];
				n_occur[i]=n_occur[j];
				n_occur[j]=tmp;

				tmp_char=tokTable[i];
				tokTable[i]=tokTable[j];
				tokTable[j]=tmp_char;
			} else 
				return j;
		}
	}

	void a_token_out(FILE *f,unsigned short  *t)
	{
		unsigned short  *l;
		if(*t < 0x21)
			l = ctl_char_name_string[*t];
		else
			l = t;
		fwprintf(f,L"%s",l);
	}

	void quicksort_by_frequence(int debut,int fin)
	{
		int p;
		if (debut<fin) {
			p=partition_pour_quicksort_by_frequence(debut,fin);
			quicksort_by_frequence(debut,p);
			quicksort_by_frequence(p+1,fin);
		}
	}

    int load_file_to_mem(unichar*rbuff)
    {
    	int i;
    	int readCnt;
    	int returnVal;
    	returnVal = 0;
    
    	if(fseek(textFile,currentOffset*2,SEEK_SET))
    		return returnVal;
    	if(!(readCnt= fread(rbuff,2,(maxUnitSize-1),textFile)) )
    		return(returnVal);
    	if(readCnt == (maxUnitSize -1)){
    		for(i = readCnt -1 ; i >= 0;i--)
    			if(rbuff[i] == 0x20) break;
    		if(i)	returnVal = i+1;
    		else	returnVal = readCnt;
    	}else{
    		returnVal = readCnt;
    	}		
    	rbuff[returnVal] = 0;
    #ifdef _DDEBUG
    	wprintf(L"::");
    	printSize(mot_buff,50);
    wprintf(L"::");
    	printSize(&mot_buff[i-10],10);
    wprintf(L"::");
    wprintf(L"%x:%x:%x\n",startFileOffset,i,currentOfileIndex);
    #endif
    	lastFileOffset = startFileOffset + returnVal;
    
    #ifdef TIME_DEBUG
    time(&svt1);
    printf("%d th Time for load data "
    	   "%d double chars : %d sec\n",
    		currentOfileIndex,i,svt1 - svt0);
    		debug_time_point = svt1;
    #endif //TIME_DEBUG
    	return(returnVal);
    }

    int save_mem_to_file()
    {
#ifdef FFFFF
    	int i;
#ifdef TIME_DEBUG
    	time_t svt0,svt1;
    	time(&svt0);
    	printf("Time for  sorting %d(%d diff) tokens in the memory: %d sec \n"
    			,memSegCount,memNodeCount,svt0 - debug_time_point); 
#endif //TIME_DEBUG
    
    	nodeInfo.setptrForAddNodeGl();
    	if(nomOftraitSection == 1){
    		nodeInfo.fileArrayptr = writeFile;
    		nodeInfo.tmpFileSaveSZ = nodeInfoHeadSZ;
    		if(fseek(writeFile,alignedHeadSize +
    			segmentCount 
    			* CElementOfSegmentSZ,SEEK_SET))
    			fatal_error("fseek fail\n");
    	}	
    	for( i = 0;i <NUM_OF_GROUP_SEG;i++){
    		nodeInfo.setptrForAddNode(i);
    		sortTree[i].saveToFile();
    		*nodeInfo.segCntptr = grSegmentCnt[i];
    		grSegmentCnt[i] = 0;
    		sortTree[i].deleteAllValues();
    	}
    
    	startFileOffset = lastFileOffset;
    	currentOfileIndex++;
    
    u_printf("read %d characters\n",startFileOffset);
#ifdef TIME_DEBUG
    	time(&svt1);
    	u_printf("Time for save token to file		: %d sec\n",svt1 - svt0);
#endif //TIME_DEBUG
#endif
    	return(startFileOffset);
    }
};

// 
//  get a syllable (unicode)
//  save it to the mot buffer
//  check the input characrters which is seperator or not
unichar *phraseMark = (unsigned short *)L"{S}";
void
segmentation::getSegments(unichar *wp)
{
	int index;
	int saveFlag;
	unsigned short c;

	int speStringCnt;
	unsigned char typeChar;
	int svOffset;
	spaceCnt = 0;
	unichar *swp;
	
	index = 0;

	connection = 0;
	saveFlag = 0;

	speStringCnt = 0;
	c = *wp;
	while(c){
		if(speStringCnt){
			if( c == L'}'){	// fin
					connection = 0;
					index = ++speStringCnt;
					speStringCnt = 0;
					c = *(++wp);
					currentOffset++;
					sentenceCnt++;
					continue;
			}
			if(c == phraseMark[speStringCnt]){
				speStringCnt++;
				currentOffset++;
				c = *(++wp);
				continue;
			}
			speStringCnt = 0;
			currentOffset = svOffset;
			wp = swp;
			c  = *(++wp);
			continue;
		}
		switch(c){
		case 0x0d:	// sequence [line feed]+[new line]
			currentOffset++;
			c = *(++wp);
			if(c != '\n') fatal_error("illegal text format\n");
			typeChar = SYM_CHAR;
			saveFlag = 1;
			break;
		case UNI_SP:
			connection = DEF_CONN_SPA;
			spaceCnt++;
			typeChar = SYM_CHAR;
			saveFlag = 1;
			if(*swp == UNI_SP){
				c = *(++wp);
				currentOffset++;
				continue;
			}
			break;
		case L'{':	// specical sequence
			if(index){
			saveTmps[saveTmps_cnt++] = segments.insertWordAndInc(swp,index);
//			saveTmps[saveTmps_cnt++] = lastOffset;
			if(saveTmps_cnt >= MAX_SAVETMPS_CNT){
				fwrite(saveTmps,sizeof(int)*
				MAX_SAVETMPS_CNT,1,writeFile);
				saveTmps_cnt = 0;
			}
			}
			swp = wp;
			index = 1;
			preTypeChar = SYM_CHAR;
			lastOffset = currentOffset;
			svOffset = currentOffset++;
			speStringCnt = 1;
			c = *(++wp);
			continue;
		default:
			typeChar = check_range_character(c) & TYPE_MASK ;
			saveFlag = 0;

			switch(typeChar){
			case ASC_SYL:
			case ESC_SYL:
				typeChar = ASC_CHAR;
				if(preTypeChar == typeChar) break;
				saveFlag = 1;
				break;
			case HAN_SYL:
			case HJA_SYL:
				typeChar = HAN_CHAR;
				if(preTypeChar == typeChar) break;
				saveFlag = 1;
				break;
			case NUL_SYL:
			case SYM_SYL:
			case TPH_SYL:
			case NUM_SYL:
				typeChar = SYM_CHAR;
				saveFlag =1;
			}
		}
//wprintf(L"[%c:%d:%d:%d]",c,index,saveFlag,preTypeChar);

		
		if(saveFlag && index){ // save previous type
			lastOffset= currentOffset - index;
//for(int kk = 0; kk < index;kk++)
//wprintf(L"%c",swp[kk]);
//wprintf(L":%d\n",preTypeChar);
#ifdef AVEC_STRUCT
		saveWord(preTypeChar,swp,index);
#else
		saveTmps[saveTmps_cnt++] = 	segments.insertWordAndInc(swp,index);
//		saveTmps[saveTmps_cnt++] = lastOffset;
		if(saveTmps_cnt >= MAX_SAVETMPS_CNT){
			fwrite(saveTmps,sizeof(int)*
				MAX_SAVETMPS_CNT,1,writeFile);
			saveTmps_cnt = 0;
		}	
#endif
			connection = 0;
			index = 0;
		}

//		if(c != UNI_SP){
			if(!index){
				lastOffset = currentOffset;
				swp = wp;
			}
			index++;
//		}

		preTypeChar = typeChar;
		currentOffset++;
		c = *(++wp);
	}
	if(index){
		lastOffset = currentOffset - index;
#ifdef AVEC_STRUCT
		saveWord(preTypeChar,swp,index);
#else
		saveTmps[saveTmps_cnt++] = 	segments.insertWordAndInc(swp,index);
//		saveTmps[saveTmps_cnt++] = lastOffset;
		if(saveTmps_cnt >= MAX_SAVETMPS_CNT){
			fwrite(saveTmps,sizeof(int)*
				MAX_SAVETMPS_CNT,1,writeFile);
			saveTmps_cnt = 0;
		}	
#endif
	}
	return;
}
#include <sys/stat.h>

int
segmentation::segmentFile(char *ifile_name, char *ofile_name,int MaxBufferSize)
{
#ifdef TIME_DEBUG
	time_t svt0,svt1;
#endif
	openFiles(ifile_name,ofile_name);

#ifdef TIME_DEBUG
		time(&debug_time_point);
		svt0 = debug_time_point;
#endif
	
#ifdef TIME_DEBUG
		time(&svt1);
		u_printf("\nTime for unicode file construction : %d sec\n",
			svt1 - svt0);
#endif // TIME_DEBUG
	maxUnitSize = 0x400000;
	currentOffset=1;	// keep file offset 
						// start from 1 because unicode file mark
	segmentCount = 0;
	mot_buff = new unichar[maxUnitSize];
	if(!mot_buff)
		fatal_error("mem alloc fail\n");
	while(load_file_to_mem(mot_buff)){
		getSegments(mot_buff);
	};

	fflush(writeFile);
	fclose(writeFile);
	fclose(textFile);


	static_data();
	return 0;
}
void 
segmentation::filleCntLine(FILE *f,int cnt)
{
	unsigned short line[12];
	for(int i =0; i < 12;i++) line[i] = L'0';
	line[10] = 0x0d;
	line[11] = L'\n';
	fillIntAtArray(cnt,line,10);
	fseek(f,2,SEEK_SET);
	fwrite(line,12,2,f);
}
void 
segmentation::fillIntAtArray(int v,unsigned short *A,int cdepth)
{
	int s = v/10;
	if(cdepth < 0) return;
	A[cdepth] = (v % 10) + L'0';
	if(s){
		fillIntAtArray(s,A,cdepth-1);
	}
}

//
//
//	sort the input code into 8 cateloge
//
//
//
// input : le caractere de la form de caracteres chien 
// function: save each segment according to the classfier
// handle /corea/chine/ascii/eascii/
//


#endif
