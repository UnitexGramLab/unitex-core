#ifndef NO_UNITEX_LOGGER



#ifndef _MZ_TOOLS_H
#define _MZ_TOOLS_H 1


#ifdef __cplusplus
namespace unitex {
namespace logger {
extern "C" {
#endif
int ulpRepair(const char*file, const char*fileOut, const char*fileOutTmp, uLong*nRecovered, uLong*bytesRecovered);

#ifdef __cplusplus
} // extern "C"
} // namespace logger
} // namespace unitex
#endif

#endif

#endif
