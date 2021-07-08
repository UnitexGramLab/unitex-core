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
  *
  */

#ifndef VIRTDELAH
#define VIRTDELAH

//#include "DELA.h"
#include "VirtDela_Basic.h"
#include "FuncDeclModifier.h"
#include "VirtFileType.h"

#ifdef __cplusplus



#ifdef HAS_UNITEX_NAMESPACE
using namespace unitex;
#endif

extern "C" {
#endif


#ifdef HAS_UNITEX_NAMESPACE
namespace unitex {
struct INF_codes ;
}
#else
struct INF_codes ;
#endif

ULB_VFFUNC BOOL ULIB_CALL LoadVirtualDic(const char* DicName,
                    const unsigned char* BinInfo,
                    afs_size_type size_BinFile,
                    struct INF_codes* InfInfo,
                    BOOL fIsBinInpPermanentPointer);

ULB_VFFUNC BOOL ULIB_CALL LoadVirtualDicUsrPtr(const char* DicName,
                          const unsigned char* BinInfo,
                          afs_size_type size_BinFile,
                          struct INF_codes* InfInfo,
                          BOOL fIsBinInpPermanentPointer,
                          const void* pUsrPtr);

ULB_VFFUNC BOOL ULIB_CALL DeleteVirtualDic(const char* DicName);

typedef struct
{
    const char* name;
    const unsigned char* BinInfo;
    const struct INF_codes* InfInfo;
    const void* pUsrPtr;
    BOOL fIsBinInpPermanentPointer;
    unsigned long dwReservedMagicValue;
} ENUM_VIRTUAL_DIC;

ULB_VFFUNC BOOL ULIB_CALL InitPersistentDicEnumeration(ENUM_VIRTUAL_DIC*);
ULB_VFFUNC BOOL ULIB_CALL GetNextPersistentDicEnumeration(ENUM_VIRTUAL_DIC*);
ULB_VFFUNC BOOL ULIB_CALL ClosePersistentDicEnumeration(ENUM_VIRTUAL_DIC*);
ULB_VFFUNC BOOL ULIB_CALL DeletePersistentDicCurrentlyEnumerated(ENUM_VIRTUAL_DIC*);


ULB_VFFUNC BOOL ULIB_CALL SaveInfOnInpFile(const char*filename,const struct INF_codes* inf,int iInvertUnicharOrder);
ULB_VFFUNC BOOL ULIB_CALL ConvertInfFileToInp(const char*filenameInf,const char*filenameInp,int iInvertUnicharOrder);
    
#ifdef __cplusplus
}
#endif

#endif
