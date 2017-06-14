/**
 @file gpib/defines.h

 @brief GPIB, AMIGO, SS80 and device defines.

 @par Edit History - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.

 @par Copyright &copy; 2014 Anders Gustafsson All rights reserved..

*/

#ifndef _DEFINES_H
#define _DEFINES_H


#include "user_config.h"

extern int debuglevel;


// =============================================
///@brief Power on defaults  address and PPR for SS80 AMIGO and PRINTER
/// Can be changed if specified in user config
///@see ss80.c 

///@brief Default SS80 address 
#define SS80_DEFAULT_ADDRESS 0        /* SS80 default address */

///@brief Default SS80 Parallel Poll Response bit
/// 0 here is bit 8 on the BUS
#define SS80_DEFAULT_PPR 0            /* SS80 default PPR BIT */

#ifdef AMIGO
///@brief Default AMIGO address 
#define AMIGO_DEFAULT_ADDRESS 1       /* AMIGO default address */

///@brief Default AMIGO Parallel Poll Response bit 
/// 1 here is bit 7 on the BUS
#define AMIGO_DEFAULT_PPR 1           /* AMIGO default PPR BIT */
#endif

///@brief Default PRINTER address 
#define PRINTER_DEFAULT_ADDRESS 2     /* PRINTER default address */
///@brief printer do not use parallel poll

// =============================================

///Sector size
#define SECTOR_SIZE 256
// =============================================


#define ABORT_FLAG 1  /*< user abort */
#define MEDIA_FLAG 2  /*< missing media */

#define HP9121D     //< HP9121 dual 270K AMIGO floppy drive
#define HP9134L     //< HP9134L 40M SS/80 Winchester drive

/// =========================================================
///@brief  GPIB defines provided from Anders Gustafsson <anders.gustafsson@pedago.fi>
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
/// =========================================================

#define BASE_MLA     0x20   //<  Base listen address = 0 (0+0x20)
#define BASE_MTA     0x40   //<  Base talk address = 0 (0+0x40)
#define BASE_MSA     0x60   //<  Base seconday address = 0 (0+0x60)

#define ERR_READ   0b00000001 	//< Read Error
#define ERR_WRITE  0b00000010	//< Write Error 
#define ERR_SEEK   0b00000100	//< Seek Error
#define ERR_WP     0b00001000	//< Write Protect Error
#define ERR_DISK   0b00010000	//< Disk Error
#define ERR_GPIB   0b00100000	//<GPIB Error

// =============================================
///@brief Fault bit and Message type
typedef struct
{
    int index;
    char *msg;
} fault_t;

// =============================================
/**
  @brief Disk Layout
  @see https://groups.io/g/hpseries80/wiki/HP-85-Program-Control-Block-(BASIC-header),-Tape-directory-layout,-Disk-directory-layout

    DISK LAYOUT
    The HP-85 disks used the LIF (Logical Interchange Format) disk layout.  The first 2 sectors on the disk (cylinder 0, head 0, sector 0-1) contained the VOLUME sectors.  The important things in the VOLUME SECTORS were thus:

    BYTES   DESCRIPTION
    -----   -----------------------------------------------------------
      0-1   LIF identifier, must be 0x8000 MSB first
      2-7   6-character volume LABEL
     8-11   directory start block always 0x00000002 MSB first
    12-13   LIF identifer for System 3000 machines always 0x1000 MSB first
    14-15   always 0x0000 MSB first
    16-19   # of sectors in DIRECTORY MSB first
    20-21   LIF version number always 0x0001 MSB first
    22-23   always 0x0000 MSB first
    24-27   number of tracks per surface MSB first
    28-31   number of surfaces MSB first
    32-35   number of sectors per track MSB first
    36-41   time volume was initialized in BCD (YY,MM,DD,HH,mm,SS)
*/

/// LIF formating structures
///@see format.c
///@brief LIF disk label record
typedef struct
{
    uint16_t LIFid;					// 0
    uint8_t  Label[6];				// 2
    uint32_t DirStartSector;		// 8
    uint16_t System3000LIFid;		// 12
    uint16_t zero1;					// 14
    uint32_t DirSectors;			// 16
    uint16_t LIFVersion;			// 20
    uint16_t zero2;					// 22
	uint32_t tracks_per_side;		// 24
	uint32_t sides;					// 28
	uint32_t sectors_per_track;	    // 32
    uint8_t  date[6];               // 36 BCD (YY MM DD HH MM SS)
} VolumeLabelType;

/**
 @brief Directory layout
  @see https://groups.io/g/hpseries80/wiki/HP-85-Program-Control-Block-(BASIC-header),-Tape-directory-layout,-Disk-directory-layout

  Each DIRECTORY SECTORS held 8 32-byte directory entries.  
  Each entry contained these values:

  BYTE	DESCRIPTION
  ----	------------------------------------------------
  0-9	10-character file name (blank filled)
  10-11	File TYPE MSB first
  12-15	Start of file in sectors MSB first
  16-19	File length in sectors MSB first
  20-25	file creation DATE YY,MM,DD,HH,MM,SS
  26-27	always 0x8001 entire file is on volume MSB first
  28-29 size of file in bytes MSB first
        May be 0 use for some file types so use number of sectors instead
  30-31 bytes per record, typically 256
  Note: bytes 28-31 are implementation dependent
    i.e. non-Series-80 systems may write other information into these bytes.
*/
       
///@brief LIF directory entry
typedef struct
{
    char filename[10];			// 0
    uint16_t FileType;			// 10
    uint32_t FileStartSector;	// 12
    uint32_t FileLengthSectors;	// 16
    uint8_t  date[6];           // 20 File date in BCD (YY MM DD HH MM SS)
    uint16_t VolNumber;			// 26
    uint16_t FileBytes;			// 30
    uint16_t SectorSize;		// 28
} DirEntryType;


///@brief When formatting a disk define how many sectors we can write at once
///Depends on how much free ram we have
#define LIF_SECTOR_SIZE 256

///@brief used for formatting
#define LIF_CHUNKS 16
#define LIF_CHUNK_SIZE (LIF_SECTOR_SIZE*LIF_CHUNKS)

///@brief Used for lif_opendir(), lif_readdir()
#define LIF_DIR_SIZE 32
#define LIF_IMAGE_NAME_SIZE 64

typedef struct {
    char filename[LIF_IMAGE_NAME_SIZE+1];
	VolumeLabelType V;
    uint8_t dirbuf[LIF_DIR_SIZE];
    FILE *fp;
	DirEntryType DE;
	long imagesize;		// LIF image size
	long current;	    // first sector of file
	long next;		    // next free sector area
	long used;		    // used sectors
    long index;			// directory index
} lifdir_t;

// =============================================
#endif     // #ifndef _DEFINES_H
