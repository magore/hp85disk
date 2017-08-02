/**
 @file lifsup.c

 @brief  LIF file utilities - utilities extracted from hp85disk project for stand alone use

 @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

*/

#ifdef LIF_STAND_ALONE

#include "lifsup.h"
#include "lifutils.h"

int debuglevel = 0x0001;

#ifdef __MINGW32__
struct tm *gmtime_r(const time_t *timep, struct tm *result)
{

    struct tm *g = gmtime (timep );
    *result = *g;
    return(result);
}

char *asctime_r(const struct tm *tm, char *buf)
{
  static char day_name[7][3] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static char mon_name[12][3] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  sprintf (buf, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
       day_name[tm->tm_wday], 
       mon_name[tm->tm_mon],
       tm->tm_mday, tm->tm_hour, tm->tm_min,
       tm->tm_sec, 1900 + tm->tm_year);
  return buf;
}

struct tm *_gmtime64 (const __time64_t *);

time_t timegm(struct tm * a_tm)
{
    time_t ltime = mktime(a_tm);
    struct tm *tm_val =_gmtime64 (&ltime);
    int offset = (tm_val->tm_hour - a_tm->tm_hour);
    if (offset > 12)
    {
        offset = 24 - offset;
    }
    time_t utc = mktime(a_tm) - offset * 3600;
    return utc;
}

#endif


///@brief Match two strings and compare argument index
/// Display  message if the number of arguments is too few
///@param str: string to test
///@param pat: pattern to match
///@param min: minumum number or arguments
///@param argc: actual number of arguments
///@return 1 on match, 0 on no match or too few arguments
int MATCHARGS(char *str, char *pat, int min, int argc)
{
    if(strcmp(str,pat) == 0)
    {
        if(argc >= min)
            return(1);
        else
            printf("%s expected %d arguments only got %d\n", pat, min,argc);
    }
    return(0);
}


///@brief Remove white space at the end of lines
///@param str: string
///@return void
MEMSPACE
void trim_tail(char *str)
{
    int len = strlen(str);
    while(len--)
    {
        if(str[len] > ' ')
            break;
        str[len] = 0;
    }
}


///@brief Convert Value into byte array
/// bytes are MSB ... LSB order
///@param B: byte array
///@param index: offset into byte array
///@param size: number of bytes to process
///@param val: Value to convert
///@return void
MEMSPACE
void V2B_MSB(uint8_t *B, int index, int size, uint32_t val)
{
    int i;
    for(i=size-1;i>=0;--i)
    {
        B[index+i] = val & 0xff;
        val >>= 8;
    }
}
// =============================================
///@brief Convert Value into byte array
/// bytes are LSB ... MSB order
///@param B: byte array
///@param index: offset into byte array
///@param size: number of bytes to process
///@param val: Value to convert
///@return void
MEMSPACE
void V2B_LSB(uint8_t *B, int index, int size, uint32_t val)
{
    int i;
    for(i=0;i<size;++i)
    {
        B[index+i] = val & 0xff;
        val >>= 8;
    }
}



///@brief Convert a byte array into a value
/// bytes are MSB ... LSB order
///@param B: byte array
///@param index: offset into byte array
///@param size: number of bytes to process
///@return value
MEMSPACE
uint32_t B2V_MSB(uint8_t *B, int index, int size)
{
    int i;
    uint32_t val = 0;

    for(i=0;i<size;++i)
    {
        val <<= 8;
        val |= (uint8_t) (B[i+index] & 0xff);
    }
        return(val);
}

///@brief Convert a byte array into a value
/// bytes are LSB ... MSB order
///@param B: byte array
///@param index: offset into byte array
///@param size: number of bytes to process
///@return value
MEMSPACE
uint32_t B2V_LSB(uint8_t *B, int index, int size)
{
    int i;
    uint32_t val = 0;

    for(i=size-1;i>=0;--i)
    {
        val <<= 8;
        val |= (uint8_t) (B[i+index] & 0xff);
    }
        return(val);
}

/// @brief hex listing of data
/// @param[in] *data: date to dump
/// @param[in] size: size of data to dump
/// @retrun void
MEMSPACE
void hexdump(uint8_t *data, int size)
{
    long addr;
    int i,len;

    addr = 0;
    while(size > 0)
    {
        printf("%08lx : ", addr);
        len = size > 16 ? 16 : size;

        for(i=0;i<len;++i)
            printf("%02x ",0xff & data[addr+i]);

        for(;i<16;++i) 
            printf("   ");

        printf(" : ");
        for(i=0;i<len;++i)
        {
            if(data[addr+i] >= 0x20 && data[addr+i] <= 0x7e)
                putchar(data[addr+i]);
            else
                putchar('.');
        }
        for(;i<16;++i)
            putchar('.');

        printf("\n");

        addr += len;
        size -= len;
    }
    printf("\n");
}



void td02lif_usage(char *name)
{
        printf("Stand alone version of LIF utilities for linux\n");
        printf("HP85 Disk and Device Emulator\n");
        printf(" (c) 2014-2017 by Mike Gore\n");
        printf(" GNU version 3\n");
        printf("-> https://github.com/magore/hp85disk\n");
        printf("   GIT last pushed:   %s\n", GIT_VERSION);
        printf("   Last updated file: %s\n", LOCAL_MOD);
        printf("\n");
        printf("Usage: td02lif file.td0 file.lif\n");
}

int main(int argc, char *argv[])
{
    char *myargv[10];
    int i;
    int myargc;
    int ind = 0;

    myargv[ind++] = argv[0];
    // in stand alone mode we automatically set the 2nd argument to "lif"
    
    myargv[ind++] = "lif";

    // If we used no arguments the set default help
    if(argc < 2)
    {
        myargv[ind++] = "help";
        td02lif_usage(argv[0]);
        printf(" - OR -\n");
        printf("\n");
    }
    else
    {
        if(MATCH(basename(argv[0]),"td02lif"))
            myargv[ind++] = "td02lif";
        //@brief MingW
        if(MATCH(basename(argv[0]),"td02lif.exe"))
            myargv[ind++] = "td02lif";
    }

    for(i=1;i<argc;++i)
        myargv[ind++] = argv[i];

    myargc = ind;
    for(i=ind;i<10;++i)
        myargv[i] = NULL;

    if(!lif_tests(myargc,myargv))
        td02lif_usage(argv[0]);
}

#endif
