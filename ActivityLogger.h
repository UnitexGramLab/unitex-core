 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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


#ifndef _ACTIVITY_LOGGER_H_INCLUDED
#define _ACTIVITY_LOGGER_H_INCLUDED 1


#include "Af_stdio.h"
#include "UnitexTool.h"

void Call_logger_fnc_before_af_fopen(const char* name,const char* MODE);
void Call_logger_fnc_after_af_fopen(const char* name,const char* MODE,ABSTRACTFILE*);

void Call_logger_fnc_before_af_fclose(ABSTRACTFILE*);
void Call_logger_fnc_after_af_fclose(ABSTRACTFILE*,int);

void Call_logger_fnc_before_af_rename(const char* name1,const char* name2);
void Call_logger_fnc_after_af_rename(const char* name1,const char* name2,int);

void Call_logger_fnc_before_af_copy(const char* name1,const char* name2);
void Call_logger_fnc_after_af_copy(const char* name1,const char* name2,int);

void Call_logger_fnc_before_af_remove(const char* name);
void Call_logger_fnc_after_af_remove(const char* name,int);

void Call_logger_fnc_before_calling_tool(mainFunc*,int argc,char* argv[]);
void Call_logger_fnc_after_calling_tool(mainFunc*,int argc,char* argv[],int);


void Call_logger_fnc_LogOutWrite(const void*Buf, size_t size);
void Call_logger_fnc_LogErrWrite(const void*Buf, size_t size);

#endif
