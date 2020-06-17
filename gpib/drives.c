/**
 @file gpib/drives.c
 @brief drive definitions for HP85 disk emulator project for AVR.
 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.
 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details
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
#include <time.h>
#include "lifutils.h"
#include "debug.h"

extern hpdir_t hpdir;

/// @brief Config Parser Stack
#define MAX_STACK 5
static int stack_ind = 0;
static int stack_p[MAX_STACK];

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

///@brief hpdir.ini file processing
hpdir_t hpdir;

#if defined (SET_DEFAULTS)
// =============================================
PRINTERDeviceType PRINTERDeviceDefault =
{
    {
        2,                                        // GPIB Address
        0xff,                                     // PPR unused
        "/printer.txt"
    }
};

// =============================================
#if defined(HP9122D)
///@brief SS80 HP9122D Disk Definitions
SS80DiskType SS80DiskDefault =
{
    {                                             // HEADER
        0,                                        // GPIB Address
        0,                                        // PPR
        "/ss80.lif"                               // File name
    },
    {                                             // CONFIG
        0x222                                     // ID
    },
    {                                             // CONTROLLER
        0x8001,                                   // Installed Units 1 (15 is always on)
        100,                                      // GPIB data transfer rate in kB/s on the bus
        4,                                        // 5 = SS/80 integrated single-unit controller
    },
    {                                             // UNIT
        0,                                        // Generic Unit Type, 0 = fixed, 1 = floppy, 2 = tape
        0x00091220,                               // BCD Device number XX XX XY, X=Unit, Y=option
        0x100,                                    // Bytes per block
        1,                                        // Buffered Blocks
        0,                                        // Burst size = 0 for SS80
        2000,                                     // Block time in microseconds
        50,                                       // Continous transfer time in kB/s
        10000,                                    // Retry time in 0.01 secods
        10000,                                    // Access time in 0.01 seconds
        31,                                       // Maximum interleave factor
        1,                                        // Fixed volume byte, one bit per volume
        1                                         // Removable volume byte, one bit per volume
    },
    {                                             // VOLUME
        0,                                        // Maximum Cylinder  - not used
        0,                                        // Maximum Head      - not used
        0,                                        // Maximum Sector    - not used
        2463,                                     // Maximum Block Number
        1                                         // Interleave
    }
};
#endif                                            // #if defined(HP9122D)

#if defined(HP9134D)
///@brief SS80 HP9134D Disk Definitions
SS80DiskType SS80DiskDefault =
{
    {                                             // HEADER
        0,                                        // GPIB Address
        0,                                        // PPR
        "/ss80.lif"                               // FILE name
    },
    {                                             // CONFIG
        0x222                                     // ID
    },
    {                                             // CONTROLLER
        0x8001,                                   // Installed Units 1 (15 is always on)
        100,                                      // GPIB data transfer rate in kB/s on the bus
        4,                                        // 4 = SS/80 integrated single-unit controller
    },
    {                                             // UNIT
        0,                                        // Generic Unit Type, 0 = fixed
        0x091340,                                 // BCD Device number XX XX XY, X=Unit, Y=option
        0x100,                                    // Bytes per block
        1,                                        // Buffered Blocks
        0,                                        // Burst size = 0 for SS80
        2000,                                     // Block time in microseconds
        50,                                       // Continous transfer time in kB/s
        10000,                                    // Retry time in 0.01 secods
        10000,                                    // Access time in 0.01 seconds
        31,                                       // Maximum interleave factor
        1,                                        // Fixed volume byte, one bit per volume
        1                                         // Removable volume byte, one bit per volume
    },
    {                                             // VOLUME
        0,                                        // Maximum Cylinder  - not used
        0,                                        // Maximum Head      - not used
        0,                                        // Maximum Sector    - not used
        58175,                                    // Maximum Block Number
        1                                         // Interleave
    }
};
#endif                                            // #if defined(HP9134D)

#ifdef AMIGO

#if defined(HP9121)
/// @brief  AMIGO D9121 ident Bytes per sector, sectors per track, heads, cylinders
AMIGODiskType AMIGODiskDefault =
{
    {                                             // HEADER
        1,                                        // GPIB Address
        1,                                        // PPR
        "/amigo.lif"                              // FILE name
    },
    {                                             // CONFIG
        0x0104                                    // ID
    },
    {                                             // GEOMETRY
        256,                                      // Bytes Per Sector
        16,                                       // Sectors Per Track
        2,                                        // Sides
        35                                        // Cylinders
    }
};
#endif                                            // #if defined(HP9121)
#endif                                            // ifdef AMIGO
#endif                                            // SET_DEFAULTS

token_t tokens[] =
{
    {"ACCESS_TIME",               TOK_ACCESS_TIME},
    {"ADDRESS",                   TOK_ADDRESS},
    {"AMIGO",                     TOK_AMIGO},
    {"BLOCKS",                    TOK_BLOCKS},
    {"BLOCK_TIME",                TOK_BLOCK_TIME},
    {"BUFFERED_BLOCKS",           TOK_BUFFERED_BLOCKS},
    {"BURST_SIZE",                TOK_BURST_SIZE},
    {"BYTES_PER_BLOCK",           TOK_BYTES_PER_BLOCK},
    {"BYTES_PER_SECTOR",          TOK_BYTES_PER_SECTOR},
    {"CONFIG",                    TOK_CONFIG},
    {"CONTINUOUS_TRANSFER_RATE",  TOK_CONTINUOUS_TRANSFER_RATE},
    {"CONTINOUS_TRANSFER_RATE",   TOK_CONTINUOUS_TRANSFER_RATE},
    {"CONTINIOUS_TRANSFER_RATE",  TOK_CONTINUOUS_TRANSFER_RATE},
    {"CONTROLLER",                TOK_CONTROLLER},
    {"CS80",                      TOK_CS80},
    {"CYLINDERS",                 TOK_CYLINDERS},
    {"DEBUG",                     TOK_DEBUG},
    {"DEVICE_NUMBER",             TOK_DEVICE_NUMBER},
    {"DRIVE",                     TOK_DRIVE},
    {"END",                       TOK_END},
    {"FILE",                      TOK_FILE},
    {"FIXED_VOLUMES",             TOK_FIXED_VOLUMES},
    {"GEOMETRY",                  TOK_GEOMETRY},
    {"HEADER",                    TOK_HEADER},
    {"HEADS",                     TOK_HEADS},
    {"ID",                        TOK_ID},
    {"INTERLEAVE",                TOK_INTERLEAVE},
    {"MAX_BLOCK_NUMBER",          TOK_MAX_BLOCK_NUMBER},
    {"MAX_CYLINDER",              TOK_MAX_CYLINDER},
    {"MAX_HEAD",                  TOK_MAX_HEAD},
    {"MAXIMUM_INTERLEAVE",        TOK_MAXIMUM_INTERLEAVE},
    {"MAX_SECTOR",                TOK_MAX_SECTOR},
    {"OPTIMAL_RETRY_TIME",        TOK_OPTIMAL_RETRY_TIME},
    {"PPR",                       TOK_PPR},
    {"PRINTER",                   TOK_PRINTER},
    {"REMOVABLE_VOLUMES",         TOK_REMOVABLE_VOLUMES},
    {"SECTORS_PER_TRACK",         TOK_SECTORS_PER_TRACK},
    {"SS80",                      TOK_SS80},
    {"SS80_DEFAULT",              TOK_SS80_DEFAULT},
    {"TRANSFER_RATE",             TOK_TRANSFER_RATE},
    {"TYPE",                      TOK_TYPE},
    {"UNIT",                      TOK_UNIT},
    {"UNITS_INSTALLED",           TOK_UNITS_INSTALLED},
    {"UNIT_TYPE",                 TOK_UNIT_TYPE},
    {"VOLUME",                    TOK_VOLUME},
    {"",                          TOK_INVALID},
};

/// ===============================================
/// @brief Display Configuration File variable
/// @param str: title
/// @param val: variable value
/// @return  void
void print_var_P(__memx const char *str, uint32_t val)
{
    char tmp[128];
    int i=0;
    while( *str && i < (int) (sizeof(tmp) - 2) )
        tmp[i++] = *str++;
    tmp[i++] = 0;

    printf("    %-25s = %8lxH (%ld)\n", tmp, val, val);
}


/// ===============================================
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


/// ===============================================
/// @brief return the tokens index of the matching string
/// @param str string tro match
/// @return  index
int8_t tok_index(char *str)
{
    int8_t i;
    for (i = 0; (tokens[i].tok != TOK_INVALID) ; ++i )
    {
        if( MATCHI(str,tokens[i].name) )
            return(i);
    }
    return(-1);
}


/// ===============================================
/// @brief return string of matching token
/// @param tok token
/// @return  char *ptr;
char *tok_name(uint8_t tok)
{
    int i;
    for (i = 0; (tokens[i].tok != TOK_INVALID) ; ++i )
    {
        if(tok == tokens[i].tok)
            return(tokens[i].name);
    }
    return("");
}


/// ===============================================
/// @brief Display Configuration File variable
/// @param tok token
/// @param spaces: indent
/// @param val: variable value
/// @return  void
void print_tok_val(uint8_t tok, uint8_t spaces, uint32_t val)
{
    char *ptr = tok_name(tok);
    while(spaces--)
        putchar(' ');
    printf("%-25s = %8lxH (%ld)\n", ptr, val, val);
}


/// ===============================================
/// @brief Display Configuration File variable
/// @param tok token
/// @param spaces: indent
/// @param str: string
/// @return  void
void print_tok_str(uint8_t tok, uint8_t spaces, char *str)
{
    char *ptr = tok_name(tok);
    while(spaces--)
        putchar(' ');
    printf("%-25s = %s\n", ptr, str);
}


/// ===============================================
/// @brief Display Configuration File variable
/// ===============================================
/// @brief Display Configuration File variable
/// @param str: title
/// @param spaces: indent
/// @return  void
void print_tok(uint8_t tok, uint8_t spaces)
{
    char *ptr = tok_name(tok);
    while(spaces--)
        putchar(' ');
    printf("%s\n",ptr);
}


/// ===============================================
/// @brief Read and parse a config file using POSIX functions
/// Set all drive parameters and debuglevel
///
/// @param name: config file name to process
/// @return  number of parse errors
int Read_Config(char *name)
{
    FILE *cfg;
    int state = START_STATE;
    int errors = 0;

///@brief Printer Device
    PRINTERDeviceType *PRINTERp = NULL;
///@brief SS80 Device
    SS80DiskType *SS80p = NULL;

#ifdef AMIGO
///@brief AMIGO Device
    AMIGODiskType *AMIGOp = NULL;
#endif

    char *ptr;
    int8_t index = 0;
    int8_t ind;
    val_t val;
    int tok = TOK_INVALID;
    int lines = 0;
	int8_t ppr;
	int8_t address;

    char str[128];
    char token[128];

    init_Devices();

    printf("Reading: %s\n", name);
    cfg = fopen(name, "rb");
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

        ptr = get_token(str, token, sizeof(token)-2);

// Skip comments
        if(token[0] == 0 || token[0]  == '#')
            continue;

        val.l = 0;
        tok = TOK_INVALID;

        if( ( ind = tok_index(token) )  == -1)
        {
            printf("Unexpected token: %s, at line:%d\n", token,lines);
            ++errors;
            continue;
        }
        tok = tokens[ind].tok;

// get optional argument
        ptr = get_token(ptr, token, sizeof(token)-2);
        if(MATCH(token,"="))
            ptr = get_token(ptr, token, sizeof(token)-2);
        val.l = get_value(token);
        val.w = (uint16_t) 0xFFFF & val.l;
        val.b = (uint8_t) 0xFF & val.l;

//FIXME check for state and last state
        if(tok == TOK_END )
        {
            state = pop_state();
            continue;
        }

        switch(state)
        {
            case START_STATE:
                switch(tok)
                {
                    case TOK_SS80_DEFAULT:
                        push_state(state);
                        state = SS80_STATE;
                        index = alloc_device(SS80_DEFAULT_TYPE);
                        if(index == -1)
                            state = START_STATE;
                        else
                            SS80p = (SS80DiskType *) Devices[index].dev;
                        break;
                    case TOK_SS80:
                    case TOK_CS80:
                        push_state(state);
                        state = SS80_STATE;
                        index = alloc_device(SS80_TYPE);
                        if(index == -1)
                        {
                            state = START_STATE;
                        }
                        else
                        {
                            SS80p = (SS80DiskType *) Devices[index].dev;
                            hpdir_set_parameters(index,token); // also SS80p->HEADER.model
                        }
                        break;
#ifdef AMIGO
                    case TOK_AMIGO:
                        push_state(state);
                        state = AMIGO_STATE;
                        index = alloc_device(AMIGO_TYPE);
                        if(index == -1)
                        {
                            state = START_STATE;
                        }
                        else
                        {
                            AMIGOp = (AMIGODiskType *) Devices[index].dev;
                            hpdir_set_parameters(index,token); // also sets AMIGOp->HEADER.model
                        }
                        break;
#endif
                    case TOK_PRINTER:
                        push_state(state);
                        state = PRINTER_STATE;
                        index = alloc_device(PRINTER_TYPE);
                        if(index == -1)
                            state = START_STATE;
                        else
                            PRINTERp = (PRINTERDeviceType *) Devices[index].dev;
                        break;
                    case TOK_DEBUG:
                        debuglevel = val.w;
                        break;
                    default:
                        printf("Unexpected token: %s, at line:%d\n", ptr,lines);
                        errors++;
                        break;
                }
                break;

            case PRINTER_STATE:
                if(tok == TOK_CONFIG)
                {
                    push_state(state);
                    state = PRINTER_CONFIG;
                }
                else
                {
                    printf("Unexpected PRINTER token: %s, at line:%d\n", ptr,lines);
                    ++errors;
                    break;
                }
                break;

            case PRINTER_CONFIG:
                if(tok == TOK_ADDRESS)
                {
					address = val.b;
                    Devices[index].ADDRESS = address;
                    PRINTERp->HEADER.ADDRESS  = address;
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
                switch(tok)
                {
                    case TOK_HEADER:
                        push_state(state);
                        state = SS80_HEADER;
                        break;
                    case TOK_CONFIG:
                        push_state(state);
                        state = SS80_CONFIG;
                        break;
                    case TOK_CONTROLLER:
                        push_state(state);
                        state = SS80_CONTROLLER;
                        break;
                    case TOK_UNIT:
                        push_state(state);
                        state = SS80_UNIT;
                        break;
                    case TOK_VOLUME:
                        push_state(state);
                        state = SS80_VOLUME;
                        break;
                    default:
                        printf("Unexpected SS80 START token: %s, at line:%d\n", ptr,lines);
                        ++errors;
                        break;
                }
                break;

            case SS80_HEADER:
                switch(tok)
                {
                    case TOK_ADDRESS:
						address = val.b;
                        Devices[index].ADDRESS = address;
                        SS80p->HEADER.ADDRESS  = address;
                        break;
                    case TOK_PPR:
						ppr = val.b;
                        Devices[index].PPR = ppr;
                        SS80p->HEADER.PPR = ppr;
                        break;
                    case TOK_FILE:
                        SS80p->HEADER.NAME = stralloc(token);
                        break;
                    default:
                        printf("Unexpected SS80 CONFIG token: %s, at line:%d\n", ptr,lines);
                        ++errors;
                        break;
                }
                break;

            case SS80_CONFIG:
                if(tok == TOK_ID )
                {
                    SS80p->CONFIG.ID = val.w;
                }
                else
                {
                    printf("Unexpected SS80 CONFIG token: %s, at line:%d\n", ptr,lines);
                    ++errors;
                }
                break;

            case SS80_CONTROLLER:
                switch(tok)
                {
                    case TOK_UNITS_INSTALLED:
                        SS80p->CONTROLLER.UNITS_INSTALLED = val.w;
                        break;
                    case TOK_TRANSFER_RATE:
                        SS80p->CONTROLLER.TRANSFER_RATE = val.w;
                        break;
                    case TOK_TYPE:
                        SS80p->CONTROLLER.TYPE = val.b;
                        break;
                    default:
                        printf("Unexpected SS80 CONTROLLER token: %s, at line:%d\n", ptr,lines);
                        ++errors;
                        break;
                }
                break;

            case SS80_UNIT:
                switch(tok)
                {
                    case TOK_UNIT_TYPE:
                        SS80p->UNIT.UNIT_TYPE = val.b;
                        break;
                    case TOK_DEVICE_NUMBER:
                        SS80p->UNIT.DEVICE_NUMBER = val.l;
                        break;
                    case TOK_BYTES_PER_BLOCK:
                        SS80p->UNIT.BYTES_PER_BLOCK = val.w;
                        break;
                    case TOK_BUFFERED_BLOCKS:
                        SS80p->UNIT.BUFFERED_BLOCKS = 1 & val.b;
                        break;
                    case TOK_BURST_SIZE:
                        SS80p->UNIT.BURST_SIZE = val.b;
                        break;
                    case TOK_BLOCK_TIME:
                        SS80p->UNIT.BLOCK_TIME = val.w;
                        break;
                    case TOK_CONTINUOUS_TRANSFER_RATE:
                        SS80p->UNIT.CONTINUOUS_TRANSFER_RATE = val.w;
                        break;
                    case TOK_OPTIMAL_RETRY_TIME:
                        SS80p->UNIT.OPTIMAL_RETRY_TIME = val.w;
                        break;
                    case TOK_ACCESS_TIME:
                        SS80p->UNIT.ACCESS_TIME = val.w;
                        break;
                    case TOK_MAXIMUM_INTERLEAVE:
                        SS80p->UNIT.MAXIMUM_INTERLEAVE = val.b;
                        break;
                    case TOK_FIXED_VOLUMES:
                        SS80p->UNIT.FIXED_VOLUMES = val.b;
                        break;
                    case TOK_REMOVABLE_VOLUMES:
                        SS80p->UNIT.REMOVABLE_VOLUMES = val.b;
                        break;
                    default:
                        printf("Unexpected SS80 UNIT token: %s, at line:%d\n", ptr,lines);
                        ++errors;
                        break;
                }
                break;

            case SS80_VOLUME:
                switch(tok)
                {
                    case TOK_MAX_CYLINDER:
                        SS80p->VOLUME.MAX_CYLINDER = val.l;
                        break;
                    case TOK_MAX_HEAD:
                        SS80p->VOLUME.MAX_HEAD = val.b;
                        break;
                    case TOK_MAX_SECTOR:
                        SS80p->VOLUME.MAX_SECTOR = val.w;
                        break;
                    case TOK_MAX_BLOCK_NUMBER:
                        SS80p->VOLUME.MAX_BLOCK_NUMBER = val.l;
                        break;
                    case TOK_INTERLEAVE:
                        SS80p->VOLUME.INTERLEAVE = val.b;
                        break;
                    default:
                        printf("Unexpected SS80 VOLUME token: %s, at line:%d\n", ptr,lines);
                        ++errors;
                        break;
                }
                break;

#ifdef AMIGO
            case AMIGO_STATE:
                switch(tok)
                {
                    case TOK_HEADER:
                        push_state(state);
                        state = AMIGO_HEADER;
                        break;
                    case TOK_CONFIG:
                        push_state(state);
                        state = AMIGO_CONFIG;
                        break;
                    case TOK_GEOMETRY:
                        push_state(state);
                        state = AMIGO_GEOMETRY;
                        break;
                    default:
                        printf("Unexpected AMIGO START token: %s, at line:%d\n", ptr,lines);
                        ++errors;
                        break;
                }
                break;

            case AMIGO_HEADER:
                switch(tok)
                {
                    case TOK_DRIVE:
                        break;
                    case TOK_ADDRESS:
						address =  val.b;
                        Devices[index].ADDRESS = address;
                        AMIGOp->HEADER.ADDRESS = address;
                        break;
                    case TOK_PPR:
						ppr = val.b;
                        Devices[index].PPR = ppr;
                        AMIGOp->HEADER.PPR = ppr;
                        break;
                    case TOK_FILE:
                        AMIGOp->HEADER.NAME = stralloc(token);
                        break;
                    default:
                        printf("Unexpected HEADER CONFIG token: %s, at line:%d\n", ptr,lines);
                        ++errors;
                        break;
                }
                break;

            case AMIGO_CONFIG:
                if(tok == TOK_ID )
                {
                    AMIGOp->CONFIG.ID = val.w;
                }
                else
                {
                    printf("Unexpected AMIGO CONFIG token: %s, at line:%d\n", ptr,lines);
                    ++errors;
                }
                break;

            case AMIGO_GEOMETRY:
                switch(tok)
                {
                    case TOK_BYTES_PER_SECTOR:
                        AMIGOp->GEOMETRY.BYTES_PER_SECTOR = val.w;
                        break;
                    case TOK_SECTORS_PER_TRACK:
                        AMIGOp->GEOMETRY.SECTORS_PER_TRACK = val.w;
                        break;
                    case TOK_HEADS:
                        AMIGOp->GEOMETRY.HEADS = val.w;
                        break;
                    case TOK_CYLINDERS:
                        AMIGOp->GEOMETRY.CYLINDERS = val.w;
                        break;
                    default:
                        printf("Unexpected AMIGO GEMETRY token: %s, at line:%d\n", ptr,lines);
                        ++errors;
                        break;
                }
                break;
#endif                                // #ifdef AMIGO
            default:
                printf("Unexpected STATE: %s, at line:%d\n", ptr,lines);
                ++errors;
                break;

        }                                         // switch
    }                                             //while
    if(state != START_STATE)
    {
        printf("Missing END statement at line:%d\n", lines);
        ++errors;
    }
    printf("Read_Config: read(%d) lines\n", lines);
    if(errors)
        printf("Read_Config: ****** errors(%d) ******\n", errors);

    if(fclose(cfg) == EOF)
    {
        perror("Read_Config - close error");
        ++errors;
    }

// Post process and fixup any devices
    verify_devices();

    return(errors);
}


/// ===============================================
/// @brief Display Configuration device address saummary
/// @return  void
void display_Addresses( int verbose )
{
    int i;

    printf("Device Addresses\n");
    for(i=0;i<MAX_DEVICES;++i)
    {
        if(Devices[i].TYPE == NO_TYPE)
            continue;

        if(Devices[i].TYPE == SS80_TYPE || Devices[i].TYPE == AMIGO_TYPE)
        {
            if(Devices[i].TYPE == SS80_TYPE)
            {
                SS80p= (SS80DiskType *)Devices[i].dev;
                printf("SS80 %s\n", SS80p->HEADER.model);
                print_tok_str(TOK_FILE, 4, SS80p->HEADER.NAME);
            }
#ifdef AMIGO
            if(Devices[i].TYPE == AMIGO_TYPE )
            {
                AMIGOp= (AMIGODiskType *)Devices[i].dev;
                printf("AMIGO %s\n", AMIGOp->HEADER.model);
                print_tok_str(TOK_FILE, 4, AMIGOp->HEADER.NAME);
            }
#endif
            print_tok_val(TOK_ADDRESS, 4, (uint32_t) Devices[i].ADDRESS);
            print_tok_val(TOK_PPR, 4, (uint32_t) Devices[i].PPR);
            print_tok_val(TOK_BLOCKS, 4, (uint32_t) Devices[i].BLOCKS);
        }
        if(Devices[i].TYPE == PRINTER_TYPE )
        {
            printf("PRINTER\n");
            print_tok_val(TOK_ADDRESS, 4, (uint32_t) Devices[i].ADDRESS);
        }

        if(verbose)
        {
#if 0
            int address = Devices[i].ADDRESS;
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
#endif
        }
        printf("\n");

    }
    printf("\n");
}

/// ===============================================
/// @brief Display current Configuration File values
/// @return  void
void display_Config( int verbose)
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

            printf("SS80 %s\n", SS80p->HEADER.model);
            printf("  # HP85 BASIC ADDRESS :D7%d0\n", (int) SS80p->HEADER.ADDRESS);

            print_tok(TOK_CONFIG,4);
            print_tok_val(TOK_ADDRESS, 8, (uint32_t) SS80p->HEADER.ADDRESS);
            print_tok_val(TOK_PPR, 8, (uint32_t) SS80p->HEADER.PPR);
            print_tok_str(TOK_FILE, 8, SS80p->HEADER.NAME);
            print_tok(TOK_END,4);

            print_tok(TOK_HEADER,4);
            print_tok_val(TOK_ID, 8, (uint32_t) SS80p->CONFIG.ID);
            print_tok(TOK_END,4);

// CONTROLLER
            if(verbose)
            {
                print_tok(TOK_CONTROLLER,4);
                print_tok_val(TOK_UNITS_INSTALLED, 8, (uint32_t) SS80p->CONTROLLER.UNITS_INSTALLED);
                print_tok_val(TOK_TRANSFER_RATE, 8, (uint32_t)  SS80p->CONTROLLER.TRANSFER_RATE);
                print_tok_val(TOK_TYPE, 8, (uint32_t)  SS80p->CONTROLLER.TYPE);
                print_tok(TOK_END,4);
            }

// UNIT
            print_tok(TOK_UNIT,4);
            if(verbose)
            {
                print_tok_val(TOK_UNIT_TYPE, 8, (uint32_t)SS80p->UNIT.UNIT_TYPE);
            }
            print_tok_val(TOK_DEVICE_NUMBER, 8, (uint32_t)SS80p->UNIT.DEVICE_NUMBER);
            if(verbose)
            {
                print_tok_val(TOK_BYTES_PER_BLOCK, 8, (uint32_t)SS80p->UNIT.BYTES_PER_BLOCK);
                print_tok_val(TOK_BUFFERED_BLOCKS, 8, (uint32_t)SS80p->UNIT.BUFFERED_BLOCKS);
                print_tok_val(TOK_BURST_SIZE, 8, (uint32_t)SS80p->UNIT.BURST_SIZE);
                print_tok_val(TOK_BLOCK_TIME, 8, (uint32_t)SS80p->UNIT.BLOCK_TIME);
                print_tok_val(TOK_CONTINUOUS_TRANSFER_RATE, 8, (uint32_t)SS80p->UNIT.CONTINUOUS_TRANSFER_RATE);
                print_tok_val(TOK_OPTIMAL_RETRY_TIME, 8, (uint32_t)SS80p->UNIT.OPTIMAL_RETRY_TIME);
                print_tok_val(TOK_ACCESS_TIME, 8, (uint32_t)SS80p->UNIT.ACCESS_TIME);
                print_tok_val(TOK_MAXIMUM_INTERLEAVE, 8, (uint32_t)SS80p->UNIT.MAXIMUM_INTERLEAVE);
                print_tok_val(TOK_FIXED_VOLUMES, 8, (uint32_t)SS80p->UNIT.FIXED_VOLUMES);
                print_tok_val(TOK_REMOVABLE_VOLUMES, 8, (uint32_t)SS80p->UNIT.REMOVABLE_VOLUMES);
            }
            print_tok(TOK_END,4);

// VOLUME
            if(verbose)
            {
                print_tok(TOK_VOLUME,4);
                print_tok_val(TOK_MAX_CYLINDER, 8, (uint32_t)SS80p->VOLUME.MAX_CYLINDER);
                print_tok_val(TOK_MAX_HEAD, 8, (uint32_t)SS80p->VOLUME.MAX_HEAD);
                print_tok_val(TOK_MAX_SECTOR, 8, (uint32_t)SS80p->VOLUME.MAX_SECTOR);
                print_tok_val(TOK_MAX_BLOCK_NUMBER, 8, (uint32_t)SS80p->VOLUME.MAX_BLOCK_NUMBER);
                print_tok_val(TOK_INTERLEAVE, 8, (uint32_t)SS80p->VOLUME.INTERLEAVE);
                print_tok(TOK_END,4);
            }
            printf("    # BLOCKS = %ld\n", (long)SS80p->VOLUME.MAX_BLOCK_NUMBER+1);

            print_tok(TOK_END,0);
        }                                         // SS80_TYPE

#ifdef AMIGO
        if(Devices[i].TYPE == AMIGO_TYPE )
        {
            AMIGOp= (AMIGODiskType *)Devices[i].dev;

            printf("AMIGO %s\n", AMIGOp->HEADER.model);
            printf("  # HP85 BASIC ADDRESS :D7%d0\n", (int) AMIGOp->HEADER.ADDRESS);

            print_tok(TOK_HEADER,4);
            print_tok_val(TOK_ADDRESS, 8, (uint32_t) AMIGOp->HEADER.ADDRESS);
            print_tok_val(TOK_PPR, 8, (uint32_t) AMIGOp->HEADER.PPR);
            print_tok_str(TOK_FILE, 8, AMIGOp->HEADER.NAME);
            print_tok(TOK_END,4);

            print_tok(TOK_CONFIG,4);
            print_tok_val(TOK_ID, 8, (uint32_t) AMIGOp->CONFIG.ID);
            print_tok(TOK_END,4);

            if(verbose)
            {
                print_tok(TOK_GEOMETRY,4);
                print_tok_val(TOK_BYTES_PER_SECTOR, 8, (uint32_t) AMIGOp->GEOMETRY.BYTES_PER_SECTOR);
                print_tok_val(TOK_SECTORS_PER_TRACK, 8, (uint32_t) AMIGOp->GEOMETRY.SECTORS_PER_TRACK);
                print_tok_val(TOK_HEADS, 8, (uint32_t) AMIGOp->GEOMETRY.HEADS);
                print_tok_val(TOK_CYLINDERS, 8, (uint32_t) AMIGOp->GEOMETRY.CYLINDERS);
                print_tok(TOK_END,4);
            }
            printf("    # BLOCKS = %ld\n", (long) (AMIGOp->GEOMETRY.CYLINDERS * AMIGOp->GEOMETRY.SECTORS_PER_TRACK * AMIGOp->GEOMETRY.HEADS) );

            print_tok(TOK_END,0);
        }
#endif                                    // #ifdef AMIGO

        if(Devices[i].TYPE == PRINTER_TYPE )
        {
            PRINTERp= (PRINTERDeviceType *)Devices[i].dev;

            print_tok(TOK_PRINTER,0);

            print_tok(TOK_CONFIG,4);
            print_tok_val(TOK_ADDRESS, 8, (uint32_t) PRINTERp->HEADER.ADDRESS);
            print_tok(TOK_END,4);

            print_tok(TOK_END,0);
        }
        printf("\n");
    }
    printf("\n");
}




///@brief Seach Devices[] for ANY definitions of a disk type
///@param type: disk type like SS80_TYPE
//@return Devices[] index fopr matching type
int8_t find_type(int type)
{
    int i;
    for(i=0;i<MAX_DEVICES;++i)
    {
        if( Devices[i].TYPE == type)
            return(i);
    }
    return(-1);
}


/// @brief Count number of devices of a sertain type
///@param type: disk type like SS80_TYPE
int8_t count_drive_types(uint8_t type)
{
    int i;
    int count = 0;
    for(i=0;i<MAX_DEVICES;++i)
    {
        if( Devices[i].TYPE == type )
            ++count;
    }
    return(count);
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
int8_t find_free()
{
    return(find_type(NO_TYPE));
}


///@brief Find a device with matching type AND address
///@param type: disk type
///@param address: GPIB device address 0 based
///@param base: BASE_MLA,BASE_MTA or BASE_MSA address range
///@return index of Devices[] or -1 if not found
int8_t find_device(int type, int address, int base)
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
int8_t set_active_device(int8_t index)
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
        if(debuglevel & GPIB_ERR)
            printf("set_active_device:(%d) out of range\n", index);
        return(0);
    }

    type = Devices[index].TYPE;
    address = Devices[index].ADDRESS;
    if(address < 0 || address > 30)
    {
        if(debuglevel & GPIB_ERR)
            printf("set_active_device: index:%d address: %02XH out of range\n", index,address);
        return(0);
    }

    if(Devices[index].dev == NULL)
    {
        if(debuglevel & GPIB_ERR)
            printf("set_active_device: index:%d type:%d:%s, dev == NULL\n",
                index,type,type_to_str(type));
        return(0);
    }

    if(type == NO_TYPE)
    {
        if(debuglevel & GPIB_ERR)
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
            if(debuglevel & GPIB_ERR)
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
    if(debuglevel & GPIB_ERR)
        printf("set_active_device:(%d) invalid type:%d:%s\n",
            index,type,type_to_str(type));
    return(0);
}


///@brief Set Default Values for a new SS80 Device IF defaults have been defined
/// Most values in the CONTROLER and UNIT are defaults that should not need to be specified
/// Note all of the values are zeroed on allocation including strings
///@return void
void SS80_Set_Defaults(int8_t index)
{
    int8_t defindex = find_type(SS80_DEFAULT_TYPE);
    SS80DiskType *SS80p = (SS80DiskType *) Devices[index].dev;
    SS80DiskType *SS80DEFAULTp;

    if(defindex < 0 )
        return;

    SS80DEFAULTp = (SS80DiskType *) Devices[defindex].dev;

    SS80p->HEADER.ADDRESS               = SS80DEFAULTp->HEADER.ADDRESS;
    SS80p->HEADER.PPR                   = SS80DEFAULTp->HEADER.PPR;
    SS80p->HEADER.NAME                  = stralloc(SS80DEFAULTp->HEADER.NAME);

    SS80p->CONFIG.ID                    = SS80DEFAULTp->CONFIG.ID;
    SS80p->CONTROLLER.UNITS_INSTALLED   = SS80DEFAULTp->CONTROLLER.UNITS_INSTALLED;
    SS80p->CONTROLLER.TRANSFER_RATE     = SS80DEFAULTp->CONTROLLER.TRANSFER_RATE;
    SS80p->CONTROLLER.TYPE              = SS80DEFAULTp->CONTROLLER.TYPE;

    SS80p->UNIT.UNIT_TYPE               = SS80DEFAULTp->UNIT.UNIT_TYPE;
    SS80p->UNIT.DEVICE_NUMBER           = SS80DEFAULTp->UNIT.DEVICE_NUMBER;
    SS80p->UNIT.BYTES_PER_BLOCK         = SS80DEFAULTp->UNIT.BYTES_PER_BLOCK;
    SS80p->UNIT.BUFFERED_BLOCKS         = SS80DEFAULTp->UNIT.BUFFERED_BLOCKS;
    SS80p->UNIT.BURST_SIZE              = SS80DEFAULTp->UNIT.BURST_SIZE;
    SS80p->UNIT.BLOCK_TIME              = SS80DEFAULTp->UNIT.BLOCK_TIME;
    SS80p->UNIT.CONTINUOUS_TRANSFER_RATE= SS80DEFAULTp->UNIT.CONTINUOUS_TRANSFER_RATE;
    SS80p->UNIT.OPTIMAL_RETRY_TIME      = SS80DEFAULTp->UNIT.OPTIMAL_RETRY_TIME;
    SS80p->UNIT.ACCESS_TIME             = SS80DEFAULTp->UNIT.ACCESS_TIME;
    SS80p->UNIT.MAXIMUM_INTERLEAVE      = SS80DEFAULTp->UNIT.MAXIMUM_INTERLEAVE;
    SS80p->UNIT.FIXED_VOLUMES           = SS80DEFAULTp->UNIT.FIXED_VOLUMES;

    SS80p->VOLUME.MAX_CYLINDER          = SS80DEFAULTp->VOLUME.MAX_CYLINDER;
    SS80p->VOLUME.MAX_HEAD              = SS80DEFAULTp->VOLUME.MAX_HEAD;
    SS80p->VOLUME.MAX_SECTOR            = SS80DEFAULTp->VOLUME.MAX_SECTOR;
    SS80p->VOLUME.MAX_BLOCK_NUMBER      = SS80DEFAULTp->VOLUME.MAX_BLOCK_NUMBER;
    SS80p->VOLUME.INTERLEAVE            = SS80DEFAULTp->VOLUME.INTERLEAVE;
};


///@brief Free a Device structure for a disk or printer
///@param type: disk index
///@return void
void free_device(int8_t index)
{
	
	extern void printer_close(void);

	if(index < 0 || index >= MAX_DEVICES)
		return;

    if(Devices[index].TYPE == SS80_TYPE)
    {
        SS80DiskType *SS80p = (SS80DiskType *) Devices[index].dev;
		// Device Model Name
		safefree(SS80p->HEADER.NAME);
		// File
		safefree(SS80p->HEADER.model);
	}

#ifdef AMIGO
    if(Devices[index].TYPE == AMIGO_TYPE)
    {
        AMIGODiskType *AMIGOp = (AMIGODiskType *) Devices[index].dev;
		// Device Model Name
		safefree(AMIGOp->HEADER.NAME);
		// File
		safefree(AMIGOp->HEADER.model);
	}
#endif

	// FIXME use printer structure to permit multiple printers
    if(Devices[index].TYPE == PRINTER_TYPE)
    {
		printer_close();
	}

	safefree(Devices[index].dev);
	safefree(Devices[index].state);

	Devices[index].TYPE = NO_TYPE;
	Devices[index].ADDRESS = 0;
	Devices[index].PPR = 0xff;
	Devices[index].BLOCKS = 0;
	Devices[index].dev = NULL;
	Devices[index].state = NULL;

}

///@brief Allocate a Device structure for a disk or printer
///@param type: disk type
///@return Devices[] index on sucess or -1
int8_t alloc_device(int type)
{
    int8_t ind;
    int8_t index = -1;

// Find a free slot
    ind = find_free();
    if(ind == -1)
    {
        if(debuglevel & GPIB_ERR)
            printf("alloc_device: Device table is full\n", type);
        return(ind);
    }

    switch(type)
    {
// Same as SS80 type but sets initial defaults for any remaining SS80 drives
        case SS80_DEFAULT_TYPE:
            Devices[ind].TYPE = type;
            Devices[ind].dev = safecalloc(sizeof(SS80DiskType)+7,1);
            Devices[ind].state = safecalloc(sizeof(SS80StateType)+7,1);
            index = ind;
            break;
        case SS80_TYPE:
            Devices[ind].TYPE = type;
            Devices[ind].dev = safecalloc(sizeof(SS80DiskType)+7,1);
            Devices[ind].state = safecalloc(sizeof(SS80StateType)+7,1);
            index = ind;
            SS80_Set_Defaults(index);             // Set any defaults we may have
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
            if(debuglevel & GPIB_ERR)
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
        Devices[i].BLOCKS = 0;
        Devices[i].dev = NULL;
        Devices[i].state = NULL;
    }
}


/// ===============================================
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


/// ===============================================
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

/// ===============================================
/// @brief assigned a value
///
/// - Used only for debugging
/// @param[in] str: string to examine
/// @param[in] minval: minimum value
/// @param[in] maxval: maximum value
/// @param[in] *val: value to set
///
/// @return  1 is matched and value in range, 0 not matched or out of range
bool assign_value(char *str, uint32_t minval, uint32_t maxval, uint32_t *val)
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
        if(debuglevel & GPIB_ERR)
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


/// ===============================================
///@brief Set Defaults for any missing disk or printer devices
/// These are only used if the Config file omits them or is empty
/// @return  void
void set_Config_Defaults()
{
#if defined(SET_DEFAULTS)
    int8_t index;

///@brief Add optional hard coded devices for any that are missing
    if(find_type(SS80_TYPE) == -1)
    {
#if defined(HP9122D)
        printf("set_Config_Defaults: Using default SS/80 9122D\n");
#endif
#if defined(HP9134D)
        printf("set_Config_Defaults: Using default SS/80 9134L\n");
#endif
        index = find_free();
        if(index != -1)
        {
            Devices[index].TYPE  = SS80_TYPE;
            Devices[index].ADDRESS = SS80DiskDefault.HEADER.ADDRESS;
            Devices[index].PPR = SS80DiskDefault.HEADER.PPR;
            Devices[index].dev = (void *)&SS80DiskDefault;
            Devices[index].state = safecalloc(sizeof(SS80StateType)+7,1);
        }
    }
#ifdef AMIGO
// Make sure we have a AMIGO defined
    if(find_type(AMIGO_TYPE) == -1)
    {
#if defined(HP9121)
        printf("set_Config_Defaults:  Using default Amigo 9121\n");
#endif
        index = find_free();
        if(index != -1)
        {
            Devices[index].TYPE  = AMIGO_TYPE;
            Devices[index].ADDRESS = AMIGODiskDefault.HEADER.ADDRESS;
            Devices[index].PPR = AMIGODiskDefault.HEADER.PPR;
            Devices[index].dev = (void *) &AMIGODiskDefault;
            Devices[index].state = safecalloc(sizeof(AMIGOStateType)+7,1);
        }
    }
#endif                                        //#ifdef AMIGO

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
#endif                                        // SET_DEFAULTS

}


/// ===============================================
///@brief Set Device parameters from hpdir information
///
///@param[in] index: device index
///
///@return 1 success, 0 on error
int8_t hpdir_set_device(int8_t index)
{
    if(Devices[index].TYPE == SS80_TYPE)
    {
        SS80DiskType *SS80p = (SS80DiskType *) Devices[index].dev;
        SS80p->CONFIG.ID                = hpdir.ID;
        SS80p->UNIT.DEVICE_NUMBER       = hpdir.DEVICE_NUMBER;
        SS80p->UNIT.BYTES_PER_BLOCK     = hpdir.BYTES_PER_SECTOR;

// CHS NOT used in this emulator!
        SS80p->VOLUME.MAX_CYLINDER      = 0;      // hpdir.CYLINDERS-1;
        SS80p->VOLUME.MAX_HEAD          = 0;      // hpdir.HEADS-1;
        SS80p->VOLUME.MAX_SECTOR        = 0;      // hpdir.SECTORS-1;

        SS80p->VOLUME.MAX_BLOCK_NUMBER  = hpdir.BLOCKS-1;
        Devices[index].BLOCKS = hpdir.BLOCKS;
        SS80p->HEADER.model = stralloc(hpdir.model);
		return(1);
    }

#ifdef AMIGO
    else if(Devices[index].TYPE == AMIGO_TYPE)
    {
        AMIGODiskType *AMIGOp = (AMIGODiskType *) Devices[index].dev;
        AMIGOp->CONFIG.ID = hpdir.ID;
        AMIGOp->GEOMETRY.BYTES_PER_SECTOR = hpdir.BYTES_PER_SECTOR;
        AMIGOp->GEOMETRY.SECTORS_PER_TRACK = hpdir.SECTORS;
        AMIGOp->GEOMETRY.HEADS = hpdir.HEADS;
        AMIGOp->GEOMETRY.CYLINDERS = hpdir.CYLINDERS;
        Devices[index].BLOCKS = hpdir.BLOCKS;
        AMIGOp->HEADER.model = stralloc(hpdir.model);
		return(1);
    }
#endif
	printf("hpdir invalid type - NOT AMIGO of SS80\n");
	return(0);
}


/// ===============================================
///@brief Lookup model in and set drive parameters if found
///
///@param[in] index: Devices index
///@param[in] model: model string
///
///@return 1 on success, 0 or failure
int8_t hpdir_set_parameters(int8_t index, char *model)
{
    if ( hpdir_find_drive( model, 0 ,1) )
        return(hpdir_set_device(index) );
	printf("WARNING: model NOT found in hpdir.ini!\n");
	return(0);
}

/// ===============================================
/// @brief Verify a device and delete it is there are any errors
/// @return  1 = OK 0 = ERROR
int8_t verify_device(int8_t index)
{
    long sectors;
	int8_t type;
	int address,ppr;
	int8_t ret = 1;	

	uint8_t  ppr_bits = 0;
	uint8_t  ppr_mask;
	uint32_t addr_bits = 0;
	uint32_t addr_mask;

///@brief Active SS80 Device
    SS80DiskType *SS80p = NULL;

#ifdef AMIGO
///@brief Active AMIGO Device
    AMIGODiskType *AMIGOp = NULL;
#endif

    if(Devices[index].TYPE == NO_TYPE)
        return(ret);

	address = Devices[index].ADDRESS;
    type = Devices[index].TYPE;
	ppr = Devices[index].PPR;

	if(address < 0 || address > 31)
	{
		printf("Address (%d) out of range\n", (int) address);
		ret = 0;
	}
	
	addr_mask = (1L << address);
	if(addr_bits & addr_bits)
	{
		printf("Address (%d) duplicated\n", (int) address);
		ret = 0;
	}
	addr_bits |= addr_mask;

	
	// Printers do not use PPR
	if(type == PRINTER_TYPE)
	{
		Devices[index].PPR = 0xff;
		return(1);
	}
	if(type == SS80_TYPE || AMIGO_TYPE)
	{
		if(ppr < 0 || ppr > 7)
		{
			ret = 0;
		}
		ppr_mask = (1 << ppr);
		if(ppr_bits & ppr_bits)
		{
			printf("PPR (%d) duplicated\n", (int) ppr);
			ret = 0;
		}
		ppr_bits |= ppr_mask;
	}
    if(type == SS80_TYPE)
    {
        SS80p= (SS80DiskType *)Devices[index].dev;
        if( SS80p->UNIT.BYTES_PER_BLOCK != 256)
        {
// SS80p->UNIT.BYTES_PER_BLOCK = 256;
            printf("Warning: %s BYTES_PER_BLOCK != 256, Adjusting to 256\n", SS80p->HEADER.model);
			ret = 0;
        }
        sectors = SS80p->VOLUME.MAX_BLOCK_NUMBER+1;
        Devices[index].BLOCKS = sectors;
    }                                         // SS80_TYPE

#ifdef AMIGO
    if(type == AMIGO_TYPE )
    {
        AMIGOp = (AMIGODiskType *)Devices[index].dev;
        if( AMIGOp->GEOMETRY.BYTES_PER_SECTOR != 256)
        {
            AMIGOp->GEOMETRY.BYTES_PER_SECTOR = 256;
            printf("Warning: %s BYTES_PER_SECTOR != 256, Adjusting to 256\n", SS80p->HEADER.model);
			ret = 0;
        }
        sectors = AMIGOp->GEOMETRY.SECTORS_PER_TRACK
            * AMIGOp->GEOMETRY.HEADS
            * AMIGOp->GEOMETRY.CYLINDERS;
        Devices[index].BLOCKS = sectors;
    }
#endif                                    // #ifdef AMIGO
	if(!ret)
	{
		printf("Device errors - removing: ");
		display_mount(index);	
		free_device(index);
	}
	return(ret);
}

/// ===============================================
/// @brief Post process and Verify all devices
/// @return  1 = OK 0 = ERROR
void verify_devices()
{
	int8_t i;
	for(i=0;i<MAX_DEVICES;++i)
		verify_device(i);
}
	


/// ===============================================
/// @brief Format devices that have no image file
/// @return  void
void format_drives()
{
    int i;
    struct stat st;
    long sectors;
    char label[32];
    int count =0;
    int ss80 = 0;
    int amigo = 0;

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
                sectors = Devices[i].BLOCKS;
                printf("formating %s %ld sectors\n", SS80p->HEADER.NAME, (long) sectors);
                sprintf(label,"SS80-%d", ss80);
#ifdef LIF_SUPPORT
                lif_create_image(SS80p->HEADER.NAME,
                    label,
                    lif_dir_count(sectors),
                    sectors);
#else
                printf("please create a SS80 LIF image with %ld sectors and 128 directory sectors\n", sectors);
#endif
                ++count;
                ++ss80;

            }
        }                                         // SS80_TYPE

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
                sectors = Devices[i].BLOCKS;
                printf("formating %s %ld sectors\n", AMIGOp->HEADER.NAME, (long) sectors);
                sprintf(label,"AMIGO%d", amigo);
#ifdef LIF_SUPPORT
                lif_create_image(AMIGOp->HEADER.NAME,
                    label,
                    lif_dir_count(sectors),
                    sectors);
#else
                printf("please create a AMIGO LIF image with %ld sectors and 15 directory sectors\n", sectors);
#endif
                ++count;
                ++amigo;
            }
        }
#endif                                    // #ifdef AMIGO
    }
    if(count)
        sep();
}

/// ===============================================
/// @brief mount_usage - testing
/// @return void
void mount_usage(void)
{
	printf("Usage: \n");
    printf("Mounting drives\n");
	printf("    mount AMIGO 9121 2 amigo-22.lif\n");
	printf("    mount SS80 9134D 3 ss80-3.lif\n");
	printf("\n");
	printf("Mounting printer\n");
	printf("    mount PRINTER 5\n");
	printf("\n");
    printf("Displaying mounted drives\n");
	printf("    mount\n");
}



/// @brief return index matching address 
/// @return index or -1 if not found
int8_t index_address(int8_t address)
{
	int8_t i;
    for(i=0;i<MAX_DEVICES;++i)
    {
        if(Devices[i].TYPE != NO_TYPE && Devices[i].ADDRESS == address)
            return(i);
    }
	return(-1);
}

/// @brief test if address is in use
/// @return index 1 if found, 0 if not
int8_t test_address(int8_t address)
{
	if(address < 0 || address > 31)
	{
		printf("WARNING Address (%d) out of range\n", (int)address);
		return(0);
	}

	if(index_address(address) == -1)
		return(1);
	printf("WARNING Address (%d) already in use\n", (int)address);
	return(0);
}


/// @brief return index matching ppr
/// @return index or -1 if not found
int8_t index_ppr(int8_t  ppr)
{
	int8_t i;
    for(i=0;i<MAX_DEVICES;++i)
    {
        if(Devices[i].TYPE != NO_TYPE && Devices[i].PPR == ppr)
            return(i);
    }
	return(-1);
}

/// @brief test if PPR is in use
/// @return index 1 if found, 0 if not
int8_t test_ppr(int8_t ppr)
{
	if(ppr < 0 || ppr > 7)
	{
		printf("WARNING PPR (%d) out of range\n", (int)ppr);
		return(0);
	}

	if(index_ppr(ppr) == -1)
		return(1);
	printf("WARNING PPR (%d) already in use\n", (int)ppr);
	return(0);
}

/// ===============================================
/// @brief umount disks - testing
/// @return Devices[] index on success, -1 on error
int8_t umount(int argc, char *argv[])
{

	int8_t address;
	int8_t index;

	if(argc != 2)
	{
		printf("Usage:\n");
		printf("  umount address\n");
		printf("  - address is the device address\n");
	}
	address = atoi(argv[1]);
	index = index_address(address);
	if(index == -1)
	{
		printf("umount address:[%d] NOT found\n", address);
		return(-1);
	}
	free_device(index);
	return(index);
}

/// ===============================================
/// @brief mount disks - testing
/// @return Devices[] index on success, -1 on error
int8_t mount(int argc, char *argv[])
{
	int8_t index = -1;
	
///@brief Active Printer Device
    PRINTERDeviceType *PRINTERp = NULL;
///@brief Active SS80 Device
    SS80DiskType *SS80p = NULL;
#ifdef AMIGO
///@brief Active AMIGO Device
    AMIGODiskType *AMIGOp = NULL;
#endif
#if 0
	int8_t i;
	for(i =1;i< argc;++i)
	{
		printf("argv[%d] = %s\n", (int) i, argv[i]);

	}
#endif
	if(argc == 1)
	{
		display_mounts();
		return(1);
	}
	else if(argc == 3)
	{
			if(MATCHI(argv[1], "PRINTER"))
			{
				// FIXME - do we want to have separtate address and ppr ?
				int8_t address = atoi(argv[2]) & 0xff;
				index = alloc_device(PRINTER_TYPE);
				if(index < 0)
				{
					printf("Could not allocate PRINTER structure\n");
					return(0);
				}
				PRINTERp = (PRINTERDeviceType *) Devices[index].dev;
				PRINTERp->HEADER.ADDRESS  = address;
				Devices[index].ADDRESS = address;
				Devices[index].PPR = 0xff;
				return( verify_device(index) );
			}
	}
	else if(argc == 4)
	{
		/*
		argv[1] = 9121
		argv[2] = 2
		argv[3] = amigo2.lif
		*/
		if(!hpdir_find_drive(argv[1],0,0) )
		{
			printf("WARNING: model NOT found in hpdir.ini!\n");
			return(-1);
		}
		if(MATCHI(hpdir.TYPE, "SS80") || MATCHI(hpdir.TYPE,"CS80") )
		{
			// FIXME - do we want to have separtate address and ppr ?
			int8_t address = atoi(argv[2]) & 0xff;
			int8_t ppr = address;
			index = alloc_device(SS80_TYPE);
			if(index < 0)
			{
				printf("Could not allocate SS80 structure for %s\n",argv[2]);
				return(0);
			}
			SS80p = (SS80DiskType *) Devices[index].dev;
			if( !hpdir_set_parameters(index, argv[1] ) )
				return(-1);
			SS80p->HEADER.NAME = stralloc(argv[3]);
			SS80p->HEADER.ADDRESS  = address;
			SS80p->HEADER.PPR = ppr;
			Devices[index].ADDRESS = address;
			Devices[index].PPR = ppr;
			return( verify_device(index) );
		}
#ifdef AMIGO
		else if(MATCHI(argv[1], "AMIGO"))
		{
			// FIXME - do we want to have separtate address and ppr ?
			int8_t address = atoi(argv[2]) & 0xff;
			int8_t ppr = address;
			index = alloc_device(AMIGO_TYPE);
			if(index < 0)
			{
				printf("Could not allocate AMIGO structure for %s\n",argv[2]);
				return(0);
			}
			AMIGOp = (AMIGODiskType *) Devices[index].dev;
			if( !hpdir_set_parameters(index, argv[1] ) )
				return(-1);
			AMIGOp->HEADER.NAME = stralloc(argv[3]);
			AMIGOp->HEADER.ADDRESS  = address;
			AMIGOp->HEADER.PPR = ppr;
			Devices[index].ADDRESS = address;
			Devices[index].PPR = ppr;
			return( verify_device(index) );
		}
#endif
		else
		{
			printf("Expected AMIGO or SS80 [%s]\n",argv[1]);
			mount_usage();
			return(0);
		}
	}
	else 
	{
		mount_usage();
		return(0);
	}
	return(1);
}


void display_mount(int8_t index )
{

///@brief Active Printer Device
    PRINTERDeviceType *PRINTERp = NULL;
///@brief Active SS80 Device
    SS80DiskType *SS80p = NULL;

#ifdef AMIGO
///@brief Active AMIGO Device
    AMIGODiskType *AMIGOp = NULL;
#endif

	if(Devices[index].TYPE == NO_TYPE)
		return;

	if(Devices[index].TYPE == SS80_TYPE)
	{
		SS80p= (SS80DiskType *)Devices[index].dev;

		printf("SS80    %-8s %2d %s\n", SS80p->HEADER.model, (int) SS80p->HEADER.ADDRESS, SS80p->HEADER.NAME);
	}

#ifdef AMIGO
	if(Devices[index].TYPE == AMIGO_TYPE )
	{
		AMIGOp = (AMIGODiskType *)Devices[index].dev;
		printf("AMIGO   %-8s %2d %s\n", AMIGOp->HEADER.model, (int) AMIGOp->HEADER.ADDRESS, AMIGOp->HEADER.NAME);
	}
#endif                                    // #ifdef AMIGO

	if(Devices[index].TYPE == PRINTER_TYPE )
	{
            PRINTERp= (PRINTERDeviceType *)Devices[index].dev;
            printf("PRINTER %-8s %2d\n", " ", (int) PRINTERp->HEADER.ADDRESS);
	}
}

void display_mounts()
{
	int8_t i;
	printf("Mounted drives\n");
	for(i=0;i<MAX_DEVICES;++i)
		display_mount(i);
    printf("\n");
}
