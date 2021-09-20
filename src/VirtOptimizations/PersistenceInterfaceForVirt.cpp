/*
 * Unitex - Performance optimization code 
 *
 * File created and contributed by Gilles Vollant, working with François Liger
 * as part of an UNITEX optimization and reliability effort, first descibed at
 * http://www.smartversion.com/unitex-contribution/Unitex_A_NLP_engine_from_the_lab_to_the_iPhone.pdf
 *
 * Free software when used with Unitex 3.2 or later
 *
 * Copyright (C) 2021-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "UnitexTool.h"
#include "SyncLogger.h"

#include "AbstractFilePlugCallback.h"
#include "UserCancellingPlugCallback.h"
#include "UniLogger.h"
#include "UniRunLogger.h"

#include "UnitexLibIO.h"
//#include "PersistenceInterface.h"
#ifdef UNITEX_HAVING_MINI_PERSISTANCE
#include "CompressedDic.h"
#include "Fst2.h"
#include "Alphabet.h"
#endif

#include "Copyright.h"

#ifdef HAS_UNITEX_NAMESPACE
using namespace unitex;
#endif

#include "Unicode.h"
#include "DELA.h"
// VersatileEncodingConfigDefined was defined in Unitex near same time than introduce LoadInf.h
#ifdef VersatileEncodingConfigDefined
#include "LoadInf.h"
#endif
#include "AbstractDelaLoad.h"


#ifdef HAS_UNITEX_NAMESPACE
using namespace unitex;
#endif


#include "AbstractDelaPlugCallback.h"


#include "File.h"
#include "UnusedParameter.h"
#include "Error.h"
#include "Af_stdio.h"
#include "AbstractFilePlugCallback.h"


#include "VirtFileType.h"

#include "AbstractDelaLoad.h"

#include "VirtualSpaceManager.h"

#include "VirtDela.h"

#include "MiniMutex.h"
#include "VirtFileSystem.h"
#include "FileTools.h"


#include "Fst2.h"

#include "AbstractFilePlugCallback.h"
#include "AbstractFst2Load.h"
#include "AbstractFst2PlugCallback.h"

#include "VirtFst2.h"

//#include "PersistenceInterface.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

#define NOSTAR_PERSISTENT_RESOURCE 1

namespace unitex {

extern "C" int standard_load_persistence_dictionary(const char*filename,char* persistent_filename_buffer,size_t buffer_size)
{
	int nb_star_to_add;
	if (filename == 0) {
		return 0;
	}

	if ((*filename) == '\0') {
		return 0;
	}

#ifdef NOSTAR_PERSISTENT_RESOURCE
	nb_star_to_add = 0;
#else
	nb_star_to_add = ((*filename) == '*') ? 1 : 2;
#endif

	if (buffer_size <= (strlen(filename) + nb_star_to_add))
		return 0;

	*(persistent_filename_buffer) = '*';
	if (nb_star_to_add == 2) {
		*(persistent_filename_buffer+1) = '*';
	}

	strcpy(persistent_filename_buffer+nb_star_to_add,filename);

	return (LoadVirtualDicFromVirtualFile(filename, persistent_filename_buffer, FALSE)) ? 1 : 0;
}

extern "C" void standard_unload_persistence_dictionary(const char*filename)
{
	DeleteVirtualDic(filename);
}



extern "C" int standard_load_persistence_fst2(const char*filename,char* persistent_filename_buffer,size_t buffer_size)
{
	int nb_star_to_add;
	if (filename == 0) {
		return 0;
	}

	if ((*filename) == '\0') {
		return 0;
	}

#ifdef NOSTAR_PERSISTENT_RESOURCE
	nb_star_to_add = 0;
#else
	nb_star_to_add = ((*filename) == '*') ? 1 : 2;
#endif

	if (buffer_size <= (strlen(filename) + nb_star_to_add))
		return 0;

	*(persistent_filename_buffer) = '*';
	if (nb_star_to_add == 2) {
		*(persistent_filename_buffer+1) = '*';
	}

	strcpy(persistent_filename_buffer+nb_star_to_add,filename);

	return (LoadVirtualGraphFst2FromVirtualFile(filename, persistent_filename_buffer, 1)) ? 1 : 0;
}

extern "C" void standard_unload_persistence_fst2(const char*filename)
{
	DeleteVirtualGraphFst2(filename,1);
}

#ifdef UNITEX_HAVING_MINI_PERSISTANCE


extern "C" int standard_load_persistence_alphabet(const char*filename,char* persistent_filename_buffer,size_t buffer_size)
{

	if (filename == 0) {
		return 0;
	}

	if ((*filename) == '\0') {
		return 0;
	}

	if (buffer_size <= strlen(filename)) {
		return 0;
	}

	strcpy(persistent_filename_buffer, filename);

	return (load_persistent_alphabet(filename)) ? 1 : 0;
}

extern "C" void standard_unload_persistence_alphabet(const char*filename)
{
	free_persistent_alphabet(filename);
}

}
#else


extern "C" int standard_load_persistence_alphabet(const char*filename,char* persistent_filename_buffer,size_t buffer_size)
{
	return 0;
}

extern "C" void standard_unload_persistence_alphabet(const char*filename)
{
}

}

#endif
