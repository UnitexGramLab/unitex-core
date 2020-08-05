/*
 * Unitex
 *
 * Copyright (C) 2001-2020 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * additional information: http://www.smartversion.com/unitex-contribution/
 * https://github.com/ergonotics/JNI-for-Unitex-2.1
 * contact : info@winimage.com
 *
 */


#ifdef UNITEX_RUBY_EXTENSION

// Include the Ruby headers and goodies
#include "ruby.h"

#ifndef RUBY_VINTAGE
#include "ruby/encoding.h"
#endif

#include "UnitexRuby.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "UnitexTool.h"
#include "SyncLogger.h"

#include "AbstractFilePlugCallback.h"
#include "UserCancellingPlugCallback.h"
#include "UniLogger.h"
#include "UniRunLogger.h"

#include "UnitexLibIO.h"

#if defined(UNITEX_HAVING_PERSISTANCE_INTERFACE) && (!(defined(UNITEX_PREVENT_USING_PERSISTANCE_INTERFACE)))
#include "PersistenceInterface.h"
#endif

#include "Copyright.h"

#ifdef HAS_UNITEX_NAMESPACE
using namespace unitex;
#endif

#ifdef HAS_LOGGER_NAMESPACE
using namespace logger;
#endif

#if (!defined(SVN_REVISION))
#include "Unitex_revision.h"
#define SVN_REVISION UNITEX_REVISION
#endif

#if ((defined(WIN32) || defined(_WIN32) || defined (_WIN64) || defined (_M_IX86)  || \
     defined(__i386) || defined(__i386__) || defined(__x86_64) || defined(__x86_64__) || \
     defined(_M_X64) || defined(_M_X86) || defined(TARGET_CPU_X86) || defined(TARGET_CPU_X86_64) || \
	 defined(__arm__) || defined(_ARM_) || defined(__CC_ARM) || defined(_M_ARM) || defined(_M_ARMT) || \
	 defined(__LITTLE_ENDIAN__) \
           ) && (!(defined(INTEL_X86_LIKE_LITTLE_ENDIAN))))
#define INTEL_X86_LIKE_LITTLE_ENDIAN 1
#endif

/*

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
 */

#ifndef RUBY_VINTAGE

#ifdef UNITEX_HAVE_SYNCTOOL
#include "SyncTool.h"
#else
// for unitex before release 3.0
#include "logger/SyncLogger.h"
#endif


class rubyTool
{
public:
    rubyTool() :
      utf8_encoding_index(0), utf16le_encoding_index(0), utf16be_encoding_index(0),
      utf8_encoding(NULL), utf16le_encoding(NULL), utf16be_encoding(NULL),
      mutex(NULL), counterLow(0), counterHigh(0)
    {
        utf8_encoding_index = rb_enc_find_index("UTF-8");
        utf16le_encoding_index = rb_enc_find_index("UTF-16LE");
        utf16be_encoding_index = rb_enc_find_index("UTF-16BE");

        utf8_encoding = rb_enc_find("UTF-8");
        utf16le_encoding = rb_enc_find("UTF-16LE");
        utf16be_encoding = rb_enc_find("UTF-16BE");

        //
        /* name -> rb_encoding */
        //rb_encoding * rb_enc_find(const char *name);

        mutex = SyncBuildMutex();
    };

    ~rubyTool()
    {
        SyncDeleteMutex(mutex);
        mutex = NULL;
    };

    int get_utf8_encoding_index() { return utf8_encoding_index; } ;
    int get_utf16le_encoding_index() { return utf16le_encoding_index; } ;
    int get_utf16be_encoding_index() { return utf16be_encoding_index; } ;

    rb_encoding * get_utf8_encoding() { return utf8_encoding; } ;
    rb_encoding * get_utf16le_encoding() { return utf16le_encoding; } ;
    rb_encoding * get_utf16be_encoding() { return utf16be_encoding; } ;

    void get_incremented_counter(unsigned long *high, unsigned long* low)
    {
        SyncGetMutex(mutex);
        unsigned long new_low = counterLow + 1;
        if (new_low < counterLow)
            counterHigh ++;
        counterLow ++;
        if (low != NULL)
            *low = counterLow;
        if (high != NULL)
            *high = counterHigh;
        SyncReleaseMutex(mutex);
    }

private:
    int utf8_encoding_index;
    int utf16le_encoding_index;
    int utf16be_encoding_index;

    rb_encoding * utf8_encoding;
    rb_encoding * utf16le_encoding;
    rb_encoding * utf16be_encoding;

    SYNC_Mutex_OBJECT mutex;
    unsigned long counterLow, counterHigh;
};

rubyTool _rubyTool;


VALUE method_getUniqueString(VALUE self)
{
    char szText[0x200];
    unsigned long low = 0;
    unsigned long high = 0;
    _rubyTool.get_incremented_counter(&high, &low);
    sprintf(szText, "%ld_%ld",low,high);

    return rb_str_new2(szText);
}

#endif

// Defining a space for information and references about the module to be stored internally
VALUE UnitexRuby = Qnil;



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


static char**argsFromCmdLine( const char* cmdLine)
{
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


VALUE method_UnitexTool(VALUE self, VALUE r_args)
{
    char* args_cstr = StringValueCStr(r_args);

	char**args=argsFromCmdLine(args_cstr);
	int retValue = (int)main_UnitexTool_C(countArgs(args),args);
	freeArgs(args);

    return INT2NUM(retValue);
}

VALUE method_UnitexToolArray(VALUE self, VALUE r_args_array)
{
    int nb_args = RARRAY_LEN(r_args_array);
    int i;
    char** args = (char**)malloc((nb_args+1) * sizeof(char*));
    for (i = 0; i < nb_args; i++)
    {
        VALUE item = rb_ary_entry(r_args_array, i);
        const char* curArgs = StringValueCStr(item);
        args[i] = strdup(curArgs);
    }
    args[i] = NULL;
	int retValue = (int)main_UnitexTool_C(countArgs(args),args);
	freeArgs(args);

    return INT2NUM(retValue);
}

VALUE method_getMajorVersionNumber(VALUE self)
{
#ifdef UNITEX_MAJOR_VERSION_NUMBER
    return INT2NUM(UNITEX_MAJOR_VERSION_NUMBER);
#else
    return INT2NUM(2);
#endif
}


VALUE method_getMinorVersionNumber(VALUE self)
{
#ifdef UNITEX_MINOR_VERSION_NUMBER
    return INT2NUM(UNITEX_MINOR_VERSION_NUMBER);
#else
    return INT2NUM(1);
#endif
}


VALUE method_getSvnRevisionNumber(VALUE self)
{
#ifdef SVN_REVISION
    return INT2NUM(SVN_REVISION);
#else
    return INT2NUM(-1);
#endif
}


VALUE method_getUnitexInfoString(VALUE self)
{
    char szText[0x200];
    sprintf(szText, "Unitex %d.%d revision %d",


#ifdef UNITEX_MAJOR_VERSION_NUMBER
            UNITEX_MAJOR_VERSION_NUMBER,
#else
            2,
#endif
#ifdef UNITEX_MINOR_VERSION_NUMBER
            UNITEX_MINOR_VERSION_NUMBER,
#else
            1,
#endif

#ifdef SVN_REVISION
            SVN_REVISION
#else
            -1
#endif
            );

    return rb_str_new2(szText);
}


VALUE method_usingOffsetFile(VALUE self)
{
#ifdef UNITEX_HAVING_OFFSET_FILE
    return Qtrue;
#endif
#ifdef UNITEX_REVISION
    return (UNITEX_REVISION >= 2292) ? Qtrue : Qfalse;
#endif
    return Qfalse;
}

VALUE method_numberAbstractFileSpaceInstalled(VALUE self)
{
    int numberAbstractFileSpace = GetNbAbstractFileSpaceInstalled();

    return INT2NUM(numberAbstractFileSpace);
}



VALUE method_setStdOutTrashMode(VALUE self, VALUE isTrash)
{
    int trashOutput = RTEST(isTrash);
	enum stdwrite_kind swk=stdwrite_kind_out;
	return (SetStdWriteCB(swk, trashOutput ? 1 : 0, NULL, NULL) == 1) ? Qtrue : Qfalse;
}


VALUE method_setStdErrTrashMode(VALUE self, VALUE isTrash)
{
    int trashOutput = RTEST(isTrash);
	enum stdwrite_kind swk=stdwrite_kind_err;
	return (SetStdWriteCB(swk, trashOutput ? 1 : 0, NULL, NULL) == 1) ? Qtrue : Qfalse;
}




VALUE method_loadPersistentDictionary(VALUE self, VALUE r_args)
{
    char* filename = StringValueCStr(r_args);
    VALUE rret = Qnil;

    size_t len_buffer = strlen(filename) + 0x200;
    char* persistent_filename=(char*)malloc(len_buffer+1);
    if (persistent_filename == NULL) {
        return Qnil;
    }
    if (standard_load_persistence_dictionary(filename,persistent_filename,len_buffer)) {
        rret = rb_str_new2(persistent_filename);
    }
    free(persistent_filename);
    return rret;
}

VALUE method_loadPersistentFst2(VALUE self, VALUE r_args)
{
    char* filename = StringValueCStr(r_args);
    VALUE rret = Qnil;

    size_t len_buffer = strlen(filename) + 0x200;
    char* persistent_filename=(char*)malloc(len_buffer+1);
    if (persistent_filename == NULL) {
        return Qnil;
    }
    if (standard_load_persistence_fst2(filename,persistent_filename,len_buffer)) {
        rret = rb_str_new2(persistent_filename);
    }
    free(persistent_filename);
    return rret;
}

VALUE method_loadPersistentAlphabet(VALUE self, VALUE r_args)
{
    char* filename = StringValueCStr(r_args);
    VALUE rret = Qnil;

    size_t len_buffer = strlen(filename) + 0x200;
    char* persistent_filename=(char*)malloc(len_buffer+1);
    if (persistent_filename == NULL) {
        return Qnil;
    }
    if (standard_load_persistence_alphabet(filename,persistent_filename,len_buffer)) {
        rret = rb_str_new2(persistent_filename);
    }
    free(persistent_filename);
    return rret;
}

VALUE method_freePersistentDictionary(VALUE self, VALUE r_args)
{
    char* persistent_filename = StringValueCStr(r_args);
    standard_unload_persistence_dictionary(persistent_filename);
    return Qnil;
}

VALUE method_freePersistentFst2(VALUE self, VALUE r_args)
{
    char* persistent_filename = StringValueCStr(r_args);
    standard_unload_persistence_fst2(persistent_filename);
    return Qnil;
}

VALUE method_freePersistentAlphabet(VALUE self, VALUE r_args)
{
    char* persistent_filename = StringValueCStr(r_args);
    standard_unload_persistence_alphabet(persistent_filename);
    return Qnil;
}

VALUE method_copyUnitexFile(VALUE self, VALUE fileSrc, VALUE fileDst)
{
    char* filenameSrc = StringValueCStr(fileSrc);
    char* filenameDst = StringValueCStr(fileDst);

    int result = (CopyUnitexFile(filenameSrc,filenameDst) == 0);

    return result ? Qtrue : Qfalse;
}

VALUE method_renameUnitexFile(VALUE self, VALUE fileSrc, VALUE fileDst)
{
    char* filenameSrc = StringValueCStr(fileSrc);
    char* filenameDst = StringValueCStr(fileDst);

    int result = (RenameUnitexFile(filenameSrc,filenameDst) == 0);

    return result ? Qtrue : Qfalse;
}

VALUE method_unitexAbstractPathExists(VALUE self, VALUE filename)
{
    char* c_filename = StringValueCStr(filename);

    int result = (UnitexAbstractPathExists(c_filename) != 0);

    return result ? Qtrue : Qfalse;
}

VALUE method_removeUnitexFolder(VALUE self, VALUE filename)
{
    char* c_filename = StringValueCStr(filename);

    int result = (RemoveUnitexFolder(c_filename) == 0);

    return result ? Qtrue : Qfalse;
}

VALUE method_createUnitexFolder(VALUE self, VALUE filename)
{
    char* c_filename = StringValueCStr(filename);

    int result = (CreateUnitexFolder(c_filename) == 0);

    return result ? Qtrue : Qfalse;
}

VALUE method_removeUnitexFile(VALUE self, VALUE filename)
{
    char* c_filename = StringValueCStr(filename);

    int result = (RemoveUnitexFile(c_filename) == 0);

    return result ? Qtrue : Qfalse;
}


VALUE method_getUnitexFileRaw(VALUE self, VALUE filename)
{
    char* c_filename = StringValueCStr(filename);

    UNITEXFILEMAPPED *amf = NULL;
    const void*buffer = 0;
    size_t size_file = 0;
	GetUnitexFileReadBuffer(c_filename,&amf, &buffer,&size_file);
    if (amf == NULL)
        return Qnil;

    VALUE vret = rb_str_new((const char*)buffer, size_file);

    CloseUnitexFileReadBuffer(amf, buffer, size_file);
    return vret;
}

VALUE method_writeUnitexFileRaw(VALUE self, VALUE filename, VALUE string)
{
    char* c_filename = StringValueCStr(filename);
    char* c_string = StringValueCStr(string);

	int ret = (WriteUnitexFile(c_filename,NULL,0,
                               c_string,strlen(c_string)) == 0);

	return ret ? Qtrue : Qfalse;
}

VALUE method_appendUnitexFileRaw(VALUE self, VALUE filename, VALUE string)
{
    char* c_filename = StringValueCStr(filename);
    char* c_string = StringValueCStr(string);

	int ret = (AppendUnitexFile(c_filename,
                                   c_string,strlen(c_string)) == 0);

	return ret ? Qtrue : Qfalse;
}



#ifndef RUBY_VINTAGE
VALUE method_writeUnitexFileString(VALUE self, VALUE filename, VALUE string)
{
    char* c_filename = StringValueCStr(filename);

    rb_encoding *utf8_enc = rb_enc_find("UTF-8");
    VALUE utf8_string = rb_str_export_to_enc(string, utf8_enc);
    const char *c_string = StringValueCStr(utf8_string);


	const unsigned char UTF8BOM[3] = { 0xef,0xbb,0xbf };
    int hasBom = 1; // put 1 to write UTF8 bom

	int ret = (WriteUnitexFile(c_filename, UTF8BOM, hasBom ? 3:0,
                               c_string, strlen(c_string)) == 0);

	return ret ? Qtrue : Qfalse;
}


VALUE method_getUnitexFileStringEncoded(VALUE self, VALUE filename)
{
    UNITEXFILEMAPPED *amf = NULL;
    const void*buffer = 0;
    size_t size_file = 0;
    VALUE vret = Qnil;
	size_t size_bom = 0;
    char* c_filename = StringValueCStr(filename);

	GetUnitexFileReadBuffer(c_filename,&amf, &buffer,&size_file);
	const unsigned char* bufchar=(const unsigned char*)buffer;

    if (amf == NULL)
        return Qnil;

    if (size_file>1)
        if (((*(bufchar))==0xff) && ((*(bufchar+1))==0xfe))
        {
			// utf-16 little endian
            size_bom = 2;

            vret = rb_external_str_new_with_enc((const char*)bufchar + size_bom, size_file - size_bom, rb_enc_find("UTF-16LE"));
            vret = rb_str_export_to_enc(vret, rb_enc_find("UTF-8"));
        }

    if (size_file>1)
        if (((*(bufchar))==0xfe) && ((*(bufchar+1))==0xff))
        {
			// utf-16 big endian
            size_bom = 2;

            vret = rb_external_str_new_with_enc((const char*)bufchar + size_bom, size_file - size_bom, rb_enc_find("UTF-16BE"));
            vret = rb_str_export_to_enc(vret, rb_enc_find("UTF-8"));
        }

    if (size_file>2)
        if (((*(bufchar))==0xef) && ((*(bufchar+1))==0xbb) && ((*(bufchar+2))==0xbf))
        {
            size_bom = 3;
        }


    if (vret == Qnil)
    {
        vret = rb_external_str_new_with_enc((const char*)bufchar + size_bom, size_file - size_bom, rb_enc_find("UTF-8"));
    }

    CloseUnitexFileReadBuffer(amf, buffer, size_file);
    return vret;
}


VALUE method_getUnitexFileString(VALUE self, VALUE filename)
{
    UNITEXFILEMAPPED *amf = NULL;
    const void*buffer = 0;
    size_t size_file = 0;
    VALUE vret = Qnil;
	size_t size_bom = 0;
    char* c_filename = StringValueCStr(filename);

	GetUnitexFileReadBuffer(c_filename,&amf, &buffer,&size_file);
	const unsigned char* bufchar=(const unsigned char*)buffer;

    if (amf == NULL)
        return Qnil;

    if (size_file>1)
        if (((*(bufchar))==0xff) && ((*(bufchar+1))==0xfe))
        {
			// utf-16 little endian
            size_bom = 2;

            vret = rb_external_str_new_with_enc((const char*)bufchar + size_bom, size_file - size_bom, rb_enc_find("UTF-16LE"));
        }

    if (size_file>1)
        if (((*(bufchar))==0xfe) && ((*(bufchar+1))==0xff))
        {
			// utf-16 big endian
            size_bom = 2;

            vret = rb_external_str_new_with_enc((const char*)bufchar + size_bom, size_file - size_bom, rb_enc_find("UTF-16BE"));
        }

    if (size_file>2)
        if (((*(bufchar))==0xef) && ((*(bufchar+1))==0xbb) && ((*(bufchar+2))==0xbf))
        {
            size_bom = 3;
        }


    if (vret == Qnil)
    {
        vret = rb_external_str_new_with_enc((const char*)bufchar + size_bom, size_file - size_bom, rb_enc_find("UTF-8"));
    }

    CloseUnitexFileReadBuffer(amf, buffer, size_file);
    return vret;
}



#endif

static struct UniLoggerSpace * p_ruby_ule = NULL;

VALUE method_installLogger(VALUE self, VALUE foldername, VALUE v_store_file_out_content)
{
    char* c_foldername = StringValueCStr(foldername);
	if (p_ruby_ule)
		return Qfalse;

	p_ruby_ule= (struct UniLoggerSpace *)malloc(sizeof(struct UniLoggerSpace));
    if (!p_ruby_ule)
		return Qfalse;
	memset(p_ruby_ule,0,sizeof(struct UniLoggerSpace));

	p_ruby_ule->size_of_struct = sizeof(struct UniLoggerSpace);
	p_ruby_ule->privateUnloggerData = NULL;
	p_ruby_ule->szPathLog = strdup(c_foldername);
	p_ruby_ule->szNameLog = NULL;
	p_ruby_ule->store_file_out_content = RTEST(v_store_file_out_content) ? 1 : 0;
	p_ruby_ule->store_list_file_out_content = 1;
	p_ruby_ule->store_file_in_content = 1;
	p_ruby_ule->store_list_file_in_content = 1;
	p_ruby_ule->store_std_out_content = 0;
	p_ruby_ule->store_std_err_content = 0;
	p_ruby_ule->auto_increment_logfilename = 1;

	if (0 != AddActivityLogger(p_ruby_ule))
		return Qtrue;

	free(p_ruby_ule);
	return Qfalse;
}






VALUE method_removeLogger(VALUE self)
{
	if (p_ruby_ule == NULL)
		return Qfalse;
	RemoveActivityLogger(p_ruby_ule);
	free((void*)(p_ruby_ule->szPathLog));
	free(p_ruby_ule);
	p_ruby_ule=NULL;
	return Qtrue;
}


VALUE method_getFileList(VALUE self, VALUE foldername)
{
    char* c_foldername = StringValueCStr(foldername);

	char**list=GetUnitexFileList(c_foldername);
	if (list==NULL)
		return Qnil;

    VALUE array = rb_ary_new();
	unsigned int iter_file = 0;
	while ((*(list + iter_file)) != NULL)
	{
        VALUE item = rb_str_new_cstr(*(list + iter_file));
        rb_ary_push(array, item);
		iter_file ++;
	}

    ReleaseUnitexFileList(c_foldername, list);

    return array;
}




struct UnitexRubyInitializer {
	t_unitex_ruby_initializer_func_array func_array;
	void* privateSpacePtr;
} ;


struct List_UnitexRubyInitializer {
	UnitexRubyInitializer uri;
	List_UnitexRubyInitializer* next;
} ;



struct List_UnitexRubyInitializer* p_unitex_ruby_initializer_space_list = NULL;


UNITEX_FUNC int UNITEX_CALL AddUnitexRubyInitializer(const t_unitex_ruby_initializer_func_array* func_array_param,void* privateSpacePtr)
{
	struct List_UnitexRubyInitializer* new_item;
	new_item = (struct List_UnitexRubyInitializer*)malloc(sizeof(struct List_UnitexRubyInitializer));
	if (new_item == NULL)
      return 0;
    new_item->uri.func_array.size_func_array = func_array_param->size_func_array;
    new_item->uri.func_array.fnc_unitex_ruby_initializer = func_array_param->fnc_unitex_ruby_initializer;
    new_item->uri.privateSpacePtr = privateSpacePtr;

	new_item->next = NULL;

	if (p_unitex_ruby_initializer_space_list == NULL)
      p_unitex_ruby_initializer_space_list = new_item;
	else {
		struct List_UnitexRubyInitializer* tmp = p_unitex_ruby_initializer_space_list;
		while ((tmp->next) != NULL)
          tmp = tmp->next;
		tmp->next = new_item;
	}

	return 1;
}

UNITEX_FUNC int UNITEX_CALL RemoveUnitexRubyInitializer(const t_unitex_ruby_initializer_func_array* func_array_param,void* privateSpacePtr)
{
	struct List_UnitexRubyInitializer* tmp = p_unitex_ruby_initializer_space_list;
	struct List_UnitexRubyInitializer* tmp_previous = NULL;

	while (tmp != NULL)
	{
        if ((tmp->uri.func_array.fnc_unitex_ruby_initializer == func_array_param->fnc_unitex_ruby_initializer) &&
            (tmp->uri.privateSpacePtr == privateSpacePtr))
		{
			if (tmp_previous == NULL)
              p_unitex_ruby_initializer_space_list = tmp->next;
			else
              tmp_previous->next = tmp->next;

			free(tmp);
			return 1;
		}
		tmp_previous = tmp;
		tmp = tmp->next;
	}

	return 0;
}



typedef VALUE (*t_r_method)(ANYARGS);

// Prototype for the initialization method - Ruby calls this, not you
extern "C" UNITEX_FUNC void Init_unitexruby()
{
    UnitexRuby = rb_define_module("UnitexRuby");

	rb_define_method(UnitexRuby, "UnitexTool", (t_r_method) &method_UnitexTool, 1);
	rb_define_method(UnitexRuby, "UnitexToolArray", (t_r_method) &method_UnitexToolArray, 1);

	rb_define_method(UnitexRuby, "getUnitexInfoString", (t_r_method) &method_getUnitexInfoString, 0);
	rb_define_method(UnitexRuby, "getMajorVersionNumber", (t_r_method) &method_getMajorVersionNumber, 0);
	rb_define_method(UnitexRuby, "getMinorVersionNumber", (t_r_method) &method_getMinorVersionNumber, 0);
	rb_define_method(UnitexRuby, "usingOffsetFile", (t_r_method) &method_usingOffsetFile, 0);
	rb_define_method(UnitexRuby, "numberAbstractFileSpaceInstalled", (t_r_method) &method_numberAbstractFileSpaceInstalled, 0);
	rb_define_method(UnitexRuby, "getSvnRevisionNumber", (t_r_method) &method_getSvnRevisionNumber, 0);
	rb_define_method(UnitexRuby, "setStdOutTrashMode", (t_r_method) &method_setStdOutTrashMode, 1);
	rb_define_method(UnitexRuby, "setStdErrTrashMode", (t_r_method) &method_setStdErrTrashMode, 1);
	rb_define_method(UnitexRuby, "loadPersistentDictionary", (t_r_method) &method_loadPersistentDictionary, 1);
	rb_define_method(UnitexRuby, "loadPersistentFst2", (t_r_method) &method_loadPersistentFst2, 1);
	rb_define_method(UnitexRuby, "loadPersistentAlphabet", (t_r_method) &method_loadPersistentAlphabet, 1);
	rb_define_method(UnitexRuby, "freePersistentDictionary", (t_r_method) &method_freePersistentDictionary, 1);
	rb_define_method(UnitexRuby, "freePersistentFst2", (t_r_method) &method_freePersistentFst2, 1);
	rb_define_method(UnitexRuby, "freePersistentAlphabet", (t_r_method) &method_freePersistentAlphabet, 1);
	rb_define_method(UnitexRuby, "copyUnitexFile", (t_r_method) &method_copyUnitexFile, 2);
	rb_define_method(UnitexRuby, "renameUnitexFile", (t_r_method) &method_renameUnitexFile, 2);
	rb_define_method(UnitexRuby, "unitexAbstractPathExists", (t_r_method) &method_unitexAbstractPathExists, 1);
	rb_define_method(UnitexRuby, "removeUnitexFolder", (t_r_method) &method_removeUnitexFolder, 1);
	rb_define_method(UnitexRuby, "createUnitexFolder", (t_r_method) &method_createUnitexFolder, 1);
	rb_define_method(UnitexRuby, "removeUnitexFile", (t_r_method) &method_removeUnitexFile, 1);
	rb_define_method(UnitexRuby, "getFileList", (t_r_method) &method_getFileList, 1);
	rb_define_method(UnitexRuby, "getUnitexFileRaw", (t_r_method) &method_getUnitexFileRaw, 1);
	rb_define_method(UnitexRuby, "writeUnitexFileRaw", (t_r_method) &method_writeUnitexFileRaw, 2);
	rb_define_method(UnitexRuby, "appendUnitexFileRaw", (t_r_method) &method_appendUnitexFileRaw, 2);
	rb_define_method(UnitexRuby, "installLogger", (t_r_method) &method_installLogger, 2);
	rb_define_method(UnitexRuby, "removeLogger", (t_r_method) &method_removeLogger, 0);


#ifndef RUBY_VINTAGE
	rb_define_method(UnitexRuby, "writeUnitexFileString", (t_r_method) &method_writeUnitexFileString, 2);
	rb_define_method(UnitexRuby, "getUnitexFileStringEncoded", (t_r_method) &method_getUnitexFileStringEncoded, 1);
	rb_define_method(UnitexRuby, "getUnitexFileString", (t_r_method) &method_getUnitexFileString, 1);
	rb_define_method(UnitexRuby, "getUniqueString", (t_r_method) &method_getUniqueString, 0);

#endif

	struct List_UnitexRubyInitializer* tmp = p_unitex_ruby_initializer_space_list;

	while (tmp != NULL)
	{
        (tmp->uri.func_array.fnc_unitex_ruby_initializer)(UnitexRuby, tmp->uri.privateSpacePtr);
		tmp = tmp->next;
	}

}

#endif
