/**
 @file gpib/format.c

 @brief LIF Disk Format.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

/// @brief LDIF Format file.
///
/// @param[in] name: file name of LIF image to format.
///
/// @todo  Work in progress.
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

    myprintf("Format:[%s]\n", name);

    rc = dbf_open(&fp, name, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if(rc)
        return (rc);

    memset(buffer,0,10);
    rc=dbf_read(&fp, buffer,10,&nbytes);

/// @return return(rc);
    myprintf("Read (%d) bytes:[%s]\n", nbytes, buffer);

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
    myprintf("Done\n");
    safefree(buffer);
    return(rc);
}
