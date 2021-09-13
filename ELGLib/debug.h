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
#ifndef ELGLIB_DEBUG_H_
#define ELGLIB_DEBUG_H_
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
#include "Unicode.h"
#include "base/debug/elg_build_mode.h"
/* ************************************************************************** */
#define elg_error(L,message)                        \
  return luaL_error(L,"[%s:%s:%d] Error: %s",       \
                    ELG_ENVIRONMENT_PREFIX,          \
                    UNITEX_COMPILER_IDENTIFIER_FUNC, \
                    UNITEX_FILE_LINE,                \
                    message);
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
namespace elg {
/* ************************************************************************** */
namespace {   // namespace elg::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */

/* ************************************************************************** */
}  // namespace unitex::elg::{unnamed
/* ************************************************************************** */
}  // namespace unitex::elg
/* ************************************************************************** */

/* ************************************************************************** */
namespace {   // namespace elg::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
#if ELG_BUILD_MODE(DEBUG)
void entryprint(lua_State *L, const int entry, const char *kv)
{
		if (lua_type(L, (entry)) == LUA_TSTRING)
			u_printf("%s%s", kv, lua_tostring(L, entry));
		else if (lua_isnumber(L, entry))
			u_printf("%s" LUA_NUMBER_FMT, kv, lua_tonumber(L, entry));
		else if (lua_isfunction(L, entry))
			u_printf("%sfunction %p", kv, lua_topointer(L, entry));
		else
			u_printf("%s%s", kv, luaL_typename(L, entry));
}

void tableprint(lua_State *L, const int tindex, const char *ttype)
{
	if (!lua_istable(L, tindex))
	{
		u_printf("\tcontents of %s: not a table but %s\n",
								ttype, luaL_typename(L, tindex));
		return;
	}
	/* must push table to top */
	u_printf("\tcontents of %s:\n", ttype);
	lua_pushvalue(L, tindex);
	int ttindex = lua_gettop(L);
	lua_pushnil(L); /* first key */
	while (lua_next(L, ttindex) != 0)
	{
		/* key at respectively -2 */
		entryprint(L, -2, "\t(key)");
		/* value at -1 */
		entryprint(L, -1, "\t(value)");
		lua_pop(L, 1); /* pop value, keep key for next round */
		u_printf("\n");
	}
	lua_pop(L, 1);	/* remove copied table */
}

void stack_dump(lua_State *L,
					const char *mark = "",		/* print after *** stackdump */
					const bool listtable = "true")	/* show (meta)table contents */
{

	u_printf("******************************************** stackdump %s\n", mark);
	int i;
	int top = lua_gettop(L);

	for (i = 1; i <= top; ++i)
	{
		int t = lua_type(L, i);
		const char *tn = (char *)lua_islightuserdata(L, i) ?
						"lightuserdata" : (char *)lua_typename(L, t);
		size_t l;
		const char *meta = "(no mt)", *metastr = "(no__tostring)";
		char *s;

		u_printf("\n(stack %d/%d %s) ", i, i-top-1, tn);
		switch (t)
		{
			case LUA_TNIL:
				break;

			case LUA_TBOOLEAN:
				u_printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
				break;

			case LUA_TLIGHTUSERDATA:
				u_printf("%p\n", lua_touserdata(L, i));
				break;

			case LUA_TNUMBER:
				u_printf("%g\n", lua_tonumber(L, i));
				break;

			case LUA_TSTRING:
				s = (char *)lua_tolstring(L, i, &l);
				u_printf("[l=%lu] %s\n", l, s);
				break;

			case LUA_TTABLE:
				if (lua_getmetatable(L, i))
				{
					u_printf("(has mt)\n");
					lua_pop(L, 1);
				} else u_printf("(no mt)\n");
				if (listtable)
				{
					tableprint(L, i, "table");
					if (lua_getmetatable(L, i))
					{
						tableprint(L, i, "metatable");
						lua_pop(L, 1);
					}
				}
				break;

			case LUA_TFUNCTION:
				break;

			case LUA_TUSERDATA:	   /* print description if tostring function */
				if (lua_getmetatable(L, i))
				{
//					meta = "(has mt)";
//					if (luaL_callmeta(L, i, "__tostring")) {
//						metastr = (char *)lua_tostring(L, -1);
//						lua_pop(L, 1);						 /* pop function */
//					}
					u_printf("%p %s %s\n", lua_touserdata(L, i), meta, metastr);
					if (listtable) tableprint(L, lua_gettop(L), "metatable");
					lua_pop(L,1);							/* pop metatable */
				}
				else u_printf("%p %s %s\n", lua_touserdata(L, i), meta, metastr);
				break;

			case LUA_TTHREAD:
				u_printf("\n");
				break;

			default:
				u_printf("CAN'T HAPPEN typevalue = %d\n", t);
				break;
		}
	}
	u_printf("\n**************************************** end stackdump %s\n", mark);
}
//// dump the stack to stderr
//// this is based on the Listing 27.2 of Programming in Lua, 4th ed.
//static void stack_dump(lua_State* L) {
  //luaL_traceback(L, L, NULL, 1);
  //u_printf("%s\n", lua_tostring(L, -1));
  //// get the index of the top element in the stack
  //int top = lua_gettop(L);
  //// type of an element in the stack
  //int type = LUA_TNONE;
  //// loop over the stack from the top to the bottom
  //for (int i = 1; i <= top; ++i) {
    //// get the type at current index
    //type = lua_type(L, i);
    //// print
    //switch (type) {
      //case LUA_TNONE:           // -1
        //u_printf("%-14s\n","NONE");
        //break;
      //case LUA_TNIL:            // 0
        //u_printf("%-14s\n","NIL");
        //break;
      //case LUA_TBOOLEAN:        // 1
        //u_printf("%-14s\n","BOOLEAN");
        //break;
      //case LUA_TLIGHTUSERDATA:  // 2
        //u_printf("%-14s\n","LIGHTUSERDATA");
        //break;
      //case LUA_TNUMBER:         // 3
        //u_printf("%-14s\n","NUMBER");
        //break;
      //case LUA_TSTRING:         // 4
        //u_printf("%-14s\n","STRING");
        //break;
      //case LUA_TTABLE:          // 5
        //u_printf("%-14s\n","TABLE");
        //break;
      //case LUA_TFUNCTION:       // 6
        //u_printf("%-14s\n","FUNCTION");
        //break;
      //case LUA_TUSERDATA:       // 7
        //u_printf("%-14s\n","USERDATA");
        //break;
      //case LUA_TTHREAD:         // 8
        //u_printf("%-14s\n","THREAD");
        //break;
    //}
  //}
//}
# define elg_stack_dump(L) stack_dump(L,UNITEX_COMPILER_IDENTIFIER_FUNC)
#else
# define elg_stack_dump(L)
#endif
/* ************************************************************************** */
}  // namespace unitex::{unnamed
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif // ELGLIB_DEBUG_H_
