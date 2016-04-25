/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef CopyrightH
#define CopyrightH
/* ************************************************************************** */
#include "Version.h"
#include "Unicode.h"


/*
 * Historic svn : https://svnigm.univ-mlv.fr/svn/unitex/Unitex-C++
 * new svn path for github: https://github.com/UnitexGramLab/unitex-core/trunk
 * new git path for unitex-core on github: https://github.com/UnitexGramLab/unitex-core
 */
/* ************************************************************************** */
#if (!defined(SVN_REVISION)) && (!defined(SVN_NEW_REVISION)) && (!defined(GIT_HEAD))
# include "Unitex_revision.h"
#ifdef UNITEX_REVISION
# if UNITEX_REVISION != -1
#  define SVN_REVISION UNITEX_REVISION
# endif
#endif
#ifdef UNITEX_NEW_REVISION
# if UNITEX_NEW_REVISION != -1
#  define SVN_NEW_REVISION UNITEX_NEW_REVISION
# endif
#endif
#ifdef UNITEX_CORE_VERSION_GIT_COMMIT_HASH_STRING
#define GIT_HEAD_REVISION UNITEX_CORE_VERSION_GIT_COMMIT_HASH_STRING
#else
#ifdef UNITEX_CORE_VERSION_GIT_COMMIT_HASH
#define GIT_HEAD_STRING_AND_QUOTE(s) "\""#s"\""
#define GIT_HEAD_REVISION GIT_HEAD_STRING_AND_QUOTE(UNITEX_CORE_VERSION_GIT_COMMIT_HASH)
#endif
#endif
#endif

#define UNITEX_NEW_SVN_DIFFERENCE 1632

//see http://stackoverflow.com/questions/7126329/how-do-i-stringify-macros-that-are-the-results-of-operations-on-macros

#if (defined(SVN_REVISION)) && (!defined(SVN_NEW_REVISION))
#define SVN_NEW_REVISION (SVN_REVISION - UNITEX_NEW_SVN_DIFFERENCE)
#endif


#if (defined(SVN_NEW_REVISION)) && (!defined(SVN_REVISION))
#define SVN_REVISION (SVN_NEW_REVISION + UNITEX_NEW_SVN_DIFFERENCE)
#endif

#if (!defined(SVN_NEW_REVISION)) && (!defined(SVN_REVISION))
#define SVN_REVISION  -1
#define SVN_NEW_REVISION  -1
#endif

/* ************************************************************************** */
#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
// only for backward compatibility
#define UNITEX_MAJOR_VERSION_NUMBER UNITEX_VERSION_MAJOR_NUMBER

// only for backward compatibility
#define UNITEX_MINOR_VERSION_NUMBER UNITEX_VERSION_MINOR_NUMBER

// only for backward compatibility
#define UNITEX_SEMVER_STRING        UNITEX_VERSION_SEMVER

#define UNITEX_HAVE_SYNCTOOL 1

#define STRINGIZE_COPYRIGHT_2(s) #s
#define STRINGIZE_COPYRIGHT(s) STRINGIZE_COPYRIGHT_2(s)
/* ************************************************************************** */
/**
 * @brief Defines a canonical copyright notice
 *
 * This format is intend to be compliant with the Semantic Versioning scheme
 * VERSION = MAJOR.MINOR.PATCH('-'SUFFIX)? and with the information string
 * of the "Standards for Command Line Interfaces"
 *
 * @see http://semver.org/spec/v2.0.0.html
 * @see https://www.gnu.org/prep/standards/html_node/Command_002dLine-Interfaces.html
 * @author Cristian Martinez
 */
#define COPYRIGHT_NOTICE                                           \
   UNITEX_VERSION_NAME      " " UNITEX_VERSION_SEMVER         "\n" \
   UNITEX_VERSION_COPYRIGHT " " UNITEX_VERSION_COMPANY        "\n" \
   "License"                " " UNITEX_VERSION_LICENSE        "\n" \
   "Contact"               " <" UNITEX_VERSION_CONTACT     ">\n\n"

#define SIZE_COPYRIGHT_NOTICE_BUFFER 0x400
/* ************************************************************************** */
const char* get_copyright_utf8();

void display_copyright_notice();

void get_copyright_notice(unichar* dest_buffer, size_t size_unichar_dest_buffer);
/* ************************************************************************** */
} // namespace unitex
/* ************************************************************************** */
#endif
