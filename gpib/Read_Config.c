/// ===============================================
/// @brief Read and parse a config file using POSIX functions
/// Set all drive parameters and debuglevel
///
/// @param name: config file name to process
/// @return  number of parse errors
int Read_Config(char *name)
{
    int ret,len;
    FILE *cfg;
    int state = START_STATE;
    int errors = 0;
    int index = 0;
    val_t val;

///@brief Printer Device
    PRINTERDeviceType *PRINTERp = NULL;
///@brief SS80 Device
    SS80DiskType *SS80p = NULL;

#ifdef AMIGO
///@brief AMIGO Device
    AMIGODiskType *AMIGOp = NULL;
#endif

    char *ptr;
    char str[128];
    char token[128];
    char arg[128];

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

        ptr = str;

        trim_tail(ptr);
        ptr = skipspaces(ptr);
        len = strlen(ptr);

        if(!len)
            continue;

// Skip comments
        if(*ptr == '#')
            continue;

        *token = 0;
        *arg = 0;
        val.l = 0;

// To save on code we process a token and optional argument here
        ptr = get_token(ptr,token, sizeof(token)-2);

// Argument
        ptr = get_token(ptr,arg, sizeof(arg)-2);
        if( MATCHI(arg,"=") )
            ptr = get_token(ptr,arg,sizeof(arg)-2);
        val.l = get_value(arg);

//FIXME check for state and last state
        if( MATCHI(token,"END") )
        {
            state = pop_state();
            continue;
        }

        switch(state)
        {
            case START_STATE:

                if( MATCHI (token,"SS80_DEFAULT") )
                {
                    push_state(state);
                    state = SS80_STATE;
                    index = alloc_device(SS80_DEFAULT_TYPE);
                    if(index == -1)
                        state = START_STATE;
                    else
                        SS80p = (SS80DiskType *) Devices[index].dev;
                }

                else if( MATCHI (token,"SS80") || MATCHI (token, "CS80") )
                {
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
                                                  // Also sets Devices[index].model
                        hpdir_set_parameters(index,arg);
                    }
                }

#ifdef AMIGO
                else if( MATCHI (token,"AMIGO") )
                {
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
                                                  // Also sets Devices[index.model
                        hpdir_set_parameters(index,arg);
                    }
                }
#endif
                else if( MATCHI (token,"PRINTER") )
                {
                    push_state(state);
                    state = PRINTER_STATE;
                    index = alloc_device(PRINTER_TYPE);
                    if(index == -1)
                        state = START_STATE;
                    else
                        PRINTERp = (PRINTERDeviceType *) Devices[index].dev;
                }
                else if( MATCHI (token,"DEBUG") )
                {
                    debuglevel = val.w;
                }
                else if( MATCHI (token,"PRINTER_DEFAULT_ADDRESS") )
                {
//FIXME REMOVE from config
                    printf("Skipping %s, at line:%d\n", str,lines);
                }
                else
                {
                    printf("Unexpected START token: %s, at line:%d\n", str,lines);
                    ++errors;
                }
                break;

            case PRINTER_STATE:
                if( MATCHI (token,"CONFIG") )
                {
                    push_state(state);
                    state = PRINTER_CONFIG;
                }
                else
                {
                    printf("Unexpected PRINTER token: %s, at line:%d\n", str,lines);
                    ++errors;
                }
                break;

            case PRINTER_CONFIG:
                if( MATCHI (token,"ADDRESS") )
                {
                    if(val.b > 31)
                    {
                        printf("Fatal PRINTER ADDRESS out of range: %ld disabled:%d\n", val.l, lines);
                        val.b = 0xff;
                        ++errors;
                    }
                    Devices[index].ADDRESS = val.b;
                    PRINTERp->HEADER.ADDRESS  = val.b;
// NO PPR
                    Devices[index].PPR = 0xff;
                    PRINTERp->HEADER.PPR = 0xff;
                }
                else
                {
                    printf("Unexpected PRINTER CONFIG token: %s, at line:%d\n", str,lines);
                    ++errors;
                }
                break;

            case SS80_STATE:
                if( MATCHI (token,"HEADER") )
                {
                    push_state(state);
                    state = SS80_HEADER;
                }
                else if( MATCHI (token,"CONFIG") )
                {
                    push_state(state);
                    state = SS80_CONFIG;
                }
                else if( MATCHI (token,"CONTROLLER") )
                {
                    push_state(state);
                    state = SS80_CONTROLLER;
                }
                else if( MATCHI (token,"UNIT") )
                {
                    push_state(state);
                    state = SS80_UNIT;
                }
                else if( MATCHI (token,"VOLUME") )
                {
                    push_state(state);
                    state = SS80_VOLUME;
                }
                else
                {
                    printf("Unexpected SS80 START token: %s, at line:%d\n", str,lines);
                    ++errors;
                }
                break;

            case SS80_HEADER:
                if( MATCHI (token,"ADDRESS") )
                {
                    if(val.b > 31)
                    {
                        printf("Fatal SS80 ADDRESS > 31 at line d\n", lines);
                        val.b = 0xff;
                        ++errors;
                    }
                    Devices[index].ADDRESS = val.b;
                    SS80p->HEADER.ADDRESS  = val.b;
                }
                else if( MATCHI (token,"PPR") )
                {
                    if(val.b > 7)
                    {
                        printf("Warning SS80 PPR > 7 at line:%d\n", lines);
                        val.b = 0xff;
                        ++errors;
                    }
                    Devices[index].PPR = val.b;
                    SS80p->HEADER.PPR = val.b;
                }
                else if( MATCHI (token,"FILE") )
                {
                    SS80p->HEADER.NAME = stralloc(arg);
                }
                else
                {
                    printf("Unexpected SS80 CONFIG token: %s, at line:%d\n", token,lines);
                    ++errors;
                }
                break;

            case SS80_CONFIG:
                if( MATCHI (token,"ID") )
                {
                    SS80p->CONFIG.ID = val.w;
                }
                else
                {
                    printf("Unexpected SS80 CONFIG token: %s, at line:%d\n", str,lines);
                    ++errors;
                }
                break;

            case SS80_CONTROLLER:
                if( MATCHI (token,"UNITS_INSTALLED") )
                {
                    SS80p->CONTROLLER.UNITS_INSTALLED = val.w;
                }
                else if( MATCHI (token,"TRANSFER_RATE") )
                {
                    SS80p->CONTROLLER.TRANSFER_RATE = val.w;
                }
                else if( MATCHI (token,"TYPE") )
                {
                    SS80p->CONTROLLER.TYPE = val.w;
                }
                else
                {
                    printf("Unexpected SS80 CONTROLLER token: %s, at line:%d\n", token,lines);
                    ++errors;
                }
                break;

            case SS80_UNIT:
                if( MATCHI (token,"UNIT_TYPE") )
                {
                    SS80p->UNIT.UNIT_TYPE = val.w;
                }
                else if( MATCHI (token,"DEVICE_NUMBER") )
                {
                    SS80p->UNIT.DEVICE_NUMBER = val.l;
                }
                else if( MATCHI (token,"BYTES_PER_BLOCK") )
                {
                    if(val.w > 0x1000)
                    {
                        printf("Fatal: SS80 BYTES_PER_BLOCK > 0x1000, set to 256 at line:%d\n", lines);
                        val.w = 256;
                        ++errors;
                    }
                    SS80p->UNIT.BYTES_PER_BLOCK = val.w;
                }
                else if( MATCHI (token,"BUFFERED_BLOCKS") )
                {
                    if(val.b > 1)
                    {
                        printf("Fatal: SS80 BUFFERED_BLOCKS > 1, set to 1 at line:%d\n", lines);
                        ++errors;
                        val.w = 1;
                    }
                    SS80p->UNIT.BUFFERED_BLOCKS = val.b;
                }
                else if( MATCHI (token,"BURST_SIZE") )
                {
                    if(val.b > 0)
                    {
                        printf("Fatal: SS80 BURST_SIZE > 0, set to 1 at line:%d\n", lines);
                        ++errors;
                        val.w = 0;
                    }
                    SS80p->UNIT.BURST_SIZE = val.b;
                }
                else if( MATCHI (token,"BLOCK_TIME") )
                {
                    SS80p->UNIT.BLOCK_TIME = val.w;
                }
                else if( MATCHI (token,"CONTINUOUS_TRANSFER_RATE") || MATCHI (token,"CONTINOUS_TRANSFER_RATE") )
                {
                    SS80p->UNIT.CONTINUOUS_TRANSFER_RATE = val.w;
                }
                else if( MATCHI (token,"OPTIMAL_RETRY_TIME") )
                {
                    SS80p->UNIT.OPTIMAL_RETRY_TIME = val.w;
                }
                else if( MATCHI (token,"ACCESS_TIME") )
                {
                    SS80p->UNIT.ACCESS_TIME = val.w;
                }
                else if( MATCHI (token,"MAXIMUM_INTERLEAVE") )
                {
                    SS80p->UNIT.MAXIMUM_INTERLEAVE = val.b;
                }
                else if( MATCHI (token,"FIXED_VOLUMES") )
                {
                    SS80p->UNIT.FIXED_VOLUMES = val.b;
                }
                else if( MATCHI (token,"REMOVABLE_VOLUMES") )
                {
                    SS80p->UNIT.REMOVABLE_VOLUMES = val.b;
                }
                else
                {
                    printf("Unexpected SS80 UNIT token: %s, at line:%d\n", token,lines);
                    ++errors;
                }
                break;

            case SS80_VOLUME:
                if( MATCHI (token,"MAX_CYLINDER") )
                {
                    SS80p->VOLUME.MAX_CYLINDER = val.l;
                }
                else if( MATCHI (token,"MAX_HEAD") )
                {
                    SS80p->VOLUME.MAX_HEAD = val.b;
                }
                else if( MATCHI (token,"MAX_SECTOR") )
                {
                    SS80p->VOLUME.MAX_SECTOR = val.w;
                }
                else if( MATCHI (token,"MAX_BLOCK_NUMBER") )
                {
                    SS80p->VOLUME.MAX_BLOCK_NUMBER = val.w;
                }
                else if( MATCHI (token,"INTERLEAVE") )
                {
                    SS80p->VOLUME.INTERLEAVE = val.b;

                }
                else
                {
                    printf("Unexpected SS80 VOLUME token: %s, at line:%d\n", token,lines);
                    ++errors;
                }
                break;

#ifdef AMIGO
            case AMIGO_STATE:
                if( MATCHI (token,"HEADER") )
                {
                    push_state(state);
                    state = AMIGO_HEADER;
                }
                else if( MATCHI (token,"CONFIG") )
                {
                    push_state(state);
                    state = AMIGO_CONFIG;
                }
                else if( MATCHI (token,"GEOMETRY") )
                {
                    push_state(state);
                    state = AMIGO_GEOMETRY;
                }
                else
                {
                    printf("Unexpected AMIGO START token: %s, at line:%d\n", str,lines);
                    ++errors;
                }
                break;

            case AMIGO_HEADER:
                if( MATCHI (token,"DRIVE") )
                {
//skip this
                    printf("Skipping %s, at line:%d\n", str,lines);
                }
                else if( MATCHI (token,"ADDRESS") )
                {
                    if(val.b > 31)
                    {
                        printf("Fatal AMIGO ADDRESS: %d > 31 disabled at line:%d\n", str,lines);
                        val.b = 0xff;
                        ++errors;
                    }
                    Devices[index].ADDRESS = val.b;
                    AMIGOp->HEADER.ADDRESS = val.b;
                }
                else if( MATCHI (token,"PPR") )
                {
                    if(val.b > 7)
                    {
                        printf("Warning AMIGO ADDRESS: %d > 7, disabled at line:%d\n", str,lines);
                        val.b = 0xff;
                        ++errors;
                    }
                    Devices[index].PPR = val.b;
                    AMIGOp->HEADER.PPR = val.b;
                }
                else if( MATCHI (token,"FILE") )
                {
                    AMIGOp->HEADER.NAME = stralloc(arg);
                }
                else
                {
                    printf("Unexpected HEADER CONFIG token: %s, at line:%d\n", str,lines);
                    ++errors;
                }
                break;

            case AMIGO_CONFIG:
                if( MATCHI (token,"ID") )
                {
                    AMIGOp->CONFIG.ID = val.w;
                }
                else
                {
                    printf("Unexpected AMIGO CONFIG token: %s, at line:%d\n", str,lines);
                    ++errors;
                }
                break;

            case AMIGO_GEOMETRY:
                if( MATCHI (token,"BYTES_PER_SECTOR") )
                {
                    if(val.w > 0x1000)
                    {
                        printf("Fatal: AMIGO BYTES_PER_SECTOR > 0x1000, set to 256 at line:%d\n", lines);
                        val.w = 256;
                        ++errors;
                    }
                    AMIGOp->GEOMETRY.BYTES_PER_SECTOR = val.w;
                }
                else if( MATCHI (token,"SECTORS_PER_TRACK") )
                {
                    AMIGOp->GEOMETRY.SECTORS_PER_TRACK = val.w;
                }
                else if( MATCHI (token,"HEADS") )
                {
                    AMIGOp->GEOMETRY.HEADS = val.w;
                }
                else if( MATCHI (token,"CYLINDERS") )
                {
                    AMIGOp->GEOMETRY.CYLINDERS = val.w;
                }
                else
                {
                    printf("Unexpected AMIGO GEMETRY token: %s, at line:%d\n", str,lines);
                    ++errors;
                }
                break;
#endif                                // #ifdef AMIGO

            default:
                printf("Fatal Unexpected STATE %d at line:%d\n", state, lines);
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
