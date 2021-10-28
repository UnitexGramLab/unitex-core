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
/**
 * @file     UnitexDicEntry.h
 * @brief    Implements the Unitex::UnitexDicEntry class, a wrapper around Unitex
 *           dela data types and functions
 *
 * @author   cristian.martinez@unitexgramlab.org (martinec)
 *
 * @note     Use cpplint.py tool to detect style errors:
 *           `curl -L http://goo.gl/O1I32H -o cpplint.py ; chmod +x cpplint.py ;
 *           ./cpplint.py UnitexDicEntry.h`
 *
 * @date     October 2021
 *
 * This file was contributed by Cogniteva SAS. For further information on this,
 * please contact unitex-at-cogniteva.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_UNITEXDICENTRY_H_                                      // NOLINT
#define UNITEX_UNITEXDICENTRY_H_                                      // NOLINT
/* ************************************************************************** */
// C system files                  (try to order the includes alphabetically)
#include <cstddef>                 // ptrdiff_t
#include <cassert>                 // assert
/* ************************************************************************** */
// C++ system files                (try to order the includes alphabetically)

/* ************************************************************************** */
// Other libraries' .h files       (try to order the includes alphabetically)

/* ************************************************************************** */
// Unitex's .h files               (try to order the includes alphabetically)
#include "Unicode.h"               // u_* functions                 // NOLINT
#include "Ustring.h"               // u_* functions                 // NOLINT
#include "UnitexString.h"          // u_* functions                 // NOLINT
#include "DELA.h"                  // DELA* functions               // NOLINT
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
#ifndef  HAS_UNITEX_NAMESPACE
# define HAS_UNITEX_NAMESPACE 1
#endif  // !defined(HAS_UNITEX_NAMESPACE)
/* ************************************************************************** */
#define UNITEX_DELA_ENTRY_IS_NULL       (data_ == NULL)
/* ************************************************************************** */
namespace {   // namespace ::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
struct dela_entry* u_decode_dic_line_entry(const unichar* line,
                      Abstract_allocator prv_alloc = STANDARD_ALLOCATOR) {
  int decode_error = 0;
  // tokenize the current line entry
  struct dela_entry* entry = tokenize_DELAF_line(line,  // entry to parse
                              1,              // comments are allowed at EOL
                              &decode_error,  // must not print error messages
                              prv_alloc);
  return entry;
}
/* ************************************************************************** */
}  // namespace unitex::{unnamed}
/* ************************************************************************** */
/**
 * @class    UnitexDicEntry
 *
 * @brief    A class wrapper (RAII) around Unitex DELA related data types and
 *           functions
 *
 * @details  This class is build upon the next data types :
 *
 * @code{.cpp}
 *           dela_entry            // struct
 *           UnitexString          // class
 * @endcode
 *
 * @defgroup UnitexDicEntry UnitexDicEntry
 * @ingroup  Unicode
 *
 * @see      dela_entry.h
 */
class UnitexDicEntry {
 public :
  // Typedefs and Enums
  /**
   * @brief  Alias of size_t, used to represent sizes and counts
   */
  typedef size_t  size_type;


  /**
   * @brief  Unitex dela entry elemental type
   */
  typedef struct dela_entry value_type;

  /**
   * @brief  Read/Write reference to a dictionary entry
   *
   * @see    value_type
   */
  typedef struct dela_entry& reference;

  /**
   * @brief  Read only reference to a dictionary entry
   */
  typedef const struct dela_entry& const_reference;

  /**
   * @brief  Read/Write Pointer to a dictionary entry buffer
   */
  typedef struct dela_entry* pointer_t;

  /**
   * @brief  Read only pointer to a dictionary entry buffer
   */
  typedef const pointer_t const_pointer_t;

  /**
   * @brief  Represent the result of any valid pointer subtraction operation
   */
  typedef std::ptrdiff_t difference_type_t;

  /**
   * @brief  Read/Write struct dela_entry pointer used as iterator
   *
   */
  typedef pointer_t iterator;

  /**
   * @brief  Read only struct dela_entry pointer used as iterator
   */
  typedef const_pointer_t const_iterator;

  // Constants (including static const data members)
  /// @addtogroup member-constants member constants
  /// @ingroup    UnitexDicEntry
  /// @{

  /**
   * @brief Max line size
   *
   * Maximum size of a DELA line
   */
  static const size_type  kMaxDicLineSize = DIC_LINE_SIZE;

  /**
   * @brief  Max word size
   *
   * Maximum size of a word (word form or lemma)
   */
  static const size_type  kMaxDicWordSize = DIC_WORD_SIZE;

  /**
   * @brief  Max semantic codes
   *
   * Maximum number of semantic codes (Hum,Conc,z1,...) per line
   */
  static const size_type  kMaxSemanticCodes = MAX_SEMANTIC_CODES;

  /**
   * @brief  Max inflectional codes
   *
   * Maximum number of inflectional codes (ms,Kf,W,...) per line
   *
   * @see DELA.h
   */
  static const size_type  kMaxInflectionalCodes = MAX_INFLECTIONAL_CODES;


  /**
   * @brief  Max filters
   *
   * Maximum number of filters
   */
  static const size_type  kMaxFilters = MAX_FILTERS;


  /**
   * @brief  Not in Dictionary
   *
   * Value returned when a word is not found in a dictionary
   */
  static const int kNotInDictionary = NOT_IN_DICTIONARY;
  /// @}

  // Constructors
  /**
   * @brief  Default constructor
   *
   * Allocates and initializes an empty entry
   */
  UnitexDicEntry() :
    data_(acquire()) {
  }

  /**
   * @brief  Copy constructor
   *
   * Constructs entry as copy of a entry
   *
   * @param  entry Source entry
   */
  UnitexDicEntry(const UnitexDicEntry& entry) :
    data_(acquire(entry.data_)) {
  }

  /**
   * @brief  Constructor from unitex dela_entry
   *
   * Allocates and initializes a entry from a dela_entry
   *
   * @param  entry A dela_entry
   */
  explicit UnitexDicEntry(const struct dela_entry* entry) :
        data_(acquire(entry)) {
  }

  /**
   * @brief  Constructor from unitex dela_entry
   *
   * Initializes an already allocate dela_entry
   *
   * @param  entry A dela_entry
   */
  explicit UnitexDicEntry(struct dela_entry* entry) :
      data_(attach(entry)) {
  }

  /**
   * @brief  Constructor from unichar string
   *
   * Initializes an entry by tokenizing the input string
   *
   * @param  string An entry
   */
  explicit UnitexDicEntry(const unichar* line) :
      data_(acquire(line)) {
  }

  // Destructor

  /**
   * @brief  Destroys the entry object
   *
   * Free the memory allocated to the internal entry
   */
  ~UnitexDicEntry() {
    // if data_ is acquired then release, detach otherwise
    release() || detach();
  }

  // Iterators

  // assignment operators =

  /**
   * @brief  Entry assignment from a C-string
   *
   * Assigns a new value to the entry, replacing its current contents
   *
   * @param  rhs    A null-terminated character sequence (C-string). The sequence
   *                is parsed as the new value for the entry.
   * @return *this
   */
//  UnitexDicEntry& operator=(const char* rhs) {
//    //unitex::u_strcpy(data_, rhs);
//    return *this;
//  }

  /**
   * @brief  Entry assignment from a unichar-string
   *
   * Assigns a new value to the entry, replacing its current contents
   *
   * @param  rhs    A Unitex unichar-string. The sequence is parsed as the new
   *                value for the entry.
   * @return *this
   */
//  UnitexDicEntry& operator=(const unichar* rhs) {
//    //if (data_->str != rhs) {
//       //unitex::u_strcpy(data_, rhs);
//    //}
//    return *this;
//  }

  /**
   * @brief  Entry assignment from a dela_entry
   *
   * Assigns a new value to the entry, replacing its current contents
   *
   * @param  rhs    A Unitex dela_entry. The passed entry is copied as the new
   *                value for the internal entry.
   * @return *this
   */
  UnitexDicEntry& operator=(const_pointer_t rhs) {
    //if (data_ != rhs) {
       //unitex::u_strcpy(data_, rhs);
    //}
    return *this;
  }

  /**
   * @brief  String assignment from a UnitexDicEntry object
   *
   * Assigns a new value to the entry, replacing its current contents
   *
   * @param  rhs    A UnitexDicEntry. The passed entry is copied as the new
   *                value for the internal entry.
   * @return *this
   */
  UnitexDicEntry& operator=(const UnitexDicEntry& rhs) {
    return operator=(rhs.data_);
  }

  // index operands

  // boolean equality operands

  /**
   * @brief  Test equivalence of C-string and UnitexDicEntry
   *
   * @param  rhs    A null-terminated character sequence (C-string)
   * @return True if this->compare(rhs) == 0.  False otherwise
   */
//  bool operator==(const char* rhs) const {
//    return this->compare(rhs) == 0;
//  }

  /**
   * @brief  Test equivalence of unichar-string and UnitexDicEntry
   *
   * @param  rhs    A Unitex unichar-string
   * @return True if this->compare(rhs) == 0.  False otherwise
   */
//  bool operator==(const unichar* rhs) const {
//    return this->compare(rhs) == 0;
//  }

  /**
   * @brief  Test equivalence of dela_entry and UnitexDicEntry
   *
   * @param  rhs    A Unitex dela_entry
   * @return True if this->compare(rhs) == 0.  False otherwise
   */
//  bool operator==(const_pointer_t rhs) const {
////  return u_equal(this->data_, rhs);
//    return 1;
//  }

  /**
   * @brief  Test equivalence between two UnitexDicEntry objects
   *
   * @param  rhs    A UnitexDicEntry object
   * @return True if both strings are equals.  False otherwise
   */
//  bool operator==(const UnitexDicEntry& rhs) const {
//// return u_equal(this->data_, rhs.data_);
//    return 1;
//  }

  // boolean not equal operands

  /**
   * @brief  Test difference of C-string and UnitexDicEntry
   *
   * @param  rhs    A null-terminated character sequence (C-string)
   * @return True if this->compare(rhs) != 0.  False otherwise
   */
//  bool operator!=(const char* rhs) const {
//    return !(*this == rhs);
//  }

  /**
   * @brief  Test difference of unichar-string and UnitexDicEntry
   *
   * @param  rhs    A Unitex unichar-string
   * @return True if this->compare(rhs) != 0.  False otherwise
   */
//  bool operator!=(const unichar* rhs) const {
//    return !(*this == rhs);
//  }

  /**
   * @brief  Test difference of dela_entry and UnitexDicEntry
   *
   * @param  rhs    A Unitex dela_entry
   * @return True if this->compare(rhs) != 0.  False otherwise
   */
//  bool operator!=(const_pointer_t rhs) const {
//    return !(*this == rhs);
//  }

  /**
   * @brief  Test difference between two UnitexDicEntry objects
   *
   * @param  rhs    A UnitexDicEntry object
   * @return True if this->compare(rhs) != 0.  False otherwise
   */
//  bool operator!=(const UnitexDicEntry& rhs) const {
//    return !(*this == rhs);
//  }

  // Static methods

  // Methods

  // element access

  /**
   * @brief  Get the inflected form
   *
   * @return The inflected form of the dictionary entry
   */
  const unichar* inflected() const {
    assert(!this->is_empty());
    return this->data_->inflected;
  }

  /**
   * @brief  Get the lemma
   *
   * @return The lemma of the dictionary entry
   */
  const unichar* lemma() const {
    assert(!this->is_empty());
    return this->data_->lemma;
  }


   /**
   * @brief  Get the attribute value at key
   *
   * @return The value of the semantic code of the form key=value
   */
  int code_attr(const unichar* key, UnitexString* value) const {
    assert(!this->is_empty());
    size_t keylen = u_strlen(key);
    const struct dela_entry* entry = this->data_;

    for (int j = 0; j < entry->n_semantic_codes; ++j) {
      if (u_starts_with(entry->semantic_codes[j], key) &&
          entry->semantic_codes[j][keylen] ==
          DIC_SEMANTIC_CODE_KEY_VALUE_CHAR_SEP){
          value->append(entry->semantic_codes[j] + keylen + 1);
          return 1;
      }
    }

    return 0;
  }

  /**
   * @brief  Return length of the dictionary entry
   *
   * Returns the length of the entry, in terms of number of characters
   */
  size_type len() const {
// return static_cast<size_type>(data_->len);
    return 0;
  }

  /**
   * @brief  Return length of string
   *
   * Returns the length of the entry, in terms of number of characters
   */
  size_type size() const {
    return len();
  }

  /**
   * @brief  Return length of string
   *
   * Returns the length of the string, in terms of bytes.
   */
  size_type bytes() const {
    return (len() * sizeof(unichar));
  }

  // string append functions

  /**
   * @brief  Compare to a C-string
   *
   * @param  string A null-terminated character sequence (C-string)
   * @return Returns a signed integral indicating the relation between
   *         the strings:
   *         -  0 : They compare equal
   *         - <0 : Either the value of the first character that does not match
   *                is lower in the compared string, or all compared characters
   *                match but the compared string is shorter
   *         - >0 : Either the value of the first character that does not match
   *                is greater in the compared string, or all compared characters
   *                match but the compared string is longer.
   *
   * @note   Null strings are allowed
   */
  int compare(const char* string) const {
    // return unitex::u_strcmp(data_->str, string);
    return 1;
  }

  /**
   * @brief  Compare to a unichar-string
   *
   * @param  string A Unitex unichar-string
   * @return Returns a signed integral indicating the relation between
   *         the strings.
   *
   * @see    UnitexDicEntry::compare(const char* string) const
   */
  int compare(const unichar* string) const {
    //return unitex::u_strcmp(data_->str, string);
    return 1;
  }

  /**
   * @brief  Compare to a dela_entry
   *
   * @param  string A Unitex dela_entry
   * @return Returns a signed integral indicating the relation between
   *         the strings.
   *
   * @see    UnitexDicEntry::compare(const char* string) const
   */
  int compare(const struct dela_entry* string) const {
//    return this->compare(string->str);
    return 1;
  }

  /**
   * @brief  Compare to a UnitexDicEntry object
   *
   * @param  string A UnitexDicEntry object
   * @return Returns a signed integral indicating the relation between
   *         the strings.
   *
   * @see    UnitexDicEntry::compare(const char* string) const
   */
  int compare(const UnitexDicEntry& string) const {
//    return this->compare(string.data_->str);
    return 1;
  }

  /**
   * @brief  Case insensitive compare to a C-string
   *
   * @param  string A null-terminated character sequence (C-string)
   * @return Returns a signed integral indicating the relation between
   *         the strings:
   *         -  0 : They compare equal
   *         - <0 : Either the value of the first character that does not match
   *                is lower in the compared string, or all compared characters
   *                match but the compared string is shorter
   *         - >0 : Either the value of the first character that does not match
   *                is greater in the compared string, or all compared characters
   *                match but the compared string is longer.
   *
   * @note   Null strings are allowed
   */
//  int icompare(const char* string) const {
////    return unitex::u_strcmp_ignore_case(data_->str, string);
//    return 1;
//  }

  /**
   * @brief  Case insensitive compare to a unichar-string
   *
   * @param  string A Unitex unichar-string
   * @return Returns a signed integral indicating the relation between
   *         the strings.
   *
   * @see    UnitexDicEntry::icompare(const char* string) const
   */
//  int icompare(const unichar* string) const {
////    return unitex::u_strcmp_ignore_case(data_->str, string);
//    return 1;
//  }

  /**
   * @brief  Case insensitive compare to a dela_entry
   *
   * @param  string A Unitex dela_entry
   * @return Returns a signed integral indicating the relation between
   *         the strings.
   *
   * @see    UnitexDicEntry::icompare(const char* string) const
   */
//  int icompare(const struct dela_entry* string) const {
////    return this->icompare(string->str);
//    return 1;
//  }

  /**
   * @brief  Case insensitive compare to a UnitexDicEntry object
   *
   * @param  string A UnitexDicEntry object
   * @return Returns a signed integral indicating the relation between
   *         the strings.
   *
   * @see    UnitexDicEntry::icompare(const char* string) const
   */
//  int icompare(const UnitexDicEntry& string) const {
////    return this->compare(string.data_->str);
//    return 1;
//  }

  /**
   * @brief  Get dela_entry equivalent
   *
   * @return A pointer to the underlying built-in unichar
   *
   * @attention The caller should not delete the return value
   */
  const struct dela_entry* c_dela_entry() const {
    // this->push_back(0);
    // this->pop_back();
    return data();
  }

  /**
   * @brief  Conversion from UnitexDicEntry to the underlying dela_entry
   *         buffer
   *
   * @see  c_dela_entry() const
   */
  UNITEX_EXPLICIT_CONVERSIONS
  operator const struct dela_entry*() const {
    return data();
  }

  /**
   * @brief  Conversion from UnitexDicEntry to a decoded UnitexString
   */
  UNITEX_EXPLICIT_CONVERSIONS
  operator const UnitexString*() const {
    return new UnitexString("Test");
    //return data();
  }

  /**
   * @brief  Get string data
   *
   * @return A pointer to the underlying built-in dela_entry
   *
   * @attention The caller should not delete the return value
   */
  const struct dela_entry* data() const {
    if (UNITEX_DELA_ENTRY_IS_NULL) {
      return (const struct dela_entry*) NULL;
    }
    return data_;
  }

  /**
   * @brief  Test whether the string is NULL
   *
   * @return True if the string is NULL. False otherwise
   *
   * @note   NULL and the empty string are different
   */
  bool is_null() const {
    return UNITEX_DELA_ENTRY_IS_NULL;
  }

  /**
   * @brief  Test whether the string is empty
   *
   * @return True if the string is empty. False otherwise
   *
   * Returns whether the string is empty (i.e. whether its length is 0).
   *
   * @note   This differs from unitex::empty() function in dela_entry.h. Note also
   *         that NULL and the empty string are different.
   *
   * @see    is_null() const
   */
  bool is_empty() const {
    return (!UNITEX_DELA_ENTRY_IS_NULL        &&
         data_->inflected             == NULL &&
         data_->lemma                 == NULL &&
         data_->semantic_codes[0]     == NULL &&
         data_->inflectional_codes[0] == NULL &&
         data_->filter_codes[0]       == NULL &&
         data_->n_semantic_codes      == 0    &&
         data_->n_inflectional_codes  == 0    &&
         data_->n_filter_codes        == 0);
  }

  /**
   * @brief  Test whether the underline data_ is attached
   *
   * @return True if the data_ allocation is managed outside. False otherwise
   *
   * @see    attach()
   */
  bool is_attached() const {
    return engaged_ == 0;
  }

  /**
   * @brief  Test whether the underline data_ is acquired
   *
   * @return True if the data_ allocation is done by this class. False otherwise
   *
   * @see    attach()
   */
  bool is_acquired() const {
     return engaged_ == 1;
   }

  /**
   * @brief  Swap string values
   *
   * Exchanges the content of the container by the content of string,
   * which is another UnitexDicEntry object.
   *
   * @param  string Another UnitexDicEntry object, whose value is swapped with
   *                that of this string
   */
  void swap(UnitexDicEntry& entry) {
    if (this != &entry) {
       this->swap(engaged_,                    entry.engaged_);
       this->swap(data_->inflected,            entry.data_->inflected);
       this->swap(data_->lemma,                entry.data_->lemma);
       this->swap(data_->n_semantic_codes,     entry.data_->n_semantic_codes);
       this->swap(data_->n_inflectional_codes, entry.data_->n_inflectional_codes);
       this->swap(data_->n_filter_codes,       entry.data_->n_filter_codes);
       this->swap(data_->filter_polarity,      entry.data_->filter_polarity);
//       this->swap(data_->semantic_codes,       entry.data_->semantic_codes);
//       this->swap(data_->inflectional_codes,   entry.data_->inflectional_codes);
//       this->swap(data_->filter_codes,         entry.data_->filter_codes);
    }
  }

  // friends in an unnamed namespace
  // nothing

  // Data Members (except static const data members)
  // nothing

 private :
  // Typedefs and Enums
  // nothing

  // Constants (including static const data members)
  // nothing

  // Constructors
  // nothing

  // Destructor
  // nothing

  // Methods, including static

  /**
   * @brief  Attach an dela_entry
   * @note   To be used only from a constructor
   * @see    data_
   */
  struct dela_entry* attach(struct dela_entry* entry) {
    engaged_ = 0;
    return entry;
  }

  /**
   * @brief  Detach data_ from an attached dela_entry
   * @note   To be used only from a destructor
   * @see    data_
   */
  int detach() {
    if (is_attached()) {
      engaged_ = 2;
      data_ = NULL;
      return 1;
    }
    return 0;
  }

  /**
   * @brief  Allocate an empty dela_entry
   * @see    data_
   */
  struct dela_entry* acquire() {
    engaged_ = 1;
    return new_dela_entry();
  }

  /**
   * @brief  Allocate an dela_entry representing the given string
   * @see    data_
   */
  struct dela_entry* acquire(const struct dela_entry* entry) {
    engaged_ = 1;
    return clone_dela_entry(entry);
  }

  /**
   * @brief  Allocate an dela_entry representing the given string
   * @see    data_
   */
  struct dela_entry* acquire(const unichar* line) {
    engaged_ = 1;
    return u_decode_dic_line_entry(line);
  }

  /**
   * @brief  Allocate an dela_entry representing the given string
   * @see    data_
   */
  struct dela_entry* acquire(const unichar* inflected,const unichar* lemma,const unichar* code) {
    engaged_ = 1;
    return new_dela_entry(inflected, lemma, code);
  }

  /**
   * @brief  Free the memory allocated to the internal dela_entry
   * @see    data_
   */
  int release() {
    if (is_acquired()) {
      engaged_ = 0;
      free_dela_entry(data_);
      return 1;
    }
    return 0;
  }

  /**
   * @brief  Swap two values T
   *
   * Exchanges the values of a and b
   *
   * @param  a,b Two objects, whose contents are swapped
   */
  template<class T>
  void swap(T& a, T& b) {
    T c(a);
    a = b;
    b = c;
  }

  // Data Members (except static const data members)

  /**
   * @brief  whether the underline data_is attached, acquired or detached
   *
   * attached (0)
   * acquired (1)
   * detached (2)
   */
  int8_t engaged_;

  /**
   * @brief  underline dela_entry container
   */
  struct dela_entry* data_;
};  // class UnitexDicEntry
/* ************************************************************************** */
// [BEGIN class UnitexDicEntry non-member overloads
/// @addtogroup non-member-overloads non-member overloads
/// @ingroup    UnitexDicEntry
/// @{

/**
 * @brief    Exchanges the values of two UnitexDicEntry objects
 *
 * @param    a,b Two UnitexDicEntry objects, whose contents are swapped
 */
inline void swap(UnitexDicEntry& a, UnitexDicEntry& b) {            // NOLINT
  a.swap(b);
}
/// @}
// END] class UnitexDicEntry non-member overloads
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_UNITEXDICENTRY_H_                                   // NOLINT
