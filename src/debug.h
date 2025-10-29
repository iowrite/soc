#ifndef _SOX_DEBUG_H
#define _SOX_DEBUG_H



#if SOX_DEBUG && FULL_STD_CLIB
    #include <stdio.h>
    #define DEBUG_LOG(fmt, ...) \
        printf("[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(fmt, ...)
#endif




#endif // _SOX_DEBUG_H

