/**
 @file gpib/gpib.c

 @brief GPIB emulator for HP85 disk emulator project for AVR.

 @par Edit History - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.

 @par Based on work by Anders Gustafsson.

 @par Copyright &copy; 2014 Anders Gustafsson All rights reserved..


*/

#include <avr/pgmspace.h>
#include <avr/portpins.h>
#include <avr/io.h>

#include "user_config.h"
#include "hardware/hal.h"
#include "gpib_hal.h"

#include "defines.h"
#include "gpib.h"
#include "gpib_task.h"
#include "amigo.h"
#include "ss80.h"

#include "posix.h"


/// - References: Documenation and related sources of information
///  - Web Resources:
///    - http://www.hp9845.net
///    - http://www.hpmuseum.net
///    - http://www.hpmusuem.org
///    - http://bitsavers.trailing-edge.com
///    - http://en.wikipedia.org/wiki/IEEE-488
/// 
/// - SS80 References: 
///   - "SS80" is the short form used in the project
///   - "Subset 80 from Fixed and flexible disc drives"
///   - Printed November, 1985
///   - HP Part# 5958-4129
/// 
/// - CS80 References: 
///   - "CS80" is the short form used in the project
///   - "CS/80 Instruction Set Programming Manual"
///   - Printed: APR 1983
///   - HP Part# 5955-3442
/// 
/// - Amigo References: 
///   - "Amigo" is the short form used in the project
///   - "Appendix A of 9895A Flexible Disc Memory Service Manual"
///   - HP Part# 09895-90030
/// 
/// - HP-IB
///   - "HP-IB" is the short form used in the project
///   - "Condensed Description of the Hewlett Packard Interface Bus"
///   - Printed March 1975
///   - HP Part# 59401-90030
/// 
/// - Tutorial Description of The Hewllet Packard Interface Bus"
///   - "HP-IB Tutorial" is the short form used in the project
///   - http://www.hpmemory.org/an/pdf/hp-ib_tutorial_1980.pdf
///     - Printed January 1983
///   - http://www.ko4bb.com/Manuals/HP_Agilent/HPIB_tutorial_HP.pdf
///     - Printed 1987
/// 
/// - GPIB / IEEE 488 Tutorial by Ian Poole
///   - http://www.radio-electronics.com/info/t_and_m/gpib/ieee488-basics-tutorial.php

/// @brief This is the default BUS timeout of 0.5 Seconds in Microseconds
#define HTIMEOUT (500000L / GPIB_TASK_TIC_US)

/// @brief common IO buffer for  gpib_read_str() and gpib_write_str()
uint8_t gpib_iobuff[GPIB_IOBUFF_LEN];

/// @brief gpib_unread() flag
uint8_t gpib_unread_f = 0;                        // saved character flag
/// @brief gpib_unread() data
uint16_t gpib_unread_data;                        // saved character and status

/// @brief gpib talk address
uint8_t talking;
/// @brief gpib last talk address
uint8_t talking_last;

/// @brief gpib listen address
uint8_t listening;
/// @brief gpib last listen address
uint8_t listening_last;

/// @brief gpib serial poll status
uint8_t spoll;
/// @brief gpib current and last command
uint16_t lastcmd,current;

/// @brief gpib secondary
uint8_t secondary;

///  See gpib_hal.c for CPU specific code
///  Provide Elapsed timer and Timeout timer - both in Microseconds


/// @brief  Start measuring time - used with hpib_timer_elapsed_end()
///
///  We use the lower level clock elapsed timer functions here
/// @return  void
void gpib_timer_elapsed_begin( void )
{

#ifdef SYSTEM_ELAPSED_TIMER
    clock_elapsed_begin();
#else
    cli();
    gpib_timer.elapsed = 0;
    sei();
#endif
}


/// @brief  Reset elapsed and timeout timers
///    Elapses and Timeout Timers
/// @return  void

void gpib_timer_reset( void )
{
    cli();
    gpib_timer.elapsed = 0;
    gpib_timer.down_counter = 0;
    gpib_timer.down_counter_done = 1;
    sei();
}


/// @brief Display user message and time delta since gpib_timer_elapsed_begin() call
///
///   Display a message
/// @return  void
void gpib_timer_elapsed_end( char *msg)
{
#ifdef SYSTEM_ELAPSED_TIMER
    clock_elapsed_end( msg );
#else
    uint32_t val;
    uint32_t seconds;
    uint32_t useconds;

    cli();
    val  = gpib_timer.elapsed;
    sei();

    seconds = val * GPIB_TASK_TIC_US / 1000000L;
    useconds = (val * GPIB_TASK_TIC_US ) % 1000000L;

    printf("[%s: %ld.%06ld]\n", msg, seconds,useconds);
#endif
}


/// @brief  Main GPIB timer task called by low level interrup hander
///
/// - Provides Down timers and Elapsed timers
/// - Called every GPIB_TASK_TIC_US Micro Seconds
/// @return  void
void gpib_timer_task()
{
    cli();
#ifndef SYSTEM_ELAPSED_TIMER
    gpib_timer.elapsed++;
#endif
    if(gpib_timer.down_counter)
        gpib_timer.down_counter--;
    else
        gpib_timer.down_counter_done = 1;
    sei();
}


/// @brief  Set GPIB timeout timer in units of GPIB_TASK_TIC_US
/// @see: gpib_timeout_test()
/// @return  void
void gpib_timeout_set(uint32_t time)
{
    cli();
    gpib_timer.down_counter = time;
    gpib_timer.down_counter_done = 0;
    sei();
}


/// @brief  Test GPIB timeout timer for timeout condition
/// @see: gpib_timeout_set()
/// @return  1 if timeout, 0 if not
uint8_t gpib_timeout_test()
{
    return(gpib_timer.down_counter_done);
}


/// @brief  Initialize GPIB Bus control lines for powerup or reset conditions
/// - Set cold = 1 at powerup
/// - set cold = 0 in operation to reset bus state
///  - If cold = 0 NRFD/NDAC is setto 0 busy
/// - References:
///  - HP-IB Tutorial
///  -  HP-IB pg 8-11
/// @return  void
void gpib_bus_init( int cold )
{
    gpib_unread_f = 0;

    GPIB_BUS_IN();

    GPIB_PIN_FLOAT(EOI);
    GPIB_PIN_FLOAT(DAV);
    GPIB_PIN_FLOAT(ATN);
    GPIB_PIN_FLOAT(IFC);

    GPIB_PIN_FLOAT(REN);
    GPIB_PIN_FLOAT(SRQ);

    if(cold == 0)
    {
        GPIB_PIN_FLOAT(NRFD);
        GPIB_PIN_FLOAT(NDAC);
    }
    else
    {
///  References:
///   HP-IB Tutorial pg 12,13
/// 	HP-IB pg 8-11
        GPIB_IO_LOW(NDAC);
        GPIB_IO_LOW(NRFD);
    }

    GPIB_BUS_SETTLE();                            // Let Data BUS settle
#if SDEBUG
    if(debuglevel & 4 )
        printf("[GPIB BUS_INIT]\n");
#endif
}


/// @brief  Reset GPIB states and related variables 
///
/// - Called at powerup and IFC or reset states.
/// - Does not reset BUS
/// @return  void
void gpib_state_init( void )
{
#if SDEBUG
    if(debuglevel & 4 )
        printf("[GPIB STATE INIT]\n");
#endif
    ppr_init();

    listen_cleanup();

    talk_cleanup();

    spoll = 0;                                    // SPOLL disabled
    talking = 0;                                  // Listening/Talking State
    talking_last = 0;
    listening = 0;
    listening_last  = 0;
    lastcmd = 0;
    current = 0;
    secondary = 0;
}

///
/// - Reference: SS80 pg 3-4, section 3-3
/// @return  void
void EnablePPR(int bit)
{
	if(bit < 0 || bit > 7)
	{
		printf("EnablePPR: bit %d out of range\n", (int) bit);
		return;
	}
    ppr_bit_set(bit);
#if SDEBUG
    if(debuglevel & 2)
        printf("[EPPR bit:%d, mask:%02XH]\n",0xff & bit , 0xff & ppr_reg());
#endif
}


/// @brief Disable PPR (Parallel Poll Response) for a device
///
/// - Reference: SS80 pg 3-4, section 3-3
/// @return  void

void DisablePPR(int bit)
{
	if(bit < 0 || bit > 7)
	{
		printf("DisablePPR: bit %d out of range\n", (int) bit);
		return;
	}
    ppr_bit_clr(bit);
#if SDEBUG
    if(debuglevel & 2)
        printf("[DPPR bit:%d, mask:%02XH]\n",0xff & bit, 0xff & ppr_reg());
#endif
}


/// @brief  Attempt to detect the Parallel Poll Reposnse state
/// Used only for debugging - it is unlikely that we will catch this state
///  - PPR is short for "Parellel Poll Response"
///  - PPR happens when the CIC (controller in charge) holds 
///     BOTH of ATN == 0 and EOI == 0
///  - PPR Response happens using hardware 
///       The hardware pulls a bit low on the GPIB bus corresponding to the device.
///       But only if any PPR mask bits are set in the hardware
///  - Note: EOI is never used in command mode (ATN = 0)
///        1) Hardware will automatically hold DATA BUS lines low 
///  - ppr_reg() determines if we have PPR enabled.
///  - References:
///  	- SS80 pg 3-4, section 3-3
/// @see EnablePPR()
/// @see DisablePPR()
/// @return  1 if we see a PPR, 0 if not

uint8_t Parallel_Poll()
{
    uint8_t ddr, pins;

    if(GPIB_IO_RD(ATN) == 0 && GPIB_IO_RD(EOI) == 0 )
    {
		///@brief Bus pin states
        pins = GPIB_PPR_RD();
		///@brief debugging - ddr bits should be 0xff
        ddr = GPIB_PPR_DDR_RD();
#if SDEBUG
        if(debuglevel & (2+512))
            printf("[PPR:%02XH, PIN:%02XH, DDR:%02XH]\n",
                ppr_reg(), 0xff & pins, 0xff & ddr );
#endif
//FIXME do we want to wait for this state to end ?
#if 0
		while(GPIB_IO_RD(ATN) == 0 && GPIB_IO_RD(EOI) == 0 )
		{
			if(GPIB_IO_RD(IFC) == 0)
				break;
		}
#endif
        return(1);
    }
    return(0);
}


/// @brief  Assert Interface clear for 250us
///
/// - 100us is the Minumum
/// - Reference: SS80 section 3-15, pg 3-26
/// @return  void

void gpib_assert_ifc(void)
{
    GPIB_IO_LOW(IFC);
    delayus(250);

    GPIB_PIN_FLOAT(IFC);
    delayus(250);
#if SDEBUG
    if(debuglevel & 4)
        printf("[IFC SENT]\n");
#endif
}


/// @brief Assert REN to put instrument in remote mode
///
/// - If state != 0 -> assert REN
/// - If state == 0 -> deassert REN
/// @return  void

void gpib_assert_ren(unsigned char state)
{
    if(state)
    {
#if SDEBUG
        if(debuglevel & 4)
            printf("[REN LOW]\n");
#endif
        GPIB_IO_LOW(REN);
    }
    else
    {
#if SDEBUG
        if(debuglevel & 4)
            printf("[REN HI]\n");
#endif
        GPIB_PIN_FLOAT(REN);
    }
}


/// @brief  Send 1 byte and control line states to GPIB BUS
///
/// - We assume we are talking to an active controller already in read state
///   - Any error flags set on return imply the data was not likely sent
///   - You can OR the control flags ATN_FLAG, EOI_FLAG with (ch)
///     to send them these states.
///   - Results can be displayed for debugging with the decode(ch) funnction
///   - We always exit with NRFD and NDAC LOW
/// - References: HP-IB Tutorial pg13.
///   HP-IB Tutorial pg 13 for the receive and send control line states
///   Correction: The send routine MUST also wait for NDAC LOW before exit.
///   (You can verify this by checking the receive part of the handshake
///   diagram just before it returns to the start of the loop)
///   Failing to wait will cause problems that may masqurade as timing issues.
/// - (It caused problems with my HP85 before I added the code)
///
/// @param[out] ch: ( Data or Command ) and control flags
/// - Upper 8 bits: contril flags
/// - Flags:
///  - EOI_FLAG
///  - ATN_FLAG
///
/// @return 
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
uint16_t gpib_write_byte(uint16_t ch)
{
    uint8_t tx_state;

    GPIB_PIN_FLOAT(DAV);
    GPIB_PIN_FLOAT(EOI);
    GPIB_PIN_FLOAT(ATN);                           // FYI: SS80 never sends ATN from a device

    GPIB_PIN_FLOAT(IFC);


    GPIB_BUS_IN();

///  See HP-IB Tutorial pg 13 for the receive and send control line states

    GPIB_PIN_FLOAT(NRFD);
    GPIB_PIN_FLOAT(NDAC);

    GPIB_BUS_SETTLE();                            // Let Data BUS settle

    tx_state = GPIB_TX_START;

    while(tx_state != GPIB_TX_DONE )
    {
        if(uart_keyhit(0))
            break;

        if(Parallel_Poll())
            ch |= PP_FLAG;

        if(GPIB_IO_RD(IFC) == 0)
        {
            ch |= IFC_FLAG;
            break;
        }

        switch(tx_state)
        {
            case GPIB_TX_START:
                if(GPIB_IO_RD(DAV) == 1)
                {
                    gpib_timeout_set(HTIMEOUT);
                    tx_state = GPIB_TX_WAIT_READY;
                    break;
                }
                else
                {
#if 0                             // Skip DAV == 0 test
                    ch |= BUS_ERROR_FLAG;
                    tx_state = GPIB_TX_ERROR;
                    if(debuglevel & (1+4))
                        printf("<BUS DAV>\n");
#endif
                }
                break;

            case GPIB_TX_WAIT_READY:
                if(GPIB_IO_RD(NRFD) == 1)
                {
                    if(GPIB_IO_RD(NDAC) == 0)
                    {
                        tx_state = GPIB_TX_PUT_DATA;
                    }
                    else
                    {
/// @todo FIXME
///  If we test NRFD == 1 and NDAC == 1 and exit rather then waiting
///  we frequently fail. Why does this happen ???
///  Disabling the test and waiting fixes the problem. Why ???

#if 0

						if(debuglevel & (1+4))
                            printf("<BUS NRFD=1,NDAV=1>\n");
                        ch |= BUS_ERROR_FLAG;
                        tx_state = GPIB_TX_ERROR;
#endif
                    }
                    break;
                }
                if (gpib_timeout_test())
                {
#if SDEBUG
                    if(debuglevel & (1+4))
                        printf("<BUS waiting for NRFD==1,NDAV==0>\n");
#endif

                    ch |= TIMEOUT_FLAG;
                    tx_state = GPIB_TX_ERROR;
                    break;
                }
                break;

            case GPIB_TX_PUT_DATA:

/// @todo FIXME
                if(ch & ATN_FLAG)
                {
                    GPIB_IO_LOW(ATN);             // ATN LOW
                }

                if(ch & EOI_FLAG)
                {
                    GPIB_IO_LOW(EOI);             // EOI LOW
                }

                GPIB_BUS_WR((ch & 0xff) ^ 0xff);  // Write Data inverted
                GPIB_BUS_SETTLE();                // Let Data BUS settle

                gpib_timeout_set(HTIMEOUT);
                tx_state = GPIB_TX_WAIT_FOR_NRFD_HI;
                break;

            case GPIB_TX_WAIT_FOR_NRFD_HI:
                if (GPIB_IO_RD(NRFD) == 1)
                {
                    gpib_timeout_set(HTIMEOUT);
                    tx_state = GPIB_TX_SET_DAV_LOW;
                    break;
                }
                if (gpib_timeout_test())
                {
                    ch |= TIMEOUT_FLAG;
                    tx_state = GPIB_TX_ERROR;
                    break;
                }
                break;

            case GPIB_TX_SET_DAV_LOW:
                GPIB_IO_LOW(DAV);
                gpib_timeout_set(HTIMEOUT);
                tx_state = GPIB_TX_WAIT_FOR_NDAC_HI;
                break;

            case GPIB_TX_WAIT_FOR_NDAC_HI:
                if(GPIB_IO_RD(NDAC) == 1)         // Byte byte accepted
                {
                    tx_state = GPIB_TX_SET_DAV_HI;
                    break;
                }
                if (gpib_timeout_test())
                {
                    ch |= TIMEOUT_FLAG;
                    tx_state = GPIB_TX_ERROR;
                    break;
                }
                break;

            case GPIB_TX_SET_DAV_HI:
                GPIB_PIN_FLOAT(DAV);               // Float DAV
                GPIB_BUS_SETTLE();                // Let Data BUS settle
                GPIB_PIN_FLOAT(ATN );              // ATN FLOAT
                GPIB_PIN_FLOAT(EOI);               // EOI FLOAT
                GPIB_BUS_IN();                    // Data FLOAT
                gpib_timeout_set(HTIMEOUT);
                tx_state = GPIB_TX_WAIT_FOR_NDAC_LOW;
                break;

            case GPIB_TX_WAIT_FOR_NDAC_LOW:
#if 1
                if(GPIB_IO_RD(NDAC) == 0)         // Byte byte accepted
                {
                    tx_state = GPIB_TX_FINISH;
                    break;
                }
#else
                tx_state = GPIB_TX_FINISH;
#endif

                if((ch & ATN_FLAG) == 0 && (ch & EOI_FLAG))
                {
                    GPIB_IO_LOW(NRFD);
                }

                if (gpib_timeout_test())
                {
                    ch |= TIMEOUT_FLAG;
                    tx_state = GPIB_TX_ERROR;
                }
                break;

            case GPIB_TX_FINISH:
                tx_state = GPIB_TX_DONE;
                break;

            case GPIB_TX_ERROR:
                GPIB_PIN_FLOAT(DAV);               // DAV FLOAT on error
                GPIB_BUS_SETTLE();                // Let Data BUS settle
                GPIB_PIN_FLOAT(ATN );              // ATN FLOAT
                GPIB_PIN_FLOAT(EOI);               // EOI FLOAT
                GPIB_BUS_IN();                    // Data FLOAT
                GPIB_IO_LOW(NRFD);                // BUSY
                tx_state = GPIB_TX_DONE;
                break;

            case GPIB_TX_DONE:
                break;
        }
    }
    return(ch);
}


/// @brief  GPIB ungets one character and all status states
///
///  Pushes a character back into the read buffer
/// @param[in] ch
///   - Lower 8 bits: Data or Command.
///     - If ATN is LOW then we strip parity from the byte.
///   - Upper 8 bits: Status and Errors present.
///     - @see gpib.h _FLAGS defines for a full list.
/// @return ch
uint16_t gpib_unread(uint16_t ch)
{
    if(!gpib_unread_f)
    {
        gpib_unread_data = ch;
        gpib_unread_f = 1;
    }
    else
    {
		if(debuglevel & (1+4))
            printf("gpib_unread: error, can only be called once!\n");
    }
    return(ch);
}


/// @brief  read 1 byte and control line status from GPIB BUS
///
/// - References: HP-IB Tutorial pg 13, 14.
/// - Notes: Their diagram is a bit misleading - they start in the BUS init phase.
///   The init happens much earlier in other code - we NEVER want to do that
///   here or bad things will happen - we could step on the sender NRFD test
///   and false trigger it. We instead start with NRFD FLOAT, and NDAC LOW.
///   See loop in send diagram it has NDAC == 0 and NRFD == 1 in the return
///   to send (beginning) part of the loop.
/// - The HP-IB reference has an error:
///     Read ready actually starts when NRFD = 1 and NDAC = 0.
///   If you look at the Write state diagram that can be confirmed.
///
/// @return 
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
uint16_t gpib_read_byte( void )
{
    uint16_t val,status,ch;
    uint8_t rx_state;

    ch = 0;
    status = 0;

    GPIB_PIN_FLOAT(IFC);
    GPIB_PIN_FLOAT(ATN);
    GPIB_PIN_FLOAT(EOI);
    GPIB_PIN_FLOAT(DAV);                           // DAV should be HI already


    GPIB_BUS_IN();

    if(gpib_unread_f)
    {
        gpib_unread_f = 0;
        return(gpib_unread_data);
    }

    GPIB_PIN_FLOAT(NRFD);
    GPIB_BUS_SETTLE();                            // Let Data BUS settle
    GPIB_IO_LOW(NDAC);

    rx_state = GPIB_RX_START;

    while(rx_state != GPIB_RX_DONE)
    {
        if(Parallel_Poll())
            ch |= PP_FLAG;

        if(uart_keyhit(0))
            break;
        if(GPIB_IO_RD(IFC) == 0)
        {
            ch |= IFC_FLAG;
            break;
        }

        switch(rx_state)
        {
            case GPIB_RX_START:
                GPIB_PIN_FLOAT(NRFD);
                GPIB_BUS_SETTLE();                // Let Data BUS settle
                rx_state = GPIB_RX_WAIT_FOR_DAV_LOW;
                break;

            case GPIB_RX_WAIT_FOR_DAV_LOW:
                if ( GPIB_IO_RD(DAV) == 0 )
                    rx_state = GPIB_RX_DAV_IS_LOW;
                break;

            case GPIB_RX_DAV_IS_LOW:
                GPIB_IO_LOW(NRFD);                // BUSY

                val = ~(GPIB_BUS_RD());           // Accept databyte
                val &= 0xff;

                status = 0;
                status |= (GPIB_IO_RD(EOI)) ? 0 : EOI_FLAG;
                status |= (GPIB_IO_RD(IFC)) ? 0 : IFC_FLAG;
                status |= (GPIB_IO_RD(ATN)) ? 0 : ATN_FLAG;
                status |= (GPIB_IO_RD(SRQ)) ? 0 : SRQ_FLAG;
                status |= (GPIB_IO_RD(REN)) ? 0 : REN_FLAG;

                if(status & ATN_FLAG)
                    val &= 0x7f;
                else
                    val &= 0xff;

                ch |= (val | status);

                GPIB_PIN_FLOAT(NDAC);              // Acknowledge Read
                GPIB_BUS_SETTLE();                // Let Data BUS settle
                gpib_timeout_set(HTIMEOUT);
                rx_state = GPIB_RX_WAIT_FOR_DAV_HI;
                break;

            case GPIB_RX_WAIT_FOR_DAV_HI:
                if (GPIB_IO_RD(DAV) == 1)
                    rx_state = GPIB_RX_DAV_IS_HI;

                if (gpib_timeout_test())
                {
                    ch |= TIMEOUT_FLAG;
                    rx_state = GPIB_RX_ERROR;
                }
                break;

            case GPIB_RX_DAV_IS_HI:
                GPIB_IO_LOW(NDAC);
                rx_state = GPIB_RX_FINISH;        // DONE
                break;

            case GPIB_RX_FINISH:
                GPIB_IO_LOW(NDAC);
                rx_state = GPIB_RX_DONE;
                break;

            case GPIB_RX_ERROR:
                GPIB_IO_LOW(NDAC);
                GPIB_IO_LOW(NRFD);
                rx_state = GPIB_RX_DONE;
                break;

            case GPIB_RX_DONE:
                break;
        }
    }

///  Note: see: HP-IB Tutorial pg 13
///  - Remember that NDAC and NRFD are now both LOW!
///  - The spec says to KEEP BOTH LOW when NOT ready to read otherwise
///    we may miss a transfer and cause a controller timeout!
///  - GPIB TX state expects NRFD LOW on entry or it is an ERROR!


    lastcmd = current;

    if(ch & ERROR_MASK || (ch & ATN_FLAG) == 0)
        current = 0;
    else
        current = ch & CMD_MASK;

    return (ch);
}


/// @brief  Displays help for gpib_decode() function
///
/// You would call this once at the start of a trace for example.
/// @see: gpib.h _FLAGS defines for a full list is control lines we track
/// @see: gpib_trace()
/// @return void

void gpib_decode_header( void )
{
    sep();
    printf("#GPIB flags\n");
    printf("XX p AESRPITB\n");
    printf("#   XX = Hex value of Command or Data\n");
    printf("#   . = ASCII of XX only for 0x20 .. 0x7e\n");
    printf("#   A = ATN\n");
    printf("#   E = EOI\n");
    printf("#   S = SRQ\n");
    printf("#   R = REN\n");
    printf("#   P = Parallel Poll seen\n");
    printf("#   I = IFC\n");
    printf("#   T = TIMEOUT\n");
    printf("#   B = BUS_ERROR\n");
}


/// @brief decode/display all control flags and data on the GPIB BUS
/// 
/// @param[in] ch
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
/// @param[in] str: string pointer to store the decoded result in.
///
/// @see: gpib_read_byte()
/// @see: gpib_write_byte(int ch)
/// @see: gpib_decode_header()
///
/// @return  void
char *gpib_decode_str(uint16_t ch)
{
    static char str[64];
	char *tmp = str;
    uint16_t status,val;
    uint16_t printable;

/// @return return
    status = ch & STATUS_MASK;
    val = (ch & ATN_FLAG) ? (ch & CMD_MASK) : (ch & DATA_MASK);

    if(status & ATN_FLAG)                         // Command
        printable = ' ';                          // Data
    else if (val >= 0x20 && val <= 0x7e)
        printable = val;
    else printable = ' ';

    sprintf(str, "%02XH %c ", val & 0xff, printable & 0xff);
    tmp = str + 6;

    if(status & ATN_FLAG)
        *tmp++ = 'A';
    else
        *tmp++ = '-';
    if(status & EOI_FLAG)
        *tmp++ = 'E';
    else
        *tmp++ = '-';
    if(status & SRQ_FLAG)
        *tmp++ = 'S';
    else
        *tmp++ = '-';
    if(status & REN_FLAG)
        *tmp++ = 'R';
    else
        *tmp++ = '-';
    if(status & PP_FLAG)
        *tmp++ = 'P';
    else
        *tmp++ = '-';
    if(status & IFC_FLAG)
        *tmp++ = 'I';
    else
        *tmp++ = '-';
    if(status & TIMEOUT_FLAG)
        *tmp++ = 'T';
    else
        *tmp++ = '-';
    if(status & BUS_ERROR_FLAG)
        *tmp++ = 'B';
    else
        *tmp++ = '-';
    *tmp = 0;
	return(str);
}


/// @brief  Calls gpib_decode_str() and dosplays the result.
///
/// Display: decode/display all control flags and data on GPIB BUS.
/// @see gpib_decode_str()
/// @return  void

void gpib_decode(uint16_t ch)
{
	char *ptr;
    extern void gpib_log( char *str );
    ptr = gpib_decode_str(ch);
    gpib_log(ptr);
    puts(ptr);
}


/// @brief  Read string from GPIB BUS - controlled by status flags.
///
/// - Status flags used when reading
///   - If EOI is set then EOI detection will end reading when EOI is detected.
///	  - If an early or unexpected EOI is detected a warning is displayed.
///   - The state of ATN controls the type of data read.
///     - If ATN is set then parity is stripped.
///     - Only data matching the ATN flag state set by user is read.
///       - If we see a mismatch we "unread" it and exit. 
///       - Unreading saves data AND status to be reread later on.
///
/// @see: gpib_read_byte()
/// @see: gpib_unread()
///
/// @param[in] buf: Binary gpib string to read
/// @param[in] size: Size of string we want to read
/// @param[in] status: controls sending modes and returns status
///
/// @return bytes read
/// @return status
///
/// - Errors TIMEOUT_FLAG or IFC_FLAG will cause early exit and set status.
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
int gpib_read_str(uint8_t *buf, int size, uint16_t *status)
{
    uint16_t val;
    int ind = 0;

    *status &= STATUS_MASK;

    if(!size)
    {
        if(debuglevel & (1+4))
            printf("gpib_read_str: size = 0\n");
    }

    while(ind < size)
    {
        val = gpib_read_byte();
#if SDEBUG
        if(debuglevel & 256)
            gpib_decode(val);
#endif
        if(val & ERROR_MASK)
        {
            *status |= (val & ERROR_MASK);
            break;
        }

        if((*status & ATN_FLAG) != (val & ATN_FLAG))
        {
            if(debuglevel & (1+4))
                printf("gpib_read_str(ind:%d): ATN %02XH unexpected\n",ind, 0xff & val);
            gpib_unread(val);
            break;
        }

        if(val & ATN_FLAG)
            buf[ind] = (val & CMD_MASK);
        else
            buf[ind] = (val & DATA_MASK);
        ++ind;

        if(!(val & ATN_FLAG) && (val & EOI_FLAG) )
        {

            if(*status & EOI_FLAG)
                return(ind);
/// @todo TODO
///  decode this state - for now I just set the EOI_FLAG
            *status |= EOI_FLAG;
            break;
        }
    }
    if ( ind != size )
    {
		if(debuglevel & (1+4))
			printf("[gpib_read_str read(%d) expected(%d)]\n", ind , size);
    }
    return(ind);
}


/// @brief  Send string to GPIB BUS - controlled by status flags.
///
/// - Status flags used when sending
///   - If EOI is set then EOI is sent at last character
///
/// @param[in] buf: Binary gpib string to send
/// @param[in] size: Size of string
/// @param[in] status: User status flags that control the sending process.
///
/// @return bytes sent
///  - will match size on success (no other tests needed).
///  - Any size mismatch impiles error flags IFC_FLAG and TIMEOUT_FLAG.
///
/// - Errors TIMEOUT_FLAG or IFC_FLAG will cause early exit and set status.
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
/// @see: gpib_write_byte()
/// @see: gpib.h _FLAGS defines for a full list)
int gpib_write_str(uint8_t *buf, int size, uint16_t *status)
{
    uint16_t val, ch;
    int ind = 0;

    *status &= STATUS_MASK;

    if(!size)
    {
		if(debuglevel & (1+4))
            printf("gpib_write_str: size = 0\n");
    }

    while(ind < size)
    {
        ch = buf[ind++] & 0xff;                   // unsigned

        if(*status & ATN_FLAG)
        {
            ch |= ATN_FLAG;
        }

        if( (*status & EOI_FLAG) && (ind == size ) )
            ch |= EOI_FLAG;

/// @return Returns

        val = gpib_write_byte(ch);
        *status |= (val & ERROR_MASK);

#if SDEBUG
        if(debuglevel & 256)
            gpib_decode(val);
#endif
        if(val & ERROR_MASK)
        {
            break;
        }

    }                                             // while(ind < size)
    if ( ind != size )
    {
		if(debuglevel & (1+4))
            printf("[gpib_write_str sent(%d) expected(%d)]\n", ind,size);
    }
    return(ind);
}
