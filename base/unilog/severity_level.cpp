/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      severity_level.cpp
 * @brief     Logger severity levels class
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 severity_level.cpp`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
// Header for this file
#include "base/unilog/severity_level.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
#define Initialize__SeverityLevel__(Instance, Name, Alias)  \
  const SeverityLevel SeverityLevel::Instance(              \
        SeverityLevel::constant::Instance);                 \
  const char*  SeverityLevel::Instance::name  =   Name;     \
  const char*  SeverityLevel::Instance::alias =   Alias
/* ************************************************************************** */
Initialize__SeverityLevel__(Trace,    "trace",    "++"); // detailed trace of execution
Initialize__SeverityLevel__(Debug,    "debug",    "%%"); // debug-level messages
Initialize__SeverityLevel__(Info,     "info",     "II"); // informational messages
Initialize__SeverityLevel__(Notice,   "notice",   "!!"); // normal but significant condition
Initialize__SeverityLevel__(Warning,  "warning",  "WW"); // warning conditions
Initialize__SeverityLevel__(Error,    "error",    "EE"); // error conditions
Initialize__SeverityLevel__(Critical, "critical", "CC"); // critical conditions
Initialize__SeverityLevel__(Alert,    "alert",    "^^"); // action must be taken immediately
Initialize__SeverityLevel__(Panic,    "panic",    "@@"); // system is unusable
Initialize__SeverityLevel__(None,     "none",       ""); // not a message logging
/* ************************************************************************** */
#undef Initialize__SeverityLevel__
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
