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
/**
 * @file     UnitexString.h
 * @brief    Implements the Unitex::UnitexString class, a wrapper around Unitex
 *           unicode strings data types and functions
 *
 * @author   cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @note     Use cpplint.py tool to detect style errors:
 *           `curl -L http://goo.gl/O1I32H -o cpplint.py ; chmod +x cpplint.py ;
 *           ./cpplint.py UnitexString.h`
 *
 * @date     November 2014
 *
 * This file was contributed as part of the [DataMaTex](http://unitex.amabis.fr)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_UNITEXSTRING_H_                                      // NOLINT
#define UNITEX_UNITEXSTRING_H_                                      // NOLINT
/* ************************************************************************** */
// C system files                  (try to order the includes alphabetically)
#include <cstddef>                 // ptrdiff_t
#include <cwchar>                  // wcslen
#include <cassert>                 // assert
/* ************************************************************************** */
// C++ system files                (try to order the includes alphabetically)

/* ************************************************************************** */
// Other libraries' .h files       (try to order the includes alphabetically)

/* ************************************************************************** */
// Unitex's .h files               (try to order the includes alphabetically)
#include "Unicode.h"               // u_* functions                 // NOLINT
#include "Ustring.h"               // u_* functions                 // NOLINT
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
#ifndef  HAS_UNITEX_NAMESPACE
# define HAS_UNITEX_NAMESPACE 1
#endif  // !defined(HAS_UNITEX_NAMESPACE)
/* ************************************************************************** */
#define UNITEX_STRING_IS_NULL       (data_->str == NULL)
/* ************************************************************************** */
/**
 * @class    UnitexString
 *
 * @brief    A class wrapper (RAII) around Unitex unicode strings data types and
 *           functions
 *
 * @details  This class is build upon the next data types :
 *
 * @code{.cpp}
 *           const char            // character
 *           const char*           // string
 *           const unichar         // character
 *           const unichar*        // string
 *           const Ustring*        // string
 *           const UnitexString&   // string
 * @endcode
 *
 * @defgroup UnitexString UnitexString
 * @ingroup  Unicode
 *
 * @see      Ustring.h
 * @see      Unicode.h
 */
class UnitexString {
 public :
  // Typedefs and Enums
  /**
   * @brief  Alias of size_t, used to represent sizes and counts
   */
  typedef size_t  size_type;

  /**
   * @brief  Unitex unicode elemental type
   *
   * unichar is the type of a unicode character. Note that it is a 16-bits type,
   * so that it cannot handle characters >= 0xFFFF. Such characters, theoretically
   * represented with low and high surrogate characters are not handled by Unitex
   */
  typedef unichar value_type;

  /**
   * @brief  Read/Write reference to a unichar character
   *
   * @see    value_type
   */
  typedef unichar& reference;

  /**
   * @brief  Read only reference to a unichar character
   */
  typedef const unichar& const_reference;

  /**
   * @brief  Read/Write Pointer to a unichar buffer
   */
  typedef unichar* pointer_t;

  /**
   * @brief  Read only pointer to a unichar buffer
   */
  typedef const unichar* const_pointer_t;

  /**
   * @brief  Represent the result of any valid pointer subtraction operation
   */
  typedef std::ptrdiff_t difference_type_t;

  /**
   * @brief  Read/Write unichar pointer used as iterator
   *
   */
  typedef unichar* iterator;

  /**
   * @brief  Read only unichar pointer used as iterator
   */
  typedef const unichar* const_iterator;

  // Constants (including static const data members)
  /// @addtogroup member-constants member constants
  /// @ingroup    UnitexString
  /// @{

  /**
   * @brief  Returned by functions when they fail, literally indicates past
   *         the end
   */
  static const size_type  npos = static_cast<size_type>(-1);

  /**
   * @brief  Max buffer size
   *
   * Max buffer size, expressed in number of characters, used in
   * UnitexString::format and UnitexString::append_format functions
   *
   * @see    format
   * @see    append_format
   */
  static const size_type  kMaxBufferSize = 1024;

  /// @}

  // Constructors
  /**
   * @brief  Default constructor
   *
   * Allocates and initializes an empty string, with a length of zero
   * characters
   */
  UnitexString() :
      data_(acquire()) {
  }

  /**
   * @brief  Copy constructor
   *
   * Constructs string as copy of a string
   *
   * @param  string Source string
   */
  UnitexString(const UnitexString& string) :
    data_(acquire(string.data_->str)) {
  }

  /**
   * @brief  Capacity constructor
   *
   * Requests that the string capacity be adapted to a planned size
   * to a length of *up to* n characters.
   *
   * @param  n      Planned length for the string, expressed in number of
   *                characters
   *
   * @note          The resulting string capacity may be equal or greater
   *                than @e n.
   */
  UNITEX_EXPLICIT_CONVERSIONS
  UnitexString(size_type n) :
    data_(acquire(n)) {
  }

  /**
   * @brief  Substring constructor
   *
   * Copies the portion of str that begins at the character position pos and
   * spans n characters, or until the end of str, if either str is too short
   * or if n is UnitexString::npos
   *
   * @param  string Source string
   * @param  pos    Index of first character to copy from
   * @param  n      Number of characters to copy (default remainder)
   */
  UnitexString(const UnitexString& string, size_t pos, size_type n = npos) {
    if (pos < string.len()) {
      if (n > (string.len() - pos)) {
        n = string.len() - pos;
      }
      data_ = acquire(n);
      this->append(string.data() + pos, n);
    } else {
      data_ = acquire();
    }
  }

  /**
   * @brief  Constructor from c-string
   *
   * Allocates and initializes a string from a null-terminated character
   * sequence (C-string)
   *
   * @param  string A null-terminated character sequence (C-string)
   */
  UNITEX_EXPLICIT_CONVERSIONS
  UnitexString(const char* string) :                   // NOLINT
      data_(acquire()) {
    this->append(string);
  }

  /**
   * @brief  Constructor from c-buffer
   *
   * Copies  the first n chars from the array of characters pointed by string
   *
   * @param  string A null-terminated character sequence (C-string)
   * @param  n      Number of characters to copy
   */
  UnitexString(const char* string, size_type n) :
        data_(acquire(n)) {
    this->append(string, n);
  }

  /**
   * @brief  Fill constructor from char
   *
   * Fills the string with n consecutive copies of character c
   *
   * @param  n         Number of characters to fill
   * @param  character Character to fill the string with. Each of the n
   *                   characters in the string will be initialized to a
   *                   copy of this value.
   */
  UnitexString(size_type n, char character) :
      data_(acquire(n)) {
    // sets the first num bytes of the block of memory pointed by
    // data_->str to the specified value (interpreted as a char)
    // we avoid to use memset(data_->str, character, n);
    for (unsigned int i = 0; i < n; i++) {
      data_->str[data_->len + i] = character;
    }
    data_->len = n;
    data_->str[data_->len] = '\0';
  }

  /**
   * @brief  Range constructor from C-string iterators
   *
   * Copies the sequence of characters in the range [first,last), in the
   * same order, including the character pointed by first but not the
   * character pointed by last.
   *
   * @param  first  Input iterator (C-string) to the initial position in a range
   * @param  last   Input iterator (C-string) to the final position in a range
   */
  UnitexString(const char* first, const char* last) {
    if (first < last) {
      const size_t distance = last - first;
      data_ = acquire(distance);
      this->append(first, distance);
    } else {
      data_ = acquire();
    }
  }

  /**
   * @brief  Constructor from unitex unichar
   *
   * Allocates and initializes a string from a null-terminated character
   * sequence (unichar-string)
   *
   * @param  string A null-terminated character sequence (unichar-string)
   */
  explicit UnitexString(const unichar* string) :
      data_(acquire(string)) {
  }

  /**
   * @brief  Constructor from unichar buffer
   *
   * Copies the first n chars from the array of characters pointed by string
   *
   * @param  string A null-terminated character sequence (unichar-string)
   * @param  n      Number of characters to copy
   */
  UnitexString(const unichar* string, size_type n) :
        data_(acquire(n)) {
    this->append(string, n);
  }

  /**
   * @brief  Fill constructor from unichar
   *
   * Fills the string with n consecutive copies of character c (unichar)
   *
   * @param  n         Number of characters to fill
   * @param  character Character to fill the string with. Each of the n
   *                   characters in the string will be initialized to a
   *                   copy of this value.
   */
  UnitexString(size_type n, unichar character) :
      data_(acquire(n)) {
    // sets the first num bytes of the block of memory pointed by
    // data_->str to the specified value (interpreted as a char)
    // we avoid to use memset(data_->str, character, n);
    for (unsigned int i = 0; i < n; ++i) {
      data_->str[data_->len + i] = character;
    }
    data_->len = n;
    data_->str[data_->len] = '\0';
  }

  /**
   * @brief  Fill constructor from an UnitexString
   *
   * Fills the string with n consecutive copies of a string
   *
   * @param  n         Number of times to fill
   * @param  string    String to fill the string with.
   */
  UnitexString(size_type n, const UnitexString& string) :
      data_(acquire(n * string.len())) {
    // sets n consecutive copies of the string
    for (unsigned int i = 0; i < n; ++i) {
      unitex::u_strcat(data_,  string.data_->str, string.data_->len);
    }
  }

  /**
   * @brief  Range constructor from unichar-string iterators
   *
   * Copies the sequence of characters in the range [first,last), in the
   * same order, including the character pointed by first but not the
   * character pointed by last.
   *
   * @param  first  Input iterator (unichar) to the initial position in a range
   * @param  last   Input iterator (unichar) to the final position in a range
   */
  UnitexString(const unichar* first, const unichar* last) {
    if (first < last) {
      const size_t distance = last - first;
      data_ = acquire(distance);
      this->append(first, distance);
    } else {
      data_ = acquire();
    }
  }

  /**
   * @brief  Constructor from a encoded c-string
   *
   * Allocates and initializes a string from a null-terminated and encoded
   * character sequence (C-string)
   *
   * @param  string A null-terminated character sequence (C-string)
   */
  UnitexString(Encoding encoding, const char* string,
               size_type buffer_size = kMaxBufferSize) :  // NOLINT
      data_(acquire(buffer_size)) {
    size_type length = 0;
    switch (encoding) {
      case UTF16_LE:
        break;
      case BIG_ENDIAN_UTF16:
        break;
      case PLATFORM_DEPENDENT_UTF16:
        break;
      case ASCII:
        // same as binary
        this->append(string);
        return;
      case UTF8:
        // decode from UTF-8
        length = unitex::u_decode_utf8(string, data_->str);
        break;
    }
    // set the length of the resulting string
    data_->len = length;
  }

  /**
   * @brief  Constructor from unitex Ustring
   *
   * Allocates and initializes a string from a null-terminated character
   * sequence (Ustring-string)
   *
   * @param  string A null-terminated character sequence (Ustring-string)
   */
  explicit UnitexString(const Ustring* string) :
        data_(acquire(string->str)) {
  }

  /**
   * @brief  Constructor from unitex Ustring
   *
   * Initializes an already allocate string representing a null-terminated
   * character sequence (Ustring-string)
   *
   * @param  string A null-terminated character sequence (Ustring-string)
   */
  explicit UnitexString(Ustring* string) :
      data_(attach(string)) {
  }

  /**
   * @brief  Constructor from Ustring buffer
   *
   * Copies the first n chars from the array of characters pointed by string
   *
   * @param  string A null-terminated character sequence (Ustring-string)
   * @param  n      Number of characters to copy
   */
  UnitexString(const Ustring* string, size_type n) :
        data_(acquire(n)) {
    this->append(string, n);
  }

  // Destructor

  /**
   * @brief  Destroys the string object
   *
   * Free the memory allocated to the internal string
   */
  ~UnitexString() {
    // if data_ is acquired release, detach otherwise
    release() || detach();
  }

  // Iterators
  // Note that iterators can be invalidated by operations on the string

  /**
   * @brief  Return iterator to beginning
   *
   * Returns an iterator pointing to the first character of the string
   *
   * @return An iterator to the beginning of the string
   */
  iterator begin() {
    return iterator(data_->str);
  }

  /**
   * @brief  Return constant iterator to beginning
   *
   * Returns a constant iterator pointing to the first character of the string
   *
   * @return A constant iterator to the beginning of the string
   */
  const_iterator begin() const {
    return const_iterator(data_->str);
  }

  /**
   * @brief  Return constant iterator to beginning
   *
   * Returns a constant iterator pointing to the first character of the string.
   * This is an alias of UnitexString::begin
   *
   * @return A constant iterator to the beginning of the string
   */
  const_iterator cbegin() const {
    return const_iterator(data_->str);
  }

  /**
   * @brief  Returns an iterator pointing to the past-the-end character of
   * the string
   *
   * The past-the-end character is a theoretical character that would follow
   * the last character in the string. It shall not be dereferenced
   *
   * @return An iterator to the past-the-end of the string
   */
  iterator end() {
    return iterator(begin() + len());
  }

  /**
   * @brief  Returns a constant iterator pointing to the past-the-end character
   *  of  the string
   *
   * The past-the-end character is a theoretical character that would follow
   * the last character in the string. It shall not be dereferenced
   *
   * @return A constant iterator to the past-the-end of the string
   */
  const_iterator end() const {
    return const_iterator(begin() + len());
  }

  /**
   * @brief  Returns a constant iterator pointing to the past-the-end character
   *  of  the string
   *
   * The past-the-end character is a theoretical character that would follow
   * the last character in the string. It shall not be dereferenced. This is an
   * alias of UnitexString::end
   *
   * @return A constant iterator to the past-the-end of the string
   */
  const_iterator cend() const {
     return const_iterator(begin() + len());
  }

  // assignment operators =

  /**
   * @brief  String assignment from char
   *
   * Assigns a new value to the string, replacing its current contents
   *
   * @param  rhs    A character. The string value is set to a single copy of this
   *                character (the string length becomes 1).
   * @return *this
   */
  UnitexString& operator=(char rhs) {
    clear();
    this->append(rhs);
    return *this;
  }

  /**
   * @brief  String assignment from a C-string
   *
   * Assigns a new value to the string, replacing its current contents
   *
   * @param  rhs    A null-terminated character sequence (C-string). The sequence
   *                is copied as the new value for the string.
   * @return *this
   */
  UnitexString& operator=(const char* rhs) {
    unitex::u_strcpy(data_, rhs);
    return *this;
  }

  /**
   * @brief  String assignment from unichar
   *
   * Assigns a new value to the string, replacing its current contents
   *
   * @param  rhs    A character. The string value is set to a single copy of this
   *                character (the string length becomes 1).
   * @return *this
   */
  UnitexString& operator=(const unichar rhs) {
    clear();
    this->append(rhs);
    return *this;
  }

  /**
   * @brief  String assignment from a unichar-string
   *
   * Assigns a new value to the string, replacing its current contents
   *
   * @param  rhs    A Unitex unichar-string. The sequence  is copied as the new
   *                value for the string.
   * @return *this
   */
  UnitexString& operator=(const unichar* rhs) {
    if (data_->str != rhs) {
       unitex::u_strcpy(data_, rhs);
    }
    return *this;
  }

  /**
   * @brief  String assignment from a Ustring-string
   *
   * Assigns a new value to the string, replacing its current contents
   *
   * @param  rhs    A Unitex Ustring-string. The sequence  is copied as the new
   *                value for the string.
   * @return *this
   */
  UnitexString& operator=(const Ustring* rhs) {
    if (data_ != rhs) {
       unitex::u_strcpy(data_, rhs);
    }
    return *this;
  }

  /**
   * @brief  String assignment from a UnitexString object
   *
   * Assigns a new value to the string, replacing its current contents
   *
   * @param  rhs  A null-terminated character sequence (C-string). The sequence
   *              is copied as the new value for the string.
   * @return *this
   */
  UnitexString& operator=(const UnitexString& rhs) {
    return operator=(rhs.data_);
  }

  // concatenation operands +=

  /**
   * @brief  Append to string from char
   *
   * Extends the string by appending an additional character at the end of its
   * current value
   *
   * @param  rhs    A character, which is appended to the current value of
   *                the string
   * @return *this
   */
  UnitexString& operator+=(const char rhs) {
    return this->append(rhs);
  }

  /**
   * @brief Append to string from a C-string
   *
   * Extends the string by appending additional characters at the end of its
   * current value
   *
   * @param  rhs    A null-terminated character sequence (C-string). The
   *                sequence is appended to the current value of the string
   * @return *this
   */
  UnitexString& operator+=(const char* rhs) {
    return this->append(rhs);
  }

  /**
   * @brief  Append to string from unichar
   *
   * Extends the string by appending an additional character at the end of its
   * current value
   *
   * @param  rhs    A character, which is appended to the current value of
   *                the string
   * @return *this
   */
  UnitexString& operator+=(const unichar rhs) {
    return this->append(rhs);
  }

  /**
   * @brief  Append to string from a unichar-string
   *
   * Extends the string by appending additional characters at the end of its
   * current value
   *
   * @param  rhs    A Unitex unichar-string. The sequence is appended to the
   *                current value of the string
   * @return *this
   */
  UnitexString& operator+=(const unichar* rhs) {
    return this->append(rhs);
  }

  /**
   * @brief  Append to string from a Ustring-string
   *
   * Extends the string by appending additional characters at the end of its
   * current value
   *
   * @param  rhs    A Unitex Ustring-string. The sequence is appended to the
   *                current value of the string
   * @return *this
   */
  UnitexString& operator+=(const Ustring* rhs) {
    return this->append(rhs);
  }

  /**
   * @brief  Append to string from a UnitexString object
   *
   * Extends the string by appending additional characters at the end of its
   * current value
   *
   * @param  rhs    A UnitexString object, whose value is copied at the end
   * @return *this
   */
  UnitexString& operator+=(const UnitexString& rhs) {
    return this->append(rhs);
  }

  // index operands

  /**
   * @brief  Get character of string
   *
   * Returns a reference to the character at position index in the string
   *
   * @param  index  Value with the position of a character within the string
   * @return a UnitexString::reference
   *
   * @attention This function doesn't check bounds, unlike UnitexString::at()
   * method.
   */
  reference operator[] (size_type index) {
    return *(begin() + index);
  }

  /**
   * @brief  Get character of string
   *
   * Returns a constant reference to the character at position index in
   * the string
   *
   * @param  index  Value with the position of a character within the string
   * @return a UnitexString::const_reference
   *
   * @attention This function doesn't check bounds, unlike UnitexString::at()
   * method.
   */
  const_reference operator[] (size_type index) const {
    return *(begin() + index);
  }

  // boolean equality operands

  /**
   * @brief  Test equivalence of C-string and UnitexString
   *
   * @param  rhs    A null-terminated character sequence (C-string)
   * @return True if this->compare(rhs) == 0.  False otherwise
   */
  bool operator==(const char* rhs) const {
    return this->compare(rhs) == 0;
  }

  /**
   * @brief  Test equivalence of unichar-string and UnitexString
   *
   * @param  rhs    A Unitex unichar-string
   * @return True if this->compare(rhs) == 0.  False otherwise
   */
  bool operator==(const unichar* rhs) const {
    return this->compare(rhs) == 0;
  }

  /**
   * @brief  Test equivalence of Ustring-string and UnitexString
   *
   * @param  rhs    A Unitex Ustring-string
   * @return True if this->compare(rhs) == 0.  False otherwise
   */
  bool operator==(const Ustring* rhs) const {
    return u_equal(this->data_, rhs);
  }

  /**
   * @brief  Test equivalence between two UnitexString objects
   *
   * @param  rhs    A UnitexString object
   * @return True if both strings are equals.  False otherwise
   */
  bool operator==(const UnitexString& rhs) const {
    return u_equal(this->data_, rhs.data_);
  }

  // boolean not equal operands

  /**
   * @brief  Test difference of C-string and UnitexString
   *
   * @param  rhs    A null-terminated character sequence (C-string)
   * @return True if this->compare(rhs) != 0.  False otherwise
   */
  bool operator!=(const char* rhs) const {
    return !(*this == rhs);
  }

  /**
   * @brief  Test difference of unichar-string and UnitexString
   *
   * @param  rhs    A Unitex unichar-string
   * @return True if this->compare(rhs) != 0.  False otherwise
   */
  bool operator!=(const unichar* rhs) const {
    return !(*this == rhs);
  }

  /**
   * @brief  Test difference of Ustring-string and UnitexString
   *
   * @param  rhs    A Unitex Ustring-string
   * @return True if this->compare(rhs) != 0.  False otherwise
   */
  bool operator!=(const Ustring* rhs) const {
    return !(*this == rhs);
  }

  /**
   * @brief  Test difference between two UnitexString objects
   *
   * @param  rhs    A UnitexString object
   * @return True if this->compare(rhs) != 0.  False otherwise
   */
  bool operator!=(const UnitexString& rhs) const {
    return !(*this == rhs);
  }

  // Static methods

  /**
   * @brief  This function formats a list of arguments to a UnitexString
   *
   * @param  format C-string that contains the text to be written to stdout.
   *                It supports all the printf format options plus some
   *                extensions described in unitex::u_vsprintf
   * @param  ...    (additional arguments) Depending on the format string, the
   *                function may expect a sequence of additional arguments,
   *                each containing a value to be used to replace a format
   *                specifier in the format string. There should be at least
   *                as many of these arguments as the number of values
   *                specified in the format specifiers.
   * @return an UnitexString object
   *
   * @see    unitex::u_vsprintf  in Unicode.h for supported @e format options
   *
   * @note   Buffer is limit to kMaxBufferSize (1024) unicode characters
   */
  // UNITEX_PARAMS_NON_NULL
  // UNITEX_PRINTF_LIKE_FORMAT_CHECK(1,2)
  static UnitexString format(const char* format, ...) {
    va_list args;
    va_start(args, format);

    unichar string_buffer[kMaxBufferSize];

    const size_type size = unitex::u_vsprintf(string_buffer, format, args);

    va_end(args);

    assert(size < kMaxBufferSize);

    return UnitexString(string_buffer, size);
  }

  // Methods

  // element access

  /**
   * @brief  Get character in string
   *
   * Returns a reference to the character at position @e pos in the string
   *
   * @param  pos    The position of the character to access
   * @return Read/write reference to the character
   */
  reference at(size_type pos) {
    // FIXME(martinec) throws unitex::out_of_range
    // if (!UNITEX_STRING_IS_NULL &&  pos <= length())
    assert(!UNITEX_STRING_IS_NULL &&  pos <= len());
    return *(begin() + pos);
  }

  /**
   * @brief  Get character in string
   *
   * Returns a constant reference to the character at position @e pos in
   * the string
   *
   * @param  pos    The position of the character to access
   * @return Read-only (const) reference to the character.
   */
  const_reference at(size_type pos) const {
    assert(!UNITEX_STRING_IS_NULL && pos <= len());
    return *(begin() + pos);
  }

  /**
   * @brief  Get the first character in string
   *
   * @return The character at position @e 0 in the string
   */
  unichar front() const {
    assert(!this->is_empty());
    return this->at(0);
  }

  /**
   * @brief  Get the last character in string
   *
   * @return The character at position @e length-1 in the string
   */
  unichar back() const {
    assert(!this->is_empty());
    return this->at(this->len()-1);
  }

  /**
   * @brief  Resize string
   *
   * Resizes the the string to a length of @a size characters.
   * The buffer size is never decreased
   *
   * @param  size   New string length, expressed in number of characters
   *
   * @attention     You cannot set a size<1
   */
  void resize(size_type size) {
    unitex::resize(data_, size);
  }

  /**
   * @brief  Return length of string
   *
   * Returns the length of the string, in terms of number of characters
   */
  size_type len() const {
    return static_cast<size_type>(data_->len);
  }

  /**
   * @brief  Return length of string
   *
   * Returns the length of the string, in terms of number of characters
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
    return (len() * sizeof(value_type));
  }

  /**
   * @brief  Truncates string to the given length
   *
   * Restricts the maximum length of the string
   *
   * @param  length The length of the string, in terms of number of characters
   */
  void truncate(size_type length) {
    unitex::truncate(data_, length);
  }

  /**
   * @brief  Generate substring
   *
   * Returns a newly constructed string object with its value initialized
   * to a copy of a substring of this object
   *
   * @param  pos    Position of the first character to be copied as a substring
   * @param  len    Number of characters to include in the substring (if the
   *                string is shorter, as many characters as possible are used).
   *                A value of string::npos indicates all characters until the
   *                end of the string
   * @return        A UnitexString object with a substring of this object
   */
  UnitexString substr(size_type pos = 0, size_type len = npos) const {
    return UnitexString(*this, pos, len);
  }

//  /**
//   *  Returns the maximum length the string can reach
//   */
//  size_type max_size() const {
//    return kMaxSize;
//  }

  /**
   * @brief  Return size of allocated storage
   *
   * Returns the size of the storage space currently allocated for the string,
   * expressed in terms of bytes.
   */
  size_type capacity() const {
    return static_cast<size_type>(data_->size);
  }

  /**
   * @brief  Request a change in capacity
   *
   * Requests that the string capacity be adapted to a planned change in size
   * to a length of *up to* n characters.
   *
   * @param  n      Planned length for the string, expressed in number of
   *                characters
   *
   * @note          The resulting string capacity may be equal or greater
   *                than @e n. Notice also that in all other cases, the buffer
   *                size is never decreased
   */
  void reserve(size_type n = 0) {
    unitex::resize(data_, data_->len + n);
  }


  // string append functions

  /**
   * @brief  Append to string from a C-string
   *
   * Extends the string by appending additional characters at the end of its
   * current value
   *
   * @param  string A null-terminated character sequence (C-string). The
   *                sequence is appended to the current value of the string
   * @return *this
   */
  UnitexString& append(const char* string) {
    unitex::u_strcat(data_, string);
    return *this;
  }

  /**
   * @brief  Append to string from a C-substring
   *
   * Appends a copy of the first @e n characters in the array of characters
   * pointed by a string const char*
   *
   * @param  string Pointer to an array of characters (C-string)
   * @param  n      Number of characters to copy
   *
   * @return *this
   */
  UnitexString& append(const char* string, size_type n) {
    unitex::u_strcat(data_, string, n);
    return *this;
  }

  /**
   * @brief  Append to string from unichar
   *
   * Extends the string by appending an additional character at the end of its
   * current value
   *
   * @param  character Unitex unichar character, which is appended to the
   *                   current value of the string
   * @return *this
   */
  UnitexString& append(const unichar character) {
    unitex::u_strcat(data_, character);
    return *this;
  }

  /**
   * @brief  Append to string from a unichar-string
   *
   * Extends the string by appending additional characters at the end of its
   * current value
   *
   * @param  string A Unitex unichar-string. The sequence is appended to the
   *                current value of the string
   * @return *this
   */
  UnitexString& append(const unichar* string) {
    unitex::u_strcat(data_, string);
    return *this;
  }

  /**
   * @brief  Append to string from a unichar-substring
   *
   * Appends a copy of the first n characters in the array of characters
   * pointed by a string const unichar*
   *
   * @param  string Pointer to an array of characters (unichar-string)
   * @param  n      Number of characters to copy
   *
   * @return *this
   */
  UnitexString& append(const unichar* string, size_type n) {
    unitex::u_strcat(data_, string, n);
    return *this;
  }

  /**
   * @brief  Append to string from a Ustring-string
   *
   * Extends the string by appending additional characters at the end of its
   * current value
   *
   * @param  string A Unitex Ustring-string. The sequence is appended to the
   *                current value of the string
   * @return *this
   */
  UnitexString& append(const Ustring* string) {
    unitex::u_strcat(data_, string);
    return *this;
  }

  /**
   * @brief  Append to string from a Ustring-substring
   *
   * Appends a copy of the first n characters in the array of characters
   * pointed by a string const Ustring*
   *
   * @param  string Pointer to an array of characters (Ustring-string)
   * @param  n      Number of characters to copy
   *
   * @return *this
   */
  UnitexString& append(const Ustring* string, size_type n) {
    return this->append(string, n);
  }

  /**
   * @brief  Append to string from a UnitexString object
   *
   * Extends the string by appending additional characters at the end of its
   * current value
   *
   * @param  string A UnitexString object, whose value is copied at the end
   * @return *this
   */
  UnitexString& append(const UnitexString& string) {
    return this->append(string.data_);
  }

  /**
   * @brief  Append to string from a UnitexString object
   *
   * Appends a copy of the first n characters in the array of characters
   * pointed by a UnitexString object
   *
   * @param  string UnitexString object, whose value is copied at the end
   * @param  n      Number of characters to copy
   *
   * @return *this
   */
  UnitexString& append(const UnitexString& string, size_type n) {
    return this->append(string.data_, n);
  }


  /**
   * @brief  This function append a formated list of arguments to a UnitexString
   *
   * Formats a list of arguments and append the final result to the underline
   * string returning the UnitexString object
   *
   * @param  format C-string that contains the text to be written to stdout.
   *                It supports all the printf format options plus some
   *                extensions described in unitex::u_vsprintf
   * @param  ...    (additional arguments) Depending on the format string, the
   *                function may expect a sequence of additional arguments,
   *                each containing a value to be used to replace a format
   *                specifier in the format string. There should be at least
   *                as many of these arguments as the number of values
   *                specified in the format specifiers.
   * @return *this
   *
   * @see    UnitexString::format(const char* format, ...)
   *
   * @note   Buffer is limit to kMaxBufferSize (1024) unicode characters
   */
  // UNITEX_PARAMS_NON_NULL
  UnitexString& append_format(const char* format, ...) {
    va_list args;
    va_start(args, format);

    unichar string_buffer[kMaxBufferSize];

    const size_type size = unitex::u_vsprintf(string_buffer, format, args);

    va_end(args);

    assert(size <= kMaxBufferSize);

    // resize to fit
    this->resize(this->len() + size + 1);

    // append string_buffer
    return this->append(string_buffer, size);
  }

  /**
   * @brief  Encode the UnitexString into a UTF-8 C-string
   *
   * @param  An already allocated buffer destination (C-string)
   *
   * @return length of the destination string
   */
  int encode(char* destination) {
    return u_encode_utf8(data_->str, destination);
  }

  /**
   * @brief  Reverse the string
   * @return *this
   */
  UnitexString& reverse() {
    u_reverse(data_->str, data_->len);
    return *this;
  }

  /**
   * @brief  Lowercase the characters in the string
   * @return *this
   */
  UnitexString& lower() {
    unitex::u_tolower(data_->str);
    return *this;
  }

  /**
   * @brief  Upper case the characters in the string
   * @return *this
   */
  UnitexString& upper() {
    unitex::u_toupper(data_->str);
    return *this;
  }

  /**
   * @brief  Fold case the characters in the string
   * @return *this
   */
  UnitexString& fold() {
    unitex::u_tofold(data_->str);
    return *this;
  }

  /**
   * @brief  Title case the characters in the string
   * @return *this
   */
  UnitexString& title() {
    unitex::u_totitle_first(data_->str);
    return *this;
  }

  /**
   * @brief  Deaccentuate the characters in the string
   * @return *this
   */
  UnitexString& deaccentuate() {
    unitex::u_deaccentuate(data_->str);
    return *this;
  }

  /**
   * @brief  Append character to string
   *
   * Appends character c to the end of the string, increasing its length by one
   *
   * @param  character Character added to the string
   */
  void push_back(const unichar character) {
     this->append(character);
  }

//  void pop_back() {
//    // TODO(martinec) UnitexString::erase()
//    this->erase(this->lenght()-1, 1);
// }

  // string comparisons functions

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
    return unitex::u_strcmp(data_->str, string);
  }

  /**
   * @brief  Compare to a unichar-string
   *
   * @param  string A Unitex unichar-string
   * @return Returns a signed integral indicating the relation between
   *         the strings.
   *
   * @see    UnitexString::compare(const char* string) const
   */
  int compare(const unichar* string) const {
    return unitex::u_strcmp(data_->str, string);
  }

  /**
   * @brief  Compare to a Ustring-string
   *
   * @param  string A Unitex Ustring-string
   * @return Returns a signed integral indicating the relation between
   *         the strings.
   *
   * @see    UnitexString::compare(const char* string) const
   */
  int compare(const Ustring* string) const {
    return this->compare(string->str);
  }

  /**
   * @brief  Compare to a UnitexString object
   *
   * @param  string A UnitexString object
   * @return Returns a signed integral indicating the relation between
   *         the strings.
   *
   * @see    UnitexString::compare(const char* string) const
   */
  int compare(const UnitexString& string) const {
    return this->compare(string.data_->str);
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
  int icompare(const char* string) const {
    return unitex::u_strcmp_ignore_case(data_->str, string);
  }

  /**
   * @brief  Case insensitive compare to a unichar-string
   *
   * @param  string A Unitex unichar-string
   * @return Returns a signed integral indicating the relation between
   *         the strings.
   *
   * @see    UnitexString::icompare(const char* string) const
   */
  int icompare(const unichar* string) const {
    return unitex::u_strcmp_ignore_case(data_->str, string);
  }

  /**
   * @brief  Case insensitive compare to a Ustring-string
   *
   * @param  string A Unitex Ustring-string
   * @return Returns a signed integral indicating the relation between
   *         the strings.
   *
   * @see    UnitexString::icompare(const char* string) const
   */
  int icompare(const Ustring* string) const {
    return this->icompare(string->str);
  }

  /**
   * @brief  Case insensitive compare to a UnitexString object
   *
   * @param  string A UnitexString object
   * @return Returns a signed integral indicating the relation between
   *         the strings.
   *
   * @see    UnitexString::icompare(const char* string) const
   */
  int icompare(const UnitexString& string) const {
    return this->compare(string.data_->str);
  }

  /**
   * @brief  Get unichar-string equivalent
   *
   * @return A pointer to the underlying built-in unichar
   *
   * @attention The caller should not delete the return value
   */
  const unichar* c_unichar() const {
    // this->push_back(0);
    // this->pop_back();
    return data();
  }

  /**
   * @brief  Implicit conversion from UnitexString to the underlying unichar
   *         buffer
   *
   * @see  c_unichar() const
   */
  UNITEX_EXPLICIT_CONVERSIONS
  operator const unichar*() const {
    return data();
  }

  /**
   * @brief  Get string data
   *
   * @return A pointer to the underlying built-in unichar
   *
   * @attention The caller should not delete the return value
   */
  const unichar* data() const {
    return UNITEX_STRING_IS_NULL ? '\0' : begin();
  }

  /**
   * @brief  Get Ustring-string equivalent
   *
   * @return A pointer to the underlying built-in Ustring
   *
   * @attention The caller should not delete the return value
   */
  const Ustring* c_ustring() const {
    return data_;
  }

  /**
   * @brief  Implicit conversion from UnitexString to the underlying Ustring
   * object
   *
   * @see  c_ustring() const
   */
  UNITEX_EXPLICIT_CONVERSIONS
  operator const Ustring*() const {
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
    return UNITEX_STRING_IS_NULL;
  }

  /**
   * @brief  Test whether the string is empty
   *
   * @return True if the string is empty. False otherwise
   *
   * Returns whether the string is empty (i.e. whether its length is 0).
   *
   * @note   This differs from unitex::empty() function in Ustring.h. Note also
   *         that NULL and the empty string are different.
   *
   * @see    is_null() const
   */
  bool is_empty() const {
    return (!UNITEX_STRING_IS_NULL && len() == 0);
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
   * @brief  Clear string
   *
   * Erases the contents of the string, which becomes an empty string
   * (with a length of 0 characters).
   *
   * @note   The capacity of the internal buffer remains unchanged
   */
  void clear() {
    unitex::empty(data_);
  }

  /**
   * @brief  Swap string values
   *
   * Exchanges the content of the container by the content of string,
   * which is another UnitexString object.
   *
   * @param  string Another UnitexString object, whose value is swapped with
   *                that of this string
   */
  void swap(UnitexString& string) {
    if (this != &string) {
       this->swap(data_->str,  string.data_->str);
       this->swap(data_->size, string.data_->size);
       this->swap(data_->len,  string.data_->len);                  // NOLINT
    }
  }

  // concatenation friends in an unnamed namespace

  /**
   * @brief  Concatenate a char and a UnitexString object
   *
   * @param  lhs    A UnitexString object
   * @param  rhs    A char whose value will be concatenated to lhs
   * @return A UnitexString whose value is the concatenation of lhs and rhs
   */
  friend UnitexString operator+(const UnitexString& lhs,
                                char rhs) {
      return UnitexString(lhs).operator+=(rhs);
  }

  /**
   * @brief  Concatenate a UnitexString object and a char
   *
   * @param  lhs    A char
   * @param  rhs    A UnitexString object whose value will be concatenated to lhs
   * @return A UnitexString whose value is the concatenation of lhs and rhs
   */
  friend UnitexString operator+(char lhs,
                                const UnitexString& rhs) {
      return UnitexString(&lhs, 1).append(rhs);
  }

  /**
   * @brief  Concatenate a C-string and a UnitexString object
   *
   * @param  lhs    A UnitexString object
   * @param  rhs    A null-terminated character sequence (C-string) whose value
   *                will be concatenated to lhs
   * @return A UnitexString whose value is the concatenation of lhs and rhs
   */
  friend UnitexString operator+(const UnitexString& lhs,
                                const char* rhs) {
    return  UnitexString(lhs) += rhs;
  }

  /**
   * @brief  Concatenate a unichar-string and a UnitexString object
   *
   * @param  lhs    A UnitexString object
   * @param  rhs    A Unitex unichar-string whose value will be concatenated to
   *                lhs
   * @return A UnitexString whose value is the concatenation of lhs and rhs
   */
  friend UnitexString operator+(const UnitexString& lhs,
                                unichar rhs) {
      return UnitexString(lhs).operator+=(rhs);
  }

  /**
   * @brief  Concatenate a UnitexString object and a unichar-string
   *
   * @param  lhs    A Unitex unichar-string
   * @param  rhs    A UnitexString object whose value will be concatenated to
   *                lhs
   * @return A UnitexString whose value is the concatenation of lhs and rhs
   */
  friend UnitexString operator+(unichar lhs,
                                const UnitexString& rhs) {
      return UnitexString(&lhs, 1).append(rhs);
  }

  /**
   * @brief  Concatenate a C-string and a UnitexString object
   *
   * @param  lhs    A null-terminated character sequence (C-string)
   * @param  rhs    A UnitexString object whose value will be concatenated to
   *                lhs
   * @return A UnitexString whose value is the concatenation of lhs and rhs
   */
  friend UnitexString operator+(const char* lhs,
                                const UnitexString& rhs) {
    return  UnitexString(lhs) += rhs;
  }

  /**
   * @brief  Concatenate a unichar-string and a UnitexString object
   *
   * @param  lhs    A UnitexString object
   * @param  rhs    A Unitex unichar-string whose value will be concatenated to
   *                lhs
   * @return A UnitexString whose value is the concatenation of lhs and rhs
   */
  friend UnitexString operator+(const UnitexString& lhs,
                                const unichar* rhs) {
    return  UnitexString(lhs) += rhs;
  }

  /**
   * @brief  Concatenate a UnitexString object and a unichar-string
   *
   * @param  lhs    A Unitex unichar-string
   * @param  rhs    A UnitexString object whose value will be concatenated to
   *                lhs
   * @return A UnitexString whose value is the concatenation of lhs and rhs
   */
  friend UnitexString operator+(const unichar* lhs,
                                const UnitexString& rhs) {
    return  UnitexString(lhs) += rhs;
  }

  /**
   * @brief  Concatenate a Ustring-string and a UnitexString object
   *
   * @param  lhs    A UnitexString object
   * @param  rhs    A Unitex Ustring-string whose value will be concatenated to
   *                lhs
   * @return A UnitexString whose value is the concatenation of lhs and rhs
   */
  friend UnitexString operator+(const UnitexString& lhs,
                                const Ustring* rhs) {
    return  UnitexString(lhs) += rhs;
  }

  /**
   * @brief  Concatenate a UnitexString object and a Ustring-string
   *
   * @param  lhs    A Unitex Ustring-string
   * @param  rhs    A UnitexString object whose value will be concatenated to
   *                lhs
   * @return A UnitexString whose value is the concatenation of lhs and rhs
   */
  friend UnitexString operator+(const Ustring* lhs,
                                const UnitexString& rhs) {
    return  UnitexString(lhs) += rhs;
  }

  // concatenation const UnitexString&
  /**
   * @brief  Concatenate two UnitexString objects
   *
   * @param  lhs    A UnitexString object
   * @param  rhs    A UnitexString object whose value will be concatenated to
   *                lhs
   * @return A UnitexString whose value is the concatenation of lhs and rhs
   */
  friend UnitexString operator+(const UnitexString& lhs,
                                const UnitexString& rhs) {
    return  UnitexString(lhs) += rhs;
  }

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
   * @brief  Attach an Ustring
   * @note   To be used only from a constructor
   * @see    data_
   */
  Ustring* attach(Ustring* string) {
    engaged_ = 0;
    return string;
  }

  /**
   * @brief  Detach data_ from an attached Ustring
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
   * @brief  Allocate an empty Ustring
   * @see    data_
   */
  Ustring* acquire() {
    engaged_ = 1;
    return new_Ustring();
  }

  /**
   * @brief  Allocate an Ustring representing the given string
   * @see    data_
   */
  Ustring* acquire(const unichar* string) {
    engaged_ = 1;
    return new_Ustring(string);
  }

  /**
   * @brief  Allocate an empty Ustring with a buffer set to the given size
   * @see    data_
   */
  Ustring* acquire(size_type size) {
    engaged_ = 1;
    return new_Ustring(size);
  }
  /**
   * @brief  Free the memory allocated to the internal Ustring
   * @see    data_
   */
  int release() {
    if (is_acquired()) {
      engaged_ = 0;
      this->clear();
      free_Ustring(data_);
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
   * @brief  underline Ustring container
   */
  int8_t engaged_;

  /**
   * @brief  underline Ustring container
   */
  Ustring* data_;
};  // class UnitexString

// [BEGIN class UnitexString non-member overloads
/// @addtogroup non-member-overloads non-member overloads
/// @ingroup    UnitexString
/// @{

/**
 * @brief    Is less than string comparison
 *
 * Return True either the value of the first character that does not match is
 * lower in the compared string, or all compared characters match but the
 * compared string is shorter.
 *
 * @param    lhs    A UnitexString object
 * @param    rhs    A UnitexString object
 * @return   True if lhs.compare(rhs) < 0. False otherwise
 */
inline bool operator<(const UnitexString& lhs, const UnitexString& rhs) {
  return lhs.compare(rhs) < 0;
}

/**
 * @brief    Is less than or equal string comparison
 *
 * Return True either the value of the first character that does not match is
 * lower in the compared string, all compared characters match but the
 * compared string is shorter, or the string compare is equal
 *
 * @param    lhs    A UnitexString object
 * @param    rhs    A UnitexString object
 * @return   True if lhs.compare(rhs) <= 0. False otherwise
 */
inline bool operator<=(const UnitexString& lhs, const UnitexString& rhs) {
  return lhs.compare(rhs) <= 0;
}

/**
 * @brief    Is greater than string comparison
 *
 * Return True Either the value of the first character that does not match is
 * greater in the compared string, or all compared characters match but the
 * compared string is longer.
 *
 * @param    lhs    A UnitexString object
 * @param    rhs    A UnitexString object
 * @return   True if lhs.compare(rhs) > 0. False otherwise
 */
inline bool operator>(const UnitexString& lhs, const UnitexString& rhs) {
  return lhs.compare(rhs) > 0;
}

/**
 * @brief    Is greater than or equal string comparison
 *
 * Return True Either the value of the first character that does not match is
 * greater in the compared string, all compared characters match but the
 * compared string is longer, or the string compare is equal
 *
 * @param    lhs    A UnitexString object
 * @param    rhs    A UnitexString object
 * @return   True if lhs.compare(rhs) >= 0. False otherwise
 */
inline bool operator>=(const UnitexString& lhs, const UnitexString& rhs) {
  return lhs.compare(rhs) >= 0;
}

/**
 * @brief    Exchanges the values of two UnitexString objects
 *
 * @param    a,b Two UnitexString objects, whose contents are swapped
 */
inline void swap(UnitexString& a, UnitexString& b) {                // NOLINT
  a.swap(b);
}
/// @}
// END] class UnitexString non-member overloads
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
/* ************************************************************************** */
#endif  // UNITEX_UNITEXSTRING_H_                                   // NOLINT
