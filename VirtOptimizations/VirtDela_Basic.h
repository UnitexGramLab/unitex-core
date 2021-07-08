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

#ifndef VIRTDELA_BASIC_H
#define VIRTDELA_BASIC_H



#ifdef __cplusplus
extern "C" {
#endif

#include "FuncDeclModifier.h"


ULB_VFFUNC BOOL ULIB_CALL InitVirtualDicSpace();
ULB_VFFUNC BOOL ULIB_CALL UnInitVirtualDicSpace(BOOL fClearMaximumMemoryValue, BOOL fClearAllPossible);


ULB_VFFUNC BOOL ULIB_CALL LoadVirtualDicFromVirtualFile(const char* FileNameDicBin,
                                   const char* DicName,
                                   BOOL fIsPermanentBinInpFile);

ULB_VFFUNC BOOL ULIB_CALL LoadVirtualDicFromVirtualFileUsrPtr(const char* FileNameDicBin,
                                         const char* DicName,
                                         BOOL fIsPermanentBinInpFile,
                                         const void* pUsrPtr);

ULB_VFFUNC BOOL ULIB_CALL DeleteVirtualDic(const char* DicName);


ULB_VFFUNC BOOL ULIB_CALL SetVirtualDicAutoLoad(BOOL);

ULB_VFFUNC BOOL ULIB_CALL GetVirtualDicAutoLoad();

#ifdef __cplusplus
}
#endif

#endif
