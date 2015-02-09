/**
 @file gpib/gpib_task.c

 @brief High level GPIB command handler for HP85 disk emulator project for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/


#include "hardware/hardware.h"

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

/// @brief protocol flag - not used at this time
uint8_t protocol;
/// @brief Debug flag - used to log GPIB and emulator messages
uint8_t debuglevel;

/// @brief GPIB log file handel
FILE *gpib_log_fp;


/// @brief  Read GPIB emulator Configuration file
///
/// Sets protocol and debuglevel
/// @return  0

void gpib_file_init()
{

    protocol = 1;                                 // Default protocol: SS/80
    debuglevel = 0;                               // Default loglevel

    myprintf("HP Disk and Device Emulator\n");
    myprintf("Created on:%s %s\n", __DATE__,__TIME__);

#ifdef SOFTWARE_PP
    myprintf("\nSoftware PP\n");
#else
    myprintf("\nHardware PP\n");
#endif                                        // SOFTWARE_PP

    delayms(500);

    FatFs_Read_Config(cfgfile);


#if defined(HP9122D)
    myprintf("SS/80 9122D\n");
#endif

#if defined(HP9134L)
    myprintf("SS/80 9134L\n");
#endif

#if defined(HP9121D)
    myprintf("Amigo 9121D\n");
#endif

    if(mmc_wp_status())
    {
        myprintf("Card is write protected\n");
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
        myprintf("Capturing GPIB BUS to:%s\n", name);

        gpib_log_fp = fopen(name,"w");
        if(gpib_log_fp == NULL)
        {
            perror("open failed");
            myprintf("exiting...\n");
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
            myprintf("%08ld\r",count);
        ++count;
    }

    myprintf("\nLogged %ld\n",count);
    if(gpib_log_fp)
    {
        fclose(gpib_log_fp);
        myprintf("Capturing Closed\n");
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
                myprintf("<IFC>\n");
            if(val & TIMEOUT_FLAG)
                myprintf("<TIMEOUT>\n");
            if(val & BUS_ERROR_FLAG)
                myprintf("<BUS>\n");
        }
#endif
        if(uart_keyhit(0))
            myprintf("<INTERRUPT>\n");

        if( mmc_ins_status() != 1 )
            myprintf("<MEDIA MISSING>\n");

        if(val & IFC_FLAG)
        {
            gpib_init_devices();
        }

#if SDEBUG >= 1
        if(debuglevel >= 1)
            myprintf("\n");
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

    if(talking != UNT)
    {

#ifdef AMIGO
        if ( listening == AMIGO_MLA )
        {
            if(unread)
                gpib_unread(val);
            status = AMIGO_COMMANDS(secondary);
            secondary = 0;
            return(status);
        }
#endif                                    // ifdef AMIGO

        if ( listening == SS80_MLA )
        {
            if(unread)
                gpib_unread(val);
            status = SS80_COMMANDS(secondary);
            secondary = 0;
            return(status);
        }

        if ( listening == PRINTER_MLA )
        {
            if(unread)
                gpib_unread(val);
            status = PRINTER_COMMANDS(secondary);
            secondary = 0;
            return(status);
        }
    }

    if(listening != UNL)
    {
#ifdef AMIGO
        if ( talking == AMIGO_MTA )
        {
            if(unread)
                gpib_unread(val);
            status = AMIGO_COMMANDS(secondary);
            secondary = 0;
            return(status);
        }
#endif                                    // ifdef AMIGO

        if ( talking == SS80_MTA )
        {
            if(unread)
                gpib_unread(val);
            status = SS80_COMMANDS(secondary);
            secondary = 0;
            return(status);
        }

        if ( talking == PRINTER_MTA )
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


/// @brief  Top most GPIB device emulator task.
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
        if(val & ATN_FLAG)
        {
            ch = val & CMD_MASK;
            if(ch <= 0x1f)
            {
                GPIB(ch);
                continue;
            }
            if(ch >= 0x20 && ch <= 0x3f)
            {
                GPIB_LISTEN(ch);
                continue;
            }
            if(ch >= 0x40 && ch <= 0x5f)
            {
                GPIB_TALK(ch);
                continue;
            }
            if( listening && lastcmd == UNT)
            {
                secondary = 0;
                GPIB_SECONDARY_ADDRESS(ch);
                continue;
            }

            secondary = ch;

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

        else                                      // GPIB Data
        {
            if ( listening == PRINTER_MLA )
            {
                printer_buffer( 0xff & val );
                continue;
            }

            if(!secondary)
                continue;

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
    myprintf("[Dump: %d]\n",length);
    for(j=0;j<80&&(j*16<length);j++)
    {
        myprintf("\n");
        for(i=0;i<16 && (i+j*16<length);i++)
        {
            ch = *(ptr+i+j*16);
            myprintf(" %02X",ch&0xFF);
        }
        myprintf(" | ");
        for(i=0;i<16 && (i+j*16<length);i++)
        {
            if(*(ptr+i+j*16)>' ') myprintf("%c",(*(ptr+i+j*16))&0xFF);
            else myprintf(".");
        }
    }
    myprintf("\n");
}


/// @brief Read and parse a config file using FatFs functions
///
/// - Set protocol and debuglevel
/// - protocol is not being used
///
/// @param name: config file name to process
///
/// @return  0

void FatFs_Read_Config(char *name)
{
    int ret;
    int len;
    int lines;
    char *str = (char *) gpib_iobuff;             // not being used at this moment
    char *ptr;
    FIL cfg;

    ret = dbf_open(&cfg, name, FA_OPEN_EXISTING | FA_READ);
    if(ret != RES_OK)
        return;

    lines = 0;
    while(f_gets(str,253,&cfg) != NULL)
    {
        ++lines;
        ptr = str;
        trim_tail(str);
        len = strlen(str);
        if(!len)
            continue;
        if( (ret = token(str, "PROTO")) )
        {
            ptr += ret;
            ptr = skipspaces(ptr);
            protocol = atoi(ptr) & 0xff;
            myprintf("protocol=%d\n", protocol);
        }
        if( (ret = token(str, "DEBUG")) )
        {
            ptr += ret;
            ptr = skipspaces(ptr);
            debuglevel= atoi(ptr) & 0xff;
            myprintf("debuglevel=%d\n", debuglevel);
        }
    }

    myprintf("Read_Config: read(%d) lines\n", lines);

    dbf_close(&cfg);
}


/// @brief Read and parse a config file using POSIX functions
///
/// - Set protocol and debuglevel
/// - protocol is not being used
///
/// @param name: config file name to process
///
/// @return  0

void POSIX_Read_Config(char *name)
{
    int ret;
    int len;
    int lines;
    char *str = (char *) gpib_iobuff;  //< not being used at this moment
    char *ptr;
    FILE *cfg;

    cfg = fopen(name, "r");
    if(cfg == NULL)
    {
        perror("Read_Config - open");
        return;
    }

    lines = 0;
    while(fgets(str,253,cfg) != NULL)
    {
        ++lines;
        ptr = str;
        trim_tail(str);
        len = strlen(str);
        if(!len)
            continue;
        if( (ret = token(str, "PROTO")) )
        {
            ptr += ret;
            ptr = skipspaces(ptr);
            protocol = atoi(ptr) & 0xff;
            myprintf("protocol=%d\n", protocol);
        }
        if( (ret = token(str, "DEBUG")) )
        {
            ptr += ret;
            ptr = skipspaces(ptr);
            debuglevel= atoi(ptr) & 0xff;
            myprintf("debuglevel=%d\n", debuglevel);
        }
    }

    myprintf("Read_Config: read(%d) lines\n", lines);

    ret = fclose(cfg);
    if(ret == EOF)
    {
        perror("Read_Config - close error");
    }
}


/// @brief  Send drive identify- 2 bytes
///
/// - References
///  - Transparent Command IDENT
///  - SS80 4-31
///  - CS80 pg 4-27, 3-10
///  - A11
///
/// @param[in] byte1: first Send Identify byte
/// @param[in] byte2: second Send Identify byte
///
/// @return  0 on GPIB error returns error flags
/// @see gpib.h ERROR_MASK for a full list.

int Send_Identify( uint8_t byte1, uint8_t byte2)
{
    uint16_t status;
    uint8_t tmp[2];

    tmp[0] = byte1;
    tmp[1] = byte2;

    status = EOI_FLAG;
    if( gpib_write_str(tmp,sizeof(tmp), &status) != sizeof(tmp))
    {
#if SDEBUG >= 1
        if(debuglevel >= 1)
            myprintf("[IDENT failed]\n");
#endif
        return(status & ERROR_MASK);
    }
#if SDEBUG > 1
    if(debuglevel > 1)
        myprintf("[IDENT %02x %02x]\n", 0xff & byte1, 0xff & byte2);
#endif
    return (status & ERROR_MASK);
}


/// @brief  Main GPIB command handler
///
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
    if(ch == 0x15)
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[PPU unsupported]\n");
#endif
        spoll = 0;
        return 0;
    }

/// @todo FIXME
#if defined(SPOLL)
    if(ch == SPE)
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[SPE]\n");
#endif
        spoll = 1;
        if(talking == SS80_MTA)
        {
            return( SS80_Report() );
        }
        return 0;
    }

    if(ch == SPD)
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[SPD]\n");
#endif
        spoll = 0;
        return 0;
    }
#endif

    if(ch == SDC )
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[SDC]\n");
#endif
        if(listening == SS80_MLA)
        {
            extern BYTE Unit;
///  Note: Suposed to be unsupported in SS80 - pg 4-2
///  CS80 3-4
#if SDEBUG > 1
            if(debuglevel > 1)
                myprintf("[SDC SS80]\n");
#endif
            return( SS80_Selected_Device_Clear( Unit) );
        }

#ifdef AMIGO
        if(listening == AMIGO_MLA)
        {
///  Note: Suposed to be unsupported in SS80 - pg 4-2
#if SDEBUG > 1
            if(debuglevel > 1)
                myprintf("[SDC AMIGO]\n");
#endif
            return( amigo_cmd_clear() );
        }
#endif

/// @todo FIXME
        return( 0 );
    }

    if(ch == DCL )
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[DCL]\n");
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
        myprintf("[HPIB (%02x) not defined]\n", 0xff & ch);
#endif
    return(0);
}


/// @brief  Process all GPIB Listen commands
///
/// @param[in] ch 8 bit listen command
///
/// @return  0 

int GPIB_LISTEN(uint8_t ch)
{


    listening_last = listening;
    listening = ch;
    listen_cleanup();

///  NOTE: we must track the "addressed state" of each device
///  so we can determine its state - ie: command vs Secondary states
    if(ch == UNL)                                 // Universal Unlisten
    {
        listening = 0;
#if SDEBUG > 1
        if(debuglevel > 1)
		{
            myprintf("[UNL]\n");
			if(lastcmd == UNT)
				myprintf("\n");
		}
#endif
        return(0);
    }

#ifdef AMIGO
    if(ch == AMIGO_MLA)
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[LA %02x AMIGO]\n", 0xff & ch);
#endif
        return(0);
    }
#endif

    if(ch == SS80_MLA)
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[LA %02x SS80]\n", 0xff & ch);
#endif
        return(0);
    }

    if(ch == PRINTER_MLA)
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[LA %02x PRINTER]\n", 0xff & ch);
#endif
        if(talking != UNT)
        {
            printer_open(NULL);
        }
        return(0);
    }
#if SDEBUG > 1
    if(debuglevel > 1)
        myprintf("[LA %02x]\n", 0xff & ch);
#endif
    return(0);
}                                                 // Listen Primary Address group


/// @brief  Process all GPIB Talk commands
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

    if(ch == UNT)                                 // Universal Untalk
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[UNT]\n");
#endif
        return(0);
    }

    if(ch == SS80_MTA)                            // SS80
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[TA %02x SS80]\n", 0xff & ch);
#endif

        if (spoll)
        {
            SS80_Report();
        }
        return(0);
    }

#ifdef AMIGO
    if(ch == AMIGO_MTA)                           // AMIGO
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[TA %02x AMIGO]\n", 0xff & ch);
#endif
        return(0);
    }
#endif

    if(ch == PRINTER_MTA)                         // PRINTER
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[TA %02x PRINTER]\n", 0xff & ch);
#endif
        return(0);
    }

    if(listening == PRINTER_MLA)
    {
        printer_open(NULL);
        return(0);
    }

#if SDEBUG > 1
    if(debuglevel > 1)
        myprintf("[TA %02x]\n", 0xff & ch);
#endif
    return(0);
}                                                 // Talk Address primary address group


/// @brief  Process all GPIB Secondary Addresses
///
/// @param[in] ch 8 bit secondary address.
///
/// @return  0

int GPIB_SECONDARY_ADDRESS(uint8_t ch)
{
///  NOTE: We now know that ch >= 0x60 && ch <= 0x7f
///  Previous tests ensure this fact is true and because CMD_MASK == 0x7f


///  note: any errors will reset lastcmd
///  Universal Talk mode
///  Treat this as a Secondary Address ?
///  SS80 Ident 4-31
///  If we have our secondary address then send IDENT
    if(ch == SS80_MSA )
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[SA %02x SS80]\n", 0xff & ch);
#endif
        DisablePPR(SS80_PPR);
        return( Send_Identify( SS80ID1, SS80ID2) );
    }

#ifdef AMIGO
    if(ch == AMIGO_MSA )
    {
/// @todo 
/// 	Two identify bytes should be repeated until untalked
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[SA %02x AMIGO]\n", 0xff & ch);
#endif
        DisablePPR(AMIGO_PPR);
        return( Send_Identify( AMIGOID1, AMIGOID2) );
    }
#endif                                        // AMIGO

#if SDEBUG > 1
    if(debuglevel > 1)
        myprintf("[SA %02x, listen:%02x, talk:%02x]\n",
            0xff & ch, 0xff & listening, 0xff & talking);
#endif
    return(0);
}                                                 // Secondary Address, talking == UNT


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
