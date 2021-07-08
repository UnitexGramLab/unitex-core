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

#ifndef VIRTFST2H
#define VIRTFST2H


#ifdef __cplusplus


#ifdef HAS_UNITEX_NAMESPACE
using namespace unitex;
#endif

extern "C" {
#endif

ULB_VFFUNC BOOL ULIB_CALL InitVirtualGraphSpace();
ULB_VFFUNC BOOL ULIB_CALL UnInitVirtualGraphSpace(BOOL fClearMaximumMemoryValue, BOOL fClearAllPossible);

    
#if (!defined(Fst2H)) && (!defined(__cplusplus))
typedef struct fst2 Fst2;
#endif

ULB_VFFUNC Fst2* ULIB_CALL load_virtual_FST2_file(char*name,int read_names,int* piMustBeFree);


ULB_VFFUNC BOOL ULIB_CALL LoadVirtualGraphFst2(const char* GraphName,int read_names,
                      Fst2* Fst2Graph);

ULB_VFFUNC BOOL ULIB_CALL LoadVirtualGraphFst2UsrPtr(const char* GraphName,int read_names,
                            Fst2* Fst2Graph,
                            const void* pUsrPtr);

ULB_VFFUNC BOOL ULIB_CALL LoadVirtualGraphFst2FromVirtualFile(const char* GraphName,const char*VirtGraphName,int read_names);
ULB_VFFUNC BOOL ULIB_CALL LoadVirtualGraphFst2FromVirtualFileUsrPtr(const char* GraphName,const char*VirtGraphName,int read_names,
                                         const void* pUsrPtr);

ULB_VFFUNC BOOL ULIB_CALL DeleteVirtualGraphFst2(const char* VirtGraphName,int read_names);

ULB_VFFUNC BOOL ULIB_CALL SaveFst2OnFst2PackFile(const char*filenameSrc,const char*filenameDest,int iVerbose);

ULB_VFFUNC BOOL ULIB_CALL SetVirtualFst2AutoLoad(BOOL);

ULB_VFFUNC BOOL ULIB_CALL GetVirtualFst2AutoLoad();

typedef struct
{
    const char* name;
    const Fst2* Fst2Graph;
    const void* pUsrPtr;
	int read_names;
    unsigned long dwReservedMagicValue;
} ENUM_VIRTUAL_GRAPH;

ULB_VFFUNC BOOL ULIB_CALL InitPersistentGraphEnumeration(ENUM_VIRTUAL_GRAPH*);
ULB_VFFUNC BOOL ULIB_CALL GetNextPersistentGraphEnumeration(ENUM_VIRTUAL_GRAPH*);
ULB_VFFUNC BOOL ULIB_CALL ClosePersistentGraphEnumeration(ENUM_VIRTUAL_GRAPH*);
ULB_VFFUNC BOOL ULIB_CALL DeletePersistentGraphCurrentlyEnumerated(ENUM_VIRTUAL_GRAPH*);


#ifdef __cplusplus
}
#endif

#endif

