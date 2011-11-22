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

#include "fr_umlv_unitex_jni_UnitexJni.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "UnitexTool.h"
#include "SyncLogger.h"

#include "AbstractFilePlugCallback.h"
#include "UserCancellingPlugCallback.h"
#include "UniLogger.h"

#include "UnitexLibIO.h"
#ifdef UNITEX_HAVING_MINI_PERSISTANCE
#include "CompressedDic.h"
#include "Fst2.h"
#include "Alphabet.h"
#endif


#if ((defined(WIN32) || defined(_WIN32) || defined (_WIN64) || defined (_M_IX86)  || \
     defined(__i386) || defined(__i386__) || defined(__x86_64) || defined(__x86_64__) || \
     defined(TARGET_CPU_X86) || defined(TARGET_CPU_X86_64) || defined(__LITTLE_ENDIAN__) \
           ) && (!(defined(INTEL_X86_LIKE_LITTLE_ENDIAN))))
#define INTEL_X86_LIKE_LITTLE_ENDIAN 1
#endif

#ifdef INTEL_X86_LIKE_LITTLE_ENDIAN
static bool is_little_endian()
{
// printf("INTEL_X86_LIKE_LITTLE_ENDIAN is,");
	return true;
}
#else
static bool is_little_endian()
{
const jchar i=1;
const char *c=(const char*)&i;
bool little_endian = ((*c) != 0);
// printf("is_little_endian = %s\n",little_endian ? "y":"n");
return little_endian;
}
#endif


class jstringToCUtf
{
public:
	jstringToCUtf();
	~jstringToCUtf();
	const char* initJString(JNIEnv *env,jstring jstr);
	const char* getJString() ;
	void clear();
private:
	JNIEnv*env;
	jstring jstr;
	const char*c_str;
};

jstringToCUtf::jstringToCUtf() :
	env(NULL),jstr(NULL),c_str(NULL)
{
}

const char* jstringToCUtf::initJString(JNIEnv *envSet,jstring jstrSet)
{
	clear();
	env=envSet;
	jstr=jstrSet;
	c_str = env->GetStringUTFChars(jstr, 0);
	return c_str;
}

const char* jstringToCUtf::getJString()
{
	return c_str;
}

void jstringToCUtf::clear()
{
	if (c_str != NULL)
		env->ReleaseStringUTFChars(jstr, c_str);
	c_str = NULL;
}

jstringToCUtf::~jstringToCUtf()
{
	clear();
}

static int countArgs(char** args)
{
	int count=0;
	
	char**tmp=args;
	while ((*tmp) != NULL)
	{
		count++;
		tmp++;
	}
	return count;
}

static void freeArgs(char**args)
{
	char** tmp=args;
	while ((*tmp) != NULL)
	{
		free(*tmp);
		tmp++;
	}
	free(args);
}


static const char* CopyStrArg(const char*lpSrc,char* lpDest,size_t dwMaxSize)
{
int isInQuote=0;
size_t dwDestPos = 0;

  *lpDest = '\0';

  if (lpSrc==NULL)
      return NULL;

  while ((*lpSrc) == ' ')
      lpSrc++;

  while (((*lpSrc) != '\0') && ((dwMaxSize==0) || (dwDestPos<dwMaxSize)))
    {
      if ((*lpSrc) == '"')
        {
          isInQuote = !isInQuote;
          lpSrc++;
		  continue;
        }

      if (((*lpSrc) == ' ') && (!isInQuote))
      {
          while ((*lpSrc) == ' ')
              lpSrc++;
          return lpSrc;
      }

      *lpDest = *lpSrc;
      lpDest++;
      dwDestPos++;
      *lpDest = '\0';
      lpSrc++;
    }
  return lpSrc;
}


static char**argsFromStrArray(JNIEnv* jenv, jobjectArray strArray)
{
int nSize = jenv->GetArrayLength(strArray);
char **args=NULL;	
    if (nSize>0)
        args = (char**)malloc((nSize+1) * sizeof(char*));

    if (args != NULL)
    {
        int nbArgs = nSize;
        int i;
	    for (i = 0; i < nbArgs; i++) {
		    jstring jstr = (jstring)jenv->GetObjectArrayElement(strArray, i);
		    const char* sz = jenv->GetStringUTFChars(jstr, NULL);

			// we can replace these line by mpszArgs[i] = strdup(sz) to not remove quote
			size_t string_alloc_size = strlen(sz)+4;
			args[i] = (char*)malloc(string_alloc_size+4);
		    CopyStrArg(sz,args[i],string_alloc_size);

			jenv->ReleaseStringUTFChars(jstr, sz);
			/* explicitly releasing to assist garbage collection, though not required */
			jenv->DeleteLocalRef(jstr);
	    }
		args[i] = NULL;
    }
	return args;
}


static char**argsFromJString(JNIEnv* jenv, jstring jstr)
{
	jstringToCUtf jtcu;
	
	const char*cmdLine=jtcu.initJString(jenv,jstr);
    size_t len_cmd_line=strlen(cmdLine);

    char* work_buf = (char*)malloc(len_cmd_line+0x10);
    if (work_buf==NULL)
        return 0;

    const char*lpParcLine;
    int nb_args_found=0;

    lpParcLine = cmdLine;
    while ((*lpParcLine) != '\0')
    {
        *work_buf=0;
        lpParcLine = CopyStrArg(lpParcLine, work_buf, len_cmd_line+8);
        nb_args_found ++;
    }

	char **args=NULL;
    if (nb_args_found>0)
        args = (char**)malloc((nb_args_found+1) * sizeof(char*));
    else
        args = NULL;

	if (args == NULL)
    {
        free(work_buf);
        return NULL;
    }

    lpParcLine = cmdLine;
    int i=0;
    while ((*lpParcLine) != '\0')
    {
        *work_buf=0;
        lpParcLine = CopyStrArg(lpParcLine, work_buf, len_cmd_line+8);
        args[i] = strdup(work_buf);
		i++;
    }
	args[i] = NULL;

    free(work_buf);
    return args;
}

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    execUnitexTool
 * Signature: ([Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_fr_umlv_unitex_jni_UnitexJni_execUnitexTool___3Ljava_lang_String_2
  (JNIEnv *jenv, jclass, jobjectArray strArray)
{
	char**args=argsFromStrArray(jenv, strArray);
	jint retValue = (jint)main_UnitexTool_C(countArgs(args),args);
	freeArgs(args);
	return retValue;
}

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    execUnitexTool
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_fr_umlv_unitex_jni_UnitexJni_execUnitexTool__Ljava_lang_String_2
  (JNIEnv *jenv, jclass, jstring jstr)
{
	char**args=argsFromJString(jenv, jstr);
	jint retValue = (jint)main_UnitexTool_C(countArgs(args),args);
	freeArgs(args);
	return retValue;
}

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    usingOffsetFile
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_usingOffsetFile
  (JNIEnv *, jclass)
{
#ifdef UNITEX_HAVING_OFFSET_FILE
    return JNI_TRUE;
#endif
#ifdef UNITEX_REVISION
    return (UNITEX_REVISION >= 2292) ? JNI_TRUE : JNI_FALSE;
#endif
    return JNI_FALSE;
}


/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    numberAbstractFileSpaceInstalled
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_fr_umlv_unitex_jni_UnitexJni_numberAbstractFileSpaceInstalled
  (JNIEnv *, jclass)
{
	return (jint)GetNbAbstractFileSpaceInstalled();
}


/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    writeUnitexFile
 * Signature: (Ljava/lang/String;[C)Z
 */
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_writeUnitexFile__Ljava_lang_String_2_3C
  (JNIEnv *env, jclass, jstring filename, jcharArray filedata)
{
	jstringToCUtf jstc_filename;
	jstc_filename.initJString(env,filename);
	jsize len = env->GetArrayLength(filedata);
	jchar * buffer=env->GetCharArrayElements(filedata,NULL);
	
	jboolean retValue= (WriteUnitexFile(jstc_filename.getJString(),buffer,len,NULL,0) == 0);
	env->ReleaseCharArrayElements(filedata,buffer,0);
	return retValue;
}

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    writeUnitexFile
 * Signature: (Ljava/lang/String;[B)Z
 */
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_writeUnitexFile__Ljava_lang_String_2_3B
  (JNIEnv *env, jclass, jstring filename, jbyteArray filedata)
{
	jstringToCUtf jstc_filename;
	jstc_filename.initJString(env,filename);
	jsize len = env->GetArrayLength(filedata);
	jbyte * buffer=env->GetByteArrayElements(filedata,NULL);
	
	jboolean retValue= (WriteUnitexFile(jstc_filename.getJString(),buffer,len,NULL,0) == 0);
	env->ReleaseByteArrayElements(filedata,buffer,0);
	return retValue;
}


/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    writeUnitexFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_writeUnitexFile__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *env, jclass, jstring filename, jstring filecontent)
{
	jstringToCUtf jstc_filename;
	jstc_filename.initJString(env,filename);

	jsize stringSize = env->GetStringLength(filecontent);
	void* buffString = (void*)malloc(sizeof(jchar)*(stringSize+1));
	unsigned char*buffBom = (unsigned char*)buffString;
	*(buffBom++)=0xff;
	*(buffBom++)=0xfe;
	jchar* bufContent = (jchar*)buffBom;
	env->GetStringRegion(filecontent,0,stringSize,bufContent);

	if (!(is_little_endian()))
	{
		for (jsize j=0;j<stringSize;j++)
		{
			jchar c=*(bufContent+j);
			*(((unsigned char*)(bufContent+j))+0) = (unsigned char)(c & 0xff);
			*(((unsigned char*)(bufContent+j))+1) = (unsigned char)(c >> 8);
		}
	}
	jboolean ret= (WriteUnitexFile(jstc_filename.getJString(),buffString,sizeof(jchar)*(stringSize+1),NULL,0) == 0);
	free(buffString);
	return ret;
}


/* utf16be version
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_writeUnitexFile__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *env, jclass, jstring filename, jstring filecontent)
{
	jstringToCUtf jstc_filename;
	jstc_filename.initJString(env,filename);

	jsize stringSize = env->GetStringLength(filecontent);
	void* buffString = (void*)malloc(sizeof(jchar)*(stringSize+1));
	unsigned char*buffBom = (unsigned char*)buffString;
	*(buffBom++)=0xff-1;
	*(buffBom++)=0xfe+1;
	jchar* bufContent = (jchar*)buffBom;
	env->GetStringRegion(filecontent,0,stringSize,bufContent);

	if ((is_little_endian()))
	{
		for (jsize j=0;j<stringSize;j++)
		{
			jchar c=*(bufContent+j);
			*(((unsigned char*)(bufContent+j))+1) = (unsigned char)(c & 0xff);
			*(((unsigned char*)(bufContent+j))+0) = (unsigned char)(c >> 8);
		}
	}
	jboolean ret= (WriteUnitexFile(jstc_filename.getJString(),buffString,sizeof(jchar)*(stringSize+1),NULL,0) == 0);
	free(buffString);
	return ret;
}
*/

static jboolean doWriteUnitexFileUtf(JNIEnv* env,jstring filename,jstring filecontent,jboolean hasBom)
{
	jstringToCUtf jstc_filename;
	jstc_filename.initJString(env,filename);

	
	jstringToCUtf jstc_content;
	jstc_content.initJString(env,filecontent);

	const unsigned char UTF8BOM[3] = { 0xef,0xbb,0xbf };

	jboolean ret= (WriteUnitexFile(jstc_filename.getJString(),UTF8BOM,hasBom ? 3:0,
									jstc_content.getJString(),strlen(jstc_content.getJString())) == 0);
	return ret;
}

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    writeUnitexFileUtf
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_writeUnitexFileUtf__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *env, jclass, jstring filename, jstring filecontent)
{
	return doWriteUnitexFileUtf(env,filename,filecontent,JNI_FALSE);
}

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    writeUnitexFileUtf
 * Signature: (Ljava/lang/String;Ljava/lang/String;Z)Z
 */
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_writeUnitexFileUtf__Ljava_lang_String_2Ljava_lang_String_2Z
 (JNIEnv *env, jclass, jstring filename, jstring filecontent, jboolean has_bom)
{
	return doWriteUnitexFileUtf(env,filename,filecontent,has_bom);
}

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    getUnitexFileDataChar
 * Signature: (Ljava/lang/String;)[C
 */
JNIEXPORT jcharArray JNICALL Java_fr_umlv_unitex_jni_UnitexJni_getUnitexFileDataChar
  (JNIEnv *env, jclass, jstring filename)
{
	jstringToCUtf jstc_filename;
	jstc_filename.initJString(env,filename);

    UNITEXFILEMAPPED *amf;
    const void*buffer;
    size_t size_file;
	GetUnitexFileReadBuffer(jstc_filename.getJString(),&amf, &buffer,&size_file);
	jcharArray jarrRet=env->NewCharArray((jsize)size_file);
	if (jarrRet != NULL)
		env->SetCharArrayRegion(jarrRet,0,(jsize)size_file,(const jchar*)buffer);
    CloseUnitexFileReadBuffer(amf, buffer, size_file);
    return jarrRet;
}

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    getUnitexFileData
 * Signature: (Ljava/lang/String;)[B
 */
JNIEXPORT jbyteArray JNICALL Java_fr_umlv_unitex_jni_UnitexJni_getUnitexFileData
  (JNIEnv *env, jclass, jstring filename)
{
	jstringToCUtf jstc_filename;
	jstc_filename.initJString(env,filename);

    UNITEXFILEMAPPED *amf;
    const void*buffer;
    size_t size_file;
	GetUnitexFileReadBuffer(jstc_filename.getJString(),&amf, &buffer,&size_file);
	jbyteArray jarrRet=env->NewByteArray((jsize)size_file);
	if (jarrRet != NULL)
		env->SetByteArrayRegion(jarrRet,0,(jsize)size_file,(const jbyte*)buffer);
    CloseUnitexFileReadBuffer(amf, buffer, size_file);
    return jarrRet;
}

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    getUnitexFileString
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_fr_umlv_unitex_jni_UnitexJni_getUnitexFileString
  (JNIEnv *env, jclass, jstring filename)
{
	jstringToCUtf jstc_filename;
	jstc_filename.initJString(env,filename);
	jstring jstrRet=NULL;

    UNITEXFILEMAPPED *amf;
    const void*buffer;
    size_t size_file;
	GetUnitexFileReadBuffer(jstc_filename.getJString(),&amf, &buffer,&size_file);
	const unsigned char* bufchar=(const unsigned char*)buffer;
	size_t size_bom=0;
	bool is_utf16_native_endianess=false;
	bool is_utf16_swap_endianess = false;

    if (size_file>1)
        if (((*(bufchar))==0xff) && ((*(bufchar+1))==0xfe))
        {
			// little endian
			is_utf16_native_endianess = is_little_endian();
            is_utf16_swap_endianess = ! is_utf16_native_endianess;
            size_bom = 2;
        }
    
    if (size_file>1)
        if (((*(bufchar))==0xfe) && ((*(bufchar+1))==0xff))
        {
			// big endian
			is_utf16_native_endianess = ! is_little_endian();
            is_utf16_swap_endianess = ! is_utf16_native_endianess;
            size_bom = 2;
        }
    
    if (size_file>2)
        if (((*(bufchar))==0xef) && ((*(bufchar+1))==0xbb) && ((*(bufchar+2))==0xbf))
        {
            size_bom = 3;
        }

	if (is_utf16_native_endianess)
	{
		jstrRet = env->NewString((const jchar*)(bufchar+size_bom), (jsize)((size_file - size_bom) / sizeof(jchar)));
	}
	else
    if (is_utf16_swap_endianess)
	{
		unsigned char* returnedUTF16buffer = (unsigned char*)malloc(size_file);
		if (returnedUTF16buffer != NULL)
		{
			for (size_t i=0;i<size_file;i+=2)
			{
				unsigned char c1 = *(bufchar+i);
				unsigned char c2 = *(bufchar+i+1);
				*(returnedUTF16buffer+i) = c2;
				*(returnedUTF16buffer+i+1) = c1;
			}
			jstrRet = env->NewString((const jchar*)(returnedUTF16buffer+size_bom), (jsize)((size_file - size_bom) / sizeof(jchar)));
			free(returnedUTF16buffer);
		}
	}
	else
	{
		char* stringUtf = (char*)malloc(size_file+1);
		memcpy(stringUtf,bufchar+size_bom,size_file-size_bom);
		*(stringUtf+size_file-size_bom)='\0';
		jstrRet = env->NewStringUTF(stringUtf);
		free(stringUtf);
	}

    CloseUnitexFileReadBuffer(amf, buffer, size_file);
    return jstrRet;
}


/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    removeUnitexFile
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_removeUnitexFile
  (JNIEnv *env, jclass, jstring filename)
{
	jstringToCUtf jstc_filename;
	jstc_filename.initJString(env,filename);
	return (RemoveUnitexFile(jstc_filename.getJString()) == 0);
}


/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    createUnitexFolder
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_createUnitexFolder
  (JNIEnv *env, jclass, jstring foldername)
{
	jstringToCUtf jstc_foldername;
	jstc_foldername.initJString(env,foldername);
	return (CreateUnitexFolder(jstc_foldername.getJString()) == 0);
}


/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    removeUnitexFolder
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_removeUnitexFolder
  (JNIEnv *env, jclass, jstring foldername)
{
	jstringToCUtf jstc_foldername;
	jstc_foldername.initJString(env,foldername);
	return (RemoveUnitexFolder(jstc_foldername.getJString()) == 0);
}


/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    renameUnitexFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_renameUnitexFile
  (JNIEnv *env, jclass, jstring filenameSrc, jstring filenameDst)
{
	jstringToCUtf jstc_filenameSrc;
	jstc_filenameSrc.initJString(env,filenameSrc);
	jstringToCUtf jstc_filenameDst;
	jstc_filenameDst.initJString(env,filenameDst);
	return (RenameUnitexFile(jstc_filenameSrc.getJString(),jstc_filenameDst.getJString()) == 0);
}


/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    copyUnitexFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_copyUnitexFile
  (JNIEnv *env, jclass, jstring filenameSrc, jstring filenameDst)
{
	jstringToCUtf jstc_filenameSrc;
	jstc_filenameSrc.initJString(env,filenameSrc);
	jstringToCUtf jstc_filenameDst;
	jstc_filenameDst.initJString(env,filenameDst);
	return (CopyUnitexFile(jstc_filenameSrc.getJString(),jstc_filenameDst.getJString()) == 0);
}

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    setStdOutTrashMode
 * Signature: (Z)Z
 */
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_setStdOutTrashMode
  (JNIEnv *, jclass, jboolean trashOutput)
{
	enum stdwrite_kind swk=stdwrite_kind_out;
	return (SetStdWriteCB(swk, trashOutput ? 1 : 0, NULL, NULL) == 1) ? JNI_TRUE : JNI_FALSE;
}

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    setStdErrTrashMode
 * Signature: (Z)Z
 */
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_setStdErrTrashMode
  (JNIEnv *, jclass, jboolean trashOutput)
{
	enum stdwrite_kind swk=stdwrite_kind_err;
	return (SetStdWriteCB(swk, trashOutput ? 1 : 0, NULL, NULL) == 1) ? JNI_TRUE : JNI_FALSE;
}



static struct UniLoggerSpace * p_ule = NULL;

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    installLogger
 * Signature: (Ljava/lang/String;Z)Z
 */
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_installLogger
  (JNIEnv *env, jclass, jstring foldername, jboolean store_file_out_content)
{
	jstringToCUtf jstc_foldername;
	jstc_foldername.initJString(env,foldername);

	if (p_ule)
		return JNI_FALSE;

	p_ule= (struct UniLoggerSpace *)malloc(sizeof(struct UniLoggerSpace));
    if (!p_ule)
		return JNI_FALSE;
	memset(p_ule,0,sizeof(struct UniLoggerSpace));

	p_ule->size_of_struct = sizeof(struct UniLoggerSpace);
	p_ule->privateUnloggerData = NULL;
	p_ule->szPathLog = strdup(jstc_foldername.getJString());
	p_ule->szNameLog = NULL;
	p_ule->store_file_out_content = store_file_out_content;
	p_ule->store_list_file_out_content = 1;
	p_ule->store_file_in_content = 1;
	p_ule->store_list_file_in_content = 1;
	p_ule->store_std_out_content = 0;
	p_ule->store_std_err_content = 0;
	p_ule->auto_increment_logfilename = 1;

	if (0 != AddActivityLogger(p_ule))
		return JNI_TRUE;

	free(p_ule);
	return JNI_FALSE;
}


/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    removeLogger
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_fr_umlv_unitex_jni_UnitexJni_removeLogger
  (JNIEnv *, jclass)
{
	if (p_ule == NULL)
		return JNI_FALSE;
	RemoveActivityLogger(p_ule);
	free((void*)(p_ule->szPathLog));
	free(p_ule);
	p_ule=NULL;
	return JNI_TRUE;
}


JNIEXPORT jint JNICALL Java_fr_umlv_unitex_jni_UnitexJni_getSvnRevisionNumber
  (JNIEnv *, jclass) {
#ifdef SVN_REVISION
return (jint)SVN_REVISION;
#else
return (jint)-1;
#endif
}


#if defined(UNITEX_HAVING_MINI_PERSISTANCE) && (!(defined(UNITEX_PREVENT_USING_MINIPERSISTANCE)))
/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    loadPersistentDictionary
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_fr_umlv_unitex_jni_UnitexJni_loadPersistentDictionary
  (JNIEnv* env, jclass, jstring filename) {
jstringToCUtf name;
name.initJString(env,filename);
if (load_persistent_dictionary(name.getJString())) {
	return env->NewStringUTF(name.getJString());
}
return NULL;
}

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    freePersistentDictionary
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT void JNICALL Java_fr_umlv_unitex_jni_UnitexJni_freePersistentDictionary
	(JNIEnv* env, jclass, jstring filename) {
jstringToCUtf name;
name.initJString(env,filename);
free_persistent_dictionary(name.getJString());
}

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    loadPersistentFst2
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_fr_umlv_unitex_jni_UnitexJni_loadPersistentFst2
	(JNIEnv* env, jclass, jstring filename) {
jstringToCUtf name;
name.initJString(env,filename);
if (load_persistent_fst2(name.getJString())) {
	return env->NewStringUTF(name.getJString());
}
return NULL;
}

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    freePersistentFst2
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT void JNICALL Java_fr_umlv_unitex_jni_UnitexJni_freePersistentFst2
	(JNIEnv* env, jclass, jstring filename) {
jstringToCUtf name;
name.initJString(env,filename);
free_persistent_fst2(name.getJString());
}


/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    loadPersistentAlphabet
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_fr_umlv_unitex_jni_UnitexJni_loadPersistentAlphabet
	(JNIEnv* env, jclass, jstring filename) {
jstringToCUtf name;
name.initJString(env,filename);
if (load_persistent_alphabet(name.getJString())) {
	return env->NewStringUTF(name.getJString());
}
return NULL;
}

/*
 * Class:     fr_umlv_unitex_jni_UnitexJni
 * Method:    freePersistentAlphabet
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT void JNICALL Java_fr_umlv_unitex_jni_UnitexJni_freePersistentAlphabet
	(JNIEnv* env, jclass, jstring filename) {
jstringToCUtf name;
name.initJString(env,filename);
free_persistent_alphabet(name.getJString());
}

#endif
