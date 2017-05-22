/**
 @file gpib/drives.c

 @brief SS80 disk emulator for HP85 disk emulator project for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.

 @par Based on work by Anders Gustafsson.

 @par Copyright &copy; 2014 Anders Gustafsson All rights reserved..

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

///@brief Convert a value to packed bytes at a given offset
/// bytes are MSB ... LSB order
void V2B(uint8_t *B, int index, int size, uint32_t val)
{
    int i;
    for(i=size-1;i>=0;--i)
    {
        B[index+i] = val & 0xff;
        val >>= 8;
    }
}

///@brief Convert a value to packed bytes at a given offset
/// bytes are MSB ... LSB order
uint32_t B2V(uint8_t *B, int index, int size)
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


/// ==================================================================
/// Disk Definitions

#if defined(HP9122D)
SS80DiskType SS80Disk =
{
	{
		0,          // Emulated Drive Number
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
#endif

#if defined(HP9134L)
SS80DiskType SS80Disk =
{
	{
		0,          // Emulated Drive Number
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
#endif

#ifdef AMIGO
/// @brief  AMIGO D9121D ident Bytes per sector, sectors per track, heads, cylinders
#if defined(HP9121D)
AMIGODiskType AMIGODisk =
{
	{
		0,          // Emulated Drive Number
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
#endif

/// @brief  AMIGO D9885A ident Bytes per sector, sectors per track, heads, cylinders
#if defined(HP9995A)
AMIGODiskType AMIGODisk =
{
	{
		0,          // Emulated Drive Number
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
#endif

/// @brief  AMIGO D9134A ident Bytes per sector, sectors per track, heads, cylinders
#if defined(HP9134A)
AMIGODiskType AMIGODisk =
{
	{
		0,          // Emulated Drive Number
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
#endif
#endif // AMIGO
/// =====================================================

///@brief SS80 Disk States
SS80StateType SS80State =
/* Status 5 + 6 + 4 */
{
	0,	// estate Execute State
	0,	// qstat
	0,	// Errors
	0,	// Unit No
	0,	// Volume No
	0,	// Address
	0	// Length
};


///@brief AMIGO Disk States
#ifdef AMIGO
AMIGOStateType AMIGOState =
{
    0, // state
	0, // unit number
	0, // volume number
	0, // current cylinder
	0, // current head
    0, // current sector
    0, // dsj status
	0, // Errors
    {0}, // status
    {0}  // address
};
#endif

/// =====================================================

/// @brief Config Parser Stack
#define MAX_STATES 5
static int states_ind = 0;
static int states[MAX_STATES];

/// @brief Init Config Parser Stack
void init_states()
{
	states_ind = 0;
}

/// @brief Push Parser State
/// @param state: parser state
/// @return state
int push_state(int state)
{
	if(states_ind < MAX_STATES)
		states[states_ind++] = state;
	else
		return(START_STATE);
	return(state);
}

/// @brief Pop Parser State
/// @return state
int pop_state()
{
	if(states_ind > 0)
		return(states[--states_ind]);
	else
		return(START_STATE);
}

/// @brief Read and parse a config file using POSIX functions
///
/// - Set debuglevel and other device settings
///
/// @param name: config file name to process
///
/// @return  0 on parse error
int POSIX_Read_Config(char *name)
{
    int ind,ret,len,lines;
    char str[128];
    char *ptr;
    FILE *cfg;
	int32_t val;
	int state = START_STATE;
	int status = 1;

	init_states();

	printf("Reading: %s\n", name);
    cfg = fopen(name, "r");
    if(cfg == NULL)
    {
        perror("Read_Config - open");
        return(0);
    }

    lines = 0;
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
			}
			else if(token(ptr,"AMIGO"))
			{
				push_state(state);
				state = AMIGO_STATE;
			}
			else if(token(ptr,"PRINTER"))
			{
				push_state(state);
				state = PRINTER_STATE;
			}
			else if( (ind = token(ptr,"DEBUG")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 255, &val) )
					debuglevel = val;
			}
			else if( (ind = token(ptr,"PRINTER_DEFAULT_ADDRESS")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 14, &val) )
					printer_addr = val;
			}
			else
			{
				printf("Unexpected START token: %s, at line:%d\n", ptr,lines);
				status = 0;
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
				status = 0;
			}
			break;

		case PRINTER_CONFIG:
			if( (ind = token(ptr,"ADDRESS")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 14, &val) )
					printer_addr = val;
				else
					status = 0;
			}
			else
			{
				printf("Unexpected PRINTER CONFIG token: %s, at line:%d\n", ptr,lines);
				status = 0;
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
				status = 0;
			}
			break;

		case SS80_HEADER:
			if( (ind = token(ptr,"DRIVE")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 7, &val) )
					SS80Disk.HEADER.DRIVE = val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"ADDRESS")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 14, &val) )
					SS80Disk.HEADER.ADDRESS = val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"PPR")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 7, &val) )
					SS80Disk.HEADER.PPR = val;
				else
					status = 0;
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
				strcpy(SS80Disk.HEADER.NAME,ptr);
			}
			else
			{
				printf("Unexpected SS80 CONFIG token: %s, at line:%d\n", ptr,lines);
				status = 0;
			}
			break;
		case SS80_CONFIG:
			if( (ind = token(ptr,"ID")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 7, &val) )
					SS80Disk.CONFIG.ID = val;
				else
					status = 0;
			}
			else
			{
				printf("Unexpected SS80 CONFIG token: %s, at line:%d\n", ptr,lines);
				status = 0;
			}
			break;

		case SS80_CONTROLLER:
			if( (ind = token(ptr,"UNITS_INSTALLED")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFFFL, &val) )
					SS80Disk.CONTROLLER.UNITS_INSTALLED= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"TRANSFER_RATE")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFFFL, &val) )
					SS80Disk.CONTROLLER.TRANSFER_RATE= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"TYPE")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFFFL, &val) )
					SS80Disk.CONTROLLER.TYPE= val;
				else
					status = 0;
			}
			else
			{
				printf("Unexpected SS80 CONTROLLER token: %s, at line:%d\n", ptr,lines);
				status = 0;
			}
			break;

		case SS80_UNIT:
			if( (ind = token(ptr,"UNIT_TYPE")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFFFL, &val) )
					SS80Disk.UNIT.UNIT_TYPE= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"DEVICE_NUMBER")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFFFFFFFL, &val) )
					SS80Disk.UNIT.DEVICE_NUMBER= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"BYTES_PER_BLOCK")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0x1000L, &val) )
					SS80Disk.UNIT.BYTES_PER_BLOCK= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"BUFFERED_BLOCKS")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 4L, &val) )
					SS80Disk.UNIT.BUFFERED_BLOCKS= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"BURST_SIZE")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0, &val) )
					SS80Disk.UNIT.BURST_SIZE= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"BLOCK_TIME")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFFFL, &val) )
					SS80Disk.UNIT.BLOCK_TIME= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"CONTINOUS_TRANSFER_RATE")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFFFL, &val) )
					SS80Disk.UNIT.CONTINOUS_TRANSFER_RATE= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"OPTIMAL_RETRY_TIME")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFFFL, &val) )
					SS80Disk.UNIT.OPTIMAL_RETRY_TIME= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"ACCESS_TIME")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFFFL, &val) )
					SS80Disk.UNIT.ACCESS_TIME= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"MAXIMUM_INTERLEAVE")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFL, &val) )
					SS80Disk.UNIT.MAXIMUM_INTERLEAVE= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"FIXED_VOLUMES")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFL, &val) )
					SS80Disk.UNIT.FIXED_VOLUMES= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"REMOVABLE_VOLUMES")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFL, &val) )
					SS80Disk.UNIT.REMOVABLE_VOLUMES= val;
				else
					status = 0;
			}
			else
			{
				printf("Unexpected SS80 UNIT token: %s, at line:%d\n", ptr,lines);
				status = 0;
			}
			break;

		case SS80_VOLUME:
			if( (ind = token(ptr,"MAX_CYLINDER")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFFFFFFFL, &val) )
					SS80Disk.VOLUME.MAX_CYLINDER= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"MAX_HEAD")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFL, &val) )
					SS80Disk.VOLUME.MAX_HEAD= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"MAX_SECTOR")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFFFL, &val) )
					SS80Disk.VOLUME.MAX_SECTOR= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"MAX_BLOCK_NUMBER")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFFFFFFFL, &val) )
					SS80Disk.VOLUME.MAX_BLOCK_NUMBER= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"INTERLEAVE")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFL, &val) )
					SS80Disk.VOLUME.INTERLEAVE= val;
				else
					status = 0;
			}
			else
			{
				printf("Unexpected SS80 VOLUME token: %s, at line:%d\n", ptr,lines);
				status = 0;
			}
			break;

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
				status = 0;
			}
			break;

		case AMIGO_HEADER:
			if( (ind = token(ptr,"DRIVE")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 7, &val) )
					AMIGODisk.HEADER.DRIVE = val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"ADDRESS")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 14, &val) )
					AMIGODisk.HEADER.ADDRESS = val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"PPR")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 7, &val) )
					AMIGODisk.HEADER.PPR = val;
				else
					status = 0;
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
				strcpy(AMIGODisk.HEADER.NAME,ptr);
			}
			else
			{
				printf("Unexpected HEADER CONFIG token: %s, at line:%d\n", ptr,lines);
				status = 0;
			}
			break;
		case AMIGO_CONFIG:
			if( (ind = token(ptr,"ID")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 7, &val) )
					AMIGODisk.CONFIG.ID = val;
				else
					status = 0;
			}
			else
			{
				printf("Unexpected AMIGO CONFIG token: %s, at line:%d\n", ptr,lines);
				status = 0;
			}
			break;

		case AMIGO_GEOMETRY:
			if( (ind = token(ptr,"BYTES_PER_SECTOR")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0x1000L, &val) )
					AMIGODisk.GEOMETRY.BYTES_PER_SECTOR= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"SECTORS_PER_TRACK")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFL, &val) )
					AMIGODisk.GEOMETRY.SECTORS_PER_TRACK= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"HEADS")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFL, &val) )
					AMIGODisk.GEOMETRY.HEADS= val;
				else
					status = 0;
			}
			else if( (ind = token(ptr,"CYLINDERS")) )
			{
				ptr += ind;
				if ( assign_value(ptr, 0, 0xFFL, &val) )
					AMIGODisk.GEOMETRY.CYLINDERS= val;
				else
					status = 0;
			}
			else
			{
				printf("Unexpected AMIGO GEMETRY token: %s, at line:%d\n", ptr,lines);
				status = 0;
			}
			break;

		default:
			printf("Unexpected STATE: %s, at line:%d\n", ptr,lines);
			status = 0;
			break;

		} // switch
	} //while
	if(state != START_STATE)
	{
		printf("Missing END statement at line:%d\n", lines);
		status = 0;
	}
    printf("Read_Config: read(%d) lines\n", lines);

    ret = fclose(cfg);
    if(ret == EOF)
    {
        perror("Read_Config - close error");
    }
	display_settings();
	return(status);
}


/// @brief Display Config Variable
/// @param str: variable name
/// @param val: variable value
/// @return  void
void print_var_P(__memx const char *str, uint32_t val)
{
	char tmp[64];
	int i=0;
	while( *str && i < 62)
		tmp[i++] = *str++;
	tmp[i++] = 0;

	printf("    %-25s = %8lx (%ld)\n", tmp, val, val);	
}

/// @brief Display Config Variable
/// @param str: variable name
/// @param val: variable value
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

///@convert print_var strings into __memx space
#define print_var(format, args...) print_var_P(PSTR(format), ##args)
///@convert print_var strings into __memx space
#define print_str(format, args...) print_str_P(PSTR(format), ##args)

/// @brief Display Current config values
/// @return  void
void display_Config()
{
	printf("=================\n");
	printf("SS80\n");
		printf("  CONFIG\n");
			print_var("DRIVE", (uint32_t) SS80Disk.HEADER.DRIVE);
			print_var("ADDRESS", (uint32_t) SS80Disk.HEADER.ADDRESS);
			print_var("PPR", (uint32_t) SS80Disk.HEADER.PPR);
			print_str("FILE", SS80Disk.HEADER.NAME);
		printf("  HEADER\n");
			print_var("ID", (uint32_t) SS80Disk.CONFIG.ID);
		printf("  CONTROLLER\n");
			print_var("UNITS_INSTALLED", (uint32_t) SS80Disk.CONTROLLER.UNITS_INSTALLED);
			print_var("TRANSFER_RATE", (uint32_t)  SS80Disk.CONTROLLER.TRANSFER_RATE);
			print_var("TYPE", (uint32_t)  SS80Disk.CONTROLLER.TYPE);
		printf("  UNIT\n");
			print_var("UNIT_TYPE", (uint32_t)SS80Disk.UNIT.UNIT_TYPE);
			print_var("DEVICE_NUMBER", (uint32_t)SS80Disk.UNIT.DEVICE_NUMBER);
			print_var("BYTES_PER_BLOCK", (uint32_t)SS80Disk.UNIT.BYTES_PER_BLOCK);
			print_var("BUFFERED_BLOCKS", (uint32_t)SS80Disk.UNIT.BUFFERED_BLOCKS);
			print_var("BURST_SIZE", (uint32_t)SS80Disk.UNIT.BURST_SIZE);
			print_var("BLOCK_TIME", (uint32_t)SS80Disk.UNIT.BLOCK_TIME);
			print_var("CONTINOUS_TRANSFER_RATE", (uint32_t)SS80Disk.UNIT.CONTINOUS_TRANSFER_RATE);
			print_var("OPTIMAL_RETRY_TIME", (uint32_t)SS80Disk.UNIT.OPTIMAL_RETRY_TIME);
			print_var("ACCESS_TIME", (uint32_t)SS80Disk.UNIT.ACCESS_TIME);
			print_var("MAXIMUM_INTERLEAVE", (uint32_t)SS80Disk.UNIT.MAXIMUM_INTERLEAVE);
			print_var("FIXED_VOLUMES", (uint32_t)SS80Disk.UNIT.FIXED_VOLUMES);
			print_var("REMOVABLE_VOLUMES", (uint32_t)SS80Disk.UNIT.REMOVABLE_VOLUMES);
		printf("  VOLUME\n");
			print_var("MAX_CYLINDER", (uint32_t)SS80Disk.VOLUME.MAX_CYLINDER);
			print_var("MAX_HEAD", (uint32_t)SS80Disk.VOLUME.MAX_HEAD);
			print_var("MAX_SECTOR", (uint32_t)SS80Disk.VOLUME.MAX_SECTOR);
			print_var("MAX_BLOCK_NUMBER", (uint32_t)SS80Disk.VOLUME.MAX_BLOCK_NUMBER);
			print_var("INTERLEAVE", (uint32_t)SS80Disk.VOLUME.INTERLEAVE);

	printf("AMIGO\n");
		printf("  HEADER\n");
			print_var("DRIVE", (uint32_t) AMIGODisk.HEADER.DRIVE);
			print_var("ADDRESS", (uint32_t) AMIGODisk.HEADER.ADDRESS);
			print_var("PPR", (uint32_t) AMIGODisk.HEADER.PPR);
			print_str("FILE", AMIGODisk.HEADER.NAME);
		printf("  CONFIG\n");
			print_var("ID", (uint32_t) AMIGODisk.CONFIG.ID);
		printf("  GEOMETRY\n");
			print_var("BYTES_PER_SECTOR", (uint32_t) AMIGODisk.GEOMETRY.BYTES_PER_SECTOR);
			print_var("SECTORS_PER_TRACK", (uint32_t) AMIGODisk.GEOMETRY.SECTORS_PER_TRACK);
			print_var("HEADS", (uint32_t) AMIGODisk.GEOMETRY.HEADS);
			print_var("CYLINDERS", (uint32_t) AMIGODisk.GEOMETRY.CYLINDERS);

    printf("PRINTER\n");
		printf("  CONFIG\n");
			print_var("ADDRESS", (uint32_t) printer_addr);
	printf("END\n");
}

/// @brief Display configuration settings
///
/// @return void
void display_settings()
{
    printf("HP Disk and Device Emulator\n");
    printf("Created on:%s %s\n", __DATE__,__TIME__);

#ifdef SOFTWARE_PP
    printf("\nSoftware PP\n");
#else
    printf("\nHardware PP\n");
#endif                                        // SOFTWARE_PP

#if defined(HP9122D)
    printf("SS/80 9122D\n");
#endif

#if defined(HP9134L)
    printf("SS/80 9134L\n");
#endif

#if defined(HP9121D)
    printf("Amigo 9121D\n");
#endif

	display_Config();

	printf("debuglevel   = %02x\n",(int)debuglevel);
	printf("\n");
	printf("BASE_MLA     = %02x\n",BASE_MLA);
	printf("BASE_MTA     = %02x\n",BASE_MTA);
	printf("BASE_MSA     = %02x\n",BASE_MSA);
	printf("\n");
	printf("SS80_MLA     = %02x\n",SS80_MLA);
	printf("SS80_MTA     = %02x\n",SS80_MTA);
	printf("SS80_MSA     = %02x\n",SS80_MSA);
	printf("SS80_PPR     = %02x\n",SS80_PPR);
	printf("\n");
	printf("AMIGO_MLA    = %02x\n",AMIGO_MLA);
	printf("AMIGO_MTA    = %02x\n",AMIGO_MTA);
	printf("AMIGO_MSA    = %02x\n",AMIGO_MSA);
	printf("AMIGO_PPR    = %02x\n",AMIGO_PPR);
	printf("\n");
	printf("PRINTER_MLA  = %02x\n",PRINTER_MLA);
	printf("PRINTER_MTA  = %02x\n",PRINTER_MTA);
	printf("PRINTER_MSA  = %02x\n",PRINTER_MSA);
	printf("\n");

}
