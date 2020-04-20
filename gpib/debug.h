#ifndef _DEBUG_H_
#define _DEBUG_H_

#define GPIB_ERR                         1        /* GPIB error messages */
#define GPIB_PPR                         2        /* GPIB PPR Parallel Poll Response states */
#define GPIB_BUS_OR_CMD_BYTE_MESSAGES    4        /* GPIB BUS level single command byte decoded messages */
#define GPIB_TOP_LEVEL_BUS_DECODE        8        /* GPIB main loop - top level data and controll lines decoded */
#define GPIB_TODO                       16        /* GPIB TODO DEVICE missing support code */
#define GPIB_DEVICE_STATE_MESSAGES      32        /* GPIB Device Level states such as AMIGO,SS80,PRINTER */
#define GPIB_DISK_IO_TIMING             64        /* GPIB Disk I/O read/write times */
#define GPIB_RW_STR_TIMING             128        /* GPIB read/write string timing */
#define GPIB_RW_STR_BUS_DECODE         256        /* GPIB read/write string byte decode */
#define GPIB_PP_BUS_STATUS             512        /* GPIB try to detect Parallel Poll BUS state */
#define LIF_DEBUG                     1024        /* LIF utitilites debugging */
#endif

extern int debuglevel;
