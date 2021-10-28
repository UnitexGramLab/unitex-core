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
#ifndef UNITEX_ELGLIB_H_
#define UNITEX_ELGLIB_H_
/* ************************************************************************** */
#define EXTENSION_ID_ELG       elg
#define EXTENSION_VERSION_ELG  "0.1.0"
#define EXTENSION_NAME_ELG     "elg"
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
#include "ELGLib/uString/uString.h"
#include "ELGLib/uDicEntry/uDicEntry.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
namespace elg {
/* ************************************************************************** */
namespace {   // namespace elg::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
const struct luaL_Reg lua_elg_lib_preload[] = {
  { EXTENSION_NAME_USTRING,    ustring::luaopen_elg_ustring     },
  { EXTENSION_NAME_UDICENTRY,  udicentry::luaopen_elg_uDicEntry },
  { NULL,   NULL }
};
/* ************************************************************************** */
int openlibs(lua_State *L) {
  // create the elg table
  // [-0, +1] > (+1)
  lua_newtable(L);
  elg_stack_dump(L);
  // [-0+n, +1+n] > (+1)

  // register preload modules into the elg table
  const luaL_Reg *lib;
  for (lib = lua_elg_lib_preload; lib->func; ++lib) {
    // first, push the lib table onto the stack
    lib->func(L);
    // then, set its name
    lua_setfield(L, -2,  lib->name);
    elg_stack_dump(L);
  }

  // set the name of the elg module
  lua_pushliteral(L, EXTENSION_NAME_ELG);
  lua_setfield(L, -2, "_NAME");

  // set the version of the elg module
  lua_pushliteral(L, EXTENSION_VERSION_ELG);
  lua_setfield(L, -2, "_VERSION");
  elg_stack_dump(L);

  // register as a global "elg" table
  // [-1, +0] > (+0)
  lua_setglobal(L, EXTENSION_NAME_ELG);
  elg_stack_dump(L);
  return 1;
}
/* ************************************************************************** */
}  // namespace unitex::elg::{unnamed
/* ************************************************************************** */
}  // namespace unitex::elg
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif // UNITEX_ELGLIB_H_
