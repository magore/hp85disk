uint16_t gpib_write_byte(uint16_t ch)
{
    uint8_t tx_state;
	uint8_t sreg;

// Wait for DAV to be released before starting
// Read state for DAV
///@brief NRFD,NDAC SRQ are outputs

	if(GPIB_PIN_TST(TE) != 1 && GPIB_PIN_TST(DC) != 0)
		gpib_bus_read_init(0); 

	// GPIB_BUS_LATCH_WR(0xff); // float OC BUS on write
	// GPIB_IO_LOW(PE);			// BUS OC PULLUP
	// BUS IN, DAV IN, NDAC OUT, NRFD OUT
	// ATN IN, EOI IN, SRQ OUT OC

    tx_state = GPIB_TX_START;
    gpib_timeout_set(HTIMEOUT);

    while(tx_state != GPIB_TX_DONE )
    {
        // User task that is called while waiting for commands
        gpib_user_task();

		if(uart_keyhit(0))
		{
			if(debuglevel & GPIB_ERR)
				printf("gpib_write_byte: KEY state=%d\n", tx_state);
			break;
		}

#if 0
// FIXME - this is disabled as it breaks write
// Try to detect PPR - only for debugging
        if(gpib_detect_PP())
            ch |= PP_FLAG;
#endif

// IFC is always in for a device
        if(GPIB_PIN_TST(IFC) == 0)
        {
            ch |= IFC_FLAG;
			if(debuglevel & GPIB_ERR)
				printf("gpib_write_byte: IFC state=%d\n", tx_state);
            gpib_bus_init();
            break;
        }

        switch(tx_state)
        {
            case GPIB_TX_START:
				//DEBUG
				
				if (GPIB_PIN_TST(DAV) == 0)
				{
					GPIB_BUS_SETTLE();                // Let Data BUS settle
					if(debuglevel & GPIB_ERR)
						printf("gpib DAV LOW unexpected at TX START!");
				}

				// Wait while DAV is LOW before starting
				// IF DAV = 0 the bus is busy - it should not be - ERROR - but it is
                if(GPIB_PIN_TST(DAV) == 1)
                {
					sreg = SREG;
					cli();
#if BOARD == 2
					// Switch GBIB driver direction to Write
                    GPIB_IO_HI(TE);               // BUS OUT, DAV OUT, NRFD and NDAC IN
                    GPIB_IO_LOW(DC);              // ATN OUT, EOI OUT, SRQ IN
//GPIB_IO_HI(DAV);
#endif
// My testing with various GPIB devices shows that we MUST assert ATN EARLY!
                    if(ch & ATN_FLAG)
					{
                        GPIB_IO_LOW(ATN);         // FYI: SS80 never sends ATN from a device
					}
                    else
					{
#if BOARD == 2
						GPIB_IO_HI(ATN); 
#else
						GPIB_PIN_FLOAT_UP(ATN);
#endif
					}
					SREG = sreg;

                    gpib_timeout_set(HTIMEOUT);
                    tx_state = GPIB_TX_WAIT_READY;
                }

                if (gpib_timeout_test())
                {
                    if(debuglevel & (GPIB_ERR + GPIB_BUS_OR_CMD_BYTE_MESSAGES))
                        printf("<BUS waiting for DAV==1>\n");
                    ch |= TIMEOUT_FLAG;
                    tx_state = GPIB_TX_ERROR;
                    break;
                }
                break;

            case GPIB_TX_WAIT_READY:
// DEV is high
// Wait for NRFD HI and NDAC LOW
                if(GPIB_PIN_TST(NRFD) == 1 && GPIB_PIN_TST(NDAC) == 0)
                {
                    gpib_timeout_set(HTIMEOUT);
                    tx_state = GPIB_TX_PUT_DATA;
                }
                if (gpib_timeout_test())
                {
                    if(debuglevel & (GPIB_ERR + GPIB_BUS_OR_CMD_BYTE_MESSAGES))
                        printf("<BUS waiting for NRFD==1 && NDAC == 0>\n");
                    ch |= TIMEOUT_FLAG;
                    tx_state = GPIB_TX_ERROR;
                    break;
                }
                break;

            case GPIB_TX_PUT_DATA:

                if(ch & EOI_FLAG)
				{
                    GPIB_IO_LOW(EOI);
				}
                else
				{
#if BOARD == 2
                    GPIB_IO_HI(EOI);
#else
                    GPIB_PIN_FLOAT_UP(EOI);
#endif
				}

                GPIB_BUS_WR((ch & 0xff) ^ 0xff);  // Write Data inverted
#if BOARD == 2
// Switch to Tristate BUS drive mode to soeed up Write
                GPIB_IO_HI(PE);
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
                    if(debuglevel & (GPIB_ERR + GPIB_BUS_OR_CMD_BYTE_MESSAGES))
                        printf("<BUS waiting for NRFD==0>\n");
                    break;
                }
                break;

///@brief ALL devices are ready
            case GPIB_TX_WAIT_FOR_NDAC_HI:
                if(GPIB_PIN_TST(NDAC) == 1)       // Byte byte accepted
                {
                    tx_state = GPIB_TX_SET_DAV_HI;
                    break;
                }
                if (gpib_timeout_test())
                {
                    ch |= TIMEOUT_FLAG;
                    tx_state = GPIB_TX_ERROR;
                    if(debuglevel & (GPIB_ERR + GPIB_BUS_OR_CMD_BYTE_MESSAGES))
                        printf("<BUS waiting for NDAC==1>\n");
                    break;
                }
                break;

///@release BUS
            case GPIB_TX_SET_DAV_HI:
#if BOARD == 2
                GPIB_IO_HI(DAV);
#else
                GPIB_PIN_FLOAT_UP(DAV);
#endif
                GPIB_BUS_SETTLE();                // give some time

                // FIXME ?
                gpib_bus_read_init(1);            // Free BUS busy

                gpib_timeout_set(HTIMEOUT);
                tx_state = GPIB_TX_WAIT_FOR_DAV_HI;
                break;

			// Wait for DAV to float HI - BOARD Version 1
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
                    if(debuglevel & (GPIB_ERR + GPIB_BUS_OR_CMD_BYTE_MESSAGES))
                        printf("<BUS waiting for DAV==1>\n");

                }
                break;
            case GPIB_TX_FINISH:
                tx_state = GPIB_TX_DONE;
                break;

            case GPIB_TX_ERROR:
                gpib_bus_read_init(1);	// BUSY
				// gpib_bus_read_init() sets the following states
				// GPIB_BUS_LATCH_WR(0xff); // float OC BUS on write
				// GPIB_IO_LOW(PE);			// BUS OC PULLUP
				// BUS IN, DAV IN, NDAC OUT, NRFD OUT
				// ATN IN, EOI IN, SRQ OUT OC

                if(debuglevel & (GPIB_ERR + GPIB_BUS_OR_CMD_BYTE_MESSAGES))
                    printf("<NRFD=%d,NDAV=%d>\n", GPIB_PIN_TST(NRFD),GPIB_PIN_TST(NDAC));
// Free BUS, BUSY on error
                tx_state = GPIB_TX_DONE;
                break;

// FIXME do we want to be busy at this point
            case GPIB_TX_DONE:
                break;
        }
    }
    return(ch);
}
