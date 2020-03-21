/**
 @file gpib/defines.h

 @brief GPIB, AMIGO, SS80 and device defines.

 @par Edit History - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Anders Gustafsson All rights reserved..

 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

*/

#ifndef _DRIVES_H
#define _DRIVES_H
/*-------------------------------------------------------------------------
  defines.h - global defines

             (c) 2014 Anders Gustafsson <anders.gustafsson@pedago.fi>

-------------------------------------------------------------------------*/
#include "user_config.h"
#include "posix.h"


///@brief defulats if not drives defined in sdcard config file
#ifdef AMIGO
#define HP9121D     //< HP9121 dual 270K AMIGO floppy drive
#endif
#define HP9134L     //< HP9134L 40M SS/80 Winchester drive

// =============================================
///@brief Maximum number of emulated devices
#define MAX_DEVICES 12

///@brief Maximun lengh of device file name
#define MAX_FILE_NAME_LEN 32

//@brief Drive index, Address, PPR and file name for emulated drive
typedef struct 
{
    uint8_t ADDRESS;    //< GPIB Address
    uint8_t PPR;        //< Parallel Poll Response Bit
    char     NAME[MAX_FILE_NAME_LEN+1]; // Filename of emulated image
} HeaderType;

//@brief Identify Bytes for Drives
typedef struct 
{
    uint16_t ID;        //<  Identify, For 9122 I1=02, I2=22H
} ConfigType;

// =============================================
/// @brief AMIGO emulator state machine index.
typedef struct
{
    uint8_t state;
    /// @brief AMIGO disk unit number
    uint8_t unitNO;
    /// @brief AMIGO disk volume number
    uint8_t volNO;
    uint8_t cyl;
    uint8_t head;
    uint8_t sector;
    uint8_t dsj;
    int Errors;
    /// @brief AMIGO disk status
    uint8_t status[4];
    /// @brief AMIGO disk address
    uint8_t logical_address[4];
} AMIGOStateType;

// =============================================
///@brief Printer structure 
typedef struct 
{
    HeaderType HEADER;
} PRINTERDeviceType;

// =============================================
///@brief Plotter file data structure definition used for saving plot data.
typedef struct 
{
    uint32_t count;                               // total bytes
    int16_t ind;                                  // buffer cache index
    int16_t size;                                 // buffer size
    uint8_t error;                                // error status
    FILE *fp;
    char *buf;
} PRINTERStateType;

// =============================================
typedef struct
{
    int16_t BYTES_PER_SECTOR;
    int16_t SECTORS_PER_TRACK;
    int16_t HEADS;
    int16_t CYLINDERS;
} AMIGOGeometryType;

///@brief AMIGO Disk structure - ID bytes and layout.
typedef struct 
{
    HeaderType HEADER;
    ConfigType CONFIG;
    AMIGOGeometryType GEOMETRY;
} AMIGODiskType;


// =============================================
///@brief SS80 Controller 
typedef struct {
    /*
        When packed this is 5 bytes
        CONTROLLER DESCRIPTION
        C1-C2    C1 = MSB, C2 = LSB
                 Installed unit byte; 1 bit for each unit.
                 Unit 15 is always present
        C3-C4    GPIB data transfer rate in kB/s on the bus
        C5       Controller Type
                 0 = CS/80 integrated single unit controller.
                 1 = CS/SO integrated multi-unit controller.
                 2 = CS/SO integrated multi-port controller.
                 4 = SS/SO integrated single unit controller.
                 5 = SS/80 integrated multi-unit controller.
                 6 SS/80 integrated multi-port controller.


    */
    uint16_t UNITS_INSTALLED;
    uint16_t TRANSFER_RATE;
    uint8_t TYPE;
} SS80ControllerType;

///@brief SS80 Unit 
typedef struct   //<  Unit description, 19 bytes
{
    /*
        When packed this is 19 bytes
        UNIT DESCRIPTION
        U1      Generic Unit Type, 0 = fixed, 1 - floppy, 2 = tape
                OR with 128 implies dumb can not detect media change
        U2-U4   Device number in BCD
                XX XX XY, X = BCD Unit number, Y = option number
        U5-U6   Number of bytes per block, MSB-LSB
        U7      Number of blocks which can be buffered
        U8      0 for SS80, Recommended burst size
        U9-U10  Block time in microseconds
        U11-U12 Continuous average transfer rate for long transfers kB/s
        U13-U14 Optimal retry time in 1O's of milliseconds
        U15-U16 Access time parameter in 1O's of milliseconds
        U17     Maximum Interleave factor
        U18     Fixed volume byte; one bit per volume (set if fixed);
        U19     Removable volume byte; one bit per volume (set if removable);
    */
    uint8_t UNIT_TYPE;
    uint32_t DEVICE_NUMBER;
    uint16_t BYTES_PER_BLOCK;
    uint8_t BUFFERED_BLOCKS;
    uint8_t BURST_SIZE;
    uint16_t BLOCK_TIME;
    uint16_t CONTINOUS_TRANSFER_RATE;
    uint16_t OPTIMAL_RETRY_TIME;
    uint16_t ACCESS_TIME;
    uint8_t MAXIMUM_INTERLEAVE;
    uint8_t FIXED_VOLUMES;
    uint8_t REMOVABLE_VOLUMES;
} SS80UnitType;

///@brief Volume Information Structure
typedef struct 
{
    /*
        When packed this is 13 bytes
        VOLUME DESCRIPTION
        SS80 units use single vecor mode so MAX CYLINDER,HEAD and SECTOR are not used
        V1-V3  Maximum value of cylinder address vector.
        V4     Maximum value of the head address vector.
        V5-V6  Maximum value of sector address vector.
               For devices that use bot the following expression must be true
               (V1,V2,V3+1)(V4+1)(V5,V6+1) = V7,V8,V9,V10,V11,V12+1
        V7-V12 Maximum value of single vector address in blocks.
        V13    Current Interleave Factor
    */
    uint32_t MAX_CYLINDER;
    uint8_t MAX_HEAD;
    uint16_t MAX_SECTOR;
    uint32_t MAX_BLOCK_NUMBER;
    uint8_t INTERLEAVE;
} SS80VolumeType;


///@brief Disk Information Structure
typedef struct 
{
    HeaderType HEADER;
    ConfigType CONFIG;
    SS80ControllerType CONTROLLER;
    SS80UnitType UNIT;
    SS80VolumeType VOLUME;
} SS80DiskType;
// =============================================

///@brief SS80 Emulated disk state information
typedef struct
{
    /// @brief Execute state index
    int estate;
    /// @brief Qstat variable
    uint8_t qstat;
    ///@brief Errors
    int Errors;         //< Error byte
    ///@brief SS80 Unit 
    BYTE unitNO;        //< Unit Number - we only do 1
    ///@brief SS80 Volume 
    BYTE volNO;         //< Volume Number - we only do 1
    ///@brief Address in Blocks
    uint32_t AddressBlocks; 
    ///@brief Length in Bytes
    uint32_t Length;
} SS80StateType;

// =============================================
enum PARSE_STATES
{
    START_STATE,
    SS80_STATE,
    SS80_HEADER,
    SS80_CONFIG,
    SS80_CONTROLLER,
    SS80_UNIT,
    SS80_VOLUME,
    CONTROLLER_STATE,
    CONTROLLER_CONTROLLER,
    CONTROLLER_UNIT,
    AMIGO_STATE,
    AMIGO_HEADER,
    AMIGO_CONFIG,
    AMIGO_GEOMETRY,
    PRINTER_STATE,
    PRINTER_CONFIG
};

// =============================================
///@brief
enum DEVICE_TYPES
{
    NO_TYPE,    
    AMIGO_TYPE,
    SS80_DEFAULT_TYPE,
    SS80_TYPE,
    PRINTER_TYPE
};

#define MODEL_SIZE 32

///@brief Device Type 
typedef struct
{
    uint8_t  TYPE;   // TYPE SS80,AMIGO or PRINTER TYPE
    uint8_t  ADDRESS;// ADDRESS
    uint8_t  PPR;    // PPR
	uint32_t BLOCKS;// Disk Blocks
	char     model[MODEL_SIZE];
    void     *dev;      // Disk or Printer Structure
    void     *state;    // Disk or Printer State Structure
} DeviceType;
// =============================================
///@convert print_var strings into __memx space
#define print_var(format, args...) print_var_P(PSTR(format), ##args)
///@convert print_var strings into __memx space
#define print_str(format, args...) print_str_P(PSTR(format), ##args)


extern SS80DiskType *SS80p;
extern SS80StateType *SS80s;
#ifdef AMIGO
extern AMIGODiskType *AMIGOp;
extern AMIGOStateType *AMIGOs;
#endif
extern PRINTERDeviceType *PRINTERp;
extern DeviceType Devices[MAX_DEVICES];

/// =================================================
///@brief Data structure for hpdir.ini HPDir project disk parameters 
///
/// This structure will containes the parsed and computed
/// disk parameters from a single hpdir disk entry
///
///; AMIGIO/CS80/SS80 disk drives
///; 1 - SS80 [HP9134L]            unique drive model identifier (should be identical to [fsinfo] section above)
///; 2 - COMMENT                   descriptive string
///; 3 - SS80/AMIGO:               drive protocol AMIGO, CS80 or SS80
///; 4 - ID:                       ID returned by AMIGO identify
///; 5 -                           mask for secondary ID returned by high byte of STAT2
///; 6 -                           ID returned by high byte of STAT2
///; 7 - DEVICE_NUMBER:            drive number returned by CS/80 describe command
///; 8 - UNITS_INSTALLED(0x8001):  number of units installed
///; 9 - MAX_CYLINDER:             number of cylinders
///;10 - MAX_HEAD:                 number of heads or surfaces
///;11 - MAX_SECTOR:               number of sectors per track
///;12 - BYTES_PER_BLOCK:          number of bytes per block or sector
///;13 - INTERLEAVE:               default interleave
///;14 - media type 0=fixed media, 1=removable media
///; # Removable volume byte; one bit per volume (set if removable)
///; 1,                                           x,           3,      4,    5,    6,        7, 8,    9, 10, 11,  12,13,14
/// Example entry
///9895    = "HP9895A dual 1.15M AMIGO floppy disc",       AMIGO, 0x0081, 0x00, 0x00, 0x000000, 1,   77,  2, 30, 256, 7, 1
///
///@return void


typedef struct {
    char model[MODEL_SIZE];	// 1 HP Model number
    char comment[64];  		// 2
	char TYPE[32];     		// 3 SS80,CS80,AMIGO
    long ID;				// 4 Request Identify ID
	long mask_stat2;		// 5
	long id_stat2;			// 6
	long DEVICE_NUMBER;		// 7 BCD part of model number * 10
	long UNITS_INSTALLED; 	// 8	ALWAYS 1 , FIXED
	long CYLINDERS;	    	// 9
	long HEADS;				// 10
	long SECTORS;			// 11
	long BYTES_PER_BLOCK;	// 12
	long INTERLEAVE;		// 13
    long FIXED;				// 14 ALWAYS 1
	// Computed values
	long BLOCKS;
	long LIF_DIR_BLOCKS;
} hpdir_t;
// =============================================

/* drives.c */
void V2B_MSB ( uint8_t *B , int index , int size , uint32_t val );
void V2B_LSB ( uint8_t *B , int index , int size , uint32_t val );
uint32_t B2V_MSB ( uint8_t *B , int index , int size );
uint32_t B2V_LSB ( uint8_t *B , int index , int size );
int find_type ( int type );
int count_drive_types ( uint8_t type );
char *type_to_str ( int type );
char *base_to_str ( int base );
int find_free ( void );
int find_device ( int type , int address , int base );
int set_active_device ( int index );
void SS80_Set_Defaults ( int index );
int alloc_device ( int type );
void init_Devices ( void );
int push_state ( int state );
int pop_state ( void );
bool assign_value ( char *str , uint32_t minval , uint32_t maxval , uint32_t *val );
void set_Config_Defaults ( void );
void hpdir_init ( void );
long lif_dir_count ( long blocks );
void hpdir_parameters ( int index, char *model );
int Read_Config ( char *name );
void print_var_P ( __memx const char *str , uint32_t val );
void print_str_P ( __memx const char *str , char *arg );
void display_Addresses ( void );
void display_Config ( void );
void format_drives ( void );

// =============================================
#endif     // _DRIVES_H
