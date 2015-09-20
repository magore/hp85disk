/**
 @file hardware/rtc.c

 @brief DS1307 RTC Driver AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#include "user_config.h"
#include "lib/time.h"

/// @brief Convert number >= 0 and <= 99 to BCD.
///
///  - BCD format has each hex nibble has a digit 0 .. 9
///
/// @param[in] data: number to convert.
/// @return  BCD value
/// @warning we assume the number is in range.
uint8_t BINtoBCD(uint8_t data)
{
    return(  ( (data/10U) << 4 ) | (data%10U) );
}


/// @brief Convert two "digit" BCD number to binary.
///
/// @param[in] data: BCD number.
///  - number >= 0 and <= 99 to BCD
/// @return  Binary value.
uint8_t BCDtoBIN(uint8_t data)
{
    return  ( ( ( (data&0xf0U) >> 4) * 10 ) + (data&0x0fU) );
}



/// @brief Check if the DS1307 device is running.
///
/// - display error if read error occurs.
///
/// @return 1 if running.
/// @return 0 if not running.
int8_t rtc_run_test()
{
    uint8_t  ReadAddress;
    uint8_t b = 0;

    ReadAddress = 0;
    if (TWI_ReadPacket(DS1307_R, 20, &ReadAddress, sizeof(ReadAddress),
        (uint8_t*)&b, 1) != TWI_ERROR_NoError)
    {
        printf("rtc_state read error\n");
        return -1;
    }
    if(b & 8)
        return 0;
    return 1;
}


/// @brief Set DS1307 run state.
/// @param[in] run state.
///  - 1 = run.
///  - 0 = stop.
///  - -1 = check run state.
///
/// @return  run state 0/1 on success.
/// @return -1 on error.
int rtc_run(int run)
{
    uint8_t  WriteAddress;
    uint8_t  ReadAddress;
    uint8_t b = 0;

    ReadAddress = 0;
    if (TWI_ReadPacket(DS1307_R, 20, &ReadAddress, sizeof(ReadAddress),
        (uint8_t*)&b, 1) != TWI_ERROR_NoError)
    {
        printf("rtc_run read error\n");
        return -1;
    }

    if(run == -1)
        return ((b & 0x80) ? 0 : 1);

    b = ( b  & 0x7f) | (run ? 0 : 0x80);

    WriteAddress = 0;
    if (TWI_WritePacket(DS1307_W, 20, &WriteAddress, sizeof(WriteAddress),
        (uint8_t*)&b, 1) != TWI_ERROR_NoError)
    {
        printf("rtc_run - write error\n");
        return(-1);
    }
    return(run);
}


/// @brief Initialize DS1307 rtc if not initialied - or if forced.
///
/// @param[in] force: force initialiation flag.
/// - If 1 then alwasy force initialiation.
/// @param[in] seconds: POSIX EPOCH time in seconds.
///
/// @return  1 on success.
/// @return 0 on fail.
uint8_t rtc_init (int force, time_t seconds)
{
    uint8_t buf[8];                               /* RTC R/W buffer */
    uint8_t addr;
    uint8_t  WriteAddress;
    int8_t    state;

    tm_t *tmp;

    TWI_Init(TWI_BIT_PRESCALE_4, TWI_BITLENGTH_FROM_FREQ(4, 50000));

    if(!force)
    {
        state = rtc_run(-1);
        if(state < 0)
        {
            rtc_ok = 0;
            return 0;
        }
        if(state == 0)                            // stopped
            force = 1;
    }

    if(force)                                     // INIT
    {
        if(rtc_run(0) < 0)                        // STOP RTC
        {
            rtc_ok = 0;                           // Fail
            return 0;
        }
        tmp = gmtime(&seconds);
        if(!rtc_write(tmp))
        {
            printf("rtc _write epoch failed\n");
            rtc_ok = 0;
            return 0;
        }

        memset(buf, 0, 8);
        for (addr = 8; addr < 0x3f; addr += 8)
        {
            WriteAddress = addr;
            if (TWI_WritePacket(DS1307_W, 20, &WriteAddress, sizeof(WriteAddress),
                (uint8_t*)buf, 8) != TWI_ERROR_NoError)
            {
                printf("rtc_init ram - write error\n");
                return(0);
            }
        }

        if(rtc_run(1) < 0)                        // START RTC
        {
            rtc_ok = 0;                           // Fail
            return 0;
        }
    }
    rtc_ok = 1;
    return 1;
}



/// @brief  Set DS1307 RTC from POSIX struct tm * structure.
///
/// @param[in] t: POSIX struct tm * time to set.
///
/// @return 1 on sucess.
/// @return 0 on fail.
uint8_t rtc_write(tm_t *t)
{
    uint8_t buf[8];
    uint8_t WriteAddress;

    buf[0] =  BINtoBCD(t->tm_sec) & 0x7f;
    buf[1] =  BINtoBCD(t->tm_min) & 0x7f;
    buf[2] =  BINtoBCD(t->tm_hour) & 0x3f;
    buf[3] =  ((t->tm_wday & 7) + 1) & 0x0f;
    buf[4] =  BINtoBCD(t->tm_mday ) & 0x3f;
    buf[5] =  BINtoBCD(t->tm_mon + 1) & 0x1f;
    buf[6] =  BINtoBCD(t->tm_year - 100) & 0xff;  // 2000 = 0
    buf[7] = 0x93;                                // 32khz, out square wave

#ifdef RTC_DEBUG
    printf("rtc_write():%d, day:%d,mon:%d,hour:%d,min:%d,sec:%d, wday:%d\n",
        t->tm_year + 1900,
        t->tm_mon,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec,
        t->tm_wday);
#endif

#ifdef RTC_DEBUG
    printf("%4x\n", t);
    int i;
    printf("rtc_write buf: ");
	for(i=0;i<7;++i)
		printf("%02x ", 0xff & buf[i]);
    printf("\n");
#endif

    WriteAddress = 0;
    if (TWI_WritePacket(DS1307_W, 20, &WriteAddress, sizeof(WriteAddress),
        (uint8_t*)buf, 8) != TWI_ERROR_NoError)
    {
        printf("rtc_write error\n");
        return(0);
    }

    return(1);
}



/// @brief  Read DS1307 RTC into POSIX struct tm * structure.
///
/// @param[out] t: struct tm * POSIX time returned.
/// @return  1 on sucess.
/// @return 0 on fail.
uint8_t rtc_read(tm_t *t)
{
    uint8_t buf[8];

    uint8_t ReadAddress = 0;

    ReadAddress = 0;
    if (TWI_ReadPacket(DS1307_R, 20, &ReadAddress, sizeof(ReadAddress),
        (uint8_t*)buf, 8) != TWI_ERROR_NoError)
    {
        printf("rtc_read error\n");
        return 0;
    }

#ifdef RTC_DEBUG
    printf("%4x\n", t);
    int i;
    printf("rtc_read buf: ");
    for(i=0;i<7;++i)
        printf("%02x ", 0xff & buf[i]);
    printf("\n");
#endif

    t->tm_sec =   BCDtoBIN( buf[0] & 0x7f);
    t->tm_min =   BCDtoBIN( buf[1] & 0x7f);
    t->tm_hour =  BCDtoBIN( buf[2] & 0x3f);
    t->tm_wday =  ( buf[3] & 0x07) - 1;
    t->tm_mday =  BCDtoBIN( buf[4] & 0x3f) ;
    t->tm_mon=    BCDtoBIN( buf[5] & 0x1f) - 1;
    t->tm_year =  BCDtoBIN( buf[6] & 0xff) + 100;

#ifdef RTC_DEBUG
    printf("rtc_read():%d, day:%d,mon:%d,hour:%d,min:%d,sec:%d, wday:%d\n",
        t->tm_year + 1900,
        t->tm_mon,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec,
        t->tm_wday);
#endif

    return 1;
}


/// @brief FAT time structer reference.
/// @see rtc.h
/// @see http://lxr.free-electrons.com/source/fs/fat/misc.c
/// @verbatim
/// typedef struct
/// {
///     WORD   year;                                  /* 2000..2099 */
///     BYTE   month;                                 /* 1..12 */
///     BYTE   mday;                                  /* 1.. 31 */
///     BYTE   wday;                                  /* 1..7 */
///     BYTE   hour;                                  /* 0..23 */
///     BYTE   min;                                   /* 0..59 */
///     BYTE   sec;                                   /* 0..59 */
/// } RTC;
/// @endverbatim


#if 0
/// @brief Convert Linux POSIX tm_t * to FAT32 time.
///
/// @param[in] t: POSIX struct tm * to convert.
///
/// @return  FAT32 time.
uint32_t tm_to_fat(tm_t *t)
{
    uint32_t fat;
/* Pack date and time into a uint32_t variable */
    fat = ((uint32_t)(t->tm_year - 80) << 25)
        | (((uint32_t)t->tm_mon+1) << 21)
        | (((uint32_t)t->tm_mday) << 16)
        | ((uint32_t)t->tm_hour << 11)
        | ((uint32_t)t->tm_min << 5)
        | ((uint32_t)t->tm_sec >> 1);
    return(fat);
}
#endif


#if 0
/// @brief Read DS1307 RTC and convert to FAT32 time.
///
/// @return FAT32 time.
/// @see tm_to_fat().
uint32_t get_fattime (void)
{
    tm_t t;

/* Get local time */
    if(!rtc_read(&t))
        return(0);
    return( tm_to_fat(&t) );
}
#endif
