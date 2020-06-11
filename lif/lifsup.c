/**
 @file lifsup.c

 @brief  LIF file utilities - utilities extracted from hp85disk project for stand alone use

 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

*/

#ifdef LIF_STAND_ALONE

#include <unistd.h>

#include "user_config.h"

#include "lifsup.h"
#include "lifutils.h"
#include "td02lif.h"

#include "../lib/parsing.c"
#include "../gpib/vector.c"
#include "../gpib/drives_sup.c"

int debuglevel = 0x0001;

#ifdef __MINGW32__
struct tm *gmtime_r(const time_t *timep, struct tm *result)
{

    struct tm *g = gmtime (timep );
    *result = *g;
    return(result);
}


///@brief Short Name of each Day in a week.
///
/// - Day 0 .. 6 to string.
///
///@see asctime_r()
const char *__WDay[] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat","BAD"};

/// @brief Short Name or each Month in a year.
///
/// - Month 0 .. 11 to string.
///
const char *__Month[]= \
{ \
    "Jan","Feb","Mar","Apr","May","Jun","Jul", \
        "Aug","Sep","Oct","Nov","Dec","BAD"
};

/// @brief Convert tm_t *t structure into POSIX asctime() ASCII string *buf.
///
/// @param[in] t: tm_t structure pointer.
/// @param[out] buf: user buffer for POSIX asctime() string result.
/// - Example output: "Thu Dec  8 21:45:05 EST 2011".
///
/// @return buf string pointer.
char *asctime_r(const struct tm *t, char *buf)
{
// normaize t before output
//(void) normalize(t,0);

    memset(buf,0,32);
    snprintf(buf,32,"%s %s %2d %02d:%02d:%02d %4d",
        __WDay[t->tm_wday % 7],
        __Month[t->tm_mon % 12],
        (int)t->tm_mday,
        (int)t->tm_hour,
        (int)t->tm_min,
        (int)t->tm_sec,
        (int)t->tm_year + 1900);
    return(buf);
}


struct tm *_gmtime64 (const __time64_t *);

time_t timegm(struct tm * a_tm)
{
    const __time64_t ltime = mktime(a_tm);
    struct tm *tm_val =_gmtime64 ((const __time64_t *) &ltime);
    int offset = (tm_val->tm_hour - a_tm->tm_hour);
    if (offset > 12)
    {
        offset = 24 - offset;
    }
    time_t utc = mktime(a_tm) - offset * 3600;
    return utc;
}
#endif

///@brief Display Copyright
///@return void
void copyright()
{
    printf("Stand alone version of LIF/TELEDISK utilities for linux\n");
    printf("HP85 Disk and Device Emulator\n");
    printf(" (c) 2014-2020 by Mike Gore\n");
    printf(" GNU version 3\n");
    printf("-> https://github.com/magore/hp85disk\n");
    printf("   GIT last pushed:   %s\n", GIT_VERSION);
    printf("   Last updated file: %s\n", LOCAL_MOD);
    printf("\n");
}


int main(int argc, char *argv[])
{

    int i;
    int verbose = 0;
    char *ptr;

    if( MATCHI(basename(argv[0]),"lif") || MATCHI(basename(argv[0]),"lif.exe") )
    {
        argv[0] = "lif";
		if(argc <= 1)
		{
			copyright();
			lif_help(1);
			return(0);
		}
        return( !lif_tests(argc, argv) );
    }
    if( MATCHI(basename(argv[0]),"td02lif") || MATCHI(basename(argv[0]),"td02lif.exe") )
    {
#ifdef TELEDISK
        argv[0] = "td02lif";
		if(argc <= 1)
		{
			copyright();
			td0_help(1);
			return(0);
		}
        return ( !td02lif(argc, argv) );
#else
		printf("td02lif not enabled\n");
#endif
    }
}
#endif
