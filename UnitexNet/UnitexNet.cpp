// Il s'agit du fichier DLL principal.

#include "stdafx.h"

#include "clix.h"
#include "UnitexNet.h"

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

#ifdef INTEL_X86_LIKE_LITTLE_ENDIAN
static bool is_little_endian()
{
	// printf("INTEL_X86_LIKE_LITTLE_ENDIAN is,");
	return true;
}
#else
static bool is_little_endian()
{
	const jchar i = 1;
	const char *c = (const char*)&i;
	bool little_endian = ((*c) != 0);
	// printf("is_little_endian = %s\n",little_endian ? "y":"n");
	return little_endian;
}
#endif

namespace UnitexNet
{

	const char* CopyStrArg(const char* lpSrc, char* lpDest, size_t dwMaxSize)
	{
		int isInQuote = 0;
		size_t dwDestPos = 0;

		*lpDest = '\0';

		if (lpSrc == NULL)
			return NULL;

		while ((*lpSrc) == ' ')
			lpSrc++;

		while (((*lpSrc) != '\0') && ((dwMaxSize == 0) || (dwDestPos < dwMaxSize)))
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

	char** argsFromManagedStringArray(array<String^>^ strArray)
	{
		int nSize = strArray->Length;
		char **args = NULL;
		if (nSize > 0)
			args = (char**)malloc((nSize + 1) * sizeof(char*));

		if (args != NULL)
		{
			int nbArgs = nSize;
			int i;
			for (i = 0; i < nbArgs; i++) {
				String^ managedString = strArray[i];
				std::string stdString = msclr::interop::marshal_as<std::string>(managedString);
				const char* sz = stdString.c_str();

				// we can replace these line by mpszArgs[i] = strdup(sz) to not remove quote
				size_t string_alloc_size = strlen(sz) + 4;
				args[i] = (char*)malloc(string_alloc_size + 4);
				CopyStrArg(sz, args[i], string_alloc_size);
			}
			args[i] = NULL;
		}
		return args;
	}

	char** argsFromManagedString(String^ string)
	{
		std::string stdString = msclr::interop::marshal_as<std::string>(string);
		const char* cmdLine = stdString.c_str();
		size_t len_cmd_line = strlen(cmdLine);

		char* work_buf = (char*)malloc(len_cmd_line + 0x10);
		if (work_buf == NULL)
			return 0;

		const char* lpParcLine;
		int nb_args_found = 0;

		lpParcLine = cmdLine;
		while ((*lpParcLine) != '\0')
		{
			*work_buf = 0;
			lpParcLine = CopyStrArg(lpParcLine, work_buf, len_cmd_line + 8);
			nb_args_found++;
		}

		char** args = NULL;
		if (nb_args_found > 0)
			args = (char**)malloc((nb_args_found + 1) * sizeof(char*));
		else
			args = NULL;

		if (args == NULL)
		{
			free(work_buf);
			return NULL;
		}

		lpParcLine = cmdLine;
		int i = 0;
		while ((*lpParcLine) != '\0')
		{
			*work_buf = 0;
			lpParcLine = CopyStrArg(lpParcLine, work_buf, len_cmd_line + 8);
			args[i] = strdup(work_buf);
			i++;
		}
		args[i] = NULL;

		free(work_buf);
		return args;
	}

	int countArgs(char** args)
	{
		int count = 0;

		char**tmp = args;
		while ((*tmp) != NULL)
		{
			count++;
			tmp++;
		}
		return count;
	}

	void freeArgs(char**args)
	{
		char** tmp = args;
		while ((*tmp) != NULL)
		{
			free(*tmp);
			tmp++;
		}
		free(args);
	}

	int Unitex::NumberAbstractFileSpaceInstalled::get()
	{
		return ::GetNbAbstractFileSpaceInstalled();
	}

	int Unitex::ExecUnitexTool(array<String^>^ cmdArray)
	{
		char** args = argsFromManagedStringArray(cmdArray);
		int retValue = main_UnitexTool_C(countArgs(args), args);
		freeArgs(args);
		return retValue;
	}

	int Unitex::ExecUnitexTool(String^ cmdLine)
	{
		char** args = argsFromManagedString(cmdLine);
		int retValue = main_UnitexTool_C(countArgs(args), args);
		freeArgs(args);
		return retValue;
	}

	int Unitex::ExecRunLog(array<String^>^ cmdArray)
	{
		char** args = argsFromManagedStringArray(cmdArray);
		int retValue = main_RunLog(countArgs(args), args);
		freeArgs(args);
		return retValue;
	}

	int Unitex::ExecRunLog(String^ cmdLine)
	{
		char** args = argsFromManagedString(cmdLine);
		int retValue = main_RunLog(countArgs(args), args);
		freeArgs(args);
		return retValue;
	}

	bool Unitex::WriteUnitexFile(String^ fileName, String^ fileContent)
	{
		std::string stdFilename = clix::marshalString<clix::E_UTF8>(fileName);
		std::wstring stdContents = clix::marshalString<clix::E_UTF16>(fileContent);

		std::wstring::size_type stringSize = stdContents.length();
		long contentByteLength = sizeof(WCHAR) * stringSize;
		long bufferByteLength = sizeof(WCHAR) * (stringSize + 1);

		void* buffString = (void*)malloc(bufferByteLength);
		unsigned char* buffBom = (unsigned char*)buffString;
		*(buffBom++) = 0xff;
		*(buffBom++) = 0xfe;
		WCHAR* bufContent = (WCHAR*)buffBom;
		memcpy(bufContent, stdContents.c_str(), contentByteLength);

		if (!(is_little_endian()))
		{
			for (std::wstring::size_type j = 0; j < stringSize; j++)
			{
				WCHAR c = *(bufContent + j);
				*(((unsigned char*)(bufContent + j)) + 0) = (unsigned char)(c & 0xff);
				*(((unsigned char*)(bufContent + j)) + 1) = (unsigned char)(c >> 8);
			}
		}
		bool ret = (::WriteUnitexFile(stdFilename.c_str(), buffString, bufferByteLength, NULL, 0) == 0);
		free(buffString);
		return ret;
	}

	bool Unitex::WriteUnitexFile(String^ filename, array<Byte>^ byteArray) 
	{
		std::string stdFilename = clix::marshalString<clix::E_UTF8>(filename);

		pin_ptr<unsigned char> pinBuffer = &byteArray[0];
		unsigned char* byteBuffer = pinBuffer;
		int len = byteArray->Length;

		bool retValue = (::WriteUnitexFile(stdFilename.c_str(), byteBuffer, len, NULL, 0) == 0);
		return retValue;
	}

	bool Unitex::WriteUnitexFile(String^ filename, array<Char>^ charArray)
	{
		std::string stdFilename = clix::marshalString<clix::E_UTF8>(filename);

		pin_ptr<Char> pinBuffer = &charArray[0];
		Char* charBuffer = pinBuffer;
		int len = charArray->Length;

		bool retValue = (::WriteUnitexFile(stdFilename.c_str(), charBuffer, len * sizeof(Char), NULL, 0) == 0);
		return retValue;
	}

	static bool doWriteUnitexFileUtf(String^ filename, String^ filecontent, bool hasBom)
	{
		std::string stdFilename = clix::marshalString<clix::E_UTF8>(filename);
		std::string stdContent = clix::marshalString<clix::E_UTF8>(filecontent);

		const unsigned char UTF8BOM[3] = { 0xef, 0xbb, 0xbf };

		bool ret = (::WriteUnitexFile(
			stdFilename.c_str(), UTF8BOM, hasBom ? 3 : 0,
			stdContent.c_str(), strlen(stdContent.c_str())) == 0);
		return ret;
	}

	bool Unitex::WriteUnitexFileUtf(String^ fileName, String^ fileContent)
	{
		return doWriteUnitexFileUtf(fileName, fileContent, false);
	}

	bool Unitex::WriteUnitexFileUtf(String^ fileName, String^ fileContent, bool withBom)
	{
		return doWriteUnitexFileUtf(fileName, fileContent, withBom);
	}

	String^ Unitex::GetUnitexFileString(String^ fileName)
	{
		std::string stdFilename = clix::marshalString<clix::E_ANSI>(fileName);
		String^ result;

		UNITEXFILEMAPPED *amf;
		const void*buffer;
		size_t size_file;
		GetUnitexFileReadBuffer(stdFilename.c_str(), &amf, &buffer, &size_file);
		const unsigned char* bufchar = (const unsigned char*)buffer;
		size_t size_bom = 0;
		bool is_utf16_native_endianess = false;
		bool is_utf16_swap_endianess = false;

		if (size_file > 1) {
			if (((*(bufchar)) == 0xff) && ((*(bufchar + 1)) == 0xfe))
			{
				// little endian
				is_utf16_native_endianess = is_little_endian();
				is_utf16_swap_endianess = !is_utf16_native_endianess;
				size_bom = 2;
			}
		}

		if (size_file > 1) {
			if (((*(bufchar)) == 0xfe) && ((*(bufchar + 1)) == 0xff))
			{
				// big endian
				is_utf16_native_endianess = !is_little_endian();
				is_utf16_swap_endianess = !is_utf16_native_endianess;
				size_bom = 2;
			}
		}

		if (size_file > 2) {
			if (((*(bufchar)) == 0xef) && ((*(bufchar + 1)) == 0xbb) && ((*(bufchar + 2)) == 0xbf))
			{
				size_bom = 3;
			}
		}

		if (is_utf16_native_endianess)
		{
			result = gcnew String((const wchar_t*)(bufchar + size_bom), 0, (size_file - size_bom) / sizeof(wchar_t));
		}
		else {
			if (is_utf16_swap_endianess)
			{
				unsigned char* returnedUTF16buffer = (unsigned char*)malloc(size_file);
				if (returnedUTF16buffer != NULL)
				{
					for (size_t i = 0; i < size_file; i += 2)
					{
						unsigned char c1 = *(bufchar + i);
						unsigned char c2 = *(bufchar + i + 1);
						*(returnedUTF16buffer + i) = c2;
						*(returnedUTF16buffer + i + 1) = c1;
					}
					result = gcnew String((const wchar_t*)(returnedUTF16buffer + size_bom), 0, (size_file - size_bom) / sizeof(wchar_t));
					free(returnedUTF16buffer);
				}
			}
			else
			{
				char* stringUtf = (char*)malloc(size_file + 1);
				memcpy(stringUtf, bufchar + size_bom, size_file - size_bom);
				*(stringUtf + size_file - size_bom) = '\0';
				result = gcnew String(stringUtf, 0, strlen(stringUtf), System::Text::Encoding::UTF8);
				free(stringUtf);
			}
		}

		CloseUnitexFileReadBuffer(amf, buffer, size_file);
		return result;
	}

	array<Byte>^ Unitex::GetUnitexFileData(String^ filename)
	{
		std::string stdFilename = clix::marshalString<clix::E_ANSI>(filename);

		UNITEXFILEMAPPED* amf;
		const void* buffer;
		size_t size_file;
		GetUnitexFileReadBuffer(stdFilename.c_str(), &amf, &buffer, &size_file);
		
		array<Byte>^ byteArray = gcnew array<Byte>(size_file);
		if (byteArray != nullptr) {
			IntPtr intPtr = IntPtr((unsigned char*) buffer);
			Marshal::Copy(intPtr, byteArray, 0, size_file);
		}

		CloseUnitexFileReadBuffer(amf, buffer, size_file);
		return byteArray;
	}

	array<Char>^ Unitex::GetUnitexFileDataChar(String^ filename)
	{
		std::string stdFilename = clix::marshalString<clix::E_ANSI>(filename);

		UNITEXFILEMAPPED* amf;
		const void* buffer;
		size_t size_file;
		GetUnitexFileReadBuffer(stdFilename.c_str(), &amf, &buffer, &size_file);

		array<Char>^ charArray = gcnew array<Char>(size_file / sizeof(Char));
		if (charArray != nullptr) {
			IntPtr intPtr = IntPtr((Char*)buffer);
			Marshal::Copy(intPtr, charArray, 0, size_file);
		}

		CloseUnitexFileReadBuffer(amf, buffer, size_file);
		return charArray;
	}

	bool Unitex::RemoveUnitexFile(String^ fileName)
	{
		std::string stdFilename = clix::marshalString<clix::E_ANSI>(fileName);
		return (::RemoveUnitexFile(stdFilename.c_str()) == 0);
	}

	bool Unitex::CreateUnitexFolder(String^ folderName)
	{
		std::string stdFoldername = clix::marshalString<clix::E_ANSI>(folderName);
		return (::CreateUnitexFolder(stdFoldername.c_str()) == 0);
	}

	bool Unitex::RemoveUnitexFolder(String^ folderName)
	{
		std::string stdFoldername = clix::marshalString<clix::E_ANSI>(folderName);
		return (::RemoveUnitexFolder(stdFoldername.c_str()) == 0);
	}

	bool Unitex::RenameUnitexFile(String^ fileNameSrc, String^ fileNameDst)
	{
		std::string stdSource = clix::marshalString<clix::E_ANSI>(fileNameSrc);
		std::string stdDestination = clix::marshalString<clix::E_ANSI>(fileNameDst);
		return (::RenameUnitexFile(stdSource.c_str(), stdDestination.c_str()) == 0);
	}

	bool Unitex::CopyUnitexFile(String^ fileNameSrc, String^ fileNameDst)
	{
		std::string stdSource = clix::marshalString<clix::E_ANSI>(fileNameSrc);
		std::string stdDestination = clix::marshalString<clix::E_ANSI>(fileNameDst);
		return (::CopyUnitexFile(stdSource.c_str(), stdDestination.c_str()) == 0);
	}

	bool Unitex::UnitexAbstractPathExists(String^ path)
	{
		std::string stdPath = clix::marshalString<clix::E_ANSI>(path);
		return (::UnitexAbstractPathExists(stdPath.c_str()) != 0);
	}

	array<String^>^ Unitex::GetFileList(String^ path)
	{
		std::string stdPath = clix::marshalString<clix::E_ANSI>(path);

		char** list = ::GetUnitexFileList(stdPath.c_str());
		if (list == NULL)
			return gcnew array<String^>(0);

		unsigned int nb_file = 0;
		while ((*(list + nb_file)) != NULL)
		{
			nb_file++;
		}

		array<String^>^ result = gcnew array<String^>(nb_file);
		unsigned int iter_file = 0;
		while ((*(list + iter_file)) != NULL)
		{
			result[iter_file] = gcnew String(*(list + iter_file));
			iter_file++;
		}

		ReleaseUnitexFileList(stdPath.c_str(), list);

		return result;
	}

	bool Unitex::SetStdOutTrashMode(bool trashOutput)
	{
		enum stdwrite_kind swk = stdwrite_kind_out;
		return (::SetStdWriteCB(swk, trashOutput ? 1 : 0, NULL, NULL) == 1) ? true : false;
	}

	bool Unitex::SetStdErrTrashMode(bool trashOutput)
	{
		enum stdwrite_kind swk = stdwrite_kind_err;
		return (::SetStdWriteCB(swk, trashOutput ? 1 : 0, NULL, NULL) == 1) ? true : false;
	}

	static struct UniLoggerSpace * p_ule = NULL;

	bool Unitex::InstallLogger(String^ loggerFolder, bool storeFileOut)
	{
		std::string stdFoldername = clix::marshalString<clix::E_ANSI>(loggerFolder);

		if (p_ule)
			return false;

		p_ule = (struct UniLoggerSpace *)malloc(sizeof(struct UniLoggerSpace));
		if (!p_ule)
			return false;
		memset(p_ule, 0, sizeof(struct UniLoggerSpace));

		p_ule->size_of_struct = sizeof(struct UniLoggerSpace);
		p_ule->privateUnloggerData = NULL;
		p_ule->szPathLog = strdup(stdFoldername.c_str());
		p_ule->szNameLog = NULL;
		p_ule->store_file_out_content = storeFileOut;
		p_ule->store_list_file_out_content = 1;
		p_ule->store_file_in_content = 1;
		p_ule->store_list_file_in_content = 1;
		p_ule->store_std_out_content = 0;
		p_ule->store_std_err_content = 0;
		p_ule->auto_increment_logfilename = 1;

		if (0 != AddActivityLogger(p_ule))
			return true;

		free(p_ule);
		return false;
	}

	bool Unitex::RemoveLogger()
	{
		if (p_ule == NULL)
			return false;
		RemoveActivityLogger(p_ule);
		free((void*)(p_ule->szPathLog));
		free(p_ule);
		p_ule = NULL;
		return true;
	}

	String^ Unitex::LoadPersistentDictionary(String^ filename)
	{
		std::string name = clix::marshalString<clix::E_ANSI>(filename);
		String^ persistentPath = nullptr;

		size_t len_buffer = strlen(name.c_str()) + 0x200;
		char* persistent_filename = (char*)malloc(len_buffer + 1);
		if (persistent_filename == NULL) {
			return nullptr;
		}
		if (standard_load_persistence_dictionary(name.c_str(), persistent_filename, len_buffer)) {
			persistentPath = gcnew String(persistent_filename);
		}
		free(persistent_filename);
		return persistentPath;
	}

	void Unitex::FreePersistentDictionary(String^ filename)
	{
		std::string name = clix::marshalString<clix::E_ANSI>(filename);
		standard_unload_persistence_dictionary(name.c_str());
	}

	String^ Unitex::LoadPersistentFst2(String^ filename)
	{
		std::string name = clix::marshalString<clix::E_ANSI>(filename);
		String^ persistentPath = nullptr;

		size_t len_buffer = strlen(name.c_str()) + 0x200;
		char* persistent_filename = (char*)malloc(len_buffer + 1);
		if (persistent_filename == NULL) {
			return nullptr;
		}
		if (standard_load_persistence_fst2(name.c_str(), persistent_filename, len_buffer)) {
			persistentPath = gcnew String(persistent_filename);
		}
		free(persistent_filename);
		return persistentPath;
	}

	void Unitex::FreePersistentFst2(String^ filename)
	{
		std::string name = clix::marshalString<clix::E_ANSI>(filename);
		standard_unload_persistence_fst2(name.c_str());
	}

	String^ Unitex::LoadPersistentAlphabet(String^ filename)
	{
		std::string name = clix::marshalString<clix::E_ANSI>(filename);
		String^ persistentPath = nullptr;

		size_t len_buffer = strlen(name.c_str()) + 0x200;
		char* persistent_filename = (char*)malloc(len_buffer + 1);
		if (persistent_filename == NULL) {
			return nullptr;
		}
		if (standard_load_persistence_alphabet(name.c_str(), persistent_filename, len_buffer)) {
			persistentPath = gcnew String(persistent_filename);
		}
		free(persistent_filename);
		return persistentPath;
	}

	void Unitex::FreePersistentAlphabet(String^ filename)
	{
		std::string name = clix::marshalString<clix::E_ANSI>(filename);
		standard_unload_persistence_alphabet(name.c_str());
	}
}