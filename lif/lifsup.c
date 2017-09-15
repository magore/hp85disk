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

/// @brief Create a string from data that has no EOS but known size
/// @param[in] *B: source
/// @param[in] index: index offset into source data
/// @param[out] *name: target string
/// @param[in] size: size of string to write - not including EOS
/// @return void
void B2S(uint8_t *B, int index, uint8_t *name, int size)
{
    int i;
    for(i=0;i<size;++i)
        name[i] = B[index+i];
    name[i] = 0;
}


///@brief Set bit in vector
///@param[out] p: vector
///@param[in] bit: bit number to set
///@return void
void BITSET_LSB(uint8_t *p, int bit)
{
    int index = bit >> 3;
    bit &= 0x07;
    p[index] |= (1 << bit);
}

///@brief Clear bit in vector
///@param[out] p: vector
///@param[in] bit: bit number to clear
///@return void
void BITCLR_LSB(uint8_t *p, int bit)
{
    int index = bit >> 3;
    bit &= 0x07;
    p[index] &= ~(1 << bit);
}

///@brief Test bit in vector
///@param[out] p: vector
///@param[in] bit: bit number to test
///@return 1 if set, 0 if not
int BITTST_LSB(uint8_t *p, int bit)
{
    int index = bit >> 3;
    bit &= 0x07;
    return( (p[index] & (1 << bit)) ? (int) 1 : (int) 0 );
}
    
/// @brief Compute CRC16 of 8bit data
/// @see https://en.wikipedia.org/wiki/Cyclic_redundancy_check
/// FYI normal CRC16 typically use 0x1021 for ploy
/// Note: You can do a CRC16 of data in blocks by passing the result
/// as the crc initial value for the next call
/// @param[in] *B:      8 bit binary data
/// @param[in] crc: initial crc value
/// @param[out] poly:   ploynomial
/// @param[in] size:    number of bytes
/// @return crc16 of result
uint16_t crc16(uint8_t *B, uint16_t crc, uint16_t poly, int size)
{
    int i,bit;
    for(i=0; i<size; ++i)
    {
        crc ^= (0xff00 & ((uint16_t)B[i] << 8));
        // Loop for 8 bits per byte
        for (bit = 0; bit < 8; bit++)
        {
            if ((crc & 0x8000) != 0) 
                crc = (uint16_t) ((crc << 1) ^ poly);
            else
                crc <<= 1;
        }
    }
    return (crc);
}


/// @brief hex listing of data
/// @param[in] *data: date to dump
/// @param[in] size: size of data to dump
/// @retrun void
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

///@brief Display Copyright
///@return void
void copyright()
{
    printf("Stand alone version of LIF/TELEDISK utilities for linux\n");
    printf("HP85 Disk and Device Emulator\n");
    printf(" (c) 2014-2017 by Mike Gore\n");
    printf(" GNU version 3\n");
    printf("-> https://github.com/magore/hp85disk\n");
    printf("   GIT last pushed:   %s\n", GIT_VERSION);
    printf("   Last updated file: %s\n", LOCAL_MOD);
    printf("\n");
}

int main(int argc, char *argv[])
{

    int i;
    char *ptr;

    if(argc <= 1)
    {
        copyright();
    }

    if( MATCH(basename(argv[0]),"lif") || MATCH(basename(argv[0]),"lif.exe") )
    {
        argv[0] = "lif";
        return( lif_tests(argc, argv) );
    }
#ifdef TELEDISK
    if( MATCH(basename(argv[0]),"td02lif") || MATCH(basename(argv[0]),"td02lif.exe") )
    {
        argv[0] = "td02lif";
        return ( td02lif(argc, argv) );
    }
#endif

    return(0);
}

#endif
