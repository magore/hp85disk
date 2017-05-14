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
#include "user_config.h"



/// ==============================================================
///@brief Power on defaults  address and PPR for SS80 AMIGO and PRINTER
/// Can be changed if specified in user config
///@see ss80.c 

///@brief Default SS80 address 
#define SS80_DEFAULT_ADDRESS 0        /* SS80 default address */

///@brief Default SS80 Parallel Poll Response bit
/// 0 here is bit 8 on the BUS
#define SS80_DEFAULT_PPR 0            /* SS80 default PPR BIT */

///@brief Default AMIGO address 
#define AMIGO_DEFAULT_ADDRESS 1       /* AMIGO default address */

///@brief Default AMIGO Parallel Poll Response bit 
/// 1 here is bit 7 on the BUS
#define AMIGO_DEFAULT_PPR 1           /* AMIGO default PPR BIT */

///@brief Default PRINTER address 
#define PRINTER_DEFAULT_ADDRESS 2     /* PRINTER default address */
///@brief printer do not use parallel poll

/// ==============================================================


#define ABORT_FLAG 1  /*< user abort */
#define MEDIA_FLAG 2  /*< missing media */

#define HP9121D     //< HP9121 dual 270K AMIGO floppy drive
#define HP9134L     //< HP9134L 40M SS/80 Winchester drive

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

#define BASE_MLA     0x20   //<  Base listen address = 0 (0+0x20)
#define BASE_MTA     0x40   //<  Base talk address = 0 (0+0x40)
#define BASE_MSA     0x60   //<  Base seconday address = 0 (0+0x60)



#define ERR_READ   0b00000001 	//< Read Error
#define ERR_WRITE  0b00000010	//< Write Error 
#define ERR_SEEK   0b00000100	//< Seek Error
#define ERR_WP     0b00001000	//< Write Protect Error
#define ERR_DISK   0b00010000	//< Disk Error
#define ERR_GPIB   0b00100000	//<GPIB Error

/// ====================================================================
///@brief used with AMIGO and SS80
typedef struct   //< Identify, 2 bytes
{
    uint8_t I1;  //<  Identify, For 9122 I1=02, I2=22H
    uint8_t I2;  //<  Identify
} IdentifyType;


/// ====================================================================

///@brief AMIGO Disk structure - ID bytes and layout.
typedef struct 
{
	///@brief AMIGO address 
	uint8_t amigo_addr;
	///@brief AMIGO parallel poll bit number 
	uint8_t amigo_ppr;
    IdentifyType id;
    int16_t bytes_per_sector;
    int16_t sectors_per_track;
    int16_t heads;
    int16_t cylinders;
} AMIGODiskType;

///@brief AMIGO status structure - position and error status
typedef struct
{
    uint8_t cyl;
    uint8_t head;
    uint8_t sector;
    uint8_t dsj;
    int Amigo_Errors;
} AMIGOStatusType;


/// ====================================================================
///@brief Controller Information Structure
typedef struct   //< Controller description, 5 bytes
{
    uint8_t C1;  //<  Installed unit byte, one unit:
    uint8_t C2;  //<  C1 = 10000000 C2 = 00000001
    uint8_t C3;  //<  MSB Transfer rate in kb/s 
    uint8_t C4;  //<  LSB
    uint8_t C5;  //<  Controller type 
                 //< 0 = CS/80 integrated single unit controller.
                 //< 1 = CS/8O integrated multi-unit controller.
                 //< 2 = CS/8O integrated multi-port controller.
                 //< 4 = SS/8O integrated single unit controller.
                 //< 5 = SS/80 integrated multi-unit controller.
                 //< 6 = SS/80 integrated multi-port controller.
} SS80ControllerType;

///@brief Unit Information Structure
typedef struct   //<  Unit description, 19 bytes
{
    uint8_t U1;  //<  Type 0-Fixed, 1-Flexible, 2-Tape (+128-dumb, does not detect media change)
    uint8_t U2;  //<  MSB Device number 
    uint8_t U3;  //<  (HP 9133 = 09 13 30)
    uint8_t U4;  //<     LSB
    uint8_t U5;  //<  MSB Bytes per block 
    uint8_t U6;  //<     LSB
    uint8_t U7;  //<  Number of buffered blocks
    uint8_t U8;  //<  Burst size (0 for SS/80)
    uint8_t U9;  //<  MSB Block time in ms 
    uint8_t U10; //<     LSB
    uint8_t U11; //<  MSB Continous average transfer rate 
    uint8_t U12; //<     LSB
    uint8_t U13; //<  MSB Optimal retry in tens of ms 
    uint8_t U14; //<     LSB
    uint8_t U15; //<  MSB Access time in tens of ms 
    uint8_t U16; //<     LSB
    uint8_t U17; //<  Maximum interleave or 0
    uint8_t U18; //<  Fixed volume byte, one bit per volume, ie 00000111 = 3 volumes
    uint8_t U19; //<  Removable volume byte, one bit per volume, ie 00000111 = 3 volumes
} SS80UnitType;

///@brief Volume Information Structure
typedef struct   //<  Volume description,  bytes
{
    uint8_t V1;  //<  MSB Max cylinder 
    uint8_t V2;  //< 
    uint8_t V3;  //<     LSB
    uint8_t V4;  //<  Maximum head 0 based
    uint8_t V5;  //<  MSB Maximum sector 0 based
    uint8_t V6;  //<     LSB
    uint8_t V7;  //<  MSB Max number of blocks 
    uint8_t V8;
    uint8_t V9;
    uint8_t V10;
    uint8_t V11;
    uint8_t V12; //<     LSB
    uint8_t V13; //<  Interleave

} SS80VolumeType;


///@brief used with SS80
typedef union
{
    DWORD L;
    BYTE B[4];
} SS80LengthType;

///@brief used with SS80
typedef union
{
    DWORD L;
    BYTE B[6];
} SS80AddressType;

///@brief Disk Information Structure
typedef struct {
	///@brief SS80 address 
	uint8_t ss80_addr;
	///@brief SS80 parallel poll bit number
	uint8_t ss80_ppr;
    IdentifyType id;
    SS80ControllerType Controller;
    SS80UnitType Unit;
    SS80VolumeType Volume;
} SS80DiskType;


/// ====================================================================
/// LIF formating structures
///@see format.c
///@brief LIF disk label record
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

///@brief LIF directory entry
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
/// =================

/// ==============================================================
///@brief address and PPR for SS80 AMIGO and PRINTER
/// Can be set if specified in user config
/// If NOT specified see Power on Defaults below
///@see ss80.c 

///@brief PRINTER address
extern uint8_t printer_addr;
///@brief printer do not use parallel poll


extern SS80DiskType SS80Disk;
extern AMIGODiskType AMIGODisk;

#define SS80_MLA     (BASE_MLA + SS80Disk.ss80_addr)    //<  SS80 listen address 
#define SS80_MTA     (BASE_MTA + SS80Disk.ss80_addr)   //<  SS80 talk address 
#define SS80_MSA     (BASE_MSA + SS80Disk.ss80_addr)   //<  SS80 seconday address 
#define SS80_PPR     (SS80Disk.ss80_ppr)             //<  SS80 PPR Address
#define AMIGO_MLA    (BASE_MLA + AMIGODisk.amigo_addr)  //<  AMIGO listen address
#define AMIGO_MTA    (BASE_MTA + AMIGODisk.amigo_addr)  //<  AMIGO talk address 
#define AMIGO_MSA    (BASE_MSA + AMIGODisk.amigo_addr)  //<  AMIGO seconday address
#define AMIGO_PPR    (AMIGODisk.amigo_ppr)            //<  AMIGO PPR Address

#define PRINTER_MLA  (BASE_MLA + printer_addr) //<  PRINTER listen address 
#define PRINTER_MTA  (BASE_MTA + printer_addr) //<  PRINTER talk address 
#define PRINTER_MSA  (BASE_MSA + printer_addr) //<  PRINTER seconday address 
/// =================
#endif     // DEFINES_H
