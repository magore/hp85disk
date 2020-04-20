/**
 @file gpib/amigo.c

 @brief AMIGO disk emulator for HP85 disk emulator project for AVR.

 @par Edit History - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details
@see http://github.com/magore/hp85disk
@see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

@par Based on work by Anders Gustafsson.

@par Copyright &copy; 2014 Anders Gustafsson All rights reserved..

*/

#include "user_config.h"

#include "defines.h"
#include "gpib.h"
#include "gpib_hal.h"
#include "gpib_task.h"
#include "amigo.h"
#include "debug.h"

#ifdef AMIGO

/// @verbatim
/// SS80 References: ("SS80" is the short form used in the project)
///  "Subset 80 from Fixed and flexible disc drives"
///   Printed November, 1985
///   HP Part# 5958-4129
///
/// CS80 References: ("CS80" is the short form used in the project)
///   "CS/80 Instruction Set Programming Manual"
///   Printed: APR 1983
///   HP Part# 5955-3442
///
/// Amigo References: ("Amigo" is the short form used in the project)
///   "Appendix A of 9895A Flexible Disc Memory Service Manual"
///   HP Part# 09895-90030
/// @endverbatim

/// @verbatim
///  Reference: Amigo Command Set Reference used in this document
///  Short Form         Title
///  SS80       "Subset 80 from Fixed and flexible disc drives" 5958-4129
///  HP9123         9123 disc drive manual addendum 5957-6584
///  A          Appendix A "HP 9895A Disc Memory Commends Set"
///             See Receive Status A16
///  L    Buffered Read             0x05    2       A33     Y
///  L    Request Physical Address  0x14    2       A21     Y 0x68
///             See Send Address A20
///
///  0x6B Command Op Codes          OP      bytes
///  L    Buffered Read Verify      0x05    2       A37     Y
///  L    ID Triggered Read         0x06    2       A41     Y
///
///  0x6C Command Op Codes          OP      bytes
///  L    Unbuffered Read Verify    0x05    2       A38     Y
///  L    Request Physical Address  0x14    2       A21     Y 0x68
///  L    Format Request            0x18    2       A50     N
///  L    Door Lock                 0x19    2       A30     N
///  L    Door Unlock               0x1A    2       A31     N
///
///  0x6F Command Op Codes          OP      bytes
///  L    Download Controller       ---     1..256  A26     N
///
///  0x70 Command Op Codes          OP      bytes
///  L    HP-300 Clear              ---     1       A23     N
///  T    DSJ                       ---     1       A11     N
///
///  0x77 Command Op Codes          OP      bytes
///  T/L  HP-IB CRC                 ---     ---     A30     N
///
///  0x7E Command Op Codes          OP      bytes
///  L    Write Loopback Record     ---     1..256  A25     N
///  T    Read Loopback Record      ---     1..256  A14     N
///
///  0x7F Command Op Codes          OP      bytes
///  T    Read Self Test Results    ---     2       A13     N
///  L    Initiat Self Test         ---     2       A24     N
/// @endverbatim

/// @verbatim
/// Secondary_Commands and OP Code processing
///
/// 0x60 Command Op Codes           OP      bytes
/// L    Receive Data               ---     ---
///         Execute of most receive data requests
///         All Disk Write
/// T    Send Data                  ---     ---
///         Execute of most send data requests
///         All Disk Read
///
/// 0x68 Command Op Codes           OP      bytes   Ref     Execute Phase
/// L    Cold Load Read             0x00    2       A40     Y
/// L    Seek                       0x02    6       A27     N
/// L    Request Status buffered    0x03    2       A15     Y 0x68
/// L    Unbuffered Read            0x05    2       A35     Y
/// L    Verify                     0x07    4       A36     N
/// L    Unbuffered Write Request   0x08    2       A45     Y
/// L    Initialize                 0x0B    2       A46     Y
/// L    Initialize DBIT                0x2B    2       A46     Y
/// L    Request Logical Address    0x14    2       A20     Y 0x68
/// L   End                         0x15    2       A29     N
/// T    Send Status or Address         ----    4       A20     N
///         Execute of all Address and Status Requests
///         (Y 0x68)
///
/// 0x69 Command Op Codes           OP      bytes
/// L    Buffered Write Request     0x08    2       A43     Y
///
/// 0x6A Command Op Codes           OP      bytes
/// L    Request Status Unbuffered  0x03    2      A15     Y 0x68
///         See Receive Status A16
/// L    Buffered Read              0x05    2       A33     Y
/// L    Request Physical Address   0x14    2       A21     Y 0x68
///         See Send Address A20
///
/// 0x6B Command Op Codes           OP      bytes
/// L    Buffered Read Verify       0x05    2       A37     Y
/// L    ID Triggered Read          0x06    2       A41     Y
///
/// 0x6C Command Op Codes           OP      bytes
/// L    Unbuffered Read Verify     0x05    2       A38     Y
/// L    Request Physical Address   0x14    2       A21     Y 0x68
/// L    Format Request             0x18    2       A50     N
/// L    Door Lock                  0x19    2       A30     N
/// L    Door Unlock                0x1A    2       A31     N
///
/// 0x6F Command Op Codes           OP      bytes
/// L    Download Controller        ---     1..256  A26     N
///
/// 0x70 Command Op Codes           OP      bytes
/// L    HP-300 Clear               ---     1       A23     N
/// T    DSJ                        ---     1       A11     N
///
/// 0x77 Command Op Codes           OP      bytes
/// T/L  HP-IB CRC                  ---     ---     A30     N
///
/// 0x7E Command Op Codes           OP      bytes
/// L    Write Loopback Record      ---     1..256  A25     N
/// T    Read Loopback Record       ---     1..256  A14     N
///
/// 0x7F Command Op Codes           OP      bytes
/// T    Read Self Test Results     ---     2       A13     N
/// L    Initiat Self Test          ---     2       A24     N
/// @endverbatim

extern uint8_t talking;
extern uint8_t listening;

enum AMIGO_states
{
    AMIGO_IDLE = 0,
    AMIGO_REQUEST_STATUS,
    AMIGO_REQUEST_STATUS_UNBUFFERED,
    AMIGO_REQUEST_STATUS_BUFFERED,
    AMIGO_REQUEST_LOGICAL_ADDRESS,
    AMIGO_COLD_LOAD_READ,
    AMIGO_READ_UNBUFFERED,
    AMIGO_READ_BUFFERED,
    AMIGO_WRITE_UNBUFFERED,
    AMIGO_WRITE_BUFFERED,
    AMIGO_INITIALIZE,
};

/// @brief Initialize Amigo state machine, Disk Position, Error status
/// @return  void

void amigo_init()
{
    int i;
    for(i=MAX_DEVICES-1;i>=0;--i)
    {
        if(Devices[i].TYPE == AMIGO_TYPE)
        {
            set_active_device(i);
            AMIGOs->state = AMIGO_IDLE;

            memset(AMIGOs->status,0,sizeof(AMIGOs->status));
            memset(AMIGOs->logical_address,0,sizeof(AMIGOs->logical_address));

///TODO we do NOT support multiple units yet
            AMIGOs->unitNO = 0;

            AMIGOs->sector = 0;
            AMIGOs->head = 0;
            AMIGOs->cyl = 0;

            AMIGOs->dsj = 2;
            AMIGOs->Errors = 0;

/// @todo  verify that we always want PPR disabled
            gpib_disable_PPR(AMIGOp->HEADER.PPR);
        }
    }
}


/// @brief  Set current address in AMIGOs->logical_address[]
///
/// - Reference: A15-A19
/// - State: Command
/// @return  0

int amigo_request_logical_address()
{

    AMIGOs->logical_address[0] = 0xff & (AMIGOs->cyl >> 8);
//LSB
    AMIGOs->logical_address[1] = 0xff & (AMIGOs->cyl);
    AMIGOs->logical_address[2] = 0xff & (AMIGOs->head);
    AMIGOs->logical_address[3] = 0xff & (AMIGOs->sector);
    return(0);
}


/// @brief  Set drive status in AMIGOs->status[]
///
/// - Reference: A15-A19.
/// - State: Command.
/// @return  0

int amigo_request_status()
{
/// @todo  dsj
#if SDEBUG
    if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
        printf("[AMIGO request status]\n");
#endif
    AMIGOs->status[0] = 0x00;                     // Status 1
///TODO we do NOT support multiple units yet
    AMIGOs->status[1] = AMIGOs->unitNO;           // Unit
    AMIGOs->status[2] = 0x0d;                     // Status 2 (0110 = hp format) << 1, 1=HP9121
    AMIGOs->status[3] = 0x00;                     //

    if(mmc_wp_status())
    {
        AMIGOs->status[3] |= 0x40;                // Write protect 0x40, reserved = 0x20
        AMIGOs->status[3] |= 0x20;                // reserved = 0x20 ???
    }

    if(AMIGOs->dsj == 2)
    {
        AMIGOs->status[0] = 0b00010011;           // S1 error power on
        AMIGOs->status[3] |= 0x08;                // F bit, power up
    }
    else if(AMIGOs->Errors || AMIGOs->dsj == 1)
    {
//FIXME added invalid unit error
        if(AMIGOs->Errors & ERR_UNIT)
            AMIGOs->status[0] = 0b00010011;       // Unit Error
        else if(AMIGOs->Errors & ERR_GPIB)
            AMIGOs->status[0] = 0b00001010;       // S1 error I/O error
        else if(AMIGOs->Errors & ERR_DISK)
            AMIGOs->status[3] |= 0x03;            // Do disk in drive
        else if(AMIGOs->Errors & ERR_WRITE)
            AMIGOs->status[0] = 0b00010011;       // S1 error write error
        else if(AMIGOs->Errors & ERR_SEEK)
            AMIGOs->status[3] |= 0x04;            // Seek

        AMIGOs->status[3] |= 0x10;                // E bit hardware failure
        AMIGOs->status[2] |= 0x80;                // Bit 15
    }

    gpib_enable_PPR(AMIGOp->HEADER.PPR);
    return(0);
}


/// @brief  Send current address in AMIGOs->logical_address[]
///
/// - Reference: A15-A19.
/// - State: Execute
/// @return  0 on sucess
/// @return status GPIB error flags on fail
/// @see gpib.h ERROR_MASK defines for a full list.

int amigo_send_logical_address()
{
    uint16_t status;
    UINT len;

#if SDEBUG
    if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
        printf("[AMIGO send logical address]\n");
#endif
    status = EOI_FLAG;
    len = gpib_write_str(AMIGOs->logical_address,4,&status);
    if(status & ERROR_MASK)
        AMIGOs->Errors |= ERR_GPIB;
    if( len != 4)
    {
        AMIGOs->Errors |= ERR_GPIB;
        AMIGOs->dsj = 1;
        if(debuglevel & GPIB_PPR)
            printf("[AMIGO GPIB write error]\n");
        gpib_enable_PPR(AMIGOp->HEADER.PPR);
        return(status & ERROR_MASK);
    }
    gpib_enable_PPR(AMIGOp->HEADER.PPR);
    return(status & ERROR_MASK);
}


/// @brief Send drive status in AMIGOs->status[]
///
/// - Reference: A15-A19.
/// - State: Execute>
/// @return  0
/// @return status GPIB error flags on fail
/// @see gpib.h ERROR_MASK defines for a full list.

int amigo_send_status()
{
    uint16_t status;
    UINT len;

#if SDEBUG
    if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
        printf("[AMIGO send status]\n");
#endif
    status = EOI_FLAG;
    len = gpib_write_str(AMIGOs->status,4,&status);
    if(status & ERROR_MASK)
        AMIGOs->Errors |= ERR_GPIB;
    if( len != 4)
    {
        AMIGOs->Errors |= ERR_GPIB;
        AMIGOs->dsj = 1;
        if(debuglevel & GPIB_PPR)
            printf("[AMIGO GPIB write error]\n");
        gpib_enable_PPR(AMIGOp->HEADER.PPR);
        return(status & ERROR_MASK);
    }
    AMIGOs->Errors = 0;
    AMIGOs->dsj = 0;
    gpib_enable_PPR(AMIGOp->HEADER.PPR);
    return(status & ERROR_MASK);
}


/// @brief  Convert Drive CHS position to offset
///
/// - Reference: A15-A19.
/// @param[in] p: Disk Layout
/// @param[in] msg: user message on error
/// @return  0

static DWORD amigo_chs_to_logical(AMIGOStateType *p, char *msg)
{
    DWORD pos;
    pos =  (long) ( AMIGOp->GEOMETRY.SECTORS_PER_TRACK * p->head);
    pos += (long) ( (AMIGOp->GEOMETRY.SECTORS_PER_TRACK * AMIGOp->GEOMETRY.HEADS) * p->cyl);
    pos += (long) p->sector;
    pos *= (long) AMIGOp->GEOMETRY.BYTES_PER_SECTOR;

#if SDEBUG
    if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
        printf("[AMIGO %s, P:%08lxH, U:%d C:%d H:%d S:%d]\n",
            msg, pos, AMIGOs->unitNO, p->cyl, p->head, p->sector);
#endif
    return(pos);
}


/// @brief  Check for CHS position overflow
///
/// - Reference: A15-A19
/// @return 0
/// @return 1 overflow

static int amigo_overflow_check(AMIGOStateType *p, char *msg)
{
    int stat = 0;
    while(p->sector >= AMIGOp->GEOMETRY.SECTORS_PER_TRACK)
    {
        p->sector = 0;
        p->head++;
        while (p->head >= AMIGOp->GEOMETRY.HEADS)
        {
            p->head = 0;
            p->cyl++;
            if (p->cyl >= AMIGOp->GEOMETRY.CYLINDERS)
            {
                stat = 1;
                if(debuglevel & GPIB_PPR && msg != NULL)
                    printf("[AMIGO %s pos OVERFLOW]\n", msg);
            }
        }
    }
    return(stat);
}


/// @brief Increment sector if it will not overflow.
///
/// @param[in] msg: user message on overflow.
///
/// @return 0 ok.
/// @return 1 error.

int amigo_increment(char *msg)
{
    int stat = 0;
    AMIGOStateType tmp = *AMIGOs;

    ++tmp.sector;

    stat = amigo_overflow_check((AMIGOStateType *) &tmp, msg);
    if(!stat)
    {
        *AMIGOs = tmp;
    }
    return(stat);
}


/// @brief Seek to the position held in the disk structure.
///
///  - Reference: A27.
///  - We should not commit bad head/sector/cylinder values until tested!..
///    We update our real address only on sucess
///
/// @param[in] p:  AMIGOStateType (Current Disk Position) pointer.
///
/// @return 0 ok
/// @return 1 error

int amigo_seek( AMIGOStateType *p)
{
    int stat = 0;

    stat = amigo_overflow_check(p, "Seek");
    if(stat)
    {
        AMIGOs->dsj = 1;
        AMIGOs->Errors |= ERR_SEEK;
    }
    else
    {
        AMIGOs->sector = p->sector;
        AMIGOs->head = p->head;
        AMIGOs->cyl = p->cyl;
    }

    gpib_enable_PPR(AMIGOp->HEADER.PPR);
    return(stat);
}


/// @brief  Verify Disk Sectors at current position
///
/// - Refernce: A36
/// @return 0 ok
/// @return 1 Error, Sets dsj and Amigo_errors

int amigo_verify(uint16_t sectors)
{
    int len;
    int stat = 0;
    DWORD pos;

    pos = amigo_chs_to_logical(AMIGOs, "Verify Start");

#if SDEBUG
    if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
        printf("[AMIGO verify P:%08lXH, sectors:%04XH]\n", pos, sectors);
#endif

    while(sectors--)
    {
        pos = amigo_chs_to_logical(AMIGOs, "Verfify");

#if SDEBUG
        if(debuglevel & GPIB_DISK_IO_TIMING)
            gpib_timer_elapsed_begin();
#endif

        len = dbf_open_read(AMIGOp->HEADER.NAME, pos, gpib_iobuff, AMIGOp->GEOMETRY.BYTES_PER_SECTOR, &AMIGOs->Errors);

#if SDEBUG
        if(debuglevel & GPIB_DISK_IO_TIMING)
            gpib_timer_elapsed_end("Disk Read");
#endif
        if(len != AMIGOp->GEOMETRY.BYTES_PER_SECTOR)
        {
            AMIGOs->dsj = 1;
            AMIGOs->Errors |= ERR_READ;
            stat = 1;
            break;
        }
        if(amigo_increment("Verify"))             // address overflow
        {
            stat = 1;
            break;
        }
    }
    gpib_enable_PPR(AMIGOp->HEADER.PPR);
    return(stat);
}


/// @brief  Format Disk
///
/// - Refernce: A50.
///
/// @param[in] db: byte to fill sector buffer with)
///
/// @return 0 ok
/// @return 1 on Error,Sets dsj and Amigo_errors
int amigo_format(uint8_t db)
{

    DWORD pos;
    int len;
    int stat = 0;

    AMIGOs->sector = 0;
    AMIGOs->head = 0;
    AMIGOs->cyl = 0;

    memset((void *) gpib_iobuff, db, AMIGOp->GEOMETRY.BYTES_PER_SECTOR);

#if SDEBUG
    if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
        printf("[AMIGO format]\n");
    if(debuglevel & GPIB_DISK_IO_TIMING)
        gpib_timer_elapsed_begin();
#endif
    while( 1 )
    {
///@brief computer logical block
        pos = amigo_chs_to_logical(AMIGOs, "Format");

        len = dbf_open_write(AMIGOp->HEADER.NAME,
            pos, gpib_iobuff,AMIGOp->GEOMETRY.BYTES_PER_SECTOR, &AMIGOs->Errors);

        if(len != AMIGOp->GEOMETRY.BYTES_PER_SECTOR)
        {
            AMIGOs->Errors |= ERR_WRITE;
            AMIGOs->dsj = 1;
            stat = 1;
            break;
        }

///@brief increment sector/cyl do not display at overflow (expected)
        if( amigo_increment(NULL) )
        {
// reset sector,head,cyl
            AMIGOs->sector = 0;
            AMIGOs->head = 0;
            AMIGOs->cyl = 0;
            AMIGOs->dsj = 0;
            stat = 0;
            break;
        }

    }
#if SDEBUG
    if(debuglevel & GPIB_DISK_IO_TIMING)
        gpib_timer_elapsed_end("Format");
    if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
        printf("[AMIGO Format Done]\n");
#endif
    gpib_enable_PPR(AMIGOp->HEADER.PPR);
    return(stat);
}


/// @brief  Do a buffered read of the current sector.
///
/// - Reference: A33.
/// @return 0 ok
/// @return GPIB Errors status,Sets dsj and Amigo_errors

int amigo_buffered_read()
{
    uint16_t status;
    int len;
    DWORD pos;

    pos = amigo_chs_to_logical(AMIGOs, "Buffered Read");

#if SDEBUG
    if(debuglevel & GPIB_DISK_IO_TIMING)
        gpib_timer_elapsed_begin();
#endif

    len = dbf_open_read(AMIGOp->HEADER.NAME, pos, gpib_iobuff, AMIGOp->GEOMETRY.BYTES_PER_SECTOR, &AMIGOs->Errors);

#if SDEBUG
    if(debuglevel & GPIB_DISK_IO_TIMING)
        gpib_timer_elapsed_end("Disk Read");
#endif
    if(len != AMIGOp->GEOMETRY.BYTES_PER_SECTOR)
    {
        AMIGOs->dsj = 1;
        return(0);
    }

#if SDEBUG
    if(debuglevel & GPIB_RW_STR_TIMING)
        gpib_timer_elapsed_begin();
#endif
    status = EOI_FLAG;
    len = gpib_write_str(gpib_iobuff, AMIGOp->GEOMETRY.BYTES_PER_SECTOR, &status);
#if SDEBUG
    if(debuglevel & GPIB_RW_STR_TIMING)
        gpib_timer_elapsed_end("GPIB write");
#endif
    if(status & ERROR_MASK || len != AMIGOp->GEOMETRY.BYTES_PER_SECTOR)
    {
        AMIGOs->dsj = 1;
        AMIGOs->Errors |= ERR_GPIB;
        if(debuglevel & GPIB_PPR)
            printf("[AMIGO GPIB write error]\n");
        gpib_enable_PPR(AMIGOp->HEADER.PPR);
        return(status & ERROR_MASK);
    }

/// @todo  Do we fail on overflow ?
///  currently djs is set - do we want to report that now ?
    if( amigo_increment("Buffered Read") )        //overflow
    {
        AMIGOs->dsj = 1;
        AMIGOs->Errors |= ERR_SEEK;
    }

    gpib_enable_PPR(AMIGOp->HEADER.PPR);
    return(0);
}


/// @brief  Do a buffered write of the current sector.

/// - Reference: A43.
/// @return 0 ok
/// @return GPIB Errors status,Sets dsj and Amigo_errors

int amigo_buffered_write()
{
    uint16_t status;
    int len;

    DWORD pos;

    pos = amigo_chs_to_logical(AMIGOs, "Buffered Write");

#if SDEBUG
    if(debuglevel & GPIB_RW_STR_TIMING)
        gpib_timer_elapsed_begin();
#endif
    status = 0;
    len = gpib_read_str(gpib_iobuff, AMIGOp->GEOMETRY.BYTES_PER_SECTOR, &status);

#if SDEBUG
    if(debuglevel & GPIB_RW_STR_TIMING)
        gpib_timer_elapsed_end("GPIB read str");
#endif

    if(status & ERROR_MASK || len != AMIGOp->GEOMETRY.BYTES_PER_SECTOR)
    {
        AMIGOs->dsj = 1;
        AMIGOs->Errors |= ERR_GPIB;
        if(debuglevel & GPIB_PPR)
            printf("[AMIGO Write GPIB read error]\n");
        gpib_enable_PPR(AMIGOp->HEADER.PPR);
        return(status & ERROR_MASK);
    }

#if SDEBUG
    if(debuglevel & GPIB_DISK_IO_TIMING)
        gpib_timer_elapsed_begin();
#endif

    len = dbf_open_write(AMIGOp->HEADER.NAME, pos, gpib_iobuff, AMIGOp->GEOMETRY.BYTES_PER_SECTOR, &AMIGOs->Errors);

#if SDEBUG
    if(debuglevel & GPIB_DISK_IO_TIMING)
        gpib_timer_elapsed_end("Disk Write");
#endif

    if(len != AMIGOp->GEOMETRY.BYTES_PER_SECTOR)
    {
        AMIGOs->dsj = 1;
        return(0);
    }
    if( amigo_increment("Buffered Write") )       //overflow
    {
        AMIGOs->dsj = 1;
        AMIGOs->Errors |= ERR_SEEK;
    }
    gpib_enable_PPR(AMIGOp->HEADER.PPR);
    return(status & ERROR_MASK);
}


/// @brief  Send DSJ
///
/// - Reference: A11.
/// @return 0 ok
/// @return GPIB send Errors status
/// - Sets dsj and Amigo_errors

int amigo_cmd_dsj()
{
    uint8_t tmp[1];
    uint16_t status;
    int len;

    tmp[0] = AMIGOs->dsj;

    status = EOI_FLAG;
    len = gpib_write_str(tmp, sizeof(tmp), &status);
    if(status & ERROR_MASK)
        AMIGOs->Errors |= ERR_GPIB;
    if(len != sizeof(tmp))
    {
        AMIGOs->dsj = 1;
        AMIGOs->Errors |= ERR_GPIB;
        if(debuglevel & GPIB_PPR)
            printf("[AIMGO: DSJ send failed]\n");
        return(status & ERROR_MASK);
    }
    else
    {
#if SDEBUG
        if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
            printf("[DSJ %02XH]\n", AMIGOs->dsj);
#endif
    }
    AMIGOs->dsj = 0;
    AMIGOs->Errors = 0;
    return ( 0 );
}


/// @brief  Wakeup (undocumented), just enable PPR response.
///
/// @todo - not finished
/// - Sets dsj and Amigo_errors
///
/// @return  GPIB Errors status - or 0 OK

int amigo_cmd_wakeup()
{
    uint8_t tmp[1];
    uint16_t status;
    UINT len;

#if SDEBUG
    if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
        printf("[AMIGO Wakeup]\n");
#endif
    tmp[0] = AMIGOs->dsj;
    len = gpib_write_str(tmp, 1, &status);
    if(status & ERROR_MASK)
        AMIGOs->Errors |= ERR_GPIB;
    if( len != 1)
    {
        AMIGOs->dsj = 1;
        AMIGOs->Errors |= ERR_GPIB;
        if(debuglevel & GPIB_PPR)
            printf("[AMIGO GPIB write error]\n");
    }
/// @todo FIXME
    gpib_enable_PPR(AMIGOp->HEADER.PPR);
    AMIGOs->dsj = 0;
    return(status & ERROR_MASK);
}


/// @brief  Response to selected device clear or all clear
///
/// - Reference: A23.
/// - Select sector 0.
/// @return  0

int amigo_cmd_clear()
{
#if SDEBUG
    if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
        printf("[AMIGO Clear]\n");
#endif
    AMIGOs->sector = 0;
    AMIGOs->head = 0;
    AMIGOs->cyl = 0;
/// @todo FIXME
///
///  Clear the DSJ byte that might be 2 after powerup
///
    AMIGOs->dsj = 0;
    AMIGOs->Errors =0;

    gpib_enable_PPR(AMIGOp->HEADER.PPR);
    return(0);
}


/// @brief  Report any Unimplimented commands with Op Codes.
///
/// @param[in] secondary: GPIB secondary command
/// @param[in] opcode: GPIB data byte OP Code
/// @param[in] len: Length of GOIB data returned
/// @return  0

int amigo_todo_op(uint8_t secondary, uint8_t opcode, int len)
{
    if(AMIGO_is_MLA(listening))
        printf("[L   Amigo TODO secondary: %02XH, state:%02XH, opcode:%02XH, len:%3d, listening:%02XH, talking:%02XH]\n",
            secondary, AMIGOs->state, opcode, len, listening, talking);
    else if(AMIGO_is_MTA(talking))
        printf("[T   Amigo TODO secondary: %02XH, state:%02XH, opcode:%02XH, len:%3d, listening:%02XH, talking:%02XH]\n",
                secondary, AMIGOs->state, opcode, len, listening, talking);
    else if(talking == UNT)
        printf("[UNT Amigo TODO secondary: %02XH, state:%02XH, opcode:%02XH, len:%3d, listening:%02XH, talking:%02XH]\n",
                secondary, AMIGOs->state, opcode, len, listening, talking);
    else
        printf("[U Amigo TODO secondary: %02XH, state:%02XH, opcode:%02XH, len:%3d, listening:%02XH, talking:%02XH]\n",
            secondary, AMIGOs->state, opcode, len, listening, talking);
    gpib_enable_PPR(AMIGOp->HEADER.PPR);
    return(0);
}


/// @brief  Report any Unimplimented commands.
///
/// @param[in] secondary: GPIB seconday command
///
/// @return  0

int amigo_todo(uint8_t secondary)
{
    if(AMIGO_is_MLA(listening))
        printf("[L   Amigo TODO secondary: %02XH, state:%02XH, listening:%02XH, talking:%02XH]\n",
            secondary,AMIGOs->state,listening,talking);
    else if(AMIGO_is_MTA(talking))
        printf("[T   Amigo TODO secondary: %02XH, state:%02XH, listening:%02XH, talking:%02XH]\n",
                secondary,AMIGOs->state,listening,talking);
    else if(talking == UNT)
        printf("[UNT Amigo TODO secondary: %02XH, state:%02XH, listening:%02XH, talking:%02XH]\n",
                secondary,AMIGOs->state,listening,talking);
    else
        printf("[E   Amigo ERROR secondary: %02XH, state:%02XH, listening:%02XH, talking:%02XH]\n",
            secondary,AMIGOs->state,listening,talking);
    gpib_enable_PPR(AMIGOp->HEADER.PPR);
    return(0);
}


/// @brief  Amigo Check Valid unit
/// @param[in] unit: unit to set
/// @return void
void amigo_check_unit(uint8_t unit)
{
    if(unit != 15)
        AMIGOs->unitNO = unit;
    if(AMIGOs->unitNO != 0)
        AMIGOs->Errors |= ERR_UNIT;
}


/// @brief  Amigo Command and OP Code Processing functions.
///
/// - We disbale PPR as soon as a valide command is decoded.
/// - ONLY process Secondary Commands in listen mode with op codes and optional data
///  - Ignore Talking.
///  - Ignore Seconday Addresses.
///  - Ignore Execute state "0x60" here.
///  - Must have OP Codes, or Data, and EOI.
/// - Read all of the Data/Opcodes/Parameters at once - while ATN is false.
/// - Last byte read should be EOI or an error.
/// - Unknown OP Code processing rules.
///  - Skip the remaining codes, Wait for Report Phase.
/// - We enable PPR on exit.
///
/// @param[in] secondary: command
///
/// @return  0 on sucess
/// @return or GPIB error flags on fail
/// @see  gpib.h ERROR_MASK defines for a full list)

int Amigo_Command( int secondary )
{
    uint8_t op;                                   // Current OP Code
    uint8_t *ptr;
    uint16_t status;                              // Current status
    UINT len;                                     // Size of Data/Op Codes/Parameters read in bytes

#if SDEBUG
    if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
        printf("[AMIGO Command(%02XH): listen:%02XH, talk:%02XH]\n",
            secondary, listening, talking);
#endif

///  Reference: A14
    if (secondary == 0x7e && AMIGO_is_MTA(talking))
    {
        gpib_disable_PPR(AMIGOp->HEADER.PPR);
        status = EOI_FLAG;
        len = gpib_write_str(gpib_iobuff, GPIB_IOBUFF_LEN, &status);
        gpib_enable_PPR(AMIGOp->HEADER.PPR);
        if(status & ERROR_MASK)
        {
            AMIGOs->dsj = 1;
            AMIGOs->Errors |= ERR_GPIB;
            if(debuglevel & GPIB_PPR)
                printf("[AMIGO_Command:GPIB write error]\n");
        }
        return(status & ERROR_MASK);
    }

///  Reference: A25
    if (secondary == 0x7f && AMIGO_is_MLA(listening))
    {
        gpib_disable_PPR(AMIGOp->HEADER.PPR);
#if SDEBUG
        if(debuglevel & GPIB_RW_STR_TIMING)
            gpib_timer_elapsed_begin();
#endif
        status = EOI_FLAG;
        len = gpib_read_str(gpib_iobuff, GPIB_IOBUFF_LEN, &status);
#if SDEBUG
        if(debuglevel & GPIB_RW_STR_TIMING)
            gpib_timer_elapsed_end("GPIB read str");
#endif
        gpib_enable_PPR(AMIGOp->HEADER.PPR);
        if(status & ERROR_MASK)
        {
            AMIGOs->dsj = 1;
            AMIGOs->Errors |= ERR_GPIB;
            if(debuglevel & GPIB_PPR)
                printf("[AMIGO Command:GPIB read error]\n");
        }
        return(status & ERROR_MASK);
    }

    if( !AMIGO_is_MLA(listening) || AMIGO_is_MTA(talking))
        return(0);

    gpib_disable_PPR(AMIGOp->HEADER.PPR);

///  Note: the function will "unread" any commands and return
///  All Data MUST have EOI

    status = EOI_FLAG;
    len = gpib_read_str(gpib_iobuff, GPIB_IOBUFF_LEN, &status);
    if(status & ERROR_MASK)
    {
        AMIGOs->dsj = 1;
        AMIGOs->Errors |= ERR_GPIB;
        if(debuglevel & GPIB_PPR)
            printf("[AMIGO Command:GPIB read error]\n");
        return(status & ERROR_MASK);
    }
#if SDEBUG
    if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
        printf("[AMIGO Command(%02XH): GPIB read bytes:%02XH]\n",
            secondary, len);
#endif
    if(!len)
    {
        AMIGOs->dsj = 1;
        AMIGOs->Errors |= ERR_GPIB;
        return(status & ERROR_MASK);
    }

    ptr = gpib_iobuff;
    op = *ptr++;

    if (secondary == 0x68)
    {
        if(op == 0x00 && len == 2)
        {
///  Reference: A40

            AMIGOStateType tmp;
#if SDEBUG
            if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                printf("[AMIGO Cold Load Read Command]\n");
#endif
///TODO we do NOT support multiple units yet
            AMIGOs->unitNO = 0;
            AMIGOs->dsj = 0;
            AMIGOs->Errors = 0;
/// Fill in temparary address
            tmp.cyl = 0;
            tmp.head = ( (0xff & *ptr) >> 6) & 0x03;
            tmp.sector = 0x3f & *ptr;
            ++ptr;
//update to real address on sucess
            amigo_seek((AMIGOStateType *) &tmp);
            AMIGOs->state = AMIGO_COLD_LOAD_READ;
            gpib_enable_PPR(AMIGOp->HEADER.PPR);
            return(status & ERROR_MASK);
        }
        else if(op == 0x02 && len == 5)
        {
///  Reference: A27
/// @brief
///  Seek 1 byte cylinder

            AMIGOStateType tmp;
#if SDEBUG
            if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                printf("[AMIGO Seek len=5]\n");
#endif
///TODO we do not support multiple units yet
///FIXME Added unit error
            amigo_check_unit(0xff & *ptr++);

/// Fill in temparary address
            tmp.cyl = 0xff & *ptr++;
            tmp.head = 0xff & *ptr++;
            tmp.sector = 0xff & *ptr++;
//update to real address on sucess
            amigo_seek((AMIGOStateType *)&tmp);
            gpib_enable_PPR(AMIGOp->HEADER.PPR);
            return(status & ERROR_MASK);
        }
        else if(op == 0x02 && len == 6)
        {
///  Reference: A27
/// @brief
///  Seek 2 byte cylinder

            AMIGOStateType tmp;
#if SDEBUG
            if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                printf("[AMIGO Seek len=6]\n");
#endif
///TODO we do not support multiple units yet
///FIXME Added unit error
            amigo_check_unit(0xff & *ptr++);
/// Fill in temparary address
            tmp.cyl = (0xff & *ptr++) << 8;       // MSB
            tmp.cyl |= (0xff & *ptr++);           // LSB
            tmp.head = 0xff & *ptr++;
            tmp.sector = 0xff & *ptr++;
//update to real address on sucess
            amigo_seek((AMIGOStateType *)&tmp);
            gpib_enable_PPR(AMIGOp->HEADER.PPR);
            return(status & ERROR_MASK);
        }
        else if(op == 0x03 && len == 2)
        {
///  Reference: A15
#if SDEBUG
            if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                printf("[AMIGO Request Status Buffered Command]\n");
#endif
///TODO we do not support multiple units yet
///FIXME Added unit error
            amigo_check_unit(0xff & *ptr++);
            amigo_request_status();
            AMIGOs->state = AMIGO_REQUEST_STATUS_BUFFERED;
            return(status & ERROR_MASK);
        }
        else if(op == 0x05 && len == 2)
        {
///  Reference: A35
#if SDEBUG
            if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                printf("[AMIGO Read Unbuffered Command]\n");
#endif
///TODO we do not support multiple units yet
///FIXME Added unit error
            amigo_check_unit(0xff & *ptr++);
            AMIGOs->state = AMIGO_READ_UNBUFFERED;
            gpib_enable_PPR(AMIGOp->HEADER.PPR);
            return(status & ERROR_MASK);
        }
        else if(op == 0x07 && len == 4)
        {
            uint16_t sectors;
#if SDEBUG
            if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                printf("[AMIGO Verify]\n");
#endif
///TODO we do not support multiple units yet
///FIXME Added unit error
            amigo_check_unit(0xff & *ptr++);
            sectors = (0xff & *ptr++) << 8;
            sectors |= (0xff & *ptr++);
            return ( amigo_verify( sectors) );
        }
        else if(op == 0x08 && len == 2)
        {
#if SDEBUG
            if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                printf("[AMIGO Write Unbuffered Command]\n");
#endif
///TODO we do not support multiple units yet
///FIXME Added unit error
            amigo_check_unit(0xff & *ptr++);
            AMIGOs->state = AMIGO_WRITE_UNBUFFERED;
            gpib_enable_PPR(AMIGOp->HEADER.PPR);
            return(status & ERROR_MASK);
        }
        else if((op == 0x0B || op == 0x2b) && len == 2)
        {
#if SDEBUG
            if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                printf("[AMIGO Initialize Command]\n");
#endif
///TODO we do not support multiple units yet
///FIXME Added unit error
            amigo_check_unit(0xff & *ptr++);
            AMIGOs->state = AMIGO_INITIALIZE;
            gpib_enable_PPR(AMIGOp->HEADER.PPR);
            return(status & ERROR_MASK);
        }
        else if(op == 0x14 && len == 2)
        {
#if SDEBUG
            if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                printf("[AMIGO Request Logical Address Command]\n");
#endif
            amigo_request_logical_address();
            AMIGOs->state = AMIGO_REQUEST_LOGICAL_ADDRESS;
            gpib_enable_PPR(AMIGOp->HEADER.PPR);
            return(status & ERROR_MASK);
        }
    }
    else if (secondary == 0x69)
    {
        if(op == 0x08 && len == 2)
        {
#if SDEBUG
            if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                printf("[AMIGO Write Buffered Command]\n");
#endif
///TODO we do not support multiple units yet
///FIXME Added unit error
            amigo_check_unit(0xff & *ptr++);
            AMIGOs->state = AMIGO_WRITE_BUFFERED;
            gpib_enable_PPR(AMIGOp->HEADER.PPR);
            return(status & ERROR_MASK);
        }
    }
    else if (secondary == 0x6A)
    {
        if(op == 0x08 && len == 2)
        {
#if SDEBUG
            if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                printf("[AMIGO Request Status Unbuffered Command]\n");
#endif
///TODO we do not support multiple units yet
///FIXME Added unit error
            amigo_check_unit(0xff & *ptr++);
            AMIGOs->state = AMIGO_REQUEST_STATUS_UNBUFFERED;
            amigo_request_status();
            return(status & ERROR_MASK);
        }
        if(op == 0x05 && len == 2)
        {
#if SDEBUG
            if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                printf("[AMIGO Read Buffered Command]\n");
#endif
///TODO we do not support multiple units yet
///FIXME Added unit error
            amigo_check_unit(0xff & *ptr++);
            AMIGOs->state = AMIGO_READ_BUFFERED;
            gpib_enable_PPR(AMIGOp->HEADER.PPR);
            return(status & ERROR_MASK);
        }
    }
    else if (secondary == 0x6C)
    {
        if(op == 0x18 && len == 5)
        {
///  Reference: A48 ..  A50

            uint8_t db;
#if SDEBUG
            if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                printf("[AMIGO Format]\n");
#endif
///TODO we do not support multiple units yet
///FIXME Added unit error
            amigo_check_unit(0xff & *ptr++);
            ++ptr;                                // override not used
            ++ptr;                                // interleave not used
            db = 0xff & *ptr++;
            amigo_format(db);
            return(status & ERROR_MASK);
        }
    }
    else if (secondary == 0x70)                   // HP-300 Clear
    {
///  Reference: A23
        ++ptr;                                    // Dummy byte
        return(status & ERROR_MASK);
    }
    return ( amigo_todo_op(secondary, op, len) );
}


/// @brief  Amigo Execute command processing
///
/// @param[in] secondary: command
///
/// @return  0 on sucess
/// @return or GPIB error flags on fail
/// @see  gpib.h ERROR_MASK defines for a full list)

int Amigo_Execute( int secondary )
{
#if SDEBUG
    if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
        printf("[AMIGO Execute(%02XH): listen:%02XH, talk:%02XH]\n",
            secondary, listening, talking);
#endif

    if(talking == UNT)
        return(0);

    if(!AMIGO_is_MTA(talking) && !AMIGO_is_MLA(listening))
        return(0);

    if(secondary != 0x60 && secondary != 0x68)
        return(0);

    gpib_disable_PPR(AMIGOp->HEADER.PPR);

    if(secondary == 0x60)
    {
        switch(AMIGOs->state)
        {
            case AMIGO_IDLE:
                return(0);
            case AMIGO_COLD_LOAD_READ:
#if SDEBUG
                if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                    printf("[AMIGO Execute Cold Load Read]\n");
#endif
                return ( amigo_buffered_read() );
            case AMIGO_READ_UNBUFFERED:
#if SDEBUG
                if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                    printf("[AMIGO Execute Read Unbuffered]\n");
#endif
                return ( amigo_buffered_read() );
            case AMIGO_READ_BUFFERED:
#if SDEBUG
                if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                    printf("[AMIGO Execute Read Buffered]\n");
#endif
                return ( amigo_buffered_read() );
            case AMIGO_WRITE_UNBUFFERED:
#if SDEBUG
                if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                    printf("[AMIGO Execute Write Unbuffered]\n");
#endif
                return ( amigo_buffered_write() );
            case AMIGO_INITIALIZE:
#if SDEBUG
                if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                    printf("[AMIGO Execute Initialize]\n");
#endif
                return ( amigo_buffered_write() );
            case AMIGO_WRITE_BUFFERED:
#if SDEBUG
                if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                    printf("[AMIGO Execute Write Buffered]\n");
#endif
                return ( amigo_buffered_write() );
            default:
                return ( amigo_todo(secondary) );
        }
        AMIGOs->state = AMIGO_IDLE;
    }
    if(secondary == 0x68)
    {
        switch(AMIGOs->state)
        {
            case AMIGO_IDLE:
                return(0);
            case AMIGO_REQUEST_STATUS_BUFFERED:
#if SDEBUG
                if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                    printf("[AMIGO Execute Request Status Buffered]\n");
#endif
                return ( amigo_send_status() );
            case AMIGO_REQUEST_STATUS_UNBUFFERED:
#if SDEBUG
                if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                    printf("[AMIGO Exicute Request Status Unbuffered]\n");
#endif
                return ( amigo_send_status() );
            case AMIGO_REQUEST_LOGICAL_ADDRESS:
#if SDEBUG
                if(debuglevel & GPIB_DEVICE_STATE_MESSAGES)
                    printf("[AMIGO Execute Request Logical Address]\n");
#endif
                return ( amigo_send_logical_address() );
            default:
                return ( amigo_todo(secondary) );
        }
        AMIGOs->state = AMIGO_IDLE;
    }
    return(0);
}


/// @brief AMIGO Command state processing.
///
/// - Perform Command, Execute or Reporting Phase functions.
///   - Amigo_Command().
///   - Amigo_Execute().
///   - amigo_cmd_dsj().
///
/// @param[in] ch: command.
///
/// @return  0 on sucess
/// @return or GPIB error flags on fail
/// @see  gpib.h ERROR_MASK defines for a full list.

int AMIGO_COMMANDS(uint8_t ch)
{

    if(AMIGO_is_MTA(talking) || AMIGO_is_MLA(listening))
    {

        if(talking == UNT && AMIGO_is_MLA(listening))
        {
// printf("AMIGO COMMANDS %02XH NO TALK ADDRESS!\n", ch);
        }
        if(listening == 0 && AMIGO_is_MTA(talking))
        {
// printf("AMIGO COMMANDS %02XH NO LISTEN ADDRESS!\n", ch);
        }

        if(ch == 0x60 && (AMIGO_is_MTA(talking) || AMIGO_is_MLA(listening)) )
        {
            return (Amigo_Execute(ch) );
        }

        if(ch == 0x68 && AMIGO_is_MTA(talking) )
        {
            return (Amigo_Execute(ch) );
        }

        if(ch == 0x68 && AMIGO_is_MLA(listening) )// Single byte command
        {
            return (Amigo_Command(ch) );
        }
        if(ch == 0x69 && AMIGO_is_MLA(listening) )// Single byte command
        {
            return (Amigo_Command(ch) );
        }
        if(ch == 0x6a && AMIGO_is_MLA(listening) )// Single byte command
        {
            return (Amigo_Command(ch) );
        }
        if(ch == 0x6c && AMIGO_is_MLA(listening) )// Single byte command
        {
            return (Amigo_Command(ch) );
        }
        if(ch == 0x70 && AMIGO_is_MTA(talking))
        {
            gpib_disable_PPR(AMIGOp->HEADER.PPR);
            return( amigo_cmd_dsj() );
        }
        if(ch == 0x7e && ch == 0x7f)
        {
            return (Amigo_Command(ch) );
        }
        if(ch == 0x70 && AMIGO_is_MLA(listening))
        {
// NOP
        }
    }
    return(0);
}
#endif                                            //ifdef AMIGO
