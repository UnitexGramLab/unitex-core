
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <db.h>


int main(int argc, char **argv) {
    /* Initialize our handles */
    DB *dbp = NULL;
    DB_ENV *envp = NULL;
    DB_MPOOLFILE *mpf = NULL;

    int ret, ret_t; 
    const char *db_name = "in_mem_db1";
    u_int32_t open_flags;

    /* Create the environment */
    ret = db_env_create(&envp, 0);
    if (ret != 0) {
        fprintf(stderr, "Error creating environment handle: %s\n", db_strerror(ret));
        goto err;
    }

    open_flags =
      DB_CREATE     |  /* Create the environment if it does not exist */
      DB_INIT_MPOOL |  /* Initialize the memory pool (in-memory cache) */
      DB_PRIVATE;      /* Region files are not backed by the filesystem. 
                        * Instead, they are backed by heap memory.  */

    /* 
     * Specify the size of the in-memory cache. 
     */
    ret = envp->set_cachesize(envp, 0, 1 * 1024 * 1024, 1);
    if (ret != 0) {
        fprintf(stderr, "Error increasing the cache size: %s\n",
            db_strerror(ret));
        goto err;
    }

    /* 
     * Now actually open the environment. Notice that the environment home
     * directory is NULL. This is required for an in-memory only
     * application. 
     */
    ret = envp->open(envp, "/var/tmp", open_flags, 0);
    if (ret != 0) {
        fprintf(stderr, "Error opening environment: %s\n",
            db_strerror(ret));
        goto err;
    }


   /* Initialize the DB handle */
    ret = db_create(&dbp, envp, 0);
    if (ret != 0) {
         envp->err(envp, ret,
                "Attempt to create db handle failed.");
        goto err;
    }


    /* 
     * Set the database open flags. Autocommit is used because we are 
     * transactional. 
     */
    open_flags = DB_CREATE | DB_TRUNCATE;// | DB_AUTO_COMMIT;
    ret = dbp->open(dbp,         /* Pointer to the database */
             NULL,               /* Txn pointer */
             "/var/tmp/oha",     /* File name -- Must be NULL for inmemory! */
             db_name,            /* Logical db name */
             DB_HASH,            /* Database type (using btree) */
             open_flags,         /* Open flags */
             0);                 /* File mode. Using defaults */

    if (ret != 0) {
         envp->err(envp, ret, "Attempt to open db failed");
        goto err;
    }

	printf("the database was opened successfully\nSizeof DB: %d\n", sizeof(DB));

	char *description = "Grocery bill.";
	char *mescription = "Mrocery bill.";
	DBT key, data;
	unsigned money;
	money = 0;

	/* Zero out the DBTs before using them. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	key.data = &money;
	key.size = sizeof(float);

	data.data = description;
	data.size = strlen(description) +1;
	ret = dbp->put(dbp, NULL, &key, &data, 0);

	data.data = mescription;
	data.size = strlen(description) +1;
	ret = dbp->put(dbp, NULL, &key, &data, 0);

	if (ret == DB_KEYEXIST) {
		dbp->err(dbp, ret, "Put failed because key %f already exists\n", money);
	}

	/* Zero out the DBTs before using them. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	key.data = &money;
	key.size = sizeof(float);

	ret = dbp->get(dbp, NULL, &key, &data, 0);
	if (ret) {
         envp->err(envp, ret, "Attempt to get from db failed");
	}
	printf("%8.2f - %s\n",*((float*)key.data),(char *)data.data);

	dbp->del(dbp, NULL, &key, 0);

	ret = dbp->get(dbp, NULL, &key, &data, 0);
	if (ret) {
         envp->err(envp, ret, "Attempt to get from db failed");
	}
	printf("%8.2f - %s\n",*((float*)key.data),(char *)data.data);
	money=0;
	while(++money) {
	 	ret = dbp->put(dbp, NULL, &key, &data, 0);
		printf("%4d\n",money);
	}



err:
    /* Close our database handle, if it was opened. */
    if (dbp != NULL) {
        ret_t = dbp->close(dbp, 0);
        if (ret_t != 0) {
            fprintf(stderr, "%s database close failed.\n",
                db_strerror(ret_t));
            ret = ret_t;
        }
    }

    /* Close our environment, if it was opened. */
    if (envp != NULL) {
        ret_t = envp->close(envp, 0);
        if (ret_t != 0) {
            fprintf(stderr, "environment close failed: %s\n",
                db_strerror(ret_t));
                ret = ret_t;
        }
    }

    /* Final status message and return. */
    printf("I'm all done.\n");
    return (ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
} 


