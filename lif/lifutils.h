/**
 @file gpib/defines.h

 @brief GPIB, AMIGO, SS80 and device defines.

 @par Edit History - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.

 @par Copyright &copy; 2014 Anders Gustafsson All rights reserved..

*/

#ifndef _LIFUTILS_H
#define _LIFUTILS_H


#ifndef LIF_STAND_ALONE
#include "user_config.h"
#include "defines.h"
#endif

extern int debuglevel;
typedef struct stat stat_t;

///Depends on how much free ram we have
#define LIF_SECTOR_SIZE 256

///@brief LIF directory entry size
#define LIF_DIR_SIZE 32
#define LIF_DIR_RECORDS_PER_SECTOR (LIF_SECTOR_SIZE/LIF_DIR_SIZE)

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
    uint8_t  Label[6+1];		    // 2
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
} lifvol_t;

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
    uint8_t  filename[10+1];	// 0
    uint16_t FileType;			// 10
    uint32_t FileStartSector;	// 12
    uint32_t FileSectors;		// 16
    uint8_t  date[6];           // 20 File date in BCD (YY MM DD HH MM SS)
    uint16_t VolNumber;			// 26
    uint16_t FileBytes;			// 30
    uint16_t SectorSize;		// 28
} lifdir_t;


///@brief Master LIF data structure
/// Contains image file name
/// Volume Structure
/// Current Directory Entry
/// read/write flag
typedef struct 
{
    char *name; 			// LIF image file name
	FILE *fp;				// LIF file handel
	uint32_t sectors;		// LIF image size in sectors
	uint32_t imagebytes;		// LIF image size in bytes
	uint32_t filestart;     // LIF file area start in sectors
	uint32_t filesectors;	// LIF file area size in sectors
	uint32_t usedsectors;	// Used sector count
	uint32_t freesectors;	// Free sector count
	lifvol_t VOL;			// LIF Volume header
	lifdir_t DIR;			// LIF directory entry
	int	   	files;			// File count
	int	   	purged;			// Purged file count
    int    	dirindex;		// Directory index 0..N
	int     EOFindex;		// Position of EOF directory record
} lif_t;

// =============================================


/* lifutils.c */
#ifdef LIF_STAND_ALONE
void trim_tail ( char *str );
void V2B_MSB ( uint8_t *B , int index , int size , uint32_t val );
void V2B_LSB ( uint8_t *B , int index , int size , uint32_t val );
uint32_t B2V_MSB ( uint8_t *B , int index , int size );
uint32_t B2V_LSB ( uint8_t *B , int index , int size );
#endif

MEMSPACE void lif_help ( void );
MEMSPACE int lif_tests ( int argc , char *argv []);
MEMSPACE void *lif_calloc ( long size );
MEMSPACE void lif_free ( uint8_t *p );
MEMSPACE void *lif_stralloc ( char *str );
MEMSPACE FILE *lif_open ( char *name , char *mode );
MEMSPACE stat_t *lif_stat ( char *name , stat_t *p );
MEMSPACE int lif_seek_msg ( FILE *fp , long offset , char *msg );
MEMSPACE long lif_read ( lif_t *LIF , void *buf , long offset , int bytes );
MEMSPACE int lif_write ( lif_t *LIF , void *buf , long offset , int bytes );
MEMSPACE int lif_chars ( int c , int index );
MEMSPACE int lif_B2S ( uint8_t *B , uint8_t *name , int size );
MEMSPACE int lif_checkname ( char *name );
MEMSPACE void lif_S2B ( uint8_t *B , uint8_t *name , int size );
MEMSPACE int lif_fixname ( uint8_t *B , char *name , int size );
MEMSPACE void lif_vol2str ( lif_t *LIF , uint8_t *B );
MEMSPACE void lif_str2vol ( uint8_t *B , lif_t *LIF );
MEMSPACE void lif_dir2str ( lif_t *LIF , uint8_t *B );
MEMSPACE void lif_str2dir ( uint8_t *B , lif_t *LIF );
MEMSPACE uint8_t lif_BIN2BCD ( uint8_t data );
MEMSPACE int lif_BCD2BIN ( uint8_t bin );
MEMSPACE void lif_time2lifbcd ( time_t t , uint8_t *bcd );
MEMSPACE time_t lif_lifbcd2time ( uint8_t *bcd );
MEMSPACE char *lif_ctime_gmt ( time_t *tp );
MEMSPACE char *lif_lifbcd2timestr ( uint8_t *bcd );
MEMSPACE void lif_image_clear ( lif_t *LIF );
MEMSPACE void lif_dir_clear ( lif_t *LIF );
MEMSPACE void lif_vol_clear ( lif_t *LIF );
MEMSPACE void lif_dump_vol ( lif_t *LIF, char *msg );
MEMSPACE int lif_check_volume ( lif_t *LIF );
MEMSPACE int lif_check_dir ( lif_t *LIF );
MEMSPACE lif_t *lif_create_volume ( char *imagename , char *liflabel , long dirstart , long dirsectors , long filesectors );
MEMSPACE void lif_close_volume ( lif_t *LIF );
MEMSPACE uint32_t lif_bytes2sectors ( uint32_t bytes );
MEMSPACE void lif_rewinddir ( lif_t *LIF );
MEMSPACE void lif_closedir ( lif_t *LIF );
MEMSPACE int lif_checkdirindex ( lif_t *LIF , int index );
MEMSPACE int lif_readdirindex ( lif_t *LIF , int index );
MEMSPACE int lif_writedirindex ( lif_t *LIF , int index );
MEMSPACE int lif_writedirEOF ( lif_t *LIF , int index );
MEMSPACE lifdir_t *lif_readdir ( lif_t *LIF );
MEMSPACE lif_t *lif_updatefree ( lif_t *LIF );
MEMSPACE int lif_newdir ( lif_t *LIF , long sectors );
MEMSPACE lif_t *lif_open_volume ( char *name , char *mode );
MEMSPACE void lif_dir ( char *lifimagename );
MEMSPACE int lif_find_file ( lif_t *LIF , char *liflabel );
MEMSPACE int lif_e010_pad_sector ( long offset , uint8_t *wbuf );
MEMSPACE int lif_ascii_string_to_e010 ( char *str , long offset , uint8_t *wbuf );
MEMSPACE long lif_add_ascii_file_as_e010_wrapper ( lif_t *LIF , uint32_t offset , char *username );
MEMSPACE long lif_add_ascii_file_as_e010 ( char *lifimagename , char *lifname , char *userfile );
MEMSPACE int lif_extract_e010_as_ascii ( char *lifimagename , char *lifname , char *username );
MEMSPACE int lif_extract_lif_as_lif ( char *lifimagename , char *lifname , char *username );
MEMSPACE long lif_add_lif_file ( char *lifimagename , char *lifname , char *userfile );
MEMSPACE int lif_del_file ( char *lifimagename , char *lifname );
MEMSPACE int lif_rename_file ( char *lifimagename , char *oldlifname , char *newlifname );
MEMSPACE long lif_create_image ( char *lifimagename , char *liflabel , uint32_t dirsectors , uint32_t sectors );

#endif     // #ifndef _LIFUTILS_H
