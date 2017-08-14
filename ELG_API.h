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
/* ************************************************************************** */
#define ELG_ENVIRONMENT_PREFIX          "elg"
/* ************************************************************************** */
#define ELG_EXTENSION_EVENT_LOAD      "load_event"
#define ELG_EXTENSION_EVENT_UNLOAD    "unload_event"
#define ELG_FUNCTION_ON_FAIL_NAME     "fail_event"
/* ************************************************************************** */
#define ELG_FUNCTION_DEFAULT_EXTENSION  ".upp"
/* ************************************************************************** */
#define ELG_GLOBAL_LOCATE_STATE         "uLocateState"
#define ELG_GLOBAL_LOCATE_POS           "uLocatePos"
#define ELG_GLOBAL_LOCATE_MATCHES       "uLocateMatches"
#define ELG_GLOBAL_LOCATE_NUM_MATCHES   "uLocateNumMatches"
#define ELG_GLOBAL_LOCATE_CONTEXT       "uLocateContext"
#define ELG_GLOBAL_LOCATE_PARAMS        "uLocateParams"
/* ************************************************************************** */
#define ELG_GLOBAL_TOKEN                "uToken"
#define ELG_GLOBAL_TOKEN_BIT_MASK       "kBitMask"
#define ELG_GLOBAL_TOKEN_META           "kMeta"
/* ************************************************************************** */
#define ELG_GLOBAL_MATCH                "uMatch"
//#define ELG_GLOBAL_TEXT                 "uText"
#define ELG_GLOBAL_CONSTANT             "uConstant"
#define ELG_GLOBAL_STRING               "uString"
/* ************************************************************************** */
#define ELG_GLOBAL_ENVIRONMENT          "uEnvironment"
#define ELG_ENVIRONMENT_LOADED          "uLoaded"
#define ELG_ENVIRONMENT_CALLED          "uCalled"
#define ELG_ENVIRONMENT_VALUES          "uValues"
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
                                          0);

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
int match_meta(struct locate_parameters* p,
              int pos,
              int pos_shift,
              char negation,
              enum meta_symbol meta) {
  // return value
  int has_meta = 0;

  // auxiliary variables
  int pos2 = -1;

  // we only use the current token at the current pos
  int token = -1;


  int token2 = -1;

  /* If we have reached the end of the token buffer, we indicate it by setting
   * the current tokens to -1 */
  if ((((pos + pos_shift) >= p->buffer_size)) || (pos == -1)) {
    token = -1;
    token2 = -1;
  } else {
    token = p->buffer[pos + pos_shift];
    if (token == p->SPACE) {
      pos2 = pos + 1;
    }
    /* Now, we deal with the SPACE, if any. To do that, we use several variables:
     * pos: current position in the token buffer, relative to the current origin
     * pos2: position of the first non-space token from 'pos'.
     * token2: token at pos2  or -1 if 'pos2' is outside the token buffer */
    else {
      pos2 = pos;
    }
    if ((pos2 + pos_shift) >= p->buffer_size) {
      token2 = -1;
    } else {
      token2 = p->buffer[pos2 + pos_shift];
    }
  }

  int ctrl = 0;
  if (token2 != -1) {
    ctrl = p->token_control[token2];
  }

  switch (meta) {
    case META_SHARP:
      if (token == -1 || token != p->SPACE) {
        // true
      }
      break;

    case META_SPACE:
      if (token != -1 && token == p->SPACE) {
        return 1;
      }
      break;

    case META_EPSILON:
      return 0;
      break;

    case META_TEXT_START:
      if (pos == 0 || (pos == 1 && p->buffer[0] == p->SPACE)) {
        // true
      }
      break;

    case META_TEXT_END:
      if ((pos + pos_shift == p->buffer_size) ||
          ((pos + pos_shift + 1 == p->buffer_size) &&
            p->buffer[pos + pos_shift] == p->SPACE)) {
        // true
      }
      break;

    case META_WORD:
    case META_MOT:
      if (token2 == p->SENTENCE || token2 == p->STOP) {
        return 0;
        break;
      }
      /* If we want to catch a space with <!MOT> */
      if ((p->space_policy == START_WITH_SPACE) && (token2 == p->SPACE)
          && negation) {
        return 1;
      // this differs from Locate, because we're dealing with the tokens
      // outside of the window analysis
      } else if (token2 != p->SPACE && XOR(negation, ctrl & MOT_TOKEN_BIT_MASK)) {
        return 1;
      }
      break;

    case META_DIC:
      if (token2 == -1) {
        // false
        break;
      }
      if (token2 == p->STOP) {
        // false
        break;
      }
      if (!negation) {
        /* Now, we look for a simple word */
        if ((ctrl & DIC_TOKEN_BIT_MASK) && (1)) {
          // true
        }
        break;
      }
      /* We have the meta <!DIC> */
      if ((ctrl & NOT_DIC_TOKEN_BIT_MASK) && (1)) {
        // true
      }
      break;

    case META_SDIC:
      if (token2 == -1) {
        // false
        break;
      }
      if (token2 == p->STOP) {
        // false
        break;
      }
      if ((1) && (ctrl & DIC_TOKEN_BIT_MASK) && !(ctrl & CDIC_TOKEN_BIT_MASK)) {
        // true
      }
      break;

    case META_CDIC:
      if (token2 == -1) {
        // false
        break;
      }
      if (token2 == p->STOP) {
        // false
        break;
      }
      if ((1) && (ctrl & CDIC_TOKEN_BIT_MASK)) {
        // true
      }
      break;

    case META_TDIC:
      if (token2 == -1) {
        // false
        break;
      }
      if (token2 == p->STOP) {
        // false
        break;
      }
      if ((1) && (ctrl & TDIC_TOKEN_BIT_MASK)) {
        // true
      }
      break;

    case META_UPPER:
    case META_MAJ:
      if (token2 == -1) {
        // false
        break;
      }
      if (token2 == p->STOP) {
        // false
        break;
      }
      if (!negation) {
        if (ctrl & MAJ_TOKEN_BIT_MASK) {
          // true
        }
        break;
      }
      if (!(ctrl & MAJ_TOKEN_BIT_MASK) && (ctrl & MOT_TOKEN_BIT_MASK)) {
        /* If we have <!MAJ>, the matching token must be matched by <MOT> */
        // true
      }
      break;

    case META_LOWER:
    case META_MIN:
      if (token2 == -1) {
        // false
        break;
      }
      if (token2 == p->STOP) {
        // false
        break;
      }
      if (!negation) {
        if (ctrl & MIN_TOKEN_BIT_MASK) {
          // true
        }
        break;
      }
      if (!(ctrl & MIN_TOKEN_BIT_MASK) && (ctrl & MOT_TOKEN_BIT_MASK)) {
        /* If we have <!MIN>, the matching token must be matched by <MOT> */
        // true
      }
      break;

    case META_FIRST:
    case META_PRE:
      if (token2 == -1) {
        // false
        break;
      }
      if (token2 == p->STOP) {
        // false
        break;
      }
      if (!negation) {
        if (ctrl & PRE_TOKEN_BIT_MASK) {
          // true
        }
        break;
      }
      if (!(ctrl & PRE_TOKEN_BIT_MASK) && (ctrl & MOT_TOKEN_BIT_MASK)) {
        /* If we have <!PRE>, the matching token must be matched by <MOT> */
        // true
      }
      break;

    case META_NB:
      if (token2 == -1) {
        // false
        break;
      }
      if (token2 == p->STOP) {
        // false
        break;
      }
      { /* This block avoids visibility problem about 'z' */
        // local_is_not_a_digit_token return 1 if s is a digit sequence, 0 else
        if (!(pos2 + pos_shift < p->buffer_size
            && (u_test_flag(p->tokens->value[p->buffer[pos2 + pos_shift]],
                            U_FLAG_DIGIT) == 0))) {

          // false
          break;
        }

        /* If we have found a contiguous digit sequence */

        int z = pos2 + 1;
        int pos_limit = p->buffer_size - pos_shift;

        int next_pos_add = 0;
        while (z < pos_limit
            && ((next_pos_add = u_test_flag(
                p->tokens->value[p->buffer[z + pos_shift]], U_FLAG_DIGIT)) == 0)) {
          z++;
        }

        // If we have stopped because of the end of the buffer, next_pos_add = 0
        // If we have stopped because of a non matching token, next_pos_add = 1
        // true
      }
      break;

    case META_LETTER:
      if (token2 == p->SENTENCE || token2 == p->STOP) {
        // false
        break;
      }
      // len = length(token2)
      if ((p->space_policy == START_WITH_SPACE) && (token2 == p->SPACE)
          && negation) {
        // true
      } else if (XOR(negation, ctrl & MOT_TOKEN_BIT_MASK)) {
        // true
      }
      break;

    case META_TOKEN:
      if (token2 == -1) {
        // false
        break;
      }
      if (token2 == p->STOP) {
        // false
        /* The {STOP} tag must NEVER be matched by any pattern */
        break;
      }
      // true
      break;

    case META_BEGIN_MORPHO:
    case META_END_MORPHO:
    case META_LEFT_CONTEXT:
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
    has_meta = match_meta(p, pos, 0, (char) 0, meta);
  }

  lua_pushboolean(L ,has_meta);
  // number of results
  return 1;
}


int negmeta(lua_State * L) {

  return 1;
}

int is_space(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);
  int is_space = 0;

  if(p && lua_gettop(L)>0) {
    int pos = lua_tointeger(L,1);
    if(pos >= 0 && p->buffer[pos] == p->SPACE) {
       is_space = 1;
    }
  }

  lua_pushboolean(L,is_space);
  return 1;
}
/* ************************************************************************** */

int tag(lua_State * L) {
  // get locate params
  struct locate_parameters* p = get_locate_params(L);

  locate(/*graph_depth,*/
          p->optimized_states[p->fst2->initial_states[1]],
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
