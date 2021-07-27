/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * contact : unitex-contribution@ergonotics.com
 *
 */
package fr.umlv.unitex.jni;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * UnitexJni is the import java class for Unitex JNI libary
 * 
 * @author Gilles Vollant (Ergonotics SAS) <br>
 *         created and contributed by Gilles Vollant (Ergonotics SAS) as part of
 *         an UNITEX optimization and reliability effort <br>
 *         https://github.com/ergonotics/JNI-for-Unitex-2.1
 *         http://www.ergonotics.com/unitex-contribution/
 * 
 *         Modified by Sébastien Paumier
 */
public class UnitexJni {
	/* Load the .dll/.so that exports functions callable from java */
	public static boolean loaded;
	static {
		try {
			System.loadLibrary("UnitexJni");
			loaded=true;
		} catch (UnsatisfiedLinkError e) {
			loaded=false;
		}
	}

	/*
	 * function imported from Jni
	 */

	/**
	 * Function to run UnitexTool with string or string array, like java exec in
	 * java runtime <br>
	 * you can combine several tool using { } <br>
	 * (see UnitexTool in Unitex manual for more information)<br>
	 * <code>
	 * String [] strArrayCmds={"UnitexTool","{","Normalize","corpus.txt", "-r", "Norm.txt","}","{","Tokenize","corpus.txt", "-r", "Alphabet.txt","}"};
	 * <br>
	 * UnitexLibAndJni.execUnitexTool(strArrayCmds);
     * </code>
	 * 
	 * @return value : the return value of the tools (0 for success)
	 */
	public native static int execUnitexTool(String[] cmdarray);

	/**
	 * Function to run UnitexTool with string or string array, like java exec in
	 * java runtime <br>
	 * you can combine several tool using { } <br>
	 * (see UnitexTool in Unitex manual for more information)<br>
	 * <code>
	 * UnitexLibAndJni.execUnitexTool("UnitexTool Normalize \"corpus.txt\" -r \"Norm.txt\"");
	 * <br>
	 * UnitexLibAndJni.execUnitexTool("UnitexTool Tokenize \"corpus.txt\" -a \"Alphabet.txt\"");
	 * <br>
	 * UnitexLibAndJni.execUnitexTool("UnitexTool { Normalize \"corpus.txt\" -r \"Norm.txt\" } { Tokenize \"corpus.txt\" -a \"Alphabet.txt\" }");
     * </code>
	 * 
	 * @return value : the return value of the tools (0 for success)
	 */
	public native static int execUnitexTool(String cmdline);


	/**
	 * Function to run RunLog with string or string array, like java exec in
	 * java runtime <br>
	 * <code>
	 * String [] strArrayCmds={"RunLog","mylog.ulp"};
	 * <br>
	 * UnitexLibAndJni.execRunLog(strArrayCmds);
     * </code>
	 * 
	 * @return value : the return value of the tools (0 for success)
	 */
	public native static int execRunLog(String[] cmdarray);

	/**
	 * Function to run RunLog with string or string array, like java exec in
	 * java runtime <br>
	 * <code>
	 * UnitexLibAndJni.execRunLog("RunLog mylog.ulp");
     * </code>
	 * 
	 * @return value : the return value of the tools (0 for success)
	 */
	public native static int execRunLog(String cmdline);

	/**
	 * function to check the compatibility of Unitex with offset file
	 * 
	 * @return true for version of Unitex which uses file offset (stating Unitex
	 *         3.0)
	 */
	public native static boolean usingOffsetFile();

	/**
	 * function to known how many abstract file system are installed
	 * 
	 * @return the number of Abstract file system installed in Unitex
	 */
	public native static int numberAbstractFileSpaceInstalled();

	/*
	 * file function below are compatible with both disk file system and
	 * abstract file system
	 */
	/**
	 * writeUnitexFile* function create file to be used by Unitex.
	 */
	/**
	 * create a file from a raw binary char array
	 */
	public native static boolean writeUnitexFile(String fileName,
			char[] fileContent);

	/**
	 * create a file from a raw binary byte array
	 */
	public native static boolean writeUnitexFile(String fileName,
			byte[] fileContent);

	/**
	 * create a file from a string using UTF16LE encoding with BOM (native
	 * Unitex format)
	 */
	public native static boolean writeUnitexFile(String fileName,
			String fileContent);

	/**
	 * create a file from a string using UTF8 encoding without BOM
	 */
	public native static boolean writeUnitexFileUtf(String fileName,
			String fileContent);

	/**
	 * create a file from a string using UTF8 encoding with or without BOM
	 */
	public native static boolean writeUnitexFileUtf(String fileName,
			String fileContent, boolean isBom);

	/**
	 * append to a file a raw binary byte array
	 */
	public native static boolean appendUnitexFile(String fileName,
			byte[] fileContent);

	/**
	 * read a file to a raw binary char array representation
	 */
	public native static char[] getUnitexFileDataChar(String fileName);

	/**
	 * read a file to a raw binary byte array representation
	 */
	public native static byte[] getUnitexFileData(String fileName);

	/**
	 * read and decode a file to a string.
	 */
	public native static String getUnitexFileString(String fileName);

	/**
	 * remove a file
	 */
	public native static boolean removeUnitexFile(String fileName);

	/**
	 * create a folder, if needed
	 */
	public native static boolean createUnitexFolder(String folderName);

	/**
	 * remove a folder and the folder content
	 */
	public native static boolean removeUnitexFolder(String folderName);

	/**
	 * rename a file
	 */
	public native static boolean renameUnitexFile(String fileNameSrc,
			String fileNameDst);

	/**
	 * copy a file
	 */
	public native static boolean copyUnitexFile(String fileNameSrc,
			String fileNameDst);
			
	/**
	 * tests whether a path is already present in Unitex's abstact file space
	 */
	public native static boolean unitexAbstractPathExists(String path);

	/**
	 * retrieve array of file in abstract space
	 */
	public native static String[] getFileList(String path);

	/**
	 * allow ignore (flushMode is TRUE) or emit normal message to stdout
	 */
	public native static boolean setStdOutTrashMode(boolean flushMode);

	/**
	 * allow ignore (flushMode is TRUE) or emit error message to stderr
	 */
	public native static boolean setStdErrTrashMode(boolean flushMode);

	/**
	 * function to enable or disable logger (creating .ULP log file when run an
	 * Unitex tool) logger function must NOT be used when another thread uses a
	 * function from this JNI
	 */
	/**
	 * enable logger and write log file on loggerFolder if storeFileOut is TRUE,
	 * log will contain destination file
	 */
	public native static boolean installLogger(String loggerFolder,
			boolean storeFileOut);

	/**
	 * disable the logger (after calling installLogger)
	 */
	public native static boolean removeLogger();

	/**
	 * function to check if running under an edition of Microsoft Windows
	 */
	public static boolean isUnderWindows() {
		String os = System.getProperty("os.name").toLowerCase();
		return (os.indexOf("win") >= 0);
	}

	/**
	 * function to get the path separator of current operating system
	 */
	public static final String pathSeparator = java.io.File.separator;

	/**
	 * combine several path items and a filename to create a full filename
	 */
	public static final String combineUnitexFileComponent(
			String... FilePathNameItems) {
		String retStr = "";
		String pSep = pathSeparator;
		boolean isFirst = true;
		for (String strItem : FilePathNameItems) {
			if (isFirst)
				retStr = strItem;
			else
				retStr = retStr + pSep + strItem;
			isFirst = false;
		}
		return retStr;
	}

	/**
	 * combine several path items and a filename to create a full filename with
	 * quote
	 */
	public static final String combineUnitexFileComponentWithQuote(
			String... FilePathNameItems) {
		String retStr = "";
		String pSep = pathSeparator;
		boolean isFirst = true;
		for (String strItem : FilePathNameItems) {
			if (isFirst)
				retStr = strItem;
			else
				retStr = retStr + pSep + strItem;
			isFirst = false;
		}
		return "\"" + retStr + "\"";
	}
	
	/**
	 * Note: The persistence mechanism uses file name comparisons
	 *       to detect that a file has already been loaded. So, you
	 *       must use the exact same file name returned by load function,
	 *       even if your system is case tolerant
	 */
	public native static String loadPersistentDictionary(String filename);
	public native static void freePersistentDictionary(String filename);
	public native static String loadPersistentFst2(String filename);
	public native static void freePersistentFst2(String filename);
	public native static String loadPersistentAlphabet(String filename);
	public native static void freePersistentAlphabet(String filename);


	/**
	 * Returns the major Unitex version number
	 */
	public native static int getMajorVersionNumber();

	/**
	 * Returns the minor Unitex version number
	 */
	public native static int getMinorVersionNumber();

	/**
	 * Returns the SVN revision number of the C/C++ code or -1 if this
	 * information was not provided at compile time.
	 */
	public native static int getSvnRevisionNumber();
	
	public static boolean isJniLoaded() {
		return loaded;
	}
}
