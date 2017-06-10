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


///@brief Pack VolumeLabelType data into bytes
///@param[out] B: byte vector to pack data into
///@param[int] T: VolumeLabelType structure pointer
///@return null
void LIFPackVolume(uint8_t *B, VolumeLabelType *T)
{
	V2B_MSB(B,0,2,T->LIFid);
	memcpy(B+2,T->Label,6);
	V2B_MSB(B,8,4,T->DirStartSector);
	V2B_MSB(B,12,2,T->System3000LIFid);
	V2B_MSB(B,14,2,T->zero1);
	V2B_MSB(B,16,4,T->DirSectors);
	V2B_MSB(B,20,2,T->LIFVersion);
	V2B_MSB(B,22,2,T->zero2);
	V2B_MSB(B,24,4,T->tracks_per_size);
	V2B_MSB(B,28,4,T->sides);
	memcpy(B+36,6,T->date,6);
	return(B);
}

///@brief Pack DirEntryType data into bytes
///@param[out] B: byte vector to pack data into
///@param[int] D: DirEntryType structure pointer
///@return null
uint8_t * LIFPackDir(uint8_t *B, DirEntryType *D)
{
	memcpy(B+0,D->filename,10);
	V2B_MSB(B,10,2,D->FileTYpe);
	V2B_MSB(B,12,4,D->FileStartSector);
	V2B_MSB(B,16,4,D->FileLengthSectors);
	memcpy(B+20,D->date,6);
	V2B_MSB(B,26,2,D->VolNumber);
	V2B_MSB(B,28,2,D->SectorSize);
	V2B_MSB(B,30,2,D->implimentation);
	return(B);
}


/// @brief Format LIF disk
/// TODO
/// @param[in] name: file name of LIF image to format.
///
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
