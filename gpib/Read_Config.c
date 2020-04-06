enum {
    TOK_INVALID,
    TOK_ACCESS_TIME,
    TOK_ADDRESS,
    TOK_AMIGO,
    TOK_BLOCK_TIME,
    TOK_BUFFERED_BLOCKS,
    TOK_BURST_SIZE,
    TOK_BYTES_PER_BLOCK,
    TOK_BYTES_PER_SECTOR,
    TOK_CONFIG,
    TOK_CONTINUOUS_TRANSFER_RATE,
    TOK_CONTROLLER,
    TOK_CS80,
    TOK_CYLINDERS,
    TOK_DEBUG,
    TOK_DEVICE_NUMBER,
    TOK_DRIVE,
    TOK_END,
    TOK_FILE,
    TOK_FIXED_VOLUMES,
    TOK_GEOMETRY,
    TOK_HEADER,
    TOK_HEADS,
    TOK_ID,
    TOK_INTERLEAVE,
    TOK_MAX_BLOCK_NUMBER,
    TOK_MAX_CYLINDER,
    TOK_MAX_HEAD,
    TOK_MAXIMUM_INTERLEAVE,
    TOK_MAX_SECTOR,
    TOK_OPTIMAL_RETRY_TIME,
    TOK_PPR,
    TOK_PRINTER,
    TOK_PRINTER_CONFIG,
    TOK_REMOVABLE_VOLUMES,
    TOK_SECTORS_PER_TRACK,
    TOK_SS80,
    TOK_SS80_DEFAULT,
    TOK_TRANSFER_RATE,
    TOK_TYPE,
    TOK_UNIT,
    TOK_UNITS_INSTALLED,
    TOK_UNIT_TYPE,
    TOK_VOLUME
};

enum {
  ARG_NONE,
  ARG_STR,
  ARG_NUM
};

typedef struct 
{
	char *name;
	int  tok;
	int  arg;
} token_t;


token_t tokens[] =
{
    {"ACCESS_TIME",               TOK_ACCESS_TIME,            ARG_NUM},
    {"ADDRESS",                   TOK_ADDRESS,                ARG_NUM},
    {"AMIGO",                     TOK_AMIGO,                  ARG_STR},
    {"BLOCK_TIME",                TOK_BLOCK_TIME,             ARG_NUM},
    {"BUFFERED_BLOCKS",           TOK_BUFFERED_BLOCKS,        ARG_NUM},
    {"BURST_SIZE",                TOK_BURST_SIZE,             ARG_NUM},
    {"BYTES_PER_BLOCK",           TOK_BYTES_PER_BLOCK,        ARG_NUM},
    {"BYTES_PER_SECTOR",          TOK_BYTES_PER_SECTOR,       ARG_NUM},
    {"CONFIG",                    TOK_CONFIG,                 ARG_NONE},
    {"CONTINOUS_TRANSFER_RATE",   TOK_CONTINUOUS_TRANSFER_RATE,ARG_NUM},
    {"CONTINUOUS_TRANSFER_RATE",  TOK_CONTINUOUS_TRANSFER_RATE,ARG_NUM},
    {"CONTROLLER",                TOK_CONTROLLER,             ARG_NONE},
    {"CS80",                      TOK_CS80,                   ARG_STR},
    {"CYLINDERS",                 TOK_CYLINDERS,              ARG_NUM},
    {"DEBUG",                     TOK_DEBUG,                  ARG_NUM},
    {"DEVICE_NUMBER",             TOK_DEVICE_NUMBER,          ARG_NUM},
    {"DRIVE",                     TOK_DRIVE,                  ARG_NUM},
    {"END",                       TOK_END,                    ARG_NONE},
    {"FILE",                      TOK_FILE,                   ARG_STR},
    {"FIXED_VOLUMES",             TOK_FIXED_VOLUMES,          ARG_NUM},
    {"GEOMETRY",                  TOK_GEOMETRY,               ARG_NONE},
    {"HEADER",                    TOK_HEADER,                 ARG_NONE},
    {"HEADS",                     TOK_HEADS,                  ARG_NUM},
    {"ID",                        TOK_ID,                     ARG_NUM},
    {"INTERLEAVE",                TOK_INTERLEAVE,             ARG_NUM},
    {"MAX_BLOCK_NUMBER",          TOK_MAX_BLOCK_NUMBER,       ARG_NUM},
    {"MAX_CYLINDER",              TOK_MAX_CYLINDER,           ARG_NUM},
    {"MAX_HEAD",                  TOK_MAX_HEAD,               ARG_NUM},
    {"MAXIMUM_INTERLEAVE",        TOK_MAXIMUM_INTERLEAVE,     ARG_NUM},
    {"MAX_SECTOR",                TOK_MAX_SECTOR,             ARG_NUM},
    {"OPTIMAL_RETRY_TIME",        TOK_OPTIMAL_RETRY_TIME,     ARG_NUM},
    {"PPR",                       TOK_PPR,                    ARG_NUM},
    {"PRINTER",                   TOK_PRINTER,                ARG_NONE},
    {"REMOVABLE_VOLUMES",         TOK_REMOVABLE_VOLUMES,      ARG_NUM},
    {"SECTORS_PER_TRACK",         TOK_SECTORS_PER_TRACK,      ARG_NUM},
    {"SS80_DEFAULT",              TOK_SS80_DEFAULT,           ARG_NONE},
    {"SS80",                      TOK_SS80,                   ARG_STR},
    {"TRANSFER_RATE",             TOK_TRANSFER_RATE,          ARG_NUM},
    {"TYPE",                      TOK_TYPE,                   ARG_NUM},
    {"UNITS_INSTALLED",           TOK_UNITS_INSTALLED,        ARG_NUM},
    {"UNIT",                      TOK_UNIT,                   ARG_NONE},
    {"UNIT_TYPE",                 TOK_UNIT_TYPE,              ARG_NUM},
    {"VOLUME",                    TOK_VOLUME,                 ARG_NONE},
    {NULL,                        TOK_INVALID,                ARG_NONE},
};



/// ===============================================
/// @brief Read and parse a config file using POSIX functions
/// Set all drive parameters and debuglevel 
///
/// @param name: config file name to process
/// @return  number of parse errors
int Read_Config(char *name)
{
    int i,ret;
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

    char *ptr,*pat;
	int index = 0;
	uint32_t value32 = 0;
	uint16_t value16 = 0;
	uint8_t  value8  = 0;
	int tok = TOK_INVALID;
    char str[128];
    char token[128];

    init_Devices();

    lines = 0;

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

		value32 = 0;
		value16 = 0;
		value8  = 0;
		tok = TOK_INVALID;

		for (i = 0; (pat = tokens[i].name) != NULL; ++i )
		{
			if( MATCHI(token,pat) )
			{
				tok = tokens[i].tok;
				// get optional argument
				ptr = get_token(ptr, token, sizeof(token)-2);                       
				if(MATCH(token,"="))
					ptr = get_token(ptr, token, sizeof(token)-2);                       
				if(tokens[i].arg == ARG_NUM)
				{
					 value32 = get_value(token);
					 value16 = (uint16_t) 0xFFFF & value32;
					 value8 = (uint8_t) 0xFF & value32;
					
				}
				break;
			}
		}
		if(tok == TOK_INVALID)
		{
            printf("Unexpected token: %s, at line:%d\n", token,lines);
			++errors;
			continue;
		}


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
						hpdir_set_parameters(index,token);	// Also sets Devices[index].model
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
						hpdir_set_parameters(index,token);	// Also sets Devices[index.model
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
					debuglevel = value16;
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
				Devices[index].ADDRESS = 0x1f & value8;
				PRINTERp->HEADER.ADDRESS  = 0x1f & value8;
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
					Devices[index].ADDRESS = 0x1f & value8;
					SS80p->HEADER.ADDRESS  = 0x1f & value8;
					break;
				case TOK_PPR:
					Devices[index].PPR = 0x07 & value8;
					SS80p->HEADER.PPR = 0x07 & value8;
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
                SS80p->CONFIG.ID = value16;
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
					SS80p->CONTROLLER.UNITS_INSTALLED = value16;
					break;
				case TOK_TRANSFER_RATE:
					SS80p->CONTROLLER.TRANSFER_RATE = value16;
					break;
            	case TOK_TYPE:
					SS80p->CONTROLLER.TYPE = value8;
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
					SS80p->UNIT.UNIT_TYPE = value8;
            		break;
            	case TOK_DEVICE_NUMBER:
					SS80p->UNIT.DEVICE_NUMBER = value32;
            		break;
            	case TOK_BYTES_PER_BLOCK:
					SS80p->UNIT.BYTES_PER_BLOCK = value16;
            		break;
            	case TOK_BUFFERED_BLOCKS:
					SS80p->UNIT.BUFFERED_BLOCKS = value8;
            		break;
            	case TOK_BURST_SIZE:
					SS80p->UNIT.BURST_SIZE = value8;
            		break;
            	case TOK_BLOCK_TIME:
					SS80p->UNIT.BLOCK_TIME = value16;
            		break;
            	case TOK_CONTINUOUS_TRANSFER_RATE:
					SS80p->UNIT.CONTINUOUS_TRANSFER_RATE = value16;
					break;
				case TOK_OPTIMAL_RETRY_TIME:
					SS80p->UNIT.OPTIMAL_RETRY_TIME = value16;
            		break;
            	case TOK_ACCESS_TIME:
					SS80p->UNIT.ACCESS_TIME = value16;
            		break;
            	case TOK_MAXIMUM_INTERLEAVE:
					SS80p->UNIT.MAXIMUM_INTERLEAVE = value8;
            		break;
				case TOK_FIXED_VOLUMES:
					SS80p->UNIT.FIXED_VOLUMES = value8;
            		break;
            	case TOK_REMOVABLE_VOLUMES:
					SS80p->UNIT.REMOVABLE_VOLUMES = value8;
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
					SS80p->VOLUME.MAX_CYLINDER = value32;
					break;
            	case TOK_MAX_HEAD:
					SS80p->VOLUME.MAX_HEAD = value8;
					break;
            	case TOK_MAX_SECTOR:
					SS80p->VOLUME.MAX_SECTOR = value16;
					break;
            	case TOK_MAX_BLOCK_NUMBER:
					SS80p->VOLUME.MAX_BLOCK_NUMBER = value32;
					break;
            	case TOK_INTERLEAVE:
					SS80p->VOLUME.INTERLEAVE = value8;
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
					Devices[index].ADDRESS = 0x1f & value8;
					AMIGOp->HEADER.ADDRESS = 0x1f & value8;
					break;
            	case TOK_PPR:
					Devices[index].PPR = 0x07 & value8;
					AMIGOp->HEADER.PPR = 0x07 & value8;
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
                AMIGOp->CONFIG.ID = value16;
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
					AMIGOp->GEOMETRY.BYTES_PER_SECTOR = value16;
					break;
				case TOK_SECTORS_PER_TRACK:
					AMIGOp->GEOMETRY.SECTORS_PER_TRACK = value16;
					break;
				case TOK_HEADS:
					AMIGOp->GEOMETRY.HEADS = value16;
					break;
            	case TOK_CYLINDERS:
					AMIGOp->GEOMETRY.CYLINDERS = value16;
					break;
            	default:
					printf("Unexpected AMIGO GEMETRY token: %s, at line:%d\n", ptr,lines);
					++errors;
					break;
            }
            break;
#endif // #ifdef AMIGO
        default:
            printf("Unexpected STATE: %s, at line:%d\n", ptr,lines);
			++errors;
            break;

        } // switch
    } //while
    if(state != START_STATE)
    {
        printf("Missing END statement at line:%d\n", lines);
        ++errors;
    }
    printf("Read_Config: read(%d) lines\n", lines);
    if(errors)
        printf("Read_Config: ****** errors(%d) ******\n", errors);

    ret = fclose(cfg);
    if(ret == EOF)
    {
        perror("Read_Config - close error");
        ++errors;
    }

	// Post process device values
	Post_Config();

    return(errors);
}
#endif
