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
/**
 * @file      ELG.cpp
 * @brief     ELGs
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/common.h header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 `ELG.cpp`
 *
 * @date      August 2016
 */
/* ************************************************************************** */
// Header for this file
#include "ELG.h"
/* ************************************************************************** */
// @source http://lua-users.org/wiki/DoItYourselfCppBinding
void* operator new(size_t size, lua_State* L, const char* metatableName) {
  void* ptr = lua_newuserdata(L, size);
  elg_stack_dump(L);
  luaL_getmetatable(L, metatableName);
  elg_stack_dump(L);
  lua_setmetatable(L, -2);
  elg_stack_dump(L);
  return ptr;
}

// This placement delete matches the placement new above, it will be called only
// from the placement new, placement delete functions are defined as no-operations
// by the Standard C++ library.
void operator delete(void* pMem, lua_State* L, const char* metatableName) {
  // do nothing as lua is supposed to handle this.
  fatal_error("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
}
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */

/* ************************************************************************** */
}  // namespace unitex
