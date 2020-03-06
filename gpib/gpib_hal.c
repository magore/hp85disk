/**
 @file gpib/gpib_hal.c
 
 @brief GPIB emulator hardwware layer for HP85 disk emulator project for AVR.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

*/


#include "gpib_hal.h"
#include "gpib.h"
#include "fatfs.h"
#include "posix.h"
#include "defines.h"

gpib_t gpib_timer;

/// @brief Install GPIB timers,  Elapsed time and Timeout tasks.
///
/// - Has some platform dependent code.
/// @return  void

void gpib_timer_init()
{
    if(set_timers(gpib_timer_task,1) == -1)       // Install Clock Task
        printf("GPIB Clock task init failed\n");

    gpib_timer_reset();
}




///@brief Parallel Poll Response bit mask.
static uint8_t _ppr_reg;

/// @brief  Reverse the bits in an 8 bit value 
///
/// - GPIB Parallel poll response bits are reversed.
/// - GPIB D8 controls first device.
/// - GPIB D1 controls last device.
///
/// @param [in] mask: bit mask to reverse.
/// @return  reversed bits
uint8_t reverse_8bits(uint8_t mask)
{
    int8_t i;
    uint8_t rmask = 0;
    for(i=0;i<8;++i)
    {
        rmask <<= 1;
        if(mask & 1)
            rmask |= 1;
        mask >>= 1;
    }
    return(rmask & 0xff);
}


/// @brief Enable or Disable Parallel Poll Response bits - PPR.
//
/// - The hardware implimentation does reversal automatically.
/// - The software implimentation must have the bits reversed.
/// - Aside: Software PPR is impractical - timing is sub microsecond.
///
/// - Mask Bits.
///  - HI  = PPR bus LOW.
///  - LOW = PPR bus FLOAT.
///  - Device PPR bits are specified in reversed order in GPIB.
///  - 0 controlls GPIB D8.
///  - 1 controlls GPIB D7.
///  - 2 controlls GPIB D6.
///  - 3 controlls GPIB D5.
///
/// @param[in] mask: Parallel Poll Response bits to enable or disable
///
/// @return  void

void ppr_set(uint8_t mask)
{
///@brief optionally reverse bit order in PPR mask
/// Used only of PPR circuit board PPR bits are not reversed in hardware
#if PPR_REVERSE_BITS == 1
    _ppr_reg = reverse_8bits(mask);
#else
    _ppr_reg = mask;
#endif
    SPI0_TXRX_Byte(_ppr_reg);

    GPIB_IO_HI(PPE);
    GPIB_IO_LOW(PPE);
}


/// @brief  Return PPR enable register.
///
/// - Hides the register access implimentation from the upper level.
/// @return  PPR enable register

uint8_t ppr_reg()
{
///@brief optionally reverse bit order in PPR mask
/// Used only of PPR circuit board PPR bits are not reversed in hardware
#if PPR_REVERSE_BITS == 1
    return(reverse_8bits(_ppr_reg));
#else
    return(_ppr_reg);
#endif
}


/// @brief  Reset PPR enable register - all disable..
///
/// - Hides the register access implimentation from the upper level.
/// @return  void

void ppr_init()
{
#if SDEBUG
    if(debuglevel & 2 )
        printf("[PPR DISABLE ALL]\n");
#endif
    ppr_set(0);
}


/// @brief  Enable hardware PPR response for a given device.
///
///@param[in] bit: PPR bit to enable.
///  - HI  = PPR bus LOW.
///  - LOW = PPR bus FLOAT.
///  - Device PPR bits are specified in reversed order in GPIB.
///  - 0 controlls GPIB D8.
///  - 1 controlls GPIB D7.
///  - 2 controlls GPIB D6.
///  - 3 controlls GPIB D5.
///
/// @return  void

void ppr_bit_set(uint8_t bit)
{
///FIXME _ppr_reg = 0;
    BIT_SET(_ppr_reg,bit);
    ppr_set(_ppr_reg);
}


/// @brief  Disbale hardware PPR response for a given device.
///
///@param[in] bit: PPR bit to disable.
///  - HI  = PPR bus LOW.
///  - LOW = PPR bus FLOAT.
///  - Device PPR bits are specified in reversed order in GPIB.
///  - 0 controlls GPIB D8.
///  - 1 controlls GPIB D7.
///  - 2 controlls GPIB D6.
///  - 3 controlls GPIB D5.
/// @return  void

void ppr_bit_clr(uint8_t bit)
{
///FIXME _ppr_reg = 0;
    BIT_CLR(_ppr_reg,bit);
    ppr_set(_ppr_reg);
}


/// @brief Wrapper for FatFs f_open() that can displays errors.
///
/// @param[in] fp: FatFs FIL handle
/// @param[in] path: path name of file to open
/// @param[in] mode: FatFs open mode flags
///
/// @see ff.h
///
/// @return  FRESULT f_open(fp,path, mode);

FRESULT dbf_open (FIL* fp, const TCHAR* path, BYTE mode)
{
    int rc;
    rc = f_open(fp,path, mode);
    if(rc)
    {
        printf("Open error:[%s] ", path);
        put_rc(rc);
        return (rc);
    }
    return(0);
}


/// @brief  Wrapper for FatFs f_read() that can display errors.
///
/// @param[in] fp: FatFs FIL handle.
/// @param[in] buff: buffer to read data into.
/// @param[in] btr: bytes to read.
/// @param[in] br: bytes actually read.
///
/// @see ff.h
///
/// @return  FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br)

FRESULT dbf_read (FIL* fp, void* buff, UINT btr, UINT* br)
{
    int rc;
    rc = f_read(fp, buff, btr, br);
    if(rc)
    {
        printf("Read error: ");
        put_rc(rc);
        return (rc);
    }
    return(0);
}


/// @brief  wrapper for FatFs f_write() that can display errors.
///
/// @param[in] fp: FatFs FIL handle.
/// @param[in] buff: buffer to write data from.
/// @param[in] btw: bytes to write.
/// @param[in] bw: bytes actually written.
///
/// @see ff.h
/// @return  FRESULT f_write (FIL* fp, void* buff, UINT btw, UINT* bw)

FRESULT dbf_write (FIL* fp, const void* buff, UINT btw, UINT* bw)
{
    int rc;
    rc = f_write(fp, buff, btw, bw);
    if(rc)
    {
        printf("Write error: ");
        put_rc(rc);
        return (rc);
    }
    return(0);
}


/// @brief  Wrapper of FatFs f_seek() that can display errors.
///
/// @param[in] fp: FatFs FIL * pointer.
/// @param[in] ofs: seek offset.
///
/// @see ff.h.
/// @return  FRESULT

FRESULT dbf_lseek (FIL* fp, DWORD ofs)
{
    int rc;
    rc = f_lseek(fp, ofs);
    if(rc)
    {
        printf("Seek error: ");
        put_rc(rc);
        return (rc);
    }
    return(0);
}


/// @brief  Wrapper of FatFs f_close() that can display errors.
///
/// @param[in] fp: FatFs FIL * pointer.
///
/// @see ff.h.
/// @return  FRESULT dbf_close (FIL* fp)

FRESULT dbf_close (FIL* fp)
{
    int rc;
    rc = f_close(fp);
    if(rc != FR_OK)
    {
        printf("Close error: ");
        put_rc(rc);
        return (rc);
    }
    return(0);
}


/// @brief Open, Seek, Read data and Close FatFs functions.
///
/// @param[in] name: File name to open.
/// @param[in] pos: file offset.
/// @param[out] buff: buffer to read data into.
/// @param[in] size: bytes to read.
/// @param[in] errors: error flags pointer.
///
/// @return  bytes actually read.
/// @return -1 on error.
/// @see: ff.h.
/// @return  FRESULT

int dbf_open_read(char *name, uint32_t pos, void *buff, int size, int *errors)
{
    int rc;
    FIL fp;
    int flags = 0;
    UINT bytes = 0;

    rc = dbf_open(&fp, name, FA_OPEN_EXISTING | FA_READ | FA_WRITE);
    if( rc != FR_OK)
    {
        flags |= ERR_DISK;
        flags |= ERR_READ;
        *errors = flags;
        return( -1 );
    }

///  SEEK
    rc = dbf_lseek(&fp, pos);
    if( rc != FR_OK)
    {
        flags |= ERR_SEEK;
        flags |= ERR_READ;
        *errors = flags;
        dbf_close(&fp);
        return( -1 );
    }

    rc = dbf_read(&fp, buff,size,&bytes);
    if( rc != FR_OK || (UINT) size != bytes)
    {
        flags |= ERR_READ;
        *errors = flags;
        dbf_close(&fp);
        return( -1 );
    }
    rc = dbf_close(&fp);
    if( rc != FR_OK)
    {
        flags |= ERR_DISK;
        *errors = flags;
        return( -1 );
    }
    return(bytes);
}


/// @brief Open, Seek, Write data and Close FatFs functions.
///
/// @param[in] name: File name to open.
/// @param[in] pos: file offset.
/// @param[in] buff: buffer to write.
/// @param[in] size: bytes to write.
/// @param[in] errors: error flags pointer.
///
/// @return  bytes actually written.
/// @return -1 on error.
/// @see: ff.h.
/// @return  FRESULT
int dbf_open_write(char *name, uint32_t pos, void *buff, int size, int *errors)
{
    int rc;
    FIL fp;
    int flags = 0;
    UINT bytes = 0;

    rc = dbf_open(&fp, name, FA_OPEN_EXISTING | FA_READ | FA_WRITE);
    if( rc != FR_OK)
    {
        flags |= ERR_DISK;
        flags |= ERR_READ;
        *errors = flags;
        return( -1 );
    }

///  SEEK
    rc = dbf_lseek(&fp, pos);
    if( rc != FR_OK)
    {
        flags |= ERR_SEEK;
        flags |= ERR_READ;
        *errors = flags;
        dbf_close(&fp);
        return( -1 );
    }

    rc = dbf_write(&fp, buff,size,&bytes);
    if( rc != FR_OK || (UINT) size != bytes)
    {
        flags |= ERR_READ;
        *errors = flags;
        dbf_close(&fp);
        return( -1 );
    }
    rc = dbf_close(&fp);
    if( rc != FR_OK)
    {
        flags |= ERR_DISK;
        *errors = flags;
        return( -1 );
    }
    return(bytes);
}
