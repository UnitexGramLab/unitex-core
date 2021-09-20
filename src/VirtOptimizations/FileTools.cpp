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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "AbstractFilePlugCallback.h"
#include "Af_stdio.h"


#include "FuncDeclModifier.h"
#include "VirtFileType.h"
#include "VirtualSpaceManager.h"
#include "VirtFileSystem.h"
#include "MiniMutex.h"




#include "FileTools.h"


#define BUFFER_IO_SIZE (0x4000)
ULB_VFFUNC int ULIB_CALL DuplicateFile(const char* srcFile, const char* dstFile)
{
    ABSTRACTFILE* vfRead;
    ABSTRACTFILE* vfWrite;
    long size_to_do;
    char szBuffer[BUFFER_IO_SIZE];
    int iSuccessCopyingRet=0;

    vfRead = af_fopen(srcFile,"rb");
    if (vfRead == NULL)
        return 1;
    vfWrite = af_fopen(dstFile,"wb");
    if (vfWrite == NULL)
    {
        af_fclose(vfRead);
        return 1;
    }


    if (af_fseek(vfRead, 0, SEEK_END) != 0)
        iSuccessCopyingRet = 1;
    size_to_do = af_ftell(vfRead);

    af_setsizereservation(vfWrite, size_to_do);

    if (af_fseek(vfRead, 0, SEEK_SET) != 0)
        iSuccessCopyingRet = 1;

    while (size_to_do>0)
    {
        int iThis = (size_to_do<BUFFER_IO_SIZE) ? (((int)size_to_do)) : BUFFER_IO_SIZE;
        int iReadDone = (int)af_fread(szBuffer,1,iThis,vfRead);
        int iWriteDone ;
        if (iReadDone == 0)
            break;
        iWriteDone = (int)af_fwrite(szBuffer,1,iReadDone,vfWrite);
        if (iWriteDone != iReadDone)
            break;
        size_to_do -= iWriteDone;
    }
    af_fclose(vfRead);
    af_fclose(vfWrite);
    return ((size_to_do==0) ? iSuccessCopyingRet : 1);
}
