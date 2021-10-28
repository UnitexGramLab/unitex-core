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
#ifndef UNITEX_ELGLIB_USTRING_H_
#define UNITEX_ELGLIB_USTRING_H_
/* ************************************************************************** */
#if UNITEX_COMPILER_AT_LEAST(MSVC,15,0)
#pragma warning(push)
#pragma warning(disable:4291)    // no matching operator delete found
#endif  // UNITEX_COMPILER_AT_LEAST(MSVC,15,0)
/* ************************************************************************** */
#define EXTENSION_ID_USTRING          ustring
#define EXTENSION_VERSION_USTRING     "0.1.0"
#define EXTENSION_USERDATA_USTRING    UnitexString
#define EXTENSION_NAME_USTRING        EXTENSION_NAME(EXTENSION_ID_USTRING)
#define EXTENSION_PREFIX_USTRING      EXTENSION_PREFIX(EXTENSION_ID_USTRING)
#define EXTENSION_UnitexString        EXTENSION_ID_USTRING
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
namespace unitex {
/* ************************************************************************** */
namespace elg {
/* ************************************************************************** */
namespace ustring {
/* ************************************************************************** */
namespace {   // namespace elg::ustring::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
U__DECLARE__ELG__EXTENSION__METHOD__CALL__(UnitexString, deaccentuate)
U__DECLARE__ELG__EXTENSION__METHOD__CALL__(UnitexString, fold)
U__DECLARE__ELG__EXTENSION__METHOD__CALL__(UnitexString, lower)
U__DECLARE__ELG__EXTENSION__METHOD__CALL__(UnitexString, reverse)
U__DECLARE__ELG__EXTENSION__METHOD__CALL__(UnitexString, title)
U__DECLARE__ELG__EXTENSION__METHOD__CALL__(UnitexString, upper)
U__DECLARE__ELG__EXTENSION__METHOD__PUSH__(UnitexString, len, lua_pushinteger)
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
/* static */ int elg_ustring___lt(lua_State* L) {
  // string1 is lower or equal to string2 if compare() returns < 0
  UnitexString* str1 = lua_testudata_cast(L, 1, UnitexString);
  UnitexString* str2 = lua_testudata_cast(L, 2, UnitexString);
  lua_pushboolean(L, str1 && str2 ? str1->compare(*str2) < 0 : 0);
  return 1;
}
/* ************************************************************************** */
/* static */ int elg_ustring___le(lua_State* L) {
  // string1 is lower or equal to string2 if compare() returns <= 0
  UnitexString* str1 = lua_testudata_cast(L, 1, UnitexString);
  UnitexString* str2 = lua_testudata_cast(L, 2, UnitexString);
  lua_pushboolean(L, str1 && str2 ? str1->compare(*str2) <= 0 : 0);
  return 1;
}
/* ************************************************************************** */
/* static */ int elg_ustring___eq(lua_State* L) {
  // string1 is equal to string2 if compare() returns 0
  UnitexString* str1 = lua_testudata_cast(L, 1, UnitexString);
  UnitexString* str2 = lua_testudata_cast(L, 2, UnitexString);
  lua_pushboolean(L, str1 && str2 ? str1->compare(*str2) == 0 : 0);
  return 1;
}
/* ************************************************************************** */
// This constants are from @see LuaJIT/lib_string.c
/* maximum size of each formatted item (> len(format('%99.99f', -1e308))) */
#define U_MAX_FMTITEM 512
/* valid flags in a format specification */
#define U_FMT_FLAGS "-+ #0"
/*
 * maximum size of each format specification (such as '%-099.99d')
 * (+10 accounts for %99.99x plus margin of error)
 */
#define U_MAX_FMTSPEC (sizeof(U_FMT_FLAGS) + sizeof(LUA_INTFRMLEN) + 10)
/* ************************************************************************** */
/**
 * Lua's elg.ustring sprintf
 * based on Unitex's 3.2 u_vsprintf()    @see 3.2/Unicode.cpp
 * based on Lua's 5.3 str_format()       @see 5.3/lstrlib.c
 * based on LuaJIT's 2.1 string_format() @see 2.1/lib_string.c
 *
 * @brief Returns a formatted version of its variable number of arguments
 *        following the description given in its first argument (which must
 *        be a unicode string)
 *
 * The format string follows the same rules as the printf family of standard C
 * functions. It supports the next specifiers:
 *
 *  %%    a percent sign
 *  %c    a character with the given number
 *  %d    a signed integer, in decimal
 *  %e    a floating-point number, in scientific notation
 *  %E    like %e, but using an upper-case "E"
 *  %f    a floating-point number, in fixed decimal notation
 *  %g    a floating-point number, in %e or %f notation
 *  %G    like %g, but with an upper-case "E" (if applicable)
 *  %i    signed integer, in decimal
 *  %o    an unsigned integer, in octal
 *  %p    a pointer (outputs the Lua value's address in hexadecimal or nil)
 *  %s    a string
 *  %u    an unsigned integer, in decimal
 *  %x    an unsigned integer, in hexadecimal
 *  %X    like %x, but using upper-case letters
 *
 * Similarly to Lua's string.format(), it also supports the next specifier:
 *
 *  %q    a string in a form suitable to be safely read back by the Lua interpreter
 *
 * Additionally, for Unicode strings, it also supports the additional specifiers:
 *
 *  %C    a unicode character with the given number
 *  %S    a unicode string
 *  %Q    a string in a form suitable to be safely read back by the Lua interpreter
 *
 *  %>Q   a string in a form suitable to be safely read back by the Lua interpreter
 *  %>R   a reversed unicode string
 *  %>H   a HTML escaped unicode string
 *  %>U   a URL escaped unicode string
 *  %>J
 *  %>X
 *  %>L
 *  %>l
 *  %>t
 *  %>u
 *  %>f
 *
 * The options/modifiers *, l, L, n, and h are not supported
 */
/* static */ int elg_ustring_format(lua_State* L) {
  int top = lua_gettop(L);
  int arg = 1;
  size_t sfl;
  const char *strfrmt = luaL_checklstring(L, arg, &sfl);
  const char *strfrmt_end = strfrmt + sfl;

  // main buffer to build the final string
  UnitexString* b = lua_pushlightobject(L, UnitexString)(U_MAX_FMTITEM);
  // temporal buffer to contain the append/modify operations
  UnitexString* a = lua_pushlightobject(L, UnitexString)(U_MAX_FMTITEM);

  lua_Number d;                       //
  char c = '\0';                      //
  unichar uc = '\0';                  //
  lua_Integer n = 0;                  //
  const void* p = NULL;               //
  const char* s = NULL;               //
  const UnitexString* us = NULL;      //

  while (strfrmt < strfrmt_end) {
     if (*strfrmt=='%') {
        /* If we have a special sequence introduced by '%' */
        strfrmt++;
        if(*strfrmt!='%' && ++arg > top) {
          luaL_argerror(L, arg, "no value");
        }
        switch (*strfrmt) {
           // %% a percent sign
           case '%':
             b->append('%');
             break;

           // %C a unicode character with the given number
           case 'C': {
              uc=(unichar)luaL_checkint(L, arg);
              b->append(uc);
              break;
           }

           // %Q a string in a form suitable to be safely read back by the Lua interpreter
           case 'Q': {
              us=lua_checkudata_cast(L, arg, UnitexString);
              b->append(us->c_ustring(), (U_TRANSLATE_FUNCTION) u_quotize, 2 * us->len());
              break;
           }

           // %S a unicode string
           case 'S': {
              us=lua_checkudata_cast(L, arg, UnitexString);
              b->append(us->c_ustring());
              break;
           }

           // %> an appender/modifier
           case '>': {
             int appender = 1;
             us=lua_checkudata_cast(L, arg, UnitexString);
             a->clear();
             next_strfrm_modifier:
             strfrmt++;
             // %>?
             switch (*strfrmt) {
               // %>R
               case 'R':
                  if (appender)  a->append(us->c_ustring(), (U_TRANSLATE_FUNCTION) u_reverse, 0);
                  else           a->reverse();
                 break;

               // %>H
               case 'H':
                  if (appender)  a->append(us->c_ustring(), (U_TRANSLATE_FUNCTION) htmlize, 0);
                  else           a->reverse();
                 break;

               // %>U
               case 'U':
                  if (appender)  a->append(us->c_ustring(), (U_TRANSLATE_FUNCTION) URLize, 0);
                  else           a->reverse();
                 break;

               // %>J
               case 'J':
                  if (appender)  a->append(us->c_ustring(), (U_TRANSLATE_FUNCTION) u_jsonize, 0);
                  else           a->reverse();
                 break;

               // %>X
               case 'X':
                  if (appender)  a->append(us->c_ustring(), (U_TRANSLATE_FUNCTION) u_escape, 0) ;
                  else           a->reverse();
                 break;

               // %>L
               case 'L':
                  if (appender)  a->append(us->c_ustring(), (U_TRANSLATE_FUNCTION) u_toupper, 0);
                  else           a->upper();
                 break;

               // %>l
               case 'l':
                  if (appender)  a->append(us->c_ustring(), (U_TRANSLATE_FUNCTION) u_tolower, 0);
                  else           a->lower();
                 break;

               // %>t
               case 't':
                  if (appender)  a->append(us->c_ustring(), (U_TRANSLATE_FUNCTION) u_totitle, 0);
                  else           a->title();
                 break;

               // %>f
               case 'f':
                  if (appender)  a->append(us->c_ustring(), (U_TRANSLATE_FUNCTION) u_tofold, 0);
                  else           a->title();
                 break;

               // %>u
               case 'u':
                  if (appender)  a->append(us->c_ustring(), (U_TRANSLATE_FUNCTION) u_deaccentuate, 0);
                  else           a->deaccentuate();
                 break;

               default:
                   char msg[64];
                   sprintf(msg, "invalid format option %c%c",*(strfrmt-1),*strfrmt);
                   luaL_argerror(L, arg, msg);
                 break;
             }
             if(strfrmt < strfrmt_end && *(strfrmt+1) == '>') {
               strfrmt++;
               appender = 0;
               goto next_strfrm_modifier;
             } else if (!a->is_empty()) {
               b->append(a->c_ustring());
             }
             break;
           }

           // not supported for now
           // case 'n':

           /* If we have '%???', we let sprintf do the job */
           default: {
              /* We get back on the '%' */
              strfrmt--;
              int z=0;
              char form[U_MAX_FMTSPEC];  /* to store the format (`%...') */
              char buff[U_MAX_FMTITEM];  /* to store the formatted item */
              do {
                 form[z++]=*strfrmt;
                 strfrmt++;
              } while (form[z-1]!='\0' && !strchr("diouxXeEfgGqcsp",form[z-1]));
              /* We get back one character */
              strfrmt--;
              if (form[z-1]=='\0') {
                char msg[64];
                sprintf(msg, "invalid format option %s",form);
                luaL_argerror(L, arg, msg);
              }
              form[z]='\0';
              int l=0;
              switch (form[z-1]) {
                 case 'd': case 'i': {
                    n=luaL_checkinteger(L, arg);
                    l=sprintf(buff,form,(LUA_INTFRM_T)n);
                    break;
                 }
                 case 'o': case 'u': case 'x': case 'X': {
                   n=luaL_checknumber(L, arg);
                   l=sprintf(buff,form,(unsigned LUA_INTFRM_T)n);
                   break;
                 }
                 case 'e': case 'E': case 'f':
                 case 'g': case 'G': {
                    d=luaL_checknumber(L, arg);
                    l=sprintf(buff,form,d);
                    break;
                 }
                 case 'q': {
                   s=luaL_checkstring(L, arg);
                   l=u_quotize(s,buff);
                   break;
                 }
                 case 'c': {
                    c=(char)luaL_checkint(L, arg);
                    l=sprintf(buff,form,c);
                    break;
                 }
                 case 's': {
                    s=luaL_checkstring(L, arg);
                    l=sprintf(buff,form,s);
                    break;
                 }
                 case 'p': {
                    p=(void*)lua_topointer(L, arg);
                    l=sprintf(buff,form,p);
                    break;
                 }
              }
              if (l) {
                b->append(U_ENCODE_UTF8, buff, l);
              }
              break;
           }
        }
     } else {
        size_t readed = 0;
        // a normal character, we append it
        b->append(U_ENCODE_UTF8, strfrmt, 1, &readed);
        strfrmt = strfrmt + readed - 1;
     }
     strfrmt++;
  }

  lua_pop(L,1);
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
int elg_ustring_encode(lua_State* L) {   \
  UnitexString* str = lua_checkudata_cast(L, 1, UnitexString);
  pushustring(L, str->c_unichar());
  return 1;
}
/* ************************************************************************** */
/* static */ int elg_ustring_decode_(lua_State* L, int numArg) {
  // length of the source string
  size_t length;
  // return the string at the top of stack if any
  const char* source = luaL_checklstring(L, numArg, &length);
  // decode source as UTF-8
  lua_pushlightobject(L, UnitexString)(U_ENCODE_UTF8, source, length);
  // number of values returned
  return 1;
}
/* ************************************************************************** */
/* static */ int elg_ustring_decode(lua_State* L) {
  return elg_ustring_decode_(L, -1);
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
/* static */ int elg_ustring___concat(lua_State* L) {
  // Checks whether the arguments are uStrings. testudata works like checkudata,
  // except that, when the test fails, it returns NULL instead of throwing an error
  UnitexString* strptr1 = lua_testudata_cast(L, 1, UnitexString);
  UnitexString* strptr2 = lua_testudata_cast(L, 2, UnitexString);

  // if both strings are of type uString
  if (strptr1 && strptr2) {
    // push a new string resulting of the concatenation of both strings
    lua_pushlightobject(L, UnitexString)(*strptr1 + *strptr2);
    return 1;
  }

  // at least one pointer is a uString, convert it to string and
  // let lua_concat to perform the concatenation
  //
  // (?): unknown type   =>  strptr == null
  // (S): uString type   =>  strptr != null
  // (s): string type
  //
  // stack:
  //  strptr1(?)
  //  strptr2(?)
  //
  // swap if strptr1 == null   | push if strptr1 != null
  //
  //                           |  strptr1(S)
  //  strptr2(S)               |  strptr2(?)
  //  strptr1(?)               | *strptr1(s)
  //
  // push if strptr2 != null   | swap if strptr2 == null
  //
  //  strptr2(S)               |  strptr1(S)
  //  strptr1(?)               | *strptr1(s)
  //  strptr2(s)               |  strptr2(?)
  //
  // top n=2                   | top n=2
  //  strptr1(?)               | *strptr1(s)
  // *strptr2(s)               |  strptr2(?)

  // put strptr1 as string at the top of the stack
  pushustring_or_insert(L, -2, strptr1);
  // put strptr2 as string at the top of the stack
  pushustring_or_insert(L, -2, strptr2);

  // concatenates the 2 values at the top of the stack
  // at least one of them being a string
  lua_concat(L,2);
  return 1;
}
/* ************************************************************************** */
/* static */ int elg_ustring_self_format(lua_State* L) {
  elg_stack_dump(L);
  // swap the first argument, the caller itself
  lua_insert(L, -2);
  elg_stack_dump(L);
  return elg_ustring_format(L);
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
/* static */ const struct luaL_Reg lua_elg_lib_functions[] = {
  U__DECLARE__FUNCTION__ENTRY__(USTRING, rep),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, format),
  {NULL, NULL}
};
/* ************************************************************************** */
/* static */ const struct luaL_Reg lua_elg_lib_methods[] = {
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
  U__DECLARE__FUNCTION__ENTRY__ALIAS__(USTRING, self_format, format),

  //
  U__DECLARE__FUNCTION__ENTRY__(USTRING, byte),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, char),

  //
  U__DECLARE__FUNCTION__ENTRY__(USTRING, decode),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, encode),
  U__DECLARE__FUNCTION__ENTRY__ALIAS__(USTRING, encode, string),

  // methamethods
  U__DECLARE__FUNCTION__ENTRY__(USTRING, __call),
  U__DECLARE__FUNCTION__ENTRY__ALIAS__(USTRING, len, __len),
  U__DECLARE__FUNCTION__ENTRY__ALIAS__(USTRING, encode, __tostring),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, __concat),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, __lt),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, __le),
  U__DECLARE__FUNCTION__ENTRY__(USTRING, __eq),

  // allow to release any resource associated with the userdata object
  U__DECLARE__GC__ENTRY__(UnitexString),
  {NULL, NULL}
};
/* ************************************************************************** */
U__DECLARE__FUNCTION__ELG__OPEN__EXTENSION__WITH__FUNCTIONS__AND__METHODS__(USTRING);
/* ************************************************************************** */
}  // namespace unitex::elg::ustring::{unnamed}
/* ************************************************************************** */
}  // namesapce unitex::elg::ustring
/* ************************************************************************** */
}  // namespace unitex::elg
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#if UNITEX_COMPILER_AT_LEAST(MSVC,15,0)
#pragma warning(pop)
#endif  // UNITEX_COMPILER_AT_LEAST(MSVC,15,0)
/* ************************************************************************** */
#endif // UNITEX_ELGLIB_USTRING_H_
