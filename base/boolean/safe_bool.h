/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
/**
 * @file      safe_bool.h
 * @brief     Safe bool idiom
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 safe_bool.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_BOOLEAN_SAFE_BOOL_H_                            // NOLINT
#define UNITEX_BASE_BOOLEAN_SAFE_BOOL_H_                            // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/compiler/compiler.h"
/* ************************************************************************** */
// C system files                  (try to order the includes alphabetically)
#include <cstring>                 // strcmp
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
struct Bool {
  // Typedefs and Enums
  /**
  * Wrapper enum for
  */
  struct constant {
    enum state {
      invalid_ = -1,  // invalid state
      False,          // negative
      True,           // positive
    };
  };

  // Constants (static const data members)
  /**
   * Default state
   */
  // Initialize the default state level, this value is used in Bool
  // construction
  static const constant::state kDefaultState = constant::False;

  /**
   * Bool states
   */
  static const Bool False;
  static const Bool True;


  // Constructors
  /**
   * Constructor without parameters
   * unitex::state_ is False by default
   */
  Bool() : state_(kDefaultState) {
  }

  /**
   * Constructor from integer
   */
  explicit Bool(int state_number) {
    *this = state_number;
  }

  /**
   * Constructor from string
   */
  explicit Bool(const char* state_name) {
    *this = state_name;
  }

  /**
   * Constructor from bool
   */
  explicit Bool(bool state) {
    *this = state;
  }

  /**
   * Contructor from Bool
   * Note that this struct disallow implicit copy constructor
   * use "const Bool& state" to pass as parameter
   */
  explicit Bool(const Bool& state) {
    *this = state;
  }

  // Operator overloads

  /**
   * Assignement operator from Bool
   */
  Bool& operator =(const Bool& rhs) {
    if (this != &rhs) {
      state_ = rhs.state_;
    }
    return *this;
  }

  /**
   * Assignement operator from integer
   * Valid input : {0, 1}
   */
  Bool& operator =(int state_number) {
    state_ = (state_number != 0 || state_number != 1 ) ?
              constant::invalid_ :
              static_cast<constant::state>(state_number);

    return *this;
  }

  /**
   * Assignement operator from string
   * Valid input : {"true", "yes", "false", "no"}
   */
  Bool& operator =(const char* state_name) {
    state_ = string_to_state(state_name);

    return *this;
  }

  /**
   * Assignement operator from boolean
   * Valid input : {true, false}
   */
  Bool& operator =(bool state) {
    state_ =  static_cast<constant::state>(state);

    return *this;
  }

  // Overload some operators, note that this explicit allows comparisons
  // between different instances of this class

  /**
   * Equality operator over Bool
   */
  bool operator ==(const Bool& rhs) const {
    return state_ == rhs.state_;
  }

  /**
   * Equality operator over boolean
   */
  bool operator ==(bool state) const {
    return state_ == static_cast<constant::state>(state);
  }

  /**
   * Inequality operator over Bool
   */
  bool operator !=(const Bool& rhs) const {
    return !(*this == rhs);
  }

  /**
   * Inequality operator over boolean
   */
  bool operator !=(bool state) const {
    return !(*this == state);
  }

#if UNITEX_COMPILER_COMPLIANT(CXX11)
  /**
   * C++11 support explicit type conversion operators
   * This is a ready to use version of the Safe Bool Idiom
   */
  explicit operator bool() const  {
    return (state_ == constant::True);
  }
#else
  /**
   * The next block is part of "The Safe Bool Idiom"
   * note that we allows comparisons between different
   * instances of this class. For more details please visit
   * @see http://www.artima.com/cppsource/safeboolP.html
   */
  private :
    typedef void (Bool::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}

  public :
    /**
     * Returns true if state_ is constant::True, false otherwise
     */
    operator bool_type() const {
      return  (state_ == constant::True) ?
        &Bool::this_type_does_not_support_comparisons : 0;
    }
#endif

    // Methods, including static
    /**
     * Return a constant char pointer with the stringify name of the current
     * level
     */
    const char* name() const {
      return names(*this);
    }

    /**
     * Return a constant char pointer with the stringify alias of the current
     * state
     */
    const char* alias() const {
      return aliases(*this);
    }

    /**
     * Return an integer with the number of the current state
     */
    int number() const {
      return static_cast<int>(state_);
    }



    /**
     * Test if Bool isn't invalid
     * @return false if state diffent to True or False
     */
    bool is_valid() const {
      return state_ != constant::invalid_;
    }

    // Data Members

  private:
  // Typedefs and Enums

    /**
     * Name string encapsulation for states
     */
#define Declare__BoolParam__(Instance)       \
    struct Instance  {                       \
      static const char* name;               \
      static const char* alias;              \
      static const constant::state value =   \
                   constant::Instance;       \
    }

    Declare__BoolParam__(False);
    Declare__BoolParam__(True);

#undef Declare__BoolParam__ /* undef */

  // Constants (static const data members)

  // Constructors

  /**
   * Explicit constructor from constant::state
   */
  explicit Bool(constant::state state)
          : state_(state) {}

  // Methods, including static

  /**
   * Return a constant char pointer with the stringify name of the current
   * state
   */
  static const char* names(const Bool& state) {
    switch (state.state_) {
      case constant::False:    return False::name;
      case constant::True:     return True::name;
      default:                 return ((const char *) 0);
    }
  }

  /**
   * Return a constant char pointer with the stringify alias of the current
   * state
   */
  static const char* aliases(const Bool& state) {
    switch (state.state_) {
      case constant::False:    return False::alias;
      case constant::True:     return True::alias;
      default:                 return ((const char *) 0);
    }
  }

  /**
   * Return a constant::state from the stringify name (or alias)
   * passed as argument
   */
  static constant::state string_to_state(const char* state_name) {
    //  "Premature optimization is the root of all evil
    //  (or at least most of it) in programming" --Donald Knuth
    if (state_name) {
      if (strcmp(state_name, False::name)  == 0) return constant::False;
      if (strcmp(state_name, False::alias) == 0) return constant::False;
      if (strcmp(state_name, True::name)   == 0) return constant::True;
      if (strcmp(state_name, True::alias)  == 0) return constant::True;
    }
    return constant::invalid_;
  }

  // Data Members
  /**
   *
   */
  constant::state state_;
};
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_BOOLEAN_SAFE_BOOL_H_                         // NOLINT
