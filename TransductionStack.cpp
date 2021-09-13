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
 */
#include <lua.hpp>
#include <stdio.h>
#include "TransductionStack.h"
#include "Error.h"
#include "DicVariables.h"
#include "Korean.h"
#include "TransductionVariables.h"
#include "OutputTransductionVariables.h"
#include "VariableUtils.h"
#include "DebugMode.h"
//#include "Stack_unichar.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const int TRANSDUCTION_STACK_SIZE = 16383;

/**
 * This function returns a non zero value if c can be a part of a variable name;
 * 0 otherwise.
 */
#if !UNITEX_USE(BASE_UNICODE)
int is_variable_char(unichar c) {
return ((c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9') || c=='_');
}
#endif

/**
 * Pushes the given character.
 */
void push_input_char(struct stack_unichar* s,unichar c,int protect_dic_chars) {
if (protect_dic_chars && (c==',' || c=='.')) {
    /* We want to protect dots and commas because the Locate program can be used
     * by Dico to generate dictionary entries */
    push(s,'\\');
}
push(s,c);
}


/**
 * Pushes the given character.
 */
void push_output_char(struct stack_unichar* s,unichar c) {
push(s,c);
}


/**
 * Pushes the given string to the output, if not NULL.
 */
void push_input_string(struct stack_unichar* stack,unichar* s,int protect_dic_chars) {
int i;
if (s==NULL) {
   return;
}
for (i=0;s[i]!='\0';i++) {
    push_input_char(stack,s[i],protect_dic_chars);
}
}

/**
 * Pushes the given string to the output, if not NULL.
 */
void push_input_string(struct stack_unichar* stack,const char* s,int protect_dic_chars) {
int i;
if (s==NULL) {
   return;
}
for (i=0;s[i]!='\0';i++) {
    push_input_char(stack,s[i],protect_dic_chars);
}
}


/**
 * Pushes the given string to the output, if not NULL, in the limit of 'length' chars.
 */
void push_input_substring(struct stack_unichar* stack,unichar* s,int length,int protect_dic_chars) {
int i;
if (s==NULL) {
   return;
}
for (i=0;i<length && s[i]!='\0';i++) {
   push_input_char(stack,s[i],protect_dic_chars);
}
}


/**
 * Pushes the given string to the output, if not NULL.
 */
void push_output_string(struct stack_unichar* stack,unichar* s) {
push_input_string(stack,s,0);
}

/**
 * Pushes the given string to the output, if not NULL.
 */
void push_output_string(struct stack_unichar* stack,const char* s) {
push_input_string(stack,s,0);
}

//// A RAII class to make sure lua state gets closed when exceptions are thrown
//class LuaClose {
//    lua_State* L;
//public:
//    LuaClose(lua_State* state) : L(state) {}
//    ~LuaClose() {
//        lua_close(L);
//    }
//};



// A RAII class to make sure stack_unichar gets closed when exceptions are thrown
class StackClose {
    struct stack_unichar* stack;
public:
    StackClose(struct stack_unichar* stack) : stack(stack) {}
    ~StackClose() {
      free_stack_unichar(stack);
    }
};

//// traceback function taken straight out of luajit.c
//static int traceback(lua_State *L)
//{
//    if (!lua_isstring(L, 1)) { /* Non-string error object? Try metamethod. */
//        if (lua_isnoneornil(L, 1) ||
//                !luaL_callmeta(L, 1, "__tostring") ||
//                !lua_isstring(L, -1))
//            return 1;  /* Return non-string error object. */
//        lua_remove(L, 1);  /* Replace object by result of __tostring metamethod. */
//    }
//    luaL_traceback(L, L, lua_tostring(L, 1), 1);
//    return 1;
//}

/**
 * This function processes the given extended output string.
 *
 * Returns
 *
 * 0: output error or not satisfied
 *    for instance, if a variable is not correctly defined or
 *    an extended function isn't satisfied
 * 1: output it's OK
 *
 * IMPORTANT: every new feature added here to handle new things in outputs
 *            should also be reported into Grf2Fst2_lib->check_dollar_sequence
 */
int process_extended_output(unichar* s,
                   struct locate_parameters* p,
                   int capture_in_debug_mode,
                   struct extended_output_render* r,
                   OutputVariables* extra_variables) {
int old_stack_pointer=r->stack_template->top;
int i1=0;
if (capture_in_debug_mode) {
    /* If we have a capture in debug mode, we must skip the initial char #1 */
    i1++;
}
if (s==NULL) {
   /* We do nothing if there is no output */
   return 1;
}

//u_printf("######/[%S]\n",s);
unichar variable_name[MAX_TRANSDUCTION_VAR_LENGTH]   = {0};
int variable_index = -1;

// function name
unichar function_name[FILENAME_MAX]  = {0};
char char_function_name[FILENAME_MAX] = {0};

// extension name
unichar extension_name[FILENAME_MAX]   = {0};
char char_extension_name[FILENAME_MAX] = {0};

// a boolean parameter
int tboolean_parameter = 0;

// a number parameter
int tnumber_parameter = -1;

// a userdata parameter
void* tuserdata  = NULL;

// a lightuserdata parameter
void* tlightuserdata  = NULL;

// a Ustring parameter
Ustring* tustring  = NULL;

// a char string parameter
char tstring_parameter_stack[4096*6] = {0};

// a unichar parameter stack
struct stack_unichar* parameter_stack = new_stack_unichar(4096);
StackClose stack_closer(parameter_stack);

for (;;) {
    /* First, we push all chars before '\0' or '$' */
    int char_to_push_count = 0;
    while ((s[i1 + char_to_push_count] != '\0')
        && (s[i1 + char_to_push_count] != '$')
        && (s[i1 + char_to_push_count] != DEBUG_INFO_COORD_MARK)) {
      char_to_push_count++;
    }

    if (char_to_push_count!=0) {
        push_array(r->stack_template,&s[i1],char_to_push_count);
        i1+=char_to_push_count;
    }

    if (s[i1]=='\0') {
        return 1;
    }

    if (s[i1]==DEBUG_INFO_COORD_MARK) {
        /* If we have found the debug mark indicating the end of the real output,
         * we rawly copy the end of the output without interpreting it,
         * or return if we were in capture mode */
        if (capture_in_debug_mode) return 1;
        char_to_push_count=u_strlen(s+i1);
        push_array(r->stack_template,&s[i1],char_to_push_count);
        return 1;
    }
    /* Now we are sure to have s[i1]=='$' */
    {
/* ************************************************************************** */
      /* Case of an extended function call $@extension.function()$ */
      if (s[i1+1]=='@') {
        int name_length=0;
        i1 = i1+2 ;
        extension_name[0] = '\0';

        while (u_is_identifier(s[i1]) && name_length<MAX_TRANSDUCTION_VAR_LENGTH) {
           extension_name[name_length++]=s[i1++];
        }

        if (name_length>=MAX_TRANSDUCTION_VAR_LENGTH) {
           fatal_error("Too long extension name (>%d chars) in following extended output:\n%S\n",MAX_TRANSDUCTION_VAR_LENGTH,s);
        }

        extension_name[name_length]='\0';
//        u_printf("%S\n",function_name);

        // function name
        function_name[0] = '\0';
        if (s[i1]=='.') {
          name_length=0;
          i1 = i1+1 ;
          while (u_is_identifier(s[i1]) && name_length<MAX_TRANSDUCTION_VAR_LENGTH) {
             function_name[name_length++]=s[i1++];
          }
          if (name_length>=MAX_TRANSDUCTION_VAR_LENGTH) {
             fatal_error("Too long extended function name (>%d chars) in following extended output:\n%S\n",MAX_TRANSDUCTION_VAR_LENGTH,s);
          }
          function_name[name_length]='\0';
        }

        if (s[i1]!='(') {
          fatal_error("Function error: missing open parenthesis after @%S\n",function_name);
        }

        i1++;

//        // create a new lua environment
//        lua_State* L = luaL_newstate();
//
//        // make sure it gets closed even when exception is thrown
//        LuaClose closer(L);
//
//        // load the standard library
//        luaL_openlibs(L);
//
//        // add the traceback function to the stack
//        lua_pushcfunction(L, traceback);

        // load the file, compile it as a function,
        // and push the compiled function onto the lua stack
        char_extension_name[0] = '\0';
        char_function_name[0] = '\0';
//        char char_script_name[MAX_TRANSDUCTION_VAR_LENGTH] = {};
//        char char_script_file[MAX_TRANSDUCTION_VAR_LENGTH] = {};

        // convert the extension name to char
        u_encode_utf8(extension_name,char_extension_name);
        // convert the function name to char
        // if function_name is null use the extension_name
        u_encode_utf8(function_name[0] ? function_name : extension_name, char_function_name);
//        strcat(char_script_name,char_function_name);
//        strcat(char_script_name,".upp");
//        strcat(char_script_file,UNITEX_SCRIPT_PATH);
//        strcat(char_script_file,char_script_name);

//        //  priming run: loads and runs script's main function, i.e. loadfile + lua_pcall
//        if (luaL_dofile(L, char_script_file)) {
//            fatal_error("Error calling @%S: %s\n",function_name,lua_tostring(L, -1));
//        }
//
//        //  tell what function to run
//        lua_getglobal(L, char_function_name);
//        if(!lua_isfunction(L,-1)){
//          lua_pop(L,1);
//          fatal_error("Error loading @%S, function doesn't exists\n",function_name);
//        }

        p->elg->load_extension(char_extension_name, char_function_name);

        variable_name[0] = '\0';
        variable_index   = -1;
        int variable_lenght = 0;

        // reset parameter_stack
        int script_params_count = 0;
        empty(parameter_stack);
        tstring_parameter_stack[0] = '\0';

        param_type_t param_type = PARAM_TNONE;
        variable_pass_type_t variable_pass_type = VARIABLE_PASS_BY_VALUE;

        struct transduction_variable* v=NULL;

        // Save function params
        read_script_param:

        /* First, we push all chars before '\0' or '$' or ',' or ')'*/

//        for (int ww=0; ww < p->buffer_size;++ww) {
//          u_printf("%S\n",p->tokens->value[p->buffer[ww]]);
//        }

        char_to_push_count=0;
        while ((s[i1+char_to_push_count]!='\0') &&
               (s[i1+char_to_push_count]!='$')  &&
               (s[i1+char_to_push_count]!='&')  &&
               (s[i1+char_to_push_count]!=')')  &&
               (s[i1+char_to_push_count]!=',')) {
          char_to_push_count++;
        }

        switch(s[i1]) {
          // 21.07.17 add varrefs
          case '&':
          case '$':
            variable_pass_type = s[i1] == '&' ? VARIABLE_PASS_BY_REFERENCE : VARIABLE_PASS_BY_VALUE;
            ++i1;
            switch(s[i1]) {
              // $$ or &&
              case '&':
              case '$':
                // $$ => $, && => &
                if(s[i1-1] == s[i1]) {
                  param_type = PARAM_TSTRING;
                  ::push(parameter_stack,s[i1]);
                  ++i1;
                } else {
                // &$ or $&
                  fatal_error("ELG error: bad string sequence %C%C found when calling @%S\n", s[i1-1], s[i1], function_name);
                }
                break;
              // &{foo} or ${foo}
              case '{':
                ++i1;
                variable_lenght = 0;
                while (u_is_identifier(s[i1]) && variable_lenght<MAX_TRANSDUCTION_VAR_LENGTH) {
                  variable_name[variable_lenght++]=s[i1++];
                }
                if (variable_lenght == 0) {
                   fatal_error("ELG error: ${%C} is an invalid variable name calling @%S\n", s[i1], function_name);
                }
                if (variable_lenght >= MAX_TRANSDUCTION_VAR_LENGTH) {
                   fatal_error("ELG error: too long variable name (%d chars) calling @%S\n", variable_lenght, function_name);
                }
                if(s[i1] != '}') {
                  fatal_error("ELG error: variable %S without closing braces calling @%S\n", variable_name, function_name);
                }
                ++i1;
                variable_name[variable_lenght]='\0';

                /* ************************************************************************** */
                /* Case of a script variable like ${a} or &{a} that can be either a
                 * normal one or an output one, passed by value ($) or reference (&) */
                v=get_transduction_variable(p->input_variables, variable_name, &variable_index);
                // the variable is not an input one
                if (v==NULL) {
                  /* Not a normal one ? Maybe an output one */
                  Ustring* output = get_mutable_output_variable(p->output_variables, variable_name);
                  // Starting v4.0 Fst2List is dealing with input/output variables. Output
                  // variables are tracked by locate_parameters.output_variables but (I do not understand
                  // the reason for this) Input variables are tracked with the function parameter
                  // extra_variables, i.e. not using locate_parameters.input_variables as might be
                  // expected. For now the following lines are a dirty hotfix.
                  // FIXME() begins --------------------------------------------------------------
                  if (extra_variables!=NULL && output==NULL) {
                    output=get_mutable_output_variable(extra_variables,variable_name);
                  }
                  // FIXME() ends    --------------------------------------------------------------
                  // neither input nor output variable exists
                  if (output==NULL) {
                      if (variable_pass_type == VARIABLE_PASS_BY_VALUE) {
                        if(param_type == PARAM_TNONE || param_type == PARAM_TSTRING) {
  //                    switch (p->variable_error_policy) {
  //                    case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: undefined variable $%S$\n",variable_name); break;
  //                    case IGNORE_VARIABLE_ERRORS: /* same as BACKTRACK_ON_VARIABLE_ERRORS */
  //                    case BACKTRACK_ON_VARIABLE_ERRORS: r->stack->stack_pointer=old_stack_pointer; return 0;
  //                    }
                        // 01.08.15 : to handle nil params
                        // if the referenced variable is part of other arguments
                        // e.g func(a,b${c},d)
                        if(!is_empty(parameter_stack)) {
                          param_type = PARAM_TSTRING;
                          push_output_string(parameter_stack,"");
                        } else {
                          // isolated parameter treat as nil
                          param_type = PARAM_TNIL;
                        }
                      } else {
                        fatal_error("ELG error: calling @%S, parameter %d: attempt to concatenate incompatible types into a single parameter\n", function_name, script_params_count+1);
                      }
                    } else {
                      fatal_error("ELG error: calling @%S with variable &{%S}: it is not possible to get a reference of an undefined variable\n", function_name, variable_name);
                    }
                  // output variable exists
                  } else {
                    if (variable_pass_type == VARIABLE_PASS_BY_VALUE) {
                      if(param_type == PARAM_TNONE || param_type == PARAM_TSTRING) {
                        // 10.09.16 : fix a bug when dealing with a null parameter
                        // after an argument
                        // e.g func(a,${b},c) with b => nil
                        if(output->str && (*output->str) != U_NULL ) {
                          param_type = PARAM_TSTRING;
                          push_output_string(parameter_stack,output->str);
                        } else {
                          // 21.08.17 : to handle empty output within literal arguments
                          // isolated parameter treat as nil if is alone
                          // 01.08.15 : to handle nil params
                          // if the referenced variable is part of other arguments
                          // e.g func(a,b${c},d)
                          if(!is_empty(parameter_stack)) {
                            param_type = PARAM_TSTRING;
                            push_output_string(parameter_stack,"");
                          } else {
                            // isolated parameter treat as nil
                            param_type = PARAM_TNIL;
                          }
                        }
                      } else {
                        fatal_error("Error calling @%S, parameter %d: attempt to concatenate incompatible types into a single parameter\n", function_name, script_params_count+1);
                      }
                    // the variable is an output one and their content will be passed by reference
                    } else {
                      if(param_type != PARAM_TNONE) {
                        fatal_error("Error calling @%S, parameter %d, variable &{%S}: attempt to concatenate a value to a variable reference\n", function_name, script_params_count+1, variable_name);
                      } else {
                        param_type = PARAM_TLIGHTUSTRING;
                        tustring   = output;
                      }
                    }
                  }
                // the variable is an input one and will be passed by reference
                } else if (variable_pass_type == VARIABLE_PASS_BY_REFERENCE) {
                    if(!is_empty(parameter_stack)) {
                      fatal_error("ELG error: calling @%S, parameter %d, variable &{%S}: attempt to concatenate a string to a variable reference\n", function_name, script_params_count+1, variable_name);
                    } else {
                      param_type = PARAM_TNUMBER;
                      tnumber_parameter = variable_index;
                    }
                // the variable is an input one and will be passed by value
                } else if (v->start_in_tokens==UNDEF_VAR_BOUND) {
                  // 01.08.15 : to handle nil params
//                   switch (p->variable_error_policy) {
//                      case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: starting position of variable $%S$ undefined\n",variable_name); break;
//                      case IGNORE_VARIABLE_ERRORS: /* same as BACKTRACK_ON_VARIABLE_ERRORS */
//                      case BACKTRACK_ON_VARIABLE_ERRORS: r->stack->stack_pointer=old_stack_pointer; return 0;
//                   }
                  // 01.08.15 : to handle nil params
                  // if the referenced variable is part of other arguments
                  // e.g func(a,b${c},d)
                  if(!is_empty(parameter_stack)) {
                    param_type = PARAM_TSTRING;
                    push_output_string(parameter_stack,"");
                  } else {
                    param_type = PARAM_TNIL;
                  }
                } else if (v->end_in_tokens==UNDEF_VAR_BOUND) {
                   switch (p->variable_error_policy) {
                      case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position of variable $%S$ undefined\n",variable_name); break;
                      case IGNORE_VARIABLE_ERRORS: /* same as BACKTRACK_ON_VARIABLE_ERRORS */
                      case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                   }
                } else if (v->start_in_tokens>v->end_in_tokens
                    || (v->start_in_tokens==v->end_in_tokens && v->end_in_chars==-1 && v->end_in_chars<v->start_in_chars)) {
                   switch (p->variable_error_policy) {
                      case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position before starting position for variable $%S$\n",variable_name); break;
                      case IGNORE_VARIABLE_ERRORS: /* same as BACKTRACK_ON_VARIABLE_ERRORS */
                      case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                   }
                } else if (v->end_in_tokens+p->current_origin > p->buffer_size) {
                   if (p->variable_error_policy != EXIT_ON_VARIABLE_ERRORS) {
                     error("Output warning: end variable position after end of text for variable $%S$\n",variable_name);
                     /*error("start=%d  end=%d   origin=%d   buffer size=%d\n",v->start_in_tokens,v->end_in_tokens,p->current_origin,p->buffer_size);
                     for (int i=p->current_origin;i<p->buffer_size;i++) {
                       error("%S",p->tokens->value[p->buffer[i]]);
                     }
                     error("\n");*/
                    }
                   switch (p->variable_error_policy) {
                      case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end variable position after end of text for variable $%S$\n",variable_name); break;
                      case IGNORE_VARIABLE_ERRORS:  /* same as BACKTRACK_ON_VARIABLE_ERRORS */
                      case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                   }
                } else {
                  param_type = PARAM_TSTRING;
                  /* If the normal variable definition is correct */
                  /* Case 1: start and end in the same token*/
                  if (v->start_in_tokens==v->end_in_tokens-1) {
                    unichar* tok=p->tokens->value[p->buffer[v->start_in_tokens+p->current_origin]];
                    int last=(v->end_in_chars!=-1) ? (v->end_in_chars) : (((int)u_strlen(tok))-1);
                    for (int k=v->start_in_chars;k<=last;k++) {
                      push_input_char(parameter_stack,tok[k],p->protect_dic_chars);
                    }
                  } else if (v->start_in_tokens==v->end_in_tokens) {
                    /* If the variable is empty, do nothing */
                  } else {
                    /* Case 2: first we deal with first token */
                    unichar* tok=p->tokens->value[p->buffer[v->start_in_tokens+p->current_origin]];
                    push_input_string(parameter_stack,tok+v->start_in_chars,p->protect_dic_chars);
                    /* Then we copy all tokens until the last one */
                      for (int k=v->start_in_tokens+1;k<v->end_in_tokens-1;k++) {
                        push_input_string(parameter_stack,p->tokens->value[p->buffer[k+p->current_origin]],p->protect_dic_chars);
                      }

                      /* Finally, we copy the last token */

                      if ((v->end_in_tokens-1+p->current_origin) < 0) {
                        error("v->end_in_tokens-1+p->current_origin is below 0\n");
                        error("start=%d  end=%d\n",v->start_in_tokens,v->end_in_tokens);
                      }
                      else {
                        tok=p->tokens->value[p->buffer[v->end_in_tokens-1+p->current_origin]];
                        int last=(v->end_in_chars!=-1) ? (v->end_in_chars) : (((int)u_strlen(tok))-1);
                        for (int k=0;k<=last;k++) {
                          push_input_char(parameter_stack,tok[k],p->protect_dic_chars);
                        }
                      }
                  }
                }
                /* ************************************************************************** */
              break;
              default:
               fatal_error("ELG error: missing opening braces after %C, parameter %d, while calling @%S\n", s[i1-1], script_params_count+1, function_name);
               break;
            }
            break;
          case ',':
          case ')':
            // 10.09.16
            // Fix to allow empty arguments f(a,,b)
            // 03.07.17 -> 08.07.17 -> 21.08.17
            // fix no break at the end case
            if(s[i1]==',' && param_type == PARAM_TNONE && is_empty(parameter_stack)) {
              param_type = PARAM_TNIL;
            }

            // TODO(martinec) number (pushnumber),
            // integer(lua_pushinteger), string (lua_pushstring)
            switch(param_type) {
              case PARAM_TNONE:
                // this is not supposed to happen
                if(!is_empty(parameter_stack)) {
                  fatal_error("FIX THIS\n",function_name);
                }
                break;
              case PARAM_TNIL:
                // push a nil value
                p->elg->pushnil();
                ++script_params_count;
                break;
              case PARAM_TBOOLEAN:
                // push a boolean
                p->elg->pushboolean(tboolean_parameter);
                ++script_params_count;
                break;
              case PARAM_TLIGHTUSERDATA:
                // push a light user data
                p->elg->pushlightuserdata(tlightuserdata);
                ++script_params_count;
                break;
              case PARAM_TNUMBER:
                // push a number
                p->elg->pushinteger(tnumber_parameter);
                ++script_params_count;
                break;
              case PARAM_TSTRING:
                // push the content of the paramater stack as an encoded utf8 string
                if(!is_empty(parameter_stack)) {
                  parameter_stack->buffer[parameter_stack->top+1]='\0';
                  // 12.09.16 add u_encode_utf8 to allow send unicode chars
                  int tstring_parameter_length = u_encode_utf8(parameter_stack->buffer,tstring_parameter_stack);
                  p->elg->pushlstring(tstring_parameter_stack, tstring_parameter_length);
                  empty(parameter_stack);
                  tstring_parameter_stack[0] = '\0';
                  ++script_params_count;
                }
                break;
              case PARAM_TTABLE:
                break;
              case PARAM_TFUNCTION:
                break;
              case PARAM_TUSERDATA:
                // push a user data
                p->elg->pushuserdata(tuserdata);
                ++script_params_count;
                break;
              case PARAM_TTHREAD:
                break;
              case PARAM_TLIGHTUSTRING:
                // push a ustring data
                p->elg->pushlightustring(tustring);
                ++script_params_count;
                break;
            }

            if (s[i1]==')') {
              ++i1;
              goto script_call;
            }

            // reset the type of parameter
            param_type = PARAM_TNONE;

            ++i1;
            break;

          case '\0':
            fatal_error("ELG error: missing closing parenthesis after script @%S call \n",function_name);
            break;

          default:
            if (char_to_push_count != 0) {
              // test for a function keyword: true, false, nil
              if (param_type == PARAM_TNONE) {
                // nil
                if (ELG_FUNCTION_KEYWORD_MATCH(NIL, &s[i1], char_to_push_count)) {
                  param_type = PARAM_TNIL;
                  i1 += char_to_push_count;
                  break;
                // true
                } else if (ELG_FUNCTION_KEYWORD_MATCH(TRUE, &s[i1], char_to_push_count)) {
                  param_type = PARAM_TBOOLEAN;
                  tboolean_parameter = 1;
                  i1 += char_to_push_count;
                  break;
                // false
                } else if (ELG_FUNCTION_KEYWORD_MATCH(FALSE, &s[i1], char_to_push_count)) {
                  param_type = PARAM_TBOOLEAN;
                  tboolean_parameter = 0;
                  i1 += char_to_push_count;
                  break;
                }
              }
              // a common string
              // FIXME(martinec) foo(${b}false)
              if (param_type == PARAM_TNONE   ||
                  param_type == PARAM_TSTRING ||
                  param_type == PARAM_TNIL) {
                  param_type = PARAM_TSTRING;
                  push_array(parameter_stack, &s[i1], char_to_push_count);
                  i1 += char_to_push_count;
                  break;
              }
              // two incompatible types
              fatal_error("ELG error: calling @%S, parameter %d, character %C: attempt to concatenate incompatible types into a single parameter\n", function_name, script_params_count+1, s[i1]);
            }
            break;
        }

        goto read_script_param;
        script_call:

        // 30.10.17 add a cut operator to have the control over the number of
        // outputs to process from an extended function which returns multiple
        // values
        int cut_after = CUT_AFTER_EXHAUSTIVELY_CHECK;

        // the cut operator: @func()!
        if (s[i1] == '!') {
          ++i1;
          cut_after = CUT_AFTER_N_MATCHES * 1;
        }

        // 25.08.17 check before execute
        // Closing function
        if(s[i1]!='$') {
          fatal_error("ELG error: missing $ after closing parentheses ) while calling @%S\n",function_name); break;
        }

        // 17.06.16: send locate params
//        lua_pushlightuserdata(L,p);
        // 26.06.17: send only once while allocating p
//        p->elg->push(p);
////        lua_setglobal(L, "u_params");
//        // [-1, +0] > (+0)
//        p->elg->setglobal(ELG_GLOBAL_LOCATE_PARAMS);
//        ++script_params_count;
//        p->elg->push(p->graph_filename);
//        p->elg->setglobal("stack_pointer");
        if(!p->elg->call(char_function_name,script_params_count,cut_after,r)) {
          r->stack_template->top=old_stack_pointer;
          p->elg->restore_local_environment();
//          p->elg->setup_local_environment();
          return 0;
        }

//        /* do the call (script_params_count arguments, 1 result (boolean or string)) */
//        if (lua_pcall(L, script_params_count, 1, 0) != 0) {
//            fatal_error("Error calling @%S: %s\n",function_name,lua_tostring(L, -1));
//        }
//
//        /* retrieve boolean or string result */
//        if (!(lua_type(L, -1) == LUA_TBOOLEAN ||
//              lua_type(L, -1) == LUA_TSTRING  ||
//              lua_type(L, -1) == LUA_TNUMBER)) {
//          fatal_error("Error calling @%S, function  must return a boolean, a number, or a string value\n",function_name,lua_tostring(L, -1));
//        }
//
//        if(lua_type(L, -1) == LUA_TBOOLEAN) {
//          bool continue_to_explore = lua_toboolean(L, -1);
//
//          /* Remove the boolean returned from the top of the stack. */
//          lua_pop(L, 1);
//
//          if(!continue_to_explore) {
//             r->stack->stack_pointer=old_stack_pointer;
//             return 0;
//           } else {
//             continue;
//           }
//        }
//
//        if (lua_type(L, -1) != LUA_TSTRING &&
//            lua_type(L, -1) != LUA_TNUMBER) {
//          fatal_error("Error calling @%S, function  must return a number or a string\n",function_name,lua_tostring(L, -1));
//        }
//
//        if (lua_type(L, -1) == LUA_TSTRING) {
//          push_output_string(r->stack,lua_tostring(L, -1));
//        }else{
//          // convert the number to
//          char buffer[LUAI_MAXNUMBER2STR];
//          lua_number2str(buffer, lua_tonumber(L, -1));
//          push_output_string(r->stack,buffer);
//        }
//
//
//        /* Remove the returned value from the top of the stack. */
//        lua_pop(L, 1);


        ++i1;
        continue;
        }

        //unichar parameter[MAX_TRANSDUCTION_VAR_LENGTH];
//      push_array(r->stack,&s[i1],parameter_length);
//      i1+=parameter_length;

/* ************************************************************************** */
      int l=0;
      if (s[i1+1]=='{') {
          /* If we have a weight of the form ${n}$ */
          int weight;
          unichar foo1,foo2;
          int ret=u_sscanf(s+i1+2,"%d%C%C%n",&weight,&foo1,&foo2,&l);
          int ok=ret==3 && weight>=0 && foo1=='}' && foo2=='$';
          if (!ok) {
              fatal_error("Output error: invalid weight definition %S\n",s+i1);
          }
          i1+=l+2;
	  p->weight= weight; //p->weight == - 1 ? weight : p->weight + weight;
          continue;
      }
      i1++;
      /* Case of a variable name */
      unichar name[MAX_TRANSDUCTION_VAR_LENGTH];
      while (u_is_identifier(s[i1]) && l<MAX_TRANSDUCTION_VAR_LENGTH) {
         name[l++]=s[i1++];
      }
      if (l==MAX_TRANSDUCTION_VAR_LENGTH) {
         fatal_error("Too long variable name (>%d chars) in following output:\n%S\n",MAX_TRANSDUCTION_VAR_LENGTH,s);
      }
      name[l]='\0';
      if (s[i1]!='$' && s[i1]!='.') {
         switch (p->variable_error_policy) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: missing closing $ after $%S\n",name); break;
            case IGNORE_VARIABLE_ERRORS: continue;
            case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
         }
      }
      if (s[i1]=='.') {
         /* Here we deal with the case of a field like $a.CODE$ */
         unichar field[MAX_TRANSDUCTION_FIELD_LENGTH];
         l=0;
         i1++;
         while (s[i1]!='\0' && s[i1]!='$' && l<MAX_TRANSDUCTION_FIELD_LENGTH) {
            field[l++]=s[i1++];
         }
         if (l==MAX_TRANSDUCTION_FIELD_LENGTH) {
            fatal_error("Too long field name (>%d chars) in following output:\n%S\n",MAX_TRANSDUCTION_FIELD_LENGTH,s);
         }
         field[l]='\0';
         if (s[i1]=='\0') {
            switch (p->variable_error_policy) {
               case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: missing closing $ after $%S.%S\n",name,field); break;
               case IGNORE_VARIABLE_ERRORS: continue;
               case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
            }
         }
         if (field[0]=='\0') {
            switch (p->variable_error_policy) {
               case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: empty field: $%S.$\n",name); break;
               case IGNORE_VARIABLE_ERRORS: continue;
               case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
            }
         }
         i1++;
         if (u_starts_with(field,"EQUAL=")) {
             int n=compare_variables(name,field+6,p,1); // strlen("EQUAL=") == 6
             if (n==VAR_CMP_EQUAL) {
                 continue;
             }
             if (n==VAR_CMP_DIFF) {
                 r->stack_template->top=old_stack_pointer;
                 return 0;
             }
             /* n==VAR_CMP_ERROR means an error while accessing variables */
             switch (p->variable_error_policy) {
                case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: empty field: $%S.$\n",name); break;
                case IGNORE_VARIABLE_ERRORS: /* This mode is not relevant for variable comparison,
                                              * so we consider it to be equivalent to backtrack */
                case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
             }
         } else if (u_starts_with(field,"EQUALcC=")) {
             int n=compare_variables(name,field+8,p,0); // strlen("EQUALcC=") == 8
             if (n==VAR_CMP_EQUAL) {
                 continue;
             }
             if (n==VAR_CMP_DIFF) {
                 r->stack_template->top=old_stack_pointer;
                 return 0;
             }
             /* n==VAR_CMP_ERROR means an error while accessing variables */
             switch (p->variable_error_policy) {
                case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: empty field: $%S.$\n",name); break;
                case IGNORE_VARIABLE_ERRORS: /* This mode is not relevant for variable comparison,
                                              * so we consider it to be equivalent to backtrack */
                case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
             }
         } else if (u_starts_with(field,"UNEQUAL=")) {
             int n=compare_variables(name,field+8,p,1); // strlen("UNEQUAL=") == 8
             if (n==VAR_CMP_DIFF) continue;
             if (n==VAR_CMP_EQUAL) {
                 r->stack_template->top=old_stack_pointer;
                 return 0;
             }
             /* n==VAR_CMP_ERROR means an error while accessing variables */
             switch (p->variable_error_policy) {
                case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: empty field: $%S.$\n",name); break;
                case IGNORE_VARIABLE_ERRORS: /* This mode is not relevant for variable comparison,
                                              * so we consider it to be equivalent to backtrack */
                case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
             }
         } else if (u_starts_with(field,"UNEQUALcC=")) {
             int n=compare_variables(name,field+10,p,0); // strlen("UNEQUALcC=") == 10
             if (n==VAR_CMP_DIFF) continue;
             if (n==VAR_CMP_EQUAL) {
                 r->stack_template->top=old_stack_pointer;
                 return 0;
             }
             /* n==VAR_CMP_ERROR means an error while accessing variables */
             switch (p->variable_error_policy) {
                case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: empty field: $%S.$\n",name); break;
                case IGNORE_VARIABLE_ERRORS: /* This mode is not relevant for variable comparison,
                                              * so we consider it to be equivalent to backtrack */
                case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
             }
         } else if (u_starts_with(field,"SUBSTR.")) {
             int n=compare_variables_substr(name,field+7,p,0); // strlen("UNEQUALcC=") == 10
             if (n==VAR_CMP_EQUAL) continue;
             if (n==VAR_CMP_DIFF) {
                 r->stack_template->top=old_stack_pointer;
                 return 0;
             }
             /* n==VAR_CMP_ERROR means an error while accessing variables */
             switch (p->variable_error_policy) {
                case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: empty field: $%S.$\n",name); break;
                case IGNORE_VARIABLE_ERRORS: /* This mode is not relevant for variable comparison,
                                              * so we consider it to be equivalent to backtrack */
                case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
             }
         } else if (u_starts_with(field,"NOT_SUBSTR.")) {
             int n=compare_variables_substr(name,field+11,p,0); // strlen("UNEQUALcC=") == 10
             if (n==VAR_CMP_DIFF) continue;
             if (n==VAR_CMP_EQUAL) {
                 r->stack_template->top=old_stack_pointer;
                 return 0;
             }
             /* n==VAR_CMP_ERROR means an error while accessing variables */
             switch (p->variable_error_policy) {
                case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: empty field: $%S.$\n",name); break;
                case IGNORE_VARIABLE_ERRORS: /* This mode is not relevant for variable comparison,
                                              * so we consider it to be equivalent to backtrack */
                case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
         }
      }


         else if (!u_strcmp(field,"TO_LOWER")) {
             /* Case of a variable like $a$ that can be either a normal one or an output one */
             struct transduction_variable* v=get_transduction_variable(p->input_variables,name);
             if (v==NULL) {
              /* Not a normal one ? Maybe an output one */
              const Ustring* output=get_output_variable(p->output_variables,name);
              if (output==NULL) {
                  switch (p->variable_error_policy) {
                      case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: undefined variable $%S$\n",name); break;
                      case IGNORE_VARIABLE_ERRORS: continue;
                      case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                  }
              }
              unichar* tmp = u_strdup(output->str);
              u_tolower(tmp);
              push_output_string(r->stack_template,tmp);
              free(tmp);
              } else if (v->start_in_tokens==UNDEF_VAR_BOUND) {
                 switch (p->variable_error_policy) {
                    case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: starting position of variable $%S$ undefined\n",name); break;
                    case IGNORE_VARIABLE_ERRORS: continue;
                    case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                 }
              } else if (v->end_in_tokens==UNDEF_VAR_BOUND) {
                 switch (p->variable_error_policy) {
                    case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position of variable $%S$ undefined\n",name); break;
                    case IGNORE_VARIABLE_ERRORS: continue;
                    case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                 }
              } else if (v->start_in_tokens>v->end_in_tokens
                          || (v->start_in_tokens==v->end_in_tokens && v->end_in_chars==-1 && v->end_in_chars<v->start_in_chars)) {
                 switch (p->variable_error_policy) {
                    case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position before starting position for variable $%S$\n",name); break;
                    case IGNORE_VARIABLE_ERRORS: continue;
                    case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                 }


                 /* begin of GV fix */
                 /* this fix is against a crash I found when (v->start_in_tokens+p->current_origin == p->buffer_size)
                    and v->start_in_tokens==v->end_in_tokens
                    here we known that v->start_in_tokens <= v->end_in_tokens
                    */

              } else if (v->end_in_tokens+p->current_origin > p->buffer_size) {
                 if (p->variable_error_policy != EXIT_ON_VARIABLE_ERRORS) {
                   error("Output warning: end variable position after end of text for variable $%S$\n",name);
                   /*error("start=%d  end=%d   origin=%d   buffer size=%d\n",v->start_in_tokens,v->end_in_tokens,p->current_origin,p->buffer_size);
                   for (int i=p->current_origin;i<p->buffer_size;i++) {
                       error("%S",p->tokens->value[p->buffer[i]]);
                   }
                   error("\n");*/
                 }
                 switch (p->variable_error_policy) {
                    case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end variable position after end of text for variable $%S$\n",name); break;
                    case IGNORE_VARIABLE_ERRORS: continue;
                    case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                 }

                 /* end of GV fix */


              } else {
                  /* If the normal variable definition is correct */
                  /* Case 1: start and end in the same token*/
                  if (v->start_in_tokens==v->end_in_tokens-1) {
                      unichar* tok=p->tokens->value[p->buffer[v->start_in_tokens+p->current_origin]];
                      int last=(v->end_in_chars!=-1) ? (v->end_in_chars) : (((int)u_strlen(tok))-1);
                      for (int k=v->start_in_chars;k<=last;k++) {
                          push_input_char(r->stack_template,u_tolower(tok[k]),p->protect_dic_chars);
                      }
                  } else if (v->start_in_tokens==v->end_in_tokens) {
                      /* If the variable is empty, do nothing */
                  } else {
                      /* Case 2: first we deal with first token */
                      unichar* tok=p->tokens->value[p->buffer[v->start_in_tokens+p->current_origin]];
                      unichar* tmp = u_strdup(tok+v->start_in_chars);
                      u_tolower(tmp);
                      push_input_string(r->stack_template,tmp,p->protect_dic_chars);
                      free(tmp);
                      /* Then we copy all tokens until the last one */
                      for (int k=v->start_in_tokens+1;k<v->end_in_tokens-1;k++) {
                          unichar* tmp2 = u_strdup(p->tokens->value[p->buffer[k+p->current_origin]]);
                          u_tolower(tmp2);
                          push_input_string(r->stack_template,tmp2,p->protect_dic_chars);
                          free(tmp2);
                      }

                      /* Finally, we copy the last token */

                      if ((v->end_in_tokens-1+p->current_origin) < 0) {
                          error("v->end_in_tokens-1+p->current_origin is below 0\n");
                          error("start=%d  end=%d\n",v->start_in_tokens,v->end_in_tokens);
                      }
                      else {
                          tok=p->tokens->value[p->buffer[v->end_in_tokens-1+p->current_origin]];
                          int last=(v->end_in_chars!=-1) ? (v->end_in_chars) : (((int)u_strlen(tok))-1);
                          for (int k=0;k<=last;k++) {
                            push_input_char(r->stack_template,u_tolower(tok[k]),p->protect_dic_chars);
                        }
                      }
                  }
              }
         }
         else if (!u_strcmp(field,"TO_UPPER")) {
             /* Case of a variable like $a$ that can be either a normal one or an output one */
             struct transduction_variable* v=get_transduction_variable(p->input_variables,name);
             if (v==NULL) {
              /* Not a normal one ? Maybe an output one */
              const Ustring* output=get_output_variable(p->output_variables,name);
              if (output==NULL) {
                  switch (p->variable_error_policy) {
                      case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: undefined variable $%S$\n",name); break;
                      case IGNORE_VARIABLE_ERRORS: continue;
                      case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                  }
              }
              unichar* tmp = u_strdup(output->str);
              u_toupper(tmp);
              push_output_string(r->stack_template,tmp);
              free(tmp);
              } else if (v->start_in_tokens==UNDEF_VAR_BOUND) {
                 switch (p->variable_error_policy) {
                    case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: starting position of variable $%S$ undefined\n",name); break;
                    case IGNORE_VARIABLE_ERRORS: continue;
                    case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                 }
              } else if (v->end_in_tokens==UNDEF_VAR_BOUND) {
                 switch (p->variable_error_policy) {
                    case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position of variable $%S$ undefined\n",name); break;
                    case IGNORE_VARIABLE_ERRORS: continue;
                    case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                 }
              } else if (v->start_in_tokens>v->end_in_tokens
                          || (v->start_in_tokens==v->end_in_tokens && v->end_in_chars==-1 && v->end_in_chars<v->start_in_chars)) {
                 switch (p->variable_error_policy) {
                    case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position before starting position for variable $%S$\n",name); break;
                    case IGNORE_VARIABLE_ERRORS: continue;
                    case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                 }


                 /* begin of GV fix */
                 /* this fix is against a crash I found when (v->start_in_tokens+p->current_origin == p->buffer_size)
                    and v->start_in_tokens==v->end_in_tokens
                    here we known that v->start_in_tokens <= v->end_in_tokens
                    */

              } else if (v->end_in_tokens+p->current_origin > p->buffer_size) {
                 if (p->variable_error_policy != EXIT_ON_VARIABLE_ERRORS) {
                   error("Output warning: end variable position after end of text for variable $%S$\n",name);
                   /*error("start=%d  end=%d   origin=%d   buffer size=%d\n",v->start_in_tokens,v->end_in_tokens,p->current_origin,p->buffer_size);
                   for (int i=p->current_origin;i<p->buffer_size;i++) {
                       error("%S",p->tokens->value[p->buffer[i]]);
                   }
                   error("\n");*/
                 }
                 switch (p->variable_error_policy) {
                    case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end variable position after end of text for variable $%S$\n",name); break;
                    case IGNORE_VARIABLE_ERRORS: continue;
                    case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                 }

                 /* end of GV fix */


              } else {
                  /* If the normal variable definition is correct */
                  /* Case 1: start and end in the same token*/
                  if (v->start_in_tokens==v->end_in_tokens-1) {
                      unichar* tok=p->tokens->value[p->buffer[v->start_in_tokens+p->current_origin]];
                      int last=(v->end_in_chars!=-1) ? (v->end_in_chars) : (((int)u_strlen(tok))-1);
                      for (int k=v->start_in_chars;k<=last;k++) {
                          push_input_char(r->stack_template,u_toupper(tok[k]),p->protect_dic_chars);
                      }
                  } else if (v->start_in_tokens==v->end_in_tokens) {
                      /* If the variable is empty, do nothing */
                  } else {
                      /* Case 2: first we deal with first token */
                      unichar* tok=p->tokens->value[p->buffer[v->start_in_tokens+p->current_origin]];
                      unichar* tmp = u_strdup(tok+v->start_in_chars);
                      u_toupper(tmp);
                      push_input_string(r->stack_template,tmp,p->protect_dic_chars);
                      free(tmp);
                      /* Then we copy all tokens until the last one */
                      for (int k=v->start_in_tokens+1;k<v->end_in_tokens-1;k++) {
                          unichar *tmp2 = u_strdup(p->tokens->value[p->buffer[k+p->current_origin]]);
                          u_toupper(tmp2);
                          push_input_string(r->stack_template,tmp2,p->protect_dic_chars);
                          free(tmp2);
                      }

                      /* Finally, we copy the last token */

                      if ((v->end_in_tokens-1+p->current_origin) < 0) {
                          error("v->end_in_tokens-1+p->current_origin is below 0\n");
                          error("start=%d  end=%d\n",v->start_in_tokens,v->end_in_tokens);
                      }
                      else {
                          tok=p->tokens->value[p->buffer[v->end_in_tokens-1+p->current_origin]];
                          int last=(v->end_in_chars!=-1) ? (v->end_in_chars) : (((int)u_strlen(tok))-1);
                          for (int k=0;k<=last;k++) {
                            push_input_char(r->stack_template,u_toupper(tok[k]),p->protect_dic_chars);
                        }
                      }
                  }
              }
         }
         else if (!u_strcmp(field,"TO_FIRSTUPPER")) {
             /* Case of a variable like $a$ that can be either a normal one or an output one */
             struct transduction_variable* v=get_transduction_variable(p->input_variables,name);
             if (v==NULL) {
              /* Not a normal one ? Maybe an output one */
              const Ustring* output=get_output_variable(p->output_variables,name);
              if (output==NULL) {
                  switch (p->variable_error_policy) {
                      case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: undefined variable $%S$\n",name); break;
                      case IGNORE_VARIABLE_ERRORS: continue;
                      case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                  }
              }
              unichar* tmp = u_strdup(output->str);
              push_output_char(r->stack_template,u_toupper(tmp[0]));
              u_tolower(tmp);
              push_output_string(r->stack_template,tmp+1);
              free(tmp);
              } else if (v->start_in_tokens==UNDEF_VAR_BOUND) {
                 switch (p->variable_error_policy) {
                    case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: starting position of variable $%S$ undefined\n",name); break;
                    case IGNORE_VARIABLE_ERRORS: continue;
                    case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                 }
              } else if (v->end_in_tokens==UNDEF_VAR_BOUND) {
                 switch (p->variable_error_policy) {
                    case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position of variable $%S$ undefined\n",name); break;
                    case IGNORE_VARIABLE_ERRORS: continue;
                    case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                 }
              } else if (v->start_in_tokens>v->end_in_tokens
                          || (v->start_in_tokens==v->end_in_tokens && v->end_in_chars==-1 && v->end_in_chars<v->start_in_chars)) {
                 switch (p->variable_error_policy) {
                    case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position before starting position for variable $%S$\n",name); break;
                    case IGNORE_VARIABLE_ERRORS: continue;
                    case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                 }


                 /* begin of GV fix */
                 /* this fix is against a crash I found when (v->start_in_tokens+p->current_origin == p->buffer_size)
                    and v->start_in_tokens==v->end_in_tokens
                    here we known that v->start_in_tokens <= v->end_in_tokens
                    */

              } else if (v->end_in_tokens+p->current_origin > p->buffer_size) {
                 if (p->variable_error_policy != EXIT_ON_VARIABLE_ERRORS) {
                   error("Output warning: end variable position after end of text for variable $%S$\n",name);
                   /*error("start=%d  end=%d   origin=%d   buffer size=%d\n",v->start_in_tokens,v->end_in_tokens,p->current_origin,p->buffer_size);
                   for (int i=p->current_origin;i<p->buffer_size;i++) {
                       error("%S",p->tokens->value[p->buffer[i]]);
                   }
                   error("\n");*/
                 }
                 switch (p->variable_error_policy) {
                    case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end variable position after end of text for variable $%S$\n",name); break;
                    case IGNORE_VARIABLE_ERRORS: continue;
                    case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                 }

                 /* end of GV fix */


              } else {
                  /* If the normal variable definition is correct */
                  /* Case 1: start and end in the same token*/
                  if (v->start_in_tokens==v->end_in_tokens-1) {
                      unichar* tok=p->tokens->value[p->buffer[v->start_in_tokens+p->current_origin]];
                      int last=(v->end_in_chars!=-1) ? (v->end_in_chars) : (((int)u_strlen(tok))-1);
                      push_input_char(r->stack_template,u_toupper(tok[v->start_in_chars]),p->protect_dic_chars);
                      for (int k=v->start_in_chars+1;k<=last;k++) {
                          push_input_char(r->stack_template,u_tolower(tok[k]),p->protect_dic_chars);
                      }
                  } else if (v->start_in_tokens==v->end_in_tokens) {
                      /* If the variable is empty, do nothing */
                  } else {
                      /* Case 2: first we deal with first token */
                      unichar* tok=p->tokens->value[p->buffer[v->start_in_tokens+p->current_origin]];
                      unichar* tmp = u_strdup(tok+v->start_in_chars);
                      push_input_char(r->stack_template,u_toupper(tmp[0]),p->protect_dic_chars);
                      u_tolower(tmp);
                      push_input_string(r->stack_template,tmp+1,p->protect_dic_chars);
                      free(tmp);
                      /* Then we copy all tokens until the last one */
                      for (int k=v->start_in_tokens+1;k<v->end_in_tokens-1;k++) {
                          unichar* tmp2 = u_strdup(p->tokens->value[p->buffer[k+p->current_origin]]);
                          u_tolower(tmp2);
                          push_input_string(r->stack_template,tmp2,p->protect_dic_chars);
                          free(tmp2);
                      }

                      /* Finally, we copy the last token */

                      if ((v->end_in_tokens-1+p->current_origin) < 0) {
                          error("v->end_in_tokens-1+p->current_origin is below 0\n");
                          error("start=%d  end=%d\n",v->start_in_tokens,v->end_in_tokens);
                      }
                      else {
                          tok=p->tokens->value[p->buffer[v->end_in_tokens-1+p->current_origin]];
                          int last=(v->end_in_chars!=-1) ? (v->end_in_chars) : (((int)u_strlen(tok))-1);
                          for (int k=0;k<=last;k++) {
                            push_input_char(r->stack_template,u_tolower(tok[k]),p->protect_dic_chars);
                        }
                      }
                  }
              }
         }




         else if (!u_strcmp(field,"SET") || !u_strcmp(field,"UNSET")) {
            /* If we have $a.SET$, we must go on only if the variable a is set, no
             * matter if it is a normal or a dictionary variable */
            struct transduction_variable* v=get_transduction_variable(p->input_variables,name);
            if (v==NULL) {
               /* We do nothing, since this normal variable may not exist */
            } else {
               if (v->start_in_tokens==UNDEF_VAR_BOUND || v->end_in_tokens==UNDEF_VAR_BOUND
                     || v->start_in_tokens>v->end_in_tokens
                     || (v->start_in_tokens==v->end_in_tokens && (v->end_in_chars==-1 || v->end_in_chars<v->start_in_chars))) {
                  /* If the variable is not defined properly */
                  if (field[0]=='S') {
                     /* $a.SET$ is false, we backtrack */
                     r->stack_template->top=old_stack_pointer; return 0;
                  } else {
                     /* $a.UNSET$ is true, we go on */
                     continue;
                  }
               } else {
                  /* If the variable is correctly defined */
                  if (field[0]=='S') {
                     /* $a.SET$ is true, we go on */
                     continue;
                  } else {
                     /* $a.UNSET$ is false, we backtrack */
                     r->stack_template->top=old_stack_pointer; return 0;
                  }
               }
            }
            /* If we arrive here, the variable was not a normal one, so we
             * try to match an output one */
            const Ustring* output=get_output_variable(p->output_variables,name);
            if (output==NULL) {
                /* We do nothing, since this output variable may not exist */
            } else {
                if (output->len==0) {
                  /* If the variable is empty */
                  if (field[0]=='S') {
                     /* $a.SET$ is false, we backtrack */
                     r->stack_template->top=old_stack_pointer; return 0;
                  } else {
                     /* $a.UNSET$ is true, we go on */
                     continue;
                  }
               } else {
                  /* If the variable is non empty */
                  if (field[0]=='S') {
                     /* $a.SET$ is true, we go on */
                     continue;
                  } else {
                     /* $a.UNSET$ is false, we backtrack */
                     r->stack_template->top=old_stack_pointer; return 0;
                  }
               }
            }

            /* If we arrive here, the variable was neither a normal one nor an output one,
             * so we try to match dictionary one */
            struct dela_entry* entry=get_dic_variable(name,p->dic_variables);
            if (entry==NULL) {
               /* If the variable is not defined properly */
               if (field[0]=='S') {
                  /* $a.SET$ is false, we backtrack */
                  r->stack_template->top=old_stack_pointer; return 0;
               } else {
                  /* $a.UNSET$ is true, we go on */
                  continue;
               }
            }
            /* If the dictionary variable is defined */
            if (field[0]=='S') {
               /* $a.SET$ is true, we go on */
               continue;
            } else {
               /* $a.UNSET$ is false, we backtrack */
               r->stack_template->top=old_stack_pointer; return 0;
            }
         }

         struct dela_entry* entry=get_dic_variable(name,p->dic_variables);
         if (entry==NULL) {
            switch (p->variable_error_policy) {
               case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: undefined morphological variable %S\n",name); break;
               case IGNORE_VARIABLE_ERRORS: continue;
               case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
            }
         }
         if (u_starts_with(field,"EQ=")) {
            /* We deal with restrictions on semantic codes, especially brewed for
             * Korean dictionary graphs */
            unichar* filter_code=field+3;
            int match=0;
            for (int i=0;!match && i<entry->n_semantic_codes;i++) {
               if (!u_strcmp(entry->semantic_codes[i],filter_code)) {
                  match=1;
               }
            }
            if (match) {
               /* If the filter match, we go on without outputing anything */
               continue;
            } else {
               /* Otherwise, we backtrack */
               r->stack_template->top=old_stack_pointer; return 0;
            }
         } else if (!u_strcmp(field,"INFLECTED")) {
            /* We use push_input_string because it can protect special chars */
            if (p->korean!=NULL) {
               /* If we work in Korean mode, we must convert text into Hanguls */
               unichar z[1024];
               convert_jamo_to_hangul(entry->inflected,z,p->korean);
               push_input_string(r->stack_template,z,p->protect_dic_chars);
            } else {
               push_input_string(r->stack_template,entry->inflected,p->protect_dic_chars);
            }
         } else if (!u_strcmp(field,"LEMMA")) {
             push_input_string(r->stack_template,entry->lemma,p->protect_dic_chars);
         } else if (!u_strcmp(field,"CODE")) {
            push_output_string(r->stack_template,entry->semantic_codes[0]);
            for (int i=1;i<entry->n_semantic_codes;i++) {
                push_output_char(r->stack_template,'+');
               push_output_string(r->stack_template,entry->semantic_codes[i]);
            }
            for (int i=0;i<entry->n_inflectional_codes;i++) {
               push_output_char(r->stack_template,':');
               push_output_string(r->stack_template,entry->inflectional_codes[i]);
            }
         } else if (!u_strcmp(field,"CODE.GRAM")) {
            push_output_string(r->stack_template,entry->semantic_codes[0]);
         } else if (!u_strcmp(field,"CODE.SEM")) {
            if (entry->n_semantic_codes>1) {
               push_output_string(r->stack_template,entry->semantic_codes[1]);
               for (int i=2;i<entry->n_semantic_codes;i++) {
                  push_output_char(r->stack_template,'+');
                  push_output_string(r->stack_template,entry->semantic_codes[i]);
               }
            }
         } else if (!u_strcmp(field,"CODE.FLEX")) {
             if (entry->n_inflectional_codes>0) {
                push_output_string(r->stack_template,entry->inflectional_codes[0]);
                for (int i=1;i<entry->n_inflectional_codes;i++) {
                   push_output_char(r->stack_template,':');
                   push_output_string(r->stack_template,entry->inflectional_codes[i]);
                }
             }
          } else if (u_starts_with(field,"CODE.ATTR=")) {
              unichar* attr_name=field+10;
              int attr_len=u_strlen(attr_name);
              int i;
              for (i=0;i<entry->n_semantic_codes;i++) {
                  if (u_starts_with(entry->semantic_codes[i],attr_name)) {
                      if (entry->semantic_codes[i][attr_len]!='='
                              || entry->semantic_codes[i][attr_len+1]=='\0') {
                          continue;
                      }
                      push_output_string(r->stack_template,entry->semantic_codes[i]+attr_len+1);
                      break;
                  }
              }
              if (i==entry->n_semantic_codes) {
                  /* If the attribute was not found, it's an error case */
                  switch (p->variable_error_policy) {
                     case EXIT_ON_VARIABLE_ERRORS: fatal_error("Attribute %S not found in a captured entry\n",attr_name); break;
                     case IGNORE_VARIABLE_ERRORS: continue;
                     case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
                  }
              }
           } else {
            switch (p->variable_error_policy) {
               case EXIT_ON_VARIABLE_ERRORS: fatal_error("Invalid morphological variable field $%S.%S$\n",name,field); break;
               case IGNORE_VARIABLE_ERRORS: continue;
               case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
            }
         }
         continue;
      }
      i1++;
      if (l==0) {
         /* Case of $$ in order to print a $ */
          push_output_char(r->stack_template,'$');
         continue;
      }
      /* Case of a variable like $a$ that can be either a normal one or an output one */
      struct transduction_variable* v=get_transduction_variable(p->input_variables,name);
      if (v==NULL) {
         /* Not a normal one ? Maybe an output one */
         const Ustring* output=get_output_variable(p->output_variables,name);
         // Starting v4.0 Fst2List is dealing with input/output variables. Output
         // variables are tracked by locate_parameters.output_variables but (I do not understand
         // the reason for this) Input variables are tracked with the function parameter
         // extra_variables, i.e. not using locate_parameters.input_variables as might be
         // expected. For now the following lines are a dirty hotfix.
         // FIXME() begins --------------------------------------------------------------
         if (extra_variables!=NULL && output==NULL) {
           output=get_output_variable(extra_variables,name);
         }
         // FIXME() ends    --------------------------------------------------------------
         if (output==NULL) {
             switch (p->variable_error_policy) {
                 case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: undefined variable $%S$\n",name); break;
                 case IGNORE_VARIABLE_ERRORS: continue;
                 case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
             }
         }
         push_output_string(r->stack_template,output->str);
      } else if (v->start_in_tokens==UNDEF_VAR_BOUND) {
         switch (p->variable_error_policy) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: starting position of variable $%S$ undefined\n",name); break;
            case IGNORE_VARIABLE_ERRORS: continue;
            case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
         }
      } else if (v->end_in_tokens==UNDEF_VAR_BOUND) {
         switch (p->variable_error_policy) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position of variable $%S$ undefined\n",name); break;
            case IGNORE_VARIABLE_ERRORS: continue;
            case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
         }
      } else if (v->start_in_tokens>v->end_in_tokens
                  || (v->start_in_tokens==v->end_in_tokens && v->end_in_chars==-1 && v->end_in_chars<v->start_in_chars)) {
         switch (p->variable_error_policy) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position before starting position for variable $%S$\n",name); break;
            case IGNORE_VARIABLE_ERRORS: continue;
            case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
         }


         /* begin of GV fix */
         /* this fix is against a crash I found when (v->start_in_tokens+p->current_origin == p->buffer_size)
            and v->start_in_tokens==v->end_in_tokens
            here we known that v->start_in_tokens <= v->end_in_tokens
            */

      } else if (v->end_in_tokens+p->current_origin > p->buffer_size) {
         if (p->variable_error_policy != EXIT_ON_VARIABLE_ERRORS) {
           error("Output warning: end variable position after end of text for variable $%S$\n",name);
           /*error("start=%d  end=%d   origin=%d   buffer size=%d\n",v->start_in_tokens,v->end_in_tokens,p->current_origin,p->buffer_size);
           for (int i=p->current_origin;i<p->buffer_size;i++) {
               error("%S",p->tokens->value[p->buffer[i]]);
           }
           error("\n");*/
         }
         switch (p->variable_error_policy) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end variable position after end of text for variable $%S$\n",name); break;
            case IGNORE_VARIABLE_ERRORS: continue;
            case BACKTRACK_ON_VARIABLE_ERRORS: r->stack_template->top=old_stack_pointer; return 0;
         }

         /* end of GV fix */


      } else {
          /* If the normal variable definition is correct */
          /* Case 1: start and end in the same token*/
          if (v->start_in_tokens==v->end_in_tokens-1) {
              unichar* tok=p->tokens->value[p->buffer[v->start_in_tokens+p->current_origin]];
              int last=(v->end_in_chars!=-1) ? (v->end_in_chars) : (((int)u_strlen(tok))-1);
              for (int k=v->start_in_chars;k<=last;k++) {
                  push_input_char(r->stack_template,tok[k],p->protect_dic_chars);
              }
          } else if (v->start_in_tokens==v->end_in_tokens) {
              /* If the variable is empty, do nothing */
          } else {
              /* Case 2: first we deal with first token */
              unichar* tok=p->tokens->value[p->buffer[v->start_in_tokens+p->current_origin]];
              push_input_string(r->stack_template,tok+v->start_in_chars,p->protect_dic_chars);
              /* Then we copy all tokens until the last one */
              for (int k=v->start_in_tokens+1;k<v->end_in_tokens-1;k++) {
                  push_input_string(r->stack_template,p->tokens->value[p->buffer[k+p->current_origin]],p->protect_dic_chars);
              }

              /* Finally, we copy the last token */

              if ((v->end_in_tokens-1+p->current_origin) < 0) {
                  error("v->end_in_tokens-1+p->current_origin is below 0\n");
                  error("start=%d  end=%d\n",v->start_in_tokens,v->end_in_tokens);
              }
              else {
                  tok=p->tokens->value[p->buffer[v->end_in_tokens-1+p->current_origin]];
                  int last=(v->end_in_chars!=-1) ? (v->end_in_chars) : (((int)u_strlen(tok))-1);
                  for (int k=0;k<=last;k++) {
                    push_input_char(r->stack_template,tok[k],p->protect_dic_chars);
                }
              }
          }
      }
   }
}
return 0;
}

void append_literal_output(struct stack_unichar* output,
                           struct locate_parameters* p,
                           int *captured_chars) {
  // if there is nothing to append then return
  if(!output) return;

  // check if there are pending variables
  int capture = capture_mode(p->output_variables);

  // on normal mode
  if (!capture) {
    // copy the passed output into the literal output stack
    push_array(p->literal_output, output->buffer, output->top);
  // on capture mode
  } else {
    // append the passed output to the pending output variables
    *captured_chars = add_raw_string_to_output_variables(p->output_variables,
                                                         output->buffer,
                                                         output->top);
  }
}

/**
 * This function deals with an extended output sequence,
 * regardless is formed only by terminal symbols or nor,
 * and regardless there are pending output variables or not.
 */
int deal_with_extended_output(unichar* output,
                              struct locate_parameters* p,
                              struct extended_output_render* r) {
  // check if there are pending variables
  int capture = capture_mode(p->output_variables);

  // In debug mode, an output to be captured must still be
  // added to the normal stack, to trace the explored grammar
  // path. But, in this case, we remove the actual output part,
  // since no output is really produced there
  if (capture && p->debug) {
      push_output_char(p->literal_output, DEBUG_INFO_OUTPUT_MARK);
      int i;
      for (i = 0; output[i] != DEBUG_INFO_COORD_MARK; ++i) {}
      push_output_string(p->literal_output, output + i);
  }

  // process the extended output
  if (!process_extended_output(output, p, capture && p->debug, r, NULL)) {
    return 0;
  }

  // there is no more chars to add to the output template,
  // hence we put a mark to indicate the end of the string
  if (!is_empty(r->stack_template)) {
    push(r->stack_template, '\0');
  }

  // prepare the output template to be rendered
  r->prepare();

  return 1;
}


} // namespace unitex
