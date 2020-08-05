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

#ifndef FileNameH
#define FileNameH


/**
 * This library provides functions for manipulating files and file
 * names. File names should be stored in arrays like:
 *
 *    char filename[FILENAME_MAX];
 *
 * FILENAME_MAX is a system-dependent constant that defines the maximum
 * size of a file name. It is defined in <stdio.h>.
 */
#include <sys/types.h>  // time_t
#include <sys/stat.h>   // stat(), _stat()
#include <errno.h>      // errno
#include <stdio.h>
#include "Unicode.h"

#ifdef _NOT_UNDER_WINDOWS
   #define PATH_SEPARATOR_CHAR    '/'
   #define PATH_SEPARATOR_STRING  "/"
#else
   #define PATH_SEPARATOR_CHAR    '\\'
   #define PATH_SEPARATOR_STRING  "\\"
#endif

#ifdef _NOT_UNDER_WINDOWS
  #include <unistd.h>  // access()
#else
// Exclude some of the less frequently used APIs in Windows.h
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif // !defined(WIN32_LEAN_AND_MEAN)
  #include <windows.h>
  #include <direct.h>  // mkdir()
#endif

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Unitex FileTypes returned by get_file_type() function
 * FUNC_ERROR and FUNC_EACCES are used as function status values.
 * FILE_NOT_FOUND, FILE_ABST, FILE_DIR, FILE_REG and FILE_UNK are
 * available on all the supported operating systems. The remaining
 * types are only available on POSIX-compliant systems
 * @author Cristian Martinez
 */
typedef enum {
  FUNC_ERROR = -3,  // error trying to get the file type
  FUNC_EACCES,      // permission denied trying to get the file type
  FILE_NOT_FOUND,   // file or directory not found
  FILE_ABST,        // Unitex abstract file
  FILE_BLK,         // block device
  FILE_CHR,         // character device (TTY)
  FILE_DIR,         // directory
  FILE_FIFO,        // FIFO (named pipe)
  FILE_LNK,         // symbolic link
  FILE_REG,         // regular file
  FILE_SOCK,        // socket
  FILE_UNK          // unknown file type
} UnitexFileType;

//macros to get status information on a file
//@see http://linux.die.net/man/2/stat
//@see https://msdn.microsoft.com/en-us/library/14h5k7ff.aspx

// bit mask used to extract the file type code from a mode value
#if !defined(S_IFMT)   && defined(_S_IFMT)
#     define S_IFMT               _S_IFMT
#endif
// file type constant of a directory file
#if !defined(S_IFDIR)  && defined(_S_IFDIR)
#     define S_IFDIR              _S_IFDIR
#endif
// file type constant of a character-oriented device file
#if !defined(S_IFCHR)  && defined(_S_IFCHR)
#     define S_IFCHR              _S_IFCHR
#endif
// file type constant of a regular file
#if !defined(S_IFREG)  && defined(_S_IFREG)
#     define S_IFREG              _S_IFREG
#endif
// file type constant of a block-oriented device file
#if !defined(S_IFBLK)  && defined(_S_IFBLK)
#     define S_IFBLK              _S_IFBLK
#endif
// file type constant of a socket
#if !defined(S_IFIFO)  && defined(_S_IFIFO)
#     define S_IFIFO              _S_IFIFO
#endif
// file type constant of a symbolic link
#if !defined(S_IFLNK)  && defined(_S_IFLNK)
#     define S_IFLNK              _S_IFLNK
#endif
// file type constant of a FIFO or pipe
#if !defined(S_IFSOCK) && defined(_S_IFSOCK)
#     define S_IFSOCK             _S_IFSOCK
#endif

// returns non-zero if mode match the file type mask
#if !defined(S_ISTYPE) && defined(S_IFMT)
#     define S_ISTYPE(mode, mask)  (((mode) & S_IFMT) == (mask))
#endif

// returns non-zero if the file is a block special file
#if !defined(S_ISBLK)  &&    defined(S_IFBLK)
#   define   S_ISBLK(m)   S_ISTYPE(m,S_IFBLK)
#endif
// returns non-zero if the file is a character special file
#if !defined(S_ISCHR)  &&    defined(S_IFCHR)
#    define  S_ISCHR(m)   S_ISTYPE(m,S_IFCHR)
#endif
// returns non-zero if the file is a directory
#if !defined(S_ISDIR)  &&    defined(S_IFDIR)
#    define  S_ISDIR(m)   S_ISTYPE(m,S_IFDIR)
#endif
// returns non-zero if the file is a regular file
#if !defined(S_ISREG)  &&    defined(S_IFREG)
#    define  S_ISREG(m)   S_ISTYPE(m,S_IFREG)
#endif
// returns non-zero if the file is a FIFO special file, or a pipe
#if !defined(S_ISFIFO) &&    defined(S_IFIFO)
#    define  S_ISFIFO(m)  S_ISTYPE(m,S_IFIFO)
#endif
// returns non-zero if the file is a symbolic link
#if !defined(S_ISLNK)  &&    defined(S_IFLNK)
#    define  S_ISLNK(m)   S_ISTYPE(m,S_IFLNK)
#endif
// returns non-zero if the file is a socke
#if !defined(S_ISSOCK) &&    defined(S_IFSOCK)
#    define  S_ISSOCK(m)  S_ISTYPE(m,S_IFSOCK)
#endif

// under Visual Studio use _stati64 and _fstati64
// @see https://msdn.microsoft.com/en-us/library/14h5k7ff.aspx
#ifdef _MSC_VER
// _stati64  struct and _stati64()  instead of stat and stat()
# undef  stat
# define stat  _stati64
// _fstati64 struct and _fstati64() instead of stat and fstat()
# undef  fstat
# define fstat _fstati64
#endif  // _MSC_VER

// under windows use _access() as access()
// @see https://msdn.microsoft.com/en-us/library/1w06ktdy.aspx
#if !defined(_NOT_UNDER_WINDOWS)
#if defined(__MINGW32__) || defined(__MINGW64__)
// Let MINGW ignore X_OK flag and
// defines access() as __mingw_access()
#  define __USE_MINGW_ACCESS 1
# endif  // defined(__MINGW32__) || defined(__MINGW64__)

# include <io.h>  // access()

// only if access isn't already defined
# if !defined(__USE_MINGW_ACCESS)
#  define access(file, mode) _access(file,mode)
# endif  // !defined(__USE_MINGW_ACCESS)

// R_OK
# if !defined(R_OK)
#  define R_OK  4  // Read-only
# endif  // !defined(R_OK)
// W_OK
# if !defined(W_OK)
#  define W_OK  2  // Write-only
# endif  // !defined(W_OK)
// X_OK
# if !defined(X_OK)
#  define X_OK  1  // Execute only
# endif  // !defined(X_OK)
// F_OK
# if !defined(F_OK)
#  define F_OK  0  // Existence only
# endif  // !defined(F_OK)
#endif  // _NOT_UNDER_WINDOWS

void add_suffix_to_file_name(char*,const char*,const char*);
void add_prefix_to_file_name(char*,const char*,const char*);
void get_extension(const char*,char*);
void remove_extension(char*);
void remove_extension(const char*,char*);
void get_path(const char*,char*);
char* to_windows_path_separators(char* filename);
char* to_unix_path_separators(char* filename);
char* to_native_path_separators(char* filename);
int get_real_path(const char* filename, char* resolved_name);
UnitexFileType get_file_type(const char* filename);
void get_snt_path(const char*,char*);
const char* filename_without_path(const char* filename);
void remove_path(const char*,char*);
void remove_path_and_extension(const char*,char*);
void replace_path_separator_by_colon(char*);
void replace_colon_by_path_separator(char*);
void new_file(const char*,const char*,char*);
void copy_file(const char*,const char*);
int fexists(const char*);
int file_exists(const char* filename);
time_t get_file_date(const char* name);
long get_file_size(const char*);
long get_file_size(U_FILE*);
int add_path_separator(char*);
int is_absolute_path(const char*);
int is_root(const char*);
int is_regular_file(const char* filename);
int is_abstract_file(const char* filename);
int is_unknown_file(const char* filename);
int is_directory(const char* file_path);
int is_character_file(const char* filename);
int create_path_to_file(const char*);

} // namespace unitex

#endif
