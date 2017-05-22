/**
 @file gpib/ss80.c

 @brief SS80 disk emulator for HP85 disk emulator project for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

 @par Based on work by Anders Gustafsson.

 @par Copyright &copy; 2014 Anders Gustafsson All rights reserved..

*/



#include "user_config.h"

#include "defines.h"
#include "gpib_hal.h"
#include "gpib.h"
#include "gpib_task.h"
#include "amigo.h"
#include "ss80.h"

/// @verbatim
///  See LIF filesystem Reference
///  Reference from "Subset 80 for Fixed and Flexible Disk Drives"
///  Page: 3-17, Figure 3-8. Secondaries and Opcodes
/// 
///  Address  Secondary  Opcode   # of other   Comment
///  State	(without            Parameter
///            parity)            bytes
///  -------  ---------  ------   ----------   -------
///  L major    65H      2XH      None        Set Unit
///  L major    65H      40-47H   None        Set Volume
///  L major    65H      10H      6 bytes     Set Address
///  L major    65H      18H      4 bytes     Set Length
///  L major    65H      34H      None        No Op
///  L major    65H      39H      2 bytes     Set RFS (No Op)
///  L major    65H      3BH      1 byte      Set Release (No Op)
///  L major    65H      3EH      8 bytes     Set Status Mask
///  L major    65H      48H      1 byte      Set Return Addressing Mode
///                                           (Only single vector allowed)
///  L major    65H      00H      None        Locate and Read
///  L major    65H      02H      None        Locate and Write
///  L major    65H      04H      None        Locate and Verif y
///  L major    65H      06H      1 byte      Spare Block
///  L major    65H      00H	    None        Request Status
///  L major    65H      0EH      None        Release (No Op
///  L major    65H      0FH      None        Release Denied (No Op)
///  L major    65H      31H      2 bytes     Validate Key
///  L major    65H      31H      2 bytes     Set Format Options
///  L major    65H      31H      6 bytes     Download
///  L major    65H      33H      3 bytes     Initiate Diagnostic
///  L major    65H      35H      None        Describe
///  L major    65H      37H      2 bytes     Initialize Media
///  L major    65H      4CH      None        Door Unlock
///  L major    65H      4DH      None        Door Lock
///  L major    72H      01H      1 byte      HP-IB Parity Checking
///  L major    72H      02H      4 bytes     Read Loopback
///  L major    72H      03H      4 bytes     Write Loopback
///  L major    72H      08H      None        Channel Independent Clear
///  L major    72H      09H      None        Cancel
///  L major    72H      --        --         Transparent execution (listen)
///                                           Write Loopback execution
///  L major    6EH      --        --         Normal execution (listen)
///                                           Write execution phase
///                                           Download execution phase
///  L major    70H      --       1 byte      Amigo Clear (1 parameter byte + SOC)
///  T major    70H      --       1 byte      Reporting phase (Stand alone QSTAT)
///  T major    6EH      --        --         Normal execution (talk)
///                                           Read execution phase
///                                           Describe execution phase
///                                           Request Status execution
///  T major    72H      --        --         Transparen texecution (talk)
///                                           Read Loopback execution
///  T minor    ADDRS    --        --         Identify
///  DEC        --       --        --         Universal Device Clear
/// 
///  Figure 3- 8. Secondaries and Opcodes
/// @endverbatim

///@brief GPIB BUS states
extern uint8_t talking;
extern uint8_t listening;
extern uint8_t spoll;

/// @brief Execute states
/// @enum
enum ss80exec 
{
    EXEC_IDLE,
    EXEC_LOCATE_AND_READ,
    EXEC_LOCATE_AND_WRITE,
    EXEC_SEND_STATUS,
    EXEC_DESCRIBE
};

/// @brief SS80 GPIB test string
/// @todo get this working
int TD[] =
{
    0x3F,
    0x5f | ATN_FLAG,
    0x20 | ATN_FLAG,
    0x65 | ATN_FLAG,
    0x20,
    0x40,
    0x34,                                         // NOP
    0x37,                                         // Format
    0x00,
    0x0a,
    -1
};

/// @brief SS80 GPIB function
/// @todo get this working
void SS80_Test(void)
{
#if SDEBUG >= 1
    if(debuglevel > 1)
        printf("[SS80 Test]\n");
#endif
#if SDEBUG >= 1
    if(debuglevel > 1)
    {
        printf("[SS80 Test Done]\n");
        printf("==============================\n");
    }
#endif
}


/// @brief Initialize SS80 state and bus
///
/// @verbatim
///  Reference:
///  CS80 Page B-12
///  Table B-5. Complementary Command Power-on Values
///  COMPLEMENTARY POWER-ON       COMPLEMENTARY               POWER-ON
///  COMMAND       VALUE COMMAND  COMMAND                     VALUE
///  Set Unit     0               Set Retry Time              device specific
///  Set Volume   0               Set Status Mask             disabled
///  Set Address  0,0,0           No Op                       Not Applicable
///  Set Length  -1 (full volume) Set Release                 T = O Z = O
///  Set Burst   disable          Set Options                 device specific
///  Set RPS     disabled         Set Return Addressing Mode  single vector
/// @endverbatim

/// @return void


///@brief default PRINTER address
uint8_t printer_addr = PRINTER_DEFAULT_ADDRESS;

///@brief SS80 Describe helper functions
///The SS80 docs number describe bytes 1 to N for Controller,Unit and Volume
/// MSB ... LSB
void SS80_V2B(uint8_t *B, int index,int size, uint32_t val)
{
	///@brief remove 1 bias
	V2B(B, index-1,size, val);
}

uint8_t *SS80ControllerPack(int *size)
{
	static uint8_t B[5];
	*size = 5;
    /*
        uint8_t C1;  //<  MSB units installed bit field
                     //< one bit per unit, Unit 15 always set
        uint8_t C2;  //<  LSB
        uint8_t C3;  //<  MSB Instantaneous transfer rate in kb/s
        uint8_t C4;  //<  LSB
        uint8_t C5;  //<  Controller type
                 //< 0 = CS/80 integrated single unit controller.
                 //< 1 = CS/8O integrated multi-unit controller.
                 //< 2 = CS/8O integrated multi-port controller.
                 //< 4 = SS/8O integrated single unit controller.
                 //< 5 = SS/80 integrated multi-unit controller.
                 //< 6 = SS/80 integrated multi-port controller.
    */
	SS80_V2B(B,1,2,SS80Disk.CONTROLLER.UNITS_INSTALLED);
	SS80_V2B(B,2,2,SS80Disk.CONTROLLER.TRANSFER_RATE);
	SS80_V2B(B,5,1,SS80Disk.CONTROLLER.TYPE);

	return(B);
}

uint8_t *SS80UnitPack(int *size)
{
	static uint8_t B[19];
	*size = 19;
    /*
        uint8_t U1;  //<  Type 0-Fixed, 1-Flexible, 2-Tape
                     //< (+128-dumb, does not detect media change)
        uint8_t U2;  //<  MSB Device number
        uint8_t U3;  //<  (HP 9133 = 09 13 30)
        uint8_t U4;  //<     LSB
        uint8_t U5;  //<  MSB Bytes per block
        uint8_t U6;  //<     LSB
        uint8_t U7;  //<  Number of buffered blocks
        uint8_t U8;  //<  Burst size (0 for SS/80)
        uint8_t U9;  //<  MSB Block time in ms
        uint8_t U10; //<     LSB
        uint8_t U11; //<  MSB Continous average transfer rate
        uint8_t U12; //<     LSB
        uint8_t U13; //<  MSB Optimal retry in tens of ms
        uint8_t U14; //<     LSB
        uint8_t U15; //<  MSB Access time in tens of ms
        uint8_t U16; //<     LSB
        uint8_t U17; //<  Maximum interleave or 0
        uint8_t U18; //<  Fixed volume byte, one bit per volume,
                     //< ie 00000111 = 3 volumes
        uint8_t U19; //<  Removable volume byte, one bit per volume,
                     //< ie 00000111 = 3 volumes
    */
	SS80_V2B(B,1,1,SS80Disk.UNIT.UNIT_TYPE);
	SS80_V2B(B,2,3,SS80Disk.UNIT.DEVICE_NUMBER);
	SS80_V2B(B,5,2,SS80Disk.UNIT.BYTES_PER_BLOCK);
	SS80_V2B(B,7,1,SS80Disk.UNIT.BUFFERED_BLOCKS);
	SS80_V2B(B,8,1,SS80Disk.UNIT.BURST_SIZE);
	SS80_V2B(B,9,2,SS80Disk.UNIT.BLOCK_TIME);
	SS80_V2B(B,11,2,SS80Disk.UNIT.CONTINOUS_TRANSFER_RATE);
	SS80_V2B(B,13,2,SS80Disk.UNIT.OPTIMAL_RETRY_TIME);
	SS80_V2B(B,15,2,SS80Disk.UNIT.ACCESS_TIME);
	SS80_V2B(B,17,1,SS80Disk.UNIT.MAXIMUM_INTERLEAVE);
	SS80_V2B(B,18,1,SS80Disk.UNIT.FIXED_VOLUMES);
	SS80_V2B(B,19,1,SS80Disk.UNIT.REMOVABLE_VOLUMES);
	return(B);
}

uint8_t *SS80VolumePack(int *size)
{
	static uint8_t B[13];
	*size = 13;
    /*
        uint8_t V1;  //<  MSB Max cylinder
        uint8_t V2;  //<
        uint8_t V3;  //<     LSB
        uint8_t V4;  //<  Maximum head 0 based
        uint8_t V5;  //<  MSB Maximum sector 0 based
        uint8_t V6;  //<     LSB
        uint8_t V7;  //<  MSB Max number of blocks
        uint8_t V8;
        uint8_t V9;
        uint8_t V10;
        uint8_t V11;
        uint8_t V12; //<     LSB
        uint8_t V13; //<  Interleave
    */
	SS80_V2B(B,1,3,SS80Disk.VOLUME.MAX_CYLINDER);
	SS80_V2B(B,4,1,SS80Disk.VOLUME.MAX_HEAD);
	SS80_V2B(B,5,2,SS80Disk.VOLUME.MAX_SECTOR);
	SS80_V2B(B,7,6,SS80Disk.VOLUME.MAX_BLOCK_NUMBER);
	SS80_V2B(B,13,1,SS80Disk.VOLUME.INTERLEAVE);
	return(B);
}

void SS80_init(void)
{
    Clear_Common(15);

    gpib_bus_init(1);

#if SDEBUG >= 1
    if(debuglevel >= 1)
        printf("[SS80 INIT]\n");
#endif

    SS80State.estate = EXEC_IDLE;

    SS80State.Errors = 0;

    SS80State.qstat = 2;

/// @todo FIXME
    DisablePPR(SS80_PPR);
}


/// @brief  SS80 Execute state process
///
/// - Reference SS80 3-1..9
/// - State: EXEC STATE COMMAND
/// @return 0 on sucess
/// @return GPIB error flags on fail
/// @see gpib.h ERROR_MASK defines for a full list of error flags.
int SS80_Execute_State(void)
{
    int ret = 0;

    DisablePPR(SS80_PPR);
    switch(SS80State.estate)
    {
        case EXEC_IDLE:
            break;
        case EXEC_LOCATE_AND_READ:
            ret = SS80_locate_and_read();
            SS80State.estate = EXEC_IDLE;
            break;
        case EXEC_LOCATE_AND_WRITE:
            ret = SS80_locate_and_write();
            SS80State.estate = EXEC_IDLE;
            break;
        case EXEC_SEND_STATUS:
            ret = SS80_send_status();
            SS80State.estate = EXEC_IDLE;
            break;
        case EXEC_DESCRIBE:
            ret = SS80_describe();
            SS80State.estate = EXEC_IDLE;
            break;
        default:
#if SDEBUG >= 1
            if(debuglevel >= 1)
                printf("[SS80 EXEC state:%d error]\n", SS80State.estate);
#endif
            SS80State.estate = EXEC_IDLE;
            break;
    }
    EnablePPR(SS80_PPR);
    return(ret);
}


/// @brief  SS80 Locate and Read COmmend
///
/// - Reference: SS80 4-39
/// - Read (SS80State.Length) disk bytes at (Address) offset.
/// - Send this data to the GPIB bus.
/// - State: EXEC STATE COMMAND.
/// @return 0 on sucess
/// @return GPIB error flags on fail.
/// @see gpib.h ERROR_MASK defines for a full list of error flags.
/// - Notes: Any Disk I/O errors will set qstat and Errors.
/// - Limitations:
///  - Currenly we will only process 2 ** 31 bytes Max in one transfer.
///  - If an seek or I/O error happens then we MUST continue to
/// read and discard the GPIB data until we get an EOI or GPIB error...

int SS80_locate_and_read( void )
{
    DWORD total_bytes;
    DWORD count;
    int chunk;
    int len;
    uint16_t status;
	uint32_t Address = (SS80State.AddressBlocks * SS80Disk.UNIT.BYTES_PER_BLOCK);


    SS80State.qstat = 0;

    status = 0;

    if( GPIB_IO_RD(IFC) == 0)
        return(IFC_FLAG);

/// @todo FIXME
///  SS80 4-39
///  For now we will assume the controller will never do this

#if SDEBUG > 1
    if(debuglevel > 1)
        printf("[SS80 Locate and Read @%08lx(%lx)]\n", Address,SS80State.Length);
#endif

    if( SS80_cmd_seek() )
    {
        return(SS80_error_return());
    }

    count = SS80State.Length;
    total_bytes = 0;
    while(count > 0 )
    {
        if( GPIB_IO_RD(IFC) == 0)
        {
            return(IFC_FLAG);
        }

        if(count > 256)
        {
            chunk = 256;
            status = 0;                           // GPIB status
        }
        else
        {
            chunk = count;
            status |= EOI_FLAG;                   // GPIB EOI on final charater
        }

#if SDEBUG > 1
        if(debuglevel > 2)
            gpib_timer_elapsed_begin();
#endif

        len = dbf_open_read(SS80Disk.HEADER.NAME, Address, gpib_iobuff, chunk, &SS80State.Errors);

#if SDEBUG > 1
        if(debuglevel > 2)
            gpib_timer_elapsed_end("Disk Read");
        if(debuglevel >= 1)
            printf("[SS80 Disk Read %d bytes]\n", len);
#endif
        if(len < 0)
        {
            SS80State.qstat = 1;
/// @return Return
#if SDEBUG >= 1
            if(debuglevel >= 1)
                printf("[SS80 Disk Read Error]\n");
#endif
            return( SS80_error_return() );
        }

#if SDEBUG > 1
        if(debuglevel > 2)
            gpib_timer_elapsed_begin();
#endif
        len = gpib_write_str(gpib_iobuff, chunk, &status);
#if SDEBUG > 1
        if(debuglevel > 2)
            gpib_timer_elapsed_end("GPIB Write");
#endif
        if( len != chunk)
        {
            SS80State.qstat = 1;
#if SDEBUG >= 1
            if(debuglevel >= 1)
                printf("[SS80 GPIB Write Wrror]\n");
#endif
            if(status & ERROR_MASK)
            {
                SS80State.Errors |= ERR_GPIB;
                break;
            }
        }

        Address += chunk;
        total_bytes = total_bytes + len;
        count -= len;
    }
///  Note: this should not happen unless we exit on errors above
    if(count > 0)
    {
#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[SS80 Buffered Read DID NOT FINISH]\n");
#endif
    }
    else
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[SS80 Buffered Read Total(%ld) Bytes]\n", total_bytes);
#endif
    }

	SS80State.AddressBlocks = (Address/SS80Disk.UNIT.BYTES_PER_BLOCK);
    return (status & ERROR_MASK);
}


/// @brief  SS80 Locate and Write Commend.
///
/// - Reference: SS80 pg 4-45, CS80 pg 2-8.
/// - Write (SS80State.Length) disk bytes from GPIB at (Address) offset.
/// - State: EXEC STATE COMMAND.
/// - Disk I/O errors will set qstat and Errors.
/// - Limitations.
///  - Currenly we will only process 2 ** 31 bytes Max in one transfer.
///  - If an seek or I/O error happens then we MUST continue to.
/// read and discard the GPIB data until we get an EOI or GPIB error...
/// @return 0 on sucess.
/// @return GPIB error flags on fail.
/// @see gpib.h ERROR_MASK defines for a full list.

int SS80_locate_and_write(void)
{
    DWORD total_bytes;                            // Keeps track of data actually written to disk
    int chunk, count, len;
    int io_skip;
    uint16_t status;
	uint32_t Address = (SS80State.AddressBlocks * SS80Disk.UNIT.BYTES_PER_BLOCK);

    io_skip = 0;

#if SDEBUG > 1
    if(debuglevel > 1)
        printf("[SS80 Locate and Write @%08lx(%lx)]\n", Address, SS80State.Length);
#endif

    SS80State.qstat = 0;

    if( GPIB_IO_RD(IFC) == 0)
        return(IFC_FLAG);

    if( SS80_cmd_seek() )
    {
        SS80State.Errors |= ERR_WRITE;
        io_skip = 1;
    }

    count = SS80State.Length;
    total_bytes = 0;

    status = 0;

    while(count > 0)                              // Loop until we have received Length sectors or EOI
    {
        if( GPIB_IO_RD(IFC) == 0)
        {
            return(IFC_FLAG);
        }

        if(count > 256)
            chunk = 256;
        else
            chunk = count;

        Mem_Clear(gpib_iobuff);

#if SDEBUG > 1
        if(debuglevel > 2)
            gpib_timer_elapsed_begin();
#endif
        len = gpib_read_str(gpib_iobuff, (UINT) chunk, &status);

#if SDEBUG > 1
        if(debuglevel > 2)
            gpib_timer_elapsed_end("GPIB Read");
#endif

        if( len != chunk)
        {
            if(status & ERROR_MASK)
            {
#if SDEBUG >= 1
                if(debuglevel >= 1)
                    printf("[GPIB Read Error]\n");
#endif
                SS80State.Errors |= ERR_WRITE;
                SS80State.qstat = 1;
                break;
            }
            if(!len && (EOI_FLAG & status))
                break;
        }


        if(!io_skip)
        {
            if(len)
            {
                int len2;
#if SDEBUG > 1
                if(debuglevel > 2)
                    gpib_timer_elapsed_begin();
#endif
                len2 = dbf_open_write(SS80Disk.HEADER.NAME, Address, gpib_iobuff, len, &SS80State.Errors);
#if SDEBUG > 1
                if(debuglevel > 2)
                    gpib_timer_elapsed_end("Disk Write");
#endif
                if(len2 != len)
                {
                    SS80State.Errors |= ERR_WRITE;
                    if(mmc_wp_status())
                        SS80State.Errors |= ERR_WP;
                    SS80State.qstat = 1;
                    io_skip = 1;                  // Stop writing
#if SDEBUG >= 1
                    if(debuglevel >= 1)
                        printf("[Disk Write Error]\n");
#endif
                }
                else
                {
#if SDEBUG > 1
                    if(debuglevel > 1)
                        printf("[SS80 Locate and Write wrote(%d)]\n", len2);
#endif
                    Address += len;
                }

                if(len2 >= len)
                    total_bytes += len2;
                else
                    total_bytes += len;
            }                                     // len
        }                                         // io_skip

        count -= len;

        if(status & EOI_FLAG)
            break;
    }

    if(count > 0)
    {
#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[SS80 Locate and Write DID NOT FINISH]\n");
#endif
    }
    else
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[SS80 Locate and Write Wrote Total(%ld)]\n", total_bytes);
#endif
    }
	SS80State.AddressBlocks = (Address/SS80Disk.UNIT.BYTES_PER_BLOCK);
    return ( status & ERROR_MASK );
}


/// @brief  SS80 send detailed status.
/// 
/// - Reference: SS80 pg 4-59..64.
/// - Send 20 bytes of extended status to host>
/// - State: EXEC STATE COMMAND.
///
/// @verbatim
///  SS80 pg 4-59
///  Bits are numbered
///    0 .. 7 (MSB..LSB "backwards")
///  Bytes are numbered
///    1 .. 20 (0..19 in message)
///  Example:
///    Bit 0 represents the MSB of Byte 1
/// 
///  We define ONLY those bits we are using, or might use
///  See: SS80 pg 4-59 for more details
///
///  Indentification Field
///    Byte 1(0)  <VVVVUUUU>
///      VVVV=Volume NUmber
///    Byte 2(1)  <11111111>
///
///  SS80 pg 4-59
///  Reject Errors Field
///       byte 3(2)           byte 4(3)
///  < 0 0 2 0 0 5 6 7 > <8 9 10 0 12 0 0 O>
///  2 = Channel Parity Error
///  5 = Illegal Opcode
///  6 = Module Addressing
///  7 = Address Bounds
///  8 = Parameter Bounds
///  9 = Illegal Parameter
///  10 = Message Sequence
///  12 = Message Length
/// 
///  SS80 pg 4-60
///  Fault Errors Field
///        byte 5(4)               byte 6(5)
///  < 0 17 0 19 0 0 22 0 > < 24 0 0 0 0 0 30 31 >
/// 
///  19 = Controller Fault
///  22 = Unit Fault
///  24 = Diagnostic Result
///  30 = Power Fail
///  31 = Re-transmit
/// 
///
///  SS80 pg 4-60
///  Access Errors Field
///        byte 7(6)                byte 8(7)
///  < 0 33 34 35 36 37 0 0 > < 40 41 0 43 44 0 0 0 >
///  33 = Uninitialized media
///  34 = No Spares Available
///  35 = Not Ready
///  36 = Write Protect
///  37 = No Data Found
///  40 = Unrecoverable Data Overflow
///  41 = Unrecoverable Data
///  43 = End of File
///  44 = End of Volume
/// 
///  SS80 pg 4-62
///  Information Errors Field
///         byte 9(8)             byte 10(9)
///  < 0 0 0 51 52 0 0 55 > < 0 57 0 59 0 0 0 0 >
///  51 = Media Wear
///  52 = Latency induced
///  55 = Auto Sparing Invoked
///  57 = Recoverable Data Overflow
///  59 = Recoverable Data
/// 
///
/// SS80 pg 4-64
/// P6 (10 .. 15) is SS80 Specific
///
/// SS80 4:65
/// P1O (16 .. 19)
///
/// @todo  add some of these
///  5 = Illegal Opcode
///  6 = Module Addressing
///  8 = Parameter Bounds
///  9 = Illegal Parameter
///  10 = Message Sequence
///  12 = Message Length
///  19 = Controller Fault
///  24 = Diagnostic Result
///  30 = Power Fail
///  33 = Uninitialized media
///  35 = Not Ready
///  37 = No Data Found
///  41 = Unrecoverable Data
///  43 = End of File
///  44 = End of Volume
/// @endverbatim
///
/// @return 0.
/// @return GPIB error flags on fail>
/// @see gpib.h ERROR_MASK defines for a full list.

int SS80_send_status( void )
{
    uint8_t tmp[20];
    uint16_t status;

#if SDEBUG > 1
    if(debuglevel > 1)
        printf("[SS80 Send Status]\n");
#endif

    Mem_Clear(tmp);

    tmp[0] = ((SS80State.volNO& 0x0f) << 4) | (SS80State.unitNO & 0x0f);

    tmp[1] = 0xff;


    if(SS80State.Errors & ERR_SEEK)
        tmp[3] = 0b00000001;                      // address bounds = Byte 3


    if(SS80State.Errors & ERR_READ)
        tmp[4] = 0b00000010;                      // unit fault = Byte 5

    if(SS80State.Errors & ERR_WRITE)
        tmp[4] = 0b00000010;                      // unit fault = Byte 5

/// @todo  add Diagnostic Result status (MSB of byte 5)


    if(SS80State.Errors & ERR_WP)
        tmp[6] = 0b00001000;                      // Write protect = Byte 7


/// 4:63,64

///  Note: Diagnostic Result bit is NOT set
///@see SET ADDRESS in blocks
	if(!SS80State.Errors)
	{
/*
		tmp[10] = SS80State.AddressBlocks.B[5];	// MSB unused
		tmp[11] = SS80State.AddressBlocks.B[4];	// unused
		tmp[12] = SS80State.AddressBlocks.B[3];
		tmp[13] = SS80State.AddressBlocks.B[2];
		tmp[14] = SS80State.AddressBlocks.B[1];
		tmp[15] = SS80State.AddressBlocks.B[0];	// LSB
*/
		V2B(tmp,10,6,SS80State.AddressBlocks);
	}

/// @todo Fixme
    if(SS80State.Errors)
        SS80State.qstat = 1;

    status = EOI_FLAG;
    if(gpib_write_str(tmp, sizeof(tmp), &status) != sizeof(tmp))
    {
#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[SS80 Send Status FAILED]\n");
#endif
    }

    return ( status & ERROR_MASK );
}


/// @brief  Send Controler,Unit and Volume description.
///
/// - Reference: SS80 4-15.
/// - State: EXEC STATE COMMAND>
///
/// @return 0 on sucess>
/// @return flags on fail.
/// @ gpib.h ERROR_MASK defines for a full list of error flags.

int SS80_describe( void )
{
    uint16_t status;

	uint8_t *B;
	int size;

		

#if SDEBUG > 1
    if(debuglevel > 1)
        printf("[SS80 Describe]\n");
#endif

    status = 0;

	B = SS80ControllerPack(&size);
    if(gpib_write_str(B,size, &status) != size)
    {
#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[SS80 Describe Controller FAILED]\n");
#endif
        return(status & ERROR_MASK);
    }

    status = 0;

	B = SS80UnitPack(&size);
    if(gpib_write_str(B,size, &status) != size)
    {
#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[SS80 Describe Unit FAILED]\n");
#endif
        return(status & ERROR_MASK);
    }

    status = EOI_FLAG;
	B = SS80VolumePack(&size);
    if(gpib_write_str(B,size,&status) != size)
    {
#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[SS80 Describe Volume FAILED]\n");
#endif
        return(status & ERROR_MASK);
    }

    return(0);
}




/// @brief  Process OP Codes following 0x65 Command State.
///
/// - References: CS80 pg 4-6.
/// - State: COMMAND STATE.
/// @return  0 on sucess.
/// @return flags on fail.
/// @see gpib.h ERROR_MASK defines for a full list.
/// 
/// @verbatim
///  Notes:
///  We set DPPR on entry and EPPR on exit
///  	DPPR = DisablePPR(SS80_PPR);
///  	EPPR = EnablePPR(SS80_PPR);
/// 
///  Valid OP Codes for Command State (0x65):
///     Real Time, General Purpose, Complementary, DIagnostic
///     Diagnostic
///     In this Order:
///        (1 to N) Complementary
///        and/or
///        (1) of Real Time, General Purpose, Diagnostic
///        (This later group is always LAST)
///     OP Code Sequence errors: TODO
/// 
///  We Read all of the Data/Opcodes/Parameters at once
///     (while ATN is false).
///     Last byte read should be EOI or an error
///  Why read all at once ?
///     SS80 "Command Messages" are buffered one at a time.
///     A "Command Message" contains all ALL opcodes & their parameters
///     (See SS80 Section 4-12, Page 4-6)
/// 
///  Unknown OP Code processing rules
///     Skip the remaining codes, Wait for Report Phase
/// @endverbatim

int SS80_Command_State( void )
{
/// @todo FIXME

    int ch;                                       // Current OP Code
    uint16_t status;                              // Current status
    int len;                                      // Size of Data/Op Codes/Parameters read in bytes
    int ind;                                      // Buffer index

    DisablePPR(SS80_PPR);


    status = EOI_FLAG;
    len = gpib_read_str(gpib_iobuff, GPIB_IOBUFF_LEN, &status);
    if(status & ERROR_MASK)
    {
/// @todo FIXME
#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[GPIB Read ERROR]\n");
#endif
        return(status & ERROR_MASK);
    }

    if(!len)
        return(0);

    if( !(status & EOI_FLAG) )
    {
#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[GPIB buffer OVERFLOW!]\n");
#endif
    }

    ind = 0;
    while(ind < len)
    {
        ch = gpib_iobuff[ind++];

#if SDEBUG > 1
///  Note - status is omitted so this is just a byte dump
        if(debuglevel >= 9)
            gpib_decode(ch);
#endif

        if(ch == 0x00)
        {
/// @todo FIXME,
///  In Execute state calls  SS80_locate_and_read();
            SS80State.estate = EXEC_LOCATE_AND_READ;
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SS80 Locate and Read]\n");
#endif
            break;
        }

        if(ch == 0x02)
        {
/// @todo FIXME,
///  SS80_locate_and_write();
            SS80State.estate = EXEC_LOCATE_AND_WRITE;
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SS80 Locate and Write]\n");
#endif
            break;
        }

/// @todo FIXME
///  Important SS80 and CS80 differences regarding Complementary Commands!
///  CS80 pg 2-1
///  1) In CS80 when only complementary commands appear in a message 
///     they will set the system defaults:
///     Unit, number, data transfer length, burst size, maximum retry time, 
///     and Rotational Position Sensing (RPS) window size and location.
///  2) If, in the same message, they proceed a Real Time, General Purpose
///     or Diagnostic they are TEMPORARY and just for that single transaction!
///  3) The exeption to these rules are Set Unit, Set Volume

/// @todo  Only handles 4-byte Addresses at the moment
///  CS80 pg 4-11, 2-14
///  SS80 pg 4-67
		// Set address
        if (ch == 0x10)
        {
			/* upper two MSB unused */
			/*
			SS80State.AddressBlocks.B[5] = gpib_iobuff[ind+0];// MSB unused
			SS80State.AddressBlocks.B[4] = gpib_iobuff[ind+1];// unused
			SS80State.AddressBlocks.B[3] = gpib_iobuff[ind+2];
			SS80State.AddressBlocks.B[2] = gpib_iobuff[ind+3];
			SS80State.AddressBlocks.B[1] = gpib_iobuff[ind+4];
			SS80State.AddressBlocks.B[0] = gpib_iobuff[ind+5];//LSB
			*/
			SS80State.AddressBlocks = B2V(gpib_iobuff,ind,6);
            ind += 6;
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SS80 Set Address:(%08lx)]\n", SS80State.AddressBlocks);
#endif
            continue;
        }

///  SS80 pg 4-73
		// Set Length
        if(ch == 0x18)
        {
			/*
            SS80State.Length.B[3] = gpib_iobuff[ind+0];// MSB
            SS80State.Length.B[2] = gpib_iobuff[ind+1];
            SS80State.Length.B[1] = gpib_iobuff[ind+2];
            SS80State.Length.B[0] = gpib_iobuff[ind+3];//LSB
			*/
            SS80State.Length = B2V(gpib_iobuff,ind,4);
            ind += 4;
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SS80 Set Length:(%08lx)]\n", SS80State.Length);
#endif
            continue;
        }

		///@brief set unit number
        if(ch >= 0x20 && ch <= 0x2f)
        {
            SS80State.unitNO = ch - 0x20;
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SS80 Set Unit:(%d)]\n", SS80State.unitNO);
#endif
            continue;
        }

// SS80 $S80 4-47
		// NO OP
        if(ch == 0x34)
        {
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SS80 NO-OP]\n");
#endif
            continue;
        }
// SS80 $S80 4-79
		// Set RPS
        if (ch == 0x39)
        {
            ind += 2;
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SS80 Set RPS NO-OP]\n");
#endif
            continue;
        }

// SS80 $S80 4-75
		// Set Release
        if (ch == 0x3B)
        {
            ind++;
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SS80 Set Release NO-OP]\n");
#endif
            continue;
        }

        if(ch >= 0x40 && ch <= 0x4f)
        {
            SS80State.volNO = ch - 0x40;
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SS80 Set Volume: (%d)]\n", SS80State.volNO);
#endif
            continue;
        }

/// @brief  Set Return Addressing
///  Type: COMPLEMENTARY
///  Phases: Command, Report
///  SS80 pg 4-81
///  CS80 4-18,2-24

        if(ch == 0x48)
        {
/// @todo TODO
///  Skip Parameters
            ind++;
#if SDEBUG >= 1
            if(debuglevel >= 1)
                printf("[SS80 Set Return Addressing - TODO]\n");
#endif
            continue;
        }

/// @todo  not implmented yet
///  SS80 pg 4-43
///  CS80 pg 4-20, 2-28

        if(ch == 0x04)
        {
/// @todo TODO
/// @todo FIXME
///  Execute NOW
#if SDEBUG >= 1
            if(debuglevel >= 1)
                printf("[SS80 Locate and Verify - TODO]\n");
#endif
            break;
        }

        if(ch == 0x0E)
        {
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SS80 Release NO-OP]\n");
#endif
/// @todo FIXME
///  The class GENERAL PURPOSE suggests yes
            break;
        }
        if(ch == 0x0F)
        {
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SS80 Release Denied NO-OP]\n");
#endif
/// @todo FIXME
///  The class GENERAL PURPOSE suggests yes
            break;
        }

/// @brief  Validate Key, Set Format Options, Download
///  Type: GENERAL PURPOSE
///  Phases: Command, Report
///  Must be last command in sequence
///  SS80 pg 4-91
///  CS80 N/A

        if(ch == 0x31)
        {
/// @todo TODO
///  Skip Parameters
            ind += 2;
#if SDEBUG >= 1
            if(debuglevel >= 1)
                printf("[SS80 Validate Key - TODO]\n");
#endif
            break;
        }

        if(ch == 0x35)
        {
            SS80State.estate = EXEC_DESCRIBE;
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SS80 Describe]\n");
#endif
            break;
        }

/// @brief  Initialize Media
///  Type: GENERAL PURPOSE
///  Phases: Command, Report
///  This command must be last and only one
///  SS80 pg 4-33
///  CS80 4-19,2-26

        if(ch == 0x37)
        {
            ind += 2;
/// @todo TODO
/// @todo FIXME
#if SDEBUG >= 1
            if(debuglevel >= 1)
                printf("[SS80 Initialize Media TODO]\n");
#endif
            break;
        }

/// @brief  Set Status Mask
///  Type: GENERAL PURPOSE
///  Phases: Command, Report
///  Must be last command in sequence
///  SS80 pg 4-81
///  CS80 4-15,2-20

        if(ch == 0x3E)
        {
/// @todo TODO
            ind += 8;
#if SDEBUG >= 1
            if(debuglevel >= 1)
                printf("[SS80 Set Status Mask - TODO]\n");
#endif
            break;
        }

        if (ch == 0x4C)
        {
/// @todo TODO
#if SDEBUG >= 1
            if(debuglevel >= 1)
                printf("[SS80 Door UnLock - TODO]\n");
#endif
            break;
        }

        if (ch == 0x4D)
        {
/// @todo TODO
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SS80 Door Lock - TODO]\n");
#endif
            break;
        }

        if(ch == 0x0D)
        {
            SS80State.estate = EXEC_SEND_STATUS;
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SS80 Request Status]\n");
#endif
            break;
        }

/// @brief  Initiate Diagnostic
///  Type: DIAGNOSTIC
///  Phases: Command, Report
///  Must be last command in sequence
///  SS80 pg 4-35
///  CS80 N/A

        if(ch == 0x33)
        {
/// @todo TODO
            ind += 3;
#if SDEBUG >= 1
            if(debuglevel >= 1)
                printf("[SS80 Iniate Diagnostic - TODO]\n");
#endif
            break;
        }

#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[SS80 Invalid OP Code (%02x)]\n", ch & 0xff);
#endif

        break;
    }

    if( ind != len)
    {
#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[SS80 Execute Command, opcode processing error (%d) of (%d) OP Codes]\n", ind, len);
#endif
    }

    EnablePPR(SS80_PPR);

    return(status & ERROR_MASK);
}


/// @brief  (0x70 or 0x72) OP Code processing.
///
/// - References: CS80 pg 4-26, 3-2,3-3,3-4,3-5,3-7,3-9,3-10.
/// - State: Transparent State.
/// @return  0 on sucess
/// @return flags on fail
/// @see gpib.h ERROR_MASK defines for a full list or error flags.
/// 
/// @verbatim
///  Notes:
///  	DPPR DisablePPR(SS80_PPR) should already be set at the secondary
///   We do NOT reenable EPPR at any state
/// 
///  Valid OP Codes for Transparent State (0x70 or 0x72):
///     [Unit Complementary] Transparent
/// 
///  Transparent Commands done at HP-IB state, NOT here:
///    Universal Device Clear
///    Selected Device Clear
///    Identify
/// 
///  Read all of the Data/Opcodes/Parameters at once
///     (while ATN is false).
///     Last byte read should be EOI or an error
///  Why read all at once ?
///     SS80 "Command Messages" are buffered one at a time.
///     A "Command Message" contains all ALL opcodes & their parameters
///     (See SS80 Section 4-12, Page 4-6)
/// 
///  Unknown OP Code processing rules
///     Skip the remaining codes, Wait for Report Phase
/// @endverbatim

int SS80_Transparent_State( void )
{
/// @todo FIXME

    int ch;                                       // Current OP Code
    uint16_t status;                              // Current status
    int len;                                      // Size of Data/Op Codes/Parameters read in bytes
    int ind;                                      // Buffer index
	///@brief  work in progress, unit support
    int cunit = 0;                                // Unit Complementary , optional


    DisablePPR(SS80_PPR);

///  Note: Enabling Parallel Poll is up to each function
///  Many transparent Commands require Parallel Poll disabled when done

    status = EOI_FLAG;
    len = gpib_read_str(gpib_iobuff, GPIB_IOBUFF_LEN, &status);
    if(status & ERROR_MASK)
    {
/// @todo FIXME
#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[GPIB Read ERROR]\n");
#endif
        return(status & ERROR_MASK);
    }

    if(!len)
        return(0);

    if( !(status & EOI_FLAG) )
    {
#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[GPIB buffer OVERFLOW!]\n");
#endif
    }

    ind = 0;

    while(ind < len)
    {
        ch = gpib_iobuff[ind++];

#if SDEBUG > 1
///  Note - status is omitted so this is just a byte dump
        if(debuglevel >= 9)
            gpib_decode(ch);
#endif

        if(ch == 0x01)
        {
/// @todo TODO
///  Skip Paramter
            ind++;
#if SDEBUG >= 1
            if(debuglevel >= 1)
                printf("[SS80 HP-IB Parity Checking - TODO]\n");
#endif
            EnablePPR(SS80_PPR);
            break;
        }
/// @todo FIXME
///  SS80 4-49
///  CS80 4-27, 3-7

        if(ch == 0x02)
        {
            ind += 4;
/// @todo TODO
///  DO NOT EPPR
#if SDEBUG >= 1
            if(debuglevel >= 1)
                printf("[SS80 Read Loopback - TODO]\n");
#endif
            break;
        }
/// @todo FIXME
///  SS80 4-49
///  CS80 4-27, 3-7

        if(ch == 0x03)
        {
            ind += 4;
/// @todo TODO
///  DO NOT EPPR
#if SDEBUG >= 1
            if(debuglevel >= 1)
                printf("[SS80 Write Loopback - TODO]\n");
#endif
            break;
        }

        if(ch >= 0x20 && ch <= 0x2f)
        {
            cunit = ch - 0x20;
            continue;
        }

/// @todo FIXME
///  SS80 4-11
///  CS80 4-26, 3-2,3-5

        if(ch == 0x08)                            // 0x08 OP Code
        {
#if SDEBUG >= 1
            if(debuglevel >= 1)
                printf("[SS80 Channel Independent Clear (%d)]\n", SS80State.unitNO);
#endif
            return(SS80_Channel_Independent_Clear( SS80State.unitNO ));
        }

/// @todo FIXME
///  SS80 4-9
///  CS80 4-26, 3-6

        if(ch == 0x09)                            // 0x09 OP Code
        {
#if SDEBUG > 1
            if(debuglevel > 1)
                printf("[SS80 Cancel (%d)]\n", SS80State.unitNO);
#endif
            return(SS80_Cancel( SS80State.unitNO ));

            break;
        }

#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[SS80 Invalid OP Code (%02x)]\n", ch & 0xff);
#endif
        break;
    }

    if( ind != len)
    {
#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[SS80 Transparent Command, processing ERROR(%d) of (%d) OP Codes]\n", ind, len);
#endif
    }


    return(status & ERROR_MASK);
}


/// @brief  Limit tests for disk seek
///
/// - eference SS80 4-39,4-45
/// - We check for read/write past end of disk during the seek
/// @see: SS80_locate_and_read( )
/// @see: SS80_locate_and_write()
/// @return  0 on sucess
/// @return fatfs error on fail
/// - Fail will set: qstat = 1, Errors |= ERR_SEEK.

int SS80_cmd_seek( void )
{

/// @todo  Let f_lseek do bounds checking instead ???
///  Will we read or write past the end of the disk ??
    if ( (SS80State.AddressBlocks + (SS80State.Length/(SS80Disk.UNIT.BYTES_PER_BLOCK)))
             > SS80Disk.VOLUME.MAX_BLOCK_NUMBER)
    {
        SS80State.qstat = 1;
        SS80State.Errors |= ERR_SEEK;

#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[SS80 Seek OVERFLOW]\n");
#endif                                    // if 0
        return(1);
    }

#if SDEBUG > 1
    if(debuglevel > 1)
        printf("[SS80 Seek:%08lx]\n", SS80State.AddressBlocks);
#endif
    return (0);
}


/// @brief send QSTAT
///
/// - Reference: CS80 pg 4-7, Section 4-14.
///
/// @verbatim
///    QSTAT, return status byte
///  NORMAL COMPLETION = 0
///     Indicates normal completion of the requested operation.
///  HARD ERROR = 1
///     Indicates that error information is available.
///     The host must issue the Request Status command in order
///     to determine what went wrong.
///  POWER ON = 2
///     Indicates that the device has just returned from a power failure
///     or some form of operator intervention (such as removal of the
///     storage media). Any incomplete transactions were aborted and
///     should be repeated.  The host must reconfigure any programmable
///     operating parameters because they have returned to their
///     power-on values.
/// @endverbatim
///
/// @return  0 on sucess
/// @see: gpib.h ERROR_MASK defines for a full list.

int SS80_Report( void )
{
    uint8_t tmp[1];
    uint16_t status;

    tmp[0] = SS80State.qstat;

    status = EOI_FLAG;
    if( gpib_write_str(tmp, sizeof(tmp), &status) != sizeof(tmp))
    {
#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[SS80 qstat send FAILED]\n");
#endif
        return(status & ERROR_MASK);
    }
    else
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[SS80 qstat %02X]\n", SS80State.qstat);
#endif
    }
    SS80State.qstat = 0;
    return ( 0 );
}


/// @brief Uiniversal and Decvice clear code
///
/// - Clear Settings to poweron values.
///  - Clear Unit, Address, Length, Error Status, qstat
/// @see: SS80_Selected_Device_Clear()
/// @see: SS80_Universal_Device_Clear()
/// @see: SS80_Amigo_Clear()
///
/// -References:
///   - SS80 ps 4-2 - *** listed as NOT supported for SS80 ***
///   - CS80 pg 4-49, 3-2, 3-4
///   - CS80 Page B-12
///   - SS80 4-11
///   - CS80 4-26, 3-2,3-5
/// - Notes: All Clears are the same 
/// - Except: Universal Device Clear to Unit other then 15.
///  - If the unit is NOT 15 then then Unit will remain set
///  - They ALL set Power on Values - Except diagnostic report
///  - The clear command will cause the device to abort the
///    transaction in process as soon as possible without losing any data.
/// @verbatim
///  Power ON Values
///  CS80 Page B-12
///  Table B-5. Complementary Command Power-on Values
///  COMPLEMENTARY POWER-ON       COMPLEMENTARY               POWER-ON
///  COMMAND       VALUE COMMAND  COMMAND                     VALUE
///  Set Unit     0               Set Retry Time              device specific
///  Set Volume   0               Set Status Mask             disabled
///  Set Address  0,0,0           No Op                       Not Applicable
///  Set Length  -1 (full volume) Set Release                 T = O Z = O
///  Set Burst   disable          Set Options                 device specific
///  Set RPS     disabled         Set Return Addressing Mode  single vector
/// 
///  CS80 3-2
///  Clears the following:
///  - clearable hardware functions
///  - internal buffers
///  - channel interface buffers
///  - Complementary values
///  - status report, unless the Diagnostic Result status bit is set
///  - other programmable functions (device dependent)
/// 
///  CS80 3-3 has more detail
///  1. Abort the current operation at the earliest opportunity such
///     that no data corruption can take place.
///  2. Clear all clearable device or interface conditions currently asserted.
///  3. Reset all Complementary parameters to their power-on values.
///  4. Reset status report, unless the Diagnostic Result status bit is
///     set. This includes resetting power-on status.
///     Note- After the device has reported a diagnostic failure on one unit,
///     the Clear command will clear the status of other units attempting to
///     report the same failure. It is unnecessary for the same failure to
///     be reported more than once.
///  5. Set QSTAT value to indicate whether or not status should be
///     requested. Note: QSTAT will indicate any diagnostic results
///     in addition to the occurrence of an internal release request.
///  6. Enter the reporting state.
/// @endverbatim

/// @todo We do not have RPS or Burst settings yet so we do not clear them/
/// @todo We do not have a diagnostic mode so bits ALWAYS get cleared.

/// @param[in] u: unit
/// @return  void

void Clear_Common(int u)
{
    if(u != SS80State.unitNO && u != 15)
        return;

    if(u == 15)
        SS80State.unitNO = 0;
    SS80State.volNO = 0;
    SS80State.AddressBlocks = 0;
    SS80State.Length = 0;
    SS80State.estate = EXEC_IDLE;

/// @todo FIXME
///    Normally the poweron value is -1 (Full volume)
///    However; Any read will try to read the whole volume.
///    (This is Only used in disk copy functions)
///    A setting of 0 will offer the best chance of the
///    error being spotted by the Controller in Charge
///    However; Typically systems will always set this value
///    BEFORE a transfer.

/// @todo FIXME
///    If we have a Diagnostic Report bit then
///    we do not clear the Status/Error bits
///  SS80 4-12
    SS80State.Errors = 0;                                   // Status Mask
    SS80State.qstat = 0;                                    // Clear qstat
}


/// @brief  Channel Independent Clear
///
/// @see: Clear_Common()
/// - Reference:
///  - SS80 4-11
///  - CS80 4-26, 3-2,3-5
/// - State: TRANSPARENT 0x72
/// - Phases: Command, Report
/// @param[in] u: Unit
/// @return  0

int SS80_Channel_Independent_Clear( int u )
{
    Clear_Common( u );
    EnablePPR(SS80_PPR);
    return(0);
}


/// @brief  Universal Device CLear
///
/// @see: Clear_Common()
/// - Reference:
///  - SS80 4-11
///  - CS80 4-26, 3-2,3-3
/// - State: TRANSPARENT, 0x14
/// - Phases: Command, Report
/// @return  0

int SS80_Universal_Device_Clear(void)
{
    Clear_Common(15);
/// @todo FIXME
    EnablePPR(SS80_PPR);
    return(0);
}


/// @brief  Selected Device Clear
///
/// @see: Clear_Common()
/// - Reference:
///  - SS80 ps 4-2 - *** listed as NOT supported for SS80 ***
///  - CS80 pg 4-49, 3-2, 3-4
///  - CS80 Page B-12
/// @return  0

int SS80_Selected_Device_Clear( int u )
{
#if SDEBUG > 1
    if(debuglevel > 1)
        printf("[SS80 SDC]\n");
#endif
    Clear_Common( u );
    EnablePPR(SS80_PPR);
    return(0);
}


/// @brief  Amigo Clear
///
/// - Reference: SS80 4-5
/// @return  0 on send
/// @return GPIB flags on send error.
/// @see gpib.h _FLAGS defines for a full list.
/// @todo  do something with Parity Check Bit ?

int SS80_Amigo_Clear( void )
{
    uint8_t parity[1];
    uint16_t status;

    status = 0;
    if( gpib_read_str(parity, sizeof(parity), &status) != sizeof(parity))
    {
#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[GPIB Read Error]\n");
#endif
        return(status & ERROR_MASK);
    }
    else
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[Amigo Clear]\n");
#endif
    }
    Clear_Common(15);
    EnablePPR(SS80_PPR);
    return(0);
}


/// @brief  Cancel transaction
///
/// - Reference:
///  - SS80 4-9
///  - CS80 4-26, 3-6
/// - State:  TRANSPARENT 0x72
///  - Phases: Command, Report
/// @todo  - add code

int SS80_Cancel( int u )
{
/// @todo FIXME

    SS80State.estate = EXEC_IDLE;
    EnablePPR(SS80_PPR);
    return(0);
}


/// @brief  Increment position Addressy by one block
///
/// - References: SS80 4-39, 4-45.
/// @return  0

int SS80_increment( void )
{
    SS80State.AddressBlocks += 1L;
#if SDEBUG > 1
    if(debuglevel > 1)
        printf("[SS80 Increment to (%lx)]\n", SS80State.AddressBlocks);
#endif
    return(0);
}


/// @brief  Common SS80 Error Return function.  Send qstat = 1.
///
/// @return  0 on send
/// @return GPIB flags on send error
/// @see gpib.h _FLAGS defines for a full list.

int SS80_error_return( void )
{
    uint8_t tmp[1];
    uint16_t status = EOI_FLAG;

    SS80State.qstat = 1;
    tmp[0] = SS80State.qstat;

    status = EOI_FLAG;
    if( gpib_write_str(tmp,sizeof(tmp), &status) != sizeof(tmp))
    {
#if SDEBUG >= 1
        if(debuglevel >= 1)
            printf("[GPIB Error Return - Write ERROR]\n");
#endif
        return(status & ERROR_MASK);
    }
    SS80State.qstat = 0;
    return(0);
}


/// @brief SS80 COMMANDS States
///
/// -Reference: SS80 3-2..9
///  - 0x6e Execute State
///  - 0x65 Command State
///  - 0x70 Report State
///  - 0x72 Transparent State
/// @param[in] ch command.
/// @return  0 on send
/// @see GPIB error status
/// @see gpib.h _FLAGS defines for a full list.
int SS80_COMMANDS(uint8_t ch)
{
    if(SS80_is_MTA(talking) || SS80_is_MLA(listening))
    {
        if(ch == 0x65 )
        {
            if(SS80_is_MLA(listening))
            {
#if SDEBUG > 1
                if(debuglevel > 1)
                    printf("[SS80 Command State]\n");
#endif
                return ( SS80_Command_State() );
            }
            return(0);
        }

        if(ch == 0x6e )
        {
            if(SS80_is_MLA(listening)  || SS80_is_MTA(talking))
            {
#if SDEBUG > 1
                if(debuglevel > 1)
                    printf("[SS00 Execute State]\n");
#endif
                return ( SS80_Execute_State() );

            }
            return(0);
        }

        if(ch == 0x70 )
        {
            if(SS80_is_MTA(talking) )
            {
#if SDEBUG > 1
                if(debuglevel > 1)
                    printf("[SS80 Report State]\n");
#endif
                return( SS80_Report() );
            }

            if(SS80_is_MLA(listening))
            {
#if SDEBUG > 1
                if(debuglevel > 1)
                    printf("[Amigo Clear]\n");
#endif
                DisablePPR(SS80_PPR);
                return( SS80_Amigo_Clear() );
            }
            return (0);
        }
        if(ch == 0x72 )
        {
            if(SS80_is_MLA(listening) )
            {
#if SDEBUG > 1
                if(debuglevel > 1)
                    printf("[SS80 Transparent]\n");
#endif
                return( SS80_Transparent_State() );
            }
        }
#if SDEBUG > 1
        if(debuglevel > 1)
            printf("[SS80 SC Unknown: %02x, listen:%02x, talk:%02x]\n",
                0xff & ch, 0xff & listening, 0xff & talking);
#endif
        return(0);
    }

#if SDEBUG > 1
    if(debuglevel > 1)
        printf("[SS80 Unknown SC: %02x, listen:%02x, talk:%02x]\n",
            0xff & ch, 0xff & listening, 0xff & talking);
#endif
    return(0);
}
