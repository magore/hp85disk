/**
 @file gpib/lifutils.c

 @brief LIF file utilities

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.

*/

#include "user_config.h"

#include "defines.h"
#include "drives.h"
#include <time.h>
#include "lifutils.h"


/// @brief
///  Help Menu for User invoked GPIB functions and tasks
///  See: int gpib_tests(char *str)
/// @return  void

void lif_help()
{
    printf(
        "lifadd lifimage lifname file\n"
        "lifcreate lifimage label directory_sectors sectors\n"
        "lifdir\n"
        );
}

/// @brief LIFGuser tests
/// @return  1 matched token, 0 if not
int lif_tests(char *str)
{

    int len;
    char *ptr;

    ptr = skipspaces(str);

    if ((len = token(ptr,"lifadd")) )
    {
        char name[64];
        char lifname[64];
        char user[64];

        ptr += len;

        // IMAGE name
        ptr = get_token(ptr, name, 63);

        // lif file name
        ptr = get_token(ptr, lifname, 63);

        // User file name
        ptr = get_token(ptr, user, 63);

        lif_add_file ( name, lifname, user);

        return(1);
    }
    else if ((len = token(ptr,"lifdir")) )
    {
        char name[64];
        ptr += len;
        // IMAGE name
        ptr = get_token(ptr, name, 63);
        lif_dir(name);
        return(1);
    }
    else if ((len = token(ptr,"lifcreate")) )
    {
        char name[64],label[6];
        char num[12];
        long dirsecs, sectors, result;

        ptr += len;

        // IMAGE name
        ptr = get_token(ptr, name, 63);

        // IMAGE LABEL
        ptr = get_token(ptr, label, 7);

        // Directory Sectors
        ptr = get_token(ptr, num, 11);
        dirsecs = atol(num);

        // Image total Sectors
        ptr = get_token(ptr, num, 11);
        sectors= atol(num);

        ///@brief format LIF image
        result = lif_create_image(name,label,dirsecs,sectors);
        if(result != sectors)
        {
            if(debuglevel & 1)
                printf("create_format_image: failed\n");
        }
        return(1);
    }
	return(0);
}


/// @brief Convert LIF space padded string name into normal string
/// @param[in] *B: LIF name space padded
/// @param[out] *name: string result
/// @retrun void
void lif_B2S(uint8_t *B, uint8_t *name, int size)
{
    int i;

	for(i=0;i<size;++i)
		name[i] = B[i];
	name[i] = 0;
	trim_tail((char *)name);
}

/// @brief string to LIF directory entry
/// @param[out] *B: LIF result
/// @param[in] *name: string
/// @retrun void
void lif_S2B(uint8_t *B, uint8_t *name, int size)
{
    int i;
    for(i=0;name[i] && i<size;++i)
        B[i] = name[i];

    for(;i<size;++i)
        B[i] = ' ';
}

///@brief Convert a file name (unix/fat32) format into a valid LIF name 
/// First we do a basename on the string (remove any leading file paths)
/// Next we remove any extensions
/// LIF names may have only these characters: [A-Z][A-Z0-09_]+
/// LIF names are converted to upper case
/// LIF names are padded at the end with spaces
///@param[out] *B: output LIF string
///@param[in] *name: input string
///@param[in] size: maximum size of output string
///@return length of result
int lif_fixname(uint8_t *B, char *name, int size)
{
	uint8_t c;
	int i,index;
	char *ptr;
	uint8_t *save = B;

	index = 0;
	ptr = basename(name);

	for(i=0; ptr[i] && index < size;++i)
	{
		c = ptr[i];
		// trim off extensions
		if(c == '.')
			break;
		if(c >= 'a' && c <= 'z')
		{
			*B++ = c - 0x20;
			++index;
			continue;
		}
		if(c >= 'A' && c <= 'Z')
		{
			*B++ = c;
			++index;
			continue;
		}
		if((c >= '0' && c <= '9') || c == '_')
		{
			if(index > 0)
			{
				*B++ = c;
				++index;
				continue;
			}
		}
		// skip all other characters
	}
	while(index < size)
	{
		*B++ = ' ';
		++index;
	};
	*B = 0;
	return(strlen((char *)save));
}


///@brief Pack lifvol_t data into bytes
///@param[out] B: byte vector to pack data into
///@param[int] T: lifvol_t structure pointer
///@return null
void lif_PackVolume(uint8_t *B, lifvol_t *V)
{
	V2B_MSB(B,0,2,V->LIFid);
	lif_S2B(B+2,V->Label,6);
	V2B_MSB(B,8,4,V->DirStartSector);
	V2B_MSB(B,12,2,V->System3000LIFid);
	V2B_MSB(B,14,2,0);
	V2B_MSB(B,16,4,V->DirSectors);
	V2B_MSB(B,20,2,V->LIFVersion);
	V2B_MSB(B,22,2,0);
	V2B_MSB(B,24,4,V->tracks_per_side);
	V2B_MSB(B,28,4,V->sides);
	V2B_MSB(B,32,4,V->sectors_per_track);
	memcpy((void *) (B+36),V->date,6);
}

///@brief UnPack lifvol_t data from bytes
///@param[in] B: byte vector to pack data into
///@param[out] V: lifvol_t structure pointer
///@return null
void lif_UnPackVolume(uint8_t *B, lifvol_t *V)
{
	V->LIFid = B2V_MSB(B,0,2);
	lif_B2S(B+2,V->Label,6);
	V->DirStartSector = B2V_MSB(B,8,4);
	V->System3000LIFid = B2V_MSB(B,12,2);
	V->zero1 = B2V_MSB(B,14,2);
	V->DirSectors = B2V_MSB(B,16,4);
	V->LIFVersion = B2V_MSB(B,20,2);
	V->zero2 = B2V_MSB(B,22,2);
	V->tracks_per_side = B2V_MSB(B,24,4);
	V->sides = B2V_MSB(B,28,4);
	V->sectors_per_track = B2V_MSB(B,32,4);
	memcpy((void *) V->date, (B+36),6);
}

///@brief Pack DireEntryType data into bytes
///@param[out] B: byte vector to pack data into
///@param[int] D: lifdirent_t structure pointer
///@return null
void lif_PackDir(uint8_t *B, lifdir_t *DIR)
{
	lif_S2B(B,DIR->DE.filename,10);				// 0
	V2B_MSB(B,10,2,DIR->DE.FileType);			// 10
	V2B_MSB(B,12,4,DIR->DE.FileStartSector);	// 12
	V2B_MSB(B,16,4,DIR->DE.FileLengthSectors);	// 16
	memcpy(B+20,DIR->DE.date,6);				// 20
	V2B_MSB(B,26,2,DIR->DE.VolNumber);			// 26
	V2B_LSB(B,28,2,DIR->DE.FileBytes);			// 28
	V2B_LSB(B,30,2,DIR->DE.SectorSize);			// 30
}

///@brief UnPack lifdirent_t data from bytes
///@param[in] B: byte vector to extract data from
///@param[int] D: lifdirent_t structure pointer
///@return null
void lif_UnPackDir(uint8_t *B, lifdir_t *DIR)
{
	lif_B2S(B,DIR->DE.filename,10);
	DIR->DE.FileType = B2V_MSB(B, 10, 2);
	DIR->DE.FileStartSector = B2V_MSB(B, 12, 4);
	DIR->DE.FileLengthSectors = B2V_MSB(B, 16, 4);
	memcpy(DIR->DE.date,B+20,6);
	DIR->DE.VolNumber = B2V_MSB(B, 26, 2); 
	DIR->DE.FileBytes = B2V_LSB(B, 28, 2);
	DIR->DE.SectorSize= B2V_LSB(B, 30, 2);
}

/// @brief Convert number >= 0 and <= 99 to BCD.
///
///  - BCD format has each hex nibble has a digit 0 .. 9
///
/// @param[in] data: number to convert.
/// @return  BCD value
/// @warning we assume the number is in range.
uint8_t lif_BIN2BCD(uint8_t data)
{
    return(  ( (data/10U) << 4 ) | (data%10U) );
}


///@brief UNIX time to LIF time format
///@param[out] bcd: packed 6 byte BCD LIF time
///   YY,MM,DD,HH,MM,SS
///@param[in] t: UNIX time_t time value
///@see time() in time.c
///@return void
void lif_time2lif(uint8_t *bcd, time_t t)
{
	tm_t tm;
	localtime_r((time_t *) &t, (tm_t *)&tm);
	bcd[0] = lif_BIN2BCD(tm.tm_year & 100);
	bcd[1] = lif_BIN2BCD(tm.tm_mon);
	bcd[2] = lif_BIN2BCD(tm.tm_mday);
	bcd[3] = lif_BIN2BCD(tm.tm_hour);
	bcd[4] = lif_BIN2BCD(tm.tm_min);
	bcd[5] = lif_BIN2BCD(tm.tm_sec);
}

/// @brief Clear main lifdir_t DIR structure 
/// @param[in] *DIR: pointer to DIR structure
/// @return void
void lif_dir_clear(lifdir_t *DIR)
{
	memset((void *) DIR,0,sizeof(lifdir_t));
}


/// @brief Clear lifdirent_t DE structure in DIR
/// @param[in] *DIR: pointer to DIR structure
/// @return void
void lif_dirent_clear(lifdir_t *DIR)
{
	memset((void *) &DIR->DE,0,sizeof(lifdirent_t));
}

/// @brief Clear lifvol_t V structure in DIR
/// @param[in] *DIR: pointer to DIR structure
/// @return void
void lif_vol_clear(lifdir_t *DIR)
{
	memset((void *) &DIR->V,0,sizeof(lifvol_t));
}

/// @brief Close LIF directory 
/// Modeled after Linux closedir()
/// @param[in] *DIR: pointer to LIF Volume/Directoy structure
/// @return 0 on sucesss, -1 on error
int lif_closedir(lifdir_t *DIR)
{
	//lif_dir_clear(DIR);
	return(0);
}

/// @brief Open a file that must exist
/// @param[in] *name: file name of LIF image
/// @param[in] *mode: open mode - see fopen
/// @return FILE * pointer
FILE *lif_open(char *name, char *mode)
{
    FILE *fp = fopen(name, mode);
    if( fp == NULL)
    {
		if(debuglevel & 1)
			printf("lif_open: Can't open:[%s] mode:[%s]\n", name, mode);
        return(NULL);
    }
	return(fp);
}
/// @brief Stat a file 
/// @param[in] *name: file name of LIF image
/// @return struct stat *
stat_t *lif_stat(char *name)
{
	static stat_t _sb;
	stat_t *p = (stat_t *) &_sb;

	if(stat(name, p) < 0)
	{
		if(debuglevel & 1)
			printf("lif_stat: Can't stat:%s\n", name);
        return(NULL);
	}
	return(p);
}



/// @brief Read data from a LIF image 
/// File is closed after read
/// WHY? We want time minimize file open to to avoid corruption on the SDCARD
/// @param[in] *name: file name of LIF image
/// @param[in] *buf: read buffer
/// @param[in] offset: read offset
/// @param[in] bytes: number of bytes to read
/// @return number of bytes read - OR - -1 on error
long lif_read(char *name, void *buf, long offset, int bytes)
{
	FILE *fp;
	long len;

	fp = lif_open(name, "r");
    if( fp == NULL)
        return(-1);

	if(fseek(fp, offset, SEEK_SET) < 0)
	{
		if(debuglevel & 1)
			printf("lif_read: Seek error %s @ %ld\n", name, offset);
		fclose(fp);
		return(-1);
	}

	///@brief Initial file position
	len = fread(buf, 1, bytes, fp);
	if( len != bytes)
	{
		if(debuglevel & 1)
			printf("lif_read: Read error %s @ %ld\n", name, offset);
		fclose(fp);
		return(-1);
	}
	fclose(fp);
	return(len);
}

/// @brief Write data to an LIF image 
/// File is closed, positioned to the end of file, after write
/// Why? We want time minimize file open to to avoid corruption on the SDCARD
/// @param[in] *name: file name of LIF image
/// @param[in] *buf: write buffer
/// @param[in] offset: write offset
/// @param[in] bytes: number of bytes to write
/// @return -1 on error or number of bytes written 
int lif_write(char *name, void *buf, long offset, int bytes)
{
	FILE *fp;
	long len;

	fp = lif_open(name, "r+");
    if( fp == NULL)
        return(-1);

	// Seek to write position
	if(fseek(fp, offset, SEEK_SET) < 0)
	{
		if(debuglevel & 1)
			printf("lif_write: Seek error %s @ %ld\n", name, offset);

		// try to seek to the end of file anyway
		fseek(fp, 0, SEEK_END);
		fclose(fp);
		return(-1);

	}

	///@brief Initial file position
	len = fwrite(buf, 1, bytes, fp);
	if( len != bytes)
	{
		if(debuglevel & 1)
			printf("lif_write: Write error %s @ %ld\n", name, offset);

		// seek to the end of file before close!
		fseek(fp, 0, SEEK_END);
		fclose(fp);
		return(len);
	}

	// seek to the end of file before close!
	fseek(fp, 0, SEEK_END);
	fclose(fp);
	return(len);
}


/// @brief Open LIF directory for reading
/// Modeled after Linux opendir()
/// @param[in] *name: file name of LIF image
/// @return NULL on error, lifdir_t pointer to LIF Volume/Directoy structure on sucesss
static lifdir_t _lifdir;
lifdir_t *lif_opendir(char *name)
{
	int len;
	stat_t *sb;
	lifdir_t *DIR = (lifdir_t *) &_lifdir;
	uint8_t buffer[LIF_SECTOR_SIZE];

	lif_dir_clear(DIR);

	sb = lif_stat(name);
	if(sb == NULL)
        return(NULL);

	DIR->imagesize = sb->st_size;
	strncpy(DIR->filename,name,LIF_IMAGE_NAME_SIZE-1);

	len = lif_read(DIR->filename, buffer, 0, LIF_SECTOR_SIZE);
	if( len < LIF_SECTOR_SIZE)
        return(NULL);

	// Get Volume header and Directory start sector
	lif_UnPackVolume(buffer, (lifvol_t *)&DIR->V);

	// stat of file area
	DIR->current = DIR->V.DirStartSector+DIR->V.DirSectors;
	DIR->next = DIR->current;

	return(DIR);
}

/// @brief Open LIF directory for reading
/// Modeled after Linux readdir()
/// @param[in] *DIR: to LIF Volume/Diractoy structure 
/// @return lifdirent_t filled with directory structure, or NULL on error of end of Directory
lifdirent_t *lif_readdir(lifdir_t *DIR)
{
	int len;
	long offset,end;
	uint8_t dirent[LIF_DIR_SIZE];

	offset = (DIR->index * LIF_DIR_SIZE) + (DIR->V.DirStartSector * LIF_SECTOR_SIZE);
	end = (DIR->V.DirStartSector +  DIR->V.DirSectors) * LIF_SECTOR_SIZE;

	if(offset >= end)
		return(NULL);

	len = lif_read(DIR->filename, dirent, offset, sizeof(dirent));
	if( len < (long)sizeof(dirent))
        return(NULL);

	// extract DIR->DE settings from dirent
	lif_UnPackDir(dirent, DIR);

	if(DIR->DE.VolNumber != 0x8001)
		return(NULL);

	if(DIR->DE.FileType == 0xffff)
		return(NULL);

	// Update used sectors
	DIR->used += DIR->DE.FileLengthSectors;
	// Update first free sector as we read
	DIR->current = DIR->DE.FileStartSector;
	DIR->next = DIR->DE.FileStartSector + DIR->DE.FileLengthSectors;	
	DIR->index++;
	return((lifdirent_t *) &DIR->DE);
}

/// @brief Write a director entry
/// Modeled after Linux readdir()
/// @param[in] *DIR: to LIF Volume/Diractoy structure 
/// @return number of bytes written
long lif_writedir(lifdir_t *DIR)
{
	int len;
	long offset,end;
	uint8_t dirent[LIF_DIR_SIZE];

	offset = (DIR->index * LIF_DIR_SIZE) + (DIR->V.DirStartSector * LIF_SECTOR_SIZE);
	end = (DIR->V.DirStartSector +  DIR->V.DirSectors) * LIF_SECTOR_SIZE;

	if(offset >= end)
		return(-1);

	// store DIR->DE settings into dirent
	lif_PackDir(dirent, DIR);

	len = lif_write(DIR->filename, dirent, offset, sizeof(dirent));
	if( len < (long)sizeof(dirent))
        return(-1);

	// Update used sectors
	DIR->used += DIR->DE.FileLengthSectors;
	// Update first free sector as we read
	DIR->current = DIR->DE.FileStartSector;
	DIR->next = DIR->DE.FileStartSector + DIR->DE.FileLengthSectors;	
	DIR->index++;
	return(len);
}


/// @brief get LIF file size from directory entry
/// @param[in] name: LIF disk image name
/// @param[in] DE: LIF directory entry
/// @retrun void
long lif_filelength(lifdirent_t *DE)
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
	lifdirent_t *DE;
	lifdir_t *DIR = lif_opendir(name);

	if(DIR == NULL)
		return(NULL);

	while(1)
	{
		DE = lif_readdir(DIR);
		if(DE == NULL)
			break;
		bytes = lif_filelength(DE);
		// We can reuse this purged sector
		if(DE->FileType == 0 && bytes > size)
			break;
	}
	lif_closedir(DIR);

 	if((DIR->next*LIF_SECTOR_SIZE) > DIR->imagesize)
		return(NULL);
	// DIR->next will point to free space
	// DIR->current points to current file
	return(DIR);		// Last valid directory entry
}

	
/// @brief Display a LIF image file directory
/// @param[in] lifimagename: LIF disk image name
/// @return -1 on error or number of files found
int lif_dir(char *lifimagename)
{
	lifdirent_t *DE;
	long bytes;
	int files = 0;
	int purged = 0;
	lifdir_t *DIR;

	if(!*lifimagename)
	{
		if(debuglevel & 1)
			printf("lif_dir: lifimagename is empty\n");
		return(-1);
	}

	DIR = lif_opendir(lifimagename);

	if(DIR == NULL)
		return(-1);
	
	printf("NAME        TYPE    START SECTOR       SIZE RECSIZE\n");
	while(1)
	{
		DE = lif_readdir(DIR);
		if(DE == NULL)
			break;

		bytes = lif_filelength(DE);

		// name type start size
		printf("%-10s  %04x       %08lxH  %9ld    %4d\n", 
			DE->filename, 
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
	printf("Used Sectors: %4lxH\n", (long)DIR->used);
	printf("First Free:   %4lxH\n", (long)DIR->next);

	lif_closedir(DIR);

	return(files);
}



///@brief Pad the current sector to the end
/// When DIR is null we do not write, just compute the write size
/// @param[in] DIR: LIF image file sructure
/// @param[in] offset: offset to write to
/// @return pad size in bytes ,  -1 on error
int lif_write_pad(lifdir_t *DIR, long offset)
{
	int i, len, pad;
	uint8_t buf[LIF_SECTOR_SIZE+1];

	pad = LIF_SECTOR_SIZE - (offset % LIF_SECTOR_SIZE);

	if(DIR && pad < LIF_SECTOR_SIZE)
	{
		buf[0]  = 0xef;
		for(i=1;i<pad;++i)
			buf[i]  = 0xff;

		len = lif_write(DIR->filename, buf, offset, pad);
		if(len < pad)
			return(-1);

		if(debuglevel & 0x400)
			printf("Write Offset:   %4lxH\n", (long)offset/LIF_SECTOR_SIZE);
	}

	return( pad );
}

/** @brief  HP85 ASCII LIF records have a 3 byte header 
	ef [ff]* = end of data in this sector (no size) , pad with ff's optionally if there is room
	df size = string
	cf size = split accross sector end "6f size" at start of next sector 
		   but the 6f size bytes are not included in cf size! (yuck!)
	6f size = split continue (always starts at sector boundry)
	df 00 00 ef [ff]* = EOF (df size = 0) ef send of sector and optional padding
	size = 16 bits LSB MSB

	Example:
	000080e0 : 4b 7c 22 0d cf 29 00 31 34 20 44 49 53 50 20 22  : K|"..).14 DISP "
	000080f0 : 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 5f  :                _
	00008100 : 6f 10 00 5f 5f 5f 5f 5f 5f 5f 5f 5f 5f 5f 5f 5f  : o.._____________
	00008110 : 5f 22 0d df 2b 00 31 35 20 44 49 53 50 20 22 20  : _"..+.15 DISP "

	cf 29 00 (19 is to sector end) (new sector start with 6F 10 00 (10 is remainder)
	So 29 = 19 and 10 (yuck!)
*/


///@brief Write a string with header to a lif file
/// When DIR is null we do not write, just compute the write size
/// @param[in] DIR: LIF image file sructure
/// @param[in] offset: offset to write to
/// @param[in] str: string to write
/// @return wtite size in bytes,  -1 on error
int lif_write_string(lifdir_t *DIR, long offset, void *str)
{
	int size;
	int bytes;
	int len;
	int pad;

	uint8_t buf[LIF_SECTOR_SIZE+1];

	bytes = 0;

	// String size
	len = strlen(str);
	size = len + 3;

	// Compute the current offset in this sector
	pad = (offset % LIF_SECTOR_SIZE);

	// Would writting the string and its header overflow  this sector ?
	// If so, then pad this sector and write, then write string in next sector
	if((pad + size) > LIF_SECTOR_SIZE)
	{
		pad = lif_write_pad(DIR, offset);
		if(pad < 0)
			return(-1);
		bytes += pad;
		offset += pad;
	}

	// Now Write string
	if(DIR)
	{
		strncpy((char *)buf+3,(char *)str, size);
		// Insert LIF header into buffer ( distance to next record )
		buf[0] = 0xdf;
		buf[1] = len & 0xff;
		buf[2] = (len >> 8) & 0xff;

		len = lif_write(DIR->filename, buf, offset, size);
		if(len < size)
			return(-1);

		if(debuglevel & 0x400)
			printf("Write Offset:   %4lxH\n", (long)offset/LIF_SECTOR_SIZE);
	}

	bytes += size;
	return( bytes );
}
	

/// @brief Convert a user ASCII file into HP85 0xE010 LIF format 
/// We must know the convered file size BEFORE writting
/// So we must call this function TWICE
///   1) find out how big the converted file is (DIR is NULL)
///   2) Specify where to put it (DIR is set)
///
/// @param[in] userfile: User ASCII file source
/// @param[in] *DIR: Where to write file if set (not NULL)
/// @return size of LIF image in bytes, or -1 on error
long lif_ascii2lif(char *name, lifdir_t *DIR)
{
	long offset;
	long bytes;
	int count;
	int ind;
	FILE *fi;

	uint8_t str[LIF_SECTOR_SIZE+1];

	fi = lif_open(name, "r");
	if(fi == NULL)
		return(-1);

	offset = 0;
	if(DIR)
	{
		// DIR->next should be pointing at first free sector
		offset  = DIR->next * LIF_SECTOR_SIZE;
	}

	bytes = 0;

	count = 0;
	// Read user file and write LIF records
	// reserve 3 LIF header bytes + 1
	while( fgets((char *)str,(int)sizeof(str)-4, fi) != NULL )
	{
		trim_tail((char *)str);

		strcat((char *)str,"\r"); // HP85 lines end with "\r"

		// Write string
		ind = lif_write_string(DIR, offset, str);
		if(ind < 0)
		{
			fclose(fi);
			return(-1);
		}

		offset += ind;
		bytes += ind;
		count += ind;

		if(DIR)
		{
			if(count > 256)
			{		
				count = 0;
				printf("Wrote: %8ld\r",(long)bytes);
			}
		}
	}

	fclose(fi);

	str[0] = 0;
	// Write EOF string
	ind = lif_write_string(DIR, offset, str);
	if(ind < 0)
		return(-1);

	offset += ind;
	bytes += ind;

	// PAD the end of this last sector IF any bytes have been written to it
	// Note: we do not add the pad to the file size!
	ind = lif_write_pad(DIR, offset);
	if(ind < 0)
		return(-1);

	// we do not add pdd to offsets of size at the file end

	if(DIR)
		printf("Wrote: %8ld\r",(long)bytes);

	return(bytes);
}



/// @brief Add a user file to the LIF image
/// The basename of the lifname, without extensions, is used as the LIF file name
/// @param[in] lifimagename: LIF image name
/// @param[in] lifname: LIF file name
/// @param[in] userfile: userfile name
/// @return size of data written into to LIF image, or -1 on error
long lif_add_file(char *lifimagename, char *lifname, char *userfile)
{
	long bytes;
	long sectors;
	long len;
	lifdir_t *DIR;

	if(!*lifimagename)
	{
		if(debuglevel & 1)
			printf("lif_add_file: lifimagename is empty\n");
		return(-1);
	}
	if(!*lifname)
	{
			if(debuglevel & 1)
				printf("lif_add_file: lifname is empty\n");
		return(-1);
	}
	if(!*userfile)
	{
		if(debuglevel & 1)
			printf("lif_add_file: userfile is empty\n");
		return(-1);
	}

	if(debuglevel & 0x400)
		printf("LIF image:[%s], LIF name:[%s], user file:[%s]\n", 
			lifimagename, lifname, userfile);

	// Find out how big converted file will be
	bytes = lif_ascii2lif(userfile, NULL);
	if(bytes < 0)
		return(-1);

	// Now find free entry
	DIR = lif_find_free(lifimagename, bytes);
	if(DIR == NULL)
		return(-1);	

	// Write converted file into free space
	bytes = lif_ascii2lif(userfile, DIR);

	// Convert byte to sectors 
	sectors = ((bytes | (LIF_SECTOR_SIZE-1))+1)/LIF_SECTOR_SIZE;

	// Setup directory entry
	lif_fixname(DIR->DE.filename, lifname,10);
	DIR->DE.FileType = 0xe010;  			// 10
	DIR->DE.FileStartSector = DIR->next;	// 12
	DIR->DE.FileLengthSectors = sectors;    // 16
	memset(DIR->DE.date,0,6);				// 20
	DIR->DE.VolNumber = 0x8001;				// 26
	DIR->DE.FileBytes = 0;					// 28
	DIR->DE.SectorSize  = 0x100;			// 30

	if(debuglevel & 0x400)
	{
		printf("New Directory Information BEFORE write\n");
		printf("Name:              %s\n", DIR->DE.filename);
		printf("Index:            %4d\n", (int)DIR->index);
		printf("First Sector:    %4lxH\n", DIR->DE.FileStartSector);
		printf("Length Sectors:  %4lxH\n", DIR->DE.FileLengthSectors);
		printf("Used Sectors:    %4lxH\n", (long)DIR->used);
	}

	// Write directory entry
	len = lif_writedir(DIR);
	if(len < 0)
		return(-1);
	if(debuglevel & 0x400)
	{
		printf("New Directory Information AFTER write\n");
		printf("Index:            %4d\n", (int)DIR->index);
		printf("First Sector:    %4lxH\n", DIR->DE.FileStartSector);
		printf("Length Sectors:  %4lxH\n", DIR->DE.FileLengthSectors);
		printf("Used Sectors:    %4lxH\n", (long)DIR->used);
	}
	printf("Wrote: %8ld\n", bytes);

	// Return file size
	return(bytes);
}


/// @brief Create/Format a LIF disk image
/// This can take a while to run, about 1 min for 10,000,000 bytes
/// @param[in] lifimagename: LIF disk image name
/// @param[in] liflabel: LIF Volume Label name
/// @param[in] dirsecs: Number of LIF directory sectors
/// @param[in] sectors: total disk image size in sectors
///@return bytes writting to disk image
long lif_create_image(char *lifimagename, char *liflabel, long dirsecs, long sectors)
{
    FILE *fp;
	long remainder, sector;
	long li;
	int len;
	int i;
	uint8_t buffer[LIF_SECTOR_SIZE];
	
	static lifdir_t _DIR;
	lifdir_t *DIR = (lifdir_t *) &_DIR;

	if(!*lifimagename)
	{
		if(debuglevel & 1)
			printf("lif_create_image: lifimagename is empty\n");
		return(-1);
	}
	if(!*liflabel)
	{
		if(debuglevel & 1)
			printf("lif_create_image: liflabel is empty\n");
		return(-1);
	}
	if(!dirsecs)
	{
		if(debuglevel & 1)
			printf("lif_create_image: dirsecs is 0\n");
		return(-1);
	}
	if(!sectors)
	{
		if(debuglevel & 1)
			printf("lif_create_image: sectors is 0\n");
		return(-1);
	}


	lif_vol_clear(DIR);

	// Initialize volume header
	DIR->V.LIFid = 0x8000;
	lif_fixname(DIR->V.Label, liflabel, 6);
	DIR->V.DirStartSector = 2;
	DIR->V.DirSectors = dirsecs;
	DIR->V.System3000LIFid = 0x1000;
	DIR->V.tracks_per_side = 0;
	DIR->V.sides = 0;
	DIR->V.sectors_per_track = 0;
	///@brief Current Date
	lif_time2lif(DIR->V.date, time(NULL));

	printf("Formating LIF image:[%s], Label:[%s], Dir Sectors:[%ld], sectors:[%ld]\n", 
		lifimagename, DIR->V.Label, (long)DIR->V.DirSectors, (long)sectors);

	// Size of of disk after volume start and directory sectors hae been written
	remainder = sectors - (DIR->V.DirSectors + DIR->V.DirStartSector);
	if(remainder < 0)
	{
		if(debuglevel & 1)
			printf("lif_create_image: Too few sectors specified in image fil\n");
		return(-1);
	}

	///@brief Open LIF disk image for writting
    fp = fopen(lifimagename, "w");
    if( fp == NULL)
    {
		if(debuglevel & 1)
			printf("lif_create_image: Can't open:%s\n", lifimagename);
        return( -1 );
    }

	///@brief Initial file position
	sector = 0;

	lif_PackVolume(buffer, (lifvol_t *) &DIR->V);

	// Write Volume Header
	len = fwrite(buffer, 1, LIF_SECTOR_SIZE, fp);
	if( len < LIF_SECTOR_SIZE)
	{
		if(debuglevel & 1)
			printf("lif_create_image: Write error %s @ %ld\n", lifimagename, sector);
		fclose(fp);
        return( -1 );
	}
	++sector;

	// Write Empty Sector
	memset(buffer,0,LIF_SECTOR_SIZE);
	for(li=1; (uint32_t) li < DIR->V.DirStartSector; ++li)
	{
		len = fwrite(buffer, 1, LIF_SECTOR_SIZE, fp);
		if( len < LIF_SECTOR_SIZE)
		{
			if(debuglevel & 1)
				printf("lif_create_image: Write error %s @ %ld\n", lifimagename, sector);
			fclose(fp);
			return( -1 );
		}
		++sector;
	}

	lif_dirent_clear(DIR);
	// Fill in Directory Entry
	///@brief File type of 0xffff is last directory entry
	DIR->DE.FileType = 0xffff;
	///FIXME this is the default that the HPdrive project uses, not sure of this
	DIR->DE.FileLengthSectors = 0x7fffUL;

	// Fill sector with Directory entries
	for(i=0; i<LIF_SECTOR_SIZE; i+= LIF_DIR_SIZE)
	{
		// store settings into buffer
		lif_PackDir(buffer+i, DIR);
	}

	// Write Directory sectors
	for(li=0;(uint32_t) li< DIR->V.DirSectors;++li)
	{
		len = fwrite(buffer, 1, LIF_SECTOR_SIZE, fp);
		if( len < LIF_SECTOR_SIZE)
		{
			if(debuglevel & 1)
				printf("lif_create_image: Write error %s @ %ld\n", lifimagename, sector);
			fclose(fp);
			return( -1 );
		}
		++sector;
		printf("sector: %ld\r", sector);
	}

	///@brief Zero out remining disk image
	memset(buffer,0,LIF_SECTOR_SIZE);

	// Write remaining in large chunks for speed
	for(li=0;li<remainder;++li)
	{
		len = fwrite(buffer, 1, LIF_SECTOR_SIZE, fp);
		if( len < LIF_SECTOR_SIZE)
		{
			if(debuglevel & 1)
				printf("lif_create_image: Write error %s @ %ld\n", lifimagename, sector);
			fclose(fp);
			return( -1 );
		}
		++sector;
		printf("sector: %ld\r", sector);
	}

	fclose(fp);

	printf("Formating: wrote:[%ld] sectors\n", sectors);
    return(sector);
}
