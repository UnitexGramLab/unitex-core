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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS)
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.ergonotics.com/unitex-contribution/
 * contact : unitex-contribution@ergonotics.com
 *
 */

/*
 * there is now three possible revision information :
 * SVN_NEW_REVISION : this is the SVN revision number at new hitHub svn https://github.com/UnitexGramLab/unitex-core
 * UNITEX_NEW_SVN_DIFFERENCE = 1632
 * SVN_REVISION was the svn revision from historic https://gforgeigm.univ-mlv.fr/svn/unitex/Unitex-C++
 * SVN_REVISION = SVN_NEW_REVISION + UNITEX_NEW_SVN_DIFFERENCE on version from new github server
 * so "historic" SVN_REVISION value allow compare svn revision number between build made on old and new repository
 *
 */

#include "Unicode.h"
#include "Copyright.h"
#include "UnitexRevisionInfo.h"



#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif


namespace unitex {

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)







#if (SVN_REVISION != -1) && defined(UNITEX_MAJOR_VERSION_NUMBER) && defined (UNITEX_MINOR_VERSION_NUMBER)
const char* UnitexRevisionConstant = "Unitex revision " STRINGIZE(SVN_REVISION) ", Unitex new revision " STRINGIZE(UNITEX_NEW_SVN_REVISION) " Unitex version " STRINGIZE(UNITEX_MAJOR_VERSION_NUMBER) "." STRINGIZE(UNITEX_MINOR_VERSION_NUMBER);
#else
#if defined(UNITEX_MAJOR_VERSION_NUMBER) && defined (UNITEX_MINOR_VERSION_NUMBER)
const char* UnitexRevisionConstant = "Unitex version " STRINGIZE(UNITEX_MAJOR_VERSION_NUMBER) "." STRINGIZE(UNITEX_MINOR_VERSION_NUMBER);
#else
const char* UnitexRevisionConstant = "Unitex version unknown";
#endif
#endif


const char* get_unitex_verbose_version_string() {
	return UnitexRevisionConstant;
}


const char* get_unitex_semver_string() {
	// with this code, you can enter under Windows type UnitexToolLogger.exe | find "semver="
	const char* include_semver_in_binary = "\n\0semver=" UNITEX_SEMVER_STRING "\0\n";
	return include_semver_in_binary + 9;
}

UNITEX_FUNC const char* UNITEX_CALL GetUnitexSemVerString() {
	return get_unitex_semver_string();
}

int get_unitex_revision()
{
	return (int)SVN_REVISION;
}

int get_unitex_new_revision()
{
	return (int)SVN_NEW_REVISION;
}

#ifdef GIT_HEAD_REVISION
	#define GIT_REVISION_STRING GIT_HEAD_REVISION
#else
	#define GIT_REVISION_STRING ""
#endif

const char* get_unitex_core_git_revision()
{
	return GIT_REVISION_STRING;
}


UNITEX_FUNC int UNITEX_CALL GetUnitexRevision()
{
	return get_unitex_revision();
}

UNITEX_FUNC int UNITEX_CALL GetUnitexNewRevision()
{
	return get_unitex_new_revision();
}

UNITEX_FUNC const char* UNITEX_CALL GetUnitexCoreGitRevision()
{
	return get_unitex_core_git_revision();
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
	const char* xmlStringRevision = "\0<UnitexRevision>" STRINGIZE(SVN_REVISION) "</UnitexRevision>\0";
	const char* xmlStringNewRevision = "\0<UnitexNewRevision>" STRINGIZE(SVN_NEW_REVISION) "</UnitexNewRevision>\0";

#ifdef GIT_HEAD_REVISION
	const char* xmlStringGitHeadRevision = "\0<UnitexGitHead>" GIT_HEAD_REVISION "</UnitexGitHead>\0";
#else
	const char* xmlStringGitHeadRevision = "\0<UnitexGitHead></UnitexGitHead>\0";
#endif

#if defined(UNITEX_MAJOR_VERSION_NUMBER) && defined(UNITEX_MINOR_VERSION_NUMBER)
	const char* xmlStringVersion = "\0<UnitexMajorVersion>"  STRINGIZE(UNITEX_MAJOR_VERSION_NUMBER) "</UnitexMajorVersion>" \
		"<UnitexMinorVersion>"  STRINGIZE(UNITEX_MINOR_VERSION_NUMBER) "</UnitexMinorVersion>" \
		"\0";
#else
	const char* xmlStringVersion = "\0<UnitexMajorVersion>x</UnitexMajorVersion>"
		"<UnitexMinorVersion>x</UnitexMinorVersion>\0";
#endif

	const char* xmlStringSemVer = "\0<UnitexSemVer>" UNITEX_SEMVER_STRING "</UnitexSemVer>";

	const char* usableXmlStringRevision = xmlStringRevision + 1;
	const char* usableXmlStringNewRevision = xmlStringNewRevision + 1;
	const char* usableXmlStringGitHead = xmlStringGitHeadRevision + 1;
	const char* usableXmlStringVersion = xmlStringVersion + 1;
	const char* usableXmlStringSemVer = xmlStringSemVer + 1;
	size_t len = strlen(usableXmlStringRevision)+strlen(usableXmlStringNewRevision)+strlen(usableXmlStringGitHead)+
		strlen(usableXmlStringVersion)+strlen(usableXmlStringSemVer)+(5*strlen("\n"));
	if (buflen > len) {
		strcpy(string, usableXmlStringVersion);
		strcat(string, "\n");
		strcat(string, usableXmlStringRevision);
		strcat(string, "\n");
		strcat(string, usableXmlStringNewRevision);
		strcat(string, "\n");
		strcat(string, usableXmlStringGitHead);
		strcat(string, "\n");
		strcat(string, usableXmlStringSemVer);
		strcat(string, "\n");
	}

	return len + 1;
}


size_t get_unitex_version_revision_json_string(char* string, size_t buflen)
{

#ifdef GIT_HEAD_REVISION
	#define JSON_GIT ", \"UnitexGitHead\":\"" GIT_HEAD_REVISION "\""
#else
	#define JSON_GIT ", \"UnitexGitHead\":\"" "\""
#endif

#if defined(UNITEX_MAJOR_VERSION_NUMBER) && defined(UNITEX_MINOR_VERSION_NUMBER) && defined (SVN_REVISION)
	const char* jsonString = "\0{\"UnitexMajorVersion\":" STRINGIZE(UNITEX_MAJOR_VERSION_NUMBER)
		", \"UnitexMinorVersion\":" STRINGIZE(UNITEX_MINOR_VERSION_NUMBER) \
		", \"UnitexRevision\":" STRINGIZE(SVN_REVISION) \
		", \"UnitexNewRevision\":" STRINGIZE(SVN_NEW_REVISION) \
		JSON_GIT \
		", \"UnitexSemVer\":\"" UNITEX_SEMVER_STRING "\"}";
#else
	const char* jsonString = "\0{\"UnitexMajorVersion\":-1, \"UnitexMinorVersion\":-1, \"UnitexRevision\":-1, \"UnitexSemVer\":\"" UNITEX_SEMVER_STRING "\"}";
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
