/**
 @file gpib/format.c

 @brief LIF Disk Format.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.

*/


#include "user_config.h"

#include "defines.h"
#include "gpib.h"
#include "gpib_hal.h"
#include "gpib_task.h"
#include "amigo.h"
#include <time.h>


///@brief When formatting a disk define how many sectors we can write at once
///Depends on how much free ram we have
#define CHUNKS 16
#define CHUNK_SIZE (SECTOR_SIZE*CHUNKS)


///@brief Pack DirEntryType data into bytes
///@param[out] B: byte vector to pack data into
///@param[int] D: DirEntryType structure pointer
///@return null
uint8_t * LIFPackDir(uint8_t *B, DirEntryType *D)
{
	memcpy(B+0,D->filename,10);
	V2B_MSB(B,10,2,D->FileType);
	V2B_MSB(B,12,4,D->FileStartSector);
	V2B_MSB(B,16,4,D->FileLengthSectors);
	memcpy(B+20,D->date,6);
	V2B_MSB(B,26,2,D->VolNumber);
	V2B_MSB(B,28,2,D->SectorSize);
	V2B_MSB(B,30,2,D->implimentation);
	return(B);
}

///@brief Pack VolumeLabelType data into bytes
///@param[out] B: byte vector to pack data into
///@param[int] T: VolumeLabelType structure pointer
///@return null
void LIFPackVolume(uint8_t *B, VolumeLabelType *V)
{
	V2B_MSB(B,0,2,V->LIFid);
	memcpy((void *)(B+2),V->Label,6);
	V2B_MSB(B,8,4,V->DirStartSector);
	V2B_MSB(B,12,2,V->System3000LIFid);
	V2B_MSB(B,14,2,0);
	V2B_MSB(B,16,4,V->DirSectors);
	V2B_MSB(B,20,2,V->LIFVersion);
	V2B_MSB(B,24,4,V->tracks_per_side);
	V2B_MSB(B,28,4,V->sides);
	V2B_MSB(B,28,4,V->sectors_per_track);
	memcpy((void *) (B+36),V->date,6);
}

/// @brief Convert number >= 0 and <= 99 to BCD.
///
///  - BCD format has each hex nibble has a digit 0 .. 9
///
/// @param[in] data: number to convert.
/// @return  BCD value
/// @warning we assume the number is in range.
uint8_t BIN2BCD(uint8_t data)
{
    return(  ( (data/10U) << 4 ) | (data%10U) );
}


///@brief UNIX time to LIF time format
///@param[out] bcd: packed 6 byte BCD LIF time
///   YY,MM,DD,HH,MM,SS
///@param[in] t: UNIX time_t time value
///@see time() in time.c
///@return void
void time_to_LIF(uint8_t *bcd, time_t t)
{
	tm_t tm;
	localtime_r((time_t *) &t, (tm_t *)&tm);
	bcd[0] = BIN2BCD(tm.tm_year & 100);
	bcd[1] = BIN2BCD(tm.tm_mon);
	bcd[2] = BIN2BCD(tm.tm_mday);
	bcd[3] = BIN2BCD(tm.tm_hour);
	bcd[4] = BIN2BCD(tm.tm_min);
	bcd[5] = BIN2BCD(tm.tm_sec);
}



/// @brief Create LIF disk image
/// This can take a while to run
/// @param[in] name: LIF disk image name
/// @param[in] label: LIF Volume Label name
/// @param[in] dirsecs: Number of LIF directory sectors
/// @param[in] sectors: total disk image size in sectors
///@return bytes writting to disk image
long create_lif_image(char *name, char *label, long dirsecs, long sectors)
{
    FILE *fp;
    uint8_t *buffer;
	long size, sector,chunks;
	long l;
	int len;
	int i;
	
	VolumeLabelType V;
	DirEntryType D;

	// size of disk after volume start and directory sectors
	size = sectors - (dirsecs + 2);

	strupper(label);

	printf("Formating LIF image:[%s], Label:[%s], Dir Sectors:[%ld], sectors:[%ld]\n", 
		name, label, (long)dirsecs, (long)sectors);

	if(size < 0)
	{
		if(debuglevel & 1)
			printf("Sectors must be > 2 + Directory Sectors\n");
		return(-1);
	}


    buffer = safecalloc(CHUNK_SIZE,1);
    if(!buffer)
	{
		if(debuglevel & 1)
			printf("Can't Allocate %ld bytes memory:%ld\n", (long)CHUNK_SIZE);
        return(-1);
	}
	memset((void *) &V,0,sizeof(V));
	// Initialize volume header
	V.LIFid = 0x8000;
	for(i=0;i<6 && label[i];++i)
		V.Label[i] = label[i];	
	for(;i<6;++i)
		V.Label[i] = ' ';

	V.DirStartSector = 2;
	V.DirSectors = dirsecs;
	V.System3000LIFid = 0x1000;
	V.tracks_per_side = 0;
	V.sides = 0;
	V.sectors_per_track = 0;
	///@brief Current Date
	time_to_LIF(V.date, time(NULL));

	LIFPackVolume(buffer, (VolumeLabelType *) &V);

	///@brief Opne disk image for writting
    fp = fopen(name, "w");
    if( fp == NULL)
    {
		if(debuglevel & 1)
			printf("Can't open:%s\n", name);
		safefree(buffer);
        return( -1 );
    }

	///@brief Initial file position
	sector = 0;

	// Write Volume Header
	len = fwrite(buffer, 1, SECTOR_SIZE, fp);
	if( len < SECTOR_SIZE)
	{
		if(debuglevel & 1)
			printf("Write error %s @ %ld\n", name, sector);
		safefree(buffer);
        return( -1 );
	}
	++sector;

	// Write Empty Sector
	memset(buffer,0,SECTOR_SIZE);
	len = fwrite(buffer, 1, SECTOR_SIZE, fp);
	if( len < SECTOR_SIZE)
	{
		if(debuglevel & 1)
			printf("Write error %s @ %ld\n", name, sector);
		safefree(buffer);
        return( -1 );
	}
	++sector;

	memset((void *) &D,0,sizeof(D));
	// Fill in Directory Entry
	///@brief File type of 0xffff is last directory entry
	D.FileType = 0xffff;
	///FIXME this is the default that the HPdrive project uses, not sure of this
	D.FileLengthSectors = 0x7fffUL;

	// Fill in full Directory sector
	for(i=0;i<8;++i)
		LIFPackDir(buffer + i*32, (DirEntryType *) &D);

	// Write Directory sectors
	for(l=0;l<dirsecs;++l)
	{
		len = fwrite(buffer, 1, SECTOR_SIZE, fp);
		if( len < SECTOR_SIZE)
		{
			if(debuglevel & 1)
				printf("Write error %s @ %ld\n", name, sector);
			safefree(buffer);
			return( -1 );
		}
		++sector;
		printf("sector: %ld\r", sector);
	}

	///@brief Zero out remining disk image
	memset(buffer,0,SECTOR_SIZE);

// If we have enough memory we can write faster by using large chunks
#if CHUNKS > 1
	chunks = size / CHUNKS;
	// Write remaining in large chunks for speed
	for(i=0;i<chunks;++i)
	{
		len = fwrite(buffer, 1, CHUNK_SIZE, fp);
		if( len < CHUNK_SIZE)
		{
			if(debuglevel & 1)
				printf("Write error %s @ %ld\n", name, sector);
			safefree(buffer);
			return( -1 );
		}
		sector += CHUNKS;
		size -= CHUNKS;
		printf("sector: %ld\r", sector);
	}
#endif

	// Write remaining in large chunks for speed
	for(l=0;l<size;++l)
	{
		len = fwrite(buffer, 1, SECTOR_SIZE, fp);
		if( len < SECTOR_SIZE)
		{
			if(debuglevel & 1)
				printf("Write error %s @ %ld\n", name, sector);
			safefree(buffer);
			return( -1 );
		}
		++sector;
		printf("sector: %ld\r", sector);
	}

	fclose(fp);

    safefree(buffer);
	printf("Formating: wrote:[%ld] sectors\n", sectors);
    return(sector);
}
