/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * File created and contributed by Gilles Vollant (Ergonotics SAS)
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.smartversion.com/unitex-contribution/
 * contact : info@winimage.com
 *
 */
 //
#include "PackFst2.h"
#include "UnusedParameter.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {



#include "AbstractAllocatorPlugCallback.h"


struct INF_codes* read_pack_inf_from_memory(const void* buf, size_t size_buf,
                                 Abstract_allocator prv_alloc);
 
 
struct INF_codes* read_pack_inf_from_file(const char* filename,
                               Abstract_allocator prv_alloc);





 

static void free_packed_INF_codes(struct INF_codes*);

void free_pack_inf(struct INF_codes* inf, Abstract_allocator prv_alloc)
{
  DISCARD_UNUSED_PARAMETER(prv_alloc)
  free_packed_INF_codes(inf);
}

#define my_around(a, b) ((((a) + (b)-1) / (b)) * (b))

struct INF_codes_monopack {
  struct INF_codes inf_codes;
  int misc;
};


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

static int read_int_on_inp(const unsigned char*p)
{
    return (((int)(*(p)))<<0) | (((int)(*(p+1)))<<8) | (((int)(*(p+2)))<<16) | (((int)(*(p+3)))<<24);
}


static void write_int_on_inp(unsigned char*p,int value)
{
    *(p+0) = (unsigned char)(value>>0);
    *(p+1) = (unsigned char)(value>>8);
    *(p+2) = (unsigned char)(value>>16);
    *(p+3) = (unsigned char)(value>>24);
}

static void invert_unichar_order(unsigned char*buf,int nb_unichar)
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


static bool is_little_endian_native()
{
  unichar c = 0x169;
  unsigned char check_order[4];
  memset(&(check_order[0]), 0, 4);
  memcpy(&(check_order[0]), &c, sizeof(unichar));
  bool check_little_endian = (check_order[1] == 1);
  // return true on x86
  return check_little_endian;
}

static int dump_inf_to_inp(const INF_codes* inf,void* rawInp,int iRawSize,int iInvertUnicharOrder)
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

    struct INP_FILE_HEADER ifh;
    memset(&ifh, 0, sizeof(ifh));
    memset(rawInp, 0, iFullFileSize);

    ifh.Sign[0] = 'I';
    ifh.Sign[1] = 'N';
    ifh.Sign[2] = 'P';
    ifh.Sign[3] = 'F';

    unichar c=0x169;
    memcpy(&ifh.SignAdd[0],&c,sizeof(unichar));
    if (iInvertUnicharOrder!=0)
        invert_unichar_order(&ifh.SignAdd[0],1);

    write_int_on_inp(&ifh.nbLine[0],iNbLine);
    write_int_on_inp(&ifh.nbString[0],iNbString);

    write_int_on_inp(&ifh.SizeUnichar[0],sizeof(unichar));
    write_int_on_inp(&ifh.PosBitArray[0],iPosBitArray);
    write_int_on_inp(&ifh.PosTextArray[0],iPosTextArray);
    write_int_on_inp(&ifh.FullFileSize[0],iFullFileSize);

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
                invert_unichar_order(TextArray+iPosOnTextArrayByte,(iCurStringLengthUnichar));
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


static bool save_inf_on_inp(const char*filename,const struct INF_codes* inf,int iInvertUnicharOrder)
{
    int res=0;
    int iSizeInp = dump_inf_to_inp(inf,NULL,0,iInvertUnicharOrder);
    if (iSizeInp == 0)
        return false;
    unsigned char* tmp1 = (unsigned char*)malloc(iSizeInp+1);
    if (tmp1==NULL)
        return false;
    int iSizeInpDone = dump_inf_to_inp(inf,tmp1,iSizeInp,iInvertUnicharOrder);

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
    return (res==0) ? false:true;
}




static bool convert_inf_file_to_inp(const char*filenameInf,const char*filenameInp,int iInvertUnicharOrder)
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
  else {
        inpName=filenameInp;
	}
    
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
    if (save_inf_on_inp(inpName, inf, iInvertUnicharOrder) == false)
    {
        res=0;
    }
	else
	{
        res=1;
    }
    free_abstract_INF(inf,&inf_free);
    
    return (res==0) ? false:true;
}

static int default_invert_unichar_order()
{
  return is_little_endian_native() ? 0 : 1;
}

bool write_pack_inp(struct INF_codes* inf, const char* inf_pack_name)
{
  return save_inf_on_inp(inf_pack_name, inf, default_invert_unichar_order());
}

bool convert_inf_to_inp_pack_file(const char* inf_name,
                                  const char* inf_pack_name)
{
  return convert_inf_file_to_inp(inf_name, inf_pack_name, default_invert_unichar_order());
}

static int check_inp_header(const void*rawInp,int sizeBuf)
{
    struct INP_FILE_HEADER ifh;
    if (sizeBuf<(int)sizeof(struct INP_FILE_HEADER))
        return 0;
    memcpy(&ifh,rawInp,sizeof(struct INP_FILE_HEADER));


    if (ifh.Sign[0] != 'I') return 0;
    if (ifh.Sign[1] != 'N') return 0;
    if (ifh.Sign[2] != 'P') return 0;
    if (ifh.Sign[3] != 'F') return 0;

    if (read_int_on_inp(&ifh.SizeUnichar[0]) != sizeof(unichar)) return 0;

    unichar c=0xabcd;
    memcpy(&c,&ifh.SignAdd[0],sizeof(unichar));
    if (c==0x169)
        return 1;
    if (c==0x6901)
        return 2;
    return 0;
}


int get_inp_secondary_buffer_size(const void*rawInp, int sizeBuf,bool fIsPermanentBinInpFile)
{
    int iCheckHeader=check_inp_header(rawInp,sizeBuf);
    if (iCheckHeader==0)
        return 0;

#ifdef VERBOSE_VIRTDELA
    {  char sz[77]; sprintf(sz,"icheckHeader1=%u, ",iCheckHeader); puts(sz);}
#endif
    struct INP_FILE_HEADER ifh;
    memcpy(&ifh,rawInp,sizeof(struct INP_FILE_HEADER));

    int iNbLine = read_int_on_inp(&ifh.nbLine[0]);
    int iNbString = read_int_on_inp(&ifh.nbString[0]);
    int iPosTextArray = read_int_on_inp(&ifh.PosTextArray[0]);
    //int iPosBitArray = read_int_on_inp(&ifh.PosBitArray[0]);
    int iFullFileSize = read_int_on_inp(&ifh.FullFileSize[0]);

    int size_list_ustring_array = ((int)sizeof(struct list_ustring*)) * (iNbLine+1);
    int size_ustring_pool = ((int)sizeof(struct list_ustring)) * (iNbString+0x10);

    int ret_size = my_around(((int)sizeof(struct INF_codes_monopack)),0x10) +
                       my_around(size_list_ustring_array,0x10) +
                       my_around(size_ustring_pool,0x10) ;

#ifdef VERBOSE_VIRTDELA
    {  char sz[77]; sprintf(sz,"icheckHeader2=%u, preret=%u ",iCheckHeader,ret_size); puts(sz);}
#endif

    if ((iCheckHeader==2) || (fIsPermanentBinInpFile==false))
        ret_size += my_around(iFullFileSize-iPosTextArray,0x10);

#ifdef VERBOSE_VIRTDELA
    {  char sz[77]; sprintf(sz,"ret=%u, ",ret_size); puts(sz);}
#endif
    return ret_size;
}

static struct INF_codes* build_inf_structure_from_inp_file(const void*rawInp, int sizeBufInp,
                                 void* SecondaryBuffer=NULL,int sizeSecondaryBuffer=0,bool fIsPermanentBinInpFile=false)
{
    int iCheckHeader=check_inp_header(rawInp,sizeBufInp);
    if (iCheckHeader == 0)
        return NULL;
    int iSecondaryBufferSizeNeeded = get_inp_secondary_buffer_size(rawInp,sizeBufInp,fIsPermanentBinInpFile);
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

    int iNbLine = read_int_on_inp(&ifh.nbLine[0]);
    int iNbString = read_int_on_inp(&ifh.nbString[0]);


    int iPosTextArray = read_int_on_inp(&ifh.PosTextArray[0]);
    int iPosBitArray = read_int_on_inp(&ifh.PosBitArray[0]);
    int iFullFileSize = read_int_on_inp(&ifh.FullFileSize[0]);


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
    if ((iCheckHeader==1) && (fIsPermanentBinInpFile!=false))
        TextArray = ((const unsigned char*)rawInp) + iPosTextArray;
    else
    {
        unsigned char*TextArrayWrite=(((unsigned char*)BufferUse)+my_around(pos_ustring_pool,0x10)+size_ustring_pool);
        memcpy(TextArrayWrite,((const unsigned char*)rawInp) + iPosTextArray,iFullFileSize-iPosTextArray);
        if (iCheckHeader==2)
            invert_unichar_order(TextArrayWrite,(iFullFileSize-iPosTextArray)/2);
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




static struct INF_codes* load_packed_INF_file(const char* filename)
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
    res = build_inf_structure_from_inp_file(file_raw, file_size);

af_fclose(f);
if (file_raw != NULL)
  free(file_raw);

return res;
}

struct INF_codes* read_pack_inf_from_memory(const void* buf, size_t size_buf,
                                            Abstract_allocator prv_alloc)
{
  DISCARD_UNUSED_PARAMETER(prv_alloc)
  return build_inf_structure_from_inp_file(buf, (int)size_buf);
}


struct INF_codes* read_pack_inf_from_permanent_memory(const void* buf, size_t size_buf,
                                                      Abstract_allocator prv_alloc,
                                                      bool permanentMemory)
{
  DISCARD_UNUSED_PARAMETER(prv_alloc)
  return build_inf_structure_from_inp_file(buf, (int)size_buf, NULL,0,permanentMemory);
}


struct INF_codes* read_pack_inf_from_permanent_memory_and_secondary_buffer(const void* buf, size_t size_buf,
                                                                           Abstract_allocator prv_alloc,
                                                                           bool permanentMemory,
                                                                           void* SecondaryBuffer,int sizeSecondaryBuffer)
{
  DISCARD_UNUSED_PARAMETER(prv_alloc)
  return build_inf_structure_from_inp_file(buf, (int)size_buf,SecondaryBuffer,sizeSecondaryBuffer,permanentMemory);
}

struct INF_codes* read_pack_inf_from_file(const char* filename,
                                          Abstract_allocator prv_alloc)
{
  DISCARD_UNUSED_PARAMETER(prv_alloc)
  return load_packed_INF_file(filename);
}

static void free_packed_INF_codes(struct INF_codes* INF)
{
    INF_codes_monopack* inf_code_monopack = (INF_codes_monopack*)INF;
    free(inf_code_monopack);
}
 

}
