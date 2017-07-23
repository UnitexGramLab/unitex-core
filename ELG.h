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
#include "ELGLib/debug.h"
// commented on 23/07/17. Do not preserve stack
#include "ELGLib/ELGLib.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
static const char* UNITEX_SCRIPT_PATH =
    "/data/devel/projects/UnitexGramLab/unitex-core-elg/unitex-core/bin/Scripts/";
/* ************************************************************************** */
class vm {
 public:
  vm(void)
      : L(NULL), env(0) {
  }

  virtual ~vm(void) {
    stop();
  }

//  // traceback function taken straight out of luajit.c
//  static int trace(lua_State* L) {
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
    const char* error = lua_tostring(L, 1);
    fatal_error("Panic from Lua runtime %s\n", error);
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
  bool restart() {
    bool retval = false;

    if (is_running()) {
      stop();
    }

    // create a new Lua environment
    if ((L = luaL_newstate()) == NULL) {
      fatal_error("Failed to create a new Lua environment\n");
    }

    if (is_running()) {
      // load the standard library
      // [-0, +0] > (+0)
      luaL_openlibs(L);
      unitex::elg::stack_dump(L,"lua_getfield");

      // load the elg library
      // [-0, +0]
      elg::openlibs(L);
      unitex::elg::stack_dump(L,"lua_getfield");

      // setup the panic handler
      // [-0, +0] > (+0)
      lua_atpanic(L, panic);

      // run the initialization script
      // custom classes are not available right now from here
      // FIXME(martinec) Remove the hard-coded path
      // [-0, +0] > (+0)
      if (luaL_dofile(L,"/data/devel/projects/UnitexGramLab/unitex-core-elg/unitex-core/bin/Scripts/init.upp")) {
        fatal_error("Error running the initialization script: %s\n",
                    lua_tostring(L, -1));
      }

      // -------------------------------------------------------------------
      // register custom classes
      // -------------------------------------------------------------------

      // Constants
      // [-0, +1] > (+1)
      lua_newtable(L);
      set("API_VERSION", "0.1.0");
      // [-1, +0] > (+0)
      lua_setglobal(L, ELG_GLOBAL_CONSTANT);

      // Functions

      //uToken
      // [-0, +1] > (+1)
      lua_newtable(L);
      set("current", elg::token::current);
      set("previous", elg::token::previous);
      set("next", elg::token::next);
      set("at", elg::token::at);
      set("pos", elg::token::pos);
      set("is_space", elg::token::is_space);
      // [-1, +0] > (+0)
      lua_setglobal(L, ELG_GLOBAL_TOKEN);

      //uMatch
      // [-0, +1] > (+1)
      lua_newtable(L);
      set("begin", elg::match::begin);
      set("fend", elg::match::end);
      set("length", elg::match::length);
      set("content", elg::match::content);
      set("start_with_space", elg::match::start_with_space);
      set("start_sentence", elg::match::start_sentence);
      set("end_sentence", elg::match::end_sentence);
      set("start_newline", elg::match::start_newline);
      set("end_newline", elg::match::end_newline);
      // [-1, +0] > (+0)
      lua_setglobal(L, ELG_GLOBAL_MATCH);

      //uString
      // [-0, +1] > (+1)
      lua_newtable(L);
      set("format", elg::string::format);
      // [-1, +0] > (+0)
      lua_setglobal(L, ELG_GLOBAL_STRING);

      // uEnvironment
      // [-0, +1] > (+1)
      lua_newtable(L);
      unitex::elg::stack_dump(L,"lua_getfield");
      // uLoaded
      // [-0, +1] > (+2)
      lua_newtable(L);
      // [-1, +0] > (+1)
      lua_setfield(L, -2,  ELG_ENVIRONMENT_LOADED);
      unitex::elg::stack_dump(L,"lua_getfield");
      // uCalled
      // [-0, +1] > (+2)
      lua_newtable(L);
      // [-1, +0] > (+1)
      lua_setfield(L, -2,  ELG_ENVIRONMENT_CALLED);
      unitex::elg::stack_dump(L,"lua_getfield");
      // [-1, +0] > (+0)
      lua_setglobal(L, ELG_GLOBAL_ENVIRONMENT);
      unitex::elg::stack_dump(L,"lua_getfield");

      //uMisc
      // isWindows

      // -------------------------------------------------------------------
      // end of register custom classes
      // -------------------------------------------------------------------

      // successful restart
      retval = true;
    }
    return retval;
  }

  void stop() {
    if (is_running()) {
      lua_gc(L, LUA_GCCOLLECT, 0);
      lua_close(L);
      L = NULL;
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

  bool is_running() {
    return L;
  }

  // [-0, +0]
  void add_environment(const char* environment_name) {
    // save environment name
    // [-0, +1] > (+2)
    lua_getglobal(L, ELG_GLOBAL_ENVIRONMENT);
    unitex::elg::stack_dump(L,"lua_getfield");
    // the ELG_GLOBAL_ENVIRONMENT table should be at the top of the stack
    luaL_checktype(L, -1, LUA_TTABLE);
    unitex::elg::stack_dump(L,"lua_getfield");
    // get ELG_ENVIRONMENT_LOADED
    lua_getfield(L, -1, ELG_ENVIRONMENT_LOADED);
    unitex::elg::stack_dump(L,"lua_getfield");
    // put the key
    lua_pushinteger(L,++env);
    unitex::elg::stack_dump(L,"lua_getfield");
    // put the environment name
    lua_pushstring(L, environment_name);
    unitex::elg::stack_dump(L,"lua_getfield");
    // set uEnvironment.uLoaded[i] = environment_name
    lua_settable(L,-3);
    unitex::elg::stack_dump(L,"lua_getfield");
    // [n-1, 0] > (+1)
    clean(-1);
  }

  bool unload(const char* function_name) {
//    luaL_unref(L, LUA_REGISTRYINDEX, 0);
    return true;
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

  // 05/09/16 load once
  // in:             (+0)
  // out: [-0, +2] > (+2)
  bool load(const char* function_name) {
    unitex::elg::stack_dump(L);

    // prepare script environment_name
    char environment_name[MAX_TRANSDUCTION_VAR_LENGTH] = { };

    // environment name = ELG_ENVIRONMENT_PREFIX-function_name
    strcat(environment_name, ELG_ENVIRONMENT_PREFIX);
    strcat(environment_name, "-");
    strcat(environment_name, function_name);

    // the registry is a global table accessible from the Lua C-API
    // we use it for both store extended functions and check if an
    // extended function is already stored

    // retrieve the script environment from the register, for that
    // push onto the stack the value stored in the registry with key
    // "foo" where foo is the environment name
    // [-0, +1] > (+1)
    lua_getfield(L, LUA_REGISTRYINDEX, environment_name);
    unitex::elg::stack_dump(L,"lua_getfield");

    // if the retrieved value is not the script environment, then
    // load once the file with the extended function implementations
    if (!lua_istable(L, -1)) {
      // pop the previous result (normally a LUA_TNIL) from the top
      // of the stack
      // [-1, +0] > (+0)
      lua_pop(L, 1);
      unitex::elg::stack_dump(L,"lua_pop");

      // prepare script_name and script_file variables
      char script_name[MAX_TRANSDUCTION_VAR_LENGTH]   = { };
      char script_file[MAX_TRANSDUCTION_VAR_LENGTH]   = { };

      // script name = function_name.upp
      strcat(script_name, function_name);
      strcat(script_name, ELG_FUNCTION_DEFAULT_EXTENSION);

      // script_file = /default/path/function_name.upp
      strcat(script_file, UNITEX_SCRIPT_PATH);
      strcat(script_file, script_name);

      // check if script_file exists

      // load the script as a Lua chunk; it does not run it
      // [-0, +1] > (+1)
      if (luaL_loadfile(L, script_file)) {
        fatal_error("Error loading @%s: %s\n", script_file,
                    lua_tostring(L, -1));
      }

      unitex::elg::stack_dump(L,"luaL_loadfile");
      // create a new empty table and pushes it onto the stack
      // we will use this table to holds the environment of the script
      // [-0, +1] > (+2)
      lua_newtable(L);
      unitex::elg::stack_dump(L,"lua_newtable");

      // create another new empty table and pushes it onto the stack
      // we will use this table to holds the global table and use then
      // as fallback
      // [-0, +1] > (+3)
      lua_newtable(L);
      unitex::elg::stack_dump(L,"lua_newtable");

      // now we try to inherit the global table

      // first, push onto the stack the value of _G, i.e. the environment
      // which holds all the global variables defined on L
      // [-0, +1] > (+4)
      lua_getglobal(L, "_G");
      unitex::elg::stack_dump(L,"lua_getglobal");

      // do t[k] = v, where t is the value at index -2 in the stack
      // and v is the value at the top of the stack, i.e.
      // __index is a versatile metamethod that allows us to use _G as
      // a "fallback" table if a key in the environment table doesn't exist
      // [-1, +0] > (+3)
      lua_setfield(L, -2, "__index");
      unitex::elg::stack_dump(L,"lua_setfield");

      // pops the table from the stack and sets it as the new metatable
      // for the value at the index -2
      // now the global table is the metatable
      // setmetatable({}, {__index=_G})
      // [-1, +0] > (+2)
      lua_setmetatable(L, -2);
      unitex::elg::stack_dump(L,"lua_setmetatable");

      // [-0, +1] > (+3)
      // duplicate environment table
      lua_pushvalue(L, -1);
      unitex::elg::stack_dump(L,"lua_setmetatable");

      // [-0, +1] > (+4)
      // duplicate environment table
      lua_pushvalue(L, -1);
      unitex::elg::stack_dump(L,"lua_setmetatable");

      // register the script environment on the script environment
      // using _S(elf) as key
      // [-1, +0] > (+3)
      lua_setfield(L, -2, "_S");
      unitex::elg::stack_dump(L,"lua_setfield");
      // register the script environment on the registry using the
      // environment_name as key
      // [-1, +0] > (+2)
      lua_setfield(L, LUA_REGISTRYINDEX, environment_name);
      unitex::elg::stack_dump(L,"lua_setfield");

      // retrieve the script environment from the register
      // [-0, +1] > (+2)
      // lua_getfield(L, LUA_REGISTRYINDEX, environment_name);
      unitex::elg::stack_dump(L,"lua_getfield");

      // in Lua functions, upvalues are the external local variables that
      // the function uses, and that are consequently included in its closure
      // the environment is the first upvalue, the next instruction set
      // as environment the script environment
      // (L, 1, 1) = L, points to the closure in the stack,
      // [-1, +0] > (+1)
      if (lua_setfenv(L, 1) == 0) {
        fatal_error("Error calling @%s: %s\n", function_name,
                    lua_tostring(L, -1));
      }

      unitex::elg::stack_dump(L,"lua_setupvalue");

      //  priming run: loads and runs script's main function
      // [-1, +0] > (+0)
      if (lua_pcall(L, 0, LUA_MULTRET, 0)) {  // LUA_MULTRET -> 0 ?
        fatal_error("Error calling @%s: %s\n", function_name,
                    lua_tostring(L, -1));
      }
      unitex::elg::stack_dump(L,"lua_pcall");

      // retrieve the script environment from the register
      // [-0, +1] > (+1)
      lua_getfield(L, LUA_REGISTRYINDEX, environment_name);
      unitex::elg::stack_dump(L,"lua_getfield");

      // if there are an onLoad() function, then run it
      // [-0, +1] > (+2)
      lua_getfield(L, -1, ELG_FUNCTION_ON_LOAD_NAME);
      unitex::elg::stack_dump(L,"lua_getfield");
      if( lua_isfunction(L, -1) ) {
//        push(p);
//        setglobal(ELG_GLOBAL_LOCATE_PARAMS);
        // [-1, +0] > (+1)
        if (lua_pcall(L, 0, 0, 0) != 0) {
          lua_remove (L, -2); // remove environment
          unitex::elg::stack_dump(L);
          fatal_error("Error loading @%s: %s:%s\n", function_name,
                      ELG_FUNCTION_ON_LOAD_NAME,
                      lua_tostring(L, -1));
        }
      } else {
        lua_pop(L, 1); // pop the function name
      }

      // push ...
      add_environment(environment_name);

    }  // if (!lua_istable(L,-1))

    // using the script environment
    // get the extended function to run
    // [-0, +1] > (+2)
    lua_getfield(L, -1, function_name);
    unitex::elg::stack_dump(L,"lua_getfield");
    if (!lua_isfunction(L, -1)) {
      lua_pop(L, 2); // pop the environment + returned field
      fatal_error("Error loading @%s, function doesn't exists\n",
                  function_name);
    }

    return true;
  }

  // call the function on the stack
  // in:                 (+2+n) 1:environment, 2:function, n:params
  // out: [-0, -(2+n)] > (+0)
  // returns:
  //  0 : step back
  //  1 : step forward
  int call(const char* function_name, int nargs, struct stack_unichar* stack) {
    unitex::elg::stack_dump(L);
    int retval = 1;

    // do the call (lua_State *L, int nargs, int nresults, int errfunc)
    // nresults => 1, one result expected
    // errfunc  => 0, the error message returned on the stack is exactly
    //                the original error message
    // [-(n + 1), +1] > (+2) 1:environment, 2:returned value
    if (lua_pcall(L, nargs, 1, 0) != 0) {
      lua_remove (L, -2); // remove environment
      unitex::elg::stack_dump(L);
      fatal_error("Error calling @%s: %s\n", function_name,
                  lua_tostring(L, -1));
    }
    unitex::elg::stack_dump(L);
    int type = lua_type(L, -1);

    // retrieve boolean or string result
    if (!(type == LUA_TNIL || type == LUA_TBOOLEAN || type == LUA_TLIGHTUSERDATA
        || type == LUA_TNUMBER || type == LUA_TSTRING)) {
      lua_pop(L, 2); // table + result
      fatal_error(
          "Error calling @%s, function  must return a boolean, a number, a string or a null value\n",
          function_name, lua_tostring(L, -1));
    }

    // LUA_TTABLE     -> lua_topointer(L, -1)
    // LUA_TFUNCTION  -> lua_topointer(L, -1)
    // LUA_TUSERDATA  -> lua_touserdata(L, -1)
    // LUA_TTHREAD    -> (void *)lua_tothread(L, -1)

    if (type == LUA_TNIL) {
      retval = 0;
    } else if (type == LUA_TBOOLEAN) {
      // if the function returns false
      // there is not a following transition
      // if the return value is equal to true,
      // the output stack stays unchanged, this
      // is equivalent to push an empty symbol
      if (!lua_toboolean(L, -1)) {
        retval = 0;
      }
    } else if (type == LUA_TLIGHTUSERDATA) {
      const unichar* S = (const unichar*) lua_touserdata(L, -1);
      // push to stack
      for (int i = 0; S[i] != '\0'; ++i) {
        ::push(stack, S[i]);
      }
    } else if (type == LUA_TNUMBER) {
      // convert the number returned to string
      char n[LUAI_MAXNUMBER2STR];
      size_t count = lua_number2str(n, lua_tonumber(L, -1));
      // push to stack
      push_array(stack, n, count);
//        12.09.16 return utf8
//          for (int i=0; n[i]!='\0'; ++i) {
//           ::push(stack,n[i]);
//          }
    } else if (type == LUA_TSTRING) {
      const char* s = lua_tostring(L, -1);
      unichar S[4096] = { 0 };
      size_t count = u_decode_utf8(s, S);
      // push to stack
      push_array(stack, S, count);
//        12.09.16 return utf8
//          for (int i=0; s[i]!='\0'; ++i) {
//            ::push(stack,s[i]);
//          }
    }

    // remove the returned value from the top of the stack
    lua_pop(L, 1);
    unitex::elg::stack_dump(L);
    // remove the environment
    lua_pop(L, 1);
    unitex::elg::stack_dump(L);

    return retval;
  }

  // push a function into table
  // [-0, +0, e]
  void set(const char* name, lua_CFunction fn) {
    // [-0, +1] > (+1)
    lua_pushstring(L, name);  // lua_pushliteral
    // [-0, +1] > (+2)
    lua_pushcfunction(L, fn);
    // [-2, +0] > (+0)
    lua_settable(L, -3);
  }

  // push a string into table
  // [-0, +0, e]
  void set(const char* name, const char* value) {
    // [-0, +1] > (+1)
    lua_pushstring(L, name);
    // [-0, +1] > (+2)
    lua_pushstring(L, value);
    // [-2, +0] > (+0)
    lua_settable(L, -3);
  }

  // push a integer into table
  // [-0, +0, e]
  void set(const char* name, int value) {
    // [-0, +1] > (+1)
    lua_pushstring(L, name);
    // [-0, +1] > (+2)
    lua_pushinteger(L, value);
    // [-2, +0] > (+0)
    lua_settable(L, -3);
  }

  // push string into stack
  // [-0, +1, m] > (+1)
  void push(const char* value, size_t len) {
    lua_pushlstring(L, value, len);
  }

//     problem with void push(void* p)
//     can be solved with http://stackoverflow.com/q/4610503
//     // push integer into stack
//     void push(int i) {
//       lua_pushinteger(L, (lua_Integer) i);
//     }

  // push float into stack
  // [-0, +1, -]
  void push(float f) {
    lua_pushnumber(L, (lua_Number) f);
  }

  // push boolean into stack
  // [-0, +1, -]
  void push(bool b) {
    lua_pushboolean(L, b);
  }

  // push null
  // [-0, +1, -]
  void push() {
    lua_pushnil(L);
  }

  // push lua_pushlightuserdata or null
  // [-0, +1, -]
  void push(void* p) {
    if (p) {
      lua_pushlightuserdata(L, p);
    } else {
      push();
    }
  }

  // pops a value from the stack and sets it as the new
  // value of global name
  // [-1, +0, e]
  void setglobal(const char* name) {
    lua_setglobal(L, name);
  }

  // UNITEX_EXPLICIT_CONVERSIONS
  operator lua_State*() const {
    return L;
  }

 private:

 protected:
  lua_State* L;
  int env;
};
/* ************************************************************************** */
}      // namespace unitex
/* ************************************************************************** */
#endif // ELG_H_
