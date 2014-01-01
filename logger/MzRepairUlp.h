/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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




#if ((!(defined(NO_UNITEX_LOGGER))) && (!(defined(NO_UNITEX_RUNLOGGER_AUTOINSTALL))))

#ifndef _MZ_REPAIR_ULP_LOGGER_H_INCLUDED
#define _MZ_REPAIR_ULP_LOGGER_H_INCLUDED 1

#include "UnitexGetOpt.h"


#ifdef __cplusplus
#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {
extern "C" {
#endif

/* */

extern const char* optstring_MzRepairUlp;
extern const struct option_TS lopts_MzRepairUlp[];
extern const char* usage_MzRepairUlp;

int main_MzRepairUlp(int argc,char* const argv[]);


#ifdef __cplusplus
} // extern "C"
} // namespace logger
} // namespace unitex
#endif

#endif

#endif

