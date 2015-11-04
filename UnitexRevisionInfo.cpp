/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


#include "Unicode.h"
#include "Copyright.h"
#include "UnitexRevisionInfo.h"


#if (!defined(SVN_REVISION))
#include "Unitex_revision.h"
#define SVN_REVISION UNITEX_REVISION
#endif

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif


namespace unitex {

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#if defined(UNITEX_REVISION) && defined(UNITEX_MAJOR_VERSION_NUMBER) && defined (UNITEX_MINOR_VERSION_NUMBER)
const char* UnitexRevisionConstant = "Unitex revision " STRINGIZE(UNITEX_REVISION) ", Unitex version " STRINGIZE(UNITEX_MAJOR_VERSION_NUMBER) "." STRINGIZE(UNITEX_MINOR_VERSION_NUMBER);
#else
#if defined(UNITEX_MAJOR_VERSION_NUMBER) && defined (UNITEX_MINOR_VERSION_NUMBER)
const char* UnitexRevisionConstant = "Unitex version "STRINGIZE(UNITEX_MAJOR_VERSION_NUMBER) "." STRINGIZE(UNITEX_MINOR_VERSION_NUMBER);
#else
const char* UnitexRevisionConstant = "Unitex version unknown";
#endif
#endif

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
   const char* UNITEX_VERSION_STRING = STRINGIZE(UNITEX_MAJOR_VERSION_NUMBER) "."
                                       STRINGIZE(UNITEX_MINOR_VERSION_NUMBER) "."
                                       STRINGIZE(SVN_REVISION)                "-"
                                       UNITEX_VERSION_SUFFIX;
# elif defined(UNITEX_MAJOR_VERSION_NUMBER) &&\
       defined(UNITEX_MINOR_VERSION_NUMBER)
   // MAJOR.MINOR.0-SUFFIX    
   const char* UNITEX_VERSION_STRING = STRINGIZE(UNITEX_MAJOR_VERSION_NUMBER) "."
                                       STRINGIZE(UNITEX_MINOR_VERSION_NUMBER) "."
                                       "0"                                    "-"
                                       UNITEX_VERSION_SUFFIX;
# else     // unknown release as 0.0.0-SUFFIX
   const char* UNITEX_VERSION_STRING = "0.0.0-" UNITEX_VERSION_SUFFIX
# endif  // defined(UNITEX_MAJOR_VERSION_NUMBER)
#else  // UNITEX_VERSION_IS_STABLE
# if  defined(UNITEX_MAJOR_VERSION_NUMBER)  &&\
      defined(UNITEX_MINOR_VERSION_NUMBER)  &&\
     !defined(UNITEX_REVISION_IS_UNDEFINED)
   // MAJOR.MINOR.PATCH
   const char* UNITEX_VERSION_STRING = STRINGIZE(UNITEX_MAJOR_VERSION_NUMBER) "."
                                       STRINGIZE(UNITEX_MINOR_VERSION_NUMBER) "."
                                       STRINGIZE(SVN_REVISION) "";
# elif defined(UNITEX_MAJOR_VERSION_NUMBER) &&\
       defined(UNITEX_MINOR_VERSION_NUMBER)
   // MAJOR.MINOR.0    
   const char* UNITEX_VERSION_STRING = STRINGIZE(UNITEX_MAJOR_VERSION_NUMBER) "."
                                       STRINGIZE(UNITEX_MINOR_VERSION_NUMBER) "."
                                       "0";
# else     // unknown release as 0.0.0
   const char* UNITEX_VERSION_STRING = "0.0.0";
# endif  // defined(UNITEX_MAJOR_VERSION_NUMBER)  &&
#endif  // defined(UNITEX_VERSION_IS_UNSTABLE)


const char* get_unitex_verbose_version_string() {
	return UnitexRevisionConstant;
}


const char* get_unitex_semver_string() {
	return UNITEX_VERSION_STRING;
}

UNITEX_FUNC const char* UNITEX_CALL GetUnitexSemVerString() {
	return get_unitex_semver_string();
}

int get_unitex_revision()
{
#ifdef SVN_REVISION
return (int)SVN_REVISION;
#else
return (int)-1;
#endif
}


UNITEX_FUNC int UNITEX_CALL GetUnitexRevision()
{
	return get_unitex_revision();
}


void get_unitex_version(unsigned int* major_version_number, unsigned int* minor_version_number)
{
// macro UNITEX_MAJOR_VERSION_NUMBER and UNITEX_MINOR_VERSION_NUMBER were introduced
//   in Unitex 3.0. Unitex 2.1 is the only version compatible with UnitexRevisionInfo without version
#ifdef UNITEX_MAJOR_VERSION_NUMBER
unsigned int unitexMajorVersion = UNITEX_MAJOR_VERSION_NUMBER;
#else
unsigned int unitexMajorVersion = 2;
#endif

#ifdef UNITEX_MINOR_VERSION_NUMBER
unsigned int unitexMinorVersion = UNITEX_MINOR_VERSION_NUMBER;
#else
unsigned int unitexMinorVersion = 1;
#endif

	if (major_version_number != NULL) {
		*major_version_number = unitexMajorVersion;
	}

	if (minor_version_number != NULL) {
		*minor_version_number = unitexMinorVersion;
	}
}


size_t get_unitex_version_revision_xml_string(char* string, size_t buflen)
{
#ifdef SVN_REVISION
	const char* xmlStringRevision = "\0<UnitexRevision>" STRINGIZE(SVN_REVISION) "</UnitexRevision>\0";
#else
	const char* xmlStringRevision = "\0<UnitexVersionInfo>xxxx</UnitexVersionInfo>\0";
#endif

#if defined(UNITEX_MAJOR_VERSION_NUMBER) && defined(UNITEX_MINOR_VERSION_NUMBER)
	const char* xmlStringVersion = "\0<UnitexMajorVersion>"  STRINGIZE(UNITEX_MAJOR_VERSION_NUMBER) "</UnitexMajorVersion>"
		"<UnitexMinorVersion>"  STRINGIZE(UNITEX_MINOR_VERSION_NUMBER) "</UnitexMinorVersion>\0";
#else
	const char* xmlStringVersion = "\0<UnitexMajorVersion>x</UnitexMajorVersion>"
		"<UnitexMinorVersion>x</UnitexMinorVersion>\0";
#endif

	const char* usableXmlStringRevision = xmlStringRevision + 1;
	const char* usableXmlStringVersion = xmlStringVersion + 1;
	size_t len = strlen(usableXmlStringRevision)+strlen(usableXmlStringVersion)+(2*strlen("\n"));
	if (buflen > len) {
		strcpy(string, usableXmlStringVersion);
		strcat(string, "\n");
		strcat(string, usableXmlStringRevision);
		strcat(string, "\n");
	}

	return len + 1;
}


size_t get_unitex_version_revision_json_string(char* string, size_t buflen)
{
#if defined(UNITEX_MAJOR_VERSION_NUMBER) && defined(UNITEX_MINOR_VERSION_NUMBER) && defined (SVN_REVISION)
	const char* jsonString = "\0{\"UnitexMajorVersion\":" STRINGIZE(UNITEX_MAJOR_VERSION_NUMBER)
		",\"UnitexMinorVersion\":" STRINGIZE(UNITEX_MINOR_VERSION_NUMBER) ",\"UnitexRevision\":" STRINGIZE(SVN_REVISION) "}";
#else
	const char* jsonString = "\0{\"UnitexMajorVersion\":-1,\"UnitexMinorVersion\":-1,\"UnitexRevision\":-1}";
#endif

	const char* usableStringRevision = jsonString + 1;

	size_t len = strlen(usableStringRevision);
	if (buflen > len) {
		strcpy(string, usableStringRevision);
	}

	return len + 1;
}


UNITEX_FUNC void UNITEX_CALL GetUnitexVersion(unsigned int* major_version_number, unsigned int* minor_version_number)
{
	char bufXml[0x200];
	get_unitex_version_revision_xml_string(bufXml, sizeof(bufXml));

	get_unitex_version(major_version_number, minor_version_number);
}

} // namespace unitex
