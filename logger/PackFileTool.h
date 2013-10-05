#ifndef NO_UNITEX_LOGGER



#ifndef _PACK_FILE_TOOLS_H
#define _PACK_FILE_TOOLS_H 1

#include "AbstractCallbackFuncModifier.h"

#ifdef __cplusplus
#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {
extern "C" {
#endif
 


int buildPackFile(const char* packFile,int append_status,const char* global_comment,
                  const char* file_or_prefix_to_add,int add_one_file_only,const char* junk_prefix,
				  int quiet);



UNITEX_FUNC int UNITEX_CALL CreateUnitexPackOneFile(const char* packFile,int append_status,
														const char* file_to_include, const char* junk_prefix,
														const char* global_comment);

UNITEX_FUNC int UNITEX_CALL CreateUnitexPackMultiFile(const char* packFile,int append_status,
														const char* file_to_include, const char* junk_prefix,
														const char* global_comment);


#ifdef __cplusplus
} // extern "C"
} // namespace logger
} // namespace unitex
#endif


 

 

#endif

#endif
