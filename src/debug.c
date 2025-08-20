#include "sox_config.h"




#if FULL_STD_CLIB

#else
    void __aeabi_assert (const char *expr, const char *file, int line) {
        while(1);
    }
#endif
