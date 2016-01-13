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
/* ************************************************************************** */
#if (!defined(SVN_REVISION))
# include "Unitex_revision.h"
# define SVN_REVISION UNITEX_REVISION
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
