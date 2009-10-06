/*
 */

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
