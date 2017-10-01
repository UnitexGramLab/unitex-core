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
#ifndef ELG_API_H_
#define ELG_API_H_
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
#include "MorphologicalLocate.h"
#include "Pattern.h"
#include "LocatePattern.h"
#include "Stack_unichar.h"
#include "TransductionVariables.h"
#include "UnitexString.h"
#include "Text_parsing.h"
#include "TransductionStack.h"
#include "Fst2.h"
#include "Text_parsing.h"
#include "Tokenization.h"
#include "Ustring.h"
/* ************************************************************************** */
#define ELG_ENVIRONMENT_PREFIX                 "elg"
/* ************************************************************************** */
#define ELG_EXTENSION_EVENT_LOAD               "load_event"
#define ELG_EXTENSION_EVENT_UNLOAD             "unload_event"
#define ELG_FUNCTION_ON_FAIL_NAME              "fail_event"
/* ************************************************************************** */
#define ELG_FUNCTION_DEFAULT_EXTENSION         ".upp"
/* ************************************************************************** */
#define ELG_GLOBAL_LOCATE_PARAMS               "uLocateParams"
/* ************************************************************************** */
#define ELG_GLOBAL_LOCATE                      "uLocate"
#define ELG_GLOBAL_LOCATE_STATE                "State"
#define ELG_GLOBAL_LOCATE_POS                  "Pos"
#define ELG_GLOBAL_LOCATE_MATCHES              "Matches"
#define ELG_GLOBAL_LOCATE_NUM_MATCHES          "NumMatches"
#define ELG_GLOBAL_LOCATE_CONTEXT              "Context"
#define ELG_GLOBAL_LOCATE_PARAMS_SIZE          5
/* ************************************************************************** */
#define ELG_GLOBAL_MORPHO_LOCATE               "uMorphoLocate"
#define ELG_GLOBAL_MORPHO_LOCATE_STATE_INDEX   "StateIndex"
#define ELG_GLOBAL_MORPHO_LOCATE_POS_IN_TOKENS "PosInTokens"
#define ELG_GLOBAL_MORPHO_LOCATE_POS_IN_CHARS  "PosInChars"
#define ELG_GLOBAL_MORPHO_LOCATE_MATCHES       "Matches"
#define ELG_GLOBAL_MORPHO_LOCATE_NUM_MATCHES   "NumMatches"
#define ELG_GLOBAL_MORPHO_LOCATE_CONTEXT       "Context"
#define ELG_GLOBAL_MORPHO_LOCATE_PARAMS        "Params"
#define ELG_GLOBAL_MORPHO_LOCATE_JAMO          "Jamo"
#define ELG_GLOBAL_MORPHO_LOCATE_POS_IN_JAMO   "PosInJamo"
#define ELG_GLOBAL_MORPHO_LOCATE_CONTENT       "Content"
#define ELG_GLOBAL_MORPHO_LOCATE_PARAMS_SIZE   10
/* ************************************************************************** */
#define ELG_GLOBAL_TOKEN                       "uToken"
#define ELG_GLOBAL_TOKEN_BIT_MASK              "BITMASK"
#define ELG_GLOBAL_TOKEN_META                  "META"
#define ELG_GLOBAL_TOKEN_U_SPACE               "SPACE"
#define ELG_GLOBAL_TOKEN_U_SENTENCE            "SENTENCE"
#define ELG_GLOBAL_TOKEN_U_STOP                "STOP"
/* ************************************************************************** */
#define ELG_GLOBAL_U_BUFFER_SIZE               "BUFFER_SIZE"
/* ************************************************************************** */
#define ELG_GLOBAL_PARSER                      "uParser"
/* ************************************************************************** */
#define ELG_GLOBAL_VARIABLE                    "uVariable"
/* ************************************************************************** */
#define ELG_GLOBAL_MATCH                       "uMatch"
//#define ELG_GLOBAL_TEXT                        "uText"
#define ELG_GLOBAL_CONSTANT                    "uConstant"
#define ELG_GLOBAL_STRING                      "uString"
/* ************************************************************************** */
#define ELG_GLOBAL_ENVIRONMENT                 "uEnvironment"
#define ELG_ENVIRONMENT_LOADED                 "uLoaded"
#define ELG_ENVIRONMENT_CALLED                 "uCalled"
#define ELG_ENVIRONMENT_VALUES                 "uValues"
/* ************************************************************************** */
static const unichar ELG_FUNCTION_KEYWORD_NIL[]    = { 'n' , 'i' , 'l',          };
static const unichar ELG_FUNCTION_KEYWORD_TRUE[]   = { 't' , 'r' , 'u', 'e'      };
static const unichar ELG_FUNCTION_KEYWORD_FALSE[]  = { 'f' , 'a' , 'l', 's', 'e' };
#define ELG_FUNCTION_KEYWORD_NIL_SIZE    3
#define ELG_FUNCTION_KEYWORD_TRUE_SIZE   4
#define ELG_FUNCTION_KEYWORD_FALSE_SIZE  5
#define ELG_FUNCTION_KEYWORD_MATCH(_keyword, _str, _length)          \
        ((_length == ELG_FUNCTION_KEYWORD_##_keyword##_SIZE)       &&\
         (!memcmp(_str, ELG_FUNCTION_KEYWORD_##_keyword,             \
          ELG_FUNCTION_KEYWORD_##_keyword##_SIZE * sizeof(unichar))))
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
namespace elg {
namespace {   // namespace elg::{unnamed}, enforce one-definition-rule

struct locate_parameters* get_locate_params(lua_State * L) {
  // put the value associated with uLocateParams into the top of the stack
  lua_getglobal(L, ELG_GLOBAL_LOCATE_PARAMS);

  // returns the light userdata pointer. Otherwise, returns NULL
  struct locate_parameters* p = (struct locate_parameters*) lua_touserdata(L,-1);

  // remove the returned value from the top of the stack
  lua_pop(L, 1);

  return p;
}

//void set_locate_params(lua_State * L, struct locate_parameters* p) {
//  lua_pushlightuserdata(L, p);
//  lua_setglobal(L, ELG_GLOBAL_LOCATE_PARAMS);
//}

/* ************************************************************************** */
}    // namespace elg::{unnamed}
/* ************************************************************************** */
namespace parser {
/* ************************************************************************** */
namespace {   // namespace elg::match::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
int setpos(lua_State * L) {
  // get locate parameters
  struct locate_parameters* p = get_locate_params(L);

  // check if there is a single argument on the stack
  if (p && lua_gettop(L) == 1) {
    // check if the argument is an integer and cast it
    int pos = luaL_checkint(L, 1);
    // allow negative indices, -1 is p->buffer_size-1
    pos = pos >= 0 ? pos : p->buffer_size + pos;
    // only if is a valid range
    if (pos >= 0 && pos < p->buffer_size) {
      p->current_origin = pos;
    }
  }
  // number of results
  return 1;
}
/* ************************************************************************** */
}  // namespace elg:parser::{unnamed}
/* ************************************************************************** */
}  // namespace elg:parser
/* ************************************************************************** */

/* ************************************************************************** */
namespace match {
/* ************************************************************************** */
namespace {   // namespace elg::match::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
int begin(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);

  if(p) {
    lua_pushinteger(L, p->current_origin);
  } else {
    lua_pushnil(L);
  }

  return 1;
}

// FIXME(martinec) same as current
int end(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);

  if(p) {
    lua_pushinteger(L, p->current_origin + p->last_tested_position);
  } else {
    lua_pushnil(L);
  }

  return 1;
}

int content(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);
  if(p) {
//    struct parsing_info* DIC_consultation = NULL;
//    int start = p->current_origin;
    p->elg_stack->stack_pointer = -1;

    struct parsing_info* matches = NULL;
    struct parsing_info* it = NULL;

    // p->current_origin <  p->pos_in_tokens =  [p->current_origin, p->pos_in_tokens-1]
    // p->current_origin == p->pos_in_tokens =  [p->current_origin[0],p->current_origin[pos_in_chars]]

    unichar* S = get_token_sequence(p,p->current_origin,p->current_origin + p->pos_in_tokens-1);
//    u_printf("%S\n",S);
//    u_printf("%d,%d,%d\n",p->current_origin,p->current_origin + p->lti->pos_in_tokens-1,p->pos_in_chars);
//    u_printf("%d,%d,%d\n",p->current_origin,p->current_origin + p->pos_in_tokens-1,p->pos_in_chars);

    explore_dic_in_morpho_mode_with_token(p,
                                          S,
                                          0,
                                          &matches,
                                          NULL,
                                          1,
                                          NULL,
                                          0,
                                          NULL);

    it = matches;
    if (it != NULL) {
      do {
//        for (int i=0; S[i]!='\0'; ++i) {
//         ::push(p->elg_stack,S[i]);
//        }

        S = it->dic_entry->semantic_codes[3];
        for (int i=0; S[i]!='\0'; ++i) {
         ::push(p->elg_stack,S[i]);
        }
        it = it->next;
      } while(it != NULL);

      p->elg_stack->stack[p->elg_stack->stack_pointer + 1] = '\0';

      free_parsing_info(matches, &p->al.pa);

//      p->elg_stack->stack[p->elg_stack->stack_pointer+1]='\0';

      lua_pushlightuserdata(L,p->elg_stack->stack);
    } else {
//      u_printf("%S\n",get_token_sequence(p,p->current_origin,p->current_origin + p->pos_in_tokens-1));
      lua_pushnil(L);
    }

//    int end   = p->current_origin + 1;
//    const unichar* S = NULL;
//    for(int i=start; i<end ; ++i ){
//     S = p->tokens->value[p->buffer[i]];
//     u_printf("%S ",S);
//    }
//    u_printf("\n");

  }

  return 1;
}

int length(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);

  if(p) {
    lua_pushinteger(L, p->last_tested_position + 1);
  } else {
    lua_pushnil(L);
  }

  return 1;
}

int start_with_space(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);
  int start_with_space = 0;

  if (p) {
    int pos = p->current_origin;
    if (pos == 0 || (pos > 0 && p->buffer[pos-1] == p->SPACE)) {
      start_with_space = 1;
    }
  }

  lua_pushboolean(L,start_with_space);

  return 1;
}

int start_newline(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);
  int start_newline = 0;

  if (p && p->enter_pos) {
    p->no_fail_fast=1;
    int pos = p->current_origin;
    if(pos == 0 ||
      (pos == 1 && p->buffer[0]     == p->SPACE)    ||
      (pos >  0 && get_value(p->enter_pos,pos-1))   ||
      (pos >  1 && p->buffer[pos-1] == p->SPACE     &&
                   get_value(p->enter_pos,pos-2))) {
      start_newline = 1;
    }
  }

  lua_pushboolean(L,start_newline);

  return 1;
}

int start_sentence(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);
  int start_sentence = 0;

  if(p) {
    p->no_fail_fast=1;
    int pos = p->current_origin;
    if(pos == 0 ||
      (pos == 1 && p->buffer[0]     == p->SPACE)    ||
      (pos >  0 && p->buffer[pos-1] == p->SENTENCE) ||
      (pos >  1 && p->buffer[pos-1] == p->SPACE     &&
                   p->buffer[pos-2] == p->SENTENCE)) {
      start_sentence = 1;
    }

//    if(pos == 0) {
//      start_sentence = 1;
//    } else {
//      while(pos > 1 && get_value(p->failfast,p->buffer[pos-1])) {
//        pos = pos - 1;
//      }
//
//      while(pos > 1 && p->buffer[pos-1] == p->SPACE) {
//        pos = pos - 1 ;
//      }
//
//      start_sentence = pos > 0 && p->buffer[pos-1] == p->SENTENCE;
//
////      while(!start_sentence && pos > 1 &&
////           (p->buffer[pos-1] == p->SPACE ||
////            get_value(p->failfast,p->buffer[pos-1]))) {
////            pos = pos - 1;
////            start_sentence = p->buffer[pos] == p->SENTENCE;
////          }
////      if(pos == 0 ||
////        (pos == 1 && p->buffer[0]     == p->SPACE)    ||
////        (pos >  0 && p->buffer[pos-1] == p->SENTENCE) ||
////        (pos >  1 && p->buffer[pos-1] == p->SPACE     &&
////                     p->buffer[pos-2] == p->SENTENCE)) {
////        start_sentence = 1;
////      }
//
//    }
  }

  //




  lua_pushboolean(L,start_sentence);

  return 1;
}

int end_sentence(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);
  int end_sentence = 0;

  if(p) {
    p->no_fail_fast=0;
    int pos = p->current_origin + p->last_tested_position;
    if( pos == p->buffer_size - 1 ||
       (pos == p->buffer_size - 2 && p->buffer[pos+1] == p->SPACE)    ||
       (pos <  p->buffer_size - 2 && p->buffer[pos+1] == p->SENTENCE) ||
       (pos <  p->buffer_size - 3 && p->buffer[pos+1] == p->SPACE     &&
                                     p->buffer[pos+2] == p->SENTENCE)) {
      end_sentence = 1;
    }
  }

  lua_pushboolean(L,end_sentence);

  return 1;
}

int end_newline(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);
  int end_newline = 0;

  if(p && p->enter_pos) {
    p->no_fail_fast=0;
    int pos = p->current_origin + p->last_tested_position;
    if( pos == p->buffer_size - 1 ||
       (pos == p->buffer_size - 2 && p->buffer[pos+1] == p->SPACE)    ||
       (pos <  p->buffer_size - 2 && get_value(p->enter_pos,pos+1))   ||
       (pos <  p->buffer_size - 3 && p->buffer[pos+1] == p->SPACE     &&
                                     get_value(p->enter_pos,pos+2))) {
      end_newline = 1;
    }
  }

  lua_pushboolean(L,end_newline);

  return 1;
}

/* ************************************************************************** */
}  // namespace elg:match::{unnamed}
/* ************************************************************************** */
}  // namespace elg:match
/* ************************************************************************** */

/* ************************************************************************** */
namespace dummy {
/* ************************************************************************** */
namespace {   // namespace elg::dummy::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
}  // namespace elg:dummy::{unnamed}
/* ************************************************************************** */
}  // namespace elg:dummy
/* ************************************************************************** */

/* ************************************************************************** */
namespace string {
/* ************************************************************************** */
namespace {   // namespace elg::string::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
int format(lua_State* L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);
  if(p) {
    lua_pushstring(L,"XaXa");
  }
  return 1;
}
/* ************************************************************************** */
}  // namespace elg:string::{unnamed}
/* ************************************************************************** */
}  // namespace elg:string
/* ************************************************************************** */


/* ************************************************************************** */
namespace core {
/* ************************************************************************** */
namespace {   // namespace elg::string::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */

/* ************************************************************************** */
}  // namespace elg:core::{unnamed}
/* ************************************************************************** */
}  // namespace elg:core
/* ************************************************************************** */

namespace token {
/* ************************************************************************** */
namespace {   // namespace elg::token::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.

int at(lua_State* L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);

  // lua_gettop returns the index of the top element
  // in the stack. Because indices start at 1, this
  // result is equal to the number of elements in the
  // stack (and so 0 means an empty stack)
  // @see http://pgl.yoyo.org/luai/i/lua_gettop
  if (p && lua_gettop(L) > 0) {
    // luaL_checkint checks whether the function argument
    // narg is a number and returns this number cast to an int
    // http://pgl.yoyo.org/luai/i/luaL_checkint
    int pos = luaL_checkint(L, 1);
    // allow negative indices, -1 is p->buffer_size-1
    pos = pos >= 0 ? pos : p->buffer_size + pos;

    if (pos >= 0 && pos < p->buffer_size) {
      lua_pushlightuserdata(L, p->tokens->value[p->buffer[pos]]);
    } else {
      lua_pushnil(L);
    }
  }
  // number of results
  return 1;
}

// FIXME(martinec) same as pos() and end()
int current(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);

  if(p) {
    lua_pushinteger(L, p->current_origin + p->last_tested_position);
  } else {
    lua_pushnil(L);
  }

  return 1;
}

int previous(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);

  if(p && (p->current_origin + p->last_tested_position) > 0) {
    lua_pushinteger(L,p->current_origin + p->last_tested_position - 1);
  } else {
    lua_pushnil(L);
  }

  return 1;
}

int next(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);

  if(p && (p->current_origin + p->last_tested_position) < p->buffer_size - 1 ) {
    lua_pushinteger(L,p->current_origin + p->last_tested_position + 1);
  } else {
    lua_pushnil(L);
  }

  return 1;
}

// FIXME(martinec) same as current
int pos(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);

  if(p) {
    lua_pushinteger(L, p->current_origin + p->last_tested_position);
  } else {
    lua_pushnil(L);
  }

  return 1;
}

// gives the index at the text buffer whereas pos gives the pos at the token buffer
int offset(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);

  if(p) {
    int pos = p->pos_in_tokens;
    int ofsset = pos > 0 ? pos + p->current_origin-1 : pos + p->current_origin;
    lua_pushinteger(L, ofsset);
  } else {
    lua_pushnil(L);
  }

  return 1;
}
/* ************************************************************************** */
int value(lua_State* L) {
  // get locate parameters
  struct locate_parameters* p = get_locate_params(L);

  // check if there is a single argument on the stack
  if (p && lua_gettop(L) == 1) {
    // check if the argument is an integer and cast it
    int pos = luaL_checkint(L, 1);
    // allow negative indices, -1 is p->buffer_size-1
    pos = pos >= 0 ? pos : p->buffer_size + pos;
    // only if is a valid range
    if (pos >= 0 && pos < p->buffer_size) {
      char char_token[MAX_TOKEN_LENGTH] = {0};
      int char_token_length = u_encode_utf8(p->tokens->value[p->buffer[pos]], char_token);
      lua_pushlstring(L, char_token, char_token_length);
    } else {
      lua_pushnil(L);
    }
  }
  // number of results
  return 1;
}
/* ************************************************************************** */
int reference(lua_State* L) {
  // get locate parameters
  struct locate_parameters* p = get_locate_params(L);

  // check if there is a single argument on the stack
  if (p && lua_gettop(L) == 1) {
    // check if the argument is an integer and cast it
    int pos = luaL_checkint(L, 1);
    // allow negative indices, -1 is p->buffer_size-1
    pos = pos >= 0 ? pos : p->buffer_size + pos;
    // only if is a valid range
    if (pos >= 0 && pos < p->buffer_size) {
      lua_pushinteger(L, p->buffer[pos]);
    } else {
      lua_pushnil(L);
    }
  }
  // number of results
  return 1;
}
/* ************************************************************************** */
int lookup(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);

  if(p) {
    // return the list of dictionaries to use
    const char* c_dicos = luaL_checkstring(L, 1);

    // length of the input string
    size_t input_length = 0;
    // return the input
    const char* c_input = luaL_checklstring(L, 2, &input_length);
    // convert to unicode string
    UnitexString u_input(UTF8, c_input, input_length);

    // length of the pattern string
    size_t pattern_length = 0;
    // return the pattern if any
    const char* c_pattern = luaL_optlstring(L, 3, NULL, &pattern_length);
    // convert to unicode string
    UnitexString u_pattern(UTF8, c_pattern, pattern_length);

    // length of the variable string
    size_t variable_length = 0;
    // return the variable if any
    const char* c_variable = luaL_optlstring(L, 4, NULL, &variable_length);
    // convert to unicode string
    UnitexString u_variable(UTF8, c_variable, variable_length);

    // build the pattern
    struct pattern* pattern = u_pattern.is_empty() ? NULL :
                               build_pattern(u_pattern.c_unichar(),
                                             NULL,
                                             p->tilde_negation_operator,
                                             p->al.prv_alloc_generic);

    //
    struct parsing_info* matches = NULL;

    // try to find an entry that matches the given input and pattern
    // in a morphological dictionary
    explore_dic_in_morpho_mode_with_token(p,
                                          u_input.c_unichar(),
                                          0,
                                          &matches,
                                          pattern,
                                          u_variable.is_empty() ? 0 : 1,
                                          NULL,
                                          0,
                                          c_dicos);
    free_pattern(pattern);

    //unichar* S = NULL;
    if (matches) {
      struct parsing_info* it = matches;
//      do {
//      } while(it != NULL);
      if (!u_variable.is_empty()) {
        set_dic_variable(u_variable.c_unichar(), it->dic_entry, &(p->dic_variables),1);
      }
      free_parsing_info(matches, &p->al.pa);
      lua_pushboolean(L, 1);
    } else {
      lua_pushboolean(L, 0);
    }

  }

//  8474 matches
//  81636 recognized units
//  (28.160% of the text is covered)
//  2515018 exploration step
//  Done.

  return 1;
}
/* ************************************************************************** */
//int lookup(lua_State * L) {
//  // get locate params
//  struct locate_parameters* p = get_locate_params(L);
//  if(p) {
//    // length of the input string
//    size_t input_length;
//    // return the first argument
//    const char* c_input = luaL_checklstring(L, 1, &input_length);
//    // convert to unicode string
//    UnitexString u_input(UTF8, c_input, input_length);
//
//    // length of the pattern string
//    size_t pattern_length;
//    // return the second argument if any
//    const char* c_pattern = luaL_optlstring(L, 2, NULL, &pattern_length);
//    // convert to unicode string
//    UnitexString u_pattern(UTF8, c_pattern, pattern_length);
//
//    struct pattern*  pattern = build_pattern(u_pattern.c_unichar(),
//                                             NULL,
//                                             p->tilde_negation_operator,
//                                             p->al.prv_alloc_generic);
//
//    p->elg_stack->stack_pointer = -1;
//
//    struct parsing_info* matches = NULL;
//    struct parsing_info* it = NULL;
//
//    explore_dic_in_morpho_mode_with_token(p,
//                                          u_input.c_unichar(),
//                                          0,
//                                          &matches,
//                                          pattern,
//                                          1,
//                                          NULL,
//                                          0);
//    free_pattern(pattern);
//
//    unichar* S = NULL;
//    it = matches;
//    if (it != NULL) {
//      do {
////        for (int i=0; S[i]!='\0'; ++i) {
////         ::push(p->elg_stack,S[i]);
////        }
//        S = it->dic_entry->semantic_codes[4];
////        u_printf("%S\n",S);
//        for (int i=0; S[i]!='\0'; ++i) {
//         ::push(p->elg_stack,S[i]);
//        }
//        it = it->next;
//      } while(it != NULL);
//
//      p->elg_stack->stack[p->elg_stack->stack_pointer + 1] = '\0';
//
//      free_parsing_info(matches, &p->al.pa);
//
////      p->elg_stack->stack[p->elg_stack->stack_pointer+1]='\0';
//
//      lua_pushlightuserdata(L,p->elg_stack->stack);
//    } else {
//      lua_pushnil(L);
//    }
//
//  }
//
//  return 1;
//}
/* ************************************************************************** */
int bitmask(lua_State * L) {
  struct locate_parameters* p = get_locate_params(L);

  if (p && lua_gettop(L) > 0) {
    int pos = lua_tointeger(L, 1);
    pos = pos >= 0 ? pos : p->buffer_size + pos;
    if (pos >= 0 && pos < p->buffer_size) {
      lua_pushinteger(L, p->token_control[pos]);
      return 1;
    }
  }

  lua_pushnil(L);
  return 1;
}
/* ************************************************************************** */
// adapted from locate() @see Text_parsing.cpp
int match_meta(const struct locate_parameters* p,
                int pos,
                int pos_shift,
                int negation,
                enum meta_symbol meta) {
  // return value
  int has_meta = 0;

  // we only use the current token at the current pos
  int token = -1;

  // if we have NOT reached the end of the token buffer
  if (!(((pos + pos_shift) >= p->buffer_size)) || (pos == -1)) {
    token = p->buffer[pos + pos_shift];
  }

  // get the control byte of this token
  int ctrl = (token != -1) ? p->token_control[token] : 0;

  // process meta
  switch (meta) {
    case META_SHARP:
      return (token == -1 || token != p->SPACE);
      break;

    case META_SPACE:
      return ((!negation && (token != -1 && token == p->SPACE)) ||
                (negation && token != p->SPACE));
      break;

    case META_EPSILON:
      return negation;
      break;

    case META_TEXT_START:
      return (pos == 0 || (pos == 1 && p->buffer[0] == p->SPACE));
      break;

    case META_TEXT_END:
      return ((pos + pos_shift == p->buffer_size)
          || ((pos + pos_shift + 1 == p->buffer_size)
              && p->buffer[pos + pos_shift] == p->SPACE));
      break;

    case META_WORD:
    case META_MOT:
      if (token == p->SENTENCE || token == p->STOP) {
        return 0;
      }
      if (token == p->SPACE) {
        // we catch a space with <!MOT>
        return (negation != 0);
        // this differs from Locate, because we're dealing with tokens
        // outside of the analysis window
      } else if (XOR(negation, ctrl & MOT_TOKEN_BIT_MASK)) {
        return 1;
      }
      break;

    case META_DIC:
      if (token == -1 || token == p->STOP) {
        return 0;
      }
      if (!negation)  {
        // only for simple words
        return (ctrl & DIC_TOKEN_BIT_MASK);
      }
      // we have the meta <!DIC>
      if ((ctrl & NOT_DIC_TOKEN_BIT_MASK)) {
        return 1;
      }
      break;

    case META_SDIC:
      if (token == -1 && token == p->STOP) {
        return 1;
      }
      if ((ctrl & DIC_TOKEN_BIT_MASK) && !(ctrl & CDIC_TOKEN_BIT_MASK)) {
        return 1;
      }
      break;

    case META_CDIC:
      if (token == -1 || token == p->STOP) {
        return 0;
      }
      if (ctrl & CDIC_TOKEN_BIT_MASK) {
        return 1;
      }
      break;

    case META_TDIC:
      if (token == -1 || token == p->STOP) {
        return 0;
      }
      if (ctrl & TDIC_TOKEN_BIT_MASK) {
        return 1;
      }
      break;

    case META_UPPER:
    case META_MAJ:
      if (token == -1 || token == p->STOP) {
        return 0;
      }
      if (!negation) {
        return (ctrl & MAJ_TOKEN_BIT_MASK);
      }
      if (!(ctrl & MAJ_TOKEN_BIT_MASK) && (ctrl & MOT_TOKEN_BIT_MASK)) {
        /* If we have <!MAJ>, the matching token must be matched by <MOT> */
        return 1;
      }
      break;

    case META_LOWER:
    case META_MIN:
      if (token == -1 || token == p->STOP) {
        return 0;
      }
      if (!negation) {
        return (ctrl & MIN_TOKEN_BIT_MASK);
      }
      if (!(ctrl & MIN_TOKEN_BIT_MASK) && (ctrl & MOT_TOKEN_BIT_MASK)) {
        /* If we have <!MIN>, the matching token must be matched by <MOT> */
        return 1;
      }
      break;

    case META_FIRST:
    case META_PRE:
      if (token == -1 || token == p->STOP) {
        return 0;
      }
      if (!negation) {
        return (ctrl & PRE_TOKEN_BIT_MASK);
      }
      if (!(ctrl & PRE_TOKEN_BIT_MASK) && (ctrl & MOT_TOKEN_BIT_MASK)) {
        /* If we have <!PRE>, the matching token must be matched by <MOT> */
        return 1;
      }
      break;

    case META_NB:
      if (token == -1 || token == p->STOP) {
        return 0;
      }
      return (u_test_flag(
               p->tokens->value[p->buffer[pos + pos_shift]],
               U_FLAG_DIGIT) != 0);
      break;

    case META_LETTER:
      if (token == p->SENTENCE || token == p->STOP) {
        return 0;
      }
      /* If we want to catch a space with <!MOT> */
      if (token == p->SPACE && negation) {
        return 1;
        // this differs from Locate, because we're dealing with tokens
        // outside of the window of analysis
      } else if (token != p->SPACE
          && XOR(negation, ctrl & MOT_TOKEN_BIT_MASK)) {
        return 1;
      }
      break;

    case META_TOKEN:
      return (token != -1 && token != p->STOP);
      break;

    case META_BEGIN_MORPHO:
    case META_END_MORPHO:
    case META_LEFT_CONTEXT:
      return 0;
      break;
  }

  return has_meta;
}
/* ************************************************************************** */
int meta(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);
  int has_meta = 0;
  // if p is not null and the stack isn't empty
  if (p && lua_gettop(L) == 2) {
    // returns the position from the stack and cast to an int
    int pos = luaL_checkint(L, -2);
    // return the meta from the stack and cast it to a meta symbol
    enum meta_symbol meta = (enum meta_symbol) luaL_checkint(L, -1);
    has_meta = match_meta(p, pos, 0, 0, meta);
  }

  lua_pushboolean(L, has_meta);

  return 1;
}

int negmeta(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);
  int has_meta = 0;
  // if p is not null and the stack isn't empty
  if (p && lua_gettop(L) == 2) {
    // returns the position from the stack and cast to an integer
    int pos = luaL_checkint(L, -2);
    // return the meta from the stack and cast it to a meta symbol
    enum meta_symbol meta = (enum meta_symbol) luaL_checkint(L, -1);
    has_meta = match_meta(p, pos, 0, NEGATION_TAG_BIT_MASK, meta);
  }
  // push the return value to the stack
  lua_pushboolean(L, has_meta);
  return 1;
}
/* ************************************************************************** */
#define U__DECLARE__ELG__FUNCTION__META_TEST__(_name, _function)     \
int _name(lua_State * L) {                                           \
  struct locate_parameters* p = get_locate_params(L);                \
  int _name = 0;                                                     \
  if(p && lua_gettop(L) == 1) {                                      \
    int pos = luaL_checkint(L, -1);                                  \
    _name = _function;                                               \
  }                                                                  \
  lua_pushboolean(L,_name);                                          \
  return 1;                                                          \
}
/* ************************************************************************** */
U__DECLARE__ELG__FUNCTION__META_TEST__(is_space,match_meta(p, pos, 0, 0, META_SPACE))

/* ************************************************************************** */
#undef U__DECLARE__ELG__FUNCTION__META_TEST__
/* ************************************************************************** */

int tag(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);

  locate(p->optimized_states[p->fst2->initial_states[1]],
         p->current_origin + p->last_tested_position + 1, NULL,
         NULL, NULL, p);

//  // get locate params
//  struct locate_parameters* p = get_locate_params(L);
//  int is_tag = 0;
//
//  if(p && lua_gettop(L)>0) {
//    UnitexString tag_string(lua_tostring(L,1));
//    Fst2Tag* tag = create_tag(NULL,tag_string.c_unichar(),NULL);
//
//  }
//
//  lua_pushboolean(L,is_tag);
  return 1;
}


/* ************************************************************************** */
}  // namespace elg::token::{unnamed}
/* ************************************************************************** */
}  // namespace elg::token
/* ************************************************************************** */
}  // namespace elg
/* ************************************************************************** */
} // namespace unitex
/* ************************************************************************** */
#endif // ELG_API_H_
