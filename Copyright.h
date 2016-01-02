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


#if (!defined(SVN_REVISION))
#include "Unitex_revision.h"
#define SVN_REVISION UNITEX_REVISION
#endif

#include "Unicode.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/*
 * This is the copyright string that must be displayed by any
 * Unitex program when called with no parameter.
 */
//static unichar COPYRIGHT[256];

#define UNITEX_MAJOR_VERSION_NUMBER 3
#define UNITEX_MINOR_VERSION_NUMBER 1

/**
 * Defines a pre-release label
 * 
 * A pre-release version indicates that the version is unstable and might not 
 * satisfy the intended compatibility requirements as denoted by its associated
 * normal version. Valid labels are:
 * 
 * - "dev"   : in-development release
 * - "alpha" : features completed but its usage is not stable enough
 * - "beta"  : features completed, only minor bugs are expected
 * - "rc"    : release candidate, all minor bugs are fixed, the software works stably
 */
#define UNITEX_VERSION_SUFFIX       "beta"
#define UNITEX_VERSION_IS_UNSTABLE  1

#define UNITEX_IS_BETA

#define UNITEX_HAVE_SYNCTOOL 1


#define STRINGIZE_COPYRIGHT_2(s) #s
#define STRINGIZE_COPYRIGHT(s) STRINGIZE_COPYRIGHT_2(s)
/*
static int init_copyright() {
u_sprintf(COPYRIGHT,"This program is part of Unitex %d.%d%C version\nCopyright %C 2001-2014 Universit%C Paris-Est Marne-la-Vall%Ce\nContact: <unitex@univ-mlv.fr>\n\n",UNITEX_MAJOR_VERSION_NUMBER,UNITEX_MINOR_VERSION_NUMBER,0x3B2,0xA9,0xE9,0xE9);
//u_sprintf(COPYRIGHT,"This program is part of Unitex %d.%d\nCopyright %C 2001-2014 Universit%C Paris-Est Marne-la-Vall%Ce\nContact: <unitex@univ-mlv.fr>\n\n",UNITEX_MAJOR_VERSION_NUMBER,UNITEX_MINOR_VERSION_NUMBER,0xA9,0xE9,0xE9);
return 0;
}*/

#ifdef UNITEX_IS_BETA
#define BETA_UTF8 "\xce\xb2" // unicode 03be
#else
#define BETA_UTF8 "" // unicode 03be
#endif


#define COPYRIGHT_UTF8 "\xc2\xa9" // unicode a9
#define E_ACUTE_UTF8 "\xc3\xa9" // unicode e9


/**
 * @brief Defines a canonical Unitex version string
 * 
 * VERSION = MAJOR.MINOR.PATCH('-'SUFFIX)?
 *  
 * This format is intend to be compliant with the Semantic Versioning scheme 
 * and with the version string of the "Standards for Command Line Interfaces"
 * 
 * @see http://semver.org/spec/v2.0.0.html
 * @see https://www.gnu.org/prep/standards/html_node/Command_002dLine-Interfaces.html
 * @author Cristian Martinez
 */
#if  defined(UNITEX_VERSION_IS_UNSTABLE)    &&\
             UNITEX_VERSION_IS_UNSTABLE
# if  defined(UNITEX_MAJOR_VERSION_NUMBER)  &&\
      defined(UNITEX_MINOR_VERSION_NUMBER)  &&\
     !defined(UNITEX_REVISION_IS_UNDEFINED)
   // MAJOR.MINOR.PATCH-SUFFIX
       #define UNITEX_SEMVER_STRING    STRINGIZE_COPYRIGHT(UNITEX_MAJOR_VERSION_NUMBER) "." \
                                       STRINGIZE_COPYRIGHT(UNITEX_MINOR_VERSION_NUMBER) "." \
                                       STRINGIZE_COPYRIGHT(SVN_REVISION)                "-" \
                                       UNITEX_VERSION_SUFFIX
# elif defined(UNITEX_MAJOR_VERSION_NUMBER) &&\
       defined(UNITEX_MINOR_VERSION_NUMBER)
   // MAJOR.MINOR.0-SUFFIX    
       #define UNITEX_SEMVER_STRING    STRINGIZE_COPYRIGHT(UNITEX_MAJOR_VERSION_NUMBER) "." \
                                       STRINGIZE_COPYRIGHT(UNITEX_MINOR_VERSION_NUMBER) "." \
                                       "0"                                    "-" \
                                       UNITEX_VERSION_SUFFIX
# else     // unknown release as 0.0.0-SUFFIX
       #define UNITEX_SEMVER_STRING   "0.0.0-" UNITEX_VERSION_SUFFIX
# endif  // defined(UNITEX_MAJOR_VERSION_NUMBER)
#else  // UNITEX_VERSION_IS_STABLE
# if  defined(UNITEX_MAJOR_VERSION_NUMBER)  &&\
      defined(UNITEX_MINOR_VERSION_NUMBER)  &&\
     !defined(UNITEX_REVISION_IS_UNDEFINED)
   // MAJOR.MINOR.PATCH
       #define UNITEX_SEMVER_STRING    STRINGIZE_COPYRIGHT(UNITEX_MAJOR_VERSION_NUMBER) "." \
                                       STRINGIZE_COPYRIGHT(UNITEX_MINOR_VERSION_NUMBER) "." \
                                       STRINGIZE_COPYRIGHT(SVN_REVISION) ""
# elif defined(UNITEX_MAJOR_VERSION_NUMBER) &&\
       defined(UNITEX_MINOR_VERSION_NUMBER)
   // MAJOR.MINOR.0    
       #define UNITEX_SEMVER_STRING    STRINGIZE_COPYRIGHT(UNITEX_MAJOR_VERSION_NUMBER) "." \
                                       STRINGIZE_COPYRIGHT(UNITEX_MINOR_VERSION_NUMBER) "." \
                                       "0"
# else     // unknown release as 0.0.0
       #define UNITEX_SEMVER_STRING = "0.0.0"
# endif  // defined(UNITEX_MAJOR_VERSION_NUMBER)  &&
#endif  // defined(UNITEX_VERSION_IS_UNSTABLE)


#define COPYRIGHT_NOTICE \
	"This program is part of Unitex " UNITEX_SEMVER_STRING " version\n" \
	"Copyright " COPYRIGHT_UTF8 " 2001-2016 Universit" E_ACUTE_UTF8 " Paris-Est Marne-la-Vall" E_ACUTE_UTF8 "e\nContact: <unitex@univ-mlv.fr>\n\n"

#define	SIZE_COPYRIGHT_NOTICE_BUFFER 0x400



const char* get_copyright_utf8();

void display_copyright_notice();

void get_copyright_notice(unichar* dest_buffer, size_t size_unichar_dest_buffer);


} // namespace unitex

#endif
