/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * additional information: http://www.ergonotics.com/unitex-contribution/
 * https://github.com/ergonotics/JNI-for-Unitex-2.1
 * contact : unitex-contribution@ergonotics.com
 *
 */


#ifndef UnitexLibIOH
#define UnitexLibIOH



#include "AbstractCallbackFuncModifier.h"
#include "Af_stdio.h"


#ifdef __cplusplus
extern "C" {
#endif

struct _UNITEXFILEMAPPED {
        void* dummy;
};
typedef struct _UNITEXFILEMAPPED UNITEXFILEMAPPED;

UNITEX_FUNC void UNITEX_CALL GetUnitexFileReadBuffer(const char*name,UNITEXFILEMAPPED** amf, const void**buffer,size_t *size_file);
UNITEX_FUNC void UNITEX_CALL CloseUnitexFileReadBuffer(UNITEXFILEMAPPED *,const void*buffer,size_t size_file);

UNITEX_FUNC int UNITEX_CALL WriteUnitexFile(const char*name,const void*buffer_prefix,size_t size_prefix,const void*buffer_suffix,size_t size_suffix);

UNITEX_FUNC int UNITEX_CALL RemoveUnitexFile(const char*name);

UNITEX_FUNC int UNITEX_CALL RenameUnitexFile(const char*oldName,const char*newName);

UNITEX_FUNC int UNITEX_CALL CopyUnitexFile(const char*srcName,const char*dstName);
    
UNITEX_FUNC int UNITEX_CALL CreateUnitexFolder(const char*name);
    
UNITEX_FUNC int UNITEX_CALL RemoveUnitexFolder(const char*name);

UNITEX_FUNC int UNITEX_CALL UnitexAbstractPathExists(const char* path);


#ifdef HAS_LIST_FILE
#define UNITEX_IO_HAS_LIST_FILE 1	

UNITEX_FUNC char** UNITEX_CALL GetUnitexFileList(const char* path);

UNITEX_FUNC void UNITEX_CALL ReleaseUnitexFileList(const char* path,char**list);
#endif
    
#ifdef __cplusplus
}
#endif


#endif
