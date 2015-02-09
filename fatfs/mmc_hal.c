/**
 @file fatfs/mmc_hal.c

 @brief MMC Hardware Layer for FatFs.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#include <hardware/hardware.h>
#include "mmc_hal.h"
#include "ff.h"




///@brief MMC timer tic in Microseconds
#ifndef MMC_TIMER_TIC_US
#error Please define MMC_TIMER_TIC_US in Microseconds per mmc_task call
#endif


///@brief MMC timeout timer used to detect timeout error conditions
volatile int32_t mmc_us_timeout;

/// @brief  Install MMC timer task: mmc_task() 
///
/// @see  mmc_task()
/// @return  void

void mmc_install_timer( void )
{

    set_timers(mmc_task,1);
}


/// @brief Set MMC timeout timer in Milliseconds
///
/// @param[in] ms: timeout in Milliseconds
///
/// @see mmc_test_timeout ( )
/// @return  void

void mmc_set_ms_timeout( int16_t ms)
{
    int32_t us = ms;
    us *= 1000L;
    us += MMC_TIMER_TIC_US;
    mmc_cli();
    mmc_us_timeout = us;
    mmc_sei();
}


/// @brief See MMC timeout timer in Microseconds
///
/// @param[in] us: timeout in Microseconds
///
/// @see mmc_test_timeout ( )
/// @return  void

void mmc_set_us_timeout( int32_t us)
{
    us += MMC_TIMER_TIC_US;
    mmc_cli();
    mmc_us_timeout = us;
    mmc_sei();
}


/// @brief  Test MMC timeout status
///
/// @see mmc_set_us_timeout() 
/// @see mmc_set_ms_timeout()
/// @return 0 still counting 
/// @return 1 timeout reached

int mmc_test_timeout ( void )
{
    int n;
    mmc_cli();
    n = mmc_us_timeout ? 0 : 1;
    mmc_sei();
    return (n);
}


/// @brief  Clear/Initialize MMC timeout value 
///
/// @see mmc_set_us_timeout() 
/// @see mmc_set_ms_timeout()
/// @return  void

void mmc_clear_timeout ( void )
{
    mmc_cli();
    mmc_us_timeout = 0;
    mmc_sei();
}


/// @brief  MMC Microsecond accumulator for mmc_task()
static int32_t __mmc_task_timer;

/// @brief  MMC FatFs insert,write protect status
extern DSTATUS Stat;

/// @brief  This function is called every MMC_TIMER_TIC_US
///
/// - Monitor timeouts, write protect and card detect status
/// @return  void

void mmc_task (void)
{
    BYTE s;

    if(mmc_us_timeout >= MMC_TIMER_TIC_US)
        mmc_us_timeout -= MMC_TIMER_TIC_US;
    else
        mmc_us_timeout = 0;

    __mmc_task_timer -= MMC_TIMER_TIC_US;
    if (__mmc_task_timer < 0)
    {
        __mmc_task_timer = 10000L;                // 10,000us

        s = Stat;

        if (mmc_wp_status())                      /* Write protected */
            s |= STA_PROTECT;
        else                                      /* Write enabled */
            s &= ~STA_PROTECT;

        if (mmc_ins_status())                     /* Card inserted */
            s &= ~STA_NODISK;
        else                                      /* Socket empty */
            s |= (STA_NODISK | STA_NOINIT);
        Stat = s;                                 /* Update MMC status */
    }
}



/// @brief  MMC SPI send and receive character.
///
/// @param[in] a: Character to send
///
/// @return  uint8_t Received character

uint8_t mmc_xchg_spi(uint8_t a)
{
///  Note: when reading only call with 0xff
    return( SPI0_WriteReadByte(a) );
}


/// @brief  MMC SPI bus init
/// @return  void

void mmc_spi_init()
{
    SPI0_Init();                         //< Initialize the SPI bus
    SPI0_Mode(0);                        //< Set the clocking mode, etc
    mmc_slow();                          //< Set the speed to "slow" the slowest speed an MMC device might need.
}


/// @brief  MMC set slow SPI bus speed 
///
/// - Used during card detect phase
/// @return  void

void mmc_slow()
{
    SPI0_Speed(250000U);  //< In HZ 100khz..400khz
}


/// @brief  MMC fast SPI bus speed 
///
/// - Used during normal file IO phases
/// @return  void

void mmc_fast()
{
    SPI0_Speed(2500000U);                         // IN HZ 2.5mhz
}


/// @brief  MMC power ON initialize
///
/// - We do not control power to the MMC device in this project.
/// @return  void

void mmc_power_on()
{
    mmc_spi_init();
    mmc_slow();
    (void)mmc_xchg_spi(0xff);
    delayms(20);
}


/// @brief  MMC power off 
///
/// - We do not control power to the MMC device in this project.
/// @return  void

void mmc_power_off()
{
}


/// @brief  MMC power status
///
/// - We do not control power to the MMC device in this project.
/// @return  1 power is alwasy on

int mmc_power_status()
{
    return (1);
}



/// @brief  MMC CS enable
///
/// - MMC SPI chip sellect 
/// @return  void

void mmc_cs_enable()
{
    IO_LOW(MMC_CS);
}


/// @brief MMC CS disable
///
/// - MMC SPI chip sellect 
/// @return  void

void mmc_cs_disable()
{
    IO_HI(MMC_CS);
}


/// @brief  MMC Card Inserted status
///
/// - We do not detect card insert status in this project.
/// @return 1 card inserted

int mmc_ins_status()
{
    return (1);
}


/// @brief  MMC Card Write Protect status
///
/// - We do not detect card write protect status in this project.
/// @return 0 == not write protected

int mmc_wp_status()
{
    return (0);
}


static int mmc_init_flag = 0;

/// @brief Initialize MMC and FatFs interface, display diagnostics.
///
/// @param[in] cold: 1 also initailize MMC timer.
/// @return

int mmc_init(int cold)
{
    int rc;

    if( !mmc_init_flag)
    {
		myprintf("==============================\n");
        myprintf("START MMC INIT\n");
    }

    mmc_spi_init();

    if( cold )
        mmc_install_timer();

    mmc_power_on();

    if( !mmc_init_flag)
    {
#if defined (_USE_LFN)
        myprintf("LFN Enabled");
#else
        myprintf("LFN Disabled");
#endif
        myprintf(", Code page: %u\n", _CODE_PAGE);
    }

    rc = disk_initialize(0);                      // aliased to mmc_disk_initialize()
    if( rc != RES_OK )
    {
        put_rc(rc);
    }

    if( rc == RES_OK)
    {
        rc = f_mount(&Fatfs[0],"/", 0);
    }

    if( rc != RES_OK)
    {
        put_rc( rc );
    }

    if( !mmc_init_flag)
    {
        DWORD blksize = 0;
        fatfs_status("/");

        rc = mmc_disk_ioctl ( Fatfs[0].drv, GET_BLOCK_SIZE, (void *) &blksize);
        if( rc != RES_OK)
        {
            put_rc( rc );
            myprintf("MMC Block Size - read failed\n");
        }
        else
        {
            myprintf("MMC Block Size: %ld\n", blksize);
        }

        myprintf("END MMC INIT\n");
		myprintf("==============================\n");
    }

    if( rc == RES_OK )
        mmc_init_flag = 1;

    return( rc ) ;
}
