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
#ifndef ELG_H_
#define ELG_H_
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
#include "ELG_API.h"
#include "ELGLib/common.h"
#include "ELGLib/ELGLib.h"
#include "ELGLib/copy_state.h"
#include "File.h"
#include "DebugMode.h"
#include "base/unicode/utf8.h"
#include "TransductionStack.h" // ExtendedOutputRender
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
#if UNITEX_OS_IS(UNIX)
# define UNITEX_EXTENSIONS_PATH "path = 'extensions/?.lua';cpath = 'extensions/?.so'\n"
#else
# define UNITEX_EXTENSIONS_PATH "path = 'extensions\\\\?.lua';cpath = 'extensions\\\\?.dll'\n"
#endif
/* ************************************************************************** */
static const char* UNITEX_SCRIPT_PATH =
    "/data/devel/projects/UnitexGramLab/unitex-core-elg/unitex-core/bin/Scripts/";
/* ************************************************************************** */
// LUA_REGISTRYINDEX  : interpreter environment
// LUA_GLOBALSINDEX   : thread environment
// LUA_ENVIRONINDEX   : function environment
/* ************************************************************************** */
enum variable_pass_type_t {
  VARIABLE_PASS_BY_VALUE,
  VARIABLE_PASS_BY_REFERENCE
};

enum param_type_t {
  PARAM_TNONE           = -1,
  PARAM_TNIL            =  0,
  PARAM_TBOOLEAN        =  1,
  PARAM_TLIGHTUSERDATA  =  2,
  PARAM_TNUMBER         =  3,
  PARAM_TSTRING         =  4,
  PARAM_TTABLE          =  5,
  PARAM_TFUNCTION       =  6,
  PARAM_TUSERDATA       =  7,
  PARAM_TTHREAD         =  8,
  PARAM_TLIGHTUSTRING   =  9,
};
/* ************************************************************************** */
typedef struct elg_Event {
  const char* name;
  int nargs;
  int nresults;
} elg_Event;
/* ************************************************************************** */
static const struct elg_Event elgMainEvents [] = {
/* name        ,  nargs, nresults */
  {"load_event",      0, 0 }, /* (load_event)   =         */
  {"unload_event",    0, 0 }, /* (unload_event) =         */
  {"slide_event",     1, 1 }, /* (index)        = index   */
  {"token_event",     1, 1 }, /* (index)        = index   */
  {NULL, 0}                   /* sentinel                 */
};
/* ************************************************************************** */
#define ELG_MAIN_EVENTS_COUNT              4
#define ELG_MAIN_EVENT_LOAD                0
#define ELG_MAIN_EVENT_UNLOAD              1
#define ELG_MAIN_EVENT_SLIDE               2
#define ELG_MAIN_EVENT_TOKEN               3
/* ************************************************************************** */
// push a nil into table
// [-0, +0, e]
#define set_nil(field)                                     \
    lua_pushliteral(L, field);                             \
    lua_pushnil(L);                                        \
    lua_settable(L, -3);

// push a number into table
// [-0, +0, e]
#define set_number(field, n)                               \
    lua_pushliteral(L, field);                             \
    lua_pushnumber(L, (lua_Number) n);                     \
    lua_settable(L, -3);

// push a integer into table
// [-0, +0, e]
#define set_integer(field, i)                              \
    lua_pushliteral(L, field);                             \
    lua_pushinteger(L, (lua_Integer) i);                   \
    lua_settable(L, -3);

// push a literal into table
// [-0, +0, e]
#define set_literal(field, l)                              \
    lua_pushliteral(L, field);                             \
    lua_pushliteral(L, l);                                 \
    lua_settable(L, -3);

// push a string into table
// [-0, +0, e]
#define set_lstring(field, s, len)                         \
    lua_pushliteral(L, field);                             \
    lua_pushlstring(L, s, (size_t) len);                   \
    lua_settable(L, -3);

// push a string into table
// [-0, +0, e]
#define set_string(field, s)                               \
    lua_pushliteral(L, field);                             \
    lua_pushstring(L, s);                                  \
    lua_settable(L, -3);

// push a function into table
// [-0, +0, e]
#define set_function(field, fn)                            \
    lua_pushliteral(L, field);                             \
    lua_pushcfunction(L, fn);                              \
    lua_settable(L, -3);

// push a boolean into table
// [-0, +0, e]
#define set_boolean(field, b)                              \
    lua_pushliteral(L, field);                             \
    lua_pushboolean(L, b);                                 \
    lua_settable(L, -3);

// push a lightuserdata into table
// [-0, +0, e]
#define set_lightuserdata(field, p)                        \
    lua_pushliteral(L, field);                             \
    if (p) {                                               \
      lua_pushlightuserdata(L, (void *) p);                \
    } else {                                               \
      lua_pushnil(L);                                      \
    }                                                      \
    lua_settable(L, -3);
/* ************************************************************************** */
#define EXTENDED_OUTPUT_STACK_SIZE            4095
#define EXTENDED_FUNCTIONS_PER_TRANSDUCTION   3
#define EXTENDED_OUTPUT_PLACEHOLDER           6
/* ************************************************************************** */
struct extended_output_render {
  int cardinality;
  struct stack_unichar* stack_template;
  struct stack_unichar* stack_render;
  vector_int* placeholders;
  vector_int* divisors;
  vector_int* cut_after_policy;
  vector_ptr* output_sets;

  int new_output_set(int n_elements, int stop_after, int placeholder) const {
  ::push(stack_template, EXTENDED_OUTPUT_PLACEHOLDER);
    vector_int_add(placeholders,placeholder + 1);
    vector_int_add(divisors,0);
    vector_int_add(cut_after_policy,stop_after);
    return vector_ptr_add(output_sets, new_vector_ptr(n_elements));
  }

  // const char*, length
  int add_output(int set_number, const char* output, int length) const {
    return vector_ptr_add((vector_ptr*) output_sets->tab[set_number],
                          u_strndup(output, length));
  }

  // unichar, length
  int add_output(int set_number, const unichar* output, int length) const {
    return vector_ptr_add((vector_ptr*) output_sets->tab[set_number],
                          u_strndup(output, length));
  }

  // const char*
  int add_output(int set_number, const char* output) const {
    return add_output(set_number, output, strlen(output));
  }

  // unichar
  int add_output(int set_number, const unichar* output) const {
    return add_output(set_number, output, u_strlen(output));
  }

  // prepare the template to be rendered
  int prepare() {
    // if there are not a template to render return 0 as cardinal
    if(is_empty(stack_template)) return 0;

    cardinality = 1;
    int i = output_sets->nbelems;

    // if output_sets contains 4 sets, e.g. A, B, C, D
    // the code below fills the "divisors" vector with the
    // cardinal product (the cardinality of the Cartesian product)
    // of the later sets of each set
    // D =          1   -> divisors[0]
    // C =         |D|  -> divisors[1]
    // B =     |C|x|D|  -> divisors[2]
    // A = |B|x|C|x|D|  -> divisors[3]
    while(i > 0) {
      --i;
      divisors->tab[i] = cardinality;
      cardinality = ((vector_ptr*) output_sets->tab[i])->nbelems * cardinality;
    }

    // will be equal to the cardinal product of all sets
    // e.g. the cardinal obtained from |A|x|B|x|C|x|D|
    return cardinality;
  }

  // Renders a template into a literal output
  // n is a combination number among the total number of allowed combinations
  // 0 <= n < cardinality.
  // e.g., if the template is equal to "foo % bar % baz" and there are two output
  // sets A = {#,&,+} and B= {1,2} this function will returns:
  // for n = 0, "foo # bar 1 baz"
  // for n = 1, "foo # bar 2 baz"
  // for n = 2, "foo & bar 1 baz"
  // for n = 3, "foo & bar 2 baz"
  // for n = 4, "foo + bar 1 baz"
  // for n = 5, "foo + bar 2 baz"
  struct stack_unichar* render(int n) const {
    // if there are nothing to render then return NULL
    if (cardinality == 0) {
      return NULL;
    }

    // if there are no output sets (nbelems == 0)  or the
    // given combination number (n) is out-of-bounds (0 <= n < cardinality)
    // then return the template without render it
    if (UNITEX_LIKELY(output_sets->nbelems == 0) || (n < 0 || n > cardinality)) {
      return stack_template;
    }

    // empty the render stack
    stack_render->top = -1;

    // auxiliary variables
    int placeholder = 0;
    int divisor = 0;
    int cardinal = 0;
    int index = 0;
    int top = 0;
    unichar* literal = NULL;
    unichar* variable = NULL;

    // rendering loop
    for (int i = 0; i < output_sets->nbelems; ++i) {
      literal = &stack_template->buffer[top];
      placeholder = placeholders->tab[i];
      divisor = divisors->tab[i];
      cardinal = ((vector_ptr*) output_sets->tab[i])->nbelems;
      index = (int) (n / divisor) % cardinal;
      variable = (unichar*) ((vector_ptr*) output_sets->tab[i])->tab[index];
      push_array(stack_render, literal, placeholder - top);
      push_array(stack_render, variable, u_strlen(variable));
      top = placeholder + 1;
    }

    // process the remaining template
    if (top < stack_template->top) {
      literal = &stack_template->buffer[top];
      push_array(stack_render, literal, stack_template->top - top);
    }

    // there is no more chars to add to the render stack,
    // hence we put a mark to indicate the end of the string
    push(stack_render, '\0');

    return stack_render;
  }

  void cut(int* n, int* n_matches, int* n_fails) {
    // auxiliary variables
    int divisor = 0;
    int cardinal = 0;
    int n_matches_has_changed = 0;
    int n_fails_has_changed = 0;

    // cut loop
    for (int i = output_sets->nbelems - 1 ; i >= 0 ; --i) {
      if (cut_after_policy->tab[i] != CUT_AFTER_EXHAUSTIVELY_CHECK) {
        if (cut_after_policy->tab[i] >= CUT_AFTER_N_MATCHES  &&
            cut_after_policy->tab[i] <= *n_matches) {
          // get the index of the set i
          divisor = divisors->tab[i];
          cardinal = ((vector_ptr*) output_sets->tab[i])->nbelems;

          if ((*n+1) >= divisor) {
            *n = ((cardinal * divisor * ((int) (*n / (cardinal*divisor)) + 1)) ) - 1;
            n_matches_has_changed = 1;
          }
       } else if (cut_after_policy->tab[i] <= CUT_AFTER_N_FAILURES &&
                  cut_after_policy->tab[i] >= *n_fails) {
         divisor = divisors->tab[i];
         cardinal = ((vector_ptr*) output_sets->tab[i])->nbelems;

         if ((*n+1) >= divisor) {
           *n = ((cardinal * divisor * ((int) (*n / (cardinal* divisor)) + 1)) ) - 1;
           n_fails_has_changed = 1;
         }
       }
      }
    }

    if (n_matches_has_changed) {
      *n_matches = 0;
    }

    if (n_fails_has_changed) {
      *n_fails = 0;
    }

  }

  extended_output_render()
      : cardinality(0),
        stack_template(new_stack_unichar(EXTENDED_OUTPUT_STACK_SIZE)),
        stack_render(new_stack_unichar(EXTENDED_OUTPUT_STACK_SIZE)),
        placeholders(new_vector_int(EXTENDED_FUNCTIONS_PER_TRANSDUCTION)),
        divisors(new_vector_int(EXTENDED_FUNCTIONS_PER_TRANSDUCTION)),
        cut_after_policy(new_vector_int(EXTENDED_FUNCTIONS_PER_TRANSDUCTION)),
        output_sets(new_vector_ptr(EXTENDED_FUNCTIONS_PER_TRANSDUCTION)) {
  }

  ~extended_output_render() {
    free_stack_unichar(stack_template);
    free_stack_unichar(stack_render);
    free_vector_int(placeholders);
    free_vector_int(divisors);
    free_vector_int(cut_after_policy);
    free_vector_ptr_element(output_sets,
                            (release_p) free_vector_ptr,
                            (release_f) free);
  }

 private:
  UNITEX_DISALLOW_COPY_AND_ASSIGN(extended_output_render);
};

//struct {
//  int32_t length; // number of elemements
//  UnitexString extended_template;
//  UnitexString extended_output[length][];
//  int32_t dm[length][3]; // div, size, placeholder
//  int32_t total; // total of combinations
//  int32_t max_output_length;
//
//  UnitexString generate_literal_output(int32_t n) {
//    if (total == 0) return extended_template;
//
//    UnitexString literal_output(max_output_length);
//
//    int index = 0;
//
//    // 0 1 2 3 4 5 6 7
//    // f o o ? b a r 0
//    int32_t i;
//
//    for (i=0;i < length; i++) {
//      literal_output.append(extended_template.c_unichar()+index, dm[i][2]-index);
//      literal_output.append(extended_output[i][(int)(n/dm[i][0]) % dm[i][1]]);
//      index = dm[i][2] + 1;
//    }
//
//    literal_output.append(extended_template.c_unichar()+index, extended_template.len()-index);
//
//    return literal_output;
//  }
//};

// in:
// template
// sets
// n=number of sets
// [x_i,y_i] -> divisor_i, number of elements in set_i
// total=total of combinations
// out:
// literal_output
/* ************************************************************************** */
namespace details {  // details
/* ************************************************************************** */
namespace {
/* ************************************************************************** */
int process_extended_function_return_type(int type,
                                          int set_number,
                                          lua_State* L,
                                          struct extended_output_render* r,
                                          const char* function_name) {
  int retval = 1;

  // LUA_TTABLE     -> lua_topointer(L, -1)
  // LUA_TFUNCTION  -> lua_topointer(L, -1)
  // LUA_TUSERDATA  -> lua_touserdata(L, -1)
  // LUA_TTHREAD    -> (void *)lua_tothread(L, -1)

  // retrieve boolean or string result
  switch(type) {
    // nil
    case LUA_TNIL: {
      retval = 0;
    } break;

    // bool
    case LUA_TBOOLEAN: {
      // if the function returns false
      // there is not a following transition
      if (!lua_toboolean(L, -1)) {
        retval = 0;
      } else {
        // if the return value is equal to true,
        // the output stack stays unchanged, this
        // is equivalent to push an empty symbol
        // However, if we are dealing with a set
        // of outputs we need to push an empty string
        if (UNITEX_UNLIKELY(set_number >= 0)) {
          r->add_output(set_number, "");
        }
      }
    } break;

    // number
    case LUA_TNUMBER: {
      // convert the number returned to string
      char number[LUAI_MAXNUMBER2STR];
      size_t count = lua_number2str(number, lua_tonumber(L, -1));
      if (UNITEX_LIKELY(set_number < 0)) {
        // push to stack
        push_array(r->stack_template, number, count);
      } else {
       // add to the output set
       r->add_output(set_number, number, count);
      }
    } break;

    // string
    case LUA_TSTRING: {
      // TODO(martinec) use instead luaL_checklstring
      const char* s = lua_tostring(L, -1);
      unichar S[MAXBUF] = { 0 };
      size_t count = u_decode_utf8(s, S);
      if (UNITEX_LIKELY(set_number < 0)) {
        // push to stack
        push_array(r->stack_template, S, count);
      } else {
        // add to the output set
        r->add_output(set_number, S, count);
      }
    } break;

    // light unichar
    case LUA_TLIGHTUSERDATA: {
      const unichar* S = (const unichar*) lua_touserdata(L, -1);
      size_t count = u_strlen(S);
      if (UNITEX_LIKELY(set_number < 0)) {
        // push to stack
        push_array(r->stack_template, S, count);
      } else {
        // add output to set
        r->add_output(set_number, S, count);
      }
    } break;

    // UnitexString
    case LUA_TUSERDATA: {
      UnitexString* S =  lua_checkudata_cast(L, -1, UnitexString);
      size_t count = S->len();
      if (UNITEX_LIKELY(set_number < 0)) {
        // push to stack
        push_array(r->stack_template, S->c_unichar(), count);
      } else {
        // add output to set
        r->add_output(set_number, S->c_unichar(), count);
      }
    } break;

    // fail
    default:
      // pop the invalid return type
      lua_pop(L, 1);
      // trown an error
      luaL_error(L,
          "Error: function @%s must return a boolean, a number, a string, an array or a null value\n",
          function_name);
      break;
  }

  return retval;
}
/* ************************************************************************** */
}
/* ************************************************************************** */
}  // namespace details
/* ************************************************************************** */
class vm {
 public:
  vm(void)
      : L(), env(0), local_env_ref(0), main_env_ref_(0) {
    memset(main_env_loaded_, 0, sizeof(int) * ELG_MAIN_EVENTS_COUNT);
  }

  virtual ~vm(void) {
    stop();
  }

// // traceback function taken straight out of luajit.c
//  static int traceback(lua_State* L) {
//    if (!lua_isstring(L, 1)) { /* Non-string error object? Try metamethod. */
//      if (lua_isnoneornil(L, 1) || !luaL_callmeta(L, 1, "__tostring")
//          || !lua_isstring(L, -1))
//        return 1; /* Return non-string error object. */
//      lua_remove(L, 1); /* Replace object by result of __tostring metamethod. */
//    }
//    luaL_traceback(L, L, lua_tostring(L, 1), 1);
//    return 1;
//  }

  // panic handler
  static int panic(lua_State* L) {
    elg_stack_dump(L);
    const char* error = lua_tostring(L, -1);
    if(error) {
      fatal_error("%s\n", error);
    } else {
      fatal_error("unknown error on the elg engine\n");
    }

    return 0;
  }

  // [-0, +0] > (+0)
  // create a new Lua environment
  // load the standard library
  // load the elg library
  // setup the panic handler
  // run the initialization script
  // register custom constants and functions
  // returns true if success; false otherwise
  // at the end the top of the stack is empty
  // [-0, +0] > (+0)
  int restart() {
    int retval = 0;

    if (is_running()) {
      stop();
    }

    // create a new Lua environment
    if ((L = luaL_newstate()) == NULL) {
      elg_error(L, "failed to create a new environment");
    }

    if (is_running()) {
      // load the standard library
      // [-0, +0] > (+0)
      luaL_openlibs(L);
      elg_stack_dump(L);

      // load the elg library
      // [-0, +0]
      elg::openlibs(L);
      elg_stack_dump(L);

      // setup the panic handler
      // [-0, +0] > (+0)
      lua_atpanic(L, panic);

      // load and run the initialization script
      // custom classes are not available right now from here
      // FIXME(martinec) Remove the hard-coded path
      // [-0, +0] > (+0)
      if (luaL_dofile(L,"/data/devel/projects/UnitexGramLab/unitex-core-elg/unitex-core/bin/Scripts/init.upp")) {
        const char* e =lua_tostring(L, -1);
        lua_pop(L,1);
        luaL_error(L,"Error running the initialization script: %s\n",e);
      }

      // -------------------------------------------------------------------
      // register custom classes
      // -------------------------------------------------------------------

      // Constants
      // [-0, +1] > (+1)
      lua_newtable(L);
      set_literal("API_VERSION", "0.1.0");
      // [-1, +0] > (+0)
      lua_setglobal(L, ELG_GLOBAL_CONSTANT);

      // uLocate
      lua_createtable(L, 0, ELG_GLOBAL_LOCATE_PARAMS_SIZE);
      set_nil(ELG_GLOBAL_LOCATE_STATE);
      set_nil(ELG_GLOBAL_LOCATE_POS);
      set_nil(ELG_GLOBAL_LOCATE_MATCHES);
      set_nil(ELG_GLOBAL_LOCATE_NUM_MATCHES);
      set_nil(ELG_GLOBAL_LOCATE_CONTEXT);
      // [-1, +0] > (+0)
      lua_setglobal(L, ELG_GLOBAL_LOCATE);

      // uMorphoLocate
      lua_createtable(L, 0, ELG_GLOBAL_MORPHO_LOCATE_PARAMS_SIZE);
      set_nil(ELG_GLOBAL_MORPHO_LOCATE_STATE_INDEX);
      set_nil(ELG_GLOBAL_MORPHO_LOCATE_POS_IN_TOKENS);
      set_nil(ELG_GLOBAL_MORPHO_LOCATE_POS_IN_CHARS);
      set_nil(ELG_GLOBAL_MORPHO_LOCATE_MATCHES);
      set_nil(ELG_GLOBAL_MORPHO_LOCATE_NUM_MATCHES);
      set_nil(ELG_GLOBAL_MORPHO_LOCATE_CONTEXT);
      set_nil(ELG_GLOBAL_MORPHO_LOCATE_PARAMS);
      set_nil(ELG_GLOBAL_MORPHO_LOCATE_JAMO);
      set_nil(ELG_GLOBAL_MORPHO_LOCATE_POS_IN_JAMO);
      set_nil(ELG_GLOBAL_MORPHO_LOCATE_CONTENT);
      // [-1, +0] > (+0)
      lua_setglobal(L, ELG_GLOBAL_MORPHO_LOCATE);

      // Functions

      //uToken
      // [-0, +1] > (+1)
      lua_newtable(L);
      set_function("current", elg::token::current);
      set_function("previous", elg::token::previous);
      set_function("next", elg::token::next);
      set_function("at", elg::token::at);
      set_function("pos", elg::token::pos);
      set_function("offset", elg::token::offset);
      set_function("meta", elg::token::meta);
      set_function("negmeta", elg::token::negmeta);
      set_function("is_space", elg::token::is_space);
      set_function("value", elg::token::value);
      set_function("reference", elg::token::reference);
      set_function("lookup", elg::token::lookup);
      set_function("bitmask", elg::token::bitmask);
      set_function("tag", elg::token::tag);

      // uToken.U_SPACE
      set_integer(ELG_GLOBAL_TOKEN_U_SPACE, -1);
      // uToken.U_SENTENCE
      set_integer(ELG_GLOBAL_TOKEN_U_SENTENCE, -1);
      // uToken.U_STOP
      set_integer(ELG_GLOBAL_TOKEN_U_STOP, -1);

      // uToken.kBitMask
      lua_newtable(L);
      set_integer("WORD",    MOT_TOKEN_BIT_MASK);
      set_integer("DIC",     DIC_TOKEN_BIT_MASK);
      set_integer("UPPER",   MAJ_TOKEN_BIT_MASK);
      set_integer("LOWER",   MIN_TOKEN_BIT_MASK);
      set_integer("FIRST",   PRE_TOKEN_BIT_MASK);
      set_integer("CDIC",    CDIC_TOKEN_BIT_MASK);
      set_integer("NOT_DIC", NOT_DIC_TOKEN_BIT_MASK);
      set_integer("TDIC",    TDIC_TOKEN_BIT_MASK);
      lua_setfield(L, -2,  ELG_GLOBAL_TOKEN_BIT_MASK);

      // uToken.kMeta
      lua_newtable(L);
      set_integer("SHARP",   META_SHARP);
      set_integer("SPACE",   META_SPACE);
      set_integer("EPSILON", META_EPSILON);
      set_integer("WORD",    META_MOT);
      set_integer("DIC",     META_DIC);
      set_integer("SDIC",    META_SDIC);
      set_integer("CDIC",    META_CDIC);
      set_integer("TDIC",    META_TDIC);
      set_integer("UPPER",   META_MAJ);
      set_integer("LOWER",   META_MIN);
      set_integer("FIRST",   META_PRE);
      set_integer("NB",      META_NB);
      set_integer("TOKEN",   META_TOKEN);
      set_integer("LETTER",  META_LETTER);
      lua_setfield(L, -2,  ELG_GLOBAL_TOKEN_META);

      // [-1, +0] > (+0)
      lua_setglobal(L, ELG_GLOBAL_TOKEN);
      elg_stack_dump(L);

      //uParser
      // [-0, +1] > (+1)
      lua_newtable(L);
      set_function("setpos", elg::parser::setpos);
      lua_setglobal(L, ELG_GLOBAL_PARSER);

      //uVariable
      lua_newtable(L);
      lua_setglobal(L, ELG_GLOBAL_VARIABLE);

      //uMatch
      // [-0, +1] > (+1)
      lua_newtable(L);
      set_function("begin", elg::match::begin);
      set_function("fend", elg::match::end);
      set_function("length", elg::match::length);
      set_function("content", elg::match::content);
      set_function("start_with_space", elg::match::start_with_space);
      set_function("start_sentence", elg::match::start_sentence);
      set_function("end_sentence", elg::match::end_sentence);
      set_function("start_newline", elg::match::start_newline);
      set_function("end_newline", elg::match::end_newline);
      // [-1, +0] > (+0)
      lua_setglobal(L, ELG_GLOBAL_MATCH);

      //uString
      // [-0, +1] > (+1)
      lua_newtable(L);
      set_function("format", elg::string::format);
      // [-1, +0] > (+0)
      lua_setglobal(L, ELG_GLOBAL_STRING);

      // uEnvironment
      // [-0, +1] > (+1)
      lua_newtable(L);
      elg_stack_dump(L);

      // uLoaded
      // [-0, +1] > (+2)
      lua_newtable(L);
      // [-1, +0] > (+1)
      lua_setfield(L, -2,  ELG_ENVIRONMENT_LOADED);
      elg_stack_dump(L);

      // uCalled
      // [-0, +1] > (+2)
      lua_newtable(L);
      // [-1, +0] > (+1)
      lua_setfield(L, -2,  ELG_ENVIRONMENT_CALLED);
      elg_stack_dump(L);

      // uValues
      // [-0, +1] > (+2)
      lua_newtable(L);
      // [-1, +0] > (+1)
      lua_setfield(L, -2,  ELG_ENVIRONMENT_VALUES);
      elg_stack_dump(L);

      // [-1, +0] > (+0)
      lua_setglobal(L, ELG_GLOBAL_ENVIRONMENT);
      elg_stack_dump(L);

      //uMisc
      // isWindows

      // -------------------------------------------------------------------
      // end of register custom classes
      // -------------------------------------------------------------------

      // lua_pushcfunction(L, traceback);

      // successful restart
      retval = true;
    }

    return retval;
  }

  void stop() {
    if (is_running()) {
      unload_all();
      elg_stack_dump(L);
      free_state();
    }
  }

  void clean(int top = 0) {
    int n = lua_gettop(L);
    if (top >= 0) {
      lua_pop(L, n - top);
    } else {
      lua_pop(L, n + top);
    }
  }

  int is_running() {
    return L != NULL;
  }

  void free_state() {
    lua_gc(L, LUA_GCCOLLECT, 0);
    lua_close(L);
    L = NULL;
  }

  // [-0, +0]
  void unload_all() {
    // get the environment storage
    // [-0, +1] > (+1)
    lua_getglobal(L, ELG_GLOBAL_ENVIRONMENT);
    elg_stack_dump(L);
    // the ELG_GLOBAL_ENVIRONMENT table should be at the top of the stack
    luaL_checktype(L, -1, LUA_TTABLE);
    elg_stack_dump(L);
    // get the loaded table ELG_ENVIRONMENT_LOADED
    // [-0, +1] > (+2)
    lua_getfield(L, -1, ELG_ENVIRONMENT_LOADED);
    elg_stack_dump(L);
    // put nil
    // [-0, +1] > (+3)
    lua_pushnil(L);
    elg_stack_dump(L);
    const char* extension_env_name = {};
    // [-1, +(2|0)] > (+(4|2))
    while (lua_next(L, -2)) {
        elg_stack_dump(L);
        // check if key is a number
        if (lua_isnumber(L, -2)) {
          // check if value is a integer
          if (lua_isstring(L, -1)) {
              extension_env_name = lua_tostring(L, -1);
              // [-0, +0] > (+4)
              unload_environment(extension_env_name);
          }
        }
        elg_stack_dump(L);
        // remove value but keep key for next iteration
        // [-1, +0] > (+3)
        lua_pop(L, 1);
    }
    elg_stack_dump(L);

    // pop uEnvironment[uLoaded]
    // [-2, +0] > (+0)
    lua_pop(L, 2);
  }

  // [-0, +0]
  void unload_environment(const char* extension_env_name) {
    // retrieve the script environment from the register
    // [-0, +1] > (+1)
    lua_getfield(L, LUA_REGISTRYINDEX, extension_env_name);
    elg_stack_dump(L);

    // if there are an onUnload() function, then run it
    // [-0, +1] > (+2)
    call_event(ELG_EXTENSION_EVENT_UNLOAD);
    elg_stack_dump(L);

    // pop the the script environment
    lua_pop(L, 1);
    elg_stack_dump(L);
  }

  // -1: environment
  // [-0, +0] > (+1)
  void register_main_events() {
    int32_t i = 0;
    while(elgMainEvents[i].name != NULL) {
      load_event(elgMainEvents[i].name);
      main_env_loaded_[i] = do_event(0, 0, 0);
      ++i;
    }
  }

  // -1: environment
  // [-0, +0] > (+1)
  int call_event(const char* event_name) {
    load_event(event_name);
    return do_event(1, 0, 0);
  }

  // -1: environment
  // [-0, +0] > (+1)
  int call_main_event(int event_number) {
    load_main_event(event_number);
    return do_event(1, 0, elgMainEvents[event_number].nresults);
  }

  // -1: environment
  // [-0, +0] > (+1)
  int load_event(const char* event_name) {
    // if there are an onLoad() function, then run it
    // [-0, +1] > (+2)
    // TODO(martinec) use lua_pushliteral
    lua_pushstring(L, event_name);
    elg_stack_dump(L);

    // similar to lua_gettable, but without metamethods
    // [-1, +1] > (+1)
    lua_rawget(L, -2);
    elg_stack_dump(L);

    return 1;
  }

  int call_event(int nargs, int nresults) {
    // [-1, +0] > (+1)
    if (lua_pcall(L, nargs, nresults, 0) != 0) {
      const char* e = lua_tostring(L, -1);
      elg_stack_dump(L);
      lua_pop(L, 1); elg_stack_dump(L);
      luaL_error(L, "Error calling @%s: %s\n", "event", e);
    }
    return 1;
  }

  int do_event(int do_call, int nargs, int nresults) {
    elg_stack_dump(L);
    if (lua_isfunction(L, -1-nargs)) {
      if (do_call) {
        // [-1, +0] > (+1)
        if (lua_pcall(L, nargs, nresults, 0) != 0) {
          const char* e = lua_tostring(L, -1);
          elg_stack_dump(L);
          lua_pop(L, 1); elg_stack_dump(L);
          luaL_error(L, "Error calling @%s: %s\n", "event", e);
        }
      } else {
        // pop the function name
        lua_pop(L, 1);
        return 1;
      }
    } else {
      // pop the returned value
      lua_pop(L, 1);
      return 0;
    }
    return 1;
  }

  void load_main_event(int event_number) {
    // only if there are a main extension
    if (UNITEX_LIKELY(!main_env_ref_)) return;

    // get the main extension environment
    lua_rawgeti(L, LUA_REGISTRYINDEX, main_env_ref_);
    elg_stack_dump(L);

    // get the first extension (main graph) environment
    lua_rawgeti(L, -1, 1);
    elg_stack_dump(L);

    load_event(elgMainEvents[event_number].name);
    elg_stack_dump(L);
  }

  int call_token_event(struct locate_parameters* p, int event_number, int* pos, int* current_origin) {
    // only if the main extension was loaded and a token_event is available
    if (UNITEX_LIKELY(!main_env_loaded_[event_number])) {
      return p->buffer[*pos + *current_origin];
    }

    // load the event
    load_main_event(event_number);

    // push index integer
    lua_pushinteger(L, *pos + *current_origin);
    elg_stack_dump(L);

    // perform the call
    call_event(elgMainEvents[event_number].nargs,
               elgMainEvents[event_number].nresults);
    elg_stack_dump(L);

    // test if we have a valid integer index
    if (lua_type(L, -1) != LUA_TNUMBER) {
      const char* e = lua_tostring(L, -1);
      lua_pop(L, 1); // error
      luaL_error(L,"Error calling the token_event it must return a integer\n",e);
    }

    // we have a valid integer
    int index = lua_tointeger(L, -1);

    // if the index was modified
    if(index != *pos + *current_origin) {
      // check if the new index is in the bounds
      if (UNITEX_LIKELY((index >= 0 && index < p->buffer_size)
          && (p->buffer[index] >= 0 && p->buffer[index] < p->tokens->size))) {

        // from a non-space token we always use the next one
        index += (p->buffer[index] != p->SPACE && index + 1 < p->buffer_size) ?
                 1 : 0;

        // now we require that the token at the index position is not a space
        index += (p->buffer[index] == p->SPACE && index + 1 < p->buffer_size) ?
                 1 : 0;

        // index is equal to pos + current_origin
        if(*pos == 0) {
          // if pos is equal to 0, the curren_origin is equal to the
          // first skipped character and there is not possible to have
          // any match starting before the skipped range, in this case,
          // we modified the current_origin to be equal to the index of
          // the last character in the skipped range
          *current_origin = index;
          *pos = 0;
        } else {
          // we update the position relative to the current origin
          *pos = index - *current_origin + 0;
        }
      } else {
        // TODO(martinec) throw a warning "index out of the bounds" from here
      }
    }

    // -3: pop the returned value
    // -2: pop the main extension environment
    // -1: pop the main graph extension environment
    lua_pop(L, 3);
    elg_stack_dump(L);

    return p->buffer[*pos + *current_origin];
  }

  // [-0, +0] > (+1)
  void save_name(const char* extension_env_name) {
    // retrieve c, the ELG_GLOBAL_ENVIRONMENT table
    // [-0, +1] > (+2)
    lua_getglobal(L, ELG_GLOBAL_ENVIRONMENT);
    elg_stack_dump(L);
    // c should be at the top of the stack
    // [-0, +0] > (+2)
    luaL_checktype(L, -1, LUA_TTABLE);
    elg_stack_dump(L);
    // retrieve o, ELG_ENVIRONMENT_LOADED table
    // [-0, +1] > (+3)
    lua_getfield(L, -1, ELG_ENVIRONMENT_LOADED);
    elg_stack_dump(L);
    // put i, a integer key equal to the number of extensions environments
    // [-0, +1] > (+4)
    lua_pushinteger(L,++env);
    elg_stack_dump(L);
    // put n, a string value equal to the environment name
    // [-0, +1] > (+5)
    lua_pushstring(L, extension_env_name);
    elg_stack_dump(L);
    // set c.o[i] = n
    // [-2, 0] > (+3)
    lua_settable(L,-3);
    elg_stack_dump(L);
    // pop o and c
    // [-2, 0] > (+1)
    lua_pop(L, 2);
    elg_stack_dump(L);
  }

  int unload(const char* function_name) {
//    luaL_unref(L, LUA_REGISTRYINDEX, 0);
    return 1;
  }

//     bool load(const char* function_name) {
//       char script_name[MAX_TRANSDUCTION_VAR_LENGTH] = {};
//       char script_file[MAX_TRANSDUCTION_VAR_LENGTH] = {};
//
//       strcat(script_name,function_name);
//       strcat(script_name,ELG_FUNCTION_DEFAULT_EXTENSION);
//       strcat(script_file,UNITEX_SCRIPT_PATH);
//       strcat(script_file,script_name);
//
//       //  priming run: loads and runs script's main function
//       if (luaL_dofile(L, script_file)) {
//         fatal_error("Error calling @%s: %s\n", script_file, lua_tostring(L, -1));
//       }
//
//       //  test if the function to run exists
//       lua_getglobal(L, function_name);
//       if(!lua_isfunction(L,-1)){
//         lua_pop(L,1);
//         fatal_error("Error loading @%s, function doesn't exists\n",function_name);
//       }
//
//       return true;
//     }

  // receives a function chunk at the top of the stack
  static int setup_sandboxed_environment(lua_State* state,
                                            int fallback_index,
                                            int index,
                                            const char* fallback_name,
                                            const char* self_name) {
    // create a new empty table (e) and pushes it onto the stack
    // we will use this table to holds the environment of the script
    // [-0, +1] > (+1)
    lua_newtable(state);
    elg_stack_dump(state);

    // create another new empty table (f) and pushes it onto the stack
    // we will use this as the metatable that holds the original global
    // table (_G)
    // [-0, +1] > (+2)
    lua_newtable(state);
    elg_stack_dump(state);

    // push onto the stack the value of _G, i.e. the environment
    // which holds all the global variables defined on L
    // [-0, +1] > (+3)
    lua_getfield(state, fallback_index, fallback_name);
    elg_stack_dump(state);

    // do f[__index] = _G, where f is the table at index -2
    // and _G is the value at the top of the stack.
    // __index is the versatile metamethod that allows us to use _G as
    // a "fallback" table if a key in the environment table doesn't exist
    // [-1, +0] > (+2)
    lua_setfield(state, -2, "__index");
    elg_stack_dump(state);

    // pops f from the stack and sets it as the new metatable
    // for e (the environment table)  at index -2
    // now f is the metatable of the environment table
    // setmetatable({}, {__index=_G})
    // [-1, +0] > (+1)
    lua_setmetatable(state, -2);
    elg_stack_dump(state);

    // [-0, +1] > (+2)
    // duplicate the environment table
    lua_pushvalue(state, -1);
    elg_stack_dump(state);

    // [-1, +0] > (+1)
    // replaces L globals
    lua_replace(state, LUA_GLOBALSINDEX);
    elg_stack_dump(state);

    // [-0, +1] > (+2)
    // duplicate the environment table
    lua_pushvalue(state, -1);
    elg_stack_dump(state);

    if (index != -1) {
      // [-0, +1] > (+3)
      // duplicate the environment table
      lua_pushvalue(state, -1);
      elg_stack_dump(state);
    }

    // register the environment table on the environment table
    // using self_name as key
    // [-1, +0] > (+2)
    lua_setfield(state, -2, self_name);
    elg_stack_dump(state);

    // pops the environment table from the stack and sets it as
    // the new environment of index . In this case, index is the chunk or
    // function that we have at the given index on the stack.
    // Take into account that every function starts with the
    // environment as its first upvalue, upvalues are the external
    // local variables that the function uses, and that are consequently
    // included in its closure
    // [-1, +0] > (+1)
    if (index != -1) {
      if (lua_setfenv(state, index) == 0) {
        elg_stack_dump(state);
        elg_error(state,lua_tostring(state, -1));
      }
    }
    elg_stack_dump(state);
    return 1;
  }

  int setup_special_constants(const struct locate_parameters* p) {
    // get the global table
    // [-0, +1] > (+1)
    lua_getglobal(L, "_G");

    // get the uToken table
    // [-0, +1] > (+2)
    lua_pushliteral(L, ELG_GLOBAL_TOKEN);
    lua_gettable(L, -2);

    // uToken.U_SPACE
    set_integer(ELG_GLOBAL_TOKEN_U_SPACE, p->SPACE);
    // uToken.U_SENTENCE
    set_integer(ELG_GLOBAL_TOKEN_U_SENTENCE, p->SENTENCE);
    // uToken.U_STOP
    set_integer(ELG_GLOBAL_TOKEN_U_STOP, p->STOP);

    // -2 pop the uToken table
    // -1 pop the global table
    // [-2, +0] > (+0)
    lua_pop(L,2);

    // add buffer size to globals
    // [-0, +1] > (+1)
    pushinteger(p->buffer_size);
    // [-1, +0] > (+0)
    setglobal(ELG_GLOBAL_U_BUFFER_SIZE);

    return 1;
  }

  int setup_local_environment() {
//    lua_rawgeti(L, LUA_REGISTRYINDEX, local_env_ref);
//    elg_stack_dump(L);
//    lua_pop(L,1);

    //
    setup_sandboxed_environment(L, LUA_GLOBALSINDEX, -1, "_G", "_L");
    elg_stack_dump(L);

    // register the function environment on the registry
    // [-1, +0] > (+1)
    local_env_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    elg_stack_dump(L);

//    lua_rawgeti(L, LUA_REGISTRYINDEX, local_env_ref);
//    elg_stack_dump(L);
//    lua_pop(L,1);

    return 1;
  }

  //
  int save_local_environment() {
// u_printf("++++++++++++++++++ save_local_environment\n");
//    // push local
//    lua_rawgeti(L, LUA_REGISTRYINDEX, local_env_ref);
//    elg_stack_dump(L);
//
//    // get FOO
//    lua_getfield(L, -1, "foo");
//    elg_stack_dump(L);
//
//    // update FOO
//    lua_getfield(L,LUA_GLOBALSINDEX,"_G");
//    elg_stack_dump(L);
//    lua_insert(L,-2);
//    elg_stack_dump(L);
//    lua_pushliteral(L,"foo");
//    elg_stack_dump(L);
//    lua_insert(L,-2);
//    elg_stack_dump(L);
//    lua_settable(L,-3);
//    elg_stack_dump(L);
//    lua_pop(L,1);
//    elg_stack_dump(L);
//
//    // pop local
//    lua_pop(L,1);
//    elg_stack_dump(L);

    return 1;
  }

  int restore_local_environment() {
//    u_printf("------------------ undo_local_environment\n");
//    lua_rawgeti(L, LUA_REGISTRYINDEX, local_env_ref);
//    elg_stack_dump(L);
//    clear_table_values(L);
//    lua_pop(L,1);
//    elg_stack_dump(L);

    return 1;
  }

  // 03/07/17
  // [+0, +0] > (+0)
  int load_main_extension(const char* fst_name, const Fst2* fst)  {
    // TODO(martinec) use assert()
    if (fst == NULL ||
        fst->graph_names    == NULL ||
        fst->graph_names[0] != NULL ||
        fst->graph_names[1] == NULL) {
      return 0;
    }

    // get the absolute path of the fst file
    char fst_file[FILENAME_MAX] = {0};
    get_real_path(fst_name, fst_file);

    // remove the file name from the path
    char fst_path[FILENAME_MAX] = {0};
    get_path(fst_file, fst_path);

    // number of graphs referenced on the compiled fst
    //int number_of_graphs = fst->number_of_graphs;
    int graph_number = 1;

    // get the main fst name
    char graph_name[FILENAME_MAX] = {0};
    u_encode_utf8(fst->graph_names[graph_number], graph_name);
    // if need, remove the debug info from the graph name
    char* pch = strchr(graph_name, DEBUG_INFO_OUTPUT_MARK);
    if (pch)  *pch = '\0';

    // convert the graph name to a file name
    replace_colon_by_path_separator(graph_name);

    // extension name = fst_path/graph_name.upp
    char extension_name[FILENAME_MAX] = {0};
    // extension_name = fst_path/
    strcat(extension_name, fst_path);
    // extension_name = fst_path/graph_name
    strcat(extension_name, graph_name);
    // extension name = fst_path/graph_name.upp
    strcat(extension_name, ELG_FUNCTION_DEFAULT_EXTENSION);

    // return if graph_name.upp doesn't exists
    if(!is_regular_file(extension_name)) {
      // [-0, +1] > (+0)
      // -2: pop the table of graph extensions environments
      // commented because for now is there only 1 main extension allowed
      // lua_pop(L,1);
      elg_stack_dump(L);
      return 0;
    }

    // create a new empty table and pushes it onto the stack
    // we will use this table to holds all the graph extension chunks
    // [-0, +1] > (+1)
    lua_newtable(L);
    elg_stack_dump(L);

    // push a copy of the table
    lua_pushvalue(L, -1);

    // register the graph chunks' table extension on the registry
    // [-1, +0] > (+0)
    main_env_ref_ = luaL_ref(L, LUA_REGISTRYINDEX);
    elg_stack_dump(L);

    //    // graph names begin at pos 1
    //    for (int i = 1; i <= number_of_graphs; ++i) {
    //      if (fst->graph_names[i] != NULL) {
    //        u_printf("%S\n",fst->graph_names[i]);
    //      }
    //    }

    // load the graph_name.upp chunk
    // [-0, +1] > (+3)
    load_file(L,extension_name);
    elg_stack_dump(L);

    // push the graph number to the stack
    // [-0, +1] > (+2)
    lua_pushinteger(L, graph_number);
    elg_stack_dump(L);

    // setup a sandboxed environment
    // [-0, +1] > (+2)
    setup_sandboxed_environment(L, LUA_GLOBALSINDEX, -4, "_G", "_S");
    elg_stack_dump(L);

    // do table[graph_number] = environment
    // [-2, +0] > (+1)
    lua_rawset(L, -4);
    elg_stack_dump(L);

    //  priming run: loads the chunk and create variables
    // [-1, +0] > (+3)
    if (lua_pcall(L, 0, 0, 0)) {  // LUA_MULTRET -> 0 ?
      const char* e = lua_tostring(L, -1);
      lua_pop(L,1);
      luaL_error(L, "Error loading @%s: %s\n", extension_name, e);
    }
    elg_stack_dump(L);

    // get the main extension environment
    lua_rawgeti(L, -1, graph_number);
    elg_stack_dump(L);

    // keep track of the available events
    register_main_events();

    // call the load event
    // [-0, +0] > (+1)
    if (UNITEX_UNLIKELY(main_env_loaded_[ELG_MAIN_EVENT_LOAD])) {
      call_event(elgMainEvents[ELG_MAIN_EVENT_LOAD].name);
      elg_stack_dump(L);
    }

    // -1: pop the graph extension environment
    lua_pop(L,1);
    elg_stack_dump(L);

    // -1: pop the table of graph extensions environments
    lua_pop(L,1);
    elg_stack_dump(L);

    return 1;
  }

  // 14/07/17
  // [+0, +0] > (+0)
  int unload_main_extension()  {
    call_main_event(ELG_MAIN_EVENT_UNLOAD);
    elg_stack_dump(L);
    return 1;
  }

  static int load_file(lua_State* state, const char* file_name) {
    int retval = luaL_loadfile(state, file_name);
    if (retval) {
      const char* e = lua_tostring(state, -1);
      elg_stack_dump(state);
      lua_pop(state,1);
      elg_stack_dump(state);
      luaL_error(state,"ELG error: %s\n", e);
    }
    return retval;
  }

  // 05/09/16 load once
  // in:             (+0)
  // out: [-0, +2] > (+2)
  int load_extension(const char* extension_name, const char* function_name) {
    elg_stack_dump(L);

    // prepare script extension_env_name
    char extension_env_name[MAX_TRANSDUCTION_VAR_LENGTH] = { };

    // environment name = ELG_ENVIRONMENT_PREFIX-extension_name
    strcat(extension_env_name, ELG_ENVIRONMENT_PREFIX);
    strcat(extension_env_name, "-");
    strcat(extension_env_name, extension_name);

    // the registry is a global table accessible from the Lua C-API
    // we use it for both store extended functions and check if an
    // extended function is already stored

    // retrieve the script environment from the register, for that
    // push onto the stack the value stored in the registry with key
    // "foo" where foo is the environment name
    // [-0, +1] > (+1)
    lua_getfield(L, LUA_REGISTRYINDEX, extension_env_name);
    elg_stack_dump(L);

    // if the retrieved value is not the script environment, then
    // load once the file with the extended functions implementations
    if (!lua_istable(L, -1)) {
      // pop the previous result (normally a LUA_TNIL) from the top
      // of the stack
      // [-1, +0] > (+0)
      lua_pop(L, 1);
      elg_stack_dump(L);

      // prepare script_name and script_file variables
      char script_name[MAX_TRANSDUCTION_VAR_LENGTH]   = { };
      char script_file[MAX_TRANSDUCTION_VAR_LENGTH]   = { };

      // script name = extension_name.upp
      strcat(script_name, extension_name);
      strcat(script_name, ELG_FUNCTION_DEFAULT_EXTENSION);

      // script_file = /default/path/extension_name.upp
      strcat(script_file, UNITEX_SCRIPT_PATH);
      strcat(script_file, script_name);

      // check if script_file exists

      // load the script as a Lua chunk; it does not run it
      // [-0, +1] > (+1)
      load_file(L, script_file);
      elg_stack_dump(L);

      // get the local environement
      lua_rawgeti(L, LUA_REGISTRYINDEX, local_env_ref);
      elg_stack_dump(L);

      // setup a sandboxed environment
      // [-0, +1] > (+2)
      elg_stack_dump(L);
      setup_sandboxed_environment(L, 2, 1, "_L", "_E");
      elg_stack_dump(L);

      // register the extension chunk on the registry using the
      // extension_env_name as key
      // [-1, +0] > (+1)
      lua_setfield(L, LUA_REGISTRYINDEX, extension_env_name);
      elg_stack_dump(L);

      // pop the local environment
      lua_pop(L, 1);
      elg_stack_dump(L);

      //  priming run: loads the script and create variables
      // [-1, +0] > (+0)
      if (lua_pcall(L, 0, 0, 0)) {  // LUA_MULTRET -> 0 ?
        const char* e = lua_tostring(L, -1);
        lua_pop(L,1);
        luaL_error(L, "Error loading @%s: %s\n", extension_name, e);
      }
      elg_stack_dump(L);

      // retrieve the script environment from the register
      // [-0, +1] > (+1)
      lua_getfield(L, LUA_REGISTRYINDEX, extension_env_name);
      elg_stack_dump(L);

      // [-0, +0] > (+1)
      call_event(ELG_EXTENSION_EVENT_LOAD);
      elg_stack_dump(L);

      // [-0, +0] > (+1)
      save_name(extension_env_name);
      elg_stack_dump(L);
    }  // if (!lua_istable(L,-1))

    // using the script environment
    // get the extended function to run
    // [-0, +1] > (+2)
    lua_getfield(L, -1, function_name);
    elg_stack_dump(L);
    if (!lua_isfunction(L, -1)) {
      lua_pop(L, 2); // pop the environment + returned field
      luaL_error(L, "Error loading @%s, function doesn't exists\n",
                  function_name);
    }

    // remove the extension environment from the stack,
    // let only the function
    // lua_remove(L, -2);

    elg_stack_dump(L);
    return 1;
  }

  // L is the main thread, lua_State represents this thread
  // this function creates a new thread (not at OS thread)
  static lua_State* create_mirror_state(lua_State* L, int* m_refkey) {
    // creates a new thread, pushes it on the stack, and returns a
    // pointer to a lua_State that represents this new thread
    // The new state returned by this function shares with the original
    // state all global objects (such as tables), but has an independent
    // execution stack
    lua_State* M = lua_newthread(L);

    // anchor new thread
    *m_refkey = luaL_ref(L, LUA_REGISTRYINDEX);
    // lua_unref(M, m_refkey);
    return M;
  }

  static int clear_table_values(lua_State* state) {
    assert(lua_type(state, -1) == LUA_TTABLE);
    elg_stack_dump(state);
    lua_pushnil(state);
    while (lua_next(state, -2) != 0) {
      elg_stack_dump(state);
      int key_type  = lua_type(state, -2);
      if (key_type == LUA_TSTRING) {
        // pop value
        lua_pop(state, 1);
        elg_stack_dump(state);
        const char* key = lua_tostring(state, -1);
         if (key[0] && key[0] != '_') {
           // duplicate key
           lua_pushvalue(state, -1);
           elg_stack_dump(state);
           lua_pushnil(state);
           // set t[v] = nil
           lua_rawset(state, -4);
         }
      } else {
        // pop value but keep key for next iteration
        lua_pop(state, 1);
      }
      elg_stack_dump(state);
    }

    elg_stack_dump(state);
    return 1;
  }

  static int move_table_values(lua_State* to, lua_State* from, int to_idx) {
    // iterate through the table
    lua_pushnil(from);
    while (lua_next(from, -2) != 0) {
      elg_stack_dump(from);

      // duplicate key
      lua_pushvalue(from, -2);

      // move key and value
      lua_xmove(from, to, 2);
      elg_stack_dump(to);
      elg_stack_dump(from);

      // swap key-value to value-key
      lua_insert(to, -2);
      elg_stack_dump(to);
      elg_stack_dump(from);

      // assign to the destination table
      lua_settable(to, to_idx);
      elg_stack_dump(to);
      elg_stack_dump(from);
    }

    // pop nil or the last key
    lua_pop(from, 1);

    return 1;
  }

//  static void unref_mirror_state(lua_State* L, int m_refkey) {
//    luaL_unref(L, LUA_REGISTRYINDEX, m_refkey);
//  }

  // call the function on the stack
  // in:                 (+1+n) 2:function, n:params
  // out: [-0, -(1+n)] > (+0)
  // returns:
  //  0 : step back
  //  1 : step forward
  int call(const char* function_name, int nargs, int stop_after, struct extended_output_render* r) {
    elg_stack_dump(L);

    int m_refkey = 0;
    lua_State* M = create_mirror_state(L, &m_refkey);
    lua_xmove(L, M, nargs+2);
    elg_stack_dump(M);
    elg_stack_dump(L);

    setup_sandboxed_environment(M, 1, 2, "_E", "_F");
    elg_stack_dump(M);

    // register the function environment on the registry
    // [-1, +0] > (+1)
    int local_func_ref = luaL_ref(M, LUA_REGISTRYINDEX);
    elg_stack_dump(M);

    // do the call (lua_State *L, int nargs, int nresults, int errfunc)
    // nresults => 1, one result expected
    // errfunc  => 0, the error message returned on the stack is exactly
    //                the original error message
    // [-(n + 1), +1] > (+1) 1:returned value
    if (lua_pcall(M, nargs, 1, 0) != 0) {
      const char* e = lua_tostring(M, -1);
      lua_pop(M,1); // error
      luaL_error(M, "Error calling @%s: %s\n", function_name,e);
    }
    elg_stack_dump(M);

    int retval =  0;
    int type = lua_type(M, -1);

    // the extened function did not return an array
    if (type != LUA_TTABLE) {
      retval = unitex::details::process_extended_function_return_type(type, -1, M, r, function_name);
    } else {
    // the extended function returned an array
      int n_elements = lua_objlen(M, -1);
      // only if the array isn't empty
      if (n_elements) {
        int set_number  = r->new_output_set(n_elements, stop_after, r->stack_template->top);
        retval = 1;
        // iterate through the table
        elg_stack_dump(M);
        // first key
        lua_pushnil(M);

        while (retval && lua_next(M, -2) != 0) {
          elg_stack_dump(M);
          // check if the key is an integer
          // arrays in Lua are indexing with integers
          if (lua_type(M, -2) != LUA_TNUMBER) {
            luaL_error(L,
                "Error: function @%s must return a table indexed only by integers\n",
                function_name);
          }

          // get the type of the returned value
          type  = lua_type(M, -1);

          // process the returned value
          retval = unitex::details::process_extended_function_return_type(type, set_number, M, r, function_name);

          // pop value but keep key for next iteration
          lua_pop(M, 1);
          elg_stack_dump(M);
        }

        elg_stack_dump(M);

        // if the loop above was broken by a retval equal to 0,
        // then the last key
        if(!retval) {
          // pop the last key
          lua_pop(M, 1);
        }

      }
    }

    if(retval) {
      elg_stack_dump(L);
      elg_stack_dump(M);
      char extension_env_name[MAX_TRANSDUCTION_VAR_LENGTH] = { };

      // environment name = ELG_ENVIRONMENT_PREFIX-function_name
      strcat(extension_env_name, ELG_ENVIRONMENT_PREFIX);
      strcat(extension_env_name, "-");
      strcat(extension_env_name, function_name);
      // retrieve the script environment from the register, for that
      // push onto the stack the value stored in the registry with key
      // "foo" where foo is the environment name
      // [-0, +1] > (+1)
      lua_rawgeti(L, LUA_REGISTRYINDEX, local_env_ref);
      //lua_getfield(L, LUA_REGISTRYINDEX, extension_env_name);
      elg_stack_dump(L);

      lua_rawgeti(M, LUA_REGISTRYINDEX, local_func_ref);
      elg_stack_dump(M);

      // remove _F
      lua_pushnil(M);
      lua_setfield(M, -2, "_F");
      elg_stack_dump(M);

      // a table at the L
      elg_stack_dump(L);
      move_table_values(L, M, 1);
      elg_stack_dump(M);
      elg_stack_dump(L);

      lua_pop(L, 1);
    }

    // -1 : remove the returned value from the top of the stack
    // -2 : remove the function environment
    // [-2, +0] > (+0)
    lua_pop(M, 2);
    elg_stack_dump(M);

    // unref the function environment
    luaL_unref(M, LUA_REGISTRYINDEX, local_func_ref);
    elg_stack_dump(M);

    // unref the thread
    luaL_unref(L, LUA_REGISTRYINDEX, m_refkey);
    elg_stack_dump(L);

    return retval;
  }

  // push string into stack
  // [-0, +1, m] > (+1)
  void pushlstring(const char* value, size_t len) {
    lua_pushlstring(L, value, len);
  }

  // push string into stack
  // [-0, +1, m] > (+1)
  void pushstring(const char* value) {
    lua_pushstring(L, value);
  }

//     problem with void push(void* p)
//     can be solved with http://stackoverflow.com/q/4610503
  // push integer into stack
  void pushinteger(int i) {
   lua_pushinteger(L, (lua_Integer) i);
  }

  // push float into stack
  // [-0, +1, -]
  void pushnumber(float f) {
    lua_pushnumber(L, (lua_Number) f);
  }

  // push boolean into stack
  // [-0, +1, -]
  void pushboolean(int b) {
    lua_pushboolean(L, b);
  }

  // push null
  // [-0, +1, -]
  void pushnil() {
    lua_pushnil(L);
  }

  // push lua_pushlightuserdata or null
  // [-0, +1, -]
  void pushlightuserdata(void* p) {
    if (p) {
      lua_pushlightuserdata(L, p);
    } else {
      lua_pushnil(L);
    }
  }

  // push lua_pushuserdata or null
  // [-0, +1, -]
  void pushuserdata(void* p) {
    if (p) {
      lua_pushlightuserdata(L, p);
    } else {
      lua_pushnil(L);
    }
  }

  // push a Ustring or null
  // Ustring is wrapped into a UnitexString object
  // No memory is allocated for p
  void* pushlightustring(Ustring* p) {
    if (p) {
      return lua_pushlightobject(L, UnitexString)(p);
    } else {
      lua_pushnil(L);
      return NULL;
    }
  }

  // pops a value from the stack and sets it as the new
  // value of global name
  // [-1, +0, e]
  void setglobal(const char* name) {
    lua_setglobal(L, name);
  }

  // [-0, +0]
  void set_locate_call_params(OptimizedFst2State current_state,
                                int pos,
                                struct parsing_info** matches,
                                int *n_matches,
                                struct list_context* ctx,
                                struct locate_parameters* p) {
    // get the global table
    // [-0, +1] > (+1)
    lua_getglobal(L,"_G");

    // get the locate table
    // [-0, +1] > (+2)
    lua_pushliteral(L,ELG_GLOBAL_LOCATE);
    lua_gettable(L,-2);

    // add the current state to globals
    // [-0, +0] > (+2)
    set_lightuserdata(ELG_GLOBAL_LOCATE_STATE, current_state);

    // add pos to globals
    // [-0, +0] > (+2)
    set_integer(ELG_GLOBAL_LOCATE_POS, pos);

    // add matches to globals
    // [-0, +0] > (+2)
    set_lightuserdata(ELG_GLOBAL_LOCATE_MATCHES, matches);

    // add n_matches to globals
    // [-0, +0] > (+2)
    set_integer(ELG_GLOBAL_LOCATE_NUM_MATCHES,
                (n_matches == NULL) ? (-1) : (*n_matches));

    // add context to globals
    // [-0, +0] > (+2)
    set_lightuserdata(ELG_GLOBAL_LOCATE_CONTEXT, ctx);

    // -2 pop the locate table
    // -1 pop the global table
    // [-2, +0] > (+0)
    lua_pop(L,2);
  }

  // [-0, +0]
   void set_morphological_locate_call_params(int current_state_index,
                                                 int pos_in_tokens,
                                                 int pos_in_chars,
                                                 struct parsing_info** matches,
                                                 int n_matches,
                                                 struct list_context* ctx,
                                                 struct locate_parameters* p,
                                                 unichar* jamo,
                                                 int pos_in_jamo,
                                                 unichar* content_buffer) {
     // get the global table
     // [-0, +1] > (+1)
     lua_getglobal(L, "_G");

     // get the locate table
     // [-0, +1] > (+2)
     lua_pushliteral(L, ELG_GLOBAL_MORPHO_LOCATE);
     lua_gettable(L, -2);

     // add the current state to globals
     // [-0, +0] > (+2)
     set_integer(ELG_GLOBAL_MORPHO_LOCATE_STATE_INDEX, current_state_index);

     // add pos in tokens to globals
     // [-0, +0] > (+2)
     set_integer(ELG_GLOBAL_MORPHO_LOCATE_POS_IN_TOKENS, pos_in_tokens);

     // add pos in chars to globals
     // [-0, +0] > (+2)
     set_integer(ELG_GLOBAL_MORPHO_LOCATE_POS_IN_CHARS, pos_in_chars);

     // add matches to globals
     // [-0, +0] > (+2)
     set_lightuserdata(ELG_GLOBAL_MORPHO_LOCATE_MATCHES, matches);

     // add n_matches to globals
     // [-0, +0] > (+2)
     set_integer(ELG_GLOBAL_MORPHO_LOCATE_NUM_MATCHES, n_matches);

     // add ctx to globals
     // [-0, +0] > (+2)
     set_lightuserdata(ELG_GLOBAL_MORPHO_LOCATE_CONTEXT, ctx);

     // add jamo to globals
     // [-0, +0] > (+2)
     set_lightuserdata(ELG_GLOBAL_MORPHO_LOCATE_JAMO, jamo);

     // add pos in jamo to globals
     // [-0, +0] > (+2)
     set_integer(ELG_GLOBAL_MORPHO_LOCATE_POS_IN_JAMO, pos_in_jamo);

     // add content to globals
     // [-0, +0] > (+2)
     set_lightuserdata(ELG_GLOBAL_MORPHO_LOCATE_CONTENT, content_buffer);

     // -2 pop the locate table
     // -1 pop the global table
     // [-2, +0] > (+0)
     lua_pop(L,2);
   }

  // UNITEX_EXPLICIT_CONVERSIONS
  operator lua_State*() const {
    return L;
  }

 private:


 protected:
  lua_State* L;
  int env;
  int local_env_ref;
  int main_env_ref_;
  int main_env_loaded_[ELG_MAIN_EVENTS_COUNT];
};
/* ************************************************************************** */
}      // namespace unitex
/* ************************************************************************** */
#endif // ELG_H_
