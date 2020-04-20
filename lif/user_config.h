#ifdef LIF_STAND_ALONE

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>
#include <libgen.h>
#include <sys/types.h>
#include <utime.h>

#define MEMSPACE /**/
#define WEAK_ATR /**/
typedef struct tm tm_t;
#define safecalloc(a,b) calloc(a,b)
#define safefree(a) free(a)
#define sync()

#include "../lib/parsing.h"
#include "../gpib/vector.h"
#include "../gpib/drives_sup.h"
#include "../gpib/debug.h"

#endif
