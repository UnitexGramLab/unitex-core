/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "RegExFacade.h"

#ifdef TRE_WCHAR
#define REGEX_FACADE_ENGINE 1

#define REGEX_FACADE_REG_NOSUB REG_NOSUB
#define REGEX_FACADE_REG_EXTENDED REG_EXTENDED

typedef tre_wchar_t unichar_regex;
typedef regex_t regex_facade_regex_t;
typedef regmatch_t regex_regmatch_t ;

int regex_facade_regwcomp(regex_facade_regex_t *preg, const unichar_regex *regex, int cflags)
{
	return tre_regwcomp(preg, regex, cflags);
}

size_t regex_facade_regerror(int errcode, const regex_facade_regex_t *preg, char *errbuf,
	 size_t errbuf_size)
{
	return tre_regerror(errcode, preg, errbuf, errbuf_size);
}

void regex_facade_regfree(regex_facade_regex_t *preg)
{
	tre_regfree(preg);
}

int regex_facade_regwexec(const regex_facade_regex_t *preg, const unichar_regex *string,
	 size_t nmatch, regex_regmatch_t pmatch[], int eflags)
{
	return tre_regwexec(preg, string, nmatch, pmatch, eflags);
}
#endif
