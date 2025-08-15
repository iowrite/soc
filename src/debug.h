#ifndef _SOX_DEBUG_H
#define _SOX_DEBUG_H



#if SOX_DEBUG && FULL_STD_CLIB
    #define DEBUG_LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(fmt, ...)
#endif





#endif // _SOX_DEBUG_H