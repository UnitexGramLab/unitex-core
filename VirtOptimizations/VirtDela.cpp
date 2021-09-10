/*
 * Unitex - Performance optimization code 
 *
 * File created and contributed by Gilles Vollant, working with François Liger
 * as part of an UNITEX optimization and reliability effort, first descibed at
 * http://www.smartversion.com/unitex-contribution/Unitex_A_NLP_engine_from_the_lab_to_the_iPhone.pdf
 *
 * Free software when used with Unitex 3.2 or later
 *
 * Copyright (C) 2021-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
  * Unitex
  *
  */

#define TRY_AUTO_LOAD_PERSISTENT_OBJECT 1

#include <string.h>
#include <stdlib.h>
#include <stdio.h>



#include "Unicode.h"
#include "DELA.h"
// VersatileEncodingConfigDefined was defined in Unitex near same time than introduce LoadInf.h
#ifdef VersatileEncodingConfigDefined
#include "LoadInf.h"

#ifndef VEC_DEFAULT
#define VEC_DEFAULT {DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT,DEFAULT_ENCODING_OUTPUT,DEFAULT_BOM_OUTPUT}
#endif

#endif
#include "AbstractDelaLoad.h"


#ifdef HAS_UNITEX_NAMESPACE
using namespace unitex;
#endif


#include "AbstractDelaPlugCallback.h"


#include "File.h"
#include "UnusedParameter.h"
#include "Error.h"
#include "Af_stdio.h"
#include "AbstractFilePlugCallback.h"


#include "VirtFileType.h"

#include "AbstractDelaLoad.h"

#include "VirtualSpaceManager.h"
#include "VirtDela.h"
#include "MiniMutex.h"


#include "VirtFileSystem.h"

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

void free_BIN(unsigned char*ptr)
{
    free(ptr);
}


/***********************************************************************/
#define MONOPACK_LOAD_INF 1

#ifdef MONOPACK_LOAD_INF
struct INF_codes* load_packed_INF_file(char*);
void free_packed_INF_codes(struct INF_codes*);

#define my_around(a,b) ((((a)+(b)-1)/(b))*(b))

struct INF_codes_monopack {
    struct INF_codes inf_codes;
    int misc;
} ;


/*
#define GetUtf8Size(ch)  \
    ((((unsigned char)(ch)) <= 0x7f) ? 1 : \
        (((((unsigned char)(ch))&0xe0)==0xC0) ? 2 : \
        (((((unsigned char)(ch))&0xf0)==0xe0) ? 3 : \
        (((((unsigned char)(ch))&0xf8)==0xf0) ? 4 : \
        (((((unsigned char)(ch))&0xfc)==0xf8) ? 5 : \
        (((((unsigned char)(ch))&0xfe)==0xfc) ? 6 : 0))))))
*/

#define GetUtf8Size(ch)  \
        (((((unsigned char)(ch)) & ((unsigned char)0x80))==((unsigned char)0x00)) ? 1 : \
        (((((unsigned char)(ch)) & ((unsigned char)0xe0))==((unsigned char)0xc0)) ? 2 : \
        (((((unsigned char)(ch)) & ((unsigned char)0xf0))==((unsigned char)0xe0)) ? 3 : \
        (((((unsigned char)(ch)) & ((unsigned char)0xf8))==((unsigned char)0xf0)) ? 4 : \
        (((((unsigned char)(ch)) & ((unsigned char)0xfc))==((unsigned char)0xf8)) ? 5 : \
        (((((unsigned char)(ch)) & ((unsigned char)0xfe))==((unsigned char)0xfc)) ? 6 : 001))))))

#define GetUtf8Mask(ch)  \
        (((((unsigned char)(ch)) & ((unsigned char)0x80))==((unsigned char)0x00)) ? ((unsigned char)0x7f) : \
        (((((unsigned char)(ch)) & ((unsigned char)0xe0))==((unsigned char)0xc0)) ? ((unsigned char)0x1f) : \
        (((((unsigned char)(ch)) & ((unsigned char)0xf0))==((unsigned char)0xe0)) ? ((unsigned char)0x0f) : \
        (((((unsigned char)(ch)) & ((unsigned char)0xf8))==((unsigned char)0xf0)) ? ((unsigned char)0x07) : \
        (((((unsigned char)(ch)) & ((unsigned char)0xfc))==((unsigned char)0xf8)) ? ((unsigned char)0x03) : \
        (((((unsigned char)(ch)) & ((unsigned char)0xfe))==((unsigned char)0xfc)) ? ((unsigned char)0x01) : 0))))))


#define GetCharSize(enc,ch) \
    ((((ch)==UTF16_LE) || ((ch)==BIG_ENDIAN_UTF16)) ? 2 : (((ch)==UTF8) ? GetUtf8Size((ch)) : 1))

struct INF_codes* load_packed_INF_file_from_memory(Encoding encoding,const unsigned char*file_raw,long file_size_byte,int number_line)
{
INF_codes_monopack* res;
int nbComma=0;
int nbLine=0;
int nbChar=0;
unichar c_before=0;
long i;

i=0;
while (i<file_size_byte)
{
    unichar c;

    switch (encoding)
    {
       case BIG_ENDIAN_UTF16: c=((*(file_raw+0+(i))) << 8) | (*(file_raw+1+(i))); i+=2; break;
       case UTF16_LE : c=((*(file_raw+1+(i))) << 8) | (*(file_raw+0+(i))); i+=2; break;
       default:
       case UTF8:
           {
               unsigned char ch= (*(file_raw+(i)));
               i++;
               if ((ch&0x80) == 0)
               {
                   c=ch;
                   break;
               }
               else
               {
                   c=ch & GetUtf8Mask(ch);
                   int nbbyte=GetUtf8Size(ch);

                   for(;;)
                   {
                       nbbyte--;
                       if (nbbyte==0)
                           break;
                       if (i==file_size_byte)
                           break;
                       c = (c<<6) | ( (*(file_raw+(i))) & 0x3F);
                       i++;
                   }
               }
           }
           break;
    }

    if (c==0)
        file_size_byte=i;
    if ((c==0x0a) && (c_before==0x0d))
        continue;

    nbChar++;
    if (c==',') nbComma++;
    if ((c==0x0d) || (c==0x0a)) nbLine++;
    if ((c==0x0a) && (c_before==0x0d)) nbLine--;
    c_before=c;
}

int pos_rawunichar = my_around((int)sizeof(INF_codes_monopack),0x10);
int size_rawunichar = (((int)sizeof(unichar)) * (nbChar+0x10));

int pos_list_ustring_array = my_around(pos_rawunichar + size_rawunichar,0x10);
int size_list_usting_array = (((int)sizeof(struct list_ustring*)) * (number_line+1));

int pos_ustring_pool = my_around(pos_list_ustring_array+size_list_usting_array,0x10);
int size_ustring_pool = ((int)sizeof(struct list_ustring))*(nbComma+nbLine+0x10);

unsigned char* main_alloc_bin=(unsigned char*)malloc(pos_ustring_pool+size_ustring_pool);

unichar* pTextPool = (unichar*)(main_alloc_bin+pos_rawunichar);

res=(struct INF_codes_monopack*)main_alloc_bin;
res->inf_codes.N = number_line;
res->inf_codes.codes=(struct list_ustring**)(main_alloc_bin+pos_list_ustring_array);
res->misc=0;

struct list_ustring* ppool = (struct list_ustring*)(main_alloc_bin+pos_ustring_pool);

int cur_line=0;
res->inf_codes.codes[cur_line]=NULL;
c_before=0;
int iCurrentStringStartPos=0;
int iCurrentUnicharWritePos=0;

i=0;

while (i<file_size_byte)
{
    unichar c;
    switch (encoding)
    {
       case BIG_ENDIAN_UTF16: c=((*(file_raw+0+(i))) << 8) | (*(file_raw+1+(i))); i+=2; break;
       case UTF16_LE : c=((*(file_raw+1+(i))) << 8) | (*(file_raw+0+(i))); i+=2; break;
       default:
       case UTF8:
           {
               unsigned char ch= (*(file_raw+(i)));
               i++;
               if ((ch&0x80) == 0)
               {
                   c=ch;
                   break;
               }
               else
               {
                   c=ch & GetUtf8Mask(ch);
                   int nbbyte=GetUtf8Size(ch);

                   for(;;)
                   {
                       nbbyte--;
                       if (nbbyte==0)
                           break;
                       if (i==file_size_byte)
                           break;
                       c = (c<<6) | ( (*(file_raw+(i))) & 0x3F);
                       i++;
                   }
               }
           }
           break;
    }

    if ((c==0x0a) && (c_before==0x0d))
    {
        continue;
    }

    if ((c==0x0d) || (c==0x0a))
    {
        *(pTextPool+iCurrentUnicharWritePos)=0;
        iCurrentUnicharWritePos++;

        ppool->string = pTextPool + iCurrentStringStartPos ;
        ppool->next = res->inf_codes.codes[cur_line];

        res->inf_codes.codes[cur_line] = ppool;

        ppool++;
        cur_line++;
        if (cur_line>number_line)
        {
            break;
        }


        res->inf_codes.codes[cur_line] = NULL;

        iCurrentStringStartPos=iCurrentUnicharWritePos;
    }
    else
    if (c==',')
    {
        *(pTextPool+iCurrentUnicharWritePos)=0;
        iCurrentUnicharWritePos++;

        ppool->string = pTextPool + iCurrentStringStartPos ;
        ppool->next = res->inf_codes.codes[cur_line];

        res->inf_codes.codes[cur_line] = ppool;

        ppool++;

        iCurrentStringStartPos=iCurrentUnicharWritePos;
    }
    else
    {
        *(pTextPool+iCurrentUnicharWritePos)=c;
        iCurrentUnicharWritePos++;
    }
    c_before=c;
}


return (INF_codes*)res;
}


static int Get_Inf_Encoding(Encoding*encoding,const unsigned char* file_raw,long filesize, int* iSkip)
{
    if (filesize>=2)
    {
        if ((*(file_raw) == 0xff) && (*(file_raw+1) == 0xfe))
        {
                *encoding=UTF16_LE;
                *iSkip=2;
                return 1;
        }
        if ((*(file_raw) == 0xfe) && (*(file_raw+1) == 0xff))
        {
                *encoding=BIG_ENDIAN_UTF16;
                *iSkip=3;
                return 1;
        }
    }
    if (filesize>=3)
    {
        if ((*(file_raw) == 0xef) && (*(file_raw+1) == 0xbb) && (*(file_raw+2) == 0xbf))
        {
                *encoding=UTF8;
                *iSkip = 3;
                return 1;
        }
        if ((*(file_raw) == '0') && (*(file_raw+1) == '0'))
        {
                *encoding=UTF8;
                *iSkip = 0;
                return 1;
        }
    }
    return 0;
}


struct INF_codes* load_packed_INF_file_memory(const unsigned char* file_raw,long filesize)
{
    int i=0;
    int is_in_num=0;
    int is_num_done=0;
    int current_value=0;

    Encoding encoding;
    int iSkip=0;
    if (Get_Inf_Encoding(&encoding,file_raw,filesize,&iSkip)==0)
        return NULL;

    int iStep;
    switch (encoding)
    {
       case UTF8: iStep=1; break;
       case BIG_ENDIAN_UTF16: iStep=2; break;
       default:
       case UTF16_LE : iStep=2; break;
    }



    while (((i*iStep)+iSkip)<filesize)
    {
        unsigned char c=((*(file_raw+iSkip+(i*iStep))));
        if (((c>='0') && (c<='9')) && (!((is_in_num==0) && (is_num_done==1))))
        {
            int iCurNumber = c - '0';
            if ((is_in_num==0) && (is_num_done==0))
            {
                current_value = iCurNumber;
                is_in_num = 1;
                is_num_done = 1;
            }
            else if (is_in_num==1)
            {
                current_value=(current_value*10)+iCurNumber;
            }
        }
        else
        {
            is_in_num=0;
            if ((c==0x0d) || (c==0x0a))
            {
                i++;
                continue;
            }

            if (is_num_done)
                break;
        }
        i++;
    }
    if (((iSkip+(i*iStep)) > filesize) || (((iSkip + (i*iStep)) == filesize) && (current_value > 0)))
        return NULL;
    if (encoding==BIG_ENDIAN_UTF16)
        iSkip--;
    return load_packed_INF_file_from_memory(encoding,file_raw+iSkip+(i*iStep),filesize-(iSkip+(i*iStep)),current_value);
}



struct INP_FILE_HEADER
{
    unsigned char Sign[4];
    unsigned char SignAdd[4];
    unsigned char nbLine[4];
    unsigned char nbString[4];

    unsigned char SizeUnichar[4];
    unsigned char PosBitArray[4];
    unsigned char PosTextArray[4];
    unsigned char FullFileSize[4];
} ;

int ReadIntOnInp(const unsigned char*p)
{
    return (((int)(*(p)))<<0) | (((int)(*(p+1)))<<8) | (((int)(*(p+2)))<<16) | (((int)(*(p+3)))<<24);
}


void WriteIntOnInp(unsigned char*p,int value)
{
    *(p+0) = (unsigned char)(value>>0);
    *(p+1) = (unsigned char)(value>>8);
    *(p+2) = (unsigned char)(value>>16);
    *(p+3) = (unsigned char)(value>>24);
}

void InvertUnicharOrder(unsigned char*buf,int nb_unichar)
{
    int i;
    for (i=0;i<nb_unichar;i++)
    {
        unsigned char a=*(buf+(i*2));
        unsigned char b=*(buf+(i*2)+1);
        *(buf+(i*2))=b;
        *(buf+(i*2)+1)=a;
    }
}

int DumpInfToInp(const INF_codes* inf,void* rawInp,int iRawSize,int iInvertUnicharOrder)
{
    int i;
    int iNbString = 0;
    int iNbStringLength = 0;
    int iNbLine = inf->N;


    for (i=0;i<iNbLine;i++)
    {
        list_ustring* ppool = inf->codes[i];
        while (ppool!=NULL)
        {
            iNbString ++;
            iNbStringLength += u_strlen(ppool->string);
            ppool = ppool->next;
        }
    }
    int iPosBitArray = my_around(sizeof(struct INP_FILE_HEADER),0x10);
    int iSizeBitArray = (iNbString+7)/8;

    int iPosTextArray = iPosBitArray + my_around(iSizeBitArray,0x10);
    int iSizeTextArray = (iNbStringLength+iNbString)*((int)sizeof(unichar));
    int iFullFileSize = iPosTextArray + my_around(iSizeTextArray,0x10);

    if (rawInp == NULL)
        return iFullFileSize;

    if (iRawSize<iFullFileSize)
        return 0;

    struct INP_FILE_HEADER  ifh;
    memset(&ifh,0,sizeof(ifh));
    memset(rawInp, 0, iFullFileSize);

    ifh.Sign[0] = 'I';
    ifh.Sign[1] = 'N';
    ifh.Sign[2] = 'P';
    ifh.Sign[3] = 'F';

    unichar c=0x169;
    memcpy(&ifh.SignAdd[0],&c,sizeof(unichar));
    if (iInvertUnicharOrder!=0)
        InvertUnicharOrder(&ifh.SignAdd[0],1);

    WriteIntOnInp(&ifh.nbLine[0],iNbLine);
    WriteIntOnInp(&ifh.nbString[0],iNbString);

    WriteIntOnInp(&ifh.SizeUnichar[0],sizeof(unichar));
    WriteIntOnInp(&ifh.PosBitArray[0],iPosBitArray);
    WriteIntOnInp(&ifh.PosTextArray[0],iPosTextArray);
    WriteIntOnInp(&ifh.FullFileSize[0],iFullFileSize);

    memcpy(rawInp,&ifh,sizeof(ifh));
    {
        unsigned char* FillHeaderEnd = ((unsigned char*)rawInp) ;
        for (i=(int)sizeof(ifh);i<(int)my_around(sizeof(struct INP_FILE_HEADER),0x10);i++)
            *(FillHeaderEnd+i)=0;
    }


    int iPosOnBitArrayByte = 0;
    unsigned char bPosOnBitArrayMask = 1;

    int iPosOnTextArrayByte = 0;
    unsigned char* TextArray = ((unsigned char*)rawInp) + iPosTextArray;

    unsigned char* BitArray = ((unsigned char*)rawInp) + iPosBitArray;


    for (i=0;i<iNbLine;i++)
    {
        list_ustring* ppool = inf->codes[i];
        while (ppool!=NULL)
        {
            int iCurStringLengthUnichar = u_strlen(ppool->string);
            int iCurStringLengthByteNeeded = (iCurStringLengthUnichar+1)*((int)sizeof(unichar)) ;
            memcpy(TextArray+iPosOnTextArrayByte,ppool->string,iCurStringLengthByteNeeded);
            if (iInvertUnicharOrder!=0)
                InvertUnicharOrder(TextArray+iPosOnTextArrayByte,(iCurStringLengthUnichar));
            iPosOnTextArrayByte += iCurStringLengthByteNeeded;

            if (ppool->next == NULL)
                *(BitArray+iPosOnBitArrayByte) |= bPosOnBitArrayMask;

            if (bPosOnBitArrayMask == 0x80)
            {
                bPosOnBitArrayMask = 1;
                iPosOnBitArrayByte++;
                *(BitArray+iPosOnBitArrayByte) = 0;
            }
            else
                bPosOnBitArrayMask *= 2;

            ppool = ppool->next;
        }
    }

    for (i=iPosOnBitArrayByte + 1;i<my_around(iSizeBitArray,0x10);i++)
        *(BitArray+i) = 0;

    for (i=iPosOnTextArrayByte;i<my_around(iSizeTextArray,0x10);i++)
        *(TextArray+i) = 0;

    return iFullFileSize;
}


ULB_VFFUNC BOOL ULIB_CALL SaveInfOnInpFile(const char*filename,const struct INF_codes* inf,int iInvertUnicharOrder)
{
    int res=0;
    int iSizeInp = DumpInfToInp(inf,NULL,0,iInvertUnicharOrder);
    if (iSizeInp == 0)
        return FALSE;
    unsigned char* tmp1 = (unsigned char*)malloc(iSizeInp+1);
    if (tmp1==NULL)
        return FALSE;
    int iSizeInpDone = DumpInfToInp(inf,tmp1,iSizeInp,iInvertUnicharOrder);

    ABSTRACTFILE* f=NULL;
    if (iSizeInpDone > 0)
        f=af_fopen(filename,"wb");
    if (f!=NULL)
    {
        {
            if (((long)af_fwrite(tmp1,1,iSizeInp,f)) == (long)iSizeInp)
                res=1;
            af_fclose(f);
        }
    }
    free(tmp1);
    return (res==0) ? FALSE:TRUE;
}




ULB_VFFUNC BOOL ULIB_CALL ConvertInfFileToInp(const char*filenameInf,const char*filenameInp,int iInvertUnicharOrder)
{
    int res;

    const char* inpName=NULL;
    char inpNameBuffer[1024];

    if (filenameInp==NULL)
    {
        int len_name=(int)strlen(filenameInf);
        if ((len_name>4) && (len_name<1024))
        {
            if (memcmp(filenameInf+len_name-4,".inf",4)==0)
            {
                strcpy(inpNameBuffer,filenameInf);
                strcpy(inpNameBuffer+len_name-4,".inp");
                inpName=inpNameBuffer;
            }
        }

    }
    else
        inpName=filenameInp;


    struct INF_free_info inf_free;
#ifdef VersatileEncodingConfigDefined
    VersatileEncodingConfig vec=VEC_DEFAULT;
    const struct INF_codes* inf=load_abstract_INF_file(&vec,filenameInf,&inf_free);
#else
    const struct INF_codes* inf=load_abstract_INF_file(filenameInf,&inf_free);
#endif
    if (inf==NULL) {
        return 1;
    }

    res=0;
    if (SaveInfOnInpFile(inpName,inf,iInvertUnicharOrder)==FALSE)
    {
        res=0;
    }
    else
    {
        res=1;
    }
    free_abstract_INF(inf,&inf_free);

    return (res==0) ? FALSE:TRUE;
}

int CheckInpHeader(const void*rawInp,int sizeBuf)
{
    struct INP_FILE_HEADER ifh;
    if (sizeBuf<(int)sizeof(struct INP_FILE_HEADER))
        return 0;
    memcpy(&ifh,rawInp,sizeof(struct INP_FILE_HEADER));


    if (ifh.Sign[0] != 'I') return 0;
    if (ifh.Sign[1] != 'N') return 0;
    if (ifh.Sign[2] != 'P') return 0;
    if (ifh.Sign[3] != 'F') return 0;

    if (ReadIntOnInp(&ifh.SizeUnichar[0]) != sizeof(unichar)) return 0;

    unichar c=0xabcd;
    memcpy(&c,&ifh.SignAdd[0],sizeof(unichar));
    if (c==0x169)
        return 1;
    if (c==0x6901)
        return 2;
    return 0;
}



int GetInpSecondaryBufferSize(const void*rawInp, int sizeBuf,BOOL fIsPermanentBinInpFile)
{
    int iCheckHeader=CheckInpHeader(rawInp,sizeBuf);
    if (iCheckHeader==0)
        return 0;

#ifdef VERBOSE_VIRTDELA
    {  char sz[77]; sprintf(sz,"icheckHeader1=%u, ",iCheckHeader); puts(sz);}
#endif
    struct INP_FILE_HEADER ifh;
    memcpy(&ifh,rawInp,sizeof(struct INP_FILE_HEADER));

    int iNbLine = ReadIntOnInp(&ifh.nbLine[0]);
    int iNbString = ReadIntOnInp(&ifh.nbString[0]);
    int iPosTextArray = ReadIntOnInp(&ifh.PosTextArray[0]);
    //int iPosBitArray = ReadIntOnInp(&ifh.PosBitArray[0]);
    int iFullFileSize = ReadIntOnInp(&ifh.FullFileSize[0]);

    int size_list_ustring_array = ((int)sizeof(struct list_ustring*)) * (iNbLine+1);
    int size_ustring_pool = ((int)sizeof(struct list_ustring)) * (iNbString+0x10);

    int ret_size = my_around(((int)sizeof(struct INF_codes_monopack)),0x10) +
                       my_around(size_list_ustring_array,0x10) +
                       my_around(size_ustring_pool,0x10) ;

#ifdef VERBOSE_VIRTDELA
    {  char sz[77]; sprintf(sz,"icheckHeader2=%u, preret=%u ",iCheckHeader,ret_size); puts(sz);}
#endif

    if ((iCheckHeader==2) || (fIsPermanentBinInpFile==FALSE))
        ret_size += my_around(iFullFileSize-iPosTextArray,0x10);

#ifdef VERBOSE_VIRTDELA
    {  char sz[77]; sprintf(sz,"ret=%u, ",ret_size); puts(sz);}
#endif
    return ret_size;
}

struct INF_codes* BuildInfStructureFromInpFile(const void*rawInp, int sizeBufInp,
                                 void* SecondaryBuffer=NULL,int sizeSecondaryBuffer=0,BOOL fIsPermanentBinInpFile=TRUE)
{
    int iCheckHeader=CheckInpHeader(rawInp,sizeBufInp);
    if (iCheckHeader == 0)
        return NULL;
    int iSecondaryBufferSizeNeeded = GetInpSecondaryBufferSize(rawInp,sizeBufInp,fIsPermanentBinInpFile);
    void * BufferUse;

    if (SecondaryBuffer != NULL)
    {
        if ((iSecondaryBufferSizeNeeded == 0) || (iSecondaryBufferSizeNeeded < sizeSecondaryBuffer))
            return NULL;
        BufferUse=SecondaryBuffer;
    }
    else
    {
        BufferUse=(void*)malloc(iSecondaryBufferSizeNeeded);
        if (BufferUse==NULL)
            return NULL;
    }


    struct INP_FILE_HEADER ifh;
    memcpy(&ifh,rawInp,sizeof(struct INP_FILE_HEADER));

    int iNbLine = ReadIntOnInp(&ifh.nbLine[0]);
    int iNbString = ReadIntOnInp(&ifh.nbString[0]);


    int iPosTextArray = ReadIntOnInp(&ifh.PosTextArray[0]);
    int iPosBitArray = ReadIntOnInp(&ifh.PosBitArray[0]);
    int iFullFileSize = ReadIntOnInp(&ifh.FullFileSize[0]);


    int pos_list_ustring_array = my_around(((int)sizeof(struct INF_codes_monopack)),0x10);
    int size_list_usting_array = ((int)sizeof(struct list_ustring*)) * (iNbLine+1);

    int pos_ustring_pool = my_around(pos_list_ustring_array+size_list_usting_array,0x10);
    int size_ustring_pool = ((int)sizeof(struct list_ustring)) * (iNbString+0x10);



    struct INF_codes_monopack* res=(struct INF_codes_monopack*)BufferUse;
    res->inf_codes.N = iNbLine;
    res->inf_codes.codes=(struct list_ustring**)(((unsigned char*)BufferUse)+pos_list_ustring_array);
    res->misc=0;

    struct list_ustring* ppool = (struct list_ustring*)(((unsigned char*)BufferUse)+pos_ustring_pool);



    int iPosOnBitArrayByte = 0;
    unsigned char bPosOnBitArrayMask = 1;
    //int iPosOnTextArrayByte = 0;


    const unsigned char* TextArray ;
    if ((iCheckHeader==1) && (fIsPermanentBinInpFile!=FALSE))
        TextArray = ((const unsigned char*)rawInp) + iPosTextArray;
    else
    {
        unsigned char*TextArrayWrite=(((unsigned char*)BufferUse)+my_around(pos_ustring_pool,0x10)+size_ustring_pool);
        memcpy(TextArrayWrite,((const unsigned char*)rawInp) + iPosTextArray,iFullFileSize-iPosTextArray);
        if (iCheckHeader==2)
            InvertUnicharOrder(TextArrayWrite,(iFullFileSize-iPosTextArray)/2);
        TextArray=TextArrayWrite;
    }

    const unsigned char* BitArray = ((const unsigned char*)rawInp) + iPosBitArray;

    int i;
    list_ustring** pInsertNextStringLocation;
    int iCurPosInTextArrayByte = 0;
    int iCurLine=0;
    pInsertNextStringLocation = &(res->inf_codes.codes[iCurLine++]);

    for (i=0;i<iNbString;i++)
    {
        *pInsertNextStringLocation = ppool;
        ppool->next = NULL;
        ppool->string = (unichar*)(TextArray + iCurPosInTextArrayByte);
        iCurPosInTextArrayByte += (u_strlen(ppool->string)+1)*((int)sizeof(unichar));

        if (((*(BitArray+iPosOnBitArrayByte)) & bPosOnBitArrayMask) != 0)
        {
            pInsertNextStringLocation = &(res->inf_codes.codes[iCurLine++]);
        }
        else
        {
            pInsertNextStringLocation = &(ppool->next);
        }

        if (bPosOnBitArrayMask == 0x80)
        {
            bPosOnBitArrayMask = 1;
            iPosOnBitArrayByte++;
        }
        else
            bPosOnBitArrayMask *= 2;

        ppool ++;
    }

    // for breakpoint at debug
    if (iCurLine != iNbLine)
        iNbLine += 0;
    return (struct INF_codes*)res;
}


#if (!(defined(BIG_ENDIAN_UTF16))) && (!(defined(USE_ABSTRACTFILE)))
#define USE_ABSTRACTFILE
#endif

#ifdef USE_ABSTRACTFILE
struct INF_codes* load_packed_INF_file(const char* filename)
{
ABSTRACTFILE* f=af_fopen(filename,"rb");
INF_codes* res=NULL;
unsigned char* file_raw;
if (f==NULL) {
   error("Cannot open %s\n",filename);
   return NULL;
}
long save_pos=af_ftell(f);
af_fseek(f,0,SEEK_END);
long file_size=af_ftell(f);
af_fseek(f,save_pos,SEEK_SET);

file_size-=save_pos;


file_raw=(unsigned char*)malloc(file_size+0x10);
if (file_raw==NULL)
{
    af_fclose(f);
    fatal_alloc_error("load_packed_INF_file");
    return NULL;
}
if (file_raw != NULL)
  if (((long)af_fread(file_raw,1,file_size,f)) == file_size)
    res=load_packed_INF_file_memory(file_raw,file_size);

af_fclose(f);
if (file_raw != NULL)
  free(file_raw);

return res;
}

#else

struct INF_codes* load_packed_INF_file(const char* filename)
{
U_FILE* f=u_fopen(UTF16_LE,filename,U_READ);
INF_codes* res=NULL;
unsigned char* file_raw;
if (f==NULL) {
   error("Cannot open %s\n",filename);
   return NULL;
}
long save_pos=ftell(f);
fseek(f,0,SEEK_END);
long file_size=ftell(f);
fseek(f,save_pos,SEEK_SET);

file_size-=save_pos;


file_raw=(unsigned char*)malloc(file_size+0x10);
if (file_raw==NULL)
{
    u_fclose(f);
    fatal_alloc_error("load_packed_INF_file");
    return NULL;
}
if (file_raw != NULL)
  if (fread(file_raw,1,file_size,f) == file_size)
    res=load_packed_INF_file_memory(file_raw,file_size);

u_fclose(f);
if (file_raw != NULL)
  free(file_raw);

return res;
}
#endif



void free_packed_INF_codes(struct INF_codes* INF)
{
    INF_codes_monopack* inf_code_monopack = (INF_codes_monopack*)INF;
    free(inf_code_monopack);
}

void free_Virtual_INF_codes(struct INF_codes* INF)
{
    free_packed_INF_codes(INF);
}
#else
void free_Virtual_INF_codes(struct INF_codes* INF)
{
    free_INF_codes(INF);
}
#endif



/***********************************************************************/


/**
 * Takes a file name and removes its extension, if any.
 */
static void remove_filename_extension(char* filename) {
signed int l;
l=((signed int)strlen(filename))-1;
while (l>=0 && filename[l]!='/' && filename[l]!='\\' && filename[l]!='.') {
   l--;
}
if (l>=0) {
   if (filename[l]=='.') filename[l]='\0';
}
}


/**
 * Takes a file name and copies it into 'result' without its extension,
 * if any.
 */
/* static void remove_filename_extension(const char* filename,char* result) {
strcpy(result,filename);
remove_filename_extension(result);
} */


typedef struct
{
    STATICARRAYC sacVirtualDicSpace;
    afs_size_type dfTotalSizeVirtualSpace;
    BOOL      fLimitSizeTotalVirtualSpace;
    afs_size_type dfMaxSizeTotalVirtualSpace;

    MINIMUTEX_OBJECT* pMiniMutex;

#ifdef TRY_AUTO_LOAD_PERSISTENT_OBJECT
    BOOL fTryAutoLoadPersistentObject;
#endif

#ifdef CHECK_AGAINST_MUTEX_REENTRANT
    int      countMutexGet;
#endif
} VIRTUALDICSPACE ;

static VIRTUALDICSPACE* pVirtualDicSpace=NULL;


#define ALLOC_MUTEX \
   { pVirtualDicSpace->pMiniMutex = BuildMutex(); } ;

#define CLEAR_MUTEX \
   { \
            if (pVirtualDicSpace->pMiniMutex!=NULL) \
            { \
                DeleteMiniMutex(pVirtualDicSpace->pMiniMutex); \
                pVirtualDicSpace->pMiniMutex = NULL; \
            } \
   } ;


#ifdef CHECK_AGAINST_MUTEX_REENTRANT
#define GET_MUTEX \
   { \
            if (pVirtualDicSpace->pMiniMutex!=NULL) \
                GetMiniMutex(pVirtualDicSpace->pMiniMutex); \
            if (pVirtualDicSpace->pMiniMutex!=NULL) pVirtualDicSpace->countMutexGet++; \
            if (pVirtualDicSpace->pMiniMutex!=NULL) if (pVirtualDicSpace->countMutexGet > 1) \
                puts("\n\nGRAVE ERROR REENTRANT MUTEX dela !!!\n\n"); \
   } ;

#define RELEASE_MUTEX \
   { \
            if (pVirtualDicSpace->pMiniMutex!=NULL) pVirtualDicSpace->countMutexGet--; \
            if (pVirtualDicSpace->pMiniMutex!=NULL) \
                ReleaseMiniMutex(pVirtualDicSpace->pMiniMutex); \
   } ;

#else

#define GET_MUTEX \
{ \
    if (pVirtualDicSpace->pMiniMutex!=NULL) \
    GetMiniMutex(pVirtualDicSpace->pMiniMutex); \
} ;

#define RELEASE_MUTEX \
{ \
    if (pVirtualDicSpace->pMiniMutex!=NULL) \
    ReleaseMiniMutex(pVirtualDicSpace->pMiniMutex); \
} ;
#endif


typedef struct
{
    const char* dfVirtualName;
    unsigned char* BinInfo;
    INF_codes* InfInfo;
    const void* pUsrPtr;
    BOOL fIsPermanentPointer;
    afs_size_type BinSize;

    ABSTRACTMAPFILE* abstractMapFileBin;
    const void* bufMapFileBin;
    size_t sizeMapBin;

    ABSTRACTMAPFILE* abstractMapFileInf;
    const void* bufMapFileInf;
    size_t sizeMapInf;
} VIRTUALDICITEMINFO;

typedef struct
{
    VIRTUALDICITEMINFO* pdii;
} VIRTUALDICITEM;



static unsigned char* GetVirtualItemBin(const char* DicName,long*filesize);
static INF_codes* GetVirtualItemInf(const char* DicName);

/*
INF_codes* load_virtual_INF_file(char*name,int* piMustBeFree)
{
    INF_codes* ret;

    if (IsVirtualDicFileName(name))
    {
        GET_MUTEX;
        ret = GetVirtualItemInf(name);
        RELEASE_MUTEX;
        *piMustBeFree=0;
    }
    else
    {
        ret = load_INF_file_Export(name);
        *piMustBeFree=1;
    }
    if (ret == NULL)
        *piMustBeFree=0;
    return ret;
}


unsigned char* load_virtual_BIN_file(char*name,int* piMustBeFree)
{
    unsigned char* ret;
    if (IsVirtualDicFileName(name))
    {
        GET_MUTEX;
        ret = GetVirtualItemBin(name);
        RELEASE_MUTEX;
        *piMustBeFree=0;
    }
    else
    {
        ret = load_BIN_file(name);
        *piMustBeFree=1;
    }
    if (ret == NULL)
        *piMustBeFree=0;
    return ret;
}
*/

static BOOL isVirtualDicPersistedForBinName(const char* FileName);
static BOOL isVirtualDicPersistedForBinNameMutexAlreadyTaken(const char* FileName);
static BOOL LoadVirtualDicFromVirtualFileMutexAlreadyTaken(const char* FileNameDicBin,
                                                           const char* DicName,
                                                           BOOL fIsPermanentBinInpFile);


ULB_VFFUNC BOOL ULIB_CALL SetVirtualDicAutoLoad(BOOL fSetAutoLoad)
{
    if (!InitVirtualDicSpace())
        return FALSE;

    if (pVirtualDicSpace==NULL)
        return FALSE;

#ifdef TRY_AUTO_LOAD_PERSISTENT_OBJECT
    pVirtualDicSpace->fTryAutoLoadPersistentObject = fSetAutoLoad;
    return TRUE;
#else
    return FALSE;
#endif
}

ULB_VFFUNC BOOL ULIB_CALL GetVirtualDicAutoLoad()
{
    if (!InitVirtualDicSpace())
        return FALSE;

    if (pVirtualDicSpace==NULL)
        return FALSE;

#ifdef TRY_AUTO_LOAD_PERSISTENT_OBJECT
    return pVirtualDicSpace->fTryAutoLoadPersistentObject;
#else
    return FALSE;
#endif
}

int ABSTRACT_CALLBACK_UNITEX IsPersistentDicFileName(const char*name,void*)
{
    if (name==NULL)
        return FALSE;
    if ((*name)==0)
        return FALSE;

    if ((*name) == '#')
        return TRUE;

    if ((*name) == '*')
        if ((*(name+1)) == '*')
            return TRUE;

    if (isVirtualDicPersistedForBinName(name))
        return TRUE;


    char tryBinName[0x400];
    size_t len_name = strlen(name);
    if ((len_name>4) && (len_name < sizeof(tryBinName)))
        if (strcmp(name+len_name-4,".inf")==0)
        {
            strcpy(tryBinName,name);
            strcpy(tryBinName+len_name-4,".bin");
            if (isVirtualDicPersistedForBinName(tryBinName))
                return TRUE;
        }

    return FALSE;
}

static int ABSTRACT_CALLBACK_UNITEX IsPersistentDicFileNameOrAutoLoad(const char*name,void* privateSpacePtr)
{
    if (name==NULL)
      return FALSE;

    if ((*name)==0)
      return FALSE;

    if (IsPersistentDicFileName(name, privateSpacePtr))
    {
        return TRUE;
    }

    int retValue = FALSE;

    BOOL TryAutoLoadPersistentObject = FALSE;
#ifdef TRY_AUTO_LOAD_PERSISTENT_OBJECT
    if ((*name) != '*')
        TryAutoLoadPersistentObject = pVirtualDicSpace->fTryAutoLoadPersistentObject;
#endif

    if (TryAutoLoadPersistentObject)
    {
        GET_MUTEX


        char rebuildBinName[0x400];
        const char* tryBinName = name;
        size_t len_name = strlen(name);
        if ((len_name>4) && (len_name < sizeof(tryBinName)))
        if (strcmp(name+len_name-4,".inf")==0)
        {
            strcpy(rebuildBinName,name);
            strcpy(rebuildBinName+len_name-4,".bin");
            tryBinName = rebuildBinName;
        }


        if (isVirtualDicPersistedForBinNameMutexAlreadyTaken(tryBinName))
        {
            RELEASE_MUTEX
            return TRUE;
        }

        retValue = LoadVirtualDicFromVirtualFileMutexAlreadyTaken(name, name, FALSE);

        RELEASE_MUTEX
    }
    return retValue;
}

int ABSTRACT_CALLBACK_UNITEX IsPersistentDicFileNameMutexAlreadyTaken(const char*name,void*)
{
    if (name==NULL)
        return FALSE;
    if ((*name)==0)
        return FALSE;

    if ((*name) == '#')
        return TRUE;

    if ((*name) == '*')
        if ((*(name+1)) == '*')
            return TRUE;

    if (isVirtualDicPersistedForBinNameMutexAlreadyTaken(name))
        return TRUE;


    char tryBinName[0x400];
    size_t len_name = strlen(name);
    if ((len_name>4) && (len_name < sizeof(tryBinName)))
        if (strcmp(name+len_name-4,".inf")==0)
        {
            strcpy(tryBinName,name);
            strcpy(tryBinName+len_name-4,".bin");
            if (isVirtualDicPersistedForBinNameMutexAlreadyTaken(tryBinName))
                return TRUE;
        }

    return FALSE;
}

INF_codes* ABSTRACT_CALLBACK_UNITEX fnc_working_load_persistent_INF_file(
#ifdef VersatileEncodingConfigDefined
                      const VersatileEncodingConfig*,
#endif
                      const char*name,struct INF_free_info* p_inf_free_info,void*)
{
    INF_codes* ret;
    GET_MUTEX;
    ret = GetVirtualItemInf(name);
    RELEASE_MUTEX;
    p_inf_free_info->must_be_free = 0;
    return ret;
}


void ABSTRACT_CALLBACK_UNITEX fnc_free_persistent_INF(struct INF_codes* INF,struct INF_free_info* p_inf_free_info,void* privateSpacePtr)
{
    DISCARD_UNUSED_PARAMETER(INF)
    DISCARD_UNUSED_PARAMETER(p_inf_free_info)
    DISCARD_UNUSED_PARAMETER(privateSpacePtr)
}


unsigned char* ABSTRACT_CALLBACK_UNITEX fnc_working_load_persistent_BIN_file(const char*name,
        #ifdef LOAD_ABSTRACT_BIN_FILE_GIVE_SIZE
        long*filesize,
        #endif
    struct BIN_free_info* p_bin_free_info,void*)
{
        #ifndef LOAD_ABSTRACT_BIN_FILE_GIVE_SIZE
        long*filesize=NULL;
        #endif
    unsigned char* ret;
    GET_MUTEX;
    ret = GetVirtualItemBin(name,filesize);
    RELEASE_MUTEX;
    p_bin_free_info->must_be_free = 0;
    return ret;
}

void ABSTRACT_CALLBACK_UNITEX fnc_free_persistent_BIN(unsigned char* BIN,struct BIN_free_info* p_bin_free_info,void* privateSpacePtr)
{
    DISCARD_UNUSED_PARAMETER(BIN)
    DISCARD_UNUSED_PARAMETER(p_bin_free_info)
    DISCARD_UNUSED_PARAMETER(privateSpacePtr)
}

const t_persistent_dic_func_array persistent_dic_func_array =
{
#ifdef IS_DELA_FUNC_ARRAY_EXTENSIBLE
    sizeof(t_persistent_dic_func_array),
#endif
    &IsPersistentDicFileNameOrAutoLoad,
    NULL,NULL,
    &fnc_working_load_persistent_INF_file,
    &fnc_free_persistent_INF,
    &fnc_working_load_persistent_BIN_file,
    &fnc_free_persistent_BIN
};

/*
void set_working_persistent_dic_func_array()
{
    set_persistent_dic_func_array_Export(&persistent_dic_func_array);
}
*/

/*
void add_working_persistent_dic_func_array()
{
    AddAbstractDelaSpace(&persistent_dic_func_array,NULL);
}
*/


#define AroundUpper(dwValue,dwModulo) (((((dwValue)) + ((dwModulo)) -1) / ((dwModulo))) * (dwModulo))




BOOL DFSCALLBACK fncDestructorVirtualDicItem(const void* lpElem)
{
    VIRTUALDICITEM* pvdi=(VIRTUALDICITEM*)lpElem;
    if (pvdi->pdii!=NULL)
    {
        if (!(pvdi->pdii->fIsPermanentPointer))
        {
            if (pvdi->pdii->BinInfo!=NULL)
            {
                free_BIN(pvdi->pdii->BinInfo);
            }
        }
        if (pvdi->pdii->InfInfo!=NULL)
        {
            free_Virtual_INF_codes(pvdi->pdii->InfInfo);
        }

        if (pvdi->pdii->dfVirtualName!=NULL)
            free((void*)(pvdi->pdii->dfVirtualName));

        pvdi->pdii->BinInfo = NULL;
        pvdi->pdii->BinSize = 0;
        pvdi->pdii->InfInfo = NULL;
        pvdi->pdii->dfVirtualName=NULL;

        pvdi->pdii->abstractMapFileBin = NULL;
        pvdi->pdii->bufMapFileBin = NULL;
        pvdi->pdii->sizeMapBin = 0;

        pvdi->pdii->abstractMapFileInf = NULL;
        pvdi->pdii->bufMapFileInf = NULL;
        pvdi->pdii->sizeMapInf = 0;

        free(pvdi->pdii);
        pvdi->pdii=NULL;
    }
    return TRUE;
}

long DFSCALLBACK fncCmpVirtualDicItem(const void* lpElem1, const void* lpElem2)
{
    VIRTUALDICITEM* pvdi1=(VIRTUALDICITEM*)lpElem1;
    VIRTUALDICITEM* pvdi2=(VIRTUALDICITEM*)lpElem2;

    return strcmp(pvdi1->pdii->dfVirtualName,pvdi2->pdii->dfVirtualName);
}


BOOL InitStaticArrayVirtualDicSpace()
{
  if (pVirtualDicSpace->sacVirtualDicSpace!=NULL)
      return TRUE;

    pVirtualDicSpace->sacVirtualDicSpace = InitStaticArrayC(sizeof(VIRTUALDICITEM),0x100);
    if (pVirtualDicSpace->sacVirtualDicSpace == NULL)
        return FALSE;

    SetFuncDestructor(pVirtualDicSpace->sacVirtualDicSpace,&fncDestructorVirtualDicItem);
    SetFuncCompareData(pVirtualDicSpace->sacVirtualDicSpace,&fncCmpVirtualDicItem);

    return TRUE;
}

ULB_VFFUNC BOOL ULIB_CALL InitVirtualDicSpace()
{
    if (pVirtualDicSpace!=NULL)
    {
        return InitStaticArrayVirtualDicSpace();
    }

    pVirtualDicSpace = (VIRTUALDICSPACE*)malloc(sizeof(VIRTUALDICSPACE));
    if (pVirtualDicSpace==NULL)
    {
        return FALSE;
    }
    ALLOC_MUTEX;

    pVirtualDicSpace->sacVirtualDicSpace = NULL;
    pVirtualDicSpace->dfTotalSizeVirtualSpace = 0;
    pVirtualDicSpace->dfMaxSizeTotalVirtualSpace = 0;
    pVirtualDicSpace->fLimitSizeTotalVirtualSpace = FALSE;

#ifdef TRY_AUTO_LOAD_PERSISTENT_OBJECT
    pVirtualDicSpace->fTryAutoLoadPersistentObject = FALSE;
#endif

#ifdef CHECK_AGAINST_MUTEX_REENTRANT
    pVirtualDicSpace->countMutexGet = 0;
#endif

    return InitStaticArrayVirtualDicSpace();
}

BOOL SetVirtualDicMaximumMemory(BOOL fLimitSize, afs_size_type dfMaxSize)
{
    if (!(InitVirtualDicSpace()))
        return FALSE;
    pVirtualDicSpace->fLimitSizeTotalVirtualSpace = fLimitSize;
    //pVirtualDicSpace->dfMaxSizeTotalVirtualSpace = dfMaxSize*1024;
    pVirtualDicSpace->dfMaxSizeTotalVirtualSpace = dfMaxSize<<10;
    return TRUE;
}

STATICARRAYC GetSacVirtualDicSpace()
{
    if (!(InitVirtualDicSpace()))
        return FALSE;
    if (pVirtualDicSpace==NULL)
        return NULL;
    return pVirtualDicSpace->sacVirtualDicSpace;
}

static BOOL UnInitVirtualDicSpaceWithDeleteOnlyIfEmptyParam(BOOL fClearMaximumMemoryValue, BOOL fClearAllPossible, BOOL fDeleteOnlyIfEmpty)
{
    if (pVirtualDicSpace!=NULL)
    {
        if (pVirtualDicSpace->sacVirtualDicSpace != NULL)
        {
            dfuLong32 dfNbItems = GetNbElem(pVirtualDicSpace->sacVirtualDicSpace);
            if (fDeleteOnlyIfEmpty && (dfNbItems > 0))
                return FALSE;
            //DeleteElem(pVirtualFileSpace->sacVirtualDicSpace, 0, GetNbElem(pVirtualFileSpace->sacVirtualDicSpace));

             DeleteStaticArrayC(pVirtualDicSpace->sacVirtualDicSpace);//+++
        }

        pVirtualDicSpace->sacVirtualDicSpace = NULL;
        pVirtualDicSpace->dfTotalSizeVirtualSpace = 0;

        if (((fClearMaximumMemoryValue) || (!pVirtualDicSpace->fLimitSizeTotalVirtualSpace)) && fClearAllPossible)
        {
            CLEAR_MUTEX;
            free(pVirtualDicSpace);
            pVirtualDicSpace = NULL;
        }
    }

    return TRUE;
}

ULB_VFFUNC BOOL ULIB_CALL UnInitVirtualDicSpace(BOOL fClearMaximumMemoryValue, BOOL fClearAllPossible)
{
    return UnInitVirtualDicSpaceWithDeleteOnlyIfEmptyParam(fClearMaximumMemoryValue, fClearAllPossible, FALSE);
}




static BOOL isVirtualDicPersistedForBinNameMutexAlreadyTaken(const char* FileName)
{
    VIRTUALDICITEM vfiSearch;
    VIRTUALDICITEMINFO vfiSearchInfo;
    dfuLong32 dwPosItem;

    STATICARRAYC sacVirtualDicSpace;

    if (!InitVirtualDicSpace())
        return FALSE;

    sacVirtualDicSpace=GetSacVirtualDicSpace();


    vfiSearchInfo.dfVirtualName = FileName;
    vfiSearch.pdii = &vfiSearchInfo;

    BOOL result = FindSameElemPos(sacVirtualDicSpace, &vfiSearch, &dwPosItem);

    return result;
}

static BOOL isVirtualDicPersistedForBinName(const char* FileName)
{
    if (!InitVirtualDicSpace())
        return FALSE;

    GET_MUTEX;

    BOOL result = isVirtualDicPersistedForBinNameMutexAlreadyTaken(FileName);

    RELEASE_MUTEX;

    return result;
}

static VIRTUALDICITEM* GetVirtualDicItemForBinNameMutexAlreadyTaken(const char* FileName)
{
    VIRTUALDICITEM vfiSearch;
    VIRTUALDICITEMINFO vfiSearchInfo;
    dfuLong32 dwPosItem;
    VIRTUALDICITEM* pDiFile;
    STATICARRAYC sacVirtualDicSpace;

    if (!InitVirtualDicSpace())
        return NULL;

    if (!IsPersistentDicFileNameMutexAlreadyTaken(FileName,NULL))
        return NULL;


    sacVirtualDicSpace=GetSacVirtualDicSpace();


    vfiSearchInfo.dfVirtualName = FileName;
    vfiSearch.pdii = &vfiSearchInfo;

    if (!FindSameElemPos(sacVirtualDicSpace, &vfiSearch, &dwPosItem))
        return NULL;
    pDiFile=(VIRTUALDICITEM*)GetElemPtr(sacVirtualDicSpace,dwPosItem);
    return pDiFile;
}

static int buildBinFileName(const char* DicNameProvided,char* dicname_reworked,size_t dicname_reworked_size,int * is_bin2)
{
    if (is_bin2!=NULL)
        *is_bin2=0;
    size_t len_DicNameProvided = strlen(DicNameProvided);
    if (len_DicNameProvided>dicname_reworked_size)
        return 0;
    strcpy(dicname_reworked,DicNameProvided);
    if (len_DicNameProvided>4)
    {
        if (strcmp(dicname_reworked+len_DicNameProvided-5,".bin2")==0)
        {
            if (is_bin2!=NULL)
                *is_bin2=1;
            return 1;
        }
    }

    remove_filename_extension(dicname_reworked);
    strcat(dicname_reworked,".bin");
    return 1;
}

static unsigned char* GetVirtualItemBin(const char* DicName,long*filesize)
{
    VIRTUALDICITEM* pvdi;
    char dicname_reworked[FILENAME_MAX+0x20];

    if (strlen(DicName)>FILENAME_MAX)
        return NULL;

    int is_bin2=0;
    buildBinFileName(DicName,dicname_reworked,FILENAME_MAX,&is_bin2);

    pvdi = GetVirtualDicItemForBinNameMutexAlreadyTaken(dicname_reworked);

    if (pvdi != NULL)
    {
        if (filesize!=NULL)
            *filesize=(long)(pvdi->pdii->BinSize);
        return pvdi->pdii->BinInfo;
    }
    else
        return NULL;
}

static INF_codes* GetVirtualItemInf(const char* DicName)
{
    VIRTUALDICITEM* pvdi;
    char dicname_reworked[FILENAME_MAX+0x20];

    if (strlen(DicName)>FILENAME_MAX)
        return NULL;


    int is_bin2=0;
    buildBinFileName(DicName,dicname_reworked,FILENAME_MAX,&is_bin2);

    if (is_bin2 != 0)
        return NULL;

    pvdi = GetVirtualDicItemForBinNameMutexAlreadyTaken(dicname_reworked);

    if (pvdi != NULL)
        return pvdi->pdii->InfInfo;
    else
        return NULL;
}

/***/

static char* strcpyAllocName(const char* str)
{
    char* ret = (char*)malloc(strlen(str)+1);
    if (ret != NULL)
        strcpy(ret,str);
    return (char*)ret;
}

static BOOL LoadVirtualDicUsrPtrWithAbstractFileToClear(const char* DicName,
                          const unsigned char* pBinContent,
                          afs_size_type size_BinFile,
                          INF_codes* InfInfo,
                          BOOL fIsPermanentPointer,
                          const void* pUsrPtr,

                          ABSTRACTMAPFILE* abstractMapFileBin,
                          const void* bufMapFileBin,
                          size_t sizeMapBin,

                          ABSTRACTMAPFILE* abstractMapFileInf,
                          const void* bufMapFileInf,
                          size_t sizeMapInf,
                          int isMutexTaken
                          )
{
    // check if exist
    VIRTUALDICITEM vfiSearch;
    VIRTUALDICITEMINFO vfiSearchInfo;
    dfuLong32 dwPosItem=0;
    BOOL fItemFound;
    STATICARRAYC sacVirtualDicSpace;
    unsigned char* pBufBinStore;
    /*
    if (!IsPersistentDicFileName(DicName,NULL))
        return FALSE;
    */
    if (!InitVirtualDicSpace())
        return FALSE;

    if (!isMutexTaken)
      GET_MUTEX;

    sacVirtualDicSpace=GetSacVirtualDicSpace();


    vfiSearchInfo.dfVirtualName = DicName;
    vfiSearch.pdii = &vfiSearchInfo;


    if (fIsPermanentPointer)
        pBufBinStore = (unsigned char*)pBinContent;
    else
    {
        /* duplicate bin if fIsPermanentPointer is false */
        // place the copy of buffer in LoadVirtualDicUsrPtr
        pBufBinStore  = (unsigned char*)malloc(size_BinFile+0x10);
        if (pBufBinStore == NULL)
        {
            if (!isMutexTaken)
              RELEASE_MUTEX;
            return FALSE;
        }
        else
        {
            int i;
            memcpy(pBufBinStore,pBinContent,size_BinFile);
            for (i=0;i<8;i++)
                *(pBufBinStore+size_BinFile+i)=0;
        }
    }


    fItemFound = FindSameElemPos(sacVirtualDicSpace, &vfiSearch, &dwPosItem);

    if (fItemFound)
    {
        BOOL fRetDel = DeleteElem(sacVirtualDicSpace,dwPosItem,1);
        if (!fRetDel)
        {
            if (!fIsPermanentPointer)
                free(pBufBinStore);
            if (!isMutexTaken)
              RELEASE_MUTEX;
            return FALSE;
        }
    }


    {
        VIRTUALDICITEM vdiAdd;
        BOOL fCreating;

        vdiAdd.pdii=(VIRTUALDICITEMINFO*)malloc(sizeof(VIRTUALDICITEMINFO));
        vdiAdd.pdii->dfVirtualName = strcpyAllocName(DicName);
        vdiAdd.pdii->BinInfo = pBufBinStore;
        vdiAdd.pdii->BinSize = size_BinFile;
        vdiAdd.pdii->InfInfo = InfInfo;
        vdiAdd.pdii->fIsPermanentPointer = fIsPermanentPointer;
        vdiAdd.pdii->pUsrPtr = pUsrPtr;

        vdiAdd.pdii->abstractMapFileBin = abstractMapFileBin;
        vdiAdd.pdii->bufMapFileBin = bufMapFileBin;
        vdiAdd.pdii->sizeMapBin = sizeMapBin;

        vdiAdd.pdii->abstractMapFileInf = abstractMapFileInf;
        vdiAdd.pdii->bufMapFileInf = bufMapFileInf;
        vdiAdd.pdii->sizeMapInf = sizeMapInf;

        fCreating = InsertSorted(sacVirtualDicSpace,&vdiAdd);
        if (!fCreating )
        {
            if (!isMutexTaken)
              RELEASE_MUTEX;
            return FALSE;
        }
        if (!FindSameElemPos(sacVirtualDicSpace, &vfiSearch, &dwPosItem))
        {
            if (!isMutexTaken)
              RELEASE_MUTEX;
            return FALSE;
        }
    }

    if (!isMutexTaken)
      RELEASE_MUTEX;
    return TRUE;
}


ULB_VFFUNC BOOL ULIB_CALL LoadVirtualDicUsrPtr(const char* DicName,
                          const unsigned char* pBinContent,
                          afs_size_type size_BinFile,
                          INF_codes* InfInfo,
                          BOOL fIsPermanentPointer,
                          const void* pUsrPtr)
{
    return LoadVirtualDicUsrPtrWithAbstractFileToClear(DicName,
                          pBinContent,
                          size_BinFile,
                          InfInfo,
                          fIsPermanentPointer,
                          pUsrPtr,
                          NULL,NULL,0,
                          NULL,NULL,0,0);
}

ULB_VFFUNC BOOL ULIB_CALL LoadVirtualDic(const char* DicName,
                    const unsigned char* BinInfo,
                    afs_size_type size_BinFile,
                    INF_codes* InfInfo,
                    BOOL fIsPermanentPointer)
{
    return LoadVirtualDicUsrPtr(DicName,
                          BinInfo,
                          size_BinFile,
                          InfInfo,
                          fIsPermanentPointer,
                          NULL);
}

ULB_VFFUNC BOOL ULIB_CALL DeleteVirtualDic(const char* DicName)
{

    VIRTUALDICITEM vfiSearch;
    VIRTUALDICITEMINFO vfiSearchInfo;
    dfuLong32 dwPosItem;
    BOOL fRet;
    STATICARRAYC sacVirtualDicSpace;
    char dicname_reworked[FILENAME_MAX+0x20];
    unsigned char* BinInfoLazzyClear = NULL;
    INF_codes* InfInfoLazzyClear = NULL;

    if (!IsPersistentDicFileName(DicName,NULL))
        return FALSE;

    if (!InitVirtualDicSpace())
        return FALSE;

    if (strlen(DicName)>FILENAME_MAX)
        return FALSE;

    GET_MUTEX;

    int is_bin2=0;
    buildBinFileName(DicName,dicname_reworked,FILENAME_MAX,&is_bin2);

    sacVirtualDicSpace=GetSacVirtualDicSpace();


    vfiSearchInfo.dfVirtualName = dicname_reworked;
    vfiSearch.pdii = &vfiSearchInfo;

    if (!FindSameElemPos(sacVirtualDicSpace, &vfiSearch, &dwPosItem))
    {
        RELEASE_MUTEX;
        return FALSE;
    }

    {
        /* before, we just do a fRet = DeleteElem(sacVirtualDicSpace,dwPosItem,1);
           But we prefer copy the InfInfo and BinInfo pointer, and set to NULL
           so the Destructor in DeleteElem will not do the free_Virtual_INF_codes (which
           can cost CPU). We can do it after release the Mutex */

        VIRTUALDICITEM* pDiFile;
        pDiFile=(VIRTUALDICITEM*)GetElemPtr(sacVirtualDicSpace,dwPosItem);
        if (pDiFile != NULL)
        {
            VIRTUALDICITEMINFO* pDiiFile ;
            pDiiFile = pDiFile->pdii;
            if (!(pDiiFile->fIsPermanentPointer))
            {
                BinInfoLazzyClear = pDiiFile->BinInfo;
                pDiiFile->BinInfo = NULL;
                pDiiFile->BinSize = 0;
            }


            if (pDiiFile->bufMapFileBin != NULL)
            {
                af_release_mapfile_pointer(pDiiFile->abstractMapFileBin,pDiiFile->bufMapFileBin,pDiiFile->sizeMapBin);
                pDiiFile->bufMapFileBin = NULL;
                pDiiFile->sizeMapBin = 0;
            }

            if (pDiiFile->abstractMapFileBin != NULL)
            {
                af_close_mapfile(pDiiFile->abstractMapFileBin);
                pDiiFile->abstractMapFileBin = NULL;
            }

            if (pDiiFile->bufMapFileInf != NULL)
            {
                af_release_mapfile_pointer(pDiiFile->abstractMapFileInf,pDiiFile->bufMapFileInf,pDiiFile->sizeMapInf);
                pDiiFile->bufMapFileInf = NULL;
                pDiiFile->sizeMapInf = 0;
            }

            if (pDiiFile->abstractMapFileInf != NULL)
            {
                af_close_mapfile(pDiiFile->abstractMapFileInf);
                pDiiFile->abstractMapFileInf = NULL;
            }

            InfInfoLazzyClear = pDiiFile->InfInfo;
            pDiiFile->InfInfo = NULL;
        }
        fRet = DeleteElem(sacVirtualDicSpace,dwPosItem,1);
    }


    if (fRet)
    {
        dfuLong32 dwNbElem=GetNbElem(sacVirtualDicSpace);
        if (dwNbElem==0)
        {
            //UnInitVirtualDicSpace(FALSE,FALSE,TRUE);
        }
    }

    RELEASE_MUTEX;

    if (BinInfoLazzyClear != NULL)
        free_BIN(BinInfoLazzyClear);
    if (InfInfoLazzyClear != NULL)
        free_Virtual_INF_codes(InfInfoLazzyClear);

    return fRet;
}
/******************************************************************/

static BOOL LoadVirtualDicFromVirtualFileUsrPtrWithMutexTaken(const char* FileNameDicBin,
                                         const char* DicName,
                                         BOOL fIsPermanentBinInpFile,
                                         const void* pUsrPtr, int isMutexTaken)
{
    const void* pBinContent=NULL;
    afs_size_type size_bin=0;
    INF_codes* inf=NULL;
    char dicname[FILENAME_MAX+0x20];
    BOOL fRet;


    ABSTRACTMAPFILE* abstractMapFileBin=NULL;
    const void* bufMapFileBin=NULL;
    size_t sizeMapBin=0;

    ABSTRACTMAPFILE* abstractMapFileInf=NULL;
    const void* bufMapFileInf=NULL;
    size_t sizeMapInf=0;

    if (strlen(FileNameDicBin)>FILENAME_MAX)
        return FALSE;

    int is_bin2=0;
    buildBinFileName(FileNameDicBin,dicname,FILENAME_MAX,&is_bin2);

    //if (!GetMemoryBufferFromMemFile(dicname,&pBinContent,&size_bin))
    //    return FALSE;

    abstractMapFileBin = af_open_mapfile(dicname,MAPFILE_OPTION_READ,0);
    if (abstractMapFileBin == NULL)
        return FALSE;
    sizeMapBin = af_get_mapfile_size(abstractMapFileBin);
    if (sizeMapBin>0)
        bufMapFileBin = af_get_mapfile_pointer(abstractMapFileBin, 0, sizeMapBin);
    if (bufMapFileBin == NULL)
    {
        af_close_mapfile(abstractMapFileBin);
        return FALSE;
    }

    pBinContent = bufMapFileBin;
    size_bin = sizeMapBin ;

    if ((bufMapFileBin != NULL) && (sizeMapBin!=0) && (fIsPermanentBinInpFile))
    {
        const void* pBinContent_vfs = NULL;
        size_t size_bin_vfs = 0;
        if (GetMemoryBufferFromMemFile(dicname,&pBinContent_vfs,&size_bin_vfs))
            if ((pBinContent_vfs == bufMapFileBin) && (size_bin_vfs == sizeMapBin))
            {

                af_release_mapfile_pointer(abstractMapFileBin,bufMapFileBin,sizeMapBin);
                af_close_mapfile(abstractMapFileBin);
                abstractMapFileBin = NULL;
                // map in vfs
            }
    }

    //    return FALSE;
    remove_filename_extension(dicname);

#ifdef MONOPACK_LOAD_INF
    if (is_bin2 == 0)
    {
        const void* pInpContent=NULL;
        afs_size_type size_inp=0;
        char* dirext = dicname+strlen(dicname);
        strcpy(dirext,".inp");

        abstractMapFileInf = af_open_mapfile(dicname,MAPFILE_OPTION_READ,0);
        if (abstractMapFileInf != NULL)
        {
            sizeMapInf = af_get_mapfile_size(abstractMapFileInf);
            if (sizeMapInf > 0)
                bufMapFileInf = af_get_mapfile_pointer(abstractMapFileInf, 0, sizeMapInf);
            if (bufMapFileInf == NULL)
            {
                af_close_mapfile(abstractMapFileInf);
                abstractMapFileInf = NULL ;
                sizeMapInf = 0;
            }
        }
        pInpContent = bufMapFileInf;
        size_inp = sizeMapInf ;



        //if (GetMemoryBufferFromMemFile(dicname,&pInpContent,&size_inp))
        if (pInpContent != NULL)
            inf = BuildInfStructureFromInpFile((const unsigned char*)pInpContent,(int)size_inp,NULL,0,fIsPermanentBinInpFile);

        if (inf == NULL)
        {
            const void* pInfContent=NULL;
            afs_size_type size_inf=0;
            strcpy(dirext,".inf");

            abstractMapFileInf = af_open_mapfile(dicname,MAPFILE_OPTION_READ,0);
            if (abstractMapFileInf != NULL)
            {
                sizeMapInf = af_get_mapfile_size(abstractMapFileInf);
                if (sizeMapInf > 0)
                    bufMapFileInf = af_get_mapfile_pointer(abstractMapFileInf, 0, sizeMapInf);
                if (bufMapFileInf == NULL)
                {
                    af_close_mapfile(abstractMapFileInf);
                    abstractMapFileInf = NULL ;
                    sizeMapInf = 0;
                }
            }
            pInfContent = bufMapFileInf;
            size_inf = sizeMapInf ;

            //if (GetMemoryBufferFromMemFile(dicname,&pInfContent,&size_inf))
            if (pInfContent != NULL)
            {
                inf = BuildInfStructureFromInpFile((const unsigned char*)pInfContent,(int)size_inf,NULL,0,fIsPermanentBinInpFile);
                if (inf==NULL)
                  inf = load_packed_INF_file_memory((const unsigned char*)pInfContent,(long)size_inf);
            }
        }
    }
#else
    strcat(dicname,".inf");
#ifdef VersatileEncodingConfigDefined
const VersatileEncodingConfig vecDefault={DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT,DEFAULT_ENCODING_OUTPUT,DEFAULT_BOM_OUTPUT};
#endif

    if (is_bin2 == 0)
        inf = load_INF_file(
#ifdef VersatileEncodingConfigDefined
            &vecDefault,
#endif
            dicname);
#endif

    if ((inf == NULL) && (is_bin2 == 0))
        return FALSE;

/*
#if defined(MONOPACK_LOAD_INF) && ((defined(_DEBUG) || defined(DEBUG)))
    {
        int iSizeInp = DumpInfToInp(inf,NULL,0);
        unsigned char* tmp1 = (unsigned char*)malloc(iSizeInp+1);
        int iSizeInpDone = DumpInfToInp(inf,tmp1,iSizeInp);

        int iSizeSecondaryBuffer = GetInpSecondaryBufferSize(tmp1, iSizeInp);
        unsigned char* tmp2 = (unsigned char*)malloc(iSizeSecondaryBuffer+1);
        struct INF_codes* infBis= BuildInfStructureFromInpFile(tmp1,iSizeInp,tmp2,iSizeSecondaryBuffer);

        int iSizeInp3 = DumpInfToInp(infBis,NULL,0);
        unsigned char* tmp3 = (unsigned char*)malloc(iSizeInp3+1);
        int iSizeInpDone3 = DumpInfToInp(inf,tmp3,iSizeInp3);
        int icmp = memcmp(tmp1,tmp3,iSizeInpDone);
        icmp +=0;
    }
#endif
*/

    fRet = TRUE;

    // a conservative test for compatibility : for file in VFS with permanent buffer
    //   we close now the memory map object
    if ((bufMapFileInf != NULL) && (sizeMapInf!=0) && (fIsPermanentBinInpFile))
    {
        const void* pInfContent_vfs = NULL;
        size_t size_inf_vfs = 0;
        if (GetMemoryBufferFromMemFile(dicname,&pInfContent_vfs,&size_inf_vfs))
            if ((pInfContent_vfs == bufMapFileInf) && (size_inf_vfs == sizeMapInf))
            {

                af_release_mapfile_pointer(abstractMapFileInf,bufMapFileInf,sizeMapInf);
                af_close_mapfile(abstractMapFileInf);
                abstractMapFileInf = NULL;
                // map in vfs
            }
    }


    fRet = LoadVirtualDicUsrPtrWithAbstractFileToClear(DicName,
                    (const unsigned char*)pBinContent,size_bin,inf,fIsPermanentBinInpFile,pUsrPtr,

                          abstractMapFileBin,bufMapFileBin,sizeMapBin,
                          abstractMapFileInf,bufMapFileInf,sizeMapInf,isMutexTaken);

    if ((!fRet) && (inf != NULL))
        free_Virtual_INF_codes(inf);

    return fRet;
}

ULB_VFFUNC BOOL ULIB_CALL LoadVirtualDicFromVirtualFileUsrPtr(const char* FileNameDicBin,
                                                              const char* DicName,
                                                              BOOL fIsPermanentBinInpFile,
                                                              const void* pUsrPtr)
{
    return LoadVirtualDicFromVirtualFileUsrPtrWithMutexTaken(FileNameDicBin,DicName,
                                                             fIsPermanentBinInpFile,pUsrPtr, 0);
}

static BOOL LoadVirtualDicFromVirtualFileMutexAlreadyTaken(const char* FileNameDicBin,
                                                      const char* DicName,
                                                      BOOL fIsPermanentBinInpFile)
{
    return LoadVirtualDicFromVirtualFileUsrPtrWithMutexTaken(FileNameDicBin,DicName,
                                               fIsPermanentBinInpFile,NULL, 1);
}


ULB_VFFUNC BOOL ULIB_CALL LoadVirtualDicFromVirtualFile(const char* FileNameDicBin,
                                   const char* DicName,
                                   BOOL fIsPermanentBinInpFile)
{
    return LoadVirtualDicFromVirtualFileUsrPtr(FileNameDicBin,DicName,
                                               fIsPermanentBinInpFile,NULL);
}



/*
typedef struct
{
    const char* name;
    const unsigned char* BinInfo;
    const INF_codes* InfInfo;
    BOOL fIsPermanentPointer;
    unsigned long dwReservedMagicValue;
} ENUM_VIRTUAL_DIC;
*/
ULB_VFFUNC BOOL ULIB_CALL InitPersistentDicEnumeration(ENUM_VIRTUAL_DIC*);
ULB_VFFUNC BOOL ULIB_CALL GetNextPersistentDicEnumeration(ENUM_VIRTUAL_DIC*);
ULB_VFFUNC BOOL ULIB_CALL ClosePersistentDicEnumeration(ENUM_VIRTUAL_DIC*);
ULB_VFFUNC BOOL ULIB_CALL DeletePersistentDicCurrentlyEnumerated(ENUM_VIRTUAL_DIC*);




ULB_VFFUNC BOOL ULIB_CALL InitPersistentDicEnumeration(ENUM_VIRTUAL_DIC* pevd)
{
    if (!InitVirtualDicSpace())
        return FALSE;

    pevd->dwReservedMagicValue = 0;
    GET_MUTEX;
    return TRUE;
}

ULB_VFFUNC BOOL ULIB_CALL GetNextPersistentDicEnumeration(ENUM_VIRTUAL_DIC* pevd)
{
    STATICARRAYC sacVirtualDicSpace;
    VIRTUALDICITEM* pDiFile;
    VIRTUALDICITEMINFO* pDiiFile;

    if (!InitVirtualDicSpace())
        return FALSE;

    sacVirtualDicSpace=GetSacVirtualDicSpace();

    pDiFile=(VIRTUALDICITEM*)GetElemPtr(sacVirtualDicSpace,pevd->dwReservedMagicValue);
    if (pDiFile == NULL)
        return FALSE;
    pDiiFile = pDiFile->pdii;

    pevd->name = pDiiFile->dfVirtualName;
    pevd->BinInfo = pDiiFile->BinInfo;
    pevd->fIsBinInpPermanentPointer = pDiiFile->fIsPermanentPointer;
    pevd->InfInfo = pDiiFile->InfInfo;
    pevd->pUsrPtr = pDiiFile->pUsrPtr ;

    pevd->dwReservedMagicValue++;
    return TRUE;
}

ULB_VFFUNC BOOL ULIB_CALL ClosePersistentDicEnumeration(ENUM_VIRTUAL_DIC*)
{
    if (!InitVirtualDicSpace())
        return FALSE;

    RELEASE_MUTEX;
    return TRUE;
}


ULB_VFFUNC BOOL ULIB_CALL DeletePersistentDicCurrentlyEnumerated(ENUM_VIRTUAL_DIC* pevd)
{
    STATICARRAYC sacVirtualDicSpace;
    BOOL fRet;

    /* 0 : we have returned nothing ! */
    /* pevd->dwReservedMagicValue contain the item number of the next to be returned,
        so the last returned item is pevf->dwReservedMagicValue -1 */

    if (pevd->dwReservedMagicValue == 0)
        return FALSE;

    if (!InitVirtualDicSpace())
        return FALSE;

    sacVirtualDicSpace=GetSacVirtualDicSpace();

    fRet = DeleteElem(sacVirtualDicSpace,pevd->dwReservedMagicValue-1,1);
    if (fRet)
        pevd->dwReservedMagicValue--;
    return fRet;
}


/**************************/



class autoinstall_virt_dela {
public:
    autoinstall_virt_dela() {
        InitVirtualDicSpace();
        AddAbstractDelaSpace(&persistent_dic_func_array,this);
    };

    ~autoinstall_virt_dela() {
        RemoveAbstractDelaSpace(&persistent_dic_func_array,this);
        UnInitVirtualDicSpaceWithDeleteOnlyIfEmptyParam(TRUE, TRUE, TRUE);
    };

} ;

autoinstall_virt_dela autoinstall_virt_dela_instance;

/*
int DoAddDela()
{
    add_working_persistent_dic_func_array();
    return 0;
}

int autoinitDelaDummy=DoAddDela();
*/
