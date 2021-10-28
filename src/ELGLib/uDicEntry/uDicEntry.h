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
#ifndef UNITEX_ELGLIB_UDICENTRY_H_
#define UNITEX_ELGLIB_UDICENTRY_H_
/* ************************************************************************** */
#if UNITEX_COMPILER_AT_LEAST(MSVC,15,0)
#pragma warning(push)
#pragma warning(disable:4291)    // no matching operator delete found
#endif  // UNITEX_COMPILER_AT_LEAST(MSVC,15,0)
/* ************************************************************************** */
#define EXTENSION_ID_UDICENTRY        uDicEntry
#define EXTENSION_VERSION_UDICENTRY   "0.1.0"
#define EXTENSION_USERDATA_UDICENTRY  UnitexDicEntry
#define EXTENSION_NAME_UDICENTRY      EXTENSION_NAME(EXTENSION_ID_UDICENTRY)
#define EXTENSION_PREFIX_UDICENTRY    EXTENSION_PREFIX(EXTENSION_ID_UDICENTRY)
#define EXTENSION_UnitexDicEntry      EXTENSION_ID_UDICENTRY
/* ************************************************************************** */
// C system files (order the includes alphabetically)

/* ************************************************************************** */
// C++ system files (order the includes alphabetically)

/* ************************************************************************** */
// Other libraries' .h files (order the includes alphabetically)

/* ************************************************************************** */
// Project's .h files. (order the includes alphabetically)
#include "ELGLib/common.h"
#include "ELGLib/debug.h"
#include "UnitexString.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
namespace elg {
/* ************************************************************************** */
namespace udicentry {
/* ************************************************************************** */
namespace {   // namespace elg::udicentry::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
U__DECLARE__ELG__EXTENSION__METHOD__PUSHLIGHT__(UnitexDicEntry, inflected, UnitexString)
U__DECLARE__ELG__EXTENSION__METHOD__PUSHLIGHT__(UnitexDicEntry, lemma, UnitexString)
/* ************************************************************************** */
///* static */ int elg_uDicEntry_encode(lua_State* L) {
//  //UnitexDicEntry* o = lua_checkudata_cast(L, 1, UnitexDicEntry);
//
//  // pushes onto the stack the string 'str'
//  // elg::pushstring(L, str);
//  // lua_pushstring(L,"Test");
//  // number of values returned
//  return 1;
//}
/* ************************************************************************** */
/* static */ int elg_uDicEntry_decode(lua_State* L) {
  //  if we are dealing with a string, converts it first to a uString
  if (lua_type(L, -1) == LUA_TSTRING) {
    elg::ustring::elg_ustring_decode(L);
  }

  // return the uString at the top of stack if any
  UnitexString* line = lua_checkudata_cast(L, -1, UnitexString);

  // decode as a UnitexDicEntry
  lua_pushlightobject(L, UnitexDicEntry)(line->c_unichar());

  // number of values returned
  return 1;
}
/* ************************************************************************** */
/* static */ int elg_uDicEntry_code_attr(lua_State* L) {
  //  if we are dealing with a string, converts it first to a uString
  if (lua_type(L, -1) == LUA_TSTRING) {
    elg::ustring::elg_ustring_decode(L);
  }

  // return the uString at the top of stack if any
  UnitexString* key = lua_checkudata_cast(L, -1, UnitexString);

  // return the uDicEntry that is performing the call
  UnitexDicEntry* entry = lua_checkudata_cast(L, 1, UnitexDicEntry);

  // create the variable that will hold the result
  // reserve space for 15 chars + null
  UnitexString* value = lua_pushlightobject(L, UnitexString)(15);

  // get the value of key if any
  entry->code_attr(key->c_unichar(), value);

  // number of values returned
  return 1;
}
/* ************************************************************************** */
/* static */ int elg_uDicEntry___call(lua_State* L) {
  // __call treat a table, in this case a elg.uDicEntry, like a function,
  // the first argument is the table itself, we remove it from the stack
  lua_remove(L,1);
  // the remain argument is expected to be a string to decode
  return elg_uDicEntry_decode(L);
}
/* ************************************************************************** */
/* static */ const struct luaL_Reg lua_elg_lib_functions[] = {
//  U__DECLARE__FUNCTION__ENTRY__(UDICENTRY, assign),
  {NULL, NULL}
};
/* ************************************************************************** */
/* static */ const struct luaL_Reg lua_elg_lib_methods[] = {
  U__DECLARE__FUNCTION__ENTRY__(UDICENTRY, inflected),
  U__DECLARE__FUNCTION__ENTRY__(UDICENTRY, lemma),
  U__DECLARE__FUNCTION__ENTRY__(UDICENTRY, code_attr),


  //
  U__DECLARE__FUNCTION__ENTRY__(UDICENTRY, decode),
  //U__DECLARE__FUNCTION__ENTRY__(UDICENTRY, encode),
  //U__DECLARE__FUNCTION__ENTRY__ALIAS__(UDICENTRY, encode, string),

  // methamethods
  // allow to treat a table like a function,
  U__DECLARE__FUNCTION__ENTRY__(UDICENTRY, __call),
  //U__DECLARE__FUNCTION__ENTRY__ALIAS__(UDICENTRY, len, __len),
  //U__DECLARE__FUNCTION__ENTRY__ALIAS__(UDICENTRY, encode, __tostring),

  // allow to release any resource associated with the userdata object
  U__DECLARE__GC__ENTRY__(UnitexDicEntry),
  {NULL, NULL}
};
/* ************************************************************************** */
U__DECLARE__FUNCTION__ELG__OPEN__EXTENSION__WITH__FUNCTIONS__AND__METHODS__(UDICENTRY);
/* ************************************************************************** */
}  // namespace unitex::elg::udicentry::{unnamed}
/* ************************************************************************** */
}  // namesapce unitex::elg::udicentry
/* ************************************************************************** */
}  // namespace unitex::elg
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#if UNITEX_COMPILER_AT_LEAST(MSVC,15,0)
#pragma warning(pop)
#endif  // UNITEX_COMPILER_AT_LEAST(MSVC,15,0)
/* ************************************************************************** */
#endif // UNITEX_ELGLIB_UDICENTRY_H_
