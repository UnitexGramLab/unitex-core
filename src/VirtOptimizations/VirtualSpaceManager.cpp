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

#include "VirtFileType.h"
#include "VirtualSpaceManager.h"
/*
typedef void *dfvoidp;
typedef const void *dfvoidpc;
typedef unsigned char dfbyte;
typedef dfbyte *dfbytep;
typedef const dfbyte *dfbytepc;
*/

//typedef unsigned int dfuInt;
//typedef unsigned short dfuShort;
//typedef unsigned short dfuInt16;

#define DFSCALLBACK

typedef unsigned char dfbyte;

#ifndef BYTE
typedef dfbyte BYTE;
#endif


#ifndef CONST
#define CONST               const
#endif

#ifndef LPBYTE
typedef BYTE *LPBYTE, *PBYTE;
#endif

#ifndef LPCBYTE
typedef CONST BYTE *LPCBYTE, *PCBYTE;
#endif


#ifndef VOID
typedef void VOID;
#endif

#ifndef LPVOID
typedef VOID *LPVOID, *PVOID;
#endif

#ifndef LPCVOID
typedef CONST VOID *LPCVOID, *PCVOID;
#endif

#ifndef DWORD
typedef dfuLong32 DWORD;
#endif

#ifndef LPDWORD
typedef DWORD *LPDWORD, *PDWORD;
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif


#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
/*

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifdef wchar_t
typedef wchar_t dfwchar;
#else
typedef unsigned short dfwchar;
#endif

#ifndef Round4
#define Round4(x) ((((x)+3)/4)*4)
#endif

*/
typedef long (DFSCALLBACK *tCompareData) (LPCVOID lpElem1, LPCVOID lpElem2);
typedef BOOL(DFSCALLBACK *tDestructorData) (LPCVOID lpElem);
/**
#define DfsFree(a) (free((a)))
#define DfsMalloc(a) (malloc((a)))
#define DfsRealloc(a,b) (realloc((a),(b)))

typedef void* LOWLEVELFILE;
*/
class STATICARRAY
{
public:
  STATICARRAY(DWORD dwSizeElem = sizeof(LPVOID), DWORD dwStepAlloc = 256, DWORD dwFirstAlloc = 0);
  ~STATICARRAY();
  DWORD GetNbElem() const;
  BOOL InitStaticArray(DWORD dwSizeElem = sizeof(LPVOID),
                       DWORD dwStepAlloc = 256,
                       DWORD dwFirstAlloc = 0);
  BOOL ReservAllocation(DWORD dwNbReserv);
  BOOL DeleteElem(DWORD dwPos, DWORD dwNbElem = 1);
  BOOL InsertElem(DWORD dwPos, LPCVOID lpData);
  BOOL SetElem(DWORD dwPos, LPCVOID lpData);
  BOOL GetElem(DWORD dwPos, LPVOID lpData) const;
  LPCVOID GetElemPtr(DWORD dwPos) const;
  BOOL AddEndElem(LPCVOID lpData);
  BOOL SetFuncCompareData(tCompareData fCompareDataSet);
  BOOL SetFuncDestructor(tDestructorData fDestructorDataSet);
  BOOL InsertSorted(LPCVOID lpData);
  BOOL FindSameElemPos(LPCVOID lpDataElem, LPDWORD lpdwPos) const;
private:
  BOOL SetNbAlloc(DWORD dwNbAllocNew);
  BOOL CheckNbAlloc(DWORD dwNbAllocNeeded);

  DWORD dwNbUsed;
  DWORD dwNbAlloc;
  DWORD dwSizeElem;
  DWORD dwStepAlloc;
  LPBYTE lpData;
  tCompareData fCompareData;
  tDestructorData fDestructorData;
};


#define STEPALLOC (0x10)




BOOL STATICARRAY::InitStaticArray(DWORD dwSizeElemNew,DWORD dwStepAllocNew, DWORD dwFirstAlloc)
{
    BOOL fRes = FALSE;
    if (dwNbAlloc == 0)
    {
        dwSizeElem = dwSizeElemNew ;
        dwStepAlloc = dwStepAllocNew ;
        fRes = TRUE;
        if (dwFirstAlloc>0)
          fRes = ReservAllocation(dwFirstAlloc);
    }
    return fRes;
}

STATICARRAY::STATICARRAY(DWORD dwSizeElemNew,DWORD dwStepAllocNew,DWORD dwFirstAlloc) :
    dwNbUsed(0),
    dwNbAlloc(0),
    dwSizeElem(dwSizeElemNew),
    dwStepAlloc(dwStepAllocNew),
    lpData(NULL),
    fCompareData(NULL),
    fDestructorData(NULL)
{
  if (dwFirstAlloc>0)
    ReservAllocation(dwFirstAlloc);
}

BOOL STATICARRAY::ReservAllocation(DWORD dwNbReserv)
{
  BOOL fRet = TRUE;
  if (dwNbReserv > dwNbAlloc)
    fRet = CheckNbAlloc(dwNbReserv);
  return fRet;
}

BOOL STATICARRAY::SetFuncCompareData(tCompareData fCompareDataSet)
{
    BOOL fRet = TRUE;//(dwNbUsed<2);
    //if (fRet)
        fCompareData=fCompareDataSet;
    return fRet;
}

BOOL STATICARRAY::SetFuncDestructor(tDestructorData fDestructorDataSet)
{
    fDestructorData = fDestructorDataSet;
    return TRUE;
}


BOOL STATICARRAY::InsertSorted(LPCVOID lpDataElem)
{
    BOOL fRet;
    if ((fCompareData==NULL) || (dwNbUsed==0))
        fRet = AddEndElem(lpDataElem);
    else
    {
        DWORD dwStep = dwNbUsed/2;
        DWORD i=0;
        BOOL fContinueLoop=TRUE;

        do
        {
            DWORD dwTest=min(dwNbUsed-1,i+dwStep);

            if ((*fCompareData)(lpDataElem,GetElemPtr(dwTest))>=0)
                i=dwTest+1;
            if (i==dwNbUsed+1)
                fContinueLoop=FALSE;

            if (dwStep>0)
                dwStep = (dwStep)/2;
            else
                fContinueLoop=FALSE;
        } while (fContinueLoop);

        fRet=InsertElem(i,lpDataElem);
    }
    return fRet;
}

BOOL STATICARRAY::FindSameElemPos(LPCVOID lpDataElem,LPDWORD lpdwPos) const
{
    BOOL fRet=FALSE;
    if ((fCompareData!=NULL) && (dwNbUsed>0))
    {
        DWORD dwStep = dwNbUsed/2;
        DWORD i=0;
        BOOL fContinueLoop=TRUE;

        do
        {
            DWORD dwTest=min(dwNbUsed-1,i+dwStep);

            long iComp=(*fCompareData)(lpDataElem,GetElemPtr(dwTest));

            if (iComp==0)
            {
                i=dwTest;
                fContinueLoop=FALSE;
            }

            if (iComp>0)
                i=dwTest+1;

            if (i==dwNbUsed+1)
                fContinueLoop=FALSE;

            if (dwStep>0)
                dwStep = (dwStep)/2;
            else
                fContinueLoop=FALSE;
        } while (fContinueLoop);

        i=min(dwNbUsed-1,i);
        fRet=((*fCompareData)(lpDataElem,GetElemPtr(i)))==0;
        if (fRet && (lpdwPos!=NULL))
            *lpdwPos=i;
    }
    return fRet;
}

STATICARRAY::~STATICARRAY()
{
    DWORD i;
    if (fDestructorData!=NULL)
        for (i=0;i<dwNbUsed;i++)
            (*fDestructorData)(GetElemPtr(i));

    if (lpData != NULL)
        free(lpData);
}

DWORD STATICARRAY::GetNbElem() const
{
    return dwNbUsed;
}

BOOL STATICARRAY::DeleteElem(DWORD dwPos,DWORD dwNbElem)
{
    BOOL fRes = FALSE;
    if (dwPos+dwNbElem <= dwNbUsed)
    {
        DWORD i;
        if (fDestructorData!=NULL)
            for (i=dwPos;i<dwPos+dwNbElem;i++)
                (*fDestructorData)(GetElemPtr(i));
        memmove(lpData+((dwPos)*dwSizeElem),lpData+((dwPos+dwNbElem)*dwSizeElem),
                        (dwNbUsed-(dwPos+dwNbElem))*dwSizeElem);
        dwNbUsed -= dwNbElem;
        fRes = CheckNbAlloc(dwNbUsed);
    }
    return fRes;
}

BOOL STATICARRAY::SetElem(DWORD dwPos, LPCVOID lpDataElem)
{
    BOOL fRes = dwPos < dwNbUsed ;
    if (dwPos == dwNbUsed)
        if (CheckNbAlloc(dwNbUsed+1))
    {
        dwNbUsed++;
        fRes=TRUE;
    }

    if (fRes)
    {
        memcpy(lpData+(dwPos*dwSizeElem),lpDataElem,dwSizeElem);
    }
    return fRes;
}

BOOL STATICARRAY::InsertElem(DWORD dwPos, LPCVOID lpDataElem)
{
    BOOL fRes = FALSE;
    if (dwPos <= dwNbUsed)
        if (CheckNbAlloc(dwNbUsed+1))
    {
        memmove(lpData+((dwPos+1)*dwSizeElem),lpData+(dwPos*dwSizeElem),
                        (dwNbUsed-dwPos)*dwSizeElem);
        dwNbUsed ++ ;
        fRes = SetElem(dwPos, lpDataElem);
    }
    return fRes;
}

BOOL STATICARRAY::GetElem(DWORD dwPos, LPVOID lpDataElem) const
{
    BOOL fRes = dwPos < dwNbUsed ;
    if (fRes)
    {
        memcpy(lpDataElem,lpData+(dwPos*dwSizeElem),dwSizeElem);
    }
    return fRes;
}

LPCVOID STATICARRAY::GetElemPtr(DWORD dwPos) const
{
    LPCVOID lpRet=NULL;
    if (dwPos < dwNbUsed)
    {
        lpRet = lpData+(dwPos*dwSizeElem) ;
    }
    return lpRet;
}

BOOL STATICARRAY::AddEndElem(LPCVOID lpDataElem)
{
    BOOL fRes = FALSE;
    if (CheckNbAlloc(dwNbUsed+1))
    {
        dwNbUsed ++ ;
        fRes = SetElem(dwNbUsed-1, lpDataElem);
    }
    return fRes;
}

BOOL STATICARRAY::SetNbAlloc(DWORD dwNbAllocNew)
{
    BOOL fRes;
    DWORD dwNewSize = (dwNbAllocNew*dwSizeElem) + 0x10;
    if (lpData == NULL)
    {
        // + 0x10 : when dwNbAllocNew==0 we will not have NULL
        lpData = (LPBYTE)malloc(dwNewSize);
        fRes = (lpData != NULL);
    }
    else
    {
        LPBYTE lpNewData = (LPBYTE)realloc(lpData,dwNewSize);
        fRes = (lpNewData != NULL);
        if (fRes)
            lpData = lpNewData;
    }
    if (fRes)
        dwNbAlloc = dwNbAllocNew;
    return fRes;
}

BOOL STATICARRAY::CheckNbAlloc(DWORD dwNbAllocNeeded)
{
    DWORD dwNbExactAlloc;
    BOOL fRes = TRUE;
    dwNbExactAlloc = ((dwNbAllocNeeded + dwStepAlloc - 1)/dwStepAlloc)*dwStepAlloc;
    if (dwNbExactAlloc != dwNbAlloc)
        fRes = SetNbAlloc(dwNbExactAlloc);
    return fRes;
}


typedef long (DFSCALLBACK *tCompareDataC) (const void* lpElem1, const void* lpElem2);
typedef BOOL(DFSCALLBACK *tDestructorDataC) (const void* lpElem);



STATICARRAYC InitStaticArrayC(DWORD dwSizeElem, DWORD dwStepAllocNew)
{
  STATICARRAY* psa = new STATICARRAY(dwSizeElem, dwStepAllocNew);
  STATICARRAYC sac = (STATICARRAYC)psa;
  return sac;
}

void DeleteStaticArrayC(STATICARRAYC sac)
{
    STATICARRAY* psa = (STATICARRAY*)sac;
    delete(psa);
}

BOOL ReservAllocation(STATICARRAYC sac,DWORD dwNbReserv)
{
    STATICARRAY* psa = (STATICARRAY*)sac;
    return psa->ReservAllocation(dwNbReserv);
}

DWORD GetNbElem(STATICARRAYC sac)
{
    STATICARRAY* psa = (STATICARRAY*)sac;
    return psa->GetNbElem();
}

BOOL InitStaticArray(STATICARRAYC sac,
                    DWORD dwSizeElem,
                    DWORD dwStepAlloc)
{
    STATICARRAY* psa = (STATICARRAY*)sac;
    return psa->InitStaticArray(dwSizeElem,dwStepAlloc);
}

BOOL DeleteElem(STATICARRAYC sac, DWORD dwPos, DWORD dwNbElem)
{
    STATICARRAY* psa = (STATICARRAY*)sac;
    return psa->DeleteElem(dwPos,dwNbElem);
}

BOOL InsertElem(STATICARRAYC sac, DWORD dwPos, LPCVOID lpData)
{
    STATICARRAY* psa = (STATICARRAY*)sac;
    return psa->InsertElem(dwPos,lpData);
}

BOOL SetElem(STATICARRAYC sac, DWORD dwPos, LPCVOID lpData)
{
    STATICARRAY* psa = (STATICARRAY*)sac;
    return psa->SetElem(dwPos,lpData);
}

BOOL GetElem(STATICARRAYC sac, DWORD dwPos, LPVOID lpData)
{
    STATICARRAY* psa = (STATICARRAY*)sac;
    return psa->GetElem(dwPos,lpData);
}

LPCVOID GetElemPtr(STATICARRAYC sac, DWORD dwPos)
{
    STATICARRAY* psa = (STATICARRAY*)sac;
    return psa->GetElemPtr(dwPos);
}

BOOL AddEndElem(STATICARRAYC sac, LPCVOID lpData)
{
    STATICARRAY* psa = (STATICARRAY*)sac;
    return psa->AddEndElem(lpData);
}

BOOL SetFuncCompareData(STATICARRAYC sac, tCompareDataC fCompareDataSet)
{
    STATICARRAY* psa = (STATICARRAY*)sac;
    return psa->SetFuncCompareData((tCompareData)fCompareDataSet);
}

BOOL SetFuncDestructor(STATICARRAYC sac, tDestructorDataC fDestructorDataSet)
{
    STATICARRAY* psa = (STATICARRAY*)sac;
    return psa->SetFuncDestructor((tDestructorData)fDestructorDataSet);
}

BOOL InsertSorted(STATICARRAYC sac, LPCVOID lpData)
{
    STATICARRAY* psa = (STATICARRAY*)sac;
    return psa->InsertSorted(lpData);
}

BOOL FindSameElemPos(STATICARRAYC sac, LPCVOID lpDataElem, LPDWORD lpdwPos)
{
    STATICARRAY* psa = (STATICARRAY*)sac;
    return psa->FindSameElemPos(lpDataElem,lpdwPos);
}
