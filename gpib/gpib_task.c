/**
 @file gpib/gpib_task.c

 @brief High level GPIB command handler for HP85 disk emulator project for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/


#include "user_config.h"

#include "posix.h"
#include "defines.h"
#include "gpib_hal.h"
#include "gpib.h"
#include "gpib_task.h"
#include "amigo.h"
#include "ss80.h"
#include "printer.h"

/// @brief Config file name
char cfgfile[] = "/hpdisk.cfg";                   
FIL  fp_file;

/// @brief Debug flag - used to log GPIB and emulator messages
uint8_t debuglevel;

/// @brief GPIB log file handel
FILE *gpib_log_fp;


void gpib_file_init()
{

    debuglevel = 0;                               // Default loglevel

    //FatFs_Read_Config(cfgfile);

    POSIX_Read_Config(cfgfile);

    if(mmc_wp_status())
    {
        printf("Card is write protected\n");
    }
}


/// @brief  Log GPIB transactions
///
/// @param str: message to log
/// @return  void
FILE *gpib_log_fp;
void gpib_log( char *str )
{
    if(gpib_log_fp != NULL )
        fprintf(gpib_log_fp,"%s\n", str);
}


/// @brief  Read GPIB emulator Configuration file
///
/// Sets debuglevel
/// @return  0

/// @brief  Check if SS80 listening address
///
/// - Used at power up, Bus IFC or user aborts
/// @return  1 if true or 0
int SS80_is_MLA(int addr)
{
	if(addr == SS80_MLA)
		return(1);
	return(0);
}

/// @brief  Check if SS80 talking address
///
/// - Used at power up, Bus IFC or user aborts
/// @return  1 if true or 0
int SS80_is_MTA(int addr)
{
	if(addr == SS80_MTA)
		return(1);
	return(0);
}

/// @brief  Check if SS80 secondary address
///
/// - Used at power up, Bus IFC or user aborts
/// @return  1 if true or 0
int SS80_is_MSA(int addr)
{
	if(addr == SS80_MSA)
		return(1);
	return(0);
}

/// @brief  Check if AMIGO listening address
///
/// - Used at power up, Bus IFC or user aborts
/// @return  1 if true or 0
int AMIGO_is_MLA(int addr)
{
	if(addr == AMIGO_MLA)
		return(1);
	return(0);
}

/// @brief  Check if AMIGO talking address
///
/// - Used at power up, Bus IFC or user aborts
/// @return  1 if true or 0
int AMIGO_is_MTA(int addr)
{
	if(addr == AMIGO_MTA)
		return(1);
	return(0);
}
/// @brief  Check if AMIGO secondary address
///
/// - Used at power up, Bus IFC or user aborts
/// @return  1 if true or 0
int AMIGO_is_MSA(int addr)
{
	if(addr == AMIGO_MSA)
		return(1);
	return(0);

}

/// @brief  Check if PRINTER listening address
///
/// - Used at power up, Bus IFC or user aborts
/// @return  1 if true or 0
int PRINTER_is_MLA(int addr)
{
	if(addr == PRINTER_MLA)
		return(1);
	return(0);
}

/// @brief  Check if PRINTER talking address
///
/// - Used at power up, Bus IFC or user aborts
/// @return  1 if true or 0
int PRINTER_is_MTA(int addr)
{
	if(addr == PRINTER_MTA)
		return(1);
	return(0);
}

/// @brief  Check if PRINTER secondary address
///
/// - Used at power up, Bus IFC or user aborts
/// @return  1 if true or 0
int PRINTER_is_MSA(int addr)
{
	if(addr == PRINTER_MSA)
		return(1);
	return(0);
}

/// @brief  Trace GPIB activity passively - saving to a log file
///
/// - We use the natural GPIB handshaking to slow GPIB talker/listeners 
/// so we can keep up
/// - A keypress will exit the trace and close the file
///
/// @param name: File name to save log file to.
/// @return  void
///   Exit on Key Press

void gpib_trace_task( char *name )
{
    int ch;
    long count;
    char str[32];

    gpib_log_fp = NULL;

    if(name && *name)
    {
        name = skipspaces(name);
        printf("Capturing GPIB BUS to:%s\n", name);

        gpib_log_fp = fopen(name,"w");
        if(gpib_log_fp == NULL)
        {
            perror("open failed");
            printf("exiting...\n");
            return;
        }
    }

    gpib_bus_init(1);                             // Warm init float ALL lines
    gpib_state_init();                            // Init PPR talking and listening states
    gpib_init_devices();

    gpib_decode_header();
    count = 0;
    while(1)                                      // Main loop, forever
    {
        if(uart_keyhit(0))
            break;

        ch = gpib_read_byte();

        gpib_decode_str(ch, str);
        gpib_log(str);
        puts(str);

        if(( count & 255L ) == 0)
            printf("%08ld\r",count);
        ++count;
    }

    printf("\nLogged %ld\n",count);
    if(gpib_log_fp)
    {
        fclose(gpib_log_fp);
        printf("Capturing Closed\n");
        gpib_log_fp = NULL;
    }
}


/// @brief Check for GPIB errors and timeouts
///
/// - Reset GPIB bus on IFC or user keypress
/// - Display debug messages if debuglevel permits. 
/// - Check media insert state
///
/// @param val: command or data byte and control or error flags.
///   - Lower 8 bits: Data or Command.
///     - If ATN is LOW then we strip parity from the byte.
///   - Upper 8 bits: Status and Errors.
///     - @see gpib.h _FLAGS defines for a full list.
///     - An error implies the data byte can't be trusted
///     - Control Line Flags.
///       - EOI_FLAG
///       - SRQ_FLAG
///       - ATN_FLAG
///       - REN_FLAG
///       - PP_FLAG
///     - Error Flags:
///       - IFC_FLAG
///  - EOI_FLAG
///  - ATN_FLAG
/// @see gpib.h _FLAGS defines for a full list.
///
/// @return  val
/// @return ABORT flag on user keypress

uint16_t gpib_error_test(uint16_t val)
{

    if(val & ERROR_MASK || uart_keyhit(0) || mmc_ins_status() != 1 )
    {
        val &= ERROR_MASK;

#if 0                                     // GPIB BUS init/reset are done in gpib_read() and gpib_write()
        if(val & IFC_FLAG || uart_keyhit(0))
        {
            gpib_bus_init(1);                     // Warm init float ALL lines
            gpib_state_init();                    // Init PPR talking and listening states
        }
#endif

#if SDEBUG >= 1
        if(debuglevel >= 1)
        {
            if(val & IFC_FLAG)
                printf("<IFC>\n");
            if(val & TIMEOUT_FLAG)
                printf("<TIMEOUT>\n");
            if(val & BUS_ERROR_FLAG)
                printf("<BUS>\n");
        }
#endif
        if(uart_keyhit(0))
            printf("<INTERRUPT>\n");

        if( mmc_ins_status() != 1 )
            printf("<MEDIA MISSING>\n");

        if(val & IFC_FLAG)
        {
            gpib_init_devices();
        }

#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("\n");
#endif

        if(uart_keyhit(0))
        {
            gpib_init_devices();
            return(ABORT_FLAG);
        }

/// @todo  do we want to always exit here ?
///  low level GPIB functions are still useful even without a DISK
        if( mmc_ins_status() != 1 )
        {
            return(ABORT_FLAG);
        }

        if(val & IFC_FLAG )
        {
            while(GPIB_IO_RD(IFC) == 0)
                ;
        }
        return(val);
    }
    return(0);
}


/// @brief  Initialize ALL emulated devices SS80, AMIGO and printer
///
/// - Used at power up, Bus IFC or user aborts
/// @return  void
void gpib_init_devices(void)
{
    SS80_init();                                  // SS80 state init
#ifdef AMIGO
    amigo_init();                                 // AMIGO state init
#endif
    printer_close();                              // Close any open fprinter files
}


/// @brief  Process all GPIB Secondary Commands
///
/// - Dispatch emulator handler based on address
/// - if unread is true we "unread" val so the corresponding device
/// emulator can process it.
/// - Unread allow us to write each emulator as a pure function.
///
/// @param[in] val: GPIB secondary command and control flags.
/// @param[in] unread: if 1 unread val prior to emulator call.
///
/// @return GPIB status from emulator command.
/// @return 0 if no emulator command processed.

uint16_t GPIB_COMMANDS(uint16_t val, uint8_t unread)
{
    uint16_t status;

	///@brief talking ?
    if(talking != UNT)
    {

#ifdef AMIGO
        if ( AMIGO_is_MLA(listening) )
        {
            if(unread)
                gpib_unread(val);
			// secondary was previously set
            status = AMIGO_COMMANDS(secondary);
            secondary = 0;
            return(status);
        }
#endif                                    // ifdef AMIGO

        if ( SS80_is_MLA(listening) )
        {
            if(unread)
                gpib_unread(val);
			// secondary was previously set
            status = SS80_COMMANDS(secondary);
            secondary = 0;
            return(status);
        }

        if ( PRINTER_is_MLA(listening) )
        {
            if(unread)
                gpib_unread(val);
            status = PRINTER_COMMANDS(secondary);
            secondary = 0;
            return(status);
        }
    }

	///@brief listening ?
    if(listening != UNL)
    {
#ifdef AMIGO
        if ( AMIGO_is_MTA(talking) )
        {
            if(unread)
                gpib_unread(val);
            status = AMIGO_COMMANDS(secondary);
            secondary = 0;
            return(status);
        }
#endif                                    // ifdef AMIGO

        if ( SS80_is_MTA(talking) )
        {
            if(unread)
                gpib_unread(val);
            status = SS80_COMMANDS(secondary);
            secondary = 0;
            return(status);
        }

        if ( PRINTER_is_MTA(talking) )
        {
            if(unread)
                gpib_unread(val);
            status = PRINTER_COMMANDS(secondary);
            secondary = 0;
            return(status);
        }
    }
    return(0);
}


/// @brief  Top most main GPIB device emulator task.
/// This is main() for GPIB state machine loop
/// All tasks are dispatched from here
///
/// - Initializes BUS, Devices, State.
/// - Reads and processes GPIB control or data bytes
/// and calls emulator task.
/// @return  void

void gpib_task(void)
{
    uint16_t val;
    uint8_t ch;
    uint16_t status;
    char str[40];

    gpib_bus_init(1);                             // Warm init float ALL lines
    gpib_state_init();                            // Init PPR talking and listening states
    gpib_init_devices();                          // Init defives

    gpib_log_fp = NULL;

    while(1)
    {

        val = gpib_read_byte();

#if SDEBUG >= 1
        if(debuglevel >= 9)
        {
            gpib_decode_str(val, str);
            puts(str);
        }
#endif
        status = gpib_error_test(val);
        if(status & ABORT_FLAG)
        {
            return;
        }
        else if(status & MEDIA_FLAG)
        {
            return;
        }
        else if(status)
        {
            continue;
        }
		///@brief GPIB commands with ATN set
        if(val & ATN_FLAG)
        {
            ch = val & CMD_MASK;
            if(ch <= 0x1f)
            {
                GPIB(ch);
                continue;
            }
			///@brief GPIB listen
            if(ch >= 0x20 && ch <= 0x3f)
            {
                GPIB_LISTEN(ch);
                continue;
            }
			///@brief GPIB talk
            if(ch >= 0x40 && ch <= 0x5f)
            {
                GPIB_TALK(ch);
                continue;
            }

			///@brief GPIB secondary
			/// Note: We know ch >= 0x60 && ch <= 0x7f because of previous tests

            if( listening && lastcmd == UNT)
            {
                secondary = 0;
                GPIB_SECONDARY_ADDRESS(ch);
                continue;
            }

			///@brief We have to keep track of secondary address that may happen out of order with older AMIGO protocol
			/// this method works for SS80 as well
            secondary = ch;
			///@brief GPIB_COMMANDS does most of the work
            status = GPIB_COMMANDS(secondary,0);
            status = gpib_error_test(status);
            if(status & ( ABORT_FLAG | MEDIA_FLAG ))
            {
                return;
            }
            if(status)
            {
                continue;
            }
            continue;

        }                                         // GPIB ATN

		///@brief GPIB commands without ATN set
        else                                      // GPIB Data
        {
            if ( PRINTER_is_MLA(listening) )
            {
                printer_buffer( 0xff & val );
                continue;
            }

            if(!secondary)
                continue;

			///@brief GPIB_COMMANDS does most of the work
            status = GPIB_COMMANDS(val,1);
            status = gpib_error_test(status);

            if(status & ( ABORT_FLAG | MEDIA_FLAG ))
            {
                return;
            }
            if(status)
            {
                continue;
            }
            continue;
        }
    }                                             // while(1)
    return;
}



/// @brief  HEX and ASCII dump of string in human-readable format
///
/// - Used only for debugging
/// @param[in] ptr: data
/// @param[in] length: length of data string
///
/// @return  void

void DumpData(unsigned char *ptr,int length)
{
    int i,j;
    char ch;
    printf("[Dump: %d]\n",length);
    for(j=0;j<80&&(j*16<length);j++)
    {
        printf("\n");
        for(i=0;i<16 && (i+j*16<length);i++)
        {
            ch = *(ptr+i+j*16);
            printf(" %02X",ch&0xFF);
        }
        printf(" | ");
        for(i=0;i<16 && (i+j*16<length);i++)
        {
            if(*(ptr+i+j*16)>' ') printf("%c",(*(ptr+i+j*16))&0xFF);
            else printf(".");
        }
    }
    printf("\n");
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

	printf("debuglevel   = %02x\n",(int)debuglevel);
	printf("\n");
	printf("ss80_addr    = %02x\n",(int)SS80Disk.ss80_addr);
	printf("ss80_ppr     = %02x\n",(int)SS80Disk.ss80_ppr);
	printf("\n");
	printf("amigo_addr   = %02x\n",(int)AMIGODisk.amigo_addr);
	printf("amigo_ppr    = %02x\n",(int)AMIGODisk.amigo_ppr);
	printf("\n");
	printf("printer_addr = %02x\n",(int)printer_addr);
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



/// @brief Read and parse a config file using POSIX functions
///
/// - Set debuglevel and other device settings
///
/// @param name: config file name to process
///
/// @return  0

void POSIX_Read_Config(char *name)
{
    int ret;
    int len;
    int lines;
    char str[128];
    char *ptr;
    FILE *cfg;
	int val;

	printf("Reading: %s\n", name);
    cfg = fopen(name, "r");
    if(cfg == NULL)
    {
        perror("Read_Config - open");
        return;
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

		if ( set_value(ptr,"DEBUG", 0, 255, &val) )
			debuglevel = val;
		if ( set_value(ptr,"SS80_DEFAULT_ADDRESS", 0, 14, &val) )
			SS80Disk.ss80_addr = val;
		if ( set_value(ptr,"SS80_DEFAULT_PPR", 0, 7, &val) )
			SS80Disk.ss80_ppr = val;
		if ( set_value(ptr,"AMIGO_DEFAULT_ADDRESS", 0, 14, &val) )
			AMIGODisk.amigo_addr = val;
		if ( set_value(ptr,"AMIGO_DEFAULT_PPR", 0, 7, &val) )
			AMIGODisk.amigo_ppr = val;
		if ( set_value(ptr,"PRINTER_DEFAULT_ADDRESS", 0, 14, &val) )
			printer_addr = val;
    }

    printf("Read_Config: read(%d) lines\n", lines);

    ret = fclose(cfg);
    if(ret == EOF)
    {
        perror("Read_Config - close error");
    }
	display_settings();
}



/// @brief  Send drive identify- 2 bytes
///
/// - References
///  - Transparent Command IDENT
///  - SS80 4-31
///  - CS80 pg 4-27, 3-10
///  - A11
///
/// @param[in] ch: channel
///
/// @return  0 on GPIB error returns error flags
/// @see gpib.h ERROR_MASK for a full list.

int Send_Identify(uint8_t ch, IdentifyType id)
{
    uint16_t status = EOI_FLAG;
	if(gpib_write_str((uint8_t *)&id,sizeof(id), &status) 
		!= sizeof(id))
	{
#if SDEBUG >= 1
		if(debuglevel >= 1)
			printf("[IDENT Unit:%02x=%02x%02x FAILED]\n", (int)ch,(int)id.I1,(int)id.I2);
#endif
		return(status & ERROR_MASK);
	}
#if SDEBUG > 1
    if(debuglevel > 1)
		printf("[IDENT Unit:%02x=%02x%02x]\n", (int)ch,(int)id.I1,(int)id.I2);
#endif
    return (status & ERROR_MASK);
}


/// @brief  Main GPIB command handler
/// Commands 0x00 .. 0x1f
/// - We only ever called with ATN=1 commands.
/// - ch = GPIB command and status (ie it has the ATN_FLAG set).
/// - Notes: We track any device states and call handelers.
///
/// @param[in] ch 8 bit command
///
/// @return  0 on sucess
/// @return GPIB error flags on fail
/// @see gpib.h ERROR_MASK defines for a full list.

int GPIB(uint8_t ch)
{
/// @todo  TODO
	///@brief Parallel Poll Configure
    if(ch == PPC)
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[PPC unsupported]\n");
#endif
        spoll = 0;
        return 0;
    }
	///@brief Parallel Poll Unconfigure
    if(ch == PPU)
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[PPU unsupported]\n");
#endif
        spoll = 0;
        return 0;
    }

/// @todo FIXME
#if defined(SPOLL)
	///@brief Serial Poll Enable
    if(ch == SPE)
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[SPE]\n");
#endif
        spoll = 1;
        if(SS80_is_MTA(talking))
        {
            return( SS80_Report() );
        }
        return 0;
    }

	///@brief Serial Poll Disable
    if(ch == SPD)
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[SPD]\n");
#endif
        spoll = 0;
        return 0;
    }
#endif

	///@brief Selected Device Clear
    if(ch == SDC )
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[SDC]\n");
#endif
        if(SS80_is_MLA(listening))
        {
///  Note: Suposed to be unsupported in SS80 - pg 4-2
///  CS80 3-4
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SDC SS80]\n");
#endif
            return(SS80_Selected_Device_Clear(SS80State.unitNO) );
        }

#ifdef AMIGO
        if(AMIGO_is_MLA(listening))
        {
///  Note: Suposed to be unsupported in SS80 - pg 4-2
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SDC AMIGO]\n");
#endif
            return( amigo_cmd_clear() );
        }
#endif

/// @todo FIXME
        return( 0 );
    }

	///@brief 	(Universal) Device Clear
    if(ch == DCL )
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[DCL]\n");
#endif
        SS80_Universal_Device_Clear();

#ifdef AMIGO
        amigo_cmd_clear();
#endif

/// @todo Fixme
        printer_close();
        return( 0 );
    }

#if SDEBUG >= 1
    if(debuglevel > 1)
        printf("[HPIB (%02x) not defined]\n", 0xff & ch);
#endif
    return(0);
}


/// @brief  Process all GPIB Listen commands
/// - We only ever called with ATN=1 commands.
///
/// @param[in] ch 8 bit listen command
///
/// @return  0 

int GPIB_LISTEN(uint8_t ch)
{


    listening_last = listening;
    listening = ch;

	device = ch;

    listen_cleanup();

///  NOTE: we must track the "addressed state" of each device
///  so we can determine its state - ie: command vs Secondary states
	///@brief  Universal Unlisten
    if(ch == UNL)                                 
    {
        listening = 0;
		device = 0;
	
#if SDEBUG > 1
        if(debuglevel > 1)
		{
            printf("[UNL]\n");
			///@brief add a break if weboth Untalk and Unlisten
			if(lastcmd == UNT)
				printf("\n");
		}
#endif
        return(0);
    }

#ifdef AMIGO
    if(AMIGO_is_MLA(ch))
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[LA %02x AMIGO]\n", 0xff & ch);
#endif
        return(0);
    }
#endif

    if(SS80_is_MLA(ch))
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[LA %02x SS80]\n", 0xff & ch);
#endif
        return(0);
    }

    if(PRINTER_is_MLA(ch))
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[LA %02x PRINTER]\n", 0xff & ch);
#endif
        if(talking != UNT)
        {
            printer_open(NULL);
        }
        return(0);
    }
#if SDEBUG > 1
    if(debuglevel > 1)
        printf("[LA %02x]\n", 0xff & ch);
#endif
    return(0);
}                                                 // Listen Primary Address group


/// @brief  Process all GPIB Talk commands
/// - We only ever called with ATN=1 commands.
///
/// @param[in] ch 8 bit talk command
///
/// @return  0

int GPIB_TALK(uint8_t ch)
{

///  NOTE: we must track the "addressed state" of each device
///  so we can determine its state - ie: command vs Secondary states
///  save talking state
    talking_last = talking;
    talking = ch;

    talk_cleanup();

	///@brief  Universal Untalk
    if(ch == UNT)
    {
		//FIXME talking = 0 ????
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[UNT]\n");
#endif
        return(0);
    }

	
    if(SS80_is_MTA(ch))                            // SS80
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[TA %02x SS80]\n", 0xff & ch);
#endif

        if (spoll)
        {
            SS80_Report();
        }
        return(0);
    }

#ifdef AMIGO
    if(AMIGO_is_MTA(ch))                           // AMIGO
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[TA %02x AMIGO]\n", 0xff & ch);
#endif
        return(0);
    }
#endif

    if(PRINTER_is_MTA(ch))                         // PRINTER
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[TA %02x PRINTER]\n", 0xff & ch);
#endif
        return(0);
    }

    if(PRINTER_is_MLA(listening))
    {
        printer_open(NULL);
        return(0);
    }

#if SDEBUG > 1
    if(debuglevel > 1)
        printf("[TA %02x]\n", 0xff & ch);
#endif
    return(0);
}                                                 // Talk Address primary address group


/// @brief  Process all GPIB Secondary Addresses
/// - We only ever called with ATN=1 commands.
/// Secondary Address, talking == UNT
///
/// @param[in] ch 8 bit secondary address.
///
/// @return  0
int GPIB_SECONDARY_ADDRESS(uint8_t ch)
{
///  NOTE: We now know that ch >= 0x60 && ch <= 0x7f
///  Previous tests ensure this fact is true and because CMD_MASK == 0x7f
	device = ch;

///  note: any errors will reset lastcmd
///  Universal Talk mode
///  Treat this as a Secondary Address ?
///  SS80 Ident 4-31
///  If we have our secondary address then send IDENT
    if(SS80_is_MSA(ch) )
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[SA %02x SS80]\n", 0xff & ch);
#endif
		///@brief ch = secondary address
        DisablePPR(SS80_PPR);
        return( Send_Identify( ch, SS80Disk.id) );

    }

#ifdef AMIGO
    if(AMIGO_is_MSA(ch) )
    {
/// @todo 
/// 	Two identify bytes should be repeated until untalked
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[SA %02x AMIGO]\n", 0xff & ch);
#endif
		///@brief ch = secondary address
        DisablePPR(AMIGO_PPR);
        return( Send_Identify( ch, AMIGODisk.id) );
    }
#endif                                        // AMIGO

#if SDEBUG > 1
    if(debuglevel > 1)
        printf("[SA %02x, listen:%02x, talk:%02x]\n",
            0xff & ch, 0xff & listening, 0xff & talking);
#endif
    return(0);
} 


/// @brief  Called when the listen address changes.
///
/// - Used to cleanup or close at the end of a listen transation.
/// - Also called when GPIB bus reset or unlisten.
/// @return  void

void listen_cleanup()
{
    if(listening_last == PRINTER_MLA)
    {
        printer_close();
    }
}


/// @brief  Called when the GPIB talk address changes
///
/// - Used to cleanup or close at the end of a talk transation.
/// - Also called when GPIB bus reset or untalk.
/// - Not used in this emulator.
/// 
/// @return  void

void talk_cleanup()
{

}
