#ifndef _DRIVES_SUP_H
#define _DRIVES_SUP_H
/// =================================================
///@brief Data structure for hpdir.ini HPDir project disk parameters 
///
/// This structure will containes the parsed and computed
/// disk parameters from a single hpdir disk entry
///
///; AMIGIO/CS80/SS80 disk drives
///; 1 - SS80 [HP9134L]            unique drive model identifier (should be identical to [fsinfo] section above)
///; 2 - COMMENT                   descriptive string
///; 3 - SS80/AMIGO:               drive protocol AMIGO, CS80 or SS80
///; 4 - ID:                       ID returned by AMIGO identify
///; 5 -                           mask for secondary ID returned by high byte of STAT2
///; 6 -                           ID returned by high byte of STAT2
///; 7 - DEVICE_NUMBER:            drive number returned by CS/80 describe command
///; 8 - UNITS_INSTALLED(0x8001):  number of units installed
///; 9 - MAX_CYLINDER:             number of cylinders
///;10 - MAX_HEAD:                 number of heads or surfaces
///;11 - MAX_SECTOR:               number of sectors per track
///;12 - BYTES_PER_BLOCK:          number of bytes per block or sector
///;13 - INTERLEAVE:               default interleave
///;14 - media type 0=fixed media, 1=removable media
///; # Removable volume byte; one bit per volume (set if removable)
///; 1,                                           x,           3,      4,    5,    6,        7, 8,    9, 10, 11,  12,13,14
/// Example entry
///9895    = "HP9895A dual 1.15M AMIGO floppy disc",       AMIGO, 0x0081, 0x00, 0x00, 0x000000, 1,   77,  2, 30, 256, 7, 1
///
///@return void

#define MODEL_SIZE 32

typedef struct {
    char model[MODEL_SIZE];	// 1 HP Model number
    char comment[64];  		// 2
	char TYPE[32];     		// 3 SS80,CS80,AMIGO
    long ID;				// 4 Request Identify ID
	long mask_stat2;		// 5
	long id_stat2;			// 6
	long DEVICE_NUMBER;		// 7 BCD part of model number * 10
	long UNITS_INSTALLED; 	// 8	ALWAYS 1 , FIXED
	long CYLINDERS;	    	// 9
	long HEADS;				// 10
	long SECTORS;			// 11
	long BYTES_PER_SECTOR;	// 12
	long INTERLEAVE;		// 13
    long FIXED;				// 14 ALWAYS 1
	// Computed values
	long BLOCKS;
	long LIF_DIR_BLOCKS;
} hpdir_t;
// =============================================
/* drives_sup.c */
void hpdir_init ( void );
long lif_dir_count ( long blocks );
int hpdir_find_drive ( char *model , int list , int verbose);
#endif

