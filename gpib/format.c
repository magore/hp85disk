/**
 @file gpib/format.c

 @brief LIF Disk Format.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.

*/

#include "user_config.h"

#include "defines.h"
#include "drives.h"
#include "format.h"
#include <time.h>


/// @brief Close LIF directory 
/// Modeled after Linux closedir()
/// @param[in] *DIR: pointer to LIF Volume/Directoy structure
/// @return 0 on sucesss, -1 on error
int lif_closedir(lifdir_t *DIR)
{
	if(DIR->fp)
	{
		fclose(DIR->fp);
		return(0);
	}
	return(-1);
}

/// @brief Open LIF directory for reading
/// Modeled after Linux opendir()
/// @param[in] *name: file name of LIF image
/// @return NULL on error, lifdir_t pointer to LIF Volume/Directoy structure on sucesss
static lifdir_t _lifdir;
lifdir_t *lif_opendir(char *name)
{
	int len;
	struct stat sb;

	uint8_t buffer[LIF_SECTOR_SIZE];
	lifdir_t *DIR = (lifdir_t *) &_lifdir;

	DIR->imagesize = 0;
	DIR->current = 0;
	DIR->next = 0;
	DIR->index = 0;
	DIR->used = 0;

	if(stat(name, (struct stat *)&sb) < 0)
	{
		if(debuglevel & 1)
			printf("Can't stat:%s\n", name);
        return(NULL);
	}

	if(!S_ISREG(sb.st_mode))
	{
		if(debuglevel & 1)
			printf("Not a file:%s\n", name);
        return(NULL);
	}
	DIR->imagesize = sb.st_size;;

	strncpy(DIR->filename,name,LIF_IMAGE_NAME_SIZE);

    DIR->fp = fopen(DIR->filename, "r");
    if( DIR->fp == NULL)
    {
		if(debuglevel & 1)
			printf("Can't open:%s\n", DIR->filename);
        return(NULL);
    }

	///@brief Initial file position
	len = fread(buffer, 1, LIF_SECTOR_SIZE, DIR->fp);
	if( len < LIF_SECTOR_SIZE)
	{
		if(debuglevel & 1)
			printf("Read error %s @ 0\n", DIR->filename);
        return(NULL);
	}

	// directory start sector
	LIFUnPackVolume(buffer, (VolumeLabelType *)&DIR->V);

	// stat of file area
	DIR->current = DIR->V.DirStartSector+DIR->V.DirSectors;
	DIR->next = DIR->current;

	return(DIR);
}



/// @brief Open LIF directory for reading
/// Modeled after Linux readdir()
/// @param[in] *DIR: to LIF Volume/Diractoy structure 
/// @return DirEntryType filled with directory structure, or NULL on error of end of Directory
DirEntryType *lif_readdir(lifdir_t *DIR)
{
	int len;
	long offset,end;

	DirEntryType *DE = (DirEntryType *) &DIR->DE;

	offset = (DIR->index * LIF_DIR_SIZE) + (DIR->V.DirStartSector * LIF_SECTOR_SIZE);
	end = (DIR->V.DirStartSector +  DIR->V.DirSectors) * LIF_SECTOR_SIZE;

	if(offset >= end)
		return(NULL);

	if(fseek(DIR->fp, offset, SEEK_SET) < 0)
	{
		if(debuglevel & 1)
			printf("Seek error %s @ %ld\n", DIR->filename, offset);
		return(NULL);

	}
	///@brief Initial file position
	len = fread(DIR->dirbuf, 1, sizeof(DIR->dirbuf), DIR->fp);
	if( len != sizeof(DIR->dirbuf))
	{
		if(debuglevel & 1)
			printf("Read error %s @ %ld\n", DIR->filename, offset);
		return(NULL);
	}

	LIFUnPackDir(DIR->dirbuf, DE);

	if(DE->VolNumber != 0x8001 )
		return(NULL);

	if(DE->FileType == 0xffff)
		return(NULL);

	// Update used sectors
	DIR->used += DE->FileLengthSectors;

	// Update first free sector as we read
	DIR->current = DE->FileStartSector;
	DIR->next = DE->FileStartSector + DE->FileLengthSectors;	


	DIR->index++;
	return(DE);
}


/// @brief get LIF file name from directory entry
/// @param[in] name: LIF disk image name
/// @param[in] DE: LIF directory entry
/// @retrun void
void lif_filename(char *name, DirEntryType *DE)
{
	int i;
	memcpy(name,DE->filename,10);

	name[10] =0;
	for(i=9;i>=0;--i)
	{
		if(name[i] != ' ')
			break;
		name[i] = 0;
	}
}

/// @brief get LIF file size from directory entry
/// @param[in] name: LIF disk image name
/// @param[in] DE: LIF directory entry
/// @retrun void
long lif_filelength(DirEntryType *DE)
{
	long bytes;
	bytes = DE->FileBytes;
	if(!bytes)
		bytes = (DE->FileLengthSectors * LIF_SECTOR_SIZE);
	return(bytes);
}

/// @brief Find offset to first free sector that is big enough
/// Modeled after Linux readdir()
/// @param[in] *DIR: to LIF Volume/Diractoy structure 
/// @return Directory pointer
lifdir_t *lif_find_free(char *name, long size)
{
	long bytes;
	DirEntryType *DE;
	lifdir_t *DIR;

	DIR = lif_opendir(name);

	if(DIR == NULL)
		return(NULL);

	while(1)
	{
		DE = lif_readdir(DIR);
		if(DE == NULL)
			break;
		bytes = lif_filelength(DE);
		// We can reuse this purged sector
		if(DE->FileType == 0 && size <= bytes)
			break;
	}
	lif_closedir(DIR);

 	if((DIR->next*SECTOR_SIZE) > DIR->imagesize)
		return(NULL);
	// DIR->next will point to free space
	// DIR->current points to current file
	return(DIR);		// Last valid directory entry
}

	
/// @brief Display a LIF image file directory
/// @param[in] name: LIF disk image name
/// @retrn -1 on error or number of files found
int lif_dir(char *name)
{
	DirEntryType *DE;
	long bytes;
	int files = 0;
	int purged = 0;
	lifdir_t *DIR;
	char fname[12];

	DIR = lif_opendir(name);

	if(DIR == NULL)
		return(-1);
	
	printf("NAME        TYPE   START SECTOR        SIZE RECSIZE\n");
	while(1)
	{
		DE = lif_readdir(DIR);
		if(DE == NULL)
			break;

		//printf("Bytes:%d, FileLengthSectors:%ld\n", (int)DE->FileBytes, (long) DE->FileLengthSectors);
		bytes = lif_filelength(DE);
		lif_filename(fname, DE);

		// name type start size
		printf("%-10s  %04x       %08lxH  %9ld    %4d\n", 
			fname, 
			(int)DE->FileType, 
			(long)DE->FileStartSector, 
			(long)bytes, 
			(int)DE->SectorSize  );

		if(DE->FileType != 0)
			++files;
		else
			++purged;
	}	

	printf("\n");
	printf("Files:         %4d\n", (int)files);
	printf("Purged:        %4d\n", (int)purged);
	printf("Used Sectors: %4lxH\n", DIR->used);
	printf("First Free:   %4lxH\n", DIR->next);

	lif_closedir(DIR);
	return(files);
}

/// @brief Create LIF disk image
/// This can take a while to run
/// @param[in] name: LIF disk image name
/// @param[in] label: LIF Volume Label name
/// @param[in] dirsecs: Number of LIF directory sectors
/// @param[in] sectors: total disk image size in sectors
///@return bytes writting to disk image
long lif_create_image(char *name, char *label, long dirsecs, long sectors)
{
    FILE *fp;
    uint8_t *buffer;
	long remainder, sector,chunks;
	long li;
	int len;
	int i;
	
	VolumeLabelType V;
	DirEntryType DE;


	///@brief Setup Volume Header
	strupper(label);
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

	printf("Formating LIF image:[%s], Label:[%s], Dir Sectors:[%ld], sectors:[%ld]\n", 
		name, label, (long)V.DirSectors, (long)sectors);

	// Size of of disk after volume start and directory sectors hae been written
	remainder = sectors - (V.DirSectors + V.DirStartSector);
	if(remainder < 0)
	{
		if(debuglevel & 1)
			printf("Too few sectors specified in image fil\n");
		return(-1);
	}

	///@brief Opne disk image for writting
    fp = fopen(name, "w");
    if( fp == NULL)
    {
		if(debuglevel & 1)
			printf("Can't open:%s\n", name);
        return( -1 );
    }

	///@brief Initial file position
	sector = 0;

	///@brief Allocate working buffer for all writes
    buffer = safecalloc(LIF_CHUNK_SIZE,1);
    if(!buffer)
	{
		if(debuglevel & 1)
			printf("Can't Allocate %ld bytes memory:%ld\n", (long)LIF_CHUNK_SIZE);
		fclose(fp);
        return(-1);
	}
	///@brief store Volume data into buffer
	LIFPackVolume(buffer, (VolumeLabelType *) &V);

	// Write Volume Header
	len = fwrite(buffer, 1, LIF_SECTOR_SIZE, fp);
	if( len < LIF_SECTOR_SIZE)
	{
		if(debuglevel & 1)
			printf("Write error %s @ %ld\n", name, sector);
		fclose(fp);
		safefree(buffer);
        return( -1 );
	}
	++sector;

	// Write Empty Sector
	memset(buffer,0,LIF_SECTOR_SIZE);
	for(li=1; (uint32_t) li < V.DirStartSector; ++li)
	{
		len = fwrite(buffer, 1, LIF_SECTOR_SIZE, fp);
		if( len < LIF_SECTOR_SIZE)
		{
			if(debuglevel & 1)
				printf("Write error %s @ %ld\n", name, sector);
			fclose(fp);
			safefree(buffer);
			return( -1 );
		}
		++sector;
	}

	memset((void *) &DE,0,sizeof(DE));
	// Fill in Directory Entry
	///@brief File type of 0xffff is last directory entry
	DE.FileType = 0xffff;
	///FIXME this is the default that the HPdrive project uses, not sure of this
	DE.FileLengthSectors = 0x7fffUL;

	// Fill sector with Directory entries
	for(i=0; i<LIF_SECTOR_SIZE; i+= LIF_DIR_SIZE)
		LIFPackDir(buffer + i, (DirEntryType *) &DE);

	// Write Directory sectors
	for(li=0;(uint32_t) li< V.DirSectors;++li)
	{
		len = fwrite(buffer, 1, LIF_SECTOR_SIZE, fp);
		if( len < LIF_SECTOR_SIZE)
		{
			if(debuglevel & 1)
				printf("Write error %s @ %ld\n", name, sector);
			fclose(fp);
			safefree(buffer);
			return( -1 );
		}
		++sector;
		printf("sector: %ld\r", sector);
	}

	///@brief Zero out remining disk image
	memset(buffer,0,LIF_SECTOR_SIZE);

// If we have enough memory we can write faster by using large chunks
#if LIF_CHUNKS > 1
	chunks = remainder / LIF_CHUNKS;
	// Write remaining in large chunks for speed
	for(li=0;li<chunks;++li)
	{
		len = fwrite(buffer, 1, LIF_CHUNK_SIZE, fp);
		if( len < LIF_CHUNK_SIZE)
		{
			if(debuglevel & 1)
				printf("Write error %s @ %ld\n", name, sector);
			fclose(fp);
			safefree(buffer);
			return( -1 );
		}
		sector += LIF_CHUNKS;
		remainder -= LIF_CHUNKS;
		printf("sector: %ld\r", sector);
	}
#endif

	// Write remaining in large chunks for speed
	for(li=0;li<remainder;++li)
	{
		len = fwrite(buffer, 1, LIF_SECTOR_SIZE, fp);
		if( len < LIF_SECTOR_SIZE)
		{
			if(debuglevel & 1)
				printf("Write error %s @ %ld\n", name, sector);
			fclose(fp);
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
