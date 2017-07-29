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
/* 
 * Portions of code based on lua-llthreads are marked as @source lua-llthreads
 * 
 * Copyright (c) 2011 by Robert G. Jakabosky <bobby@sharedrealm.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef ELGLIB_COPY_STATE_H_
#define ELGLIB_COPY_STATE_H_
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
#define UNITEX_COPY_STATE_MAX_DEPTH 10
/* ************************************************************************** */
#if defined(LUA_VERSION_NUM) &&\
            LUA_VERSION_NUM == 501
# ifndef  lua_isinteger
# define lua_isinteger(L, index)\
             ((lua_type(L, index) == LUA_TNUMBER) &&\
              (lua_tointeger(L, index) == lua_tonumber(L, index)))
#endif   // ifndef  lua_isintege
#endif  // defined(LUA_VERSION_NUM)
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
// to trace the copy between two states
// @source lua-llthreads
struct copy_state {
  lua_State* from;
  lua_State* to;
  int has_cache;
  int cache_idx;
  copy_state(void)
      : from(NULL),
        to(NULL),
        has_cache(0),
        cache_idx(0) {
  }
};
/* ************************************************************************** */
static int copy_number(lua_State* to, lua_State* from, int idx);
static int copy_boolean(lua_State* to, lua_State* from, int idx);
static int copy_string(lua_State* to, lua_State* from, int idx);
static int copy_lightuserdata(lua_State* to, lua_State* from, int idx);
static int copy_function(lua_State* to, lua_State* from, int idx);
static int copy_table_from_cache(copy_state* state, int idx);
static int copy_value(copy_state* state, int idx, int depth);
static int copy_table(copy_state* state, int idx, int depth);
static int copy_values(lua_State* to, lua_State* from,  int idx, int top);
/* ************************************************************************** */
static int copy_number(lua_State* to, lua_State* from, int idx) {
  if (lua_isinteger(from, idx)) {
    lua_pushinteger(to, lua_tointeger(from, idx));
  } else {
    lua_pushnumber(to, lua_tonumber(from, idx));
  }
  return 1;
}
/* ************************************************************************** */
static int copy_boolean(lua_State* to, lua_State* from, int idx) {
  lua_pushboolean(to, lua_toboolean(from, idx));
  return 1;
}
/* ************************************************************************** */
static int copy_string(lua_State* to, lua_State* from, int idx) {
  size_t length;
  const char* string = lua_tolstring(from, idx, &length);
  if (string) {
    lua_pushlstring(to, string, length);
    return 1;
  }
  return 0;
}
/* ************************************************************************** */
static int copy_lightuserdata(lua_State* to, lua_State* from, int idx) {
  lua_pushlightuserdata(to, lua_touserdata(from, idx));
  return 1;
}
/* ************************************************************************** */
static int copy_function(lua_State* to, lua_State* from, int idx) {
  lua_CFunction f = lua_tocfunction(from, idx);
  if (f) {
    lua_pushcfunction(to, f);
    return 1;
  }
  return 0;
}
/* ************************************************************************** */
// check if a table is cached; append to the cache if not
// @source lua-llthreads
static int copy_table_from_cache(copy_state* state, int idx) {
  void* ptr;

  // convert table to pointer for lookup in cache
  ptr = (void *) lua_topointer(state->from, idx);

  if (ptr == NULL) {
    // can't convert pointer
    return 0;
  }

  // check if we need to create the cache
  if (!state->has_cache) {
    lua_newtable(state->to);
    lua_replace(state->to, state->cache_idx);
    state->has_cache = 1;
  }

  lua_pushlightuserdata(state->to, ptr);
  lua_rawget(state->to, state->cache_idx);

  // check if the table was already cached
  if (lua_isnil(state->to, -1)) {
    // not in cache
    lua_pop(state->to, 1);
    // create new table and add to cache
    lua_newtable(state->to);
    lua_pushlightuserdata(state->to, ptr);
    lua_pushvalue(state->to, -2);
    lua_rawset(state->to, state->cache_idx);
    return 0;
  }

  // found table in cache
  return 1;
}
/* ************************************************************************** */
// copy a table between two states
static int copy_table(copy_state* state, int idx, int depth) {
  // only if it's not already on the cache
  if (!copy_table_from_cache(state, idx)) {
    lua_pushnil(state->from);
    while (lua_next(state->from, idx) != 0) {
      int value_pos = lua_gettop(state->from);
      int key_pos   = value_pos - 1;
      int key_type  = lua_type(state->from, key_pos);
      // only for numeric and string keys
      if (key_type == LUA_TNUMBER || key_type == LUA_TSTRING) {
        if (copy_value(state, key_pos, depth)) {
          if (copy_value(state, value_pos, depth)) {
            // do t[k] = v
            lua_settable(state->to, -3);
          } else {
            // remove key from the stack
            lua_pop(state->to, 1);
          }
        }
      }
      // remove value but keep key for next iteration
      lua_pop(state->from, 1);
    }
  }
  return 1;
}
/* ************************************************************************** */
// copy a single value between two states
static int copy_value(copy_state* state, int idx, int depth) {
  // check maximum recursive copy depth
  // depth is not passed by reference
  // @see https://stackoverflow.com/a/16707247/2042871
  if(++depth > UNITEX_COPY_STATE_MAX_DEPTH) {
    return luaL_error(state->from,
                       "Maximum copy depth reached (%d > %d)",
                       depth,
                       UNITEX_COPY_STATE_MAX_DEPTH);
  }

  int value_type = lua_type(state->from, idx);
  switch (value_type) {
    //   LUA_TNIL:
    case LUA_TBOOLEAN:        return copy_boolean(state->to, state->from, idx);       break;
    case LUA_TLIGHTUSERDATA:  return copy_lightuserdata(state->to, state->from, idx); break;
    case LUA_TNUMBER:         return copy_number(state->to, state->from, idx);        break;
    case LUA_TSTRING:         return copy_string(state->to, state->from, idx);        break;
    case LUA_TTABLE:          return copy_table(state, idx, depth);                   break;
    case LUA_TFUNCTION:       return copy_function(state->to, state->from, idx);      break;
    //   LUA_TUSERDATA:
    //   LUA_TTHREAD:
    default:                  return 0;                                               break;
  }
  return 0;
}
/* ************************************************************************** */
// copy values between two states
// @source lua-llthreads
UNITEX_UNUSED
static int copy_values(lua_State* to, lua_State* from, int idx, int top) {
  copy_state  state;

  int n_values = top - idx + 1;
  // make sure there is room on the new state for the values
  if (!lua_checkstack(to, n_values)) {
    elg_error(from,"stack overflow!");
  }

  // setup copy_state
  state.from = from;
  state.to = to;
  state.has_cache = 0;
  lua_pushnil(to);
  state.cache_idx = lua_gettop(to);

  // copy values one by one
  int n_copied_values = 0;
  int depth = 0;
  for (; idx <= top; ++idx) {
    if (copy_value(&state, idx, depth)) {
      ++n_copied_values;
    }
  }

  // remove cache table
  lua_remove(to, state.cache_idx);

  return n_copied_values;
}
/* ************************************************************************** */
#define copy_all_values(to, from, idx, top) copy_values(to, from, idx, top)
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // ELGLIB_COPY_STATE_H_
