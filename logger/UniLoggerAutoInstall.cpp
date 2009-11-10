 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


#if ((!(defined(NO_UNITEX_LOGGER))) && (!(defined(NO_UNITEX_LOGGER_AUTOINSTALL))))

#include <string.h>
#include <stdlib.h>
#include <memory.h>

#include "File.h"

#include "ActivityLogger.h"
#include "ActivityLoggerPlugCallback.h"

#include "UniLogger.h"


class InstallLogger
{
public:
    InstallLogger();
    ~InstallLogger();
private:
    struct UniLoggerSpace ule;
    int init_done;
};


InstallLogger::InstallLogger()
{
    init_done = 0;
    
    ABSTRACTFILE *af_fin = af_fopen_unlogged("unitex_logging_parameters.txt","rb");
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
            
            int write_file_out=0;
            char*szPath = (char*)malloc(size_param+1);
            *szPath=0;
            sscanf(param,"%s\n%u",szPath,&write_file_out);
            write_file_out+=0;
            if ((*szPath) != 0)
            {
                ule.privateUnloggerData = NULL;
                ule.szPathLog = szPath;
                ule.store_file_out_content = write_file_out;
                ule.store_list_file_out_content = 1;

                ule.store_file_in_content = 1;
                ule.store_list_file_in_content = 1;

                ule.auto_increment_logfilename = 1;

                if (AddActivityLogger(&ule) != 0)
                    init_done = 1;
            }
            else
                free(szPath);
        }
        af_fclose(af_fin);
        free(param);        
    }
}

InstallLogger::~InstallLogger()
{
    if (init_done != 0)
    {
        RemoveActivityLogger(&ule);
        free(ule.szPathLog);
        ule.szPathLog=NULL;
    }
}

InstallLogger InstallLoggerInstance;

#endif
