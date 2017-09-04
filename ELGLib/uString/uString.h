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
#include "ELGLib/debug.h"
#include "UnitexString.h"
#include "Unicode.h"
#include "base/integer/operation/round.h"

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
/* ************************************************************************** */
#define U__DECLARE__FUNCTION__ELG__USTRING__VARIANT__(_func)                \
/* static */ int elg_ustring_##_func(lua_State* L) {                        \
  UnitexString* str = lua_checkudata_cast(L, 1, UnitexString);              \
  if (str->is_attached()) {                                                 \
    str->_func();                                                           \
  } else {                                                                  \
    (lua_pushlightobject(L, UnitexString)(*str))->_func();                  \
  }                                                                         \
  return 1;                                                                 \
}
/* ************************************************************************** */
U__DECLARE__FUNCTION__ELG__USTRING__VARIANT__(deaccentuate);
U__DECLARE__FUNCTION__ELG__USTRING__VARIANT__(fold);
U__DECLARE__FUNCTION__ELG__USTRING__VARIANT__(lower);
U__DECLARE__FUNCTION__ELG__USTRING__VARIANT__(reverse);
U__DECLARE__FUNCTION__ELG__USTRING__VARIANT__(title);
U__DECLARE__FUNCTION__ELG__USTRING__VARIANT__(upper);
/* ************************************************************************** */
#undef U__DECLARE__FUNCTION__ELG__USTRING__VARIANT__
/* ************************************************************************** */
#define U__DECLARE__FUNCTION__ELG__USTRING__INT__(_func)                    \
/* static */ int elg_ustring_##_func(lua_State* L) {                        \
  UnitexString* str = lua_checkudata_cast(L, 1, UnitexString);              \
  lua_pushinteger(L, (lua_Integer) str->_func());                           \
  return 1;                                                                 \
}
/* ************************************************************************** */
U__DECLARE__FUNCTION__ELG__USTRING__INT__(len);
/* ************************************************************************** */
#undef U__DECLARE__FUNCTION__ELG__USTRING__INT__
/* ************************************************************************** */
/* static */ int elg_ustring_rep(lua_State* L) {
  UnitexString* str = lua_checkudata_cast(L, 1, UnitexString);
  // number of copies
  int n = luaL_checkint(L, 2);
  // push a new string representing a concatenation of n copies of str
  lua_pushlightobject(L, UnitexString)(n, *str);

//  for (int i = 0; i< 20000000; ++i) {
//    str->reverse();
//  }

  return 1;
}
/* ************************************************************************** */
/* static */ int elg_ustring_format(lua_State* L) {
  int arg = 1;
  int top = lua_gettop(L);
  UnitexString* fmt = lua_checkudata_cast(L, arg, UnitexString);
  const unichar* strfrmt      = fmt->begin();
  const unichar* strfrmt_end  = fmt->end();

  // create a new string with a capacity of len + 16n and push on the stack
  UnitexString* b = lua_pushlightobject(L, UnitexString)(fmt->len() + UnitexString::kMinBufferSize * top);

  int n_printed = 0;
  int i = 0;
  double d;
  char c = '\0';
  const void* p = NULL;
  unichar uc = '\0';
  const char* s = NULL;
  const UnitexString* us = NULL;

  while (strfrmt < strfrmt_end) {
     if (*strfrmt=='%') {
        /* If we have a special sequence introduced by '%' */
        strfrmt++;
        if(*strfrmt!='%' && ++arg > top) luaL_argerror(L, arg, "no value");
        switch (*strfrmt) {
           /* If we have %% we must print a '%' */
           case '%': if (b) b->append('%'); n_printed++; break;

           /* If we have %C we must print an unicode character */
           case 'C': {
              uc=(unichar)luaL_checkint(L, arg);
              if (b) b->append(uc);
              n_printed++;
              break;
           }

           case 'H':
           case 'U': {
              int (*XXXize)(const unichar*,unichar*);
              if (*strfrmt=='H') {
                  XXXize=htmlize;
              } else {
                  XXXize=URLize;
              }
              /* If we have a '%H', it means that we have to print HTML things */
              strfrmt++;
              if (*strfrmt=='S') {
                 /* If we have to print a HTML string */
                 us=lua_checkudata_cast(L, arg, UnitexString);
                 if (us==NULL) {
                    if (b) b->append("(null)");
                    n_printed=n_printed+6;
                 } else {
                    unichar html[4096];
                    int l=XXXize(us->c_unichar(),html);
                    if (b) b->append(html,l);
                    n_printed=n_printed+l;
                 }
              } else if (*strfrmt=='R') {
                 /* If we have to print a HTML reversed string */
                 us=lua_checkudata_cast(L, arg, UnitexString);
                 if (us==NULL) {
                    if (b) b->append("(null)");
                    n_printed=n_printed+6;
                 } else {
                    unichar reversed[4096];
                    mirror(us->c_unichar(),reversed);
                    unichar html[4096];
                    int l=XXXize(reversed,html);
                    if (b) b->append(html,l);
                    n_printed=n_printed+l;
                 }
              } else {
                char msg[64];
                sprintf(msg, "invalid format option %c%c",*(strfrmt-1),*strfrmt);
                luaL_argerror(L, arg, msg);
              }
              break;
           }

           /* If we have %S we must print an unicode string */
           case 'S': {
              us=lua_checkudata_cast(L, arg, UnitexString);
              if (us==NULL) {
                 if (b) b->append("(null)");
                 n_printed=n_printed+6;
              } else {
                 if (b) b->append(us->c_ustring());
                 n_printed=n_printed+us->len();
              }
              break;
           }

           /* If we have %R we must print a reversed unicode string */
           case 'R': {
              us=lua_checkudata_cast(L, arg, UnitexString);
              if (us==NULL) {
                 /* We don't want to print ")llun(" when the string to reverse is NULL */
                 if (b) b->append("(null)");
                 n_printed=n_printed+6;
                 break;
              }
              unichar reversed[4096];
              int old=n_printed;
              n_printed=n_printed+mirror(us->c_unichar(),reversed);
              if (b) b->append(reversed, us->len());
              break;
           }

           // not supported for now
           // case 'n':

           /* If we have '%???', we let sprintf do the job */
           default: {
              /* We get back on the '%' */
              strfrmt--;
              int z=0;
              char format2[64];
              char result[4096];
              do {
                 format2[z++]=*strfrmt;
                 strfrmt++;
              } while (format2[z-1]!='\0' && !strchr("diouxXeEfgcsp",format2[z-1]));
              /* We get back one character */
              strfrmt--;
              if (format2[z-1]=='\0') {
                char msg[64];
                sprintf(msg, "invalid format option %s",format2);
                luaL_argerror(L, arg, msg);
              }
              format2[z]='\0';
              int n_printed_old=n_printed;
              int l=0;
              switch (format2[z-1]) {
                 case 'd': case 'i': case 'o': case 'u': case 'x': case 'X': {
                    i=luaL_checkint(L, arg);
                    l=sprintf(result,format2,i);
                    n_printed+=l;
                    break;
                 }
                 case 'e':  case 'E': case 'f':  case 'g': {
                    d=(double)luaL_checknumber(L, arg);
                    l=sprintf(result,format2,d);
                    n_printed+=l;
                    break;
                 }
                 case 'c': {
                    c=(char)luaL_checkint(L, arg);
                    l=sprintf(result,format2,c);
                    n_printed+=l;
                    break;
                 }
                 case 's': {
                    s=luaL_checkstring(L, arg);
                    l=sprintf(result,format2,s);
                    n_printed+=l;
                    break;
                 }
                 case 'p': {
                    p=(void*)lua_topointer(L, arg);
                    l=sprintf(result,format2,p);
                    n_printed+=l;
                    break;
                 }
              }
              if (b) b->append(result,l);
              break;
           }
        }
     } else {
        /* If we have a normal character, we print it */
        if (b) b->append(*strfrmt);
        n_printed++;
     }
     strfrmt++;
  }

//
//  for (int arg = 2; arg <= n; ++arg) {
//    int t = lua_type(L, arg);
//
//    switch (t) {
//      case LUA_TNIL:
//        break;
//
//      case LUA_TBOOLEAN:
//        lua_toboolean(L, arg);
//        break;
//
//      case LUA_TLIGHTUSERDATA:
//        break;
//
//      case LUA_TNUMBER:
//        lua_tonumber(L, arg);
//        break;
//
//      case LUA_TSTRING:
//        lua_tolstring(L, arg, &len);
//        break;
//
//      case LUA_TTABLE:
//        break;
//
//      case LUA_TFUNCTION:
//        lua_topointer(L, arg);
//        break;
//
//      case LUA_TUSERDATA:
//        break;
//
//      case LUA_TTHREAD:
//        lua_topointer(L, arg);
//        break;
//
//      default:
//        "(null)";
//        break;
//    }
//  }

  return 1;
}
/* ************************************************************************** */
// based on the string_byte function of LuaJIT/lib_string.c
/* static */ int elg_ustring_byte(lua_State* L) {
  UnitexString* s =  lua_checkudata_cast(L, 1, UnitexString);
  int32_t len = (int32_t) s->len();
  int32_t start = luaL_optint(L, 2, 1);
  int32_t stop = luaL_optint(L, 3, start);
  int32_t n, arg;
  if (stop < 0) stop += len+1;
  if (start < 0) start += len+1;
  if (start <= 0) start = 1;
  if (stop > len) stop = len;
  if (start > stop) return 0;  /* Empty interval: return no results. */
  start--;
  n = stop - start;
  luaL_checkstack(L, n, "string slice too long");
  const unichar* p = ((unichar *)(s->c_unichar())) + start;
  for (arg = 0; arg < n; arg++)
    lua_pushinteger(L, p[arg]);
  return n;
}
/* ************************************************************************** */
/* static */ int elg_ustring_char(lua_State* L) {
  int n = lua_gettop(L);
  // push a new string with a capacity of n codepoints
  UnitexString* s=  lua_pushlightobject(L, UnitexString)(n);
  // push back each numerical argument as a character
  for (int arg=1; arg<=n; arg++) {
    unichar c = luaL_checkint(L, arg);
    luaL_argcheck(L, (unsigned int) c == c, arg, "invalid value");
    s->append(c);
  }
  return 1;
}
/* ************************************************************************** */
/* static */ int elg_ustring_encode(lua_State* L) {
  UnitexString* str = lua_checkudata_cast(L, 1, UnitexString);
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
/* static */ int elg_ustring___call(lua_State* L) {
  // __call treat a table, in this case a elg.ustring, like a function,
  // the first argument is the table itself, we remove it from the stack
  lua_remove(L,1);
  // the remain argument is expected to be a UTF-8 encoded string to decode
  return elg_ustring_decode(L);
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
  U__DECLARE__FUNCTION__ENTRY__(USTRING, rep),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, format),
  {NULL, NULL}
};
/* ************************************************************************** */
/* static */  const struct luaL_Reg lua_lib_methods[] = {
  // U__DECLARE__FUNCTION__ELG__USTRING__
  U__DECLARE__FUNCTION__ENTRY__(USTRING, deaccentuate),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, fold),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, lower),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, reverse),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, title),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, upper),

  // U__DECLARE__FUNCTION__ELG__USTRING__INT__
  U__DECLARE__FUNCTION__ENTRY__(USTRING, len),

  //
  U__DECLARE__FUNCTION__ENTRY__(USTRING, print),

  //
  U__DECLARE__FUNCTION__ENTRY__(USTRING, byte),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, char),

  //
  U__DECLARE__FUNCTION__ENTRY__(USTRING, decode),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, encode),

  // methamethods
  U__DECLARE__FUNCTION__ENTRY__(USTRING, __call),
  U__DECLARE__FUNCTION__ENTRY__ALIAS__(USTRING, len, __len),
  U__DECLARE__FUNCTION__ENTRY__ALIAS__(USTRING, encode, __tostring),
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
}  // namespace unitex::elg::ustring::{unnamed}
/* ************************************************************************** */
}  // namesapce unitex::elg::ustring
/* ************************************************************************** */
}  // namespace unitex::elg
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif // USTRING_H_
