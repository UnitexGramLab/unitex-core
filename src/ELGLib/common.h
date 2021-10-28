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
 * cristian.martinez@unitexgramlab.org (martinec)
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
/**
 * Push an object of a type whose metatable was created with luaL_newmetatable
 * onto the stack. This use a placement new (construct an object in memory that's
 * already allocated)
 *
 * @source http://lua-users.org/wiki/DoItYourselfCppBinding
 */
#define lua_pushobject(L, T) new(L, #T) T

/**
 * Check whether the object at a given stack position is of a given
 * userdata type (registered with luaL_newmetatable)
 *
 * @source http://lua-users.org/wiki/DoItYourselfCppBinding
 */
#define lua_userdata_cast(L, pos, T) static_cast<T*>(luaL_checkudata((L), (pos), #T))
/* ************************************************************************** */
/**
 * Push a light object of a type whose metatable was created with luaL_newmetatable
 * onto the stack. This use a placement new (construct an object in memory that's
 * already allocated)
 */
#define lua_pushlightobject(L, T) new(L, EXTENSION_NAME(EXTENSION_##T)) T

/**
 * Check whether the light object at a given stack position is of a given
 * userdata type (registered with luaL_newmetatable), if fails, returns NULL
 * instead of throws an error.
 */
#define lua_testudata_cast(L, pos, T) static_cast<T*>(luaL_testudata((L), (pos), EXTENSION_NAME(EXTENSION_##T)))

/**
 * Check whether the light object at a given stack position is of a given
 * userdata type (registered with luaL_newmetatable), if fails, throws an error
 */
#define lua_checkudata_cast(L, pos, T) static_cast<T*>(luaL_checkudata((L), (pos), EXTENSION_NAME(EXTENSION_##T)))

/**
 *
 */
#define pushustring_or_insert(L, pos, s)                \
                if (s) {                                \
                  elg::pushustring(L, s->c_unichar());  \
                } else {                                \
                  lua_insert(L, pos);                   \
                }
/* ************************************************************************** */
#define EXTENSION_NAME(_1)   UNITEX_PP_STRINGIFY_VALUE(UNITEX_PP_STRING_CONCAT(  \
                             UNITEX_PP_STRING_CONCAT(EXTENSION_ID_ELG,.),_1))
#define EXTENSION_PREFIX(_1) UNITEX_PP_TOKEN_PASTE(UNITEX_PP_TOKEN_PASTE(    \
                             UNITEX_PP_TOKEN_PASTE(EXTENSION_ID_ELG,_),_1),_)
/* ************************************************************************** */
#define EXTENSION_USERDATA(_1) UNITEX_PP_TOKEN_CAT(EXTENSION_USERDATA##_,_1)
/* ************************************************************************** */
#define U__DECLARE__FUNCTION__ENTRY__(_id,_func) \
  {UNITEX_PP_STRINGIFY_NAME(_func), UNITEX_PP_TOKEN_PASTE(EXTENSION_PREFIX_##_id,_func)}
#define U__DECLARE__FUNCTION__ENTRY__ALIAS__(_id,_func,_alias) \
  {UNITEX_PP_STRINGIFY_NAME(_alias), UNITEX_PP_TOKEN_PASTE(EXTENSION_PREFIX_##_id,_func)}
/* ************************************************************************** */
#define U__DECLARE__GC__ENTRY__(_class)      {"__gc", GCMethod<_class>}
/* ************************************************************************** */
// Create a table for the extension and registers its functions
// 1. create a new table using as name the id of the extension
// 2. register on it all functions in the list lua_elg_lib_functions
#define U__EXTENSION__CREATE__TABLE__REGISTER__FUNCTIONS__(L, _id)  \
  luaL_register(L, EXTENSION_NAME_##_id, lua_elg_lib_functions);    \
  elg_stack_dump(L)

// Set the extension name and version
// 1. set the extension name
// 2. set the extension version
#define U__EXTENSION__SET__NAME__AND__VERSION__(L, _id)  \
  lua_pushliteral(L, EXTENSION_NAME_##_id);          \
  lua_setfield(L, -2, "_NAME");                      \
  elg_stack_dump(L);                                 \
  lua_pushliteral(L, EXTENSION_VERSION_##_id);       \
  lua_setfield(L, -2, "_VERSION");                   \
  elg_stack_dump(L)

// Create a metatable with the extension methods
// 1. create a metatable using as name the id of the extension
// 2. duplicate the metatable
// 3. do mt.__index = mt
// 4. register all metamethods declared on lua_elg_lib_methods
// 5. assign the metatable to the lib table
#define U__EXTENSION__CREATE__METATABLE__REGISTER__METHODS__(L, _id)  \
  luaL_newmetatable(L, EXTENSION_NAME_##_id);       \
  elg_stack_dump(L);                                \
  lua_pushvalue(L, -1);                             \
  elg_stack_dump(L);                                \
  lua_setfield(L, -2, "__index");                   \
  elg_stack_dump(L);                                \
  luaL_register(L, NULL, lua_elg_lib_methods);      \
  elg_stack_dump(L);                                \
  lua_setmetatable(L, -2);                          \
  elg_stack_dump(L)

// 1. register functions into the lib table
// 2. set the name and version of the module
// 3. register methods into the lib table
#define U__DECLARE__FUNCTION__ELG__OPEN__EXTENSION__WITH__FUNCTIONS__AND__METHODS__(_id)  \
/* static */ int UNITEX_PP_TOKEN_PASTE(luaopen_elg_,EXTENSION_ID_##_id)(lua_State* L) {   \
      U__EXTENSION__CREATE__TABLE__REGISTER__FUNCTIONS__(L, _id);        \
      U__EXTENSION__SET__NAME__AND__VERSION__(L, _id);                   \
      U__EXTENSION__CREATE__METATABLE__REGISTER__METHODS__(L, _id);      \
      return 1;                                                          \
    }
/* ************************************************************************** */
// Basic types
#define EXTENSION_PUSH_CAST_lua_pushinteger     lua_Integer
#define EXTENSION_PUSH_CAST_lua_pushboolean     int
#define EXTENSION_PUSH_CAST_lua_pushcclosure    lua_CFunction
#define EXTENSION_PUSH_CAST_pushcfunction       lua_CFunction
#define EXTENSION_PUSH_CAST_pushfstring         const char *
#define EXTENSION_PUSH_CAST_pushlightuserdata   void *
#define EXTENSION_PUSH_CAST_pushliteral         const char *
#define EXTENSION_PUSH_CAST_pushlstring         const char *
#define EXTENSION_PUSH_CAST_pushnumber          lua_Number
#define EXTENSION_PUSH_CAST_pushstring          const char *
#define EXTENSION_PUSH_CAST_pushvfstring        const char *
// Custom types that can be converted to a basic type
#define EXTENSION_PUSH_CAST_pushustring         const unichar*
/* ************************************************************************** */
#define U__ELG__EXTENSION__METHOD__DECLARATION__(T,_func)                \
        UNITEX_PP_TOKEN_PASTE(UNITEX_PP_TOKEN_PASTE(                     \
        elg_,EXTENSION_##T),_##_func)(lua_State* L)
/* ************************************************************************** */
#define U__DECLARE__ELG__EXTENSION__METHOD__NAME__CALL__(T,_name,_func)  \
/* static */ int U__ELG__EXTENSION__METHOD__DECLARATION__(T, _name) {    \
  T* obj = lua_checkudata_cast(L, -1, T);                                \
  if (obj->is_attached()) {                                              \
    obj->_func();                                                        \
  } else {                                                               \
    (lua_pushlightobject(L, T)(*obj))->_func();                          \
  }                                                                      \
  return 1;                                                              \
}
#define U__DECLARE__ELG__EXTENSION__METHOD__CALL__(T, _func)             \
        U__DECLARE__ELG__EXTENSION__METHOD__NAME__CALL__(T,_func,_func)
/* ************************************************************************** */
#define U__DECLARE__ELG__EXTENSION__METHOD__NAME__PUSH__(                 \
        T,_name,_func,_push)                                              \
  /* static */ int U__ELG__EXTENSION__METHOD__DECLARATION__(T, _name) {   \
  T* obj = lua_checkudata_cast(L, -1, T);                                 \
  _push(L, static_cast<EXTENSION_PUSH_CAST_##_push>(obj->_func()));       \
  return 1;                                                               \
}
#define U__DECLARE__ELG__EXTENSION__METHOD__PUSH__(  \
        T,_func,_push)                                                    \
        U__DECLARE__ELG__EXTENSION__METHOD__NAME__PUSH__(T,_func,_func,_push)
/* ************************************************************************** */
#define U__DECLARE__ELG__EXTENSION__METHOD__NAME__PUSHLIGHT__(  \
        T_STACK,_name,_func,T_PUSH)                             \
  /* static */ int U__ELG__EXTENSION__METHOD__DECLARATION__(T_STACK,_name) {  \
    T_STACK* obj = lua_checkudata_cast(L, -1, T_STACK);                       \
    lua_pushlightobject(L, T_PUSH)(obj->_func());                             \
  return 1;                                                                   \
}

#define U__DECLARE__ELG__EXTENSION__METHOD__PUSHLIGHT__(        \
        T_STACK,_func,T_PUSH)                                   \
        U__DECLARE__ELG__EXTENSION__METHOD__NAME__PUSHLIGHT__(  \
                                   T_STACK,_func,_func,T_PUSH)
/* ************************************************************************** */
#define ARRAY_LAST_ELEMENT(_array)    (_array + ((sizeof _array/sizeof _array[0]) - 1))
/* ************************************************************************** */
/**
 * Define a placement new to avoid boxed pointers. Using together with macros
 * lua_pushobject() and lua_pushlightobject(), this allow to push an object
 * of a type whose metatable was created with luaL_newmetatable onto the stack
 *
 * @source http://lua-users.org/wiki/DoItYourselfCppBinding
 */
void* operator new(size_t size, lua_State* L, const char* metatableName);
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
namespace elg {
/* ************************************************************************** */
/**
 * Provide a garbaged collector method
 *
 * @tparam T  object or light object registered with luaL_newmetatable
 * @param L   per-thread Lua state object
 * @return    0
 *
 * @source http://lua-users.org/wiki/DoItYourselfCppBinding
 */
template<typename T>
int GCMethod(lua_State* L) {
  static_cast<T*>(lua_touserdata(L, 1))->~T();
  return 0;
}
/* ************************************************************************** */
namespace {   // namespace elg::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
// push a unichar string as a lua string into stack
// [-0, +1, m] > (+1)
void pushustring(lua_State* L, const unichar* value) {
  // buffer used to prepare strings
  luaL_Buffer lb;
  // initialize the buffer
  luaL_buffinit(L, &lb);
  // get the underlying buffer area
  char* cb = luaL_prepbuffer(&lb);
  // encode the UnitexString as a utf-8 string
  int cb_length = u_encode(U_ENCODE_UTF8, value, cb);
  // if length == 0 push empty
  // add to the buffer lb the string copied to the buffer area
  luaL_addsize(&lb, cb_length);
  // leave the final string on the top of the stack
  luaL_pushresult(&lb);
}
/* ************************************************************************** */
void updatefield(lua_State* L, int index, const char* k, const unichar* value) {
  // pushes onto the stack the string 'value'
  // [-0, +1] > (+2)
  elg::pushustring(L, value);
  elg_stack_dump(L);

  // update field t[k] = new_uvalue
  // [-1, +0] > (+1)
  lua_setfield(L, index, k);
}
/* ************************************************************************** */
void appendtofield(lua_State* L, int index, const char* k, const unichar* value) {
  // push onto the stack the value t[k]
  // [-0, +1] > (+2)
  lua_getfield(L, index, k);
  elg_stack_dump(L);

  size_t current_length;
  // [-0, +0] > (+2)
  const char* current_value = luaL_checklstring(L, -1, &current_length);
  elg_stack_dump(L);

  UnitexString new_uvalue(current_value, current_length);
  new_uvalue.append(value);

  // remove t[k] from the stack
  // [-1, +0] > (+1)
  lua_pop(L, 1);
  elg_stack_dump(L);

  // update field t[k] = new_uvalue
  // [+1, -1] > (+1)b
  updatefield(L,-2, k, new_uvalue.c_unichar());
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
