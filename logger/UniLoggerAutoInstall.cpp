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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS)
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.ergonotics.com/unitex-contribution/
 * contact : unitex-contribution@ergonotics.com
 *
 */
#if ((!(defined(NO_UNITEX_LOGGER))) && (!(defined(NO_UNITEX_LOGGER_AUTOINSTALL))))

#include <string.h>
#include <stdlib.h>
#include <memory.h>

#include "Unicode.h"
#include "Copyright.h"
#include "UnitexGetOpt.h"

#include "File.h"

#include "ActivityLogger.h"
#include "ActivityLoggerPlugCallback.h"

#include "UniLogger.h"
#include "UniLoggerAutoInstall.h"

#include "Copyright.h"
#include "Error.h"

#include "UnitexGetOpt.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {

void InstallLogger::LoadParamFile(const char* parameter_filename) {
    init_done = 0;

    ABSTRACTFILE *af_fin = af_fopen_unlogged((parameter_filename != NULL) ?
                                parameter_filename : "unitex_logging_parameters.txt","rb");
    if (af_fin!=NULL)
    {
        size_t size_param=0;

        if (af_fseek(af_fin, 0, SEEK_END) == 0)
        {
            size_param = af_ftell(af_fin);
            af_fseek(af_fin, 0, SEEK_SET);
        }

        char* param=(char*)malloc(size_param+1);
        *(param+size_param)=0;
        if (af_fread(param,1,size_param,af_fin) == size_param)
        {

            unsigned int write_file_out=0;
            char*szPath = (char*)malloc(size_param+1);
            *szPath=0;
            sscanf(param,"%s\n%u",szPath,&write_file_out);
            write_file_out+=0;
            if ((*szPath) != 0)
            {
                ClearUniLoggerSpaceStruct(0);

                ule.szPathLog = szPath;
                ule.szNameLog = NULL;
                ule.store_file_out_content = write_file_out;

                if (AddActivityLogger(&ule) != 0)
                    init_done = 1;
                else
                {
                    ClearUniLoggerSpaceStruct(1);
                }
            }
            else
                free(szPath);
        }
        af_fclose(af_fin);
        free(param);
    }
}

InstallLogger::InstallLogger(const char* parameter_filename) :
    ule(ule_default_init),init_done(0) {
    LoadParamFile(parameter_filename);
}

const char* usage_CreateLog =
         "Usage: CreateLog [OPTIONS]\n"
         "\n"
         "OPTIONS:\n"
         "  -g/--no_create_log: do not create any log file.\n"
         "                                   Incompatible with all others options.\n"
         "\n"
         "  -p XXX/--param_file=XXX: load a parameters file like unitex_logging_parameters.txt.\n"
         "                                   Incompatible with all others options.\n"
         "\n"
         "  -d XXX/--directory=XXX: location directory where log file to create.\n"
         "  -l XXX/--log_file=XXX: filename of log file to create.\n"
         "  -i/--store_input_file: store input file in log (default).\n"
         "  -n/--no_store_input_file: don't store input file in log (prevent rerun the logfile).\n"
         "  -o/--store_output_file: store output file in log.\n"
         "  -u/--no_store_output_file: don't store output file in log (default).\n"
         "  -s/--store_list_input_file: store list of input file in log (default).\n"
         "  -t/--no_store_list_input_file: don't store list of input file in log.\n"
         "  -r/--store_list_output_file: store list of output file in log (default).\n"
         "  -f/--no_store_list_output_file: don't store list of output file in log.\n"
         "\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n"
         "Dpecify option to create a the logfile on running tool.\n\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_CreateLog);
}


const char* optstring_CreateLog=":Vhgp:niofrtusk:q:l:d:";
const struct option_TS lopts_CreateLog[]= {
      {"only_verify_arguments",no_argument_TS,NULL,'V'},
      {"help",no_argument_TS,NULL,'h'},
      {"no_create_log",no_argument_TS,NULL,'g'},
      {"param_file",required_argument_TS,NULL,'p'},
      {"no_store_input_file",no_argument_TS,NULL,'n'},
      {"store_input_file",no_argument_TS,NULL,'i'},
      {"store_output_file",no_argument_TS,NULL,'o'},
      {"no_store_output_file",no_argument_TS,NULL,'u'},
      {"store_list_input_file",no_argument_TS,NULL,'s'},
      {"no_store_list_input_file",no_argument_TS,NULL,'t'},
      {"store_list_output_file",no_argument_TS,NULL,'r'},
      {"no_store_list_output_file",no_argument_TS,NULL,'f'},
      {"log_file",required_argument_TS,NULL,'l'},
      {"directory",required_argument_TS,NULL,'d'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {NULL,no_argument_TS,NULL,0}
};

InstallLogger::InstallLogger(int argc,char* const argv[]) :
  ule(ule_default_init), init_done(0) {
  ClearUniLoggerSpaceStruct(0);
  if (argc==1) {
    usage();
    return;
  }

  Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
  int bom_output = DEFAULT_BOM_OUTPUT;
  int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
  int val,index=-1;
  bool only_verify_arguments = false;
  UnitexGetOpt options;
  while (EOF!=(val=options.parse_long(argc,argv,optstring_CreateLog,lopts_CreateLog,&index))) {
     switch(val) {
       case 'V': only_verify_arguments = true;
                 break;
       case 'h': usage();
                 return;
       case 'n': ule.store_file_in_content = 0;
                 break;
       case 'i': ule.store_file_in_content = 1;
                 break;
       case 'o': ule.store_file_out_content = 1;
                 break;
       case 'u': ule.store_file_out_content = 0;
                 break;
       case 's': ule.store_list_file_in_content = 1;
                 break;
       case 't': ule.store_list_file_in_content = 0;
                 break;
       case 'r': ule.store_list_file_out_content = 1;
                 break;
       case 'f': ule.store_list_file_out_content = 0;
                 break;
       case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                             error("Missing argument for option --%s\n",lopts_CreateLog[index].name);
                 return;
       case 'k': if (options.vars()->optarg[0]=='\0') {
                    error("Empty input_encoding argument\n");
                    return;
                 }
                 decode_reading_encoding_parameter(&mask_encoding_compatibility_input,options.vars()->optarg);
                 break;
       case 'q': if (options.vars()->optarg[0]=='\0') {
                    error("Empty output_encoding argument\n");
                    return;
                 }
                 decode_writing_encoding_parameter(&encoding_output,&bom_output,options.vars()->optarg);
                 break;
       case 'g': ClearUniLoggerSpaceStruct(1);
                 return;
       case 'p': if (options.vars()->optarg[0]=='\0') {
                    error("You must specify a non empty param file\n");
                    return;
                 }
                 ClearUniLoggerSpaceStruct(1);
                 LoadParamFile(options.vars()->optarg);
                 return;
       case 'l': if (options.vars()->optarg[0]=='\0') {
                    error("You must specify a non empty log filename\n");
                    return;
                 }
                 if (ule.szNameLog != NULL) {
                     free((void*)ule.szNameLog);
                 }
                 ule.szNameLog = strdup(options.vars()->optarg);
                 break;

       case 'd': if (options.vars()->optarg[0]=='\0') {
                    error("You must specify a non empty directory\n");
                    return;
                 }
                 if (ule.szPathLog != NULL) {
                     free((void*)ule.szPathLog);
                 }
                 ule.szPathLog = strdup(options.vars()->optarg);
                 break;

       case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                             error("Invalid option --%s\n",options.vars()->optarg);
                 return;
     }
     index=-1;
  }

  if (options.vars()->optind!=argc-1) {
  }

  if (only_verify_arguments) {
    // freeing all allocated memory
    return;
  }

  if (AddActivityLogger(&ule) != 0) {
    init_done = 1;
  } else {
    ClearUniLoggerSpaceStruct(1);
  }
}

void InstallLogger::ClearUniLoggerSpaceStruct(int clear_memory)
{
if (clear_memory != 0) {
    if (ule.szPathLog != NULL) {
        free((char*)ule.szPathLog);
        ule.szPathLog = NULL;
    }
    if (ule.szNameLog != NULL) {
        free((char*)ule.szNameLog);
        ule.szNameLog = NULL;
    }
}

memset(&ule,0,sizeof(ule));
ule.size_of_struct = sizeof(ule);


ule.privateUnloggerData = NULL;
ule.szPathLog = NULL;
ule.szNameLog = NULL;
ule.store_file_out_content = 0;
ule.store_list_file_out_content = 1;

ule.store_file_in_content = 1;
ule.store_list_file_in_content = 1;

ule.store_std_out_content = ule.store_std_err_content = 1;

ule.auto_increment_logfilename = 1;
}

InstallLogger::~InstallLogger()
{
    if (init_done != 0)
    {
        RemoveActivityLogger(&ule);
        if (ule.szPathLog != NULL)
            free((void*)ule.szPathLog);
        ule.szPathLog=NULL;
        if (ule.szNameLog != NULL)
            free((void*)ule.szNameLog);
        ule.szNameLog=NULL;
    }
}


} // namespace logger
} // namespace unitex

using namespace unitex;
using namespace logger;

UNITEX_FUNC INSTALLLOGGER UNITEX_CALL BuildLogger()
{
    InstallLogger* pInstallLogger = new InstallLogger();
    return (INSTALLLOGGER)pInstallLogger;
}

UNITEX_FUNC INSTALLLOGGER UNITEX_CALL BuildLoggerFromArgs(int argc, char* const argv[])
{
    InstallLogger* pInstallLogger = new InstallLogger(argc,argv);
    return (INSTALLLOGGER)pInstallLogger;
}

UNITEX_FUNC INSTALLLOGGER UNITEX_CALL BuildLoggerFromParamFile(const char* paramFileName)
{
    InstallLogger* pInstallLogger = new InstallLogger(paramFileName);
    return (INSTALLLOGGER)pInstallLogger;
}

UNITEX_FUNC void UNITEX_CALL RemoveLoggerFromParamFile(INSTALLLOGGER logger)
{
    InstallLogger* pInstallLogger = (InstallLogger*)logger;
    if (pInstallLogger != NULL) {
        delete (pInstallLogger);
    }
}



#ifdef UNITEX_LOGGER_AUTOINSTALL_AUTOINSTANCE
InstallLogger InstallLoggerInstance;
#endif

#endif

