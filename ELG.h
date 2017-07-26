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
#include "ELGLib/ELGLib.h"
#include "ELGLib/copy_state.h"
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
class vm {
 public:
  vm(void)
      : L(), env(0) {
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
  bool restart() {
    bool retval = false;

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

      // run the initialization script
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

  bool is_running() {
    return L;
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
    const char* environment_name = {};
    // [-1, +(2|0)] > (+(4|2))
    while (lua_next(L, -2)) {
        elg_stack_dump(L);
        // check if key is a number
        if (lua_isnumber(L, -2)) {
          // check if value is a integer
          if (lua_isstring(L, -1)) {
              environment_name = lua_tostring(L, -1);
              // [-0, +0] > (+4)
              unload_environment(environment_name);
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
  void unload_environment(const char* environment_name) {
    // retrieve the script environment from the register
    // [-0, +1] > (+1)
    lua_getfield(L, LUA_REGISTRYINDEX, environment_name);
    elg_stack_dump(L);

    // if there are an onUnload() function, then run it
    // [-0, +1] > (+2)
    lua_getfield(L, -1, ELG_FUNCTION_ON_UNLOAD_NAME);
    elg_stack_dump(L);
    if( lua_isfunction(L, -1) ) {
//        push(p);
//        setglobal(ELG_GLOBAL_LOCATE_PARAMS);
      // [-1, +0] > (+1)
      if (lua_pcall(L, 0, 0, 0) != 0) {
        const char* e = lua_tostring(L, -1);
        elg_stack_dump(L);
        lua_pop(L, 1); // error
        elg_stack_dump(L);
        luaL_error(L,"Error loading @%s: %s:%s\n",
                      environment_name,
                      ELG_FUNCTION_ON_LOAD_NAME,
                      e);
      }
    } else {
      // pop the return value that is not a function
      lua_pop(L, 1);
    }

    // pop the the script environment
    lua_pop(L, 1);

    // delete the environment name
    elg_stack_dump(L);
  }

  void call_unload(const char* environment_name) {

  }

  // -1: environment
  // [-0, +0] > (+1)
  void call_onload(const char* environment_name) {
    // if there are an onLoad() function, then run it
    // [-0, +1] > (+2)
    lua_getfield(L, -1, ELG_FUNCTION_ON_LOAD_NAME);
    elg_stack_dump(L);
    if( lua_isfunction(L, -1) ) {
//        push(p);
//        setglobal(ELG_GLOBAL_LOCATE_PARAMS);
      // [-1, +0] > (+1)
      if (lua_pcall(L, 0, 0, 0) != 0) {
        const char* e = lua_tostring(L, -1);
        elg_stack_dump(L);
        lua_pop(L, 1);
        elg_stack_dump(L);
        luaL_error(L,"Error loading @%s: %s:%s\n",
                      environment_name,
                      ELG_FUNCTION_ON_LOAD_NAME,
                      e);
      }
    } else {
      // pop the returned value
      lua_pop(L, 1);
    }
  }

  // [-0, +0] > (+1)
  void save_name(const char* environment_name) {
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
    lua_pushstring(L, environment_name);
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

  // receives a function chunk at the top of the stack
  static int setup_extension_environment(lua_State* L) {
    // create a new empty table (e) and pushes it onto the stack
    // we will use this table to holds the environment of the script
    // [-0, +1] > (+1)
    lua_newtable(L);
    elg_stack_dump(L);

    // create another new empty table (f) and pushes it onto the stack
    // we will use this as the metatable that holds the original global
    // table (_G)
    // [-0, +1] > (+2)
    lua_newtable(L);
    elg_stack_dump(L);

    // push onto the stack the value of _G, i.e. the environment
    // which holds all the global variables defined on L
    // [-0, +1] > (+3)
    lua_getglobal(L, "_G");
    elg_stack_dump(L);

    // do f[__index] = _G, where f is the table at index -2
    // and _G is the value at the top of the stack.
    // __index is the versatile metamethod that allows us to use _G as
    // a "fallback" table if a key in the environment table doesn't exist
    // [-1, +0] > (+2)
    lua_setfield(L, -2, "__index");
    elg_stack_dump(L);

    // pops f from the stack and sets it as the new metatable
    // for e (the environment table)  at index -2
    // now f is the metatable of the environment table
    // setmetatable({}, {__index=_G})
    // [-1, +0] > (+1)
    lua_setmetatable(L, -2);
    elg_stack_dump(L);

    // [-0, +1] > (+2)
    // duplicate the environment table
    lua_pushvalue(L, -1);
    elg_stack_dump(L);

    // [-1, +0] > (+1)
    // replaces L globals
    lua_replace(L, LUA_GLOBALSINDEX);
    elg_stack_dump(L);

    // [-0, +1] > (+2)
    // duplicate the environment table
    lua_pushvalue(L, -1);
    elg_stack_dump(L);

    // [-0, +1] > (+3)
    // duplicate the environment table
    lua_pushvalue(L, -1);
    elg_stack_dump(L);

    // register the environment table on the environment table
    // using _S(elf) as key
    // [-1, +0] > (+2)
    lua_setfield(L, -2, "_S");
    elg_stack_dump(L);

    // pops the environment table from the stack and sets it as
    // the new environment of 1 . In this case, 1 is the chunk or
    // function that we have at the index 1 on the stack.
    // Take into account that every function starts with the
    // environment as its first upvalue, upvalues are the external
    // local variables that the function uses, and that are consequently
    // included in its closure
    // [-1, +0] > (+1)
    if (lua_setfenv(L, 1) == 0) {
      elg_stack_dump(L);
      elg_error(L,lua_tostring(L, -1));
    }
    elg_stack_dump(L);
    return 1;
  }

  // 05/09/16 load once
  // in:             (+0)
  // out: [-0, +2] > (+2)
  bool load(const char* function_name) {
    elg_stack_dump(L);

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
        const char* e = lua_tostring(L, -1);
        elg_stack_dump(L);
        lua_pop(L,1);
        elg_stack_dump(L);
        luaL_error(L,"Error loading %s: %s\n", script_file, e);
      }

      // setup a sandboxed envirnonment
      // [-0, +1] > (+2)
      setup_extension_environment(L);
      elg_stack_dump(L);

      // register the script environment on the registry using the
      // environment_name as key
      // [-1, +0] > (+1)
      lua_setfield(L, LUA_REGISTRYINDEX, environment_name);
      elg_stack_dump(L);

      //  priming run: loads and runs script's main function
      // [-1, +0] > (+0)
      if (lua_pcall(L, 0, 0, 0)) {  // LUA_MULTRET -> 0 ?
        const char* e = lua_tostring(L, -1);
        lua_pop(L,1);
        luaL_error(L, "Error calling @%s: %s\n", function_name, e);
      }
      elg_stack_dump(L);

      // retrieve the script environment from the register
      // [-0, +1] > (+1)
      lua_getfield(L, LUA_REGISTRYINDEX, environment_name);
      elg_stack_dump(L);

      // [-0, +0] > (+1)
      call_onload(environment_name);
      elg_stack_dump(L);

      // [-0, +0] > (+1)
      save_name(environment_name);
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

    elg_stack_dump(L);
    return true;
  }

  // L is the main thread, lua_State represents this thread
  // this function creates a new thread in L
  static lua_State* create_mirror_state(lua_State* L) {
    // creates a new thread, pushes it on the stack, and returns a
    // pointer to a lua_State that represents this new thread
    // The new state returned by this function shares with the original
    // state all global objects (such as tables), but has an independent
    // execution stack
    lua_State* M = lua_newthread(L);

    // anchor new thread
    int m_refkey = luaL_ref(L, LUA_REGISTRYINDEX);
    // lua_unref(M, m_refkey);
    return M;
  }

  // call the function on the stack
  // in:                 (+2+n) 1:environment, 2:function, n:params
  // out: [-0, -(2+n)] > (+0)
  // returns:
  //  0 : step back
  //  1 : step forward
  int call(const char* function_name, int nargs, struct stack_unichar* stack) {
    elg_stack_dump(L);
    int retval = 1;

//    lua_pushvalue(L,2);
//    // create a new empty table (e) and pushes it onto the stack
//    // we will use this table to holds the environment of the script
//    // [-0, +1] > (+1)
//    lua_newtable(L);
//    elg_stack_dump(L);
//
//    // create another new empty table (f) and pushes it onto the stack
//    // we will use this as the metatable that holds the original global
//    // table (_G)
//    // [-0, +1] > (+2)
//    lua_newtable(L);
//    elg_stack_dump(L);
//
//    // push onto the stack the value of _G, i.e. the environment
//    // which holds all the global variables defined on L
//    // [-0, +1] > (+3)
//    lua_getfield(L, 1, "_S");
//    elg_stack_dump(L);
//
//    // do f[__index] = _G, where f is the table at index -2
//    // and _G is the value at the top of the stack.
//    // __index is the versatile metamethod that allows us to use _G as
//    // a "fallback" table if a key in the environment table doesn't exist
//    // [-1, +0] > (+2)
//    lua_setfield(L, -2, "__index");
//    elg_stack_dump(L);
//
//    // pops f from the stack and sets it as the new metatable
//    // for e (the environment table)  at index -2
//    // now f is the metatable of the environment table
//    // setmetatable({}, {__index=_G})
//    // [-1, +0] > (+1)
//    lua_setmetatable(L, -2);
//    elg_stack_dump(L);
//
//    // [-0, +1] > (+2)
//    // duplicate the environment table
//    lua_pushvalue(L, -1);
//    elg_stack_dump(L);
//
//    // [-1, +0] > (+1)
//    // replaces L globals
//    lua_replace(L, LUA_GLOBALSINDEX);
//    elg_stack_dump(L);
//
//    // pops the environment table from the stack and sets it as
//    // the new environment of 1 . In this case, 1 is the chunk or
//    // function that we have at the index 1 on the stack.
//    // Take into account that every function starts with the
//    // environment as its first upvalue, upvalues are the external
//    // local variables that the function uses, and that are consequently
//    // included in its closure
//    // [-1, +0] > (+1)
//    if (lua_setfenv(L, -2) == 0) {
//      elg_stack_dump(L);
//      elg_error(L,lua_tostring(L, -1));
//    }
//    elg_stack_dump(L);
//    lua_pop(L,1);
//    elg_stack_dump(L);
//
////    elg_stack_dump(L);
////    setup_extension_environment(L);
////    lua_pop(L,2);
////    elg_stack_dump(L);

    // do the call (lua_State *L, int nargs, int nresults, int errfunc)
    // nresults => 1, one result expected
    // errfunc  => 0, the error message returned on the stack is exactly
    //                the original error message
    // [-(n + 1), +1] > (+2) 1:environment, 2:returned value
    if (lua_pcall(L, nargs, 1, 0) != 0) {
      const char* e = lua_tostring(L, -1);
      lua_pop(L,1); // error
      luaL_error(L, "Error calling @%s: %s\n", function_name,e);
    }
    elg_stack_dump(L);
    int type = lua_type(L, -1);

    // retrieve boolean or string result
    if (!(type == LUA_TNIL || type == LUA_TBOOLEAN || type == LUA_TLIGHTUSERDATA
        || type == LUA_TNUMBER || type == LUA_TSTRING)) {
      const char* e = lua_tostring(L, -1);
      lua_pop(L, 1); // error
      luaL_error(L,
          "Error calling @%s, function  must return a boolean, a number, a string or a null value\n",
          function_name, e);
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

    // remove the the environment and returned value
    // from the top of the stack
    // [-2, +0] > (+0)
    lua_pop(L, 2);
    elg_stack_dump(L);

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

  // push string into stack
  // [-0, +1, m] > (+1)
  void push(const char* value) {
    lua_pushstring(L, value);
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
