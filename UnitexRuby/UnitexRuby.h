/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * additional information: http://www.smartversion.com/unitex-contribution/
 * https://github.com/ergonotics/JNI-for-Unitex-2.1
 * contact : info@winimage.com
 *
 */


#ifdef UNITEX_RUBY_EXTENSION

// Include the Ruby headers and goodies


#include "AbstractCallbackFuncModifier.h"

#ifdef __cplusplus
extern "C" {
#endif
    
// using the same extension model than AbstractFilePlugCallback.h
//  uses it to add custom function in UnitexRuby
    
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_unitex_ruby_initializer)(VALUE module,void* privateSpacePtr);
    
    
typedef struct
{
    size_t size_func_array;
    t_fnc_unitex_ruby_initializer fnc_unitex_ruby_initializer;
} t_unitex_ruby_initializer_func_array;

UNITEX_FUNC int UNITEX_CALL AddUnitexRubyInitializer(const t_unitex_ruby_initializer_func_array* func_array_param,void* privateSpacePtr);

UNITEX_FUNC int UNITEX_CALL RemoveUnitexRubyInitializer(const t_unitex_ruby_initializer_func_array* func_array_param,void* privateSpacePtr);

UNITEX_FUNC void Init_unitexruby();
    
#ifdef __cplusplus
}
#endif

#endif
