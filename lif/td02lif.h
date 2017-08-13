/**
  @file   td02lif.c
  @brief  TeleDisk decoder library targetting LIF format images only
  @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved. GPL
  @see http://github.com/magore/hp85disk
  @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

  @help
   * LIF command help
     * lif td02lif image.td0 image.lif
       * Convert TeleDisk encoded LIF disk image into to pure LIF image
   * TELEDISK command help
     * td02lif file.td0 file.lif
       * Convert TeleDisk encoded LIF disk image into to pure LIF image

  @par Edit History
  - [1.0]   [Mike Gore]  Initial revision of file.

  @Credits
   * lif/teledisk
     * My TELEDISK LIF extracter
       * Note: The TeleDisk image MUST contain a LIF image  - we do NOT translate it
     * README.txt
       * Credits
     * Important Contributions (My converted would not have been possible without these)
       * Dave Dunfield, LZSS Code and TeleDisk documentation
         * Copyright 2007-2008 Dave Dunfield All rights reserved.
         * td0_lzss.h
         * td0_lzss.c
           * LZSS decoder
         * td0notes.txt
           * Teledisk Documentation
       * Jean-Franois DEL NERO, TeleDisk Documentation
         * Copyright (C) 2006-2014 Jean-Franois DEL NERO
           * wteledsk.htm
             * TeleDisk documenation
           * See his github project
             * https://github.com/jfdelnero/libhxcfe
*/


#ifndef _TD02LIF_H_
#define _TD02LIF_H_


///@brief Size of TeleDisk image header
#define TD_HEADER_SIZE  12
///@brief Size of TeleDisk comment header
#define TD_COMMENT_SIZE 10
///@brief Size of TeleDisk track header size
#define TD_TRACK_SIZE   4
///@brief Size of TeleDisk sector header size
#define TD_SECTOR_SIZE  6

///@brief Maximum number of sectors per track
///Used for sector data and sorting tables
#define MAXSECTOR 256

typedef struct tm tm_t;
extern int debuglevel;

///@brief Encoding format td_track.PSide & 0x80 values
enum 
{
    IBMFORMAT_SD,
    IBMFORMAT_DD
};

///@brief TD0 to LIF state machine states
enum
{
    TD0_INIT,
    TD0_START,
    TD0_WAIT_DIRECTORY,
    TD0_DIRECTORY,
    TD0_WAIT_FILE,
    TD0_FILE,
    TD0_DONE
};


///@brief TeleDisk Image Header
typedef struct 
{
	uint8_t	Header[2+1];    // Teledisk header + room for EOS
	uint8_t	VolNO;		    // Volume sequence number
	uint8_t	ChkSig;		    // Check signature 
                                // Used only for multi volume support
                                // All volumes must have the same value
	uint8_t	TDVersion;      // Teledisk version, (11 = v1.1)
	uint8_t	Density;	    // Source disk density 
                                // 0 = 250K
                                // 1 = 300K
                                // 2 = 500K 
                                // 128 = single-density FM)
	uint8_t	DriveType;      // Source drive type 
                                // 1 = 360K
                                // 2 = 1.2M
                                // 3 = 720K
                                // 4 = 1.44M
	uint8_t	TrackDensity;   // Track Density with respect to source density
                                // 0 = single sensity
                                // 1 = double density
                                // 2 = quad density)
                                // If bit 7 is set we have a comment block
	uint8_t	DosMode;	    // DOS allocation information used ?
                                // !0 teledisk used DOS allocation info
                                // When creating image
	uint8_t	Sides;	        // Disk sides stored in the image
	uint16_t CRC;		    // 16-bit CRC for this header
} td_header_t;

///@brief Comment header
/// Only used when bit 7 of TrackDensity in TeleDisk Header is set
typedef struct 
{
	uint16_t CRC;           // Comment block CRC
	uint16_t Size;          // Comment Length
	uint8_t Year;           // Year     -1900 offset
    uint8_t Month;          // Month    0..11
    uint8_t Day;            // Day      1..31
    uint8_t Hour;           // Hour     0..23
    uint8_t Minute;         // Minute   0..59
    uint8_t Second;         // Second   0..59
} td_comment_t;

///@brief Track Header
/// Contains the Physical Sectors, Cylinder, Side number
/// From the original physical disk used to make the TeleDIsk image
typedef struct 
{
	uint8_t PSectors;       // Physical sectors in this track
                                // 0xff implies end of TeleDisk image
	uint8_t PCyl;			// Physical (device) track
	uint8_t PSide;			// PSide & 0x7f = Physical (device) side 
                                // PSide & 0x80 = IBM format density
                                // 0x00 = IBM Double Desnity
                                // 0x80 = IBM Single Sensity
	uint8_t CRC;			// LSB of first 3 bytes of track header CRC16
} td_track_t;

///@brief Sector Header
/// Contents of the Sector ID fields which preceeded the sector data
/// From the original physical disk used to make the TeleDIsk image
typedef struct 
{
	uint8_t Cyl;			// Logical Track number 
	uint8_t Side;			// Logical Side
	uint8_t Sector;         // Logical Sector number 
	uint8_t SizeExp;        // Size = 2 ** Slen
                            // We support 0=128,1=256,2=512,4=1024
	uint8_t Flags;          // Sector bit flags
                            // 0x01 = Unsupported ?
                            // 0x02 = Unsupported use alternate crc
                            // 0x04 = Unsupported use altrenate data CRC
                            // 0x08 = Unsupported ?
                            // 0x10 = Unsupported ?
                            // 0x20 = Unsuported missing data address mark
                            // 0x40 = Unsupported ?
                            // 0x80 = Unsupported ?
	uint8_t CRC;		    // LSB of sector data CRC16
} td_sector_t;


///@brief Track sector information and sector data
typedef struct {
    int cylinder;           // Cylinder
    int side;               // Side
    int sector;             // Sector number
    int sectorsize;         // Sector size in bytes
    long bitrate;           // Bit rate - not used
    int encoding;           // Encoding flag = (td_track.PSide & 0x80)
    uint8_t *data;          // Sector data
} track_data_t;


#define TD0_SECTORFIRST     1
#define TD0_SECTORSIZE      2
#define TD0_SECTORPERTRACK  4
#define TD0_SIDE            8
#define TD0_TRACKS          16
#define TD0_OVERRIDE        128

typedef struct
{
    int flags;
    int sectorfirst;
    int sectorsize;
    int sectorspertrack;
    int sides;
    int tracks;
} usertel_t;


///@brief td02lif state structure
typedef struct
{
    int error;
    int state;
    int sectorspertrack;
    int sectorsize;
    int sectorfirst;
    int sectorlast;
    long sectorindex;
    long writeindex;
    usertel_t u;            // User override flags
    time_t  t;              // LIF image date in epoch format
} liftel_t;

extern liftel_t liftel;

/* td02lif.c */
int td0_unpack_disk_header ( uint8_t *B , td_header_t *p );
int td0_unpack_comment_header ( uint8_t *B , td_comment_t *p );
int td0_unpack_track_header ( uint8_t *B , td_track_t *p );
int td0_unpack_sector_header ( uint8_t *B , td_sector_t *p );
void td0_enable_decompress ( int flag );
long td0_read ( void *p , int osize , int size , FILE *fp );
void td0_init_trackdata ( int freemem );
int td0_rle ( uint8_t *dst , uint8_t *src , int max );
void td0_track ( int index );
void td0_sector ( td_sector_t *P );
long td0_density2bitrate ( uint8_t density );
int td0_read_image ( char *imgfile , lif_t *LIF );
int td02lif_sector ( uint8_t *data , int size , lif_t *LIF );
void td0_help ( int full );
int td02lif ( int argc , char *argv []);


#endif
