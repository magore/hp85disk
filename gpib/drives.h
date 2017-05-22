/**
 @file gpib/defines.h

 @brief GPIB, AMIGO, SS80 and device defines.

 @par Edit History - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Anders Gustafsson All rights reserved..

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#ifndef DRIVES_H
#define DRIVES_H
/*-------------------------------------------------------------------------
  defines.h - global defines

             (c) 2014 Anders Gustafsson <anders.gustafsson@pedago.fi>

-------------------------------------------------------------------------*/
#include "user_config.h"



#define HP9121D     //< HP9121 dual 270K AMIGO floppy drive
#define HP9134L     //< HP9134L 40M SS/80 Winchester drive

/// ==============================================================
///@brief address and PPR for SS80 AMIGO and PRINTER
/// Can be set if specified in user config
/// If NOT specified see Power on Defaults below
///@see ss80.c

#define SS80_MLA     (BASE_MLA + SS80Disk.HEADER.ADDRESS)    //<  SS80 listen address 
#define SS80_MTA     (BASE_MTA + SS80Disk.HEADER.ADDRESS)   //<  SS80 talk address 
#define SS80_MSA     (BASE_MSA + SS80Disk.HEADER.ADDRESS)   //<  SS80 seconday address 
#define SS80_PPR     (SS80Disk.HEADER.PPR)             //<  SS80 PPR Address
#define AMIGO_MLA    (BASE_MLA + AMIGODisk.HEADER.ADDRESS)  //<  AMIGO listen address
#define AMIGO_MTA    (BASE_MTA + AMIGODisk.HEADER.ADDRESS)  //<  AMIGO talk address 
#define AMIGO_MSA    (BASE_MSA + AMIGODisk.HEADER.ADDRESS)  //<  AMIGO seconday address
#define AMIGO_PPR    (AMIGODisk.HEADER.PPR)            //<  AMIGO PPR Address

#define PRINTER_MLA  (BASE_MLA + printer_addr) //<  PRINTER listen address 
#define PRINTER_MTA  (BASE_MTA + printer_addr) //<  PRINTER talk address 
#define PRINTER_MSA  (BASE_MSA + printer_addr) //<  PRINTER seconday address 


/// ====================================================================
typedef struct 
{
	uint16_t ID; 		//<  Identify, For 9122 I1=02, I2=22H
} ConfigType;

typedef struct 
{
	uint8_t DRIVE;		// Emulated Drive number
	uint8_t ADDRESS;	//< GPIB Address
	uint8_t PPR;		//< Parallel Poll Response Bit
	char     NAME[32];
} HeaderType;


/// ====================================================================
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

/// ====================================================================

typedef struct
{
	int16_t BYTES_PER_SECTOR;
	int16_t SECTORS_PER_TRACK;
	int16_t HEADS;
	int16_t CYLINDERS;
} AMIGOGemometryType;

///@brief AMIGO Disk structure - ID bytes and layout.
typedef struct 
{
	HeaderType HEADER;
	ConfigType CONFIG;
    AMIGOGemometryType GEOMETRY;
} AMIGODiskType;



/// ====================================================================
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

	

///@brief SS80 Controller 5 bytes
typedef struct {
	/*
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

///@brief SS80 Unit 19 bytes
typedef struct   //<  Unit description, 19 bytes
{
	/*
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
typedef struct {
	HeaderType HEADER;
	ConfigType CONFIG;
    SS80ControllerType CONTROLLER;
    SS80UnitType UNIT;
    SS80VolumeType VOLUME;
} SS80DiskType;


enum PARSE_STATES
{
	START_STATE,
	SS80_STATE,
	SS80_HEADER,
	SS80_CONFIG,
	SS80_CONTROLLER,
	SS80_UNIT,
	SS80_VOLUME,
	AMIGO_STATE,
	AMIGO_HEADER,
	AMIGO_CONFIG,
	AMIGO_GEOMETRY,
	PRINTER_STATE,
	PRINTER_CONFIG
};


extern SS80DiskType SS80Disk;
extern SS80StateType SS80State;
extern AMIGODiskType AMIGODisk;
extern AMIGOStateType AMIGOState;
///@brief printer do not use parallel poll
extern uint8_t printer_addr;

/* drives.c */
void V2B ( uint8_t *B , int index , int size , uint32_t val );
uint32_t B2V ( uint8_t *B , int index , int size );
int POSIX_Read_Config ( char *name );
void display_Config ( void );
void display_settings ( void );


/// =================
#endif     // DRIVES_H
