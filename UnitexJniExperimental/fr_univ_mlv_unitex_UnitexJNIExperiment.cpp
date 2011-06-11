/*
   build Jni on MacOs :
 make JNILIBRARY=yes ADDITIONAL_INCLUDE=/System/Library/Frameworks/JavaVM.framework/Versions/CurrentJDK/Headers/
  and after rename and move bin/libUnitexJNIExperiment.so to UnitexJniExperimental/libUnitexJNIExperiment.dylib)

 make JNILIBRARY=yes ADDITIONAL_INCLUDE=/usr/lib/jvm/java-6-openjdk/include ADDITIONAL_INCLUDE2=/usr/lib/jvm/java-6-openjdk/include/linux
  and after move bin/libUnitexJNIExperiment.so to UnitexJniExperimental/libUnitexJNIExperiment.so)
 
 mingw32-make.exe SYSTEM=windows JNILIBRARY=yes ADDITIONAL_INCLUDE=c:\Progra~2\Java\jdk1.6.0_13\include ADDITIONAL_INCLUDE2=c:\Progra~2\Java\jdk1.6.0_13\include\win32
  and after move bin\UnitexJNIExperiment.dll to UnitexJniExperimental\UnitexJNIExperiment.dll)
 */



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

/* now : compiling jni under MinGW32 don't add underscore, so I add manually */

#ifdef __cplusplus
extern "C" {
#endif



JNIEXPORT jint JNICALL _Java_fr_univ_1mlv_unitex_UnitexJNIExperiment_PerformUnitexToolByStringArray
  (JNIEnv *jenv, jclass jcl, jobjectArray strArray)
{
    return Java_fr_univ_1mlv_unitex_UnitexJNIExperiment_PerformUnitexToolByStringArray(jenv, jcl, strArray);
}

#ifdef __cplusplus
}
#endif
