#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <libgen.h>

#define MATCH(a,b) ( strcasecmp((a),(b)) == 0 ? 1 : 0 )

#define MAXLINE 1024

//
//; AMIGIO/CS80/SS80 disk drives
//; 1 - SS80 [HP9134L]            unique drive model identifier (should be identical to [fsinfo] section above)
//; 2 - COMMENT                   descriptive string
//; 3 - SS80/AMIGO:               drive protocol AMIGO, CS80 or SS80
//; 4 - ID:                       ID returned by AMIGO identify
//; 5 -                           mask for secondary ID returned by high byte of STAT2
//; 6 -                           ID returned by high byte of STAT2
//; 7 - DEVICE_NUMBER:            drive number returned by CS/80 describe command
//; 8 - UNITS_INSTALLED(0x8001):  number of units installed
//; 9 - MAX_CYLINDER:             number of cylinders
//;10 - MAX_HEAD:                 number of heads or surfaces
//;11 - MAX_SECTOR:               number of sectors per track
//;12 - BYTES_PER_BLOCK:          number of bytes per block or sector
//;13 - INTERLEAVE:               default interleave
//;14 - media type 0=fixed media, 1=removable media
//; # Removable volume byte; one bit per volume (set if removable)
//; 1,                                           x,           3,      4,    5,    6,        7, 8,    9, 10, 11,  12,13,14
//9895    = "HP9895A dual 1.15M AMIGO floppy disc",       AMIGO, 0x0081, 0x00, 0x00, 0x000000, 1,   77,  2, 30, 256, 7, 1

typedef struct {
    char model[MAXLINE];	// 1
    char comment[MAXLINE];  // 2
	char TYPE[MAXLINE];     // 3
    int  ID;				// 4
	int  mask_stat2;		// 5
	int  id_stat2;			// 6
	int  DEVICE_NUMBER;		// 7
	int	 UNITS_INSTALLED; 	// 8	ALWAYS 1 , FIXED
	int  CYLINDERS;	    	// 9
	int  HEADS;				// 10
	int  SECTORS;			// 11
	int  BYTES_PER_BLOCK;	// 12
	int  INTERLEAVE;		// 13
    int  FIXED;				// 14 ALWAYS 1
	// AMIGO
	long BLOCKS;
	// SS80 values
	int	 MAX_CYLINDER;
	int  MAX_HEAD;
	int  MAX_SECTOR;
	long MAX_BLOCK_NUMBER;
	long LIF_DIR_BLOCKS;
} hpdir;

hpdir disk;

void init_disk()
{
    memset(disk.model,0,MAXLINE-1);		// 1
    memset(disk.comment,0,MAXLINE-1);	// 2
    memset(disk.TYPE,0,MAXLINE-1);		// 3
    disk.ID = 0;						// 4
	disk.mask_stat2 = 0;				// 5
	disk.id_stat2 = 0;					// 6
	disk.DEVICE_NUMBER = 0;				// 7
	disk.UNITS_INSTALLED = 0x8001;		// 8
	disk.CYLINDERS = 0;  				// 9
	disk.HEADS= 0;	     				// 10
	disk.SECTORS= 0;    				// 11
	disk.BYTES_PER_BLOCK = 0;			// 12
	disk.INTERLEAVE = 0;				// 13
    disk.FIXED = 1;						// 14 ALWAYS 1

    disk.BLOCKS = 0;
	// MAX_BLOCK_NUMBER = BLOCKS - 1
	// MAX_BLOCK_NUMBER =(MAX_CYLINDER-1) * (MAX_HEAD-1) * (MAX_SECTOR-1) -1;
	disk.MAX_CYLINDER = 0;
	disk.MAX_HEAD = 0;
	disk.MAX_SECTOR = 0;
	disk.MAX_BLOCK_NUMBER = 0;
	disk.LIF_DIR_BLOCKS= 0;
}



void trim_tail(char *str)
{
    int len = strlen(str);
    while(len--)
    {
        if(str[len] > ' ')
            break;
        str[len] = 0;
    }
}

char *skipspaces(char *ptr)
{
    if(!ptr)
        return(ptr);

    while(*ptr == ' ' || *ptr == '\t')
        ++ptr;
    return(ptr);
}

char *skipbreak(char *ptr)
{
    if(!ptr)
        return(ptr);
	ptr = skipspaces(ptr);
	// break conditions
	if(*ptr == ',' )
		++ptr;
	ptr = skipspaces(ptr);
	return(ptr);
}

char *get_token(char *ptr, char *token, int max)
{
	// skip spaces
	// break conditions
	ptr = skipbreak(ptr);
	trim_tail(ptr);

    while(*ptr && max > 0) {

		// skip control characters
        if(*ptr < ' ') {
            ++ptr;
            continue;
        }

        // string processing
        if(*ptr == '"')
        {
            ++ptr;
            // We are pointing at the body of the quoted string now
            while(*ptr && *ptr != '"' && max > 0)
			{
				*token++ = *ptr++;
				--max;
			}
            if(*ptr == '"') 
			{
				++ptr;
                *token++ = 0;
				break;
			}
			break;
        }

		// skip past break character
		if (*ptr == ' ' || *ptr == '\t' || *ptr == ',' )
		{
			++ptr;
			break;
		}

		// copy token
        *token++ = *ptr++;
    }
    *token = 0;
    return(ptr);
}


// =============================================
///@brief Compare two strings without case.
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return string lenth on match.
///@return 0 on no match.
int MATCHI(char *str, char *pat)
{
    int len;
    len = strlen(pat);
    if(strcasecmp(str,pat) == 0 )
        return(len);
    return(0);
}
// =============================================
///@brief Compare two strings limted to length of pattern.
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return string lenth on match.
///@return 0 on no match.
///@warning Matches sub strings so be caeful.
int MATCH_LEN(char *str, char *pat)
{
    int len;

    if(!str || !pat)
        return(0);
    len = strlen(pat);

    if( len )
    {
        if(strncmp(str,pat,len) == 0 )
            return(len);
    }
    return(0);
}

// =============================================
///@brief Compare two strings without case limted to length of pattern.
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return string lenth on match.
///@return 0 on no match.
///@warning Matches sub strings so be caeful.
int MATCHI_LEN(char *str, char *pat)
{
    int len;

    if(!str || !pat)
        return(0);
    len = strlen(pat);

    if( len )
    {
        if(strncasecmp(str,pat,len) == 0 )
            return(len);
    }
    return(0);
}

/// @brief get a number
///
/// - Used only for debugging
/// @param[in] str: string to examine
///
/// @return  value
int32_t get_value(char *str)
{
    int base;
    int ret;
    char *ptr;
    char *endptr;


    ptr = skipspaces(str);
    base = 10;

    // convert number base 10, 16, 8 and 2
    if( (ret = MATCHI_LEN(ptr,"0x")) )
    {
        base = 16;
        ptr += ret;
    }
    else if( (ret = MATCHI_LEN(ptr,"0o")) )
    {
        base = 8;
        ptr += ret;
    }
    else if( (ret = MATCHI_LEN(ptr,"0b")) )
    {
        base = 2;
        ptr += ret;
    }
    return(strtol(ptr, (char **)&endptr, base));
}



/// @brief assigned a value
///
/// - Used only for debugging
/// @param[in] str: string to examine
/// @param[in] minval: minimum value
/// @param[in] maxval: maximum value
/// @param[in] *val: value to set
///
/// @return  1 is matched and value in range, 0 not matched or out of range
uint32_t assign_value(char *str, uint32_t minval, uint32_t maxval, uint32_t *val)
{
    uint32_t tmp;
    int bad = 0;
    char *ptr;

    // Skip spaces before assignment
    ptr = skipspaces(str);
    // Skip optional '='
    if(*ptr == '=')
    {
        ++ptr;
        // skip spaces after assignment
        ptr = skipspaces(ptr);
    }
    if(!*ptr)
    {
		printf(":%s missing value\n", str);
        bad = 1;
    }
    if(!bad)
    {
        // FIXME detect bad numbers
        tmp = get_value(ptr);
        *val = tmp;
        if((minval && (tmp < minval)))
        {
            printf("%s is below range %d\n", ptr,(int)minval);
            bad = 1;
        }
        if((maxval != 0xffffffffUL) && (tmp > maxval))
        {
            printf("%s is above range %d\n", ptr,(int)maxval);
            bad = 1;
        }
    }
    if(bad)
        return(0);
    return(1);
}

// LIF Directory blocks ~= sqrt(blocks);
// We simplify 
//  BITS = Bit position of MSB of block count 
//  DIR Size = BITS / 2
//  
long lif_dir_count(long blocks)
{
	int scale = 0;
	long num = 1;
	while(blocks)
	{
		scale++;
 		blocks >>= 1;
	}
	scale>>=1;
	while(scale--)
		num <<=1;
	return(num);
}

	

void usage(char *ptr)
{
	printf("%s [-list]| [-m model [-b]|[-d]] [-a address]\n", ptr);
	printf("   -list lists all of the drives in the hpdir.ini file\n");
	printf("   -a disk address 0..7\n");
	printf("   -m model only, list hpdisk.cfg format disk configuration\n");
	printf("   -b only display block count, you can can use this with -m\n");
	printf("   -d only display computed directory block count, you can use this with -m\n");
	printf("   -f NAME specifies the LIF image name for this drive\n");
}

/// @brief Read and parse a config file using POSIX functions
/// Set all drive parameters and debuglevel 
///
/// @param name: config file name to process
/// @return  number of parse errors
int main(int argc, char *argv[])
{
    int ind,ret,len;
	char *ptr;
    uint32_t tmp;
    uint32_t val;
    FILE *cfg;
    int errors = 0;
    int index = 0;
    int lines=0;
    int driveinfo=0;
	int i;
	int list = 0;
	int address = 0;
	int block = 0;
    int dir_size = 0;
	int ppr = 0;
    char hpdir[MAXLINE];	// 1
    char path[MAXLINE];	// 1
    char model[MAXLINE];	// 1
    char token[MAXLINE];	// 1
    char str[MAXLINE+2];
    char LIFNAME[MAXLINE+2];


	list = 0;
	address = 0;
	model[0] = 0;
	path[0] = 0;
	hpdir[0] = 0;
	LIFNAME[0] = 0;

	memset(hpdir,0,MAXLINE-1);
	len = readlink("/proc/self/exe", hpdir, MAXLINE-2);
	dirname (hpdir);
	strcat  (hpdir, "/hpdir.ini");
	// printf("HOME=%s\n",hpdir);

	if(argc < 2)
	{
		
		usage(argv[0]);
		return(0);
	}


	for(i=1;i<argc; ++i)
	{
		ptr = argv[i];
		if(MATCH(ptr,"-help") || MATCH(ptr,"-h" ) || MATCH(ptr,"?")  )
		{
			usage(argv[0]);
			return(0);
		}

		if(MATCH(ptr,"-list") || MATCH(ptr,"-l" ))
		{
			list = 1;
			continue;
		}

		if(MATCH(ptr,"-b"))
		{
			block = 1;
			continue;
		}

		if(MATCH(ptr,"-d"))
		{
			dir_size = 1;
			continue;
		}

		if(MATCH(ptr,"-m"))
		{
			ptr = argv[++i];
			if(!ptr)
				break;

			strncpy(model,ptr,MAXLINE-2);
			// printf("#Model: %s\n", model );
			continue;
		}
		if(MATCH(ptr,"-f"))
		{
			ptr = argv[++i];
			if(!ptr)
				break;

			strncpy(LIFNAME,ptr,MAXLINE-2);
			// printf("#LIF IMAGE NAME: %s\n", LIFNAME);
			continue;
		}

		if(MATCH(ptr,"-a"))
		{
			ptr = argv[++i];
			if(!ptr)
				break;
			address = atoi(ptr);
			ppr = address;	
			// printf("#Adress=%d\n",address);
			continue;
		}
	}


    cfg = fopen(hpdir, "rb");
    if(cfg == NULL)
    {
        ++errors;
        //FIXME
        perror("Read_Config - open");
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
        if(*ptr == ';' || *ptr == '#' )
            continue;

        if(*ptr == '[' && driveinfo == 1 )
                break;

		// MODEL
		init_disk();


		(void) get_token(ptr, token, 	MAXLINE-2);

        if(MATCHI(token,"[driveinfo]"))
        {
			driveinfo = 1;
            continue;
        }

		if( driveinfo != 1)
			continue;

		if(list) 
		{
			printf("%s\n",ptr);
			continue;
		}

        if( model[0] != 0 )
		{
			// printf("#MODEL: %s #TOKEN: %s\n",model, token);
			if(strcasecmp( model,token) != 0 )
				continue;
		}

		errors = 0;
		ptr = get_token(ptr, token, 	MAXLINE-2);
		strncpy(disk.model,token,MAXLINE-2);

		// =
		ptr = get_token(ptr, token, 	MAXLINE-2);
#ifdef XDEBUG
		printf("=: %s\n",token);
#endif

		// COMMENT
		ptr = get_token(ptr, disk.comment, 	MAXLINE-2);

#ifdef XDEBUG
		printf("after comment:%s\n",ptr);
#endif
		// TYPE SS80/CS80/AMIGO
		ptr = get_token(ptr, disk.TYPE, 	MAXLINE-2);
#ifdef XDEBUG
		printf("disk.TYPE: %s\n",disk.TYPE);
#endif

		// Identify ID
		ptr = get_token(ptr, token, 		MAXLINE-2);
#ifdef XDEBUG
		printf("disk.ID: %s\n",token);
#endif

		if (!assign_value(token, 0, 0xFFFFUL, &val) )
			++errors;
		disk.ID = val;						// 4

		// MASK STAT2
		ptr = get_token(ptr, token, 		MAXLINE-2);
#ifdef XDEBUG
		printf("disk.mask_stat2: %s\n",token);
#endif

		if (!assign_value(token, 0, 0xFFUL, &val) )
			++errors;
		disk.mask_stat2 = val;				// 5

		// STAT2
		ptr = get_token(ptr, token, 		MAXLINE-2);
#ifdef XDEBUG
		printf("disk.stat2: %s\n",token);
#endif

		if (!assign_value(token, 0, 0xFFUL, &val) )
			++errors;
		disk.id_stat2 = val;				// 6

		// BCD encoded device number
		ptr = get_token(ptr, token, 		MAXLINE-2);
#ifdef XDEBUG
		printf("disk.DEVICE_NUMBER: %s\n",token);
#endif

		if (!assign_value(token, 0, 0xffffffffUL, &val) )
			++errors;
		disk.DEVICE_NUMBER = val;				// 7

		// UNITS Installed
		ptr = get_token(ptr, token, 		MAXLINE-2);
#ifdef XDEBUG
		printf("disk.UNITS_INSTALLED: %s\n",token);
#endif

		if (!assign_value(token, 0, 8, &val) )
			++errors;
		disk.UNITS_INSTALLED = 1;				// 8

// ====================================================
// Heads,Cylinders,Sectors Calculation
		// Maximum cylinder
		ptr = get_token(ptr, token, 		MAXLINE-2);
#ifdef XDEBUG
		printf("disk.CYLINDERS: %s\n",token);
#endif

		if (!assign_value(token, 0, 0x00ffffffUL, &val) )
			++errors;

		// Token is CYLINDERS, MAX_CYLINDER = CYLINDERS -1
		disk.CYLINDERS = val;				// 9
		if (val)
			--val;
		disk.MAX_CYLINDER = val;				// 9

		// Maximum head
		ptr = get_token(ptr, token, 		MAXLINE-2);

#ifdef XDEBUG
		printf("disk.HEADS: %s\n",token);
#endif
		if (!assign_value(token, 0, 0xffffUL, &val) )
			++errors;
		// Token is HEADS, MAX_HEAD = HEADS -1
		disk.HEADS = val;					// 10
		if (val)
			--val;
		disk.MAX_HEAD = val;					// 10

		// Maximum sector
		ptr = get_token(ptr, token, 		MAXLINE-2);
#ifdef XDEBUG
		printf("disk.SECTORS: %s\n",token);
#endif

		if (!assign_value(token, 0, 0xffffUL, &val) )
			++errors;
		// Token is SECTORS, MAX_SECTOR = SECTORS -1
		disk.SECTORS = val;				// 11
		if (val)
			--val;
		disk.MAX_SECTOR = val;			// 11

		// Total disk blocks
		disk.BLOCKS = disk.CYLINDERS * disk.HEADS * disk.SECTORS;
		disk.LIF_DIR_BLOCKS = lif_dir_count(disk.BLOCKS);
		// Lst block in SS80 drive
		disk.MAX_BLOCK_NUMBER = disk.BLOCKS-1;
// ====================================================

		// Bytes per sector
		ptr = get_token(ptr, token, 		MAXLINE-2);
#ifdef XDEBUG
		printf("disk.BYTES_PER_BLOCK: %s\n",token);
#endif

		if (!assign_value(token, 0, 0xffffUL, &val) )
			++errors;
		disk.BYTES_PER_BLOCK = val;			// 12

		// Interleave
		ptr = get_token(ptr, token, 		MAXLINE-2);
#ifdef XDEBUG
		printf("disk.INTERLEAVE: %s\n",token);
#endif

		if (!assign_value(token, 0, 0xffffUL, &val) )
			++errors;
		disk.INTERLEAVE = val;				// 13


		if(errors)
			printf("errors: %d\n",errors);


		if(MATCH(disk.TYPE,"SS80") || MATCH(disk.TYPE,"CS80") )
		{

			if( block )
			{
				printf("%ld\n", disk.BLOCKS);
				continue;
			}
			if( dir_size )
			{
				printf("%ld\n", disk.LIF_DIR_BLOCKS);
				continue;
			}

			if ( ! LIFNAME[0] )
				sprintf(LIFNAME,"ss80-%d.lif",ppr);	
		
#ifdef XDEBUG
			printf("disk.BLOCKS: %d\n", disk.BLOCKS);
#endif

			printf(\
				"# %s\n"
                "# HP85 BASIC ADDRESS :D7%d0\n"
				"%s %s\n"
				"    HEADER\n"
				"          # GPIB Address\n"
				"        ADDRESS                 = %d\n"
				"          # Parallel Poll Reponse Bit\n"
				"        PPR                     = %d\n"
				"          # LIF image file name\n"
				"        FILE                    = %s\n"
				"    END\n"
                "\n"
				"    CONFIG\n"
				"          # Request Identify ID\n"
				"        ID                      = 0x%04x\n"
				"    END\n"
                "\n"
				"    CONTROLLER\n"
				"          # Units Installed - we only do 1\n"
				"        UNITS_INSTALLED         = 0x8001\n"
				"          # Default Transfer Rate\n"
				"        TRANSFER_RATE           = 744\n"
				"          # Single Unit COntroller\n"
				"        TYPE                    = 4\n"
				"    END\n"
                "\n"
				"    UNIT\n"
	            "            # Generic Unit Type, 0 = fixed, 1 = floppy, 2 = tape\n"
	            "            # OR with 128 implies dumb can not detect media change\n"
				"        UNIT_TYPE               = 0\n"
	            "            # BCD Device number XX XX XY, X=Unit, Y=option\n"
				"        DEVICE_NUMBER           = 0x08%x\n"
	            "            # Bytes Per Block\n"
				"        BYTES_PER_BLOCK         = %d\n"
	            "            # Bytes Per Block\n"
				"        BUFFERED_BLOCKS         = 1\n"
	            "            # Burst size = 0 for SS80\n"
				"        BURST_SIZE              = 0\n"
	            "            # Continuous average transfer rate for long transfers kB/s\n"
				"        BLOCK_TIME              = 0x1F6\n"
	            "             # Optimal retry time in 1O's of milliseconds\n"
				"        CONTINOUS_TRANSFER_RATE = 140\n"
	            "            # Optimal retry time in 1O's of milliseconds\n"
				"        OPTIMAL_RETRY_TIME      = 10000\n"
	            "            # Access time parameter in 1O's of milliseconds\n"
				"        ACCESS_TIME             = 10000\n"
	            "            # Maximum Interleave factor\n"
				"        MAXIMUM_INTERLEAVE      = 31\n"
	            "            # Fixed volume byte; one bit per volume (set if fixed)\n"
				"        FIXED_VOLUMES           = 1\n"
	            "            # Removable volume byte; one bit per volume (set if removable)\n"
				"        REMOVABLE_VOLUMES       = 1\n"
				"    END\n"
                "\n"
				"    VOLUME\n"
	            "            # Maximum Cylinder = %d = CYLINDERS-1 not used\n"
				"        MAX_CYLINDER            = 0\n"
	            "            # Maximum Head      = %d = HEADS-1 not used\n"
				"        MAX_HEAD                = 0\n"
	            "            # Maximum Sector    = %d = SECTORS-1 not used\n"
				"        MAX_SECTOR              = 0\n"
	            "            # Maximum value of single vector address in blocks.\n"
                "            #   NOTE: For devices that use both MAX_BLOCK_NUMBER and CYLINDER,HEAD,SECTOR\n"
	            "            #   Note: The follow expressions must be true if BOTH CHS and BLOCKS are used\n"
	            "            #   MAX_BLOCK_NUMBER = (MAX_CYLINDERS+1) * (MAX_HEAD+1) * (MAX_SECTOR+1) -1;\n"
	            "            #   BLOCKS = CYLINDERS * HEADS * SECTORS -1\n"
	            "            #   MAX_BLOCK_NUMBER  = BLOCKS -1\n"
	            "            #   BLOCKS = %ld\n"
				"        MAX_BLOCK_NUMBER        = %ld\n"
	            "            # Current Interleave Factor\n"
				"        INTERLEAVE              = 31\n"
				"    END\n"
                "\n"
	            "#   RESERVED DIRECTORY BLOCKS = %ld\n"
				"END\n\n",
					disk.comment,
					address,
					disk.TYPE, disk.model,
					address,
					ppr,
					LIFNAME,
					disk.ID,
					disk.DEVICE_NUMBER,
					disk.BYTES_PER_BLOCK,
					disk.MAX_CYLINDER,
					disk.MAX_HEAD,
					disk.MAX_SECTOR,
					disk.BLOCKS,
					disk.MAX_BLOCK_NUMBER,
					disk.LIF_DIR_BLOCKS );
		}	// SS80

		if( MATCH(disk.TYPE,"AMIGO") )
		{

			if ( ! LIFNAME[0] )
				sprintf(LIFNAME,"amigo%d.lif",ppr);	
#ifdef XDEBUG
			printf("disk.BLOCKS: %d\n", BLOCKS);
#endif
			if( dir_size )
			{
				printf("%ld\n", disk.LIF_DIR_BLOCKS);
				continue;
			}

			if( block )
			{
				printf("%ld\n", disk.BLOCKS);
				continue;
			}

			printf(\
				"# %s\n"
                "# HP85 BASIC ADDRESS :D7%d0\n"
				"%s %s\n"
				"    HEADER\n"
			    "            # GPIB Address\n"
				"        ADDRESS                 = %d\n"
				"            # Parallel Poll Reponse Bit\n"
				"        PPR                     = %d\n"
				"            # LIF image file name\n"
				"        FILE                    = %s\n"
				"    END\n"
                "\n"
				"    CONFIG\n"
				"            # Request Identify ID\n"
				"        ID                      = 0x%04x\n"
				"    END\n"
                "\n"
				"    GEOMETRY\n"
	            "            # Bytes Per Block\n"
				"        BYTES_PER_SECTOR        = %d\n"
	            "            # Sectors Per Track\n"
				"        SECTORS_PER_TRACK        = %d\n"
	            "            # Heads\n"
				"        HEADS                   = %d\n"
	            "            # Cylinders\n"
				"        CYLINDERS               = %d\n"
	            "            # BLOCKS = CYLINDERS * HEADS * SECTORS\n"
				"            # BLOCKS = %ld\n"
				"    END\n"
                "\n"
	            "#   RESERVED DIRECTORY BLOCKS = %ld\n"
				"END\n\n",
					disk.comment,
					address,
					disk.TYPE, disk.model,
					address,
					ppr,
					LIFNAME,
					disk.ID,
					disk.BYTES_PER_BLOCK,
					disk.SECTORS,
					disk.HEADS,
					disk.CYLINDERS,
					disk.BLOCKS,
					disk.LIF_DIR_BLOCKS );
		} // AMIGO
	}	// while

	fclose(cfg);
}
