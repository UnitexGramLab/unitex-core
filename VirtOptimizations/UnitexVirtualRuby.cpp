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
/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * https://github.com/ergonotics/JNI-for-Unitex-2.1
 * contact : unitex-contribution@ergonotics.com
 *
 */

// Include the Ruby headers and goodies
#include "ruby.h"

#ifndef RUBY_VINTAGE
#include "ruby/encoding.h"
#endif

#include "UnitexRuby.h"


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


#ifdef USE_UNITEXVIRTUAL_HEADER

#include "VirtFileType.h"
#include "VirtualSpaceManager.h"
#include "Fst2.h"
#include "VirtFst2.h"
#include "VirtDela.h"


 
ULB_VFFUNC BOOL ULIB_CALL SetVirtualFst2AutoLoad(BOOL);
 
ULB_VFFUNC BOOL ULIB_CALL GetVirtualFst2AutoLoad();
 
ULB_VFFUNC BOOL ULIB_CALL SetVirtualDicAutoLoad(BOOL);
 
ULB_VFFUNC BOOL ULIB_CALL GetVirtualDicAutoLoad();

#else

#ifdef __cplusplus
extern "C" {
#endif
    
UNITEX_FUNC int UNITEX_CALL SetVirtualFst2AutoLoad(int);

UNITEX_FUNC int UNITEX_CALL GetVirtualFst2AutoLoad();

UNITEX_FUNC int UNITEX_CALL SetVirtualDicAutoLoad(int);

UNITEX_FUNC int UNITEX_CALL GetVirtualDicAutoLoad();

}

#endif

VALUE method_GetVirtualFst2AutoLoad(VALUE self)
{
    return GetVirtualFst2AutoLoad() ? Qtrue : Qfalse;
}



VALUE method_SetVirtualFst2AutoLoad(VALUE self, VALUE isSet)
{
    int setParam = RTEST(isSet);
	return SetVirtualFst2AutoLoad(setParam ? 1 : 0) ? Qtrue : Qfalse;;
}



VALUE method_GetVirtualDicAutoLoad(VALUE self)
{
    return GetVirtualDicAutoLoad() ? Qtrue : Qfalse;
}



VALUE method_SetVirtualDicAutoLoad(VALUE self, VALUE isSet)
{
    int setParam = RTEST(isSet);
	return SetVirtualDicAutoLoad(setParam ? 1 : 0) ? Qtrue : Qfalse;;
}




void ABSTRACT_CALLBACK_UNITEX unitex_ruby_initializer_virtual(VALUE UnitexRuby,void* privateSpacePtr)
{
    typedef VALUE (*t_ruby_method)(ANYARGS);

    rb_define_method(UnitexRuby, "setVirtualFst2AutoLoad", (t_ruby_method) &method_SetVirtualFst2AutoLoad, 1);
    rb_define_method(UnitexRuby, "getVirtualFst2AutoLoad", (t_ruby_method) &method_GetVirtualFst2AutoLoad, 0);
    
    rb_define_method(UnitexRuby, "setVirtualDicAutoLoad", (t_ruby_method) &method_SetVirtualDicAutoLoad, 1);
    rb_define_method(UnitexRuby, "getVirtualDicAutoLoad", (t_ruby_method) &method_GetVirtualDicAutoLoad, 0);
}


t_unitex_ruby_initializer_func_array func_array =
{
    0,
    &unitex_ruby_initializer_virtual
};

class autoinstall_ruby_initializer_virtual {
    public:
    autoinstall_ruby_initializer_virtual() {
        AddUnitexRubyInitializer(&func_array,this);
    };
    
    ~autoinstall_ruby_initializer_virtual() {
        RemoveUnitexRubyInitializer(&func_array,this);
    };
    
} ;

autoinstall_ruby_initializer_virtual autoinstall_ruby_initializer_virtual_instance;

