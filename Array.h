
#ifndef ARRAY_H
#define ARRAY_H


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
	DBC *_arrayC; /*database cursor */ \
	DBT _arrayKey, _arrayData;          \
	int _ret;                          \
	_arrayKey.flags=0;                 \
	_arrayData.flags=0;                \
	int _tmp=1; \
	(A)->cursor( (A), NULL, &_arrayC, 0); \
	while ( \
		((_ret = _arrayC->get(_arrayC, &_arrayKey, &_arrayData, DB_NEXT)) == 0 ) && \
		( \
			( ((K).data=_arrayKey.data ) || 1 ) ||   \
			( ((K).size=_arrayKey.size ) || 1 ) ||   \
			( ((V).data=_arrayData.data) || 1 ) ||   \
			( ((V).size=_arrayData.size) || 1 ) || 1 \
		) \
	)

#define end_foreach(A,K,V) \
	if (ret != DB_NOTFOUND) { \
		fprintf(stderr,"There was a problem with the bdb in %s() %s:%d. Bailing out.\n", __FUNCTION__, __FILE__, __LINE__); \
		exit(1); \
	} \
	if (arrayC) arrayC->close(arrayC); \
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
			( ((K).data=_arrayK        ) || 1 ) && \
			( ((K).size=_arrayKL       ) || 1 ) && \
			( ((V).data=_arrayI        ) || 1 ) && \
			( ((V).size=sizeof(Pvoid_t)) || 1 )    ) || _tmp \
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
int array_get( Parray_t array, void *key, size_t keyL, void **data, size_t *dataL );
int array_set( Parray_t array, void *key, size_t keyL, void **data, size_t *dataL );
int array_del( Parray_t array, void *key, size_t keyL );
int array_free( Parray_t array, int contains_pointers );

#endif

