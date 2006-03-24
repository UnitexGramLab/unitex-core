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


#define MAX_CONSULTATION_DIC    128
#define MAX_CACHE_LENGTH       2048
class consultation_dictionnaire {
FILE *textCodeFile;
unsigned int cacheOfIndex[MAX_CACHE_LENGTH];
int lastReadOffset;
int curTokenIndex;
int curTokenCacheCnt;

int curTokenValue;
class explore_bin1 dictionnaire[MAX_CONSULTATION_DIC];
int dictionnaireCnt;
class arbre_string02 sequenceConnu;
unsigned int *findTokenTable;
int       sizeFindTokenTable;
wchar_t **tokenTable;
int tokenCount;
unsigned char *memAllocTmp;  // mem reserve for tokens files
public:
    consultation_dictionnaire(){
        dictionnaireCnt = 0;
    };
    ~consultation_dictionnaire(){
        if(tokenTable){
             delete memAllocTmp;
             delete tokenTable;
        }
        if(findTokenTable)
        delete findTokenTable;
    };
    int cacheIndex;
    void consutlationTextCode(char *fna)
    {
        int findCnt;
        int dicIndex;
        int cindex;
        int searchDepth;
        
        textCodeFile = fopen(fna,"rb");
        if(!textCodeFile) fopenErrMessage(fna);
        curTokenCacheCnt = 0;
        curTokenIndex = 0;
        cacheIndex = 0;
        
        while((cindex = getNextToken(0)) >= 0){
            findCnt= 0;
            // find in serial word
            searchDepth =((cacheIndex + sequenceConnu.sequenceMaxDepth) < MAX_CACHE_LENGTH) ?
             MAX_CACHE_LENGTH - cacheIndex:sequenceConnu.sequenceMaxDepth;
            if(sequenceConnu.find( &cacheOfIndex[curTokenIndex],searchDepth) != 0xffffffff) continue;
            if(findTokenTable[curTokenValue/32] & bitSetL[curTokenValue%32]) continue;
            for(dicIndex = 0; dicIndex < dictionnaireCnt;dicIndex++){
               dictionnaire[curTokenIndex].searchMotAtTree(tokenTable[cindex],0);
             }
             findTokenTable[curTokenValue/32] |= bitSetL[curTokenValue%32];
        }
    }
    
    wchar_t *getNextWord(int avanceIndex)
    {
        int index;
        
        return(tokenTable[index]);
    }
    int getNextToken(int avanceIndex)
    {
         int readCnt;
        if(cacheIndex < curTokenCacheCnt) return(cacheIndex++);
        
        curTokenIndex += curTokenCacheCnt;
        if(readCnt = fread(cacheOfIndex,4,MAX_CACHE_LENGTH,textCodeFile)){
            cacheIndex = 0;
            curTokenCacheCnt = readCnt;
             return(0);
        }
        return(-1);
    }
    int loadTokenTable(char *file)
    {
    
       tokenCount =  getStringTableFile(file,memAllocTmp,tokenTable);
       sizeFindTokenTable  = (tokenCount + 31) / 32;
       findTokenTable = new unsigned int [sizeFindTokenTable ];
       for( int i = 0; i < sizeFindTokenTable ;i++)
              findTokenTable[i] = 0;
       return(tokenCount);
    }
    void load_un_dic(char *dicf)
    {
            dictionnaire[dictionnaireCnt].loadBin(dicf);
	        dictionnaire[dictionnaireCnt].set_act_func(actForFinal,actForInfo);
	        dictionnaireCnt++;
    }
    
};

