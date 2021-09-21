/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * cristian.martinez@univ-paris-est.fr (martinec)
 *
 */
#ifndef ELGLIB_COMMON_H_
#define ELGLIB_COMMON_H_
/* ************************************************************************** */
// .h source file

/* ************************************************************************** */
// C system files (order the includes alphabetically)

/* ************************************************************************** */
// C++ system files (order the includes alphabetically)

/* ************************************************************************** */
// Other libraries' .h files (order the includes alphabetically)
#include <lua.hpp>
/* ************************************************************************** */
// Project's .h files. (order the includes alphabetically)
/* ************************************************************************** */
#include "ELGLib/debug.h"
#include "base/preprocessor/stringify.h"
/* ************************************************************************** */
#define EXTENSION_ID_ELG          elg
#define EXTENSION_ID_USTRING      ustring
/* ************************************************************************** */
#define EXTENSION_NAME_1(_1)      UNITEX_PP_STRINGIFY_VALUE(UNITEX_PP_TOKEN_PASTE(\
                                  EXTENSION_ID_,_1))
#define EXTENSION_NAME_2(_1,_2)   UNITEX_PP_STRINGIFY_VALUE(UNITEX_PP_TOKEN_PASTE(\
                                  EXTENSION_ID_,_1).UNITEX_PP_TOKEN_PASTE(        \
                                  EXTENSION_ID_,_2))
#define FUNCTION_PREFIX_2(_1,_2)  UNITEX_PP_TOKEN_PASTE(UNITEX_PP_TOKEN_PASTE(    \
                                  UNITEX_PP_TOKEN_PASTE(UNITEX_PP_TOKEN_PASTE(    \
                                  EXTENSION_ID_,_1),_),UNITEX_PP_TOKEN_PASTE(     \
                                  EXTENSION_ID_,_2)),_)
/* ************************************************************************** */
#define U__DECLARE__FUNCTION__ENTRY__(_id,_func) \
  {UNITEX_PP_STRINGIFY_NAME(_func), UNITEX_PP_TOKEN_PASTE(FUNCTION_PREFIX_##_id,_func)}
#define U__DECLARE__FUNCTION__ENTRY__ALIAS__(_id,_func,_alias) \
  {UNITEX_PP_STRINGIFY_NAME(_alias), UNITEX_PP_TOKEN_PASTE(FUNCTION_PREFIX_##_id,_func)}
#define U__DECLARE__GC__ENTRY__(_class)      {"__gc", GCMethod<_class>}
/* ************************************************************************** */
#define ARRAY_LAST_ELEMENT(_array)    (_array + ((sizeof _array/sizeof _array[0]) - 1))
/* ************************************************************************** */
#define LIGHTOBJECT_UnitexString EXTENSION_NAME_2(ELG,USTRING)
/* ************************************************************************** */
// @source http://lua-users.org/wiki/DoItYourselfCppBinding
void* operator new(size_t size, lua_State* L, const char* metatableName);
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
namespace elg {
/* ************************************************************************** */
// @source http://lua-users.org/wiki/DoItYourselfCppBinding
template<typename T>
int GCMethod(lua_State* L) {
  static_cast<T*>(lua_touserdata(L, 1))->~T();
  return 0;
}

// @source http://lua-users.org/wiki/DoItYourselfCppBinding
#define lua_pushobject(L, T) new(L, #T) T

// @source http://lua-users.org/wiki/DoItYourselfCppBinding
#define lua_userdata_cast(L, pos, T) static_cast<T*>(luaL_checkudata((L), (pos), #T))

//
#define lua_pushlightobject(L, T) new(L, LIGHTOBJECT_##T) T

//
#define lua_checkudata_cast(L, pos, T) static_cast<T*>(luaL_checkudata((L), (pos), LIGHTOBJECT_##T))
/* ************************************************************************** */
namespace {   // namespace elg::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
// push a UnitexString into stack
// [-0, +1, m] > (+1)
void pushstring(lua_State* L, UnitexString* value) {
  // buffer used to prepare strings
  luaL_Buffer lb;
  // initialize the buffer
  luaL_buffinit(L, &lb);
  // get the underlying buffer area
  char* cb = luaL_prepbuffer(&lb);
  // encode the UnitexString as a utf-8 string
  int cb_length = value->encode(cb);
  // if length == 0 push empty
  // add to the buffer lb the string copied to the buffer area
  luaL_addsize(&lb, cb_length);
  // leave the final string on the top of the stack
  luaL_pushresult(&lb);
}
/* ************************************************************************** */
void updatefield(lua_State* L, int index, const char* k, UnitexString* value) {
  // pushes onto the stack the string 'value'
  // [-0, +1] > (+2)
  elg::pushstring(L, value);
  elg_stack_dump(L);

  // update field t[k] = new_uvalue
  // [-1, +0] > (+1)
  lua_setfield(L, index, k);
}
/* ************************************************************************** */
void appendtofield(lua_State* L, int index, const char* k, UnitexString* value) {
  // push onto the stack the value t[k]
  // [-0, +1] > (+2)
  lua_getfield(L, index, k);
  elg_stack_dump(L);

  size_t current_length;
  // [-0, +0] > (+2)
  const char* current_value = luaL_checklstring(L, -1, &current_length);
  elg_stack_dump(L);

  UnitexString new_uvalue(current_value, current_length);
  new_uvalue.append(*value);

  // remove t[k] from the stack
  // [-1, +0] > (+1)
  lua_pop(L, 1);
  elg_stack_dump(L);

  // update field t[k] = new_uvalue
  // [+1, -1] > (+1)
  updatefield(L,-2, k, &new_uvalue);
}
/* ************************************************************************** */
}  // namespace unitex::elg::{unnamed
/* ************************************************************************** */
}  // namespace unitex::elg
/* ************************************************************************** */
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif // ELGLIB_COMMON_H_
