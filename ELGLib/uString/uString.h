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
#include <lua.hpp>
/* ************************************************************************** */
// Project's .h files. (order the includes alphabetically)
#include "ELG_API.h"
#include "UnitexString.h"
/* ************************************************************************** */
#define EXTENSION_NAME_USTRING     "ustring"
#define EXTENSION_VERSION_USTRING  "0.1.0"
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
/* static */ int format(lua_State* L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);
  if(p) {
    lua_pushstring(L,"Hello World");
    // the number of results is equal to 1
    return 1;
  }
  // the number of results is equal to 0
  return 0;
}
/* ************************************************************************** */
/* static */ int print(lua_State* L) {
  // check if there is at least an argument on the stack
  if(lua_gettop(L) >= 1) {
    // returns the light userdata pointer. Otherwise, returns NULL
    Ustring* output= (Ustring*) lua_touserdata(L, 1);
    UnitexString X(output);
    X.totitle();
    u_printf("%S\n",X.c_unichar());
    const char* second = lua_tostring(L, 2);
    if(output->str && (*output->str) != U_NULL ) {
      u_printf("%S%s\n",output->str,second);
    } else {
      u_printf("%s\n",second);
    }
    u_strcat(output,second);
    // the number of results is equal to 0
    return 0;
  }
  // the number of results is equal to 0
  return 0;
}
/* ************************************************************************** */
/* static */ const struct luaL_Reg lua_lib[] = {
  {"format", elg::ustring::format},
  {"print", elg::ustring::print},
  {NULL, NULL}
};
/* ************************************************************************** */
int luaopen_ustring(lua_State *L) {
  // create the module table
  // [-0, +1] > (+1)
  lua_newtable(L);
  elg_stack_dump(L);

  // register functions into the module table
  luaL_register(L, NULL, lua_lib);
  elg_stack_dump(L);

  // set the name of the module
  lua_pushliteral(L, EXTENSION_NAME_USTRING);
  lua_setfield(L, -2, "_NAME");

  // set the version of the module
  lua_pushliteral(L, EXTENSION_VERSION_USTRING);
  lua_setfield(L, -2, "_VERSION");

  // add functions table to the module
  lua_setfield(L, -2,  EXTENSION_NAME_USTRING);
  elg_stack_dump(L);

  return 1;
}
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
