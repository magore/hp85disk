/// @brief  Initialize GPIB Bus control lines for READ
/// Despite the name this everywhere to setup read mode
/// - Set busy = 0 after powerup everying floating or high
///  - If busy = 1 NRFD/NDAC are set to busy low
/// - References:
///  - HP-IB Tutorial
///  -  HP-IB pg 8-11
/// @return  void
// This function sets the following states
// GPIB_BUS_LATCH_WR(0xff); // float OC BUS on write
// GPIB_IO_LOW(PE);			// BUS OC PULLUP
// BUS IN, DAV IN, NDAC OUT, NRFD OUT
// ATN IN, EOI IN, SRQ OUT OC
void gpib_bus_read_init(int busy)
{

	uint8_t sreg;

// Switch GPIB Control and Data BUS to READ mode
// IN such a manner to as avoid CPU and GPIB transceiver direction conflicts
// Avoid setting any unintended GPIB BUS/Control LOW while switching

	sreg = SREG;
	cli();

// Release IFC and REN
    GPIB_PIN_FLOAT_UP(IFC);                       // IFC FLOAT PULLUP
    GPIB_PIN_FLOAT_UP(REN);                       // REN FLOAT PULLUP

	// Avoid CPU to SN75160B direction conflicts when swtching direction
    GPIB_BUS_IN();                                // CPU data direction IN
    GPIB_BUS_LATCH_WR(0xff);                      // Float BUS pins HIGH

#if BOARD == 2
    // IF we are NOT a System Controller - SC - the set LOW
    // SC LOW = REN IN, IFC IN, LOW ALWAYS for a Device
	// SN75160B
    GPIB_IO_LOW(SC);
	// Enable OC GPIB BUS transceivers
	// If we will be READING  this is harmless
	// If we were WRITTING - 0xff and OC drivers float the bus
    GPIB_IO_LOW(PE);  // OC BUS tristate
#endif

    GPIB_PIN_FLOAT_UP(DAV);                       // DAV FLOAT PULLUP
    GPIB_PIN_FLOAT_UP(EOI);                       // EOI FLOAT PULLUP

    GPIB_PIN_FLOAT_UP(ATN);                       // ATN FLOAT PULLUP
    GPIB_PIN_FLOAT_UP(SRQ);                       // SRQ FLOAT PULLUP

///@brief release handshake lines
///  References:
///   HP-IB Tutorial pg 12,13
///     HP-IB pg 8-11
    if(!busy)
    {
#if BOARD == 2
        GPIB_PIN_FLOAT_UP(NRFD);                  // OC PULLUP
        GPIB_PIN_FLOAT_UP(NDAC);                  // OC PULLUP
#endif
    }
#if BOARD == 2
		GPIB_IO_HI(DC);                           // ATN IN, EOI OUT, SRQ OUT OC
		GPIB_IO_LOW(TE);                          // BUS IN, DAV IN, NDAC OUT, NRFD OUT
#endif
	if(busy)
	{
        GPIB_IO_LOW(NDAC);
        GPIB_IO_LOW(NRFD);

	}

	GPIB_BUS_SETTLE();                            // Let Data BUS settle

	SREG = sreg;

}
