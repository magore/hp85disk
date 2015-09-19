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
MEMSPACE void mmc_install_timer ( void );
MEMSPACE void mmc_spi_TX_buffer ( const uint8_t *data , int count );
MEMSPACE void mmc_spi_RX_buffer ( const uint8_t *data , int count );
MEMSPACE uint8_t mmc_spi_RX ( void );
MEMSPACE void mmc_spi_TX ( uint8_t data );
MEMSPACE uint8_t mmc_spi_TXRX ( uint8_t data );
MEMSPACE void mmc_set_ms_timeout ( uint16_t ms );
MEMSPACE int mmc_test_timeout ( void );
MEMSPACE int mmc_init ( int verbose );
MEMSPACE void mmc_spi_init ( int32_t clock );
MEMSPACE void mmc_slow ( void );
MEMSPACE void mmc_fast ( void );
MEMSPACE void mmc_power_on ( void );
MEMSPACE void mmc_power_off ( void );
MEMSPACE int mmc_power_status ( void );
MEMSPACE void mmc_cs_enable ( void );
MEMSPACE void mmc_cs_disable ( void );
MEMSPACE int mmc_ins_status ( void );
MEMSPACE int mmc_wp_status ( void );

#endif                                            // _MMC_HAL_
