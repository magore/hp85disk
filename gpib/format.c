/**
 @file gpib/format.c

 @brief LIF Disk Format.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/


#include "user_config.h"

#include "defines.h"
#include "gpib.h"
#include "gpib_hal.h"
#include "gpib_task.h"
#include "amigo.h"


void SS80_V2B(uint8_t *B, int index,int size, uint32_t val)
{
    ///@brief remove 1 bias
    V2BMSB(B, index-1,size, val);
}



/**
 @brief Disk Layout
 @credits https://groups.io/g/hpseries80/wiki/HP-85-Program-Control-Block-(BASIC-header),-Tape-directory-layout,-Disk-directory-layout

	DISK LAYOUT
	The HP-85 disks used the LIF (Logical Interchange Format) disk layout.  The first 2 sectors on the disk (cylinder 0, head 0, sector 0-1) contained the VOLUME sectors.  The important things in the VOLUME SECTORS were thus:

	BYTES   DESCRIPTION
	-----   -----------------------------------------------------------
	  0-1   LIF identifier, must be 0x80, 0x00 (0x8000)
	  2-7   6-character volume LABEL
	 8-11   directory start block (always 0,0,0,2 = 0x00000002)
	12-13   LIF identifer for System 3000 machines (always 0x10,0 = 0x1000)
	14-15   always 0
	16-19   # of sectors in DIRECTORY (usually 0,0,0,something)
	20-21   LIF version number (always 0,1 = 0x0001)
	22-23   always 0
	24-27   number of tracks per surface
	28-31   number of surfaces
	32-35   number of sectors per track
	36-41   date and time that the volume was initialized (YY,MM,DD,HH,mm,SS)
			All date values are in BCD format.  YY is (year-1900). HH is 0-23.
*/


/// @brief LIF data structures used by stand alone formatting utility

/// @brief LIF Volume label structure
/// - Reference: hpdisk (c) 2014 Anders Gustafsson <anders.gustafsson@pedago.fi>
/// - LIF filesystem http://www.hp9845.net/9845/projects/hpdir/#lif_filesystem
/// - LIF data is BIG endian

VolumeLabelType vl =
{
    0x0080,
    { 'L','A','B','E','L','1'},
    0,0x200,
    0x0080,
    0,
    0,0x0e00,       // Changed this to 0x0e -> LIF directory too big
    0
};

/// @brief LIF DIrectory Entry
/// - Reference: hpdisk (c) 2014 Anders Gustafsson <anders.gustafsson@pedago.fi>
/// - LIF filesystem http://www.hp9845.net/9845/projects/hpdir/#lif_filesystem
/// - LIF data is BIG endian
DirEntryType de =
{
    { 'M','Y','F','I','L','E','1','2','3','4'},
    0x100,0,0x100,0,0x20,
    {0x13,0x10,0x12,0x00,0x00,0x00},
    1,0,0
};

/// @brief LDIF Format file.
///
/// @param[in] name: file name of LIF image to format.
///
/// @todo  currently coded for AMIGO - Work in progress.
/// @return  FatFs FRESULT.

FRESULT gpib_format_disk(char *name, uint32_t size)
{
    FRESULT rc;
    UINT nbytes;
    DWORD fpos, max;                              // File position
    FIL fp;
    char *buffer;

    buffer = safecalloc(512+1,1);
    if(!buffer)
    {
        return(FR_NOT_ENOUGH_CORE);
    }

	if(debuglevel & 32)
		printf("Format:[%s]\n", name);

    rc = dbf_open(&fp, name, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if(rc)
        return (rc);

    memset(buffer,0,10);
    rc=dbf_read(&fp, buffer,10,&nbytes);

/// @return return(rc);
	if(debuglevel & 32)
		printf("Read (%d) bytes:[%s]\n", nbytes, buffer);

    memset(buffer,' ',512);
    memcpy(buffer,&vl,sizeof(vl));                // Volume

    fpos = 0;
    rc=dbf_lseek(&fp,fpos);
    if(rc)
    {
        safefree(buffer);
        return(rc);
    }

    rc=dbf_write(&fp, buffer, 512, &nbytes);
    if(rc)
    {
        safefree(buffer);
        return(rc);
    }

    fpos = (DWORD)2*512L;
    memset(buffer,' ',512);
    de.filetype = 0x100;
    memcpy(buffer,&de,sizeof(de));
    memcpy(buffer+sizeof(de),&de,sizeof(de));
    de.filetype = 0xffff;
    memcpy(buffer+sizeof(de)+sizeof(de),&de,sizeof(de));

    rc=dbf_lseek(&fp, fpos);
    if(rc)
    {
        safefree(buffer);
        return(rc);
    }

    rc=dbf_write(&fp, buffer, 512, &nbytes);
    if(rc)
    {
        safefree(buffer);
        return(rc);
    }

    fpos = f_tell(fp);
    memset(buffer,' ',512);
    while(fpos < size)
    {
        rc=dbf_write(&fp, buffer, 512, &nbytes);
        if(rc)
        {
            safefree(buffer);
            return(rc);
        }
    }
    rc= dbf_close(&fp);
	if(debuglevel & 32)
		printf("Done\n");
    safefree(buffer);
    return(rc);
}
