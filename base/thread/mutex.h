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
 * @file      mutex.h
 * @brief     Mutex wrapper
 *
 * @see       base/threads/model.h
 * @see       base/threads/lock_guard.h
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 mutex.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_THREAD_MUTEX_H_                                 // NOLINT
#define UNITEX_BASE_THREAD_MUTEX_H_                                 // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/compiler/attribute/inline.h"  // UNITEX_FORCE_INLINE
#include "base/macro/helpers.h"              // UNITEX_DISCARD_UNUSED_PARAMETER
#include "base/thread/model.h"               // ISO, BOOST, POSIX, WIN32, SINGLE
#include "base/thread/types.h"               // thread_id_t, mutex_t
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
/**
 * @def    mutex_lock
 * @brief  Locks the mutex, blocks if the mutex is not available
 * @see    mutex_unlock
 * @see    http://en.cppreference.com/w/cpp/thread/mutex
 */
#if   UNITEX_USE(ISO_THREADS)
UNITEX_FORCE_INLINE
void mutex_lock(unitex::mutex_t* mutex) {
  mutex->lock();
}
/* ************************************************************************** */
#elif UNITEX_USE(BOOST_THREADS)
UNITEX_FORCE_INLINE
void mutex_lock(unitex::mutex_t* mutex) {
  mutex->lock();
}
/* ************************************************************************** */
#elif UNITEX_USE(POSIX_THREADS)
UNITEX_FORCE_INLINE
void mutex_lock(unitex::mutex_t* mutex) {
  pthread_mutex_lock(mutex);
}
/* ************************************************************************** */
#elif UNITEX_USE(WIN32_THREADS)
UNITEX_FORCE_INLINE
void mutex_lock(unitex::mutex_t* mutex) {
  EnterCriticalSection(mutex->handle());
}
/* ************************************************************************** */
#elif UNITEX_USE(SINGLE_THREADED)
// performs no locking at all
UNITEX_FORCE_INLINE
void mutex_lock(unitex::mutex_t* mutex) {
  UNITEX_DISCARD_UNUSED_PARAMETER(mutex);
}
#endif  // UNITEX_USE(ISO_THREADS)
/* ************************************************************************** */
/**
 * @def    mutex_unlock
 * @brief  unlocks the mutex
 * @see    mutex_lock
 * @see    http://en.cppreference.com/w/cpp/thread/mutex
 */
#if   UNITEX_USE(ISO_THREADS)
UNITEX_FORCE_INLINE
void mutex_unlock(unitex::mutex_t* mutex) {
  mutex->unlock();
}
/* ************************************************************************** */
#elif UNITEX_USE(BOOST_THREADS)
UNITEX_FORCE_INLINE
void mutex_unlock(unitex::mutex_t* mutex) {
  mutex->unlock();
}
/* ************************************************************************** */
#elif UNITEX_USE(POSIX_THREADS)
UNITEX_FORCE_INLINE
void mutex_unlock(unitex::mutex_t* mutex) {
  pthread_mutex_unlock(mutex);
}
/* ************************************************************************** */
#elif UNITEX_USE(WIN32_THREADS)
UNITEX_FORCE_INLINE
void mutex_unlock(unitex::mutex_t* mutex) {
  LeaveCriticalSection(mutex->handle());
}
/* ************************************************************************** */
#elif UNITEX_USE(SINGLE_THREADED)
// performs no unlocking at all
UNITEX_FORCE_INLINE
void mutex_unlock(unitex::mutex_t* mutex) {
  UNITEX_DISCARD_UNUSED_PARAMETER(mutex);
}
#endif  // UNITEX_USE(ISO_THREADS)
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_THREAD_MUTEX_H_                              // NOLINT
