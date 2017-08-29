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
#define MAXSECTORS 256

///@brief Maximum number of sides per cyinder
///TeleDisk only trargetted floppies with 2 sides max!
#define MAXSIDES 2

///@brief Maximum number of tracks per disk
#define MAXCYL 256

#define MAXTRACKS (MAXCYL * MAXSIDES)


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


///@brief Sector information and Sector "ID"
///
/// Disk Terminology: (From the original disk - not the TeleDisk encoding)
///     A "Disk" includes "Formatting" overhead, "sectors" ("Data") grouped in a concentry circle ("Track") 
///       on all surfaces ("Cylinder") as a "Disk" (all cylinders)
///
///     A "Sector" contains the "Data" - each "Sector" is preceeded by formatting overhead called the "ID" field.
///     The "Formatting" - is the invisible overhead such as the sector "ID" preceeding every sector.
///     The "ID" field contains Cylinder, Side, Sector number, Sector Size and a CRC checksum 
///     The "Data" part of the sector follows after the "ID" field. The "Data has its own formatting CRC checksum
///     A "Track" is a group of "Sectors" in one such circle on just one surface. 
///     A "Cylinder" is this "Track" but now including all "Surfaces"
///     A "Disk" is the collection of all "Cylinders"
///
///
/// Note: The values Cylinder,Side and Sectors Number - these need NOT match Physical values!;
///    The lowest sector number usually start with 0 or 1 on side 1 and side 2
///    Many disks repeat the same sector numbering on each side, However;
///    Some disks continue numbering sectors AFTER the last one on side 1
///    Frequently Sectors numbering can be interleaved, like 0 4 8 1 5 9 ...
///    The Side and Cylinder Number usually match physical values on "normal" disks.
/// 

typedef struct {
    int cylinder;           // Sectory ID Cylinder 
    int side;               // Sector ID Side
    int sector;             // Sector ID Sector Number 
    int size;               // Sector ID Sector Size  converted to Bytes
    uint8_t *data;          // Sector Data
} sector_t;

typedef struct {
    int Cyl;        // From TeleDisk Track Header Cylinder Number
    int Side;       // From TeleDisk Track Header Side Number
    int Sectors;    // TeleDisk count of Sectors on this track
    int First;
    int Last;
    int Size; // Size of first sector
    sector_t sectors[MAXSECTORS];   // Disk Sectors
} track_t;


typedef struct {
    FILE            *fi;                // TeleDisk image file handle
    char            *td0_name;          // TeleDisk file name
    int             compressed;         // TeleDIsk Image is compressed
    time_t          t;                  // LIF image date in epoch format
    td_header_t     td_header;          // TeleDisk Header
    td_comment_t    td_comment;         // Comment Header
    uint8_t         *comment;           // Optional comment string if td_comment.Size != 0
    track_t         track[MAXTRACKS];   // Track and Sector Data
} disk_t;



///@brief Master TeleDisk Format Analisis structure
/// We look for the specifications of LIF image stored inside the TeleDisk image
/// We Examine the first 30 tracks (just some number less then 35)
typedef struct
{
    // TD0 to LIF stae information
    int error;              // TD0 to LIF error state
    int state;              // TD0 to LIF process state machine
    long sectorindex;       // Sector index reading LIF image inside TeleDisk image
    long writeindex;        // Sector offset writting LIF image

    /// Parameters of LIf image inside TeleDisk image - AFTER analisis
    int             Sectors;            // LIF image Sectors
    int             Size;               // LIF image Sector Size
    int             Tracks;             // LIF image Tracks
    int             Sides;              // LIF image Sides
    int             Cylinders;          // LIF image Cylinders

/// Parameters of LIf image inside TeleDisk image - durring analisis
/// FIRST, LAST sectors (with numbers < 100) on each side
/// SIZE of fist sector on each side
/// SECTOR count matching size on each side
    struct 
    {
        int first[2];       // First - lowest sector number found in 30 tracks on each side
        int size[2];        // Size - of First Sector on each side
        int last[2];        // Last - highest sector number found (<100) in 30 tracks on each side
        int sectors[2];     // Sectors - maximum number of sectors found per track in 30 tracks matching Size on each side
    } s;

/// User overrides to aid in format analisis
/// Normally NOT needed bacause detailed format analisis from the first 30 cylinders 
///   generally gets the correct values.
/// Note: Some overrides will be ignored if format analisis clearly rules them out
///
/// Number of Sides Override:
///     Unfortunately we may have an image that was taking of a disk that was subject 
///        to multiple formates prior to imaging
///     Consider an 80 track, 2 sided disk with 9 512 bytes sectors that is reformatted 
///        to 35 tracks single sided with 16 sectors
///     Format analisis will always detect this and switch to single sided mode.
///     However if we have a 80 track two sided disk that is reformatted to a 80 track 
///        single sided analisis may not detect this.
///     You CAN specify single sided override for two sided disks  
///        (as defined in the TeleDisk headers)
///     However you can NOT specify 2 sided mode if the TeleDisk image headers 
///        have only 1 side defined.
///
/// Sector Size override:
///     We normally use the size of the sectors found with the LOWEST number in 35 tracks
///     If the format has mixed sector sizes this will help when this rule is not true
///     Note: The LIF images types I support have FIXED sectors sizes 
///           so using sector size override will likely not be that useful
///
/// Number of tracks overrides
///     If you also want to save the blank data after the last file you can specify tracks
///     The LIF decoder stops after the last sector of the last file so this option is not needed to save the LIF data
    struct 
    {
        int size;           // User size override, ONLY use this option if sector with the LOWEST number is does NOT have default SIZE 
        int sides;          // User sides overrride, you can specify LESS then found but not MORE then analise pass
        int tracks;         // user Tracks override
    } u;
} liftel_t;


extern liftel_t liftel;

/* td02lif.c */
int td0_unpack_disk_header ( uint8_t *B , td_header_t *p );
int td0_unpack_comment_header ( uint8_t *B , td_comment_t *p );
int td0_unpack_track_header ( uint8_t *B , td_track_t *p );
int td0_unpack_sector_header ( uint8_t *B , td_sector_t *p );
void td0_compressed ( int flag );
int td0_read ( void *p , int osize , int size , FILE *fp );
int td0_rle ( uint8_t *dst , uint8_t *src , int max );
void td0_trackinfo ( disk_t *disk , int trackind , int index );
void td0_sectorinfo ( td_sector_t *P );
long td0_density2bitrate ( uint8_t density );
int td0_open ( disk_t *disk , char *name );
int td0_read_disk ( disk_t *disk );
int td0_analize_format ( disk_t *disk );
int td0_save_lif ( disk_t *disk , lif_t *LIF );
int td0_save_lif_sector ( disk_t *disk , uint8_t *data , int size , lif_t *LIF );
void td0_help ( int full );
void td0_init_liftel ( void );
void td0_init_sectors ( disk_t *disk );
int td02lif ( int argc , char *argv []);


#endif
