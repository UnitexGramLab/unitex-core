
#include "fr_univ_mlv_unitex_UnitexJNIExperiment.h"




#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "UnitexTool.h"

#include "AbstractFilePlugCallback.h"
#include "UserCancellingPlugCallback.h"


#include <time.h>


JNIEXPORT jint JNICALL Java_fr_univ_1mlv_unitex_UnitexJNIExperiment_PerformUnitexToolByStringArray
  (JNIEnv *jenv, jclass, jobjectArray strArray)
{
    jint retval = 0;
    char **mpszArgs =NULL;
	printf("hi jni\n\njni\n");
    int nbArgs = (int)jenv->GetArrayLength(strArray);
    if (nbArgs>0)
        mpszArgs = (char**)malloc(nbArgs * sizeof(char*));

    if (mpszArgs != NULL)
    {
        int i;
        for (i = 0; i < nbArgs; i++) {
            jstring jstr = (jstring)jenv->GetObjectArrayElement(strArray, i);
            const char* sz = jenv->GetStringUTFChars(jstr, NULL);

            // we can replace these line by mpszArgs[i] = strdup(sz) to not remove quote
            size_t string_alloc_size = strlen(sz)+4;
            mpszArgs[i] = (char*)malloc(string_alloc_size+4);
            strcpy(mpszArgs[i],sz);
            //CopyStrArg(sz,mpszArgs[i],string_alloc_size);

            jenv->ReleaseStringUTFChars(jstr, sz);
        }
    

		


        retval = UnitexTool_several_info(nbArgs, mpszArgs, NULL, NULL);

        for (i = 0; i < nbArgs; i++) {
            free(mpszArgs[i]);
        }
        free(mpszArgs);

    }

    return retval;
}
