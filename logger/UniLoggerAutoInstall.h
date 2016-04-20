/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {

extern const char* usage_CreateLog;
extern const char* optstring_CreateLog;
extern const struct option_TS lopts_CreateLog[];

class InstallLogger
{
public:
    InstallLogger(const char* parameter_filename=NULL);
    InstallLogger(int argc,char* const argv[]);
    ~InstallLogger();
private:
    struct UniLoggerSpace ule;
    int init_done;
    void LoadParamFile(const char*parameter_filename);
    void ClearUniLoggerSpaceStruct(int clear_memory);
};

} // namespace logger
} // namespace unitex





#ifdef __cplusplus
extern "C" {
#endif

    struct _INSTALL_LOGGER {
        void* dummy;
    };
    typedef struct _INSTALL_LOGGER* INSTALLLOGGER;

    UNITEX_FUNC INSTALLLOGGER UNITEX_CALL BuildLogger();
    UNITEX_FUNC INSTALLLOGGER UNITEX_CALL BuildLoggerFromArgs(int argc, char* const argv[]);
    UNITEX_FUNC INSTALLLOGGER UNITEX_CALL BuildLoggerFromParamFile(const char* paramFileName);
    UNITEX_FUNC void UNITEX_CALL RemoveLoggerFromParamFile(INSTALLLOGGER);

#ifdef __cplusplus
}
#endif



#endif
