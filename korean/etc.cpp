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

#ifndef _MSC_VER
// using namespace std;
#endif

#include "Unicode.h"
#include "etc.h"
#include "Error.h"

//
//  change wide char string to int
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
			fatal_error("%S is not correct decimal value %d ",orgin,base);
		}
		w++;
	}
//	u_fprintf(stdout,"%s %x\n",orgin,s);
	return(s);
}



void fopenErrMessage(char* m){
fatal_error("%s: file open error\n",m);
}

void freadError(char* m){
fatal_error("%s: file read error\n",m);
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
	U_FILE *fptr;
	int count;
	if((fptr = u_fopen(BINARY,f,U_READ)) ==0 )	fopenErrMessage(f);
	/* It's a binary file, but we read it as a Unicode one */
	fptr->enc=UTF16_LE;
	fseek(fptr,0,SEEK_END);	
	int sizeFile =ftell(fptr)/2;
	mem = new unsigned short[sizeFile];
	if(!mem) fatal_alloc_error("getStringTableFile");
	fseek(fptr,2,SEEK_SET);
	if(!u_fread_raw(mem,sizeFile-1,fptr)) fatal_error("Read Tokens fail\n");
//for(int  i = 0;i <(sizeFile -1);i++) mem[i] = u_fgetc(fptr); 
	mem[sizeFile-1] = 0;
//	mem[sizeFile-2] = 0;
	loadStrTable((unichar *)mem,table,count);
	u_fclose(fptr);
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
	if(!table) fatal_error("token table mem alloc fail\n");
	unichar *curoffset = wp;
	int index  = 0;
	while(*wp ) {
		if(*wp == 0x0a){
			*wp++ = 0;
			if(*curoffset == 0) fatal_error("Format Error\n");
			table[index++] = curoffset;
			curoffset = wp;
		} else  if(*wp == 0x0d) {
			*wp++ = 0;
		} else
			wp++;			
	};
	if( index != table_sz) fatal_error("loadStrTable: illegal table size\n");
}
int getStringTableFileAvecNull(char *f,
	unsigned short *&mem,unichar **&table)
{
	U_FILE *fptr;
	int count;
	if((fptr = u_fopen(BINARY,f,U_READ)) ==0 )	fopenErrMessage(f);
	/* It's a binary file, but we read it as a Unicode one */
	fptr->enc=UTF16_LE;
	fseek(fptr,0,SEEK_END);	
	int sizeFile =ftell(fptr)/2;
	mem = new unsigned short[sizeFile];
	if(!mem) fatal_error("mem alloc fail\n");
	fseek(fptr,2,SEEK_SET);
    if(!u_fread_raw(mem,sizeFile-2,fptr)) fatal_error("Read Tokens fail\n");
//    for(int  i = 0;i <(sizeFile -1);i++) mem[i] = u_fgetc(fptr); 
	mem[sizeFile-1] = 0;
//	mem[sizeFile-2] = 0;
	loadStrTableAvecNull((unichar *)mem,table,count);
	u_fclose(fptr);
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
	if(!table) fatal_alloc_error("loadStrTableAvecNull");
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
             fatal_error("loadStrTableAvecNull: index (%d %d)\nillegal table size\n",index,table_sz);
        }
}
//
//	write the number of element of string table 
//
void strFileHeadLine(U_FILE* f,int sz) {
u_fprintf(f,"%d\n",sz);
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
			fatal_error("illegal value in conv map\n");
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
		fatal_error("illegal value\n");
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
if(debugPrFlag) u_printf("change str %s to %d\n",getUtoChar(v),*des);
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
		fatal_error("the name \"%s\"of the variable too long\n",getUtoChar(src));
	}
if(debugPrFlag){
	u_printf("%s --> %d",getUtoChar(&changeStrTo[changeStrToIdx][1]),
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
			fatal_error("The name of the variable too long %s\n",getUtoChar(str));
		}
if(debugPrFlag){
	u_printf("%s --> %d",getUtoChar(&changeStrTo[changeStrToIdx][1]),
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
	U_FILE *lf = u_fopen(UTF16_LE,f,U_READ);
	if(!lf) return(0);
	int idx;
	int srcIdx;
	unichar UtempLine[256];
	converTableInit();
	while(EOF!=u_fgets(UtempLine,lf)){
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
