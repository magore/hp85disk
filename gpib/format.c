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

FRESULT gpib_format_disk(char *name)
{
    FRESULT rc;
    UINT nbytes;
    DWORD fpos, max;                              // File position
    FIL fp;
    char *buffer;
    extern VolumeDescriptionType VolumeDescription;

    buffer = safecalloc(512+1,1);
    if(!buffer)
    {
        return(FR_NOT_ENOUGH_CORE);
    }

    printf("Format:[%s]\n", name);

    rc = dbf_open(&fp, name, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if(rc)
        return (rc);

    memset(buffer,0,10);
    rc=dbf_read(&fp, buffer,10,&nbytes);

/// @return return(rc);
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

    max = VolumeDescription.V9 * 0x100000000L +
        VolumeDescription.V10 * 0x1000000L +
        VolumeDescription.V11 * 0x10000L +
        VolumeDescription.V12 * 0x100L;

    fpos = f_tell(fp);
    memset(buffer,' ',512);
    while(fpos < max )
    {
        rc=dbf_write(&fp, buffer, 512, &nbytes);
        if(rc)
        {
            safefree(buffer);
            return(rc);
        }
    }
    rc= dbf_close(&fp);
    printf("Done\n");
    safefree(buffer);
    return(rc);
}
