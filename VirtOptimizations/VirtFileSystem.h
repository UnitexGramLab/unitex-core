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
#ifndef _VIRT_FILE_SYSTEM_DEFINED
#define _VIRT_FILE_SYSTEM_DEFINED

//typedef void* LOWLEVELFILE;

// low level operations


#ifdef __cplusplus
extern "C" {
#endif
ULB_VFFUNC BOOL ULIB_CALL InitVirtualFileNameSpace();
ULB_VFFUNC BOOL ULIB_CALL UnInitVirtualFileNameSpace(BOOL fClearMaximumMemoryValue);
ULB_VFFUNC BOOL ULIB_CALL SetVirtualFileNameMaximumMemory(BOOL fLimitSize, afs_size_type dfMaxSize);

ABSTRACTFILE_PTR ABSTRACT_CALLBACK_UNITEX memOpenLowLevel(const char* FileName, TYPEOPEN_MF TypeOpen, BOOL fProjectedSize, afs_size_type dfProjectedSize,void*);
size_t ABSTRACT_CALLBACK_UNITEX memLowLevelWrite(ABSTRACTFILE_PTR llFile, void const *Buf, size_t size,void*);
size_t ABSTRACT_CALLBACK_UNITEX memLowLevelRead(ABSTRACTFILE_PTR llFile, void *Buf, size_t size,void*);
int ABSTRACT_CALLBACK_UNITEX memLowLevelSeek(ABSTRACTFILE_PTR llFile, afs_size_type Pos, int TypeSeek,void*);
void ABSTRACT_CALLBACK_UNITEX memLowLevelGetSize(ABSTRACTFILE_PTR llFile, afs_size_type* pPos,void*);
void ABSTRACT_CALLBACK_UNITEX memLowLevelTell(ABSTRACTFILE_PTR llFile, afs_size_type* pPos,void*);
int ABSTRACT_CALLBACK_UNITEX memLowLevelClose(ABSTRACTFILE_PTR llFile,void*);
int ABSTRACT_CALLBACK_UNITEX memLowLevelSetSizeReservation(ABSTRACTFILE_PTR llFile, afs_size_type size_reserv,void*);

const void* ABSTRACT_CALLBACK_UNITEX memLowLevel_getMapPointer(ABSTRACTFILE_PTR llFile, afs_size_type pos, afs_size_type len,void* );

ULB_VFFUNC BOOL ULIB_CALL RenameMemFile(const char * _OldFilename, const char * _NewFilename);
ULB_VFFUNC BOOL ULIB_CALL DeleteMemFile(const char* lpFileName);


ABSTRACTFILE_PTR GetLLF_StdOut(BOOL *pfTrashOutput);
ABSTRACTFILE_PTR GetLLF_StdErr(BOOL *pfTrashOutput);

#ifndef _VIRTUAL_FILE_SYSTEM_EXPORT
// high level operations




ULB_VFFUNC BOOL ULIB_CALL ZeroMemFile(const char* FileName);

/* pUserPtr is a private pointer that caller can associate to a file to store private information */
/* caller must allocate and free itself if he use it */
ULB_VFFUNC BOOL ULIB_CALL SetUserPtrFile(const char* FileName,const void*pUserPtr);
ULB_VFFUNC const void* ULIB_CALL GetUserPtrFile(const char* FileName);


ULB_VFFUNC BOOL ULIB_CALL BuildMemFileFromMemoryBuffer(const char* FileName,  const void* pBuf, afs_size_type size_File, BOOL fIsPermanentPointer);
ULB_VFFUNC BOOL ULIB_CALL BuildMemFileFromMemoryBufferUsrPtr(const char* FileName, const void* pBuf, 
                                                   afs_size_type size_File, BOOL fIsPermanentPointer,
                                                   const void*pUserPtr);

ULB_VFFUNC BOOL ULIB_CALL BuildMemFileFromMemoryBufferWithPrefix(const char* FileName, 
													   const void* pBufPrefix, afs_size_type size_FilePrefix,
													   const void* pBuf, afs_size_type size_File);
ULB_VFFUNC BOOL ULIB_CALL BuildMemFileFromMemoryBufferWithPrefixUsrPtr(const char* FileName, 
															 const void* pBufPrefix, afs_size_type size_FilePrefix, 
															 const void* pBuf, afs_size_type size_File, 
                                                             const void*pUserPtr);


ULB_VFFUNC BOOL ULIB_CALL BuildMemFileFromMemoryBufferUsrPtrWithPrefixOrPermanent(const char* FileName,
															 const void* pBufPrefix, afs_size_type size_FilePrefix,
															 const void* pBufSuffix, afs_size_type size_FileSuffix,
															 BOOL fIsPermanentPointer,
															 const void*pUserPtr);

ULB_VFFUNC BOOL ULIB_CALL GetMemoryBufferFromMemFileUsrPtr(const char* FileName, const void** ppBuf, afs_size_type* size_File,
                                      const void**ppUserPtr);
ULB_VFFUNC BOOL ULIB_CALL GetMemoryBufferFromMemFile(const char* FileName, const void** ppBuf, afs_size_type* size_File);


ULB_VFFUNC void ULIB_CALL ClearVirtualFiles();


ULB_VFFUNC BOOL ULIB_CALL SetStdOutFile(const char* FileName, BOOL fTrashOutput);

ULB_VFFUNC BOOL ULIB_CALL SetStdErrFile(const char* FileName, BOOL fTrashOutput);

typedef struct
{
    const char* FileName;
    const void* pBuf;
    const void* pUserPtr;
    afs_size_type size_File;
    BOOL fIsPermanentPointer;
    unsigned long dwReservedMagicValue;
} ENUM_VIRTUAL_FILE;

ULB_VFFUNC BOOL ULIB_CALL InitMemFileEnumeration(ENUM_VIRTUAL_FILE*);
ULB_VFFUNC BOOL ULIB_CALL InitMemFileEnumerationEx(ENUM_VIRTUAL_FILE* pevf, afs_size_type* p_nb_item );
ULB_VFFUNC BOOL ULIB_CALL GetNextMemFileEnumeration(ENUM_VIRTUAL_FILE*);
ULB_VFFUNC BOOL ULIB_CALL CloseMemFileEnumeration(ENUM_VIRTUAL_FILE*);
ULB_VFFUNC BOOL ULIB_CALL DeleteMemFileCurrentlyEnumerated(ENUM_VIRTUAL_FILE*);

ULB_VFFUNC BOOL ULIB_CALL DumpMemFileOnDisk(const char* DestDir);

ULB_VFFUNC BOOL ULIB_CALL DeleteMemFileForPrefix(const char* szPrefix);

#endif

#ifdef __cplusplus
}
#endif




#endif
