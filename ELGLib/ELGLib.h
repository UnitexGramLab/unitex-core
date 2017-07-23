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
#include <lua.hpp>
/* ************************************************************************** */
// Project's .h files. (order the includes alphabetically)
#include "uString/uString.h"
/* ************************************************************************** */
#define ELG_LIBRARY_NAME  "elg"
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
  unitex::elg::stack_dump(L,"lua_getfield");
  // [-0+n, +1+n] > (+1)
  // ustring module
  ustring::luaopen_ustring(L);
  unitex::elg::stack_dump(L,"lua_getfield");
  // [-1, +0] > (+0)
  lua_setglobal(L, ELG_LIBRARY_NAME);
  unitex::elg::stack_dump(L,"lua_getfield");
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
