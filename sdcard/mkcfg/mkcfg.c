#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <inttypes.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <libgen.h>

#define MAXLINE 256
#define bool int

#include "../../lib/stringsup.c"
#include "../../gpib/drives_sup.c"


// =============================================
///@brief List Disk Parameters
///
/// - Looks for hpdir.ini in current, root or program install directory 
///
///@param[in] ppr: Parallel Poll Bit Number
///@param[in] address: GPIB address
///@param[in] lifename: LIF file name to use for image
///
///@return void
void config_list( int ppr, int address, char *lifname, int detail)
{

	if(MATCH(hpdir.TYPE,"SS80") || MATCH(hpdir.TYPE,"CS80") )
	{

		if ( ! lifname[0] )
			sprintf(lifname,"ss80-%d.lif",ppr);	
	
#ifdef XDEBUG
		printf("hpdir.BLOCKS: %ld\n", hpdir.BLOCKS);
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
			"    END\n",
				hpdir.comment,
				address,
				hpdir.TYPE, hpdir.model,
				address,
				ppr,
				lifname );

		if(detail)
		{
			printf(\
				"    CONFIG\n"
				"          # Request Identify ID\n"
				"        ID                      = 0x%04lx\n"
				"    END\n"
				"\n"
				"\n"
				"    UNIT\n"
				"            # BCD Device number XX XX XY, X=Unit, Y=option\n"
				"        DEVICE_NUMBER           = 0x%08lx\n"
				"        BYTES_PER_SECTOR        = %ld\n"
				"    END\n"
				"\n"
				"    VOLUME\n"
				"            # Maximum Cylinder = %ld = CYLINDERS-1 not used\n"
				"        MAX_CYLINDER            = 0\n"
				"            # Maximum Head      = %ld = HEADS-1 not used\n"
				"        MAX_HEAD                = 0\n"
				"            # Maximum Sector    = %ld = SECTORS-1 not used\n"
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
				"    END\n"
				"#   RESERVED DIRECTORY BLOCKS = %ld\n",
					hpdir.ID,
					hpdir.DEVICE_NUMBER,
					hpdir.BYTES_PER_SECTOR,
					hpdir.CYLINDERS,
					hpdir.HEADS,
					hpdir.SECTORS,
					hpdir.BLOCKS,
					hpdir.BLOCKS-1,
					hpdir.LIF_DIR_BLOCKS );
		}	
		printf("END\n\n");
	}	// SS80

	if( MATCH(hpdir.TYPE,"AMIGO") )
	{

		if ( ! lifname[0] )
			sprintf(lifname,"amigo%d.lif",ppr);	

#ifdef XDEBUG
		printf("hpdir.BLOCKS: %ld\n", hpdir.BLOCKS);
#endif
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
			"    END\n",
				hpdir.comment,
				address,
				hpdir.TYPE, hpdir.model,
				address,
				ppr,
				lifname );
		if(detail)
		{
				printf(\
				"    CONFIG\n"
				"            # Request Identify ID\n"
				"        ID                      = 0x%04lx\n"
				"    END\n"
				"\n"
				"    GEOMETRY\n"
				"            # Bytes Per Block\n"
				"        BYTES_PER_SECTOR        = %ld\n"
				"            # Sectors Per Track\n"
				"        SECTORS_PER_TRACK        = %ld\n"
				"            # Heads\n"
				"        HEADS                   = %ld\n"
				"            # Cylinders\n"
				"        CYLINDERS               = %ld\n"
				"            # BLOCKS = CYLINDERS * HEADS * SECTORS\n"
				"            # BLOCKS = %ld\n"
				"    END\n"
				"\n"
				"#   RESERVED DIRECTORY BLOCKS = %ld\n",
					hpdir.ID,
					hpdir.BYTES_PER_SECTOR,
					hpdir.SECTORS,
					hpdir.HEADS,
					hpdir.CYLINDERS,
					hpdir.BLOCKS,
					hpdir.LIF_DIR_BLOCKS );
		}
		printf("END\n\n");
	} // AMIGO

}


// =============================================
///@brief Help Usage
///
/// - Display Help Usage message 
///
///@return void
void usage(char *ptr)
{
	printf("%s [-list]| [-m model [-b]|[-d]] [-a address]\n", ptr);
	printf("   -list lists all of the drives in the hpdir.ini file\n");
	printf("   -a hpdir address 0..7\n");
	printf("   -m model only, list hphpdir.cfg format hpdir configuration\n");
	printf("   -s short hphpdir.cfg format\n");
	printf("   -b only display block count, you can can use this with -m\n");
	printf("   -d only display computed directory block count, you can use this with -m\n");
	printf("   -f NAME specifies the LIF image name for this drive\n");
}

// =============================================
/// @brief main 
/// process arguments and optionally display configuration or selected parameters
/// @return  status
int main(int argc, char *argv[])
{
	char *ptr;
	int i,len,ret;

	int ppr = 0;
	int address = 0;
	int detail = 1;

	int blocks = 0;
	int dir_size = 0;
	int list = 0;

	FILE *cfg;
    char lifname[MAXLINE];
    char model[MAXLINE];
    char token[MAXLINE];	// 1
	model[0] = 0;
	lifname[0] = 0;

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
		if(MATCH(ptr,"-s"))
		{
			detail = 0;
			continue;
		}

		if(MATCH(ptr,"-m"))
		{
			ptr = argv[++i];
			if(!ptr)
				break;
			strncpy(model,ptr,sizeof(model)-2);
			continue;
		}

		if(MATCH(ptr,"-f"))
		{
			ptr = argv[++i];
			if(!ptr)
				break;

			strncpy(lifname,ptr,sizeof(lifname)-2);
			continue;
		}

		if(MATCH(ptr,"-a"))
		{
			ptr = argv[++i];
			if(!ptr)
				break;
			address = atoi(ptr);
			// printf("#Adress=%d\n",address);
			continue;
		}

		if(MATCH(ptr,"-p"))
		{
			ptr = argv[++i];
			if(!ptr)
				break;
			ppr = atoi(ptr);
			// printf("#Adress=%d\n",ppr);
			continue;
		}

		if(MATCH(ptr,"-b"))
		{
			blocks = 1;
			continue;
		}

		if(MATCH(ptr,"-d"))
		{
			dir_size = 1;
		}
	}

	hpdir_init();

	ret = hpdir_find_drive(model, list, 0);

	// Success ?
	if(ret && !list)
	{
		if(dir_size)
			printf("%ld\n",lif_dir_count(hpdir.BLOCKS));
		else if(blocks)
			printf("%ld\n",hpdir.BLOCKS);
		else if(model[0] )
			config_list(address,ppr, lifname, detail);
	}
	return(0);
}

