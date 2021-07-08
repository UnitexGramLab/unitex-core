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
#ifndef _VIRT_SPACE_MANAGER_DEFINED
#define _VIRT_SPACE_MANAGER_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#define DECLARE_LTHANDLE(name)    struct name##__ { int _name##_unused; }; \
                                  typedef const struct name##__ * name

#define DFSCALLBACK

DECLARE_LTHANDLE(STATICARRAYC);

typedef unsigned long dfuLong32;
typedef signed long dfsLong32;


typedef long (DFSCALLBACK *tCompareDataC) (const void* lpElem1, const void* lpElem2);
typedef BOOL(DFSCALLBACK *tDestructorDataC) (const void* lpElem);

  STATICARRAYC InitStaticArrayC(dfuLong32 dwSizeElem, dfuLong32 dwStepAllocNew);
  void DeleteStaticArrayC(STATICARRAYC sac);
  dfuLong32 GetNbElem(STATICARRAYC sac);
  BOOL InitStaticArray(STATICARRAYC sac,
                       dfuLong32 dwSizeElem,
                       dfuLong32 dwStepAlloc);
  BOOL ReservAllocation(STATICARRAYC sac,dfuLong32 dwNbReserv);
  BOOL DeleteElem(STATICARRAYC sac, dfuLong32 dwPos, dfuLong32 dwNbElem);
  BOOL InsertElem(STATICARRAYC sac, dfuLong32 dwPos, const void* lpData);
  BOOL SetElem(STATICARRAYC sac, dfuLong32 dwPos, const void* lpData);
  BOOL GetElem(STATICARRAYC sac, dfuLong32 dwPos, void* lpData) ;
  const void* GetElemPtr(STATICARRAYC sac, dfuLong32 dwPos) ;
  BOOL AddEndElem(STATICARRAYC sac, const void* lpData);
  BOOL SetFuncCompareData(STATICARRAYC sac, tCompareDataC fCompareDataSet);
  BOOL SetFuncDestructor(STATICARRAYC sac, tDestructorDataC fDestructorDataSet);
  BOOL InsertSorted(STATICARRAYC sac, const void* lpData);
  BOOL FindSameElemPos(STATICARRAYC sac, const void* lpDataElem, dfuLong32* lpdwPos) ;


#ifdef __cplusplus
}
#endif

#endif
