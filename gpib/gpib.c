/**
 @file gpib/gpib.c

 @brief GPIB emulator for HP85 disk emulator project for AVR.

 @par Edit History - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

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

#include "fatfs.h"
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


/// @brief GPIB command mapping to printable strings
typedef struct {
    int cmd;
    char *name;
} gpib_token_t;

gpib_token_t gpib_tokens[] =
{
	{0x01,"GTL" },
	{0x04,"SDC" },
	{0x05,"PPC" },
	{0x08,"GET" },
	{0x09,"TCT" },
	{0x11,"LLO" },
	{0x14,"DCL" },
	{0x15,"PPU" },
	{0x18,"SPE" },
	{0x19,"SPD" },
	{0x3F,"UNL" },
	{0x5F,"UNT" },
	{-1,NULL }
};


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
    // printf("\n%8ld\n", (long)time);
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
    // printf("%8ld,%d\r", (long)gpib_timer.down_counter, (int)gpib_timer.down_counter_done);
    return(gpib_timer.down_counter_done);
}

/// @brief  Initialize GPIB Bus control lines for READ
/// Dispite the name this is used in both read and write functions to clear the bus
/// - Set busy = 0 after powerup everying floating or high
///  - If busy = 1 NRFD/NDAC are set to busy low
/// - References:
///  - HP-IB Tutorial
///  -  HP-IB pg 8-11
/// @return  void
void gpib_bus_read_init(int busy)
{
	// CPU side GPIB BUS in
    GPIB_BUS_LATCH_WR(0xff);// Float BUS when bus is out
	GPIB_BUS_IN();
    GPIB_BUS_SETTLE();      // Let Data BUS settle

	GPIB_PIN_FLOAT_UP(IFC);	// IFC FLOAT PULLUP
	GPIB_PIN_FLOAT_UP(REN);	// REN FLOAT PULLUP

#ifdef V2BOARD
    GPIB_IO_LOW(SC);   	// REN IN, IFC IN, SC = 0 ALWAYS unless we are a controller
    GPIB_IO_LOW(PE);    	// OC BUS float on write
#endif

	GPIB_PIN_FLOAT_UP(SRQ);	// SRQ FLOAT PULLUP
    GPIB_PIN_FLOAT_UP(EOI); // EOI FLOAT PULLUP
	GPIB_PIN_FLOAT_UP(DAV);	// DAV FLOAT PULLUP
	GPIB_PIN_FLOAT_UP(ATN);	// ATN FLOAT PULLUP

    GPIB_BUS_SETTLE();      // Let Data BUS settle


	if(busy)
	{
        GPIB_IO_LOW(NDAC);
        GPIB_IO_LOW(NRFD);
	}
	else
	{
///  References:
///   HP-IB Tutorial pg 12,13
///     HP-IB pg 8-11
        GPIB_PIN_FLOAT_UP(NRFD);	// OC PULLUP
        GPIB_PIN_FLOAT_UP(NDAC);	// OC PULLUP
	}
#ifdef V2BOARD
    GPIB_IO_LOW(TE);		// BUS IN, DAV IN, NDAC OUT, NRFD OUT
    GPIB_IO_HI(DC);			// ATN IN, EOI IN, SRQ OUT OC
#endif
    GPIB_BUS_SETTLE();      // Let Data BUS settle
}

/// @brief  Initialize GPIB Bus control lines for powerup or reset conditions
/// - Set busy = 0 at powerup
/// - set busy = 1 in operation to reset bus state
///  - If busy = 1 NRFD/NDAC is set to 0 busy
/// - References:
///  - HP-IB Tutorial
///  -  HP-IB pg 8-11
/// @return  void
void gpib_bus_init( int busy )
{
	// Basically the BUS init is the same as going into read mode
	// However the sate of NDAC and NRFD are HI (first startup) or LOW (low when on the BUS)
	// depending
    gpib_unread_f = 0;

	// WE ASUME the JTAG fuse has been disabled!!!!	
	// JTAG normally takes over Port C PIN 2 to 5
	// JTAG fuse MUST be disabled to use these bits for GPIO!!!
	gpib_bus_read_init(!busy);

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
void gpib_enable_PPR(int bit)
{
    if(bit < 0 || bit > 7)
    {
        printf("gpib_enable_PPR: bit %d out of range\n", (int) bit);
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

void gpib_disable_PPR(int bit)
{
    if(bit < 0 || bit > 7)
    {
        printf("gpib_disable_PPR: bit %d out of range\n", (int) bit);
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
///     - SS80 pg 3-4, section 3-3
/// @see gpib_enable_PPR()
/// @see gpib_disable_PPR()
/// @return  1 if we see a PPR, 0 if not

uint8_t gpib_detect_PP()
{
    uint8_t ddr, pins;

    if(GPIB_PIN_TST(ATN) == 0 && GPIB_PIN_TST(EOI) == 0 )
    {
#if SDEBUG
        //gpib_timer_elapsed_begin();
        ///@brief Bus pin states
        pins = GPIB_PPR_RD();
        ///@brief debugging - ddr bits should be 0xff
        ddr = GPIB_PPR_DDR_RD();
        if(debuglevel & (0x200))
            printf("[PPR:%02XH, PIN:%02XH, DDR:%02XH]\n",
                ppr_reg(), 0xff & pins, 0xff & ddr );
#endif
//FIXME do we want to wait for this state to end ?
#if 1
        while(GPIB_PIN_TST(ATN) == 0 && GPIB_PIN_TST(EOI) == 0 )
        {
            if(uart_keyhit(0))
                break;
            if(GPIB_PIN_TST(IFC) == 0)
                break;
        }
#endif
#if SDEBUG
        //gpib_timer_elapsed_end("PP released");
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

    GPIB_PIN_FLOAT_UP(IFC);
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
        GPIB_PIN_FLOAT_UP(REN);
    }
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


/// @brief Read GPIB data BUS only
/// @return  bus (lower 8 bits)
uint8_t gpib_bus_read()
{
    uint8_t bus = ~(GPIB_BUS_RD());

    ///@brief if a command byte (ATN low) then strip partity
    if(!GPIB_PIN_TST(ATN))
        bus &= 0x7f;
    else
        bus &= 0xff;
    return(bus);
}


/// @brief  Read GPIB control lines only
/// @return  control lines (upper 8 bits)
/// V2 boards can NOT read all bits on the control bus at once
/// @see gpib_bus_read_init()
uint16_t gpib_control_pin_read()
{
    uint16_t control = 0;
    if(GPIB_PIN_TST(ATN) == 0 )
        control |= ATN_FLAG;
    if(GPIB_PIN_TST(EOI) == 0 )
        control |= EOI_FLAG;
    if(GPIB_PIN_TST(SRQ) == 0 )
        control |= SRQ_FLAG;
    if(GPIB_PIN_TST(REN) == 0 )
        control |= REN_FLAG;
    if(GPIB_PIN_TST(IFC) == 0 )
        control |= IFC_FLAG;
    return(control);
}


/// @brief  Read GPIB handshake lines only
/// @return  handshake lines (upper 8 bits)
uint16_t gpib_handshake_pin_read()
{
    uint16_t control = 0;
    ///@brief for tracing we can reuse the error flag bit values for DAV,NRFD and NDAC
    /// FYI: This has no impact on the gpib_read_byte() functions and return values
    if(GPIB_PIN_TST(DAV) == 0 )
        control |= DAV_FLAG;
    if(GPIB_PIN_TST(NRFD) == 0 )
        control |= NRFD_FLAG;
    if(GPIB_PIN_TST(NDAC) == 0 )
        control |= NDAC_FLAG;
    return(control);
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

	// Remember we start but waiting for DAV to be released, so read state but not busy
	gpib_bus_read_init(0);

	// Bus read init sets the following states
	// GPIB_BUS_LATCH_WR(0xff); // float OC BUS on write
	// GPIB_IO_LOW(PE);			// BUS OC PULLUP

    GPIB_BUS_SETTLE();  	// Let Data BUS settle

    tx_state = GPIB_TX_START;
    gpib_timeout_set(HTIMEOUT);

    while(tx_state != GPIB_TX_DONE )
    {
        if(uart_keyhit(0))
            break;

#if 0
		// FYI - this is disabled as it breaks write
        if(gpib_detect_PP())
            ch |= PP_FLAG;
#endif

        if(GPIB_PIN_TST(IFC) == 0)
        {
            ch |= IFC_FLAG;
            break;
        }

        switch(tx_state)
        {
            case GPIB_TX_START:
                // Wait for release of DAV and EOI before starting
				// IF DAV = 0 the bus is busy
                if(GPIB_PIN_TST(DAV) == 1)
                {
#ifdef V2BOARD
                    // BUS OUT OC, DAV OUT, EOI OUT, ATN OUT
					// NDAC IN, NRDF IN, SRQ IN
					// PE is LOW
					GPIB_IO_HI(TE);			// BUS OUT, DAV OUT, NRFD and NDAC IN
					GPIB_IO_LOW(DC);		// ATN OUT, EOI OUT, SRQ IN
#endif
                    // My testing with various GPIB devices shows that we MUST assert ATN EARLY!
                    if(ch & ATN_FLAG)
                        GPIB_IO_LOW(ATN);   // FYI: SS80 never sends ATN from a device
                    else
                        GPIB_IO_HI(ATN);

                    gpib_timeout_set(HTIMEOUT);
                    tx_state = GPIB_TX_WAIT_READY;
                }
                if (gpib_timeout_test())
                {
#if SDEBUG
                    if(debuglevel & (1+4))
                        printf("<BUS waiting for DAV==1>\n");
#endif
                    ch |= TIMEOUT_FLAG;
                    tx_state = GPIB_TX_ERROR;
                    break;
                }
                break;

            case GPIB_TX_WAIT_READY:
                // Wait for ready condition
                if(GPIB_PIN_TST(NRFD) == 1 && GPIB_PIN_TST(NDAC) == 0)
                {
                    gpib_timeout_set(HTIMEOUT);
                    tx_state = GPIB_TX_PUT_DATA;
                }
                if (gpib_timeout_test())
                {
#if SDEBUG
                    if(debuglevel & (1+4))
                        printf("<BUS waiting for NRFD==1 && NDAC == 0>\n");
#endif
                    ch |= TIMEOUT_FLAG;
                    tx_state = GPIB_TX_ERROR;
                    break;
                }
                break;

            case GPIB_TX_PUT_DATA:

                if(ch & EOI_FLAG)
                    GPIB_IO_LOW(EOI);
                else
                    GPIB_IO_HI(EOI);

                GPIB_BUS_WR((ch & 0xff) ^ 0xff);  // Write Data inverted
#ifdef V2BOARD
                GPIB_IO_HI(PE);  // tristate mode to speed write
#endif
                GPIB_BUS_SETTLE();                // Let Data BUS settle

                gpib_timeout_set(HTIMEOUT);
                tx_state = GPIB_TX_SET_DAV_LOW;
                break;


            case GPIB_TX_SET_DAV_LOW:
                GPIB_IO_LOW(DAV);
                gpib_timeout_set(HTIMEOUT);
                tx_state = GPIB_TX_WAIT_FOR_NRFD_LOW;
                break;

            ///@brief first device is ready
            case GPIB_TX_WAIT_FOR_NRFD_LOW:
                if (GPIB_PIN_TST(NRFD) == 0)
                {
                    gpib_timeout_set(HTIMEOUT);
                    tx_state = GPIB_TX_WAIT_FOR_NDAC_HI;
                    break;
                }
                if (gpib_timeout_test())
                {
                    ch |= TIMEOUT_FLAG;
                    tx_state = GPIB_TX_ERROR;
#if SDEBUG
                    if(debuglevel & (1+4))
                        printf("<BUS waiting for NRFD==0>\n");
#endif
                    break;
                }
                break;

            ///@brief ALL devices are ready
            case GPIB_TX_WAIT_FOR_NDAC_HI:
                if(GPIB_PIN_TST(NDAC) == 1)         // Byte byte accepted
                {
                    tx_state = GPIB_TX_SET_DAV_HI;
                    break;
                }
                if (gpib_timeout_test())
                {
                    ch |= TIMEOUT_FLAG;
                    tx_state = GPIB_TX_ERROR;
#if SDEBUG
                    if(debuglevel & (1+4))
                        printf("<BUS waiting for NDAC==1>\n");
#endif
                    break;
                }
                break;

            ///@release BUS
            case GPIB_TX_SET_DAV_HI:

                GPIB_IO_HI(DAV);
				GPIB_BUS_SETTLE();

				gpib_bus_read_init(0);		// Free BUS

                gpib_timeout_set(HTIMEOUT);
                tx_state = GPIB_TX_WAIT_FOR_DAV_HI;
                break;

            ///@wait for DAV to finish float HI - finishing floating HI
            case GPIB_TX_WAIT_FOR_DAV_HI:
                if(GPIO_PIN_TST(DAV) == 1)
                {
                    tx_state = GPIB_TX_FINISH;
                    break;
                }
                if (gpib_timeout_test())
                {
                    ch |= TIMEOUT_FLAG;
                    tx_state = GPIB_TX_ERROR;
#if SDEBUG
                    if(debuglevel & (1+4))
                        printf("<BUS waiting for DAV==1>\n");
#endif

                }
                break;

            case GPIB_TX_FINISH:
                tx_state = GPIB_TX_DONE;
                break;

            case GPIB_TX_ERROR:
#if SDEBUG
                if(debuglevel & (1+4))
                    printf("<NRFD=%d,NDAV=%d>\n", GPIB_PIN_TST(NRFD),GPIB_PIN_TST(NDAC));
#endif
				gpib_bus_read_init(1);		// Free BUS
                tx_state = GPIB_TX_DONE;
                break;

            case GPIB_TX_DONE:
                break;
        }
    }
    return(ch);
}



/// @brief  read 1 byte and control line status from GPIB BUS
/// @param[in] trace: if non-zero do full bus handshake trace of read
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
///
/// @return (data bus, control and error status)
uint16_t gpib_read_byte(int trace)
{
    uint8_t rx_state;
    uint16_t ch;
    uint16_t bus, control, control_last;
    extern uint8_t gpib_unread_f;
    extern uint16_t gpib_unread_data;

    ch = 0;
    control_last = 0;

    // If we have an unread state it has already been traced!
    if(gpib_unread_f)
    {
        gpib_unread_f = 0;
        return(gpib_unread_data);
    }

	gpib_bus_read_init(1); // Busy


///@brief V2 boards can NOT read all bits on the control bus at once
///@brief NRFD,NDAC SRQ are outputs
    if(trace)
    {
        control_last = gpib_control_pin_read();
        control_last |= gpib_handshake_pin_read();
        gpib_trace_display(control_last, TRACE_BUS);
    }

    rx_state = GPIB_RX_START;
    while(rx_state != GPIB_RX_DONE)
    {
        if(uart_keyhit(0))
            break;

#ifndef V2BOARD
        if(gpib_detect_PP())
            ch |= PP_FLAG;
#endif

        if(GPIB_PIN_TST(IFC) == 0)
        {
            ch |= IFC_FLAG;
            break;
        }


        switch(rx_state)
        {
            case GPIB_RX_START:
                ///@brief Signal that we are ready to ready
#ifdef V2BOARD
                GPIB_IO_HI(NRFD);
#else
                GPIB_PIN_FLOAT_UP(NRFD);
#endif
                GPIB_BUS_SETTLE();                // Let Data BUS settle
                rx_state = GPIB_RX_WAIT_FOR_DAV_LOW;
                break;

            case GPIB_RX_WAIT_FOR_DAV_LOW:
                ///@brief Wait for Ready acknowledge
                if ( GPIB_PIN_TST(DAV) == 0 )
                    rx_state = GPIB_RX_DAV_IS_LOW;
                break;

// Accept Data
            case GPIB_RX_DAV_IS_LOW:
                GPIB_IO_LOW(NRFD);                // BUSY

                ///@brief gpib_bus_read strips command parity if ATN is low at read time
                bus = gpib_bus_read();
                ch |= bus;
///@brief V2 boards can NOT read all bits on the control bus at once
///@brief NRFD,NDAC SRQ are outputs
                control_last = gpib_control_pin_read();
                ch |= control_last;

                if(trace)
                {
///@brief V2 boards can NOT read all bits on the control bus at once
///@brief NRFD,NDAC SRQ are outputs
                    control_last |= gpib_handshake_pin_read();
                    gpib_trace_display(bus | control_last, TRACE_READ);
                }

#ifdef V2BOARD
                GPIB_IO_HI(NDAC);
#else
                GPIB_PIN_FLOAT_UP(NDAC);
#endif
                GPIB_BUS_SETTLE();                // Let Data BUS settle
                gpib_timeout_set(HTIMEOUT);
#ifdef V2BOARD
                rx_state = GPIB_RX_WAIT_FOR_NDAC_HI;
#else
                rx_state = GPIB_RX_WAIT_FOR_DAV_HI;
#endif
                break;

///@brief V1 boards can actually wait NDAC to go HI
///@brief V2 boards we can't - not a big deal as the DAV test works anyway
            ///@brief Wait for NDAC float HI
            case GPIB_RX_WAIT_FOR_NDAC_HI:
                if (GPIB_PIN_TST(NDAC) == 1)
                {
                    gpib_timeout_set(HTIMEOUT);
                    rx_state = GPIB_RX_WAIT_FOR_DAV_HI;
                }
                if (gpib_timeout_test())
                {
                    ch |= TIMEOUT_FLAG;
                    rx_state = GPIB_RX_ERROR;
                }
                break;

            ///@brief Wait for DAV HI
            case GPIB_RX_WAIT_FOR_DAV_HI:
                if (GPIB_PIN_TST(DAV) == 1)
				{
                    rx_state = GPIB_RX_DAV_IS_HI;
				}
                if (gpib_timeout_test())
                {
                    ch |= TIMEOUT_FLAG;
                    rx_state = GPIB_RX_ERROR;
                }
                break;

            ///@brief Ready for next byte
            case GPIB_RX_DAV_IS_HI:
                GPIB_IO_LOW(NDAC);
///FIXME drppping NRFD now may be a mistake
                GPIB_IO_LOW(NRFD);
                rx_state = GPIB_RX_FINISH;        // DONE
                break;

            case GPIB_RX_FINISH:
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

        if(trace)
        {
/// V2 boards can NOT read all bits on the control bus at once
/// NRFD,NDAC SRQ are outputs
            control = gpib_control_pin_read();
            control |= gpib_handshake_pin_read();
            if(control_last != control)
            {
                gpib_trace_display(control, TRACE_BUS);
                control_last = control;
            }
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
///@param[in] fo: FILE pointer or "stdout"
void gpib_decode_header( FILE *fo)
{
    if(fo == NULL)
        fo = stdout;
        
    fprintf(fo,"===========================================\n");
    fprintf(fo,"GPIB bus state\n");
    fprintf(fo,"HH . AESRPITB gpib\n");
    fprintf(fo,"HH = Hex value of Command or Data\n");
    fprintf(fo,"   . = ASCII of XX only for 0x20 .. 0x7e\n");
    fprintf(fo,"     A = ATN\n");
    fprintf(fo,"      E = EOI\n");
    fprintf(fo,"       S = SRQ\n");
    fprintf(fo,"        R = REN\n");
    fprintf(fo,"         I = IFC\n");
    fprintf(fo,"          P = Parallel Poll seen\n");
    fprintf(fo,"           T = TIMEOUT\n");
    fprintf(fo,"            B = BUS_ERROR\n");
    fprintf(fo,"              GPIB commands\n");
}

/// @brief decode/display all control flags and data on the GPIB BUS
/// @param[in] status: data bus value (lower 8 bits) control bus (upper 8 bits)
/// @param[in] trace_state: level of trace detail
/// TRACE_DISABLE = normal bus and control status report from read state only
/// TRACE_READ    = trace  bus and control reporting from read state only
/// TRACE_BUS     = trace  control reporting from all non-read states, data bus values are omiited
/// Note: trace states add DAV,NRFD,NDAC and ommit PPR status, BUS error and timeout 
///       given that gpib_read_byte() report these anyway
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
void gpib_trace_display(uint16_t status,int trace_state)
{
    char str[128];
    char *tmp= str;
    uint8_t bus = status & 0xff;
    extern FILE *gpib_log_fp;

    str[0] = 0;

    // Display data bus ???
    if(trace_state == TRACE_DISABLE || trace_state == TRACE_READ)
    {
        uint8_t printable = ' ';                          // Data
        if( !(status & ATN_FLAG) && (bus >= 0x20 && bus <= 0x7e) )
            printable = bus;
        sprintf(str, "%02X %c ", (int)bus & 0xff, (int)printable);
    }
    else
    {
        sprintf(str, "     ");
    }

    tmp = str + strlen(str);

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

    if(status & IFC_FLAG)
        *tmp++ = 'I';
    else
        *tmp++ = '-';

    if(trace_state == TRACE_DISABLE)
    {
        if(status & PP_FLAG)
            *tmp++ = 'P';
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
    }
    else
    {
        // not used when tracing
        *tmp++ = '-';
        *tmp++ = '-';
        *tmp++ = '-';
    }
    *tmp = 0;

    if(trace_state == TRACE_READ || trace_state == TRACE_BUS)
    {
        if(status & DAV_FLAG)
            strcat(str,"  DAV");
        else
            strcat(str,"     ");
        if(status & NRFD_FLAG)
            strcat(str," NRFD");
        else
            strcat(str,"     ");
        if(status & NDAC_FLAG)
            strcat(str," NDAC");
        else
            strcat(str,"     ");
    }

    if( (status & ATN_FLAG) )
    {
        int i;
        int cmd = status & CMD_MASK;
        if(cmd >= 0x020 && cmd <= 0x3e)
            sprintf(tmp," MLA %02Xh", cmd & 0x1f);
        else if(cmd >= 0x040 && cmd <= 0x4e)
            sprintf(tmp," MTA %02Xh", cmd & 0x1f);
        else if(cmd >= 0x060 && cmd <= 0x6f)
            sprintf(tmp," MSA %02Xh", cmd & 0x1f);
        else
        {
            for(i=0;gpib_tokens[i].cmd != -1;++i)
            {
                if(cmd == gpib_tokens[i].cmd)
                {
                    strcat(tmp," ");
                    strcat(tmp,gpib_tokens[i].name);
                    break;
                }
            }
        }
    }

    if(gpib_log_fp == NULL)
        gpib_log_fp = stdout;

    // Echo to console unless file is the console
    if(gpib_log_fp != stdout)
        puts(str);

    // Save to file
    fprintf(gpib_log_fp,"%s\n",str);
}


/// @brief  Calls gpib_decode_str() and dosplays the result.
///
/// Display: decode/display all control flags and data on GPIB BUS.
/// @see gpib_decode_str()
/// @return  void

void gpib_decode(uint16_t ch)
{
    gpib_trace_display(ch,0);
}

/// @brief  Read string from GPIB BUS - controlled by status flags.
///
/// - Status flags used when reading
///   - If EOI is set then EOI detection will end reading when EOI is detected.
///   - If an early or unexpected EOI is detected a warning is displayed.
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
        val = gpib_read_byte(NO_TRACE);
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
