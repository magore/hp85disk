/**
 @file gpib/drives.c
 @brief drive definitions for HP85 disk emulator project for AVR.
 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.
 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.
*/

#include "user_config.h"

#include <stdint.h>
#include "defines.h"
#include "drives.h"
#include "gpib_hal.h"
#include "gpib.h"
#include "gpib_task.h"
#include "amigo.h"
#include "ss80.h"
#include "format.h"
#include <time.h>

/// @brief Config Parser Stack
#define MAX_STACK 5
static int stack_ind = 0;
static int stack_p[MAX_STACK];

#define MAX_DEVICES 8
DeviceType Devices[MAX_DEVICES];

///@brief Active Printer Device
PRINTERDeviceType *PRINTERp = NULL;

///@brief Active SS80 Device
SS80DiskType *SS80p = NULL;
SS80StateType *SS80s = NULL;

#ifdef AMIGO
///@brief Active AMIGO Device
AMIGODiskType *AMIGOp = NULL;
AMIGOStateType *AMIGOs = NULL;
#endif

// =============================================
PRINTERDeviceType PRINTERDeviceDefault =
{
	{ 
		2,		// GPIB Address
		0xff,	// PPR unused
		"/printer.txt"
	}
};

// =============================================
///@brief SS80 Disk Definitions

#if defined(HP9122D)
///@brief SS80 HP9122D Disk Definitions
SS80DiskType SS80DiskDefault =
{
	{
		0,          // GPIB Address
		0,          // PPR
		"/ss80.lif"	// FILE name
	},
	{
		0x222,      // ID
	},
	{
		0x8001,     // Installed Units 1 (15 is always on)
		744,        // GPIB data transfer rate in kB/s on the bus
		5,          // 5 = SS/80 integrated multi-unit controller
	},
	{
		0,			// Generic Unit Type, 0 = fixed
		0x92200,    // BCD Device number XX XX XY, X=Unit, Y=option
		0x100,      // Bytes per block
		1,			// Buffered Blocks
		0,			// Burst size = 0 for SS80
		5888,		// Block time in microseconds
		45,			// Continous transfer time in kB/s
		4500,		// Retry time in 0.01 secods
		4500,		// Access time in 0.01 seconds
		15,			// Maximum interleave factor
		0,			// Fixed volume byte, one bit per volume
		1			// Removable volume byte, one bit per volume
	},
	{
		0,			// Maximum Cylinder  - not used
		0,			// Maximum Head		 - not used
		0,			// Maximum Sector    - not used
		0x99f,		// Maximum Block Number
		2			// Interleave
	}
};
#endif // #if defined(HP9122D)

#if defined(HP9134L)
///@brief SS80 HP9134L Disk Definitions
SS80DiskType SS80DiskDefault =
{
	{
		0,          // GPIB Address
		0,          // PPR
		"/ss80.lif"	// FILE name
	},
	{
		0x221,      // ID
	},
	{
		0x8001,     // Installed Units 1 (15 is always on)
		744,        // GPIB data transfer rate in kB/s on the bus
		5,          // 5 = SS/80 integrated multi-unit controller
	},
	{
		0,			// Generic Unit Type, 0 = fixed
		0x091340,   // BCD Device number XX XX XY, X=Unit, Y=option
		0x100,      // Bytes per block
		1,			// Buffered Blocks
		0,			// Burst size = 0 for SS80
		0x1F6,      // Block time in microseconds
		140,        // Continous transfer time in kB/s
		4500,		// Retry time in 0.01 secods
		4500,		// Access time in 0.01 seconds
		31,         // Maximum interleave factor
		1,          // Fixed volume byte, one bit per volume
		0           // Removable volume byte, one bit per volume
	},
	{
		0,			// Maximum Cylinder  - not used
		0,			// Maximum Head		 - not used
		0,			// Maximum Sector    - not used
		0xe340,		// Maximum Block Number
		31			// Interleave
	}
};
#endif // #if defined(HP9134L)

#ifdef AMIGO

#if defined(HP9121D)
/// @brief  AMIGO D9121D ident Bytes per sector, sectors per track, heads, cylinders
AMIGODiskType AMIGODiskDefault =
{
	{
		1,			// GPIB Address
		1,          // PPR
		"/amigo.lif"	// FILE name
	},
	{
		0x0104,     // ID
	},
    {
		256,        // Bytes Per Sector
		16, 		// Sectors Per Track
		2, 			// Sides
		35			// Cylinders
	}
};
#endif // #if defined(HP9121D)

#if defined(HP9995A)
/// @brief  AMIGO D9885A ident Bytes per sector, sectors per track, heads, cylinders
AMIGODiskType AMIGODiskDefault =
{
	{
		1,			// GPIB Address
		1,          // PPR
		"/amigo.lif"	// FILE name
	},
	{
		0x0081,     // ID
	}
    {
		256,        // Bytes Per Sector
		30, 		// Sectors Per Track
		2, 			// Sides
		77			// Cylinders
	}
};
#endif // #if defined(HP9995A)

#if defined(HP9134A)
/// @brief  AMIGO D9134A ident Bytes per sector, sectors per track, heads, cylinders
AMIGODiskType AMIGODiskDefault =
{
	{
		1,			// GPIB Address
		1,          // PPR
		"/amigo.lif"	// FILE name
	},
	{
		0x0106,     // ID
	},
    {
		256,        // Bytes Per Sector
		31, 		// Sectors Per Track
		4, 			// Sides
		153			// Cylinders
	}
};
#endif // #if defined(HP9134A)

#endif // ifdef AMIGO




// =============================================
// =============================================
///@brief Convert Value into byte array
/// bytes are MSB ... LSB order
///@param B: byte array
///@param index: offset into byte array
///@param size: number of bytes to process
///@param val: Value to convert
///@return void
void V2B_MSB(uint8_t *B, int index, int size, uint32_t val)
{
    int i;
    for(i=size-1;i>=0;--i)
    {
        B[index+i] = val & 0xff;
        val >>= 8;
    }
}
// =============================================
///@brief Convert Value into byte array
/// bytes are LSB ... MSB order
///@param B: byte array
///@param index: offset into byte array
///@param size: number of bytes to process
///@param val: Value to convert
///@return void
void V2B_LSB(uint8_t *B, int index, int size, uint32_t val)
{
    int i;
    for(i=0;i<size;++i)
    {
        B[index+i] = val & 0xff;
        val >>= 8;
    }
}

///@brief Convert a byte array into a value
/// bytes are MSB ... LSB order
///@param B: byte array 
///@param index: offset into byte array
///@param size: number of bytes to process
///@return value
uint32_t B2V_MSB(uint8_t *B, int index, int size)
{
    int i;
    uint32_t val = 0;

    for(i=0;i<size;++i)
    {
        val <<= 8;
        val |= (uint8_t) (B[i+index] & 0xff);
    }
	return(val);
}

///@brief Convert a byte array into a value
/// bytes are LSB ... MSB order
///@param B: byte array 
///@param index: offset into byte array
///@param size: number of bytes to process
///@return value
uint32_t B2V_LSB(uint8_t *B, int index, int size)
{
    int i;
    uint32_t val = 0;

    for(i=size-1;i>=0;--i)
    {
        val <<= 8;
        val |= (uint8_t) (B[i+index] & 0xff);
    }
	return(val);
}




///@brief Pack VolumeLabelType data into bytes
///@param[out] B: byte vector to pack data into
///@param[int] T: VolumeLabelType structure pointer
///@return null
void LIFPackVolume(uint8_t *B, VolumeLabelType *V)
{
	V2B_MSB(B,0,2,V->LIFid);
	memcpy((void *)(B+2),V->Label,6);
	V2B_MSB(B,8,4,V->DirStartSector);
	V2B_MSB(B,12,2,V->System3000LIFid);
	V2B_MSB(B,14,2,0);
	V2B_MSB(B,16,4,V->DirSectors);
	V2B_MSB(B,20,2,V->LIFVersion);
	V2B_MSB(B,22,2,0);
	V2B_MSB(B,24,4,V->tracks_per_side);
	V2B_MSB(B,28,4,V->sides);
	V2B_MSB(B,32,4,V->sectors_per_track);
	memcpy((void *) (B+36),V->date,6);
}

///@brief UnPack VolumeLabelType data from bytes
///@param[in] B: byte vector to pack data into
///@param[out] T: VolumeLabelType structure pointer
///@return null
void LIFUnPackVolume(uint8_t *B, VolumeLabelType *V)
{
	V->LIFid = B2V_MSB(B,0,2);
	memcpy((void *)V->Label,(B+2),6);
	V->DirStartSector = B2V_MSB(B,8,4);
	V->System3000LIFid = B2V_MSB(B,12,2);
	V->zero1 = B2V_MSB(B,14,2);
	V->DirSectors = B2V_MSB(B,16,4);
	V->LIFVersion = B2V_MSB(B,20,2);
	V->zero2 = B2V_MSB(B,22,2);
	V->tracks_per_side = B2V_MSB(B,24,4);
	V->sides = B2V_MSB(B,28,4);
	V->sectors_per_track = B2V_MSB(B,32,4);
	memcpy((void *) V->date, (B+36),6);
}

///@brief Pack DireEntryType data into bytes
///@param[out] B: byte vector to pack data into
///@param[int] D: DirEntryType structure pointer
///@return null
void LIFPackDir(uint8_t *B, DirEntryType *D)
{
	memcpy(B+0,D->filename,10);
	V2B_MSB(B,10,2,D->FileType);
	V2B_MSB(B,12,4,D->FileStartSector);
	V2B_MSB(B,16,4,D->FileLengthSectors);
	memcpy(B+20,D->date,6);
	V2B_MSB(B,26,2,D->VolNumber);
	V2B_MSB(B,28,2,D->FileBytes);
	V2B_MSB(B,30,2,D->SectorSize);
}

///@brief UnPack DireEntryType data from bytes
///@param[in] B: byte vector to extract data from
///@param[int] D: DirEntryType structure pointer
///@return null
void LIFUnPackDir(uint8_t *B, DirEntryType *D)
{
	memcpy(D->filename,B,10);
	D->FileType = B2V_MSB(B, 10, 2);
	D->FileStartSector = B2V_MSB(B, 12, 4);
	D->FileLengthSectors = B2V_MSB(B, 16, 4);
	memcpy(D->date,B+20,6);
	D->VolNumber = B2V_MSB(B, 26, 2); 
	D->FileBytes = B2V_LSB(B, 28, 2);
	D->SectorSize= B2V_LSB(B, 30, 2);
}

/// @brief Convert number >= 0 and <= 99 to BCD.
///
///  - BCD format has each hex nibble has a digit 0 .. 9
///
/// @param[in] data: number to convert.
/// @return  BCD value
/// @warning we assume the number is in range.
uint8_t BIN2BCD(uint8_t data)
{
    return(  ( (data/10U) << 4 ) | (data%10U) );
}


///@brief UNIX time to LIF time format
///@param[out] bcd: packed 6 byte BCD LIF time
///   YY,MM,DD,HH,MM,SS
///@param[in] t: UNIX time_t time value
///@see time() in time.c
///@return void
void time_to_LIF(uint8_t *bcd, time_t t)
{
	tm_t tm;
	localtime_r((time_t *) &t, (tm_t *)&tm);
	bcd[0] = BIN2BCD(tm.tm_year & 100);
	bcd[1] = BIN2BCD(tm.tm_mon);
	bcd[2] = BIN2BCD(tm.tm_mday);
	bcd[3] = BIN2BCD(tm.tm_hour);
	bcd[4] = BIN2BCD(tm.tm_min);
	bcd[5] = BIN2BCD(tm.tm_sec);
}





///@brief Seach Devices[] for ANY definitions of a disk type
///@param type: disk type like SS80_TYPE
//@return Devices[] index fopr matching type
int find_type(int type)
{
	int i;
	for(i=0;i<MAX_DEVICES;++i)
	{
		if( Devices[i].TYPE == type)
			return(i);
	}
	return(-1);
}

///@brief Convert a disk type into a string
///@param type: disk type like SS80_TYPE
///@return string pointer
char *type_to_str(int type)
{
	if(type == NO_TYPE)
		return("NO_TYPE");
	else if(type == AMIGO_TYPE)
		return("AMIGO_TYPE");
	else if(type == SS80_TYPE)
		return("SS80_TYPE");
	else if(type == PRINTER_TYPE)
		return("PRINTER_TYPE");
	return("INVALID TYPE");
}
///@brief Convert base address into a string identifier
///@param base: BASE_MAL, BASE_MTA or BASE_MSA only
///@return string "MLA", "MTA", "MSA" or error string
char *base_to_str(int base)
{
	if(base == BASE_MLA)
		return("MLA");
	else if(base == BASE_MTA)
		return("MTA");
	else if(base == BASE_MSA)
		return("MSA");
	return("*INVALID BASE*");
}

///@brief Find first free Devices[] slot
///@return Devices[] index of free slot or -1
int find_free()
{
	return(find_type(NO_TYPE));
}

///@brief Find a device with matching type AND address
///@param type: disk type
///@param address: GPIB device address 0 based
///@param base: BASE_MLA,BASE_MTA or BASE_MSA address range
///@return index of Devices[] or -1 if not found
int find_device(int type, int address, int base)
{
	int i;

	///@skip Only interested in device addresses
	if(address < BASE_MLA || address >(BASE_MSA+30))
		return(-1);

	///@brief Make sure address is in expected range
	if(address < base || address > (base+30))
		return(-1);

	///@brief convert to device address
	address -= base;

	///@brief search for device address
	for(i=0;i<MAX_DEVICES;++i)
	{
		if(Devices[i].TYPE == type && Devices[i].ADDRESS == address)
			return(i);
	}
	return(-1);
}

///@brief Set the Active disk or device pointers
/// Since we can be called multiple times per single GPIB state we do not
/// display state changes here. Other code displays the active state.
///@param index: Devices[] index
///@return 1 on success or 0 on fail
int set_active_device(int index)
{
	int type,address;

	///@brief We also check for -1 
	/// So the result of find_device() can be used without additional tests
	if(index == -1)
	{
		return(0);
	}

	if(index < 0 || index >= MAX_DEVICES)
	{
		if(debuglevel & 1)
			printf("set_active_device:(%d) out of range\n", index);
		return(0);
	}

	type = Devices[index].TYPE;
	address = Devices[index].ADDRESS;
	if(address < 0 || address > 30)
	{
		if(debuglevel & 1)
			printf("set_active_device: index:%d address: %02XH out of range\n", index,address);
		return(0);
	}

	if(Devices[index].dev == NULL)
	{
		if(debuglevel & 1)
			printf("set_active_device: index:%d type:%d:%s, dev == NULL\n", 
				index,type,type_to_str(type));
		return(0);
	}

	if(type == NO_TYPE)
	{
		if(debuglevel & 1)
			printf("set_active_device: index %d uninitalized type:%d:%s\n", 
				index,type,type_to_str(type));
		return(0);
	}

	if(type == PRINTER_TYPE)
	{
		PRINTERp = (PRINTERDeviceType *) Devices[index].dev;
		return(1);
	}

	if(type == AMIGO_TYPE || type == SS80_TYPE) 
	{
		if(Devices[index].state == NULL)
		{
			if(debuglevel & 1)
				printf("set_active_device: index: %d type:%d:%s, state == NULL\n",
					 index,type,type_to_str(type));
			return(0);
		}
#ifdef AMIGO
		if(type == AMIGO_TYPE)
		{
			AMIGOp = (AMIGODiskType *) Devices[index].dev;
			AMIGOs = (AMIGOStateType *) Devices[index].state;
			return(1);
		}
#endif
		if(type == SS80_TYPE)
		{
			SS80p = (SS80DiskType *) Devices[index].dev;
			SS80s = (SS80StateType *) Devices[index].state;
			return(1);
		}
	}
	if(debuglevel & 1)
		printf("set_active_device:(%d) invalid type:%d:%s\n", 
			index,type,type_to_str(type));
	return(0);
}

///@brief Allocate a Device structure for a disk or printer
///@param type: disk type
///@return Devices[] index on sucess or -1
int alloc_device(int type)
{
	int ind;
	int index = -1;

	// Find a free slot
	ind = find_free();
	if(ind == -1)
	{
		if(debuglevel & 1)
			printf("alloc_device: Device table is full\n", type);
		return(ind);
	}

	switch(type)
	{
		case SS80_TYPE:
			Devices[ind].TYPE = type;
			Devices[ind].dev = safecalloc(sizeof(SS80DiskType)+7,1);
			Devices[ind].state = safecalloc(sizeof(SS80StateType)+7,1);
			index = ind;
			break;
#ifdef AMIGO
		case AMIGO_TYPE:
			Devices[ind].TYPE = type;
			Devices[ind].dev = safecalloc(sizeof(AMIGODiskType)+7,1);
			Devices[ind].state = safecalloc(sizeof(AMIGOStateType)+7,1);
			index = ind;
			break;
#endif
		case PRINTER_TYPE:
			Devices[ind].TYPE = type;
			Devices[ind].dev = safecalloc(sizeof(PRINTERDeviceType)+7,1);
			Devices[ind].state = NULL;
			index = ind;
			break;
		default:
			if(debuglevel & 1)
				printf("alloc_device: invalid type:%d:%s\n", type,type_to_str(type));
			break;
	}
	return(index);
}

// =============================================
/// @brief Init Config Parser Stack
/// Called only durring power up so we do not have to free memory
void init_Devices()
{
	int i;
	stack_ind = 0;
	for(i=0;i<MAX_DEVICES;++i)
	{
		Devices[i].TYPE = NO_TYPE;
		Devices[i].ADDRESS = 0;
		Devices[i].PPR = 0xff;
		Devices[i].dev = NULL;
		Devices[i].state = NULL;
	}
}

/// @brief Push Parser State
/// @param state: parser state
/// @return state
int push_state(int state)
{
	if(stack_ind < MAX_STACK)
		stack_p[stack_ind++] = state;
	else
		return(START_STATE);
	return(state);
}

/// @brief Pop Parser State
/// @return state
int pop_state()
{
	if(stack_ind > 0)
		return(stack_p[--stack_ind]);
	else
		return(START_STATE);
}

///@brief Config file line number
int lines = 0;

/// @brief assigned a value
///
/// - Used only for debugging
/// @param[in] str: string to examine
/// @param[in] minval: minimum value
/// @param[in] maxval: maximum value
/// @param[in] *val: value to set
///
/// @return  1 is matched and value in range, 0 not matched or out of range
uint32_t assign_value(char *str, uint32_t minval, uint32_t maxval, uint32_t *val)
{
	uint32_t tmp;
	int bad = 0;
	char *ptr;

    // Skip spaces before assignment
    ptr = skipspaces(str);
    // Skip optional '='
    if(*ptr == '=')
    {
        ++ptr;
        // skip spaces after assignment
        ptr = skipspaces(ptr);
    }
	if(!*ptr)
	{
		if(debuglevel & 1)
			printf("line:%d, missing value\n", lines);
		bad = 1;
	}
	if(!bad)
	{
		// FIXME detect bad numbers
		tmp = get_value(ptr);
		*val = tmp;
		if((minval && (tmp < minval)))
		{
			printf("line:%d, %s is below range %d\n", lines, ptr,(int)minval);
			bad = 1;
		}
		if((maxval != 0xffffffffUL) && (tmp > maxval))
		{
			printf("line:%d, %s is above range %d\n", lines, ptr,(int)maxval);
			bad = 1;
		}
	}
	if(bad)
		return(0);
	return(1);
}

///@brief Set Defaults for any missing disk or printer devices
/// These are only used if the Config file omits them or is empty
/// @return  void
void set_Config_Defaults()
{
	int index;

	///@brief Add optional hard coded devices for any that are missing
	if(find_type(SS80_TYPE) == -1)
	{
#if defined(HP9122D)
		printf("set_Config_Defaults: Using default SS/80 9122D\n");
#endif
#if defined(HP9134L)
		printf("set_Config_Defaults: Using default SS/80 9134L\n");
#endif
		index = find_free();
		if(index != -1)
		{
			Devices[index].TYPE  = SS80_TYPE;
			Devices[index].ADDRESS = SS80DiskDefault.HEADER.ADDRESS;
			Devices[index].PPR = SS80DiskDefault.HEADER.PPR;
			Devices[index].dev = (void *)&SS80DiskDefault;
			Devices[index].state = calloc(sizeof(SS80StateType)+7,1);
		}
	}
#ifdef AMIGO
	// Make sure we have a AMIGO defined
	if(find_type(AMIGO_TYPE) == -1)
	{
#if defined(HP9121D)
		printf("set_Config_Defaults:  Using default Amigo 9121D\n");
#endif
		index = find_free();
		if(index != -1)
		{
			Devices[index].TYPE  = AMIGO_TYPE;
			Devices[index].ADDRESS = AMIGODiskDefault.HEADER.ADDRESS;
			Devices[index].PPR = AMIGODiskDefault.HEADER.PPR;
			Devices[index].dev = (void *) &AMIGODiskDefault;
			Devices[index].state = calloc(sizeof(AMIGOStateType)+7,1);
		}
	}
#endif //#ifdef AMIGO
	// Make sure we have a PRINTER defined
	if(find_type(PRINTER_TYPE) == -1)
	{
		printf("set_Config_Defaults:  Using default PRINTER settings\n");
		index = find_free();
		if(index != -1)
		{
			Devices[index].TYPE  = PRINTER_TYPE;
			Devices[index].ADDRESS = PRINTERDeviceDefault.HEADER.ADDRESS;
			Devices[index].PPR = 0xff;
			Devices[index].dev = (void *) &PRINTERDeviceDefault;
			Devices[index].state = NULL;
		}
	}
}


/// @brief Read and parse a config file using POSIX functions
/// Set all drive parameters and debuglevel 
///
/// @param name: config file name to process
/// @return  number of parse errors
int POSIX_Read_Config(char *name)
{
    int ind,ret,len;
	uint32_t tmp;
	uint32_t val;
    FILE *cfg;
	int state = START_STATE;
	int errors = 0;
	int index = 0;

	///@brief Printer Device
	PRINTERDeviceType *PRINTERp = NULL;
	///@brief SS80 Device
	SS80DiskType *SS80p = NULL;
#ifdef AMIGO
	///@brief AMIGO Device
	AMIGODiskType *AMIGOp = NULL;
#endif

    char *ptr;
    char str[128];

	init_Devices();

    lines = 0;

	printf("Reading: %s\n", name);
    cfg = fopen(name, "r");
    if(cfg == NULL)
    {
		++errors;
		//FIXME
        perror("Read_Config - open");
        printf("Read_Config: open(%s) failed\n", name);
		set_Config_Defaults();
        return(errors);
    }

    while( (ptr = fgets(str, sizeof(str)-2, cfg)) != NULL)
    {
        ++lines;

        ptr = str;

        trim_tail(ptr);
		ptr = skipspaces(ptr);
        len = strlen(ptr);
        if(!len)
            continue;
		// Skip comments
		if(*ptr == '#')
			continue;

		//FIXME check for state and last state
		if(token(ptr,"END"))
		{
			state = pop_state();
			continue;
		}

		switch(state)
		{
		case START_STATE:
			if(token(ptr,"SS80"))
			{
				push_state(state);
				state = SS80_STATE;
				index = alloc_device(SS80_TYPE);
				if(index == -1)
					state = START_STATE;
				else
					SS80p = (SS80DiskType *) Devices[index].dev;
			}
#ifdef AMIGO
			else if(token(ptr,"AMIGO"))
			{
				push_state(state);
				state = AMIGO_STATE;
				index = alloc_device(AMIGO_TYPE);
				if(index == -1)
					state = START_STATE;
				else
					AMIGOp = (AMIGODiskType *) Devices[index].dev;

			}
#endif
			else if(token(ptr,"PRINTER"))
			{
				push_state(state);
				state = PRINTER_STATE;
				index = alloc_device(PRINTER_TYPE);
				if(index == -1)
					state = START_STATE;
				else
					PRINTERp = (PRINTERDeviceType *) Devices[index].dev;
			}
			else if( (ind = token(ptr,"DEBUG")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 65535, &val) )
					debuglevel = val;
			}
			else if( (ind = token(ptr,"PRINTER_DEFAULT_ADDRESS")) )
			{
				ptr += ind;
				//FIXME REMOVE from config
				printf("Skipping %s, at line:%d\n", ptr,lines);
			}
			else
			{
				printf("Unexpected START token: %s, at line:%d\n", ptr,lines);
				++errors;
			}
			break;

		case PRINTER_STATE:
			if(token(ptr,"CONFIG"))
			{
				push_state(state);
				state = PRINTER_CONFIG;
			}
			else
			{
				printf("Unexpected PRINTER token: %s, at line:%d\n", ptr,lines);
				++errors;
			}
			break;

		case PRINTER_CONFIG:
			if( (ind = token(ptr,"ADDRESS")) )
			{
				ptr += ind;
				
				tmp = 0xff;
				if (!assign_value(ptr, 0, 14, &val) )
					++errors;
				else
					tmp = val;
				Devices[index].ADDRESS = tmp;
				PRINTERp->HEADER.ADDRESS  = tmp;
				// NO PPR
				Devices[index].PPR = 0xff;
				PRINTERp->HEADER.PPR = 0xff;
			}
			else
			{
				printf("Unexpected PRINTER CONFIG token: %s, at line:%d\n", ptr,lines);
				++errors;
			}
			break;

		case SS80_STATE:
			if(token(ptr,"HEADER"))
			{
				push_state(state);
				state = SS80_HEADER;
			}
			else if(token(ptr,"CONFIG"))
			{
				push_state(state);
				state = SS80_CONFIG;
			}
			else if(token(ptr,"CONTROLLER"))
			{
				push_state(state);
				state = SS80_CONTROLLER;
			}
			else if(token(ptr,"UNIT"))
			{
				push_state(state);
				state = SS80_UNIT;
			}
			else if(token(ptr,"VOLUME"))
			{
				push_state(state);
				state = SS80_VOLUME;
			}
			else
			{
				printf("Unexpected SS80 START token: %s, at line:%d\n", ptr,lines);
				++errors;
			}
			break;

		case SS80_HEADER:
			if( (ind = token(ptr,"ADDRESS")) )
			{
				ptr += ind;
				tmp = 0xff;
				if (!assign_value(ptr, 0, 30, &val) )
					++errors;
				else
					tmp = val;
				Devices[index].ADDRESS = tmp;
				SS80p->HEADER.ADDRESS  = tmp;
			}
			else if( (ind = token(ptr,"PPR")) )
			{
				ptr += ind;
				tmp = 0xff;
                if (!assign_value(ptr, 0, 7, &val) )
                    ++errors;
				else
					tmp = val;
                Devices[index].PPR = tmp;
                SS80p->HEADER.PPR = tmp;
			}
			else if( (ind = token(ptr,"FILE")) )
			{
				ptr += ind;
				ptr = skipspaces(ptr);
				if(*ptr == '=')
				{
					++ptr;
					ptr = skipspaces(ptr);
				}
				strncpy(SS80p->HEADER.NAME,ptr, MAX_FILE_NAME_LEN-1);
				SS80p->HEADER.NAME[MAX_FILE_NAME_LEN-1] = 0;
			}
			else
			{
				printf("Unexpected SS80 CONFIG token: %s, at line:%d\n", ptr,lines);
				++errors;
			}
			break;
		case SS80_CONFIG:
			if( (ind = token(ptr,"ID")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFFFUL, &val))
					++errors;
				SS80p->CONFIG.ID = val;
			}
			else
			{
				printf("Unexpected SS80 CONFIG token: %s, at line:%d\n", ptr,lines);
				++errors;
			}
			break;

		case SS80_CONTROLLER:
			if( (ind = token(ptr,"UNITS_INSTALLED")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFFFUL, &val))
					++errors;
				SS80p->CONTROLLER.UNITS_INSTALLED = val;
			}
			else if( (ind = token(ptr,"TRANSFER_RATE")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFFFUL, &val))
					++errors;
				SS80p->CONTROLLER.TRANSFER_RATE = val;
			}
			else if( (ind = token(ptr,"TYPE")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFFFUL, &val))
					++errors;
				SS80p->CONTROLLER.TYPE = val;
			}
			else
			{
				printf("Unexpected SS80 CONTROLLER token: %s, at line:%d\n", ptr,lines);
				++errors;
			}
			break;

		case SS80_UNIT:
			if( (ind = token(ptr,"UNIT_TYPE")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFFFUL, &val))
					++errors;
				SS80p->UNIT.UNIT_TYPE = val;
			}
			else if( (ind = token(ptr,"DEVICE_NUMBER")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFFFFFFFUL, &val))
					++errors;
				SS80p->UNIT.DEVICE_NUMBER = val;
			}
			else if( (ind = token(ptr,"BYTES_PER_BLOCK")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0x1000UL, &val) )
					++errors;
				SS80p->UNIT.BYTES_PER_BLOCK = val;
			}
			else if( (ind = token(ptr,"BUFFERED_BLOCKS")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 1UL, 1UL, &val) )
					++errors;
				SS80p->UNIT.BUFFERED_BLOCKS = val;
			}
			else if( (ind = token(ptr,"BURST_SIZE")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0, &val) )
					++errors;
				SS80p->UNIT.BURST_SIZE = val;
			}
			else if( (ind = token(ptr,"BLOCK_TIME")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFFFUL, &val) )
					++errors;
				SS80p->UNIT.BLOCK_TIME = val;
			}
			else if( (ind = token(ptr,"CONTINOUS_TRANSFER_RATE")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFFFUL, &val) )
					++errors;
				SS80p->UNIT.CONTINOUS_TRANSFER_RATE = val;
			}
			else if( (ind = token(ptr,"OPTIMAL_RETRY_TIME")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFFFUL, &val) )
					++errors;
				SS80p->UNIT.OPTIMAL_RETRY_TIME = val;
			}
			else if( (ind = token(ptr,"ACCESS_TIME")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFFFUL, &val) )
					++errors;
				SS80p->UNIT.ACCESS_TIME = val;
			}
			else if( (ind = token(ptr,"MAXIMUM_INTERLEAVE")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFUL, &val) )
					++errors;
				SS80p->UNIT.MAXIMUM_INTERLEAVE = val;
			}
			else if( (ind = token(ptr,"FIXED_VOLUMES")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFUL, &val) )
					++errors;
				SS80p->UNIT.FIXED_VOLUMES = val;
			}
			else if( (ind = token(ptr,"REMOVABLE_VOLUMES")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFUL, &val) )
					++errors;
				SS80p->UNIT.REMOVABLE_VOLUMES = val;
			}
			else
			{
				printf("Unexpected SS80 UNIT token: %s, at line:%d\n", ptr,lines);
				++errors;
			}
			break;

		case SS80_VOLUME:
			if( (ind = token(ptr,"MAX_CYLINDER")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFFFFFFFUL, &val) )
					++errors;
				SS80p->VOLUME.MAX_CYLINDER = val;
			}
			else if( (ind = token(ptr,"MAX_HEAD")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFUL, &val) )
					++errors;
				SS80p->VOLUME.MAX_HEAD = val;
			}
			else if( (ind = token(ptr,"MAX_SECTOR")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFFFUL, &val) )
					++errors;
				SS80p->VOLUME.MAX_SECTOR = val;
			}
			else if( (ind = token(ptr,"MAX_BLOCK_NUMBER")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFFFFFFFUL, &val) )
					++errors;
				SS80p->VOLUME.MAX_BLOCK_NUMBER = val;
			}
			else if( (ind = token(ptr,"INTERLEAVE")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFUL, &val) )
					++errors;
				SS80p->VOLUME.INTERLEAVE = val;
			}
			else
			{
				printf("Unexpected SS80 VOLUME token: %s, at line:%d\n", ptr,lines);
				++errors;
			}
			break;

#ifdef AMIGO
		case AMIGO_STATE:
			if(token(ptr,"HEADER"))
			{
				push_state(state);
				state = AMIGO_HEADER;
			}
			else if(token(ptr,"CONFIG"))
			{
				push_state(state);
				state = AMIGO_CONFIG;
			}
			else if(token(ptr,"GEOMETRY"))
			{
				push_state(state);
				state = AMIGO_GEOMETRY;
			}
			else
			{
				printf("Unexpected AMIGO START token: %s, at line:%d\n", ptr,lines);
				++errors;
			}
			break;

		case AMIGO_HEADER:
			if( (ind = token(ptr,"DRIVE")) )
			{
				ptr += ind;
				printf("Skipping %s, at line:%d\n", ptr,lines);
				//skip this
			}
			else if( (ind = token(ptr,"ADDRESS")) )
			{
				ptr += ind;
				tmp = 0xff;
				if (!assign_value(ptr, 0, 14UL, &val) )
					++errors;
				else
					tmp = val;
				Devices[index].ADDRESS = tmp;
				AMIGOp->HEADER.ADDRESS = tmp;
			}
			else if( (ind = token(ptr,"PPR")) )
			{
				ptr += ind;
				tmp = 0xff;
				if (!assign_value(ptr, 0, 7UL, &val) )
					++errors;
				else
					tmp = val;
				Devices[index].PPR = tmp;
				AMIGOp->HEADER.PPR = tmp;
			}
			else if( (ind = token(ptr,"FILE")) )
			{
				ptr += ind;
				ptr = skipspaces(ptr);
				if(*ptr == '=')
				{
					++ptr;
					ptr = skipspaces(ptr);
				}
				strncpy(AMIGOp->HEADER.NAME,ptr, MAX_NAME_LEN-1);
				AMIGOp->HEADER.NAME[MAX_FILE_NAME_LEN-1] = 0;
			}
			else
			{
				printf("Unexpected HEADER CONFIG token: %s, at line:%d\n", ptr,lines);
				++errors;
			}
			break;

		case AMIGO_CONFIG:
			if( (ind = token(ptr,"ID")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFFFUL, &val) )
					++errors;
				AMIGOp->CONFIG.ID = val;
			}
			else
			{
				printf("Unexpected AMIGO CONFIG token: %s, at line:%d\n", ptr,lines);
				++errors;
			}
			break;

		case AMIGO_GEOMETRY:
			if( (ind = token(ptr,"BYTES_PER_SECTOR")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0x1000UL, &val) )
					++errors;
				AMIGOp->GEOMETRY.BYTES_PER_SECTOR = val;
			}
			else if( (ind = token(ptr,"SECTORS_PER_TRACK")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFUL, &val) )
					++errors;
				AMIGOp->GEOMETRY.SECTORS_PER_TRACK = val;
			}
			else if( (ind = token(ptr,"HEADS")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFUL, &val) )
					++errors;
				AMIGOp->GEOMETRY.HEADS = val;
			}
			else if( (ind = token(ptr,"CYLINDERS")) )
			{
				ptr += ind;
				if (!assign_value(ptr, 0, 0xFFUL, &val) )
					++errors;
				AMIGOp->GEOMETRY.CYLINDERS = val;
			}
			else
			{
				printf("Unexpected AMIGO GEMETRY token: %s, at line:%d\n", ptr,lines);
				++errors;
			}
			break;
#endif // #ifdef AMIGO

		default:
			printf("Unexpected STATE: %s, at line:%d\n", ptr,lines);
			++errors;
			break;

		} // switch
	} //while
	if(state != START_STATE)
	{
		printf("Missing END statement at line:%d\n", lines);
		++errors;
	}
    printf("Read_Config: read(%d) lines\n", lines);
	if(errors)
		printf("Read_Config: ****** errors(%d) ******\n", errors);

    ret = fclose(cfg);
    if(ret == EOF)
    {
        perror("Read_Config - close error");
		++errors;
    }

	return(errors);
}

/// @brief Display Configuration File variable
/// @param str: title
/// @param val: variable value
/// @return  void
void print_var_P(__memx const char *str, uint32_t val)
{
	char tmp[64];
	int i=0;
	while( *str && i < 62)
		tmp[i++] = *str++;
	tmp[i++] = 0;

	printf("    %-25s = %8lxH (%ld)\n", tmp, val, val);	
}

/// @brief Display Configuration File string
/// @param *str: title
/// @param *arg: string
/// @return  void
void print_str_P(__memx const char *str, char *arg)
{
	char tmp[64];
	int i=0;
	while( *str && i < 62)
		tmp[i++] = *str++;
	tmp[i++] = 0;
	printf("    %-25s = \"%s\"\n", tmp, arg);	
}

/// @brief Display Configuration device address saummary
/// @return  void
void display_Addresses()
{
	int i;
	int address;
	printf("Listen and Talk Address Settings\n");
	for(i=0;i<MAX_DEVICES;++i)
	{
		if(Devices[i].TYPE == NO_TYPE)
			continue;
		address = Devices[i].ADDRESS;

		if(Devices[i].TYPE == SS80_TYPE)
		{
			printf("  SS80_MLA    = %02XH\n",BASE_MLA + address );
			printf("  SS80_MTA    = %02XH\n",BASE_MTA + address );
			printf("  SS80_MSA    = %02XH\n",BASE_MSA + address );
		}
#ifdef AMIGO
		if(Devices[i].TYPE == AMIGO_TYPE )
		{
			printf("  AMIGO_MLA   = %02XH\n",BASE_MLA + address );
			printf("  AMIGO_MTA   = %02XH\n",BASE_MTA + address );
			printf("  AMIGO_MSA   = %02XH\n",BASE_MSA + address );
		}
#endif
		if(Devices[i].TYPE == PRINTER_TYPE )
		{
			printf("  PRINTER_MLA = %02XH\n",BASE_MLA + address );
			printf("  PRINTER_MTA = %02XH\n",BASE_MTA + address );
			printf("  PRINTER_MSA = %02XH\n",BASE_MSA + address );
		}
	}
	printf("\n");
}

/// @brief Display current Configuration File values
/// @return  void
void display_Config()
{
	int i;
	///@brief Active Printer Device
	PRINTERDeviceType *PRINTERp = NULL;
	///@brief Active SS80 Device
	SS80DiskType *SS80p = NULL;

#ifdef AMIGO
	///@brief Active AMIGO Device
	AMIGODiskType *AMIGOp = NULL;
#endif

	printf("Current Configuration Settings\n");
	for(i=0;i<MAX_DEVICES;++i)
	{
		if(Devices[i].TYPE == NO_TYPE)
			continue;

		if(Devices[i].TYPE == SS80_TYPE)
		{
			SS80p= (SS80DiskType *)Devices[i].dev;

			printf("SS80\n");
			printf("  CONFIG\n");
				print_var("ADDRESS", (uint32_t) SS80p->HEADER.ADDRESS);
				print_var("PPR", (uint32_t) SS80p->HEADER.PPR);
				print_str("FILE", SS80p->HEADER.NAME);
			printf("  HEADER\n");
				print_var("ID", (uint32_t) SS80p->CONFIG.ID);
			printf("  CONTROLLER\n");
				print_var("UNITS_INSTALLED", (uint32_t) SS80p->CONTROLLER.UNITS_INSTALLED);
				print_var("TRANSFER_RATE", (uint32_t)  SS80p->CONTROLLER.TRANSFER_RATE);
				print_var("TYPE", (uint32_t)  SS80p->CONTROLLER.TYPE);
			printf("  UNIT\n");
				print_var("UNIT_TYPE", (uint32_t)SS80p->UNIT.UNIT_TYPE);
				print_var("DEVICE_NUMBER", (uint32_t)SS80p->UNIT.DEVICE_NUMBER);
				print_var("BYTES_PER_BLOCK", (uint32_t)SS80p->UNIT.BYTES_PER_BLOCK);
				print_var("BUFFERED_BLOCKS", (uint32_t)SS80p->UNIT.BUFFERED_BLOCKS);
				print_var("BURST_SIZE", (uint32_t)SS80p->UNIT.BURST_SIZE);
				print_var("BLOCK_TIME", (uint32_t)SS80p->UNIT.BLOCK_TIME);
				print_var("CONTINOUS_TRANSFER_RATE", (uint32_t)SS80p->UNIT.CONTINOUS_TRANSFER_RATE);
				print_var("OPTIMAL_RETRY_TIME", (uint32_t)SS80p->UNIT.OPTIMAL_RETRY_TIME);
				print_var("ACCESS_TIME", (uint32_t)SS80p->UNIT.ACCESS_TIME);
				print_var("MAXIMUM_INTERLEAVE", (uint32_t)SS80p->UNIT.MAXIMUM_INTERLEAVE);
				print_var("FIXED_VOLUMES", (uint32_t)SS80p->UNIT.FIXED_VOLUMES);
				print_var("REMOVABLE_VOLUMES", (uint32_t)SS80p->UNIT.REMOVABLE_VOLUMES);
			printf("  VOLUME\n");
				print_var("MAX_CYLINDER", (uint32_t)SS80p->VOLUME.MAX_CYLINDER);
				print_var("MAX_HEAD", (uint32_t)SS80p->VOLUME.MAX_HEAD);
				print_var("MAX_SECTOR", (uint32_t)SS80p->VOLUME.MAX_SECTOR);
				print_var("MAX_BLOCK_NUMBER", (uint32_t)SS80p->VOLUME.MAX_BLOCK_NUMBER);
				print_var("INTERLEAVE", (uint32_t)SS80p->VOLUME.INTERLEAVE);
		} // SS80_TYPE

#ifdef AMIGO
		if(Devices[i].TYPE == AMIGO_TYPE )
		{
			AMIGOp= (AMIGODiskType *)Devices[i].dev;

			printf("AMIGO\n");
			printf("  HEADER\n");
				print_var("ADDRESS", (uint32_t) AMIGOp->HEADER.ADDRESS);
				print_var("PPR", (uint32_t) AMIGOp->HEADER.PPR);
				print_str("FILE", AMIGOp->HEADER.NAME);
			printf("  CONFIG\n");
				print_var("ID", (uint32_t) AMIGOp->CONFIG.ID);
			printf("  GEOMETRY\n");
				print_var("BYTES_PER_SECTOR", (uint32_t) AMIGOp->GEOMETRY.BYTES_PER_SECTOR);
				print_var("SECTORS_PER_TRACK", (uint32_t) AMIGOp->GEOMETRY.SECTORS_PER_TRACK);
				print_var("HEADS", (uint32_t) AMIGOp->GEOMETRY.HEADS);
				print_var("CYLINDERS", (uint32_t) AMIGOp->GEOMETRY.CYLINDERS);
		} 
#endif // #ifdef AMIGO

		if(Devices[i].TYPE == PRINTER_TYPE )
		{
			PRINTERp= (PRINTERDeviceType *)Devices[i].dev;

			printf("PRINTER\n");
			printf("  CONFIG\n");
				print_var("ADDRESS", (uint32_t) PRINTERp->HEADER.ADDRESS);
		}
		printf("\n");
	}
	printf("END\n");
	printf("\n");
	printf("\n");
}

/// @brief Format devices that have no image file
/// @return  void
void format_drives()
{
	int i;
	struct stat st;
	long sectors;
	char label[32];
	int ss80 = 1;
	int amigo = 1;

	///@brief Active Printer Device
	PRINTERDeviceType *PRINTERp = NULL;
	///@brief Active SS80 Device
	SS80DiskType *SS80p = NULL;

#ifdef AMIGO
	///@brief Active AMIGO Device
	AMIGODiskType *AMIGOp = NULL;
#endif

	for(i=0;i<MAX_DEVICES;++i)
	{
		if(Devices[i].TYPE == NO_TYPE)
			continue;

		if(Devices[i].TYPE == SS80_TYPE)
		{
			SS80p= (SS80DiskType *)Devices[i].dev;

			if(stat(SS80p->HEADER.NAME, &st) == -1) 
			{
				if( SS80p->UNIT.BYTES_PER_BLOCK != 256)
				{
					printf("Can not use non 256 byte sectors\n");
					continue;
				}
				//SS80p->VOLUME.MAX_CYLINDER;
				//SS80p->VOLUME.MAX_HEAD;
				//SS80p->VOLUME.MAX_SECTOR;
				sectors = SS80p->VOLUME.MAX_BLOCK_NUMBER;
				sprintf(label,"SS80-%d", ss80);
				lif_create_image(SS80p->HEADER.NAME,
					label,
					128, 
					sectors);

			}
			ss80++;
		} // SS80_TYPE

#ifdef AMIGO
		if(Devices[i].TYPE == AMIGO_TYPE )
		{
			AMIGOp= (AMIGODiskType *)Devices[i].dev;
			if(stat(AMIGOp->HEADER.NAME, &st) == -1) 
			{
				if( AMIGOp->GEOMETRY.BYTES_PER_SECTOR != 256)
				{
					printf("Can not use non 256 byte sectors\n");
					continue;
				}
				sectors = AMIGOp->GEOMETRY.SECTORS_PER_TRACK
				 * AMIGOp->GEOMETRY.HEADS
				 * AMIGOp->GEOMETRY.CYLINDERS;

				sprintf(label,"AMIGO%d", amigo);
				lif_create_image(AMIGOp->HEADER.NAME,
					label,
					15, 
					sectors);
			}
			sprintf(label,"AMIGO%d", amigo);
			amigo++;
		} 
#endif // #ifdef AMIGO
	}
	printf("\n");
}
