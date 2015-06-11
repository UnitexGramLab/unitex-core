// UnitexNet.h

#pragma once

#include <msclr/marshal_cppstd.h>

#include "Copyright.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace UnitexNet {

	char** argsFromManagedString(String^ string);
	char** argsFromManagedStringArray(array<String^>^ strArray);
	int countArgs(char** args);
	void freeArgs(char** args);

	[ComVisible(true)]
	[Guid("7B4478B3-4E94-4D24-8548-922C78D1E67C")]
	[InterfaceType(ComInterfaceType::InterfaceIsDual)]
	public interface class IUnitex {
		int ExecUnitexTool(array<String^>^ cmdArray);
		int ExecUnitexTool(String^ cmdLine);
		int ExecRunLog(array<String^>^ cmdArray);
		int ExecRunLog(String^ cmdLine);
		property bool UsingOffsetFile
		{
			bool get();
		}
		property int NumberAbstractFileSpaceInstalled
		{
			int get();
		}
		bool WriteUnitexFile(String^ fileName, String^ fileContent);
		bool WriteUnitexFile(String^ filename, array<Byte>^ byteArray);
		bool WriteUnitexFile(String^ filename, array<Char>^ charArray);
		bool WriteUnitexFileUtf(String^ fileName, String^ fileContent);
		bool WriteUnitexFileUtf(String^ fileName, String^ fileContent, bool withBom);
		String^ GetUnitexFileString(String^ fileName);
		array<Byte>^ GetUnitexFileData(String^ filename);
		array<Char>^ GetUnitexFileDataChar(String^ filename);
		bool RemoveUnitexFile(String^ fileName);
		bool CreateUnitexFolder(String^ folderName);
		bool RemoveUnitexFolder(String^ folderName);
		bool RenameUnitexFile(String^ fileNameSrc, String^ fileNameDst);
		bool CopyUnitexFile(String^ fileNameSrc, String^ fileNameDst);
		bool UnitexAbstractPathExists(String^ path);
		array<String^>^ GetFileList(String^ path);
		bool SetStdOutTrashMode(bool trashOutput);
		bool SetStdErrTrashMode(bool trashOutput);
		bool InstallLogger(String^ loggerFolder, bool storeFileOut);
		bool RemoveLogger();
		String^ LoadPersistentDictionary(String^ filename);
		void FreePersistentDictionary(String^ filename);
		String^ LoadPersistentFst2(String^ filename);
		void FreePersistentFst2(String^ filename);
		String^ LoadPersistentAlphabet(String^ filename);
		void FreePersistentAlphabet(String^ filename);
		property int MajorVersionNumber
		{
			int get();
		}
		property int MinorVersionNumber
		{
			int get();
		}
		property int SvnRevisionNumber
		{
			int get();
		}
	};

	[ComVisible(true)]
	[Guid("025DB086-E3CF-42F7-A17B-1E598B912558")]
	public ref class Unitex : IUnitex
	{

	public:
		/// <summary>
		/// Function to run UnitexTool with string or string array, like java exec in
		/// java runtime.
		///
		/// You can combine several tool using { }
		/// (see UnitexTool in Unitex manual for more information).
		/// </summary>
		/// <returns>The return value of the tools (0 for success).</returns>
		virtual int ExecUnitexTool(array<String^>^ cmdArray);

		/// <summary>
		/// Function to run UnitexTool with string or string array, like java exec in
		/// java runtime.
		///
		/// You can combine several tool using { }
		/// (see UnitexTool in Unitex manual for more information).
		/// </summary>
		/// <example>
		/// <code>
		/// UnitexWrapper.ExecUnitexTool("UnitexTool Normalize \"corpus.txt\" -r \"Norm.txt\"");
		/// 
		/// UnitexWrapper.ExecUnitexTool("UnitexTool Tokenize \"corpus.txt\" -a \"Alphabet.txt\"");
		///
		/// UnitexWrapper.ExecUnitexTool("UnitexTool { Normalize \"corpus.txt\" -r \"Norm.txt\" } { Tokenize \"corpus.txt\" -a \"Alphabet.txt\" }");
		/// </code>
		/// </example>
		/// <returns>The return value of the tools (0 for success).</returns>
		virtual int ExecUnitexTool(String^ cmdLine);

		/// <summary>
		/// Function to run RunLog with string or string array, like java exec in
		/// java runtime.
		/// </summary>
		/// <example>
		/// <code>
		/// string[] strArrayCmds = new[]{ "RunLog", "mylog.ulp" };
		/// UnitexWrapper.ExecRunLog(strArrayCmds);
		/// </code>
		/// </example>
		/// <returns>The return value of the tools (0 for success).</returns>
		virtual int ExecRunLog(array<String^>^ cmdArray);

		/// <summary>
		/// Function to run RunLog with string or string array, like java exec in
		/// java runtime.
		/// </summary>
		/// <example>
		/// <code>
		/// UnitexLibAndJni.execRunLog("RunLog mylog.ulp");
		/// </code>
		/// </example>
		/// <returns>The return value of the tools (0 for success).</returns>
		virtual int ExecRunLog(String^ cmdLine);

		/// <summary>
		/// Tells whether this version is compatible with Unitex offset files.
		/// </summary>
		property bool UsingOffsetFile 
		{
			virtual bool get() {
#ifdef UNITEX_HAVING_OFFSET_FILE
				return true;
#endif
#ifdef UNITEX_REVISION
				return (UNITEX_REVISION >= 2292) ? true : false;
#endif
				return false;
			}
		}

		/// <summary>
		/// Tells how many abstract file systems are installed.
		/// </summary>
		property int NumberAbstractFileSpaceInstalled
		{
			virtual int get();
		}

		/// <summary>
		/// Creates a file from a string using UTF16LE encoding with BOM (native
		/// Unitex format).
		/// <summary>
		virtual bool WriteUnitexFile(String^ fileName, String^ fileContent);

		/// <summary>
		/// Creates a file from a managed byte array.
		/// <summary>
		/// <paramref name="filename">The destination file path.</paramref>
		/// <paramref name="byteArray">The managed bytes array.</paramref>
		virtual bool WriteUnitexFile(String^ filename, array<Byte>^ byteArray);

		/// <summary>
		/// Creates a file from a managed char array.
		/// <summary>
		/// <paramref name="filename">The destination file path.</paramref>
		/// <paramref name="charArray">The managed char array.</paramref>
		virtual bool WriteUnitexFile(String^ filename, array<Char>^ charArray);

		/// <summary>
		/// Creates a file from a string using UTF8 encoding without BOM.
		/// <summary>
		virtual bool WriteUnitexFileUtf(String^ fileName, String^ fileContent);

		/// <summary>
		/// Creates a file from a string using UTF8 encoding with or without BOM.
		/// <summary>
		virtual bool WriteUnitexFileUtf(String^ fileName, String^ fileContent, bool withBom);

		/// <summary>
		/// Reads and decodes a file to a string.
		/// </summary>
		virtual String^ GetUnitexFileString(String^ fileName);

		/// <summary>
		/// Gets the contents of a file as a byte array.
		/// </summary>
		/// <paramref name="filename">The source file path.</paramref>
		virtual array<Byte>^ GetUnitexFileData(String^ filename);

		/// <summary>
		/// Gets the contents of a file as a char array.
		/// </summary>
		/// <paramref name="filename">The source file path.</paramref>
		virtual array<Char>^ GetUnitexFileDataChar(String^ filename);

		/// <summary>
		/// Removes a file.
		/// </summary>
		virtual bool RemoveUnitexFile(String^ fileName);

		/// <summary>
		/// Creates a folder if needed.
		/// </summary>
		virtual bool CreateUnitexFolder(String^ folderName);

		/// <summary>
		/// Removes a folder.
		/// </summary>
		virtual bool RemoveUnitexFolder(String^ folderName);

		/// <summary>
		/// Renames a file.
		/// </summary>
		virtual bool RenameUnitexFile(String^ fileNameSrc, String^ fileNameDst);
		
		/// <summary>
		/// Copies a file.
		/// </summary>
		virtual bool CopyUnitexFile(String^ fileNameSrc, String^ fileNameDst);

		/// <summary>
		/// Tests whether a path is already present in Unitex's abstract file space.
		/// </summary>
		virtual bool UnitexAbstractPathExists(String^ path);

		/// <summary>
		/// Retrieve array of file in abstract space.
		/// </summary>
		virtual array<String^>^ GetFileList(String^ path);

		/// <summary>
		/// Allows ignore or emit normal message to stdout.
		/// </summary>
		virtual bool SetStdOutTrashMode(bool trashOutput);

		/// <summary>
		/// Allows ignore or emit normal message to stderr.
		/// </summary>
		virtual bool SetStdErrTrashMode(bool trashOutput);

		/// <summary>
		/// Enables logger and write log file on loggerFolder if storeFileOut is TRUE,
		/// log will contain destination file
		/// </summary>
		virtual bool InstallLogger(String^ loggerFolder, bool storeFileOut);

		/// <summary>
		/// Disables the logger (after calling InstallLogger).
		/// </summary>
		virtual bool RemoveLogger();

		/// <summary>
		/// Loads a persistent dictionary in memory.
		/// </summary>
		virtual String^ LoadPersistentDictionary(String^ filename);

		/// <summary>
		/// Frees a persistent dictionary in memory.
		/// </summary>
		virtual void FreePersistentDictionary(String^ filename);

		/// <summary>
		/// Loads a persistent fst2 compiled automaton in memory.
		/// </summary>
		virtual String^ LoadPersistentFst2(String^ filename);

		/// <summary>
		/// Frees a persistent fst2 compiled automaton in memory.
		/// </summary>
		virtual void FreePersistentFst2(String^ filename);

		/// <summary>
		/// Loads a persistent alphabet in memory.
		/// </summary>
		virtual String^ LoadPersistentAlphabet(String^ filename);

		/// <summary>
		/// Frees a persistent alphabet in memory.
		/// </summary>
		virtual void FreePersistentAlphabet(String^ filename);

		/// <summary>
		/// The major Unitex version number.
		/// </summary>
		property int MajorVersionNumber
		{
			virtual int get()
			{
#ifdef UNITEX_MAJOR_VERSION_NUMBER
				return UNITEX_MAJOR_VERSION_NUMBER;
#else
				return (2;
#endif
			}
		}

		/// <summary>
		/// The minor Unitex version number.
		/// </summary>
		property int MinorVersionNumber
		{
			virtual int get()
			{
#ifdef UNITEX_MINOR_VERSION_NUMBER
				return UNITEX_MINOR_VERSION_NUMBER;
#else
				return 1;
#endif
			}
		}

		/// <summary>
		/// the SVN revision number of the C / C++ code or - 1 if this
		/// information was not provided at compile time.
		/// </summary>
		property int SvnRevisionNumber
		{
			virtual int get()
			{
#ifdef SVN_REVISION
				return SVN_REVISION;
#else
				return -1;
#endif
			}
		}
	};
}
