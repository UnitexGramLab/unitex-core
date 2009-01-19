
 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * Author: Burak Arslan (arslan@univ-mlv.fr, plq@gsulinux.org)
 *         This File is the associative array (BDB-Judy abstraction layer) api header file.
 */

#ifndef ARRAY_H
#define ARRAY_H

/*
 * Is tested with BDB 4.6.19. may not work with older versions.
 */

#ifdef BDB
#include <db.h>
typedef DB** Parray_t;
typedef DB*  array_t;
#else
#include <Judy.h>
typedef PPvoid_t Parray_t;
typedef Pvoid_t  array_t;
#endif

typedef struct {
	void *data;
	size_t size;
} arrayI_t;


#ifdef BDB
#define foreach(A,K,V) \
{                                     \
	DBC *_arrayC;                     \
	DBT _arrayKey, _arrayData;        \
	int _ret;                         \
	_arrayKey.flags=0;                \
	_arrayData.flags=0;               \
	int _tmp=1;                       \
	(A)->cursor( (A), NULL, &_arrayC, 0); \
	while ( \
		((_ret = _arrayC->get(_arrayC, &_arrayKey, &_arrayData, DB_NEXT)) == 0 ) && \
		( ( \
			( ((K).data=_arrayKey.data ) || _tmp ) &&   \
			( ((K).size=_arrayKey.size ) || _tmp ) &&   \
			( ((V).data=_arrayData.data) || _tmp ) &&   \
			( ((V).size=_arrayData.size) || _tmp )    ) || _tmp \
		) \
	)

#define end_foreach(A,K,V) \
	if (_ret != DB_NOTFOUND) { \
		fprintf(stderr,"There was a problem with the bdb in %s() %s:%d. Bailing out.\n", __FUNCTION__, __FILE__, __LINE__); \
		exit(1); \
	} \
	if (_arrayC) _arrayC->close(_arrayC); \
}

#else
#define foreach(A,K,V) \
{ \
	JUDYHSH(_array);  \
	int _tmp=1; \
	JHSIF(_arrayI, (A), _arrayS, _arrayK, _arrayKL); \
	(K).data=_arrayK;         \
	(K).size=_arrayKL;        \
	(V).data=_arrayI;         \
	(V).size=sizeof(Pvoid_t); \
	do 

#define end_foreach(A,K,V) \
	while ( \
		((((_arrayI) = (Pvoid_t) JudyHSIterNext((A), &(_arrayS), &(_arrayK), &(_arrayKL), &_arrayErr)) != PJERR) && _arrayI ) && \
		( (\
			( ((K).data=_arrayK        ) || _tmp ) && \
			( ((K).size=_arrayKL       ) || _tmp ) && \
			( ((V).data=_arrayI        ) || _tmp ) && \
			( ((V).size=sizeof(Pvoid_t)) || _tmp )    ) || _tmp \
		) \
	); \
	if (_arrayI == PJERR) { \
		fprintf(stderr,"File '%s', line %d: %s(), " "JU_ERRNO_* == %d, ID == %d\n", "Collocation.cpp", 502, "JudyHSIterNext", ((&_arrayErr)->je_Errno), ((&_arrayErr)->je_ErrID));  \
		exit(1); \
	} \
	JHSFI(_arraySL, _arrayS); \
}

#endif

#if 0

/* the anatomy of a judyhs array: (note that this version of judy is not official.
 * many undocumented features of this modified version were used in unitex)
 */

PPvoid_t array   = NULL  // when a judy array is contained in another judy array,
                         // it's declared as PPvoid_t (which is just a void**) and used
                         // always like *array. otherwise, Pvoid_t is used. Note that a judy array
                         // should always be initialized to NULL, otherwise the first operation on 
                         // it will corrupt memory.
Pvoid_t  arrayI  = NULL; // iterator (ie pointer to an element, as in c++ map's iterator->second)
Pvoid_t  arrayK  = NULL; // key      (   pointer to an element's key, as in c++ map's iterator->first)
Pvoid_t  arrayS  = NULL; // state of the current iteration. should be NULLified before a new
                         // iteration is attempted, and should be destructed afterwards. 
Word_t   arraySL = 0;    // state length. is filled when the state is destroyed, so not of much use.
Word_t   arrayKL = 0;    // keylength
Word_t   arrayAL = 0;    // arraylength. is filled when the array is destroyed, so not of much use.
Word_t   arrayR  = 0;    // general-purpose return value. used generally for deletions.

#endif

#define _JUDYP(N,P) N ## P

#define JUDYLH( N )               \
	Pvoid_t  _JUDYP(N,I ) = NULL; \
	Word_t   _JUDYP(N,K ) = 0;    \
	Word_t   _JUDYP(N,AL) = 0;

#define JUDYHSH( N )              \
	JError_t _JUDYP(N,Err);       \
	Pvoid_t  _JUDYP(N,I ) = NULL; \
	Pvoid_t  _JUDYP(N,K ) = NULL; \
	Pvoid_t  _JUDYP(N,S ) = NULL; \
	Word_t   _JUDYP(N,SL) = 0;    \
	Word_t   _JUDYP(N,KL) = 0;    \
	Word_t   _JUDYP(N,AL) = 0;    \
	Word_t   _JUDYP(N,R)  = 0;

int array_init( Parray_t array );
int array_get ( Parray_t array, void *key, size_t keyL, void **data, size_t *dataL );
int array_set ( Parray_t array, void *key, size_t keyL, void  *data, size_t  dataL );
int array_del ( Parray_t array, void *key, size_t keyL );
int array_free( Parray_t array, int contains_pointers );

#endif

