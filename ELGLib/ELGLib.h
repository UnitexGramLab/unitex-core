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
 */
#ifndef ELGLIB_H_
#define ELGLIB_H_
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
/* ************************************************************************** */
#define EXTENSION_NAME_ELG     EXTENSION_NAME_1(ELG)
#define EXTENSION_VERSION_ELG  "0.1.0"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
namespace elg {
/* ************************************************************************** */
namespace {   // namespace elg::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
int openlibs(lua_State *L) {
  // create the module table
  // [-0, +1] > (+1)
  lua_newtable(L);
  elg_stack_dump(L);
  // [-0+n, +1+n] > (+1)

  // register modules into the module table
  // ustring module
  ustring::luaopen_ustring(L);
  elg_stack_dump(L);

  // set the name of the module
  lua_pushliteral(L, EXTENSION_NAME_ELG);
  lua_setfield(L, -2, "_NAME");

  // set the version of the module
  lua_pushliteral(L, EXTENSION_VERSION_ELG);
  lua_setfield(L, -2, "_VERSION");
  elg_stack_dump(L);

  // register a global "elg" table
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
#endif // ELGLIB_H_
