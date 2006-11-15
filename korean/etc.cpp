 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "unicode.h"
#include "etc.h"

//
//  cahnge wide char string to int
//
unichar u_null_string[]= {(unichar)'\0'};
unichar u_epsilon_string[] = {(unichar)'<',(unichar)'E',(unichar)'>',(unichar)'\0'};
unichar u_phraseMark_string[] = {(unichar)'{',(unichar)'S',(unichar)'}',(unichar)'\0'};
unichar defaultSpaceStr[] = {(unichar)'<',(unichar)'S',(unichar)'P',
                                (unichar)'>',(unichar)'\0'};
int debugPrFlag;  // for debugging

int uniToInt(unichar *orgin)
{
	unichar *w = orgin;
	int s = 0;
	int base = 10;
	
	if( (*w == '0') &&
		((*(w+1) == 'x' ) || ( *(w+1) == 'X')) ){
		base = 16;
		w+=2;
	}
	while(*w){
		if((*w >= '0') && (*w <= '9')){
			s = s * base + *w - '0';
		} else if((*w >= 'a' ) && (*w <= 'f')){
			s = s * base + (*w - 'a' + 10);
		} else if((*w >= 'A' ) && (*w <= 'F')){
			s = s * base + (*w - 'A' +10);
		} else {
			u_fprintf(stdout,"%S is not correct decimal value %d ",orgin,base);
			exitMessage("");
		}
		w++;
	}
//	u_fprintf(stdout,"%s %x\n",orgin,s);
	return(s);
}
void exitMessage(char *mes)
{
	fprintf(stderr,"%s\n",mes);

//fprintf(stderr,"return");getchar();getchar();

	exit(1);
}

void fopenErrMessage(char *m){
fprintf(stderr,"%s",m);
exitMessage(" file open error");
}
void freadError(char *m){
fprintf(stderr,"%s",m);
exitMessage(" file read error");
}

int utoi(unichar *ws)
{
	int sum = 0;
	while(*ws){
		if((*ws < '0') || (*ws > '9')) break;
		sum = sum * 10 + *ws - '0';
		ws++;
	}
	return(sum);
}
int getStringTableFile(char *f,
	unsigned short *&mem,unichar **&table)
{
	FILE *fptr;
	int count;
	if((fptr = fopen(f,"rb")) ==0 )	fopenErrMessage(f);
	fseek(fptr,0,SEEK_END);	
	int sizeFile =ftell(fptr)/2;
	mem = new unsigned short[sizeFile];
	if(!mem) exitMessage("mem alloc fail");
	fseek(fptr,2,SEEK_SET);
	if(!u_fread_raw(mem,sizeFile-1,fptr))	exitMessage("Read Tokens fail");
//for(int  i = 0;i <(sizeFile -1);i++) mem[i] = u_fgetc(fptr); 
	mem[sizeFile-1] = 0;
//	mem[sizeFile-2] = 0;
	loadStrTable((unichar *)mem,table,count);
	fclose(fptr);
	return(count);
}
void 
loadStrTable(unichar *wp,unichar **&table,int &table_sz)
{
	table_sz = 0;
	for(;(*wp >= '0') && (*wp <= '9') ;wp++)
			table_sz = table_sz * 10 + *wp - '0';
	
	if(*wp == '\r') wp++;
	if(*wp == '\n') wp++;

	table = new unichar *[table_sz];
	if(!table) exitMessage("token table mem alloc fail");
	unichar *curoffset = wp;
	int index  = 0;
	while(*wp ) {
		if(*wp == 0x0a){
			*wp++ = 0;
			if(*curoffset == 0) exitMessage("Format Error");
			table[index++] = curoffset;
			curoffset = wp;
		} else  if(*wp == 0x0d) {
			*wp++ = 0;
		} else
			wp++;			
	};
	if( index != table_sz) exitMessage("illegal table size");
}
int getStringTableFileAvecNull(char *f,
	unsigned short *&mem,unichar **&table)
{
	FILE *fptr;
	int count;
	if((fptr = fopen(f,"rb")) ==0 )	fopenErrMessage(f);
	fseek(fptr,0,SEEK_END);	
	int sizeFile =ftell(fptr)/2;
	mem = new unsigned short[sizeFile];
	if(!mem) exitMessage("mem alloc fail");
	fseek(fptr,2,SEEK_SET);
    if(!u_fread_raw(mem,sizeFile-2,fptr))exitMessage("Read Tokens fail");
//    for(int  i = 0;i <(sizeFile -1);i++) mem[i] = u_fgetc(fptr); 
	mem[sizeFile-1] = 0;
//	mem[sizeFile-2] = 0;
	loadStrTableAvecNull((unichar *)mem,table,count);
	fclose(fptr);
	return(count);
}
void 
loadStrTableAvecNull(unichar *wp,unichar **&table,int &table_sz)
{
	table_sz = 0;
	for(;(*wp >= '0') && (*wp <= '9') ;wp++)
			table_sz = table_sz * 10 + *wp - '0';
	
	if(*wp == '\r') wp++;
	if(*wp == '\n') wp++;

	table = new unichar *[table_sz];
	if(!table) exitMessage("token table mem alloc fail");
	unichar *curoffset = wp;
	int index  = 0;
	while(*wp ) {
		if(*wp == 0x0a){
			*wp++ = 0;
			table[index++] = curoffset;
			curoffset = wp;
		} else  if(*wp == 0x0d) {
			*wp++ = 0;
		} else
			wp++;			
	};
	if(wp != curoffset){
	 table[index++] = curoffset;
	}
	if( index != table_sz)  {
             fprintf(stderr," index (%d %d)\n",index,table_sz);
             exitMessage("illegal table size");
        }
}
//
//	write the number of element of string table 
//
void strFileHeadLine(FILE *f,int sz)
{

      unichar tmp[100];
      u_int_to_string(sz,tmp);
      u_fprints(tmp,f);
      u_fprints_char("\n",f);
}
void fillIntAtArray(int v,unichar *A,int cdepth)
{
	int s = v/10;
	if(cdepth < 0) return;
	A[cdepth] = (v % 10) + '0';
	if(s){
		fillIntAtArray(s,A,cdepth-1);
	}
}

int binary_search(int *arr,int s,int e,int v)
{
	if(s == e) return(s);
	int k = (s+e)/2;
	if( v <= arr[k])
			return(binary_search(arr,s,k,v));
	else
			return(binary_search(arr,k+1,e,v));
}
int find_index_array(int *arr,int sz,int v)
{
//		return(binary_search(arr,0,sz-1,v));
	int s = 0;
	int e = sz -1;
	int k;
	while (s < e){
		k = (s + e) /2;
		if( v <= arr[k])
			e = k;
		else
			s = k+1;
	}
	return(s);
}
unichar get4HexVal(unichar *l)
{
	unichar su = 0;
	unichar c;
	for (int i = 0; i < 4; i++){
		c = *l++;
		
		if(  ( c >= '0' ) && (c <='9'))
			su = su * 16 + c- '0';
		else if(  ( c >= 'a' ) && (c <='f'))
			su = su * 16 + c- 'a'+10;
		else if(  ( c >= 'A' ) && (c <='9'))
			su = su * 16 +  c- 'A'+10;
		else
			exitMessage("illegal value in conv map");
	}
	return(su);
};
unichar getValueIdx(unichar *s,int &idx)
{
	if(s[idx] == '0'){
		idx ++;
		if((s[idx] == 'x') ||( s[idx] == 'X')){
			idx += 4;
			return(get4HexVal(s));
		}
		exitMessage("illegal value");
	}
	return(s[idx++]);
}
//
// change ascii number to int and return the value of the last string pointer
//
unichar *
uascToNum(unichar *uasc,int *val)
{
	unichar *wp = uasc;
	int base = 10;
	int sum = 0;
	if((*wp == '0') && 
		(*(wp+1)=='x') || 
		(*(wp+1) == 'X'))
	{
		base = 16;
		wp+=2;
	}
	do {
		if( (*wp >= '0') && (*wp <= '9')){
			sum = sum*base + *wp - '0';
				wp++;
		} else if ((base == 16) &&
				(*wp >= 'a') && (*wp <= 'f')){
				sum = sum *base + *wp - 'a'+10;
				wp++;
		} else if ((base == 16) &&
				(*wp >= 'A') && (*wp <= 'F')){
				sum = sum*base + *wp - 'A'+10;
				wp++;
		} else {
			break;
		}
	} while(*wp);
	*val = sum;
	return(wp);
}

unichar changeStrTo[16][16];
int changeStrToIdx;
int findChangeStr(unichar *v,unichar *des)
{
	int i,j;
	unichar *wp;
	for( i = 0; i< changeStrToIdx;i++){
		wp = v;
		for( j = 1;j<16 && (*wp);j++,wp++)
		{
			if(	changeStrTo[i][j] != *wp) break;
		}
		if(	!changeStrTo[i][j] && (!*wp)){
			*des = changeStrTo[i][0];
if(debugPrFlag) printf("change str %s to %d\n",getUtoChar(v),*des);
			return(1);
		}
	}
	return(0);
}
int changeStrToVal(unichar *src)
{
	unichar *wp = src;
	int i;
	while(*wp) {
		if(*wp == '=') break;
		wp++;
	}
	if(*wp != '=') return(1);
	*wp++ = '\0';
	if(!*wp) return(1);
	wp = uascToNum(wp,&i);
	if(*wp != '\0') return(1);
//	changeStrs.setVar(src,(unichar)a);
	changeStrTo[changeStrToIdx][0] = (unichar)i;
	i = 1;
	changeStrTo[changeStrToIdx][i++] = '<';	
	for(wp = src; i <16 && (*wp) ;i++)
		changeStrTo[changeStrToIdx][i] = *wp++;
	changeStrTo[changeStrToIdx][i++] = '>';	
	changeStrTo[changeStrToIdx][i++] = '\0';
	if(i == 16) {
		fprintf(stderr,"the name \"%s\"of the variable too long "
        ,getUtoChar(src));
		exitMessage("");
	}
if(debugPrFlag){
	printf("%s --> %d",getUtoChar(&changeStrTo[changeStrToIdx][1]),
	changeStrTo[changeStrToIdx][0]);
}
	changeStrToIdx++;
	return(0);
}
int setStrToVal(unichar *str,unichar v)
{
	    int i;
		changeStrTo[changeStrToIdx][0] = v;
		for(i = 0; i <16 && str[i] ;i++)
			changeStrTo[changeStrToIdx][i+1] = str[i];
		changeStrTo[changeStrToIdx][i+1] = '\0';
		if(i == 16) {
			fprintf(stderr,"The name of the variable too long %s"
            ,getUtoChar(str));
			exitMessage("");
		}
if(debugPrFlag){
	printf("%s --> %d",getUtoChar(&changeStrTo[changeStrToIdx][1]),
	changeStrTo[changeStrToIdx][0]);
}
		changeStrToIdx++;
		return(0);
}
//
//  change table
//
unichar *changeTableMap[0x10000];
int
loadChangeFileToTable(char *f)
{
	FILE *lf = u_fopen(f,U_READ);
	if(!lf) return(0);
	int idx;
	int srcIdx;
	unichar UtempLine[256];
	converTableInit();
	while(EOF!=u_read_line(lf,(unichar *)UtempLine)){
		idx = 0;
		if(UtempLine[idx] == ' ') continue;
		srcIdx = (int)UtempLine[idx++];
		if( (UtempLine[idx] != ' ') && 
			(UtempLine[idx] != '\t') )
			return(0);
		for(;UtempLine[idx];idx++);
		if(idx == 2) return 0;
		
		changeTableMap[srcIdx] = new unichar [idx - 1];
		u_strcpy((unichar *)changeTableMap[srcIdx],(unichar *)&UtempLine[2]);
	}
	return 1;
}
unichar *getConvTable(unichar v)
{
  return(changeTableMap[(int)v & 0xFFFF]);
}
void converTableInit()
{
	for(int idx = 0; idx < 0x10000;idx++) changeTableMap[idx] = 0;
}

static char charBuffOut[1024];
char *getUtoChar(unichar *s)
{
    int i;
    for(i = 0; (i < 1023 ) && s[i] ;i++)
     charBuffOut[i] = (unsigned char)(s[i] & 0x00ff);
    charBuffOut[i] = 0;
    return(charBuffOut);
}
unichar *assignUstring(unichar *src)
{

    unichar *ret = (unichar *)malloc((u_strlen(src)+1)*2);
    u_strcpy(ret,src);
    return ret;
}
