/**
 @file gpib/defines.h

 @brief GPIB, AMIGO, SS80 and device defines.

 @par Edit History - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Anders Gustafsson All rights reserved..

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#ifndef DEFINES_H
#define DEFINES_H
/*-------------------------------------------------------------------------
  defines.h - global defines

             (c) 2014 Anders Gustafsson <anders.gustafsson@pedago.fi>

-------------------------------------------------------------------------*/
#include "hardware/hardware.h"


#define ABORT_FLAG 1  /*< user abort */
#define MEDIA_FLAG 2  /*< missing media */

#define HP9121D     //< HP9121 dual 270K AMIGO floppy drive
#define HP9134L     //< HP9134L 40M SS/80 Winchester drive

#if defined(HP9121D)
#warning Compiled for a HP 9121D Amigo floppy
#define AMIGOID1 0x01 /*< AMIGO device ID byte 1 */
#define AMIGOID2 0x04 /*< AMIGO device ID byte 2 */
#else
#error You must define an Amigo-floppy in defines.h
#endif

#if defined(HP9122D)
#warning Compiled for a HP 9122D SS/80 floppy
#define SS80ID1 0x02
#define SS80ID2 0x22
#elif defined(HP9134L)
#warning Compiled for a HP 9134L 40M SS/80 Winchester drive
#define SS80ID1 0x02
#define SS80ID2 0x21
#else
#error You must define a SS/80 floppy in defines.h
#endif

#define GTL     0x01        //<  Go to local
#define SDC     0x04        //<  Selected device clear
#define PPC     0x05        //<  Parallell poll configure
#define GET     0x08        //<  Group execute trigger
#define TCT     0x09        //<  Take control
#define LLO     0x11        //<  Local lockout
#define DCL     0x14        //<  Device clear
#define PPU     0x15        //<  Parallell poll unconfigure
#define SPE     0x18        //<  Serial poll enable
#define SPD     0x19        //<  Serial poll disable

#define UNL          0x3F   //<  Unlisten
#define UNT          0x5F   //<  Untalk

#define SS80_MLA     0x20   //<  My listen address = 0 (0+0x20)
#define SS80_MTA     0x40   //<  My talk address = 0 (0+0x40)
#define SS80_MSA     0x60   //<  My seconday address = 0 (0+0x60)
#define SS80_PPR     0      //<  PPR Address

#define AMIGO_MLA    0x21   //<  My listen address = 0 (0+0x20)
#define AMIGO_MTA    0x41   //<  My talk address = 0 (0+0x40)
#define AMIGO_MSA    0x61   //<  My seconday address = 0 (0+0x60)
#define AMIGO_PPR    1      //<  PPR Address

#define PRINTER_MLA  0x22   //<  My listen address = 0 (0+0x20)
#define PRINTER_MTA  0x42   //<  My talk address = 0 (0+0x40)
#define PRINTER_MSA  0x62   //<  My seconday address = 0 (0+0x60)

#define ERR_READ   0b00000001 	//< Read Error
#define ERR_WRITE  0b00000010	//< Write Error 
#define ERR_SEEK   0b00000100	//< Seek Error
#define ERR_WP     0b00001000	//< Write Protect Error
#define ERR_DISK   0b00010000	//< Disk Error
#define ERR_GPIB   0b00100000	//<GPIB Error

///@brief AMIGO Disk structure - position and error status
typedef struct
{
    uint8_t cyl;
    uint8_t head;
    uint8_t sector;
    uint8_t dsj;
    int Amigo_Errors;
} DiskType;

///@brief AMIGO Disk structure - ID bytes and layout.
typedef struct _disp_parm
{
    uint8_t id[2];
    int16_t bytes_per_sector;
    int16_t sectors_per_track;
    int16_t heads;
    int16_t cylinders;
} disk_parm;

typedef struct   //< Controller description, 5 bytes
{
    uint8_t C1;  //<  Installed unit byte, one unit:
    uint8_t C2;  //<  C1 = 10000000 C2 = 00000001
    uint8_t C3;  //<  Transfer rate in kb/s MSB
    uint8_t C4;  //<  LSB
    uint8_t C5;  //<  Controller type 4 = SS/80 single unit
} ControllerDescriptionType;

typedef struct   //<  Unit description, 19 bytes
{
    uint8_t U1;  //<  Type 0-Fixed, 1-Flexible, 2-Tape (+128-dumb, does not detect media change)
    uint8_t U2;  //<  Device number MSB
    uint8_t U3;  //<  (HP 9133 = 09 13 30)
    uint8_t U4;  //<  LSB
    uint8_t U5;  //<  Bytes per block MSB
    uint8_t U6;  //<  LSB
    uint8_t U7;  //<  Number of buffered blocks
    uint8_t U8;  //<  Burst size (0 for SS/80)
    uint8_t U9;  //<  Block time in ms MSB
    uint8_t U10; //<  LSB
    uint8_t U11; //<  Continous average transfer rate MSB
    uint8_t U12; //<  LSB
    uint8_t U13; //<  Optimal retry in tens of ms MSB
    uint8_t U14; //<  LSB
    uint8_t U15; //<  Access time in tens of ms MSB
    uint8_t U16; //<  LSB
    uint8_t U17; //<  Maximum interleave or 0
    uint8_t U18; //<  Fixed volume byte, one bit per volume, ie 00000111 = 3 volumes
    uint8_t U19; //<  Removable volume byte, one bit per volume, ie 00000111 = 3 volumes
} UnitDescriptionType;

typedef struct   //<  Volume description,  bytes
{
    uint8_t V1;  //<  Max cylinder MSB
    uint8_t V2;  //< 
    uint8_t V3;  //<  LSB
    uint8_t V4;  //<  Maximum head
    uint8_t V5;  //<  Maximum sector, MSB
    uint8_t V6;  //<  LSB
    uint8_t V7;  //<  Max value of single vector in blocks MSB
    uint8_t V8;
    uint8_t V9;
    uint8_t V10;
    uint8_t V11;
    uint8_t V12; //<  LSB
    uint8_t V13; //<  Interleave

} VolumeDescriptionType;

//@brief
typedef union
{
    uint32_t Length;
    uint8_t Lenbytes[4];
} LengthType;

//@brief LIF disk label record
typedef struct
{
    uint16_t LIFid;
    uint16_t label[6];
    uint16_t dirstarthi;
    uint16_t dirstartlo;
    uint16_t s3000;
    uint16_t dummy;
    uint16_t dirlenhi;
    uint16_t dirlenlo;
    uint16_t version;
} VolumeLabelType;

//@brief LIF directory entry
typedef struct
{
    char filename[10];
    uint16_t filetype;
    uint16_t startaddhi;
    uint16_t startaddlo;
    uint16_t lengthhi;
    uint16_t lengthlo;
    char createtime[6];    //< BCD digits
    uint16_t volnumber;
    uint16_t implementationhi;
    uint16_t implementationlo;
} DirEntryType;
#endif     // DEFINES_H
