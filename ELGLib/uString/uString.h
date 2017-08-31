/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#ifndef USTRING_H_
#define USTRING_H_
/* ************************************************************************** */
// .h source file

/* ************************************************************************** */
// C system files (order the includes alphabetically)

/* ************************************************************************** */
// C++ system files (order the includes alphabetically)

/* ************************************************************************** */
// Other libraries' .h files (order the includes alphabetically)

/* ************************************************************************** */
// Project's .h files. (order the includes alphabetically)
#include "ELGLib/common.h"
#include "UnitexString.h"
/* ************************************************************************** */
#define EXTENSION_NAME_USTRING        EXTENSION_NAME_2(ELG, USTRING)
#define FUNCTION_PREFIX_USTRING       FUNCTION_PREFIX_2(ELG, USTRING)
#define EXTENSION_VERSION_USTRING     "0.1.0"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
namespace elg {
/* ************************************************************************** */
namespace ustring {
/* ************************************************************************** */
namespace {   // namespace elg::ustring::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
// all U__* macros must be undefined before the end of this file
#define U__DECLARE__FUNCTION__ELG__USTRING__(_func)                         \
/* static */ int elg_ustring_##_func(lua_State* L) {                        \
  UnitexString* str = lua_lightobject_cast(L, 1, UnitexString);             \
  if (str->is_attached()) {                                                 \
    lua_pushlightobject(L, UnitexString)(str->_func());                     \
  } else {                                                                  \
    (lua_pushlightobject(L, UnitexString)(*str))->_func();                  \
  }                                                                         \
  return 1;                                                                 \
}
/* ************************************************************************** */
U__DECLARE__FUNCTION__ELG__USTRING__(deaccentuate);
U__DECLARE__FUNCTION__ELG__USTRING__(fold);
U__DECLARE__FUNCTION__ELG__USTRING__(lower);
U__DECLARE__FUNCTION__ELG__USTRING__(title);
U__DECLARE__FUNCTION__ELG__USTRING__(upper);
/* ************************************************************************** */
#define U__DECLARE__FUNCTION__ELG__USTRING__INT__(_func)                         \
/* static */ int elg_ustring_##_func(lua_State* L) {                        \
  UnitexString* str = lua_lightobject_cast(L, 1, UnitexString);             \
  lua_pushinteger(L, (lua_Integer) str->_func());                                                                       \
  return 1;                                                                 \
}
/* ************************************************************************** */
U__DECLARE__FUNCTION__ELG__USTRING__INT__(length);

/* ************************************************************************** */
/* static */ int elg_ustring_encode(lua_State* L) {
  UnitexString* str = lua_lightobject_cast(L, 1, UnitexString);
  // buffer used to prepare strings
  luaL_Buffer lb;
  // initialize the buffer
  luaL_buffinit(L, &lb);
  // get the underlying buffer area
  char* cb = luaL_prepbuffer(&lb);
  // encode the ustring as a utf-8 string
  int cb_length = str->encode(cb);
  // if length == 0 push empty
  // add to the buffer lb the string copied to the buffer area
  luaL_addsize(&lb, cb_length);
  // leave the final string on the top of the stack
  luaL_pushresult(&lb);
  // number of values returned
  return 1;
}
/* ************************************************************************** */
/* static */ int elg_ustring_decode(lua_State* L) {
  // length of the source string
  size_t length;
  // return the string at the top of stack if any
  const char* source = luaL_checklstring(L, 1, &length);
  // decode source as UTF-8
  lua_pushlightobject(L, UnitexString)(UTF8, source, length);
  // number of values returned
  return 1;
}
/* ************************************************************************** */
/* static */ int elg_ustring_print(lua_State* L) {
  // check if there is at least an argument on the stack
  if(lua_gettop(L) >= 1) {
    // returns the light userdata pointer. Otherwise, returns NULL
    UnitexString* output= (UnitexString*) lua_touserdata(L, 1);
    output->title();
    u_printf("%S\n",output->c_unichar());
    const char* second = lua_tostring(L, 2);
    if(!output->is_null()) {
      u_printf("%S%s\n",output->c_unichar(),second);
    } else {
      u_printf("%s\n",second);
    }

    *output += "YES";
    output->append(second);

    // the number of results is equal to 0
    return 0;
  }
  // the number of results is equal to 0
  return 0;
}
/* ************************************************************************** */
/* static */ const struct luaL_Reg lua_lib_functions[] = {
  {NULL, NULL}
};
/* ************************************************************************** */
/* static */  const struct luaL_Reg lua_lib_methods[] = {
  // U__DECLARE__FUNCTION__ELG__USTRING__
  U__DECLARE__FUNCTION__ENTRY__(USTRING, deaccentuate),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, fold),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, lower),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, title),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, upper),
  // U__DECLARE__FUNCTION__ELG__USTRING__INT__
  U__DECLARE__FUNCTION__ENTRY__(USTRING, length),
  //
  U__DECLARE__FUNCTION__ENTRY__(USTRING, print),
  //
  U__DECLARE__FUNCTION__ENTRY__(USTRING, encode),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, decode),

  //
  U__DECLARE__FUNCTION__ENTRY__ALIAS__(USTRING, encode,__tostring),
  //
  U__DECLARE__GC__ENTRY__(UnitexString),
  {NULL, NULL}};
/* ************************************************************************** */
int luaopen_ustring(lua_State *L) {
  // -------------------------------------------
  // create the lib table
  // [-0, +1] > (+1)
  //lua_newtable(L);
  //  luaL_register(L, EXTENSION_FULL_NAME_USTRING, ARRAY_LAST_ELEMENT(lua_lib));
  //  elg_stack_dump(L);

  // register functions into the lib table
  luaL_register(L, EXTENSION_NAME_USTRING, lua_lib_functions);
  elg_stack_dump(L);

  // set the name of the module
  lua_pushliteral(L, EXTENSION_NAME_USTRING);
  lua_setfield(L, -2, "_NAME");
  elg_stack_dump(L);

  // set the version of the module
  lua_pushliteral(L, EXTENSION_VERSION_USTRING);
  lua_setfield(L, -2, "_VERSION");
  elg_stack_dump(L);

  // -------------------------------------------
  // create the module metatable
  luaL_newmetatable(L, EXTENSION_NAME_USTRING);
  elg_stack_dump(L);

  // duplicate the metatable
  lua_pushvalue(L, -1);
  elg_stack_dump(L);

  // mt.__index = mt
  lua_setfield(L, -2, "__index");
  elg_stack_dump(L);

  // register metamethods
  luaL_register(L, NULL, lua_lib_methods);
  elg_stack_dump(L);

  // assign the metatable to the lib table
  lua_setmetatable(L, -2);
  elg_stack_dump(L);

  // -------------------------------------------
  // add the lib table to the elg lib table
  lua_setfield(L, -2,  EXTENSION_NAME_USTRING);
  elg_stack_dump(L);
  return 1;
}
/* ************************************************************************** */
#undef U__DECLARE__FUNCTION__ELG__USTRING__
/* ************************************************************************** */
}  // namespace unitex::elg::ustring::{unnamed}
/* ************************************************************************** */
}  // namesapce unitex::elg::ustring
/* ************************************************************************** */
}  // namespace unitex::elg
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif // USTRING_H_
