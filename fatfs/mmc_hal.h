/**
 @file fatfs/mmc_hal.h

 @brief MMC Hardware Layer for FatFs.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#ifndef _MMC_HAL_H
#define _MMC_HAL_

#include "hardware/hardware.h"

///@see timer.c 
///@see time.c

#define MMC_TIMER_TIC_US CLOCK_TIC_US /*< Define MMC time task in Microseconds - using AVR clock routines. */

#define mmc_cli cli /*< interrupt disable */
#define mmc_sei sei /*< interrupt enable */

#define mmc_disk_initialize disk_initialize	/*< disk_initialize() */
#define mmc_disk_status disk_status			/*< disk_status() */
#define mmc_disk_read disk_read             /*< disk_read() */
#define mmc_disk_write disk_write			/*< disk_read() */
#define mmc_disk_ioctl disk_ioctl           /*< disk_ioctl */

/* mmc_hal.c */
void mmc_install_timer ( void );
void mmc_set_ms_timeout ( int16_t ms );
void mmc_set_us_timeout ( int32_t us );
int mmc_test_timeout ( void );
void mmc_clear_timeout ( void );
void mmc_task ( void );
uint8_t mmc_xchg_spi ( uint8_t a );
void mmc_spi_init ( void );
void mmc_slow ( void );
void mmc_fast ( void );
void mmc_power_on ( void );
void mmc_power_off ( void );
int mmc_power_status ( void );
void mmc_cs_enable ( void );
void mmc_cs_disable ( void );
int mmc_ins_status ( void );
int mmc_wp_status ( void );
int mmc_init ( int cold );
#endif                                            // _MMC_HAL_
