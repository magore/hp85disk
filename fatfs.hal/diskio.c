/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */

/* mmc.c */
#include "user_config.h"
#include "fatfs.h"

#include "ff.h"                 /* Obtains integer types for FatFs */
#include "diskio.h"             /* FatFs lower layer API */


#ifdef DRV_MMC
#include "mmc.h"    	/* Header file of existing SD control module */
#endif

#ifdef DRV_CFC
#include "cfc.h"    	/* Header file of existing CF control module */
#endif

#ifdef DRV_RAM
#include "ram.h"    /* Header file of existing ram control module */
#endif

#ifdef DRV_USB
#include "usb.h"    	/* Header file of existing CF control module */
#endif


/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

/* Definitions of physical drive number for each drive */
#define DEV_MMC		0	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_CFC     1	/* Example: Map CF card to physical drive 2 */
#define DEV_RAM		2	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_USB		3	/* Example: Map USB MSD to physical drive 2 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv) {
#ifdef DRV_MMC
	case DEV_MMC :
		return ( mmc_disk_status() );
#endif
#ifdef DRV_CFC
	case DEV_CF :
		return ( cf_disk_status() );
#endif
#ifdef DRV_RAM
	case DEV_RAM :
		return ( ram_disk_status() );
#endif
#ifdef DRV_USB
	case DEV_USB :
		return ( usb_disk_status() );
#endif
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{

	switch (pdrv) {
#ifdef DRV_MMC
	case DEV_MMC :
		return ( mmc_disk_initialize() );
#endif
#ifdef DRV_CFC
	case DEV_CF :
		return ( cfc_disk_initialize() );
#endif
#ifdef DRV_RAM
	case DEV_RAM :
		return ( ram_disk_initialize() );
#endif
#ifdef DRV_USB
	case DEV_USB :
		return ( usb_disk_initialize() );
#endif
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	switch (pdrv) {
#ifdef DRV_MMC
	case DEV_MMC :
		return ( mmc_disk_read(buff, sector, count) );
#endif
#ifdef DRV_CFC
	case DEV_CF :
		return ( cf_disk_read(buff, sector, count) );
#endif
#ifdef DRV_RAM
	case DEV_RAM :
		return ( ram_disk_read(buff, sector, count) );
#endif
#ifdef DRV_USB
	case DEV_USB :
		return ( usb_disk_read(buff, sector, count) );
#endif
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	switch (pdrv) {
#ifdef DRV_MMC
	case DEV_MMC :
		return ( mmc_disk_write(buff, sector, count) );
#endif
#ifdef DRV_CFC
	case DEV_CF :
		return ( cf_disk_write(buff, sector, count) );
#endif
#ifdef DRV_RAM
	case DEV_RAM :
		return ( ram_disk_write(buff, sector, count) );
#endif
#ifdef DRV_USB
	case DEV_USB :
		return ( usb_disk_write(buff, sector, count) );
#endif
	}

	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
    BYTE pdrv,      /* Physical drive nmuber (0..) */
    BYTE cmd,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
)
{
    switch (pdrv) {
#ifdef DRV_MMC
    case DEV_MMC :
        return ( mmc_disk_ioctl(cmd, buff) );
#endif
#ifdef DRV_CFC
    case DEV_CFC :
        return ( cf_disk_ioctl(cmd, buff) );
#endif
#ifdef DRV_RAM
    case DEV_RAM :
        return ( ram_disk_ioctl(cmd, buff) );
#endif
#ifdef DRV_USB
    case DEV_MMC :
        return ( usb_disk_ioctl(cmd, buff) );
#endif
    }
    return RES_PARERR;
}
#endif

/*-----------------------------------------------------------------------*/
/* Timer driven procedure                                                */
/*-----------------------------------------------------------------------*/


void disk_timerproc (void)
{
#ifdef DRV_MMC
    mmc_disk_timerproc();
#endif
#ifdef DRV_CFC
    cf_disk_timerproc();
#endif
#ifdef DRV_RAM
    ram_disk_timerproc();
#endif
#ifdef DRV_USB
    usb_disk_timerproc();
#endif

}
