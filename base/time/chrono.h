/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      chrono.h
 * @brief     Simple Portable Time Class
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 chrono.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_TIME_CHRONO_H_                                  // NOLINT
#define UNITEX_BASE_TIME_CHRONO_H_                                  // NOLINT
/* ************************************************************************** */
#include "base/time/types.h"
#include "base/time/functions.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
class Chrono {
 public :
  // Typedefs and Enums
  //typedef detail::time_unit_t time_unit_t;
  //typedef detail::timeval_t   timeval_t;
  //typedef detail::timeinfo_t  timeinfo_t;
  //typedef detail::strtime_t   strtime_t;
  //typedef detail::strhour_t   strhour_t;

  // Constants (including static const data members)
  /**
   * 1 second represents one million microseconds
   */
  static const time_unit_t kOneSecondInMicroseconds      = TIME_UNIT_C(1000000);

  /**
   * 1 millisecond represents one thousand microseconds
   */
  static const time_unit_t kOneMillisecondInMicroseconds = TIME_UNIT_C(1000);

  /**
   * Predefined zero time value, 01/01/1970 00:00:00 (UTC)
   */
  static const Chrono Zero;

  // Constructors

  /**
   * Zero time value : 0 seconds and 0 microseconds
   * represents 01/01/1970 00:00:00 (UTC)
   */
  Chrono() : seconds_(TIME_UNIT_C(0)),
             microseconds_(TIME_UNIT_C(0)) {
  }

  /**
   *
   */
  Chrono(time_unit_t seconds,  time_unit_t microseconds)
            : seconds_(seconds), microseconds_(microseconds) {
  }

  /**
   * Explicit constructor timeval_t
   */
  explicit Chrono(const timeval_t& time)
              : seconds_(static_cast<time_unit_t>(time.tv_sec)),
                microseconds_(static_cast<time_unit_t>(time.tv_usec)) {
  }

  /**
   * Explicit constructor time_t
   */
  explicit Chrono(const time_t& time)
              : seconds_(static_cast<time_unit_t>(time)),
                microseconds_(TIME_UNIT_C(0)) {
  }

  /**
   * Copy constructor
   */
  Chrono(const Chrono& time)
              : seconds_(time.seconds_),
                microseconds_(time.microseconds_){

  }

  // Destructor
  /**
   * Destructor does nothing
   */
  ~Chrono() {
  }

  // Overload some operators

  /**
   * Assignement operator from Time
   */
  Chrono& operator=(const Chrono& rhs) {
    if (this != &rhs) {
      seconds_      = rhs.seconds_;
      microseconds_ = rhs.microseconds_;
    }
    return *this;
  }

  /**
   * Equality operator over Time
   */
  bool operator ==(const Chrono& rhs) const {
    return (seconds_      == rhs.seconds_ &&
            microseconds_ == rhs.microseconds_);
  }

  /**
   * Inequality operator over Time
   */
  bool operator !=(const Chrono& rhs) const {
    return !(*this == rhs);
  }

  /**
   * Less than operator over Time
   */
  bool operator <(const Chrono& rhs) const {
    return ((seconds_       <  rhs.seconds_ ) ||
            ((seconds_      == rhs.seconds_ ) &&
             (microseconds_ <  rhs.microseconds_)));
  }

  /**
   * Greater than operator over Time
   */
  bool operator >(const Chrono& rhs) const {
    return rhs < *this;
  }

  /**
   * Less than or equal to operator over Time
   */
  bool operator <=(const Chrono& rhs) const {
    return !(rhs < *this);
  }

  /**
   * Greater than or equal to operator over Time
   */
  bool operator >=(const Chrono& rhs) const {
    return !(*this < rhs);
  }

  /**
   * minus (neg) operator
   */
  Chrono operator -() const {
    return Chrono(-seconds_, -microseconds_);
  }

  /**
   *  addition operator over Time
   *  x + y
   */
  Chrono& operator +=(const Chrono& rhs) {
    seconds_      += rhs.seconds_;
    microseconds_ += rhs.microseconds_;
    return normalize(this);
  }

  /**
   *  substraction operator over Time
   *  x - y = x + -y
   */
  Chrono& operator -=(const Chrono& rhs) {
    return (*this) += rhs.operator-();
  }

  /**
   * Specifies a conversion from Time to timeval_t
   * using a conversion-type-id function
   */
  UNITEX_EXPLICIT_CONVERSIONS
  operator timeval_t() const {
    timeval_t time = {/* zero initialized */};
    time.tv_sec    =  seconds_;
    time.tv_usec   =  microseconds_;
    return time;
  }

  /**
   * Specifies a conversion from Time to time_t
   * using a conversion-type-id function
   */
  UNITEX_EXPLICIT_CONVERSIONS
  operator time_t() const {
    return static_cast<time_t> (seconds_);
  }


  // Methods, including static

  /**
   * Obtain the current time, expressed as seconds and microseconds
   * since the Epoch
   */
  static timeval_t gettimeofday () {
    timeval_t time = {/* zero initialized */};
    time_now(&time, NULL);
    return time;
  }

  /**
   *
   */
  static timeinfo_t localtime (const time_t* time_p) {
    timeinfo_t  timeinfo = {/* zero initialized */};
    time_unit_t timeunit = static_cast<time_unit_t>(*time_p);
    localtime_safe(&timeunit, &timeinfo);
    return timeinfo;
  }

  /**
   *
   */
  static unichar* asctime(const time_t* time_p,
                          unichar* buffer) {         // NOLINT
    timeinfo_t  timeinfo = localtime(time_p);                        // NOLINT
    return time_asctime_r(&timeinfo, buffer);
  }

  /**
   * Same as gettimeofday but returns a Time object
   */
  static Chrono now() {
    timeval_t tv = {/* zero initialized */};
    if(time_now(&tv, NULL) == 0){
       return Chrono(tv);
    }
    return Chrono();
  }

  /**
   *
   */
  timeinfo_t local() const {
    timeinfo_t timeinfo = {/* zero initialized */};
    localtime_safe(&seconds_, &timeinfo);
    return timeinfo;
  }

  // as_ unitname functions

  /**
   * The returned string has the following format: "Www Mmm dd hh:mm:ss yyyy\n"
   */
  UnitexString as_string() const {
    timeinfo_t  timeinfo = this->local();
    strtime_t   buffer   = {/* zero initialized */};
    time_asctime_r(&timeinfo, buffer);
    return UnitexString(buffer);
  }

  /**
   * The returned string has the following format: "hh:mm::ss.uuuuuu"
   */
  UnitexString as_timestamp() const {
    timeinfo_t  timeinfo = this->local();
    strhour_t   buffer   = {/* zero initialized */};
    time_aschour_r(&timeinfo, buffer);
    return UnitexString(buffer).append_format(".%06d", microseconds_);
  }

  /**
   * as seconds
   */
  double as_seconds() const {
    return (seconds_ +
            static_cast<double>(microseconds_) / kOneSecondInMicroseconds);
  }

  /**
   * as microseconds
   */
  double as_microseconds() const {
    return (seconds_ * kOneSecondInMicroseconds + microseconds_);
  }

  /**
   * as milliseconds
   */
  double as_milliseconds() const {
    return (as_microseconds() / kOneMillisecondInMicroseconds);
  }


  /**
   * get seconds
   */
  time_unit_t get_seconds() const {
    return seconds_;
  }

  /**
   * get microseconds
   * always less than one million
   */
  time_unit_t get_microseconds() const {
    return microseconds_;
  }

  /**
   * get milliseconds
   * 1 millisecond is 1000 microseconds
   */
  time_unit_t get_milliseconds() const {
    return microseconds_ / kOneMillisecondInMicroseconds;
  }

  /**
   * elapsed
   */
  Chrono elapsed(const Chrono& end_time) const {
    return end_time - *this;
  }

  // Data Members (except static const data members)
 private :
  // Typedefs and Enums

  // Constants (including static const data members)

  // Constructors

  // Destructor

  // Methods, including static
  /**
   * Set a proper number of microseconds dealing with
   * overflows (>= +1000000) and sign (<0)
   */
  static Chrono& normalize(Chrono* time) {
    // this function works even if time_unit_t is unsigned
    time_unit_t delta_seconds =   TIME_UNIT_C(0);

    // if microseconds aren'nt in the range (-1000000, +1000000)
    if((time->microseconds_ <= -kOneSecondInMicroseconds) ||
       (time->microseconds_ >=  kOneSecondInMicroseconds)) {
      delta_seconds       = time->microseconds_ / kOneSecondInMicroseconds;
      time->microseconds_ -= delta_seconds      * kOneSecondInMicroseconds;
      time->seconds_      += delta_seconds;
    }

    // microseconds are already in the range (-1000000, +1000000)
    // but we need to adjust the sign :
    // case 1 : microseconds negatives (-) seconds positives (+)
    // case 2 : microseconds positives (+) seconds negatives (-)
    // 1. microseconds are in the range (-1000000, 0) and seconds > 0
    // 2. microseconds are in the range (0, 1000000 ) and seconds < 0
    if((time->microseconds_ < 0 && time->seconds_ > 0) ||
       (time->microseconds_ > 0 && time->seconds_ < 0) ) {
      delta_seconds        = time->microseconds_ < 0 ? -TIME_UNIT_C(1):
                                                        TIME_UNIT_C(1);
      time->microseconds_ -= delta_seconds * kOneSecondInMicroseconds;
      time->seconds_      += delta_seconds;
    }

    return *time;
  }

  // Data Members (except static const data members)

  /**
   *
   */
  time_unit_t seconds_;

  /**
   *
   */
  // always less than one million
  // see the normalize function for the normalization details
  time_unit_t microseconds_;

  // time arithmetics friends

  // time addition
  friend Chrono operator+(const Chrono& lhs, const Chrono& rhs) {
    return  Chrono(lhs) += rhs;
  }

  // time subtraction
  friend Chrono operator-(const Chrono& lhs, const Chrono& rhs) {
    return  Chrono(lhs) -= rhs;
  }
};  // class Time
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_TIME_CHRONO_H_                               // NOLINT
