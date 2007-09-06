
#include <stdlib.h>
#include <string.h>
#include "Array.h"

int array_init( Parray_t array ) {
#ifdef BDB

#define TMPPREFIX "/var/tmp/Colloc_"

	int ret;

    /* Initialize our handles */
	*array = NULL;
	DB_ENV *arrayE = NULL;
	DB_MPOOLFILE *arrayPF = NULL;
	
	int ret_t; 
	const char *db_name = "db";
	u_int32_t open_flags;

	/* Create the environment */
	ret = db_env_create(&arrayE, 0);
	if (ret != 0) {
		fprintf(stderr,"Error creating environment handle: %s\n", db_strerror(ret));
		exit(1);
	}
	
	open_flags =
		DB_CREATE     |  /* Create the environment if it does not exist */
		DB_INIT_MPOOL |  /* Initialize the memory pool (in-memory cache) */
		DB_PRIVATE;      /* Region files are not backed by the filesystem. 
		                  * Instead, they are backed by heap memory.  */

	/*
	 * Specify the size of the in-memory cache.
	 */
	ret = arrayE->set_cachesize(arrayE, 0, 200 * 1024 * 1024, 1); // according to bdb documentation, if 
	                                                                // this number is smaller than 500MB
	                                                                // a %25 overhead is added.
	if (ret != 0) {
		fprintf(stderr,"Error increasing the cache size: %s\n", db_strerror(ret));
		exit(1);
	}
	
	/*
	 * Now actually open the environment. Notice that the environment home
	 * directory is NULL. This is required for an in-memory only
	 * application.
	 */
	ret = arrayE->open(arrayE, NULL, open_flags, 0);
	if (ret != 0) {
		fprintf(stderr, "Error opening environment: %s\n", db_strerror(ret));
		exit(1);
	}
	
	/* Initialize the DB handle */
	ret = db_create(array, arrayE, 0);
	if (ret != 0) {
		arrayE->err(arrayE, ret, "Attempt to create db handle failed.");
		fprintf(stderr,"Bailing out...\n");
		exit(1);
	}
	
	/*
	 * Set the database open flags.
	 */
	char filename[32];
	sprintf(filename, "%s%d", TMPPREFIX, getpid() );
	open_flags = DB_CREATE | DB_TRUNCATE;
	ret = (*array)->open(*array, /* Pointer to the database */
				NULL,             /* Txn pointer */
				filename,         /* File name -- Must be NULL for inmemory! */
				db_name,          /* Logical db name */
				DB_BTREE,         /* Database type (using btree) */
				open_flags,       /* Open flags */
				0);               /* File mode. Using defaults */
	
	if (ret != 0) {
		arrayE->err(arrayE, ret, "Attempt to open db failed.");
		fprintf(stderr,"Bailing out...\n");
		exit(1);
	}

#else
	*array=NULL;
#endif

}

int array_get(Parray_t array, void *key, size_t keyL, void **data, size_t *dataL ) {
#ifdef BDB

	DBT arrayK, arrayD;
	int ret;

	memset (&arrayK, 0, sizeof(arrayK) );
	memset (&arrayD, 0, sizeof(arrayD) );

	arrayK.data  = key;
	arrayK.size  = keyL;

	ret = (*array)->get(*array, NULL, &arrayK, &arrayD, 0);
	if (ret) {
		*data=NULL;
		dataL=0;
        //fprintf(stderr, "%s\n", db_strerror(ret));
		return 1;
	}
	else {
		*data=arrayD.data;
		*dataL=arrayD.size;
		return 0;
	}
#else
	JUDYHSH(array);

	JHSG( arrayI, *array, key, keyL );

	*data=arrayI;
	if (arrayI) { 
		*dataL=sizeof(Word_t);
		return 0;
	}
	else {
		*dataL=0;
		return 1;
	}
#endif

}

int array_set(Parray_t array, void *key, size_t keyL, void *data, size_t dataL ) {

#ifdef BDB
	DBT arrayK, arrayD;
	int ret;

	arrayK.flags = 0;
	arrayK.data  = key;
	arrayK.size  = keyL;	

	arrayD.flags = 0;
	arrayD.data  = data;
	arrayD.size  = dataL;

	ret = (*array)->put(*array, NULL, &arrayK, &arrayD, 0);
	if (ret) {
        fprintf(stderr, "%s\n", db_strerror(ret));
		exit(1);
		return 1;
	}
	else {
		return 0;
	}
#else
	if (dataL > sizeof(Word_t)) {
		fprintf(stderr,"This array library only accepts word-size values, which is for this machine %d bytes.\n",sizeof(Pvoid_t));
		exit(1);
	}

	JUDYHSH(array);

	arrayI  = data;
	JHSI( arrayI, *array, key, keyL );

	if (arrayI) { 
		return 0;
	}
	else {
		return 1;
	}
#endif

}

int array_free( Parray_t array, int contains_pointers ) {
	if ( contains_pointers ) {

	}
#ifdef BDB

	int ret, ret_t;
	DB_ENV *arrayE=NULL;

	/* Close our database handle, if it was opened. */
	if (array && *array) {
		arrayE = (*array)->get_env(*array);
		ret_t = (*array)->close(*array, 0);
		if (ret_t) {
			fprintf(stderr, "%s database close failed.\n", db_strerror(ret_t));
			ret = ret_t;
		}
		char filename[32];
		sprintf(filename, "%s%d", TMPPREFIX, getpid() );
	
		ret_t = arrayE->dbremove(arrayE,NULL,filename, NULL, 0);
		if (ret_t) {
			fprintf(stderr,"%s database remove failed.\n", db_strerror(ret_t));
			ret = ret_t;
		}
	}

	/* Close our environment, if it was opened. */
	if (arrayE) {
		ret_t = arrayE->close(arrayE, 0);
		if (ret_t) {
			fprintf(stderr,"environment close failed: %s\n", db_strerror(ret_t));
			ret = ret_t;
		}
	}

#else
	JUDYHSH(array);

	JHSFA(arrayAL,*array);

#endif

	*array=NULL;

}

int array_del( Parray_t array, void *key, size_t keyL ) {
#ifdef BDB
	DBT arrayK, arrayD;
	int ret;

	arrayK.flags = 0;
	arrayK.data  = key;
	arrayK.size  = keyL;

	arrayD.flags = 0;

	ret = (*array)->del(*array, NULL, &arrayK, 0);
	if (ret) {
		return 1;
	}
	else {
		return 0;
	}

#else
	JUDYHSH(array);

	JHSD( arrayR, *array, key, keyL );

	if (arrayR) { 
		return 0;
	}
	else {
		return 1;
	}

#endif

}

