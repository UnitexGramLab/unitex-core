/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      functions.h
 * @brief     Simple Portable Time Functions
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 functions.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_TIME_FUNCTIONS_H_                               // NOLINT
#define UNITEX_BASE_TIME_FUNCTIONS_H_                               // NOLINT
/* ************************************************************************** */
// Other libraries' .h files       (try to order the includes alphabetically)
#include "base/time/types.h"       // time-related types
/* ************************************************************************** */
// time_asctime_r  // asctime using unichar without use a static buffer
// time_aschour_r  // hh:mm:ss using unichar and without use a static buffer
// time_now        // obtain the current time since the Epoch
// localtime_safe  // safe localtime function version
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
# if defined(UNITEX_UNIX_FEATURE_MACH_CLOCK_GETTIME)
/* ************************************************************************** */
namespace helper {   // unitex::helper
/* ************************************************************************** */
/**
 * MachClockService
 */
// Resource Acquisition Is Initialization used for MACH clock_service port
class MachClockService {
 public:
    /**
     * Contructor obtains a handle to the clock
     */
    MachClockService() {
      if (!count_++) {
        kern_return_t kn_retval;

        if (clock_service_ == 0) {
          // the client first obtains a handle to the clock (clock_serv_t) by
          // calling host_get_clock_service.
          kn_retval = host_get_clock_service(
                        mach_host_self(),  // returns the host self port
                        CALENDAR_CLOCK,    // syncronized with the machine's RTC
                        &clock_service_);

          if (kn_retval != KERN_SUCCESS) {
              clock_service_ = 0;
          }
        }
      }
    }

    /**
     * Returns a reference of clock_service
     */
    clock_serv_t& get() {
      return clock_service_;
    }

    /**
     * Test if clock_service is already initialized
     */
    bool initialized() {
      return (count_ != 0 &&
              clock_service_ != 0);
    }

    /**
     * Destructor deallocate clock_service port
     */
    ~MachClockService() {
      if (!--count_) {
        mach_port_deallocate(mach_task_self(), clock_service_);
      }
    }

 private:
    /**
     * initialization counter
     */
    static uint32_t count_;

    /**
     * pointer to clock structure
     */
    static clock_serv_t clock_service_;

    /**
     * This class disallow implicit copy constructor and assignment
     */
    UNITEX_DISALLOW_COPY_AND_ASSIGN(MachClockService);
};

uint32_t MachClockService::count_ = 0;
clock_serv_t MachClockService::clock_service_ = 0;
/* ************************************************************************** */
}  // namespace unitex::helper                                      // NOLINT
/* ************************************************************************** */
#endif  // defined(UNITEX_UNIX_FEATURE_MACH_CLOCK_GETTIME)
/* ************************************************************************** */
namespace {   // namespace unitex::{unnamed}, to enforce the one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
/**
 * @def    time_asctime_r
 * @brief  asctime using unichar without use a static buffer
 *
 * Similar to asctime using unichar and without use a static buffer.
 * The buffer needs to be of at least 26 bytes (POSIX Base Specifications)
 *
 * The returned string has the following format: Www Mmm dd hh:mm:ss yyyy
 * The string is followed by a new-line character ('\n') and terminated
 * with a null-character.
 *
 * @note   asctime is not required to be thread-safe
 */
UNITEX_FORCE_INLINE
unitex::unichar* time_asctime_r(const timeinfo_t* timeinfo_p,
                                unitex::unichar* buffer) {
  assert( timeinfo_p && timeinfo_p->tm_wday >= 0 && timeinfo_p->tm_wday <= 7 );
  assert( timeinfo_p && timeinfo_p->tm_mon  >= 0 && timeinfo_p->tm_mon  <= 12);

  static const char kDummyTime[] = "??? ??? ?? ??:??:?? ????\n";

  // copy a dummy output if timeinfo_p is null
  if (timeinfo_p == NULL) {
      return unitex::u_strcpy(buffer, kDummyTime);
  }

  // time string format
  static const char kStrTimeFormat[] =
                    "%.3s %.3s%3d %02.2d:%02.2d:%02.2d %-4d\n";

  // weekends names
  static const char kWeekendName[][4]    = { "Sun", "Mon", "Tue", "Wed",
                                             "Thu", "Fri", "Sat" };

  // month names
  static const char kMonthShortName[][4] = { "Jan", "Feb", "Mar", "Apr",
                                             "May", "Jun", "Jul", "Aug",
                                             "Sep", "Oct", "Nov", "Dec" };
  // base year
  static const int kYearBase = 1900;

  // apply the format
  int chars_written = unitex::u_sprintf(buffer, kStrTimeFormat,
                                        kWeekendName[timeinfo_p->tm_wday],
                                        kMonthShortName[timeinfo_p->tm_mon],
                                        timeinfo_p->tm_mday,
                                        timeinfo_p->tm_hour,
                                        timeinfo_p->tm_min,
                                        timeinfo_p->tm_sec,
                                        kYearBase + timeinfo_p->tm_year);

  // only if u_sprintf success
  return chars_written > 0 ? buffer : unitex::u_strcpy(buffer, kDummyTime);
}  // time_asctime_r
/**
 * @def    time_aschour_r
 * @brief  hh:mm:ss using unichar and without use a static buffer
 *
 * hh:mm:ss using unichar and without use a static buffer
 * buffer needs to be exactly 9 bytes
 *
 * @note  strftime isn't always thread-safe and some implementations may have
 *        reentrancy problems. This implementation not required to be thread-safe
 */
UNITEX_FORCE_INLINE
unitex::unichar* time_aschour_r(const timeinfo_t* timeinfo_p, unitex::unichar* buffer) {
  static const char kDummyHour[] = "??:??:??";

  // copy a dummy output if timeinfo_p is null
  if (timeinfo_p == NULL) {
      return unitex::u_strcpy(buffer, kDummyHour);
  }

  // time string format
  static const char kStrHourFormat[] = "%02.2d:%02.2d:%02.2d";

  // apply the format
  int chars_written = unitex::u_sprintf(buffer, kStrHourFormat,
                                        timeinfo_p->tm_hour,
                                        timeinfo_p->tm_min,
                                        timeinfo_p->tm_sec);
  // only if u_sprintf success
  return chars_written > 0 ? buffer : unitex::u_strcpy(buffer, kDummyHour);
}  // time_aschour_r

/**
 * @def    time_now
 * @brief  obtain the current time since the Epoch
 *
 * Obtain the current time, expressed as seconds and microseconds
 * since the Epoch, and store it in the timeval structure pointed
 * to by timeval
 *
 * @return 0 for success, or -1 for failure (no errno is set)
 */
#if    UNITEX_OS_IS(UNIX)
UNITEX_FORCE_INLINE
int time_now(struct timeval* tv, void* tz /* (unused) */) {
  UNITEX_DISCARD_UNUSED_PARAMETER(tz);

  // We try to use the best precision :
  // (function)     : (precision)
  // clock_gettime  : nanoseconds  (not widely available)
  // clock_get_time : nanoseconds  (only in MACH kernels)
  // gettimeofday   : microseconds (default, if clock_gettime isn't avalaible)
  // ftime          : milliseconds (obsolete, not used here)
  // time           : seconds      (microseconds will be always set to 0)
  // Note that for UNIX the use of the timezone struct is obsolete

  // TODO(martinec) add -lrt to the list of libraries we link
# if UNITEX_HAVE(CLOCK_GETTIME)
   // using clock_gettime function
   static bool use_clock_gettime = true;
   if(use_clock_gettime) {
     struct timespec ts;

     int ts_retval = clock_gettime(CLOCK_REALTIME, &ts);

     if(ts_retval == 0) { // clock_gettime returns 0 for success
       // First copy the seconds field
       tv->tv_sec  = ts.tv_sec;
       // then convert nanoseconds to microseconds (1 ns are 1/1000 ms)
       tv->tv_usec = ts.tv_nsec / TIME_UNIT_C(1000);
       // return success
       return 0;
     } else {
       use_clock_gettime = false;
     }
   }
   // if ts_retval != 0 we continue to try with other function
# endif

# if defined(UNITEX_UNIX_FEATURE_MACH_CLOCK_GETTIME)
  // using clock_get_time function
  /*
   * For more details about Clock Objects in Mac OS X and iOS
   * @see "Mac OS X and iOS Internals: To the Apple's Core",
   * by Jonathan Levin. John Wesley & Sons, Inc, ISBN 1-118-05765-1
   * Chapter 10, page 378-380.
  */
   static bool use_clock_get_time = true;
   if(use_clock_get_time) {
     static detail::helper::MachClockService clock_service;

     if(clock_service.initialized()){
       mach_timespec_t mach_current_time;

       // clock_get_time gets the current time
       kern_return_t kn_retval = clock_get_time(clock_service.get(),
                                                &mach_current_time);

       if (kn_retval == KERN_SUCCESS) {
         // First copy the seconds field
         tv->tv_sec  = mach_current_time.tv_sec;
         // then convert nanoseconds to microseconds (1 ns are 1/1000 ms)
         tv->tv_usec = mach_current_time.tv_nsec / 1000;
         // return success
         return 0;
       } else {
         use_clock_get_time = false;
       }  // kn_retval != KERN_SUCCESS
     }    // MachClockService::initialized()
   }      // use_clock_get_time
   // if kn_retval != KERN_SUCCESS we continue to try with other function
# endif

  // using gettimeofday function
  // Note that the latest Linux versions will implement
  // gettimeofday in userspace
  static bool use_gettimeofday = true;
  if(use_gettimeofday) {
    int tv_retval = gettimeofday(tv, NULL);

    if(tv_retval == 0) {
       // return success
       return 0;
    } else {
      use_gettimeofday = false;
    }
  }

  // using time function
  static bool use_time = true;
  if(use_time) {
    time_t current_time;
    // the time since 01/01/1970 00:00:00 (UTC)
    current_time = time(NULL);

    if(current_time > 0){
      // copy the seconds field
      tv->tv_sec  = current_time;
      // time gives only seconds precision
      tv->tv_usec = 0;
      // return success
      return 0;
    } else {
      use_time = false;
    }
  }

  // return failure
  return -1;
}  // time_now
#elif  UNITEX_OS_IS(WINDOWS)
UNITEX_FORCE_INLINE
int time_now(struct timeval* tv, void* tz /* (unused) */) {
  UNITEX_DISCARD_UNUSED_PARAMETER(tz);

  // We try to use the best precision
  // GetSystemTimePreciseAsFileTime : highest possible level of precision (<1us)
  // GetSystemTimeAsFileTime        : precision depends on the system
  // Note that the use of the timezone struct is obsolete (UNIX)

  // The reference date (Epoch) in Windows-based systems starts at
  // 01/01/1601 00:00:00, in Unix-like systems starts at 01/01/1970
  // 00:00:00. Since FILETIME structure uses 100ns intervals, to
  // adjust the time we need 116444736000000000 100-nanosecond units
  // between 1601 and 1970.
  static const time_unit_t kEPOCH = TIME_UNIT_C(116444736000000000);

  FILETIME file_time;         // Contains a 64-bit value representing the number
                              // of 100-nanosecond intervals since January 1,
                              // 1601 (UTC)

  ULARGE_INTEGER ularge_int;  // Union to represent a 64-bit unsigned integer
                              // value. Recommended to perform operations with
                              // FILETIME structure (MSDN)

# if defined(UNITEX_WIN32_FEATURE_TIME_PRECISE)
  // Retrieves the current system date and time with the highest possible
  // level of precision (<1us)
  GetSystemTimePreciseAsFileTime(&file_time);
# else
  // Retrieves the current system date and time, the level of precision
  // depends on the system
  GetSystemTimeAsFileTime(&file_time);
# endif

  // Copy the low- and high-order parts of the file_time to ularge_int
  ularge_int.LowPart  = file_time.dwLowDateTime;
  ularge_int.HighPart = file_time.dwHighDateTime;

  // epoch difference in 100-ns intervals
  time_unit_t
  epoch_delta = static_cast<time_unit_t>(ularge_int.QuadPart) - kEPOCH;

  // First convert the epoch_delta to seconds (100-ns are 10^-7 s)
  tv->tv_sec  = (epoch_delta / TIME_UNIT_C(10000000));
  // and then the remainder to microseconds (100-ns are 1/10 ms)
  // note than the remainder is always by definition < 10000000 100-ns
  tv->tv_usec = (epoch_delta % TIME_UNIT_C(10000000)) / TIME_UNIT_C(10);

  return 0;
}
#endif  // UNITEX_OS_IS(WINDOWS)

#if    UNITEX_OS_IS(UNIX)
//template <typename T>
UNITEX_FORCE_INLINE
timeinfo_t* time_localtime_r (const time_t* time_p,
                              timeinfo_t* result) {
// TODO(martinec) actually this guard isn't related to any configuration check
#if UNITEX_HAVE(LOCALTIME_R)
  return localtime_r(time_p, result);
#else
  // TODO(martinec) we need here a true alternativeto to localtime_r
  const timeinfo_t* local_result = ::localtime(time_p);  // NOLINT
  if(local_result) {
    memcpy(result, local_result, sizeof(timeinfo_t));
  } else {
    memset(result, 0, sizeof(timeinfo_t));
    return NULL;
  }
  return result;
#endif  // HAVE_LOCALTIME_R
}

UNITEX_FORCE_INLINE
timeinfo_t* time_localtime64_r (const time_unit_t* time_p,
                                timeinfo_t* result) {
  if(*time_p <= TIME_T_MAX && *time_p >= TIME_T_MIN){
    time_t time_small_enough = *time_p;
    return time_localtime_r(&time_small_enough, result);
  } else {
    // TODO(martinec) we need here a true Unix-system's localtime64 version !
    // Y2K38BUG
  }
  return NULL;
}
#endif  //UNITEX_OS_IS(UNIX)

/**
 * @def    localtime_safe
 * @brief  safe localtime implementation
 *
 * Uses the value pointed by time_p to fill a timeinfo_t structure with
 * the values that represent the corresponding time, expressed for the
 * local timezone
 *
 * @return A pointer to a time_p structure with its members filled with
 *         the values that correspond to the local time representation
 *         of timer
 */
#if    UNITEX_OS_IS(UNIX)
UNITEX_FORCE_INLINE
timeinfo_t* localtime_safe(const time_unit_t* time_p,
                           timeinfo_t* result) {
  return time_localtime64_r(time_p, result);
}
#elif  UNITEX_OS_IS(WINDOWS)
UNITEX_FORCE_INLINE
timeinfo_t* localtime_safe(const time_unit_t* time_p,
                           timeinfo_t* result) {
  _localtime64_s(result, time_p);
  return result;
}
#endif  // UNITEX_OS_IS(WINDOWS)
/* ************************************************************************** */
}   // namespace unitex::{unnamed}, to enforce the one-definition-rule
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_TIME_FUNCTIONS_H_                            // NOLINT
