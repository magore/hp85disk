/**
 @file lifutils.c

 @brief LIF file utilities

 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

* LIF command help
* lif add lifimage lifname from_ascii_file
* Add ASCII file converted to E010 format to existing LIF image on SD card
* lif add /amigo1.lif TEST1 /test.bas
* lif add /amigo1.lif TREK85 /TREK85/TREK85.BAS
* Notes:
* Strings must be no longer then sector size - 3
* Any trailing "\n" and/or "\r" are coverted to "\n" when stored in LIF file

 * lif addbin lifimage lifname from_lif_image
 * Adding a binary LIF image files to another LIF image
 * Examples
 * lif addbin /amigo1.lif TEST /test.lif
 * lif addbin /amigo1.lif TREK85 /TREK85/trek.lif
 * Notes about TREK85 in the examples
 * Author: TREK85 port was done by Martin Hepperle
 * https://groups.io/g/hpseries80/topic/star_trek_game_for_hp_85/4845241

 * lif create lifimage liflabel directory_sectors total_sectors
 * Create a new LIF image
 * Example
 * lif create /amigo3.lif AMIGO3 15 1120
 * This formats an LIF image file with 15 directory sectors and a length of 1120 (16 * 2 * 35) sectors

 * lif del lifimage lifname
 * Delete a file from LIF image on SD card
 * Example
 *lif del /amigo1.lif TREK85

 * lif dir lifimage
 * Directory listing of LIF image
 * Example:
<pre>
lif dir amigo1.lif
Volume:[AMIGO2] Date:[<EMPTY>]
NAME         TYPE   START SECTOR        SIZE    RECSIZE   DATE
HELLO       e020h            10h         512        256   <EMPTY>
CIRCLE      e020h            12h         256        256   <EMPTY>
GPIB-S      e020h            13h         512        256   <EMPTY>
GPIB-T      e020h            15h        1536        256   <EMPTY>
GPIB7       e020h            1bh         256        256   <EMPTY>
AMIGO2      e020h            1ch         256        256   <EMPTY>
TREK85      e010h            1dh       27615        256   Tue Jun 27 16:28:28 2017
HELLO3      e010h            8bh         344        256   Sun Jun 18 18:13:24 2017
TREK85B     e010h            8dh       27615        256   Tue Jun 27 16:28:28 2017

9 Files
0 Purged
231 Used sectors
873 Free sectors
</pre>

* lif extract lifimage lifname to_ascii_file
* Extracts E010 file from LIF image converting to ASCII file on SD card
* Example
lif extract /amigo1.lif HELLO3 /HELLO3.BAS

* lif extractbin lifimage lifname to_lif_image
* Extracts LIF file from a LIF image and saves it as new LIF image
* Example
* lif extractbin /amigo1.lif HELLO3 /hello3.lif

* lif rename lifimage oldlifname newlifname
* Renames file in LIF image
* Example
* lif rename /amigo1.lif HELLO3 HELLO4
 * OPTIONAL - compile time add on
 * lif td02lif image.td0 image.lif
 * Convert TeleDIsk LIF encoded disk into pure LIF file
 * Uses code from external HxCFloppyEmulator library to decode TELEDISK format
 * The HxCFloppyEmulator library is Copyright (C) 2006-2014 Jean-Franâois DEL NERO
 * See: https://github.com/xXorAa/hxc-software/tree/master/HxCFloppyEmulator/libhxcfe/trunk/sources/loaders/teledisk_loader

 */

#ifdef LIF_STAND_ALONE
#include <unistd.h>
#include "user_config.h"
#include "lifsup.h"
#include "lifutils.h"
#include "td02lif.h"

#else
#include "hardware/user_config.h"
#include "vector.h"
#include "drives_sup.h"
#include "lifsup.h"
#include "lifutils.h"
#endif

extern int debuglevel;
extern hpdir_t hpdir;

/// @brief
///  Help Menu for User invoked GPIB functions and tasks
///  See: int gpib_tests(char *str)
/// @return  void
MEMSPACE
void lif_help(int full)
{
    printf( "lif       help\n" );

    if(full)
    {
        printf(
            "lif add lifimage lifname from_ascii_file\n"
            "lif addbin lifimage lifname from_lif_file\n"
            "lif create lifimage label directory_sectors sectors\n"
            "lif createdisk lifimage label model\n"
            "lif del lifimage name\n"
            "lif dir lifimage\n"
            "lif extract lifimage lifname to_ascii_file\n"
            "lif extractbin lifimage lifname to_lif_file\n"
            "    extracts a file into a sigle file LIF image\n"
            "lif rename lifimage oldlifname newlifname\n"

            "Use -d  after 'lif' keyword to enable LIF filesystem debugging\n"
            "\n"
            );
    }
}


/// @brief LIF user tests
/// @return  1 matched token, 0 if not
MEMSPACE
int lif_tests(int argc, char *argv[])
{

    int ind=0;
    char *ptr;

// display arguments for debugging
#ifdef TELEDISK
    int i;
#endif
#if 0
    for(i=0;i<argc;++i)
        printf("%d:%s\n", i, argv[i]);
    printf("\n");
#endif

// Nothing to do ?
    if(argc < 2)
        return (0);

// Argument 1
    ind = 1;
    ptr = argv[ind++];

// Argument 1 missing ?
//        Nothing to do
// argc should really get this
    if(!ptr || !*ptr)
        return(1);


	if(MATCHI_LEN(argv[0],"lif"))
	{
		if(MATCHI(ptr,"help") || MATCHI(ptr,"-help") || MATCHI(ptr,"-?") )
		{
			lif_help(1);
			return(1);
		}
	}

// Turn one debugging
// in the future we can add tests for specific messages
	debuglevel &= ~0x400;
    if (MATCHARGS(ptr,"-d", (ind + 0) ,argc))
    {
        debuglevel |= 0x400;
        ptr = argv[ind++];
    }

    if (MATCHARGS(ptr,"addbin", (ind + 3) ,argc))
    {
        lif_add_lif_file(argv[ind],argv[ind+1],argv[ind+2]);

        return(1);
    }

    if (MATCHARGS(ptr,"add", (ind + 3) ,argc))
    {
        lif_add_ascii_file_as_e010(argv[ind],argv[ind+1],argv[ind+2]);
        return(1);
    }
    if (MATCHARGS(ptr,"createdisk", (ind + 3) ,argc))
    {
///@brief format LIF image
        long dir,sectors;
        char *name = argv[ind];
        char *label = argv[ind+1];
        char *model = argv[ind+2];
        if( MATCHI_LEN(model,"hp"))
            model +=2;
        if(hpdir_find_drive(model,0, 0))
        {
            dir = lif_dir_count(hpdir.BLOCKS);
            sectors = hpdir.BLOCKS;
			// NOTE: we could grab the directory size for non 0 entries in the hpdir.ini file - I use a computed value which is also fine
            lif_create_image(name, label, dir, sectors);
            return(1);
        }
        printf("Disk: %s not found in hpdir.ini\n", model);
        return(1);
    }
    if (MATCHARGS(ptr,"create", (ind + 4) ,argc))
    {
///@brief format LIF image
        lif_create_image(argv[ind],argv[ind+1], atol(argv[ind+2]), atol(argv[ind+3]) );
        return(1);
    }
    if (MATCHARGS(ptr,"del", (ind + 2) ,argc))
    {
        lif_del_file(argv[ind],argv[ind+1]);
        return(1);
    }
    if (MATCHARGS(ptr,"dir", (ind + 1) ,argc))
    {
        lif_dir(argv[ind]);
        return(1);
    }
    if (MATCHARGS(ptr,"extractbin", (ind + 3) ,argc))
    {

        lif_extract_lif_as_lif(argv[ind],argv[ind+1],argv[ind+2]);
        return(1);
    }
    if (MATCHARGS(ptr,"extract", (ind + 3) ,argc))
    {

        lif_extract_e010_as_ascii(argv[ind],argv[ind+1],argv[ind+2]);
        return(1);
    }
    if (MATCHARGS(ptr,"rename", (ind + 3) ,argc))
    {
        lif_rename_file(argv[ind],argv[ind+1],argv[ind+2]);
        return(1);
    }

	if(MATCHI_LEN(argv[0],"td02lif"))
	{
		if(MATCHI(ptr,"help") || MATCHI(ptr,"-help") || MATCHI(ptr,"-?") )
		{
#ifdef TELEDISK
			td0_help(1);
			return(1);
#else
		    printf("td02lif support not enabled\n");
			return(1);
#endif
		}
#ifdef TELEDISK
// shift the arguments down by 1
        for(i=1;i<argc;++i)
        {
            argv[i-1] = argv[i];
        }
        argv[argc--] = NULL;

        td02lif(argc,argv);
        return(1);
#endif
    }
    return(0);
}


/// @brief Allocate and clear memory
/// Displays message on errors
/// @param[in] size: size of memory to allocate
/// @return pointer to allocated memory
MEMSPACE
void *lif_calloc(long size)
{
    uint8_t *p = safecalloc(size,1);
    if(!p)
        printf("lif_calloc:[%ld] not enough free memory\n", size);

    return(p);
}


/// @brief Free allocated memory
/// Displays message on errors
/// @param[in] p: pointer to memory to free
/// @return pointer to allocated memory
MEMSPACE
void lif_free(void *p)
{
    if(!p)
        printf("lif_free: NULL pointer\n");
    else
        safefree(p);
}


/// @brief Allocate and copy a string
/// Displays message on errors
/// @param[in] str: String to allocate and copy
/// @return pointer to allocated memory with string copy
MEMSPACE
char *lif_stralloc(char *str)
{
    int len = strlen(str);
    char *p = (char *)lif_calloc(len+4);
    if(!p)
        return(NULL);
    strcpy(p,str);
    return(p);
}


/// @brief Open a file that must exist
/// Displays message on errors
/// @param[in] *name: file name of LIF image
/// @param[in] *mode: open mode - see fopen
/// @return FILE * pointer
MEMSPACE
FILE *lif_open(char *name, char *mode)
{
    FILE *fp = fopen(name, mode);
    if( fp == NULL)
    {
        printf("lif_open: Can't open:[%s] mode:[%s]\n", name, mode);
        return(NULL);
    }
    return(fp);
}


/// @brief Stat a file
/// Displays message on errors
/// @param[in] *name: file name of LIF image
/// @param[in] *p: start_t structure pointer for result
/// @return NULL on error or copy of p
MEMSPACE
stat_t *lif_stat(char *name, stat_t *p)
{
    if(stat(name, p) < 0)
    {
        printf("lif_stat: Can't stat:%s\n", name);
        return(NULL);
    }
    return(p);
}


/// Displays message on errors
/// @param[in] *fp: FILE pointer
/// @param[in] offset: file offset
/// @param[in] mesg: user string as part of error message
/// @return 1 on success, 0 on error
MEMSPACE
int lif_seek_msg(FILE *fp, long offset, char *msg)
{
    if(ftell(fp) != offset)
    {
        if(fseek(fp, offset, SEEK_SET) < 0)
        {
            printf("lif_read_msg: %s Seek error %ld\n", msg, offset);
            return(0);
        }
    }
    return(1);
}


/// @brief Read data from a LIF image
/// Displays message on errors
/// @param[in] *LIF: lif_t structure with file pointers
/// @param[out] *buf: read buffer
/// @param[in] offset: read offset
/// @param[in] bytes: number of bytes to read
/// @return bytes read or 0 on error
MEMSPACE
long lif_read(lif_t *LIF, void *buf, long offset, int bytes)
{
    long len;

    if(!lif_seek_msg(LIF->fp,offset,LIF->name))
        return(0);

///@brief Initial file position
    len = fread(buf, 1, bytes, LIF->fp);
    if( len != bytes)
    {

        if(debuglevel & LIF_DEBUG)
            printf("lif_read: read:[%s] offset:[%ld] write:[%ld] expected:[%d]\n",
                LIF->name, (long)offset, (long)len, (int)bytes);
    }
    return(len);
}


/// @brief Write data to an LIF image
/// Displays message on errors
/// @param[in] *LIF: lif_t structure with file pointers
/// @param[in] *buf: write buffer
/// @param[in] offset: write offset
/// @param[in] bytes: number of bytes to write
/// @return bytes read or 0 on error
MEMSPACE
int lif_write(lif_t *LIF, void *buf, long offset, int bytes)
{
    int len;

// Seek to write position
    if(!lif_seek_msg(LIF->fp, offset,LIF->name))
        return(0);

///@brief Initial file position
    len = fwrite(buf, 1, bytes, LIF->fp);
    if( len != bytes)
    {
        if(debuglevel & LIF_DEBUG)
            printf("lif_write: Write:[%s] offset:[%ld] write:[%d] expected:[%d]\n",
                LIF->name, offset, len, bytes);
    }
    return(len);
}


/// @brief Check if characters in a LIF volume or LIF file name are valid
/// @param[in] c: character to test
/// @param[in] index: index of character in volume or file name
/// @retrun c (optionally upper cased) or 0 if no match
MEMSPACE
int lif_chars(int c, int index  __attribute__((unused)))
{
// Series 80 names allow most characters even though the LIF spec does not
	if(c == '.' || c == ':' || c == '"' || c == '\'' || c < ' ' || c > 128)
		return(0);
	return(c);
#if 0
    if(c == ' ')
        return(c);
    if(c >= 'a' && c <= 'z')
        return(c-0x20);
    if(c >= 'A' && c <= 'Z')
        return(c);
    if(index > 0)
	{
		if(c >= '0' && c <= '9') 
			return(c);
		if(c == '$' || c == '_' || c == '-')
			return(c);
	}
    return(0);
#endif
}


/// @brief Convert LIF space padded string name into normal string
/// @param[in] *B: LIF name space padded
/// @param[out] *name: string result with traling spaces removed
/// @param[in] size: max size of name
/// @retrun 1 if string i ok or 0 if bad characters were found
MEMSPACE
int lif_B2S(uint8_t *B, uint8_t *name, int size)
{
    int i;
    int status = 1;
    for(i=0;i<size;++i)
    {
        if( !lif_chars(B[i],i))
            status = 0;
    }
    for(i=0;i<size;++i)
        name[i] = B[i];
    name[i] = 0;
// remove space padding
    trim_tail((char *)name);
    return(status);
}


/// @brief Check volume LIF name or directory name is valid
/// @param[in] *name: name to test
/// @retrun 1 if the string is ok or 0 if invalid LIF name characters on input string
MEMSPACE
int lif_checkname(char *name)
{
    int i;
    int status = 1;
    for(i=0;name[i];++i)
    {
        if(!lif_chars(name[i],i))
            status = 0;
    }
    return(status);
}


/// @brief Convert string to LIF directory record
/// @param[out] *B: LIF result with added trailing spaces
/// @param[in] *name: string
/// @param[in] size: max size of B
/// @retrun 1 if the string is ok or 0 if invalid LIF name characters on input string
MEMSPACE
void lif_S2B(uint8_t *B, uint8_t *name, int size)
{
    int i;
    for(i=0;name[i] && i<size;++i)
    {
        B[i] = name[i];
    }
    for(;i<size;++i)
        B[i] = ' ';
}


///@brief Convert name into a valid LIF name
/// Only use the basename() part of the string and remove any file name extentions
/// LIF names may have only these characters: [A-Z][A-Z0-09_]+
/// LIF names are converted to upper case
/// LIF names are padded at the end with spaces
/// Any invalid input characters are converted into spaces
///@param[out] *B: output LIF string
///@param[in] *name: input string
///@param[in] size: maximum size of output string
///@return length of result
MEMSPACE
int lif_fixname(uint8_t *B, char *name, int size)
{
    uint8_t c,ret;
    int i,index;
    char *ptr;
    uint8_t *save = B;

    index = 0;
// remove any "/"
    ptr = basename(name);

    for(i=0; ptr[i] && index < size;++i)
    {
        c = ptr[i];
// trim off extensions
        if(c == '.')
            break;
        if( (ret = lif_chars(c,i)) )
            *B++ = ret;
        else
            *B++ = ' ';
    }
    while(i < size)
    {
        *B++ = ' ';
        ++i;
    };
    *B = 0;
    return(strlen((char *)save));
}


///@brief Convert LIF volume records into byte vector
///@param[in] *LIF: LIF image structure
///@param[out] B: byte vector to pack data into
///@return void
MEMSPACE
void lif_vol2str(lif_t *LIF, uint8_t *B)
{
    V2B_MSB(B,0,2,LIF->VOL.LIFid);
    lif_S2B(B+2,LIF->VOL.Label,6);
    V2B_MSB(B,8,4,LIF->VOL.DirStartSector);
    V2B_MSB(B,12,2,LIF->VOL.System3000LIFid);
    V2B_MSB(B,14,2,0);
    V2B_MSB(B,16,4,LIF->VOL.DirSectors);
    V2B_MSB(B,20,2,LIF->VOL.LIFVersion);
    V2B_MSB(B,22,2,0);
    V2B_MSB(B,24,4,LIF->VOL.tracks_per_side);
    V2B_MSB(B,28,4,LIF->VOL.sides);
    V2B_MSB(B,32,4,LIF->VOL.sectors_per_track);
    memcpy((void *) (B+36),LIF->VOL.date,6);
}


///@brief Convert byte vector into LIF volume records
///@param[in] B: byte vector
///@param[out] *LIF: LIF image structure
///@return void
MEMSPACE
void lif_str2vol(uint8_t *B, lif_t *LIF)
{

    LIF->VOL.LIFid = B2V_MSB(B,0,2);
    lif_B2S(B+2,LIF->VOL.Label,6);
    LIF->VOL.DirStartSector = B2V_MSB(B,8,4);
    LIF->VOL.System3000LIFid = B2V_MSB(B,12,2);
    LIF->VOL.zero1 = B2V_MSB(B,14,2);
    LIF->VOL.DirSectors = B2V_MSB(B,16,4);
    LIF->VOL.LIFVersion = B2V_MSB(B,20,2);
    LIF->VOL.zero2 = B2V_MSB(B,22,2);
    LIF->VOL.tracks_per_side = B2V_MSB(B,24,4);
    LIF->VOL.sides = B2V_MSB(B,28,4);
    LIF->VOL.sectors_per_track = B2V_MSB(B,32,4);
    memcpy((void *) LIF->VOL.date, (B+36),6);
}


///@brief Convert LIF directory records into byte vector
///@param[in] *LIF: LIF image pointer
///@param[out] B: byte vector to pack data into
///@return void
MEMSPACE
void lif_dir2str(lif_t *LIF, uint8_t *B)
{
    lif_S2B(B,LIF->DIR.filename,10);              // 0
    V2B_MSB(B,10,2,LIF->DIR.FileType);            // 10
    V2B_MSB(B,12,4,LIF->DIR.FileStartSector);     // 12
    V2B_MSB(B,16,4,LIF->DIR.FileSectors);         // 16
    memcpy(B+20,LIF->DIR.date,6);                 // 20
    V2B_MSB(B,26,2,LIF->DIR.VolNumber);           // 26
    V2B_LSB(B,28,2,LIF->DIR.FileBytes);           // 28
    V2B_LSB(B,30,2,LIF->DIR.SectorSize);          // 30
}


///@brief Convert byte vector into byte vector
///@param[in] B: byte vector to extract data from
///@param[out] LIF: lifdir_t structure pointer
///@return void
MEMSPACE
void lif_str2dir(uint8_t *B, lif_t *LIF)
{
    lif_B2S(B,LIF->DIR.filename,10);
    LIF->DIR.FileType = B2V_MSB(B, 10, 2);
    LIF->DIR.FileStartSector = B2V_MSB(B, 12, 4);
    LIF->DIR.FileSectors = B2V_MSB(B, 16, 4);
    memcpy(LIF->DIR.date,B+20,6);
    LIF->DIR.VolNumber = B2V_MSB(B, 26, 2);
    LIF->DIR.FileBytes = B2V_LSB(B, 28, 2);
    LIF->DIR.SectorSize= B2V_LSB(B, 30, 2);
}


/// @brief Convert number >= 0 and <= 99 to BCD.
/// @warning we assume the number is in range.
///  BCD format: each hex nibble has a digit 0 .. 9
/// @param[in] data: number to convert.
/// @return  BCD value
MEMSPACE
uint8_t lif_BIN2BCD(uint8_t data)
{
    return(  ( (data/10U) << 4 ) | (data%10U) );
}


/// @brief Convert BCD in the range 0 and <= 99 to BIN
///  BCD format: each hex nibble has a digit 0 .. 9
/// @param[in] data: number to convert to binary
MEMSPACE
int lif_BCD2BIN(uint8_t bin)
{
    return( ((bin>>4)*10U)+(bin & 0x0f) );
}


/// @brief UNIX time to LIF BCD time format
/// The BCD year is only the lower 2 digits of the year
///      So We assume that values <= 70 are years >= 2000
/// BCD Day and Month start at 1
/// Note: If t == 0 we set ALL BCD digits to 0
/// @param[in] t: UNIX time_t time value
/// @param[out] *bcd: pcked 6 byte BCD LIF time YY MM DD HH MM SS
/// @see time() in time.c
/// @return void
MEMSPACE
void lif_time2lifbcd(time_t t, uint8_t *bcd)
{
    tm_t tm;
    int i;

    if(t == 0)
    {
        for(i=0;i<6;++i)
            bcd[i] = 0;
        return;
    }

    gmtime_r((time_t *) &t, (tm_t *)&tm);
    bcd[0] = lif_BIN2BCD(tm.tm_year % 100);
    bcd[1] = lif_BIN2BCD(tm.tm_mon+1);
    bcd[2] = lif_BIN2BCD(tm.tm_mday);
    bcd[3] = lif_BIN2BCD(tm.tm_hour);
    bcd[4] = lif_BIN2BCD(tm.tm_min);
    bcd[5] = lif_BIN2BCD(tm.tm_sec);
}


/// @brief convert BCD date into time_t value
/// The BCD year is only the lower 2 digits of the year
///      So We assume that values <= 70 are years >= 2000
/// BCD Day and Month start at 1
/// If ALL BCD digits are 0 we return 0
/// @param[in] *bcd: packed 6 byte BCD LIF time YY MM DD HH MM SS
/// @return time_t result or 0 if all bcd digits were 0
MEMSPACE
time_t lif_lifbcd2time(uint8_t *bcd)
{
    int year,mon;
    tm_t tm;
    time_t t;
    int i;
    int zerof = 1;

    for(i=0;i<6;++i)
    {
        if(lif_BCD2BIN(bcd[i]))
            zerof = 0;
    }
    if(!zerof)
    {
        year = lif_BCD2BIN(bcd[0]);
        mon = lif_BCD2BIN(bcd[1])-1;
        if(year < 70)
            year += 100;
        tm.tm_year = year;
        tm.tm_mon = mon;
        tm.tm_mday = lif_BCD2BIN(bcd[2]);
        tm.tm_hour = lif_BCD2BIN(bcd[3]);
        tm.tm_min= lif_BCD2BIN(bcd[4]);
        tm.tm_sec= lif_BCD2BIN(bcd[5]);

        t = timegm( (tm_t *) &tm);
        return(t);
    }
    return(0);
}


/// @brief GMT version of POSIX ctime().
///
/// @param[in] tp: time_t * time input.
///
/// @return  buf[].
///  - Example: "Thu Dec  8 21:45:05 EST 2011".
/// @see ctime()
/// @warning result is overwritten on each call.
MEMSPACE
char *lif_ctime_gmt(time_t *tp)
{
    tm_t tm;
    static char _lif_ctime_buf[32];
    char *ptr;
    memset(_lif_ctime_buf,0,sizeof(_lif_ctime_buf));
    ptr = asctime_r( gmtime_r(tp,&tm), _lif_ctime_buf);
    trim_tail(ptr);
    return(ptr);
}


///@breif convert BCD time into a date string
/// The BCD year is only the lower 2 digits of the year
///      So We assume that values <= 70 are years >= 2000
/// BCD Day and Month start at 1
/// If BCD string is all zero we return <EMPTY>
///@param[in] *bcd: pcked 6 byte BCD LIF time YY MM DD HH MM SS
///@return string result
MEMSPACE
char *lif_lifbcd2timestr(uint8_t *bcd)
{
    static char _timestr[32];
    memset(_timestr,0,sizeof(_timestr));
    time_t t = lif_lifbcd2time(bcd);

    if(t)
        strcpy(_timestr,lif_ctime_gmt((time_t *)&t));
    else
        strcpy(_timestr,"<EMPTY>");
    return(_timestr);
}


/// @brief File seek with error message

/// @brief Clear LIF structure
/// @param[in] *LIF: pointer to LIF structure
/// @return void
MEMSPACE
void lif_image_clear(lif_t *LIF)
{
    memset((void *) LIF,0,sizeof(lif_t));
}


/// @brief Clear DIR part of LIF structure
/// @param[in] *LIF: pointer to LIF structure
/// @return void
MEMSPACE
void lif_dir_clear(lif_t *LIF)
{
    memset((void *) &LIF->DIR,0,sizeof(lifdir_t));
}


/// @brief Clear VOL part of LIF structure
/// @param[in] *LIF: pointer to LIF structure
/// @return void
MEMSPACE
void lif_vol_clear(lif_t *LIF)
{
    memset((void *) &LIF->VOL,0,sizeof(lifvol_t));
}


/// @brief Dump LIF struture data for debugging
/// @param[in] *LIF: pointer to LIF structure
/// @return void
MEMSPACE
void lif_dump_vol(lif_t *LIF,char *msg)
{
    printf("\n%s\n",msg);
    printf("LIF name:             %s\n", LIF->name);
    printf("LIF sectors:          %8lXh\n", (long)LIF->sectors);
    printf("LIF bytes:            %8lXh\n", (long)LIF->imagebytes);
    printf("LIF filestart:        %8lXh\n", (long)LIF->filestart);
    printf("LIF file sectors:     %8lXh\n", (long)LIF->filesectors);
    printf("LIF used:             %8lXh\n", (long)LIF->usedsectors);
    printf("LIF free:             %8lXh\n", (long)LIF->freesectors);
    printf("LIF files:            %8lXh\n",(long)LIF->files);
    printf("LIF purged:           %8lXh\n",(long)LIF->purged);

    printf("VOL Label:            %s\n", LIF->VOL.Label);
    printf("VOL LIFid:            %8Xh\n",(int)LIF->VOL.LIFid);
    printf("VOL Dir start:        %8lXh\n",(long)LIF->VOL.DirStartSector);
    printf("VOL Dir sectors:      %8lXh\n",(long)LIF->VOL.DirSectors);
    printf("VOL 3000LIFid:        %8Xh\n",(unsigned int)LIF->VOL.System3000LIFid);
    printf("VOL LIFVersion:       %8Xh\n",(unsigned int)LIF->VOL.LIFVersion);
    printf("VOL Date:             %s\n", lif_lifbcd2timestr(LIF->VOL.date));
    printf("DIR File Name:        %s\n", LIF->DIR.filename);
    printf("DIR File Type:        %8Xh\n", (int)LIF->DIR.FileType);
    printf("DIR File Volume#:     %8Xh\n", (int)LIF->DIR.VolNumber);
    printf("DIR File start:       %8lXh\n", (long)LIF->DIR.FileStartSector);
    printf("DIR File sectors:     %8lXh\n", (long)LIF->DIR.FileSectors);
    printf("DIR File bytes:       %8lXh\n", (long)LIF->DIR.FileBytes);
    printf("DIR File sector size: %8Xh\n", (int)LIF->DIR.SectorSize);
    printf("DIR File Date:        %s\n", lif_lifbcd2timestr(LIF->DIR.date));
    printf("\n");
}


///@brief Check Volume Table for values in range
///@param[in] *LIF: Image structure
///@return 1 of ok, 0 on eeror
MEMSPACE
int lif_check_volume(lif_t *LIF)
{
    int status = 1;
    uint32_t filestart;

    if( !lif_checkname((char *)LIF->VOL.Label) )
    {
        if(debuglevel & LIF_DEBUG)
            printf("LIF Volume invalid Volume Name");
        status = 0;
    }

    if(LIF->VOL.System3000LIFid != 0x1000)
    {
        if(debuglevel & LIF_DEBUG)
            printf("LIF Volume invalid System3000 ID (%04XH) expected 1000H\n", LIF->VOL.System3000LIFid);
        status = 0;
    }

    if(LIF->VOL.LIFVersion > 1)
    {
        if(debuglevel & LIF_DEBUG)
            printf("LIF Version: %04XH > 1\n", LIF->VOL.LIFVersion);
        status = 0;
    }

    if(LIF->VOL.zero1 != 0)
    {
        if(debuglevel & LIF_DEBUG)
            printf("LIF Volume invalid bytes at offset 14&15 should be zero\n");
        status = 0;
    }

    if(LIF->VOL.zero2 != 0)
    {
        if(debuglevel & LIF_DEBUG)
            printf("LIF Volume invalid bytes at offset 22&23 should be zero\n");
        status = 0;
    }

    if(LIF->VOL.DirStartSector < 1)
    {
        if(debuglevel & LIF_DEBUG)
            printf("LIF Volume invalid start sector:%ld\n", (long)LIF->VOL.DirStartSector);
        status = 0;
    }

    if(LIF->VOL.DirSectors < 1)
    {
        if(debuglevel & LIF_DEBUG)
            printf("LIF Volume invalid Directory Sector Count < 1\n");
        status = 0;
    }

// File area start and size
    filestart = LIF->VOL.DirStartSector + LIF->VOL.DirSectors;
    if(filestart > LIF->sectors)
    {
        if(debuglevel & LIF_DEBUG)
            printf("LIF Volume invalid file start > image size\n");
        status = 0;
    }

    return(status);
}


///@brief Validate Directory record values
/// We only do basic out of bounds tests for this record
/// Purged or EOF directory records are NOT checked and always return 1
///@param[in] *LIF: Image structure
///@param[in] debug: dispaly diagostice messages
///@return 1 of ok, 0 on eeror
MEMSPACE
int lif_check_dir(lif_t *LIF)
{
    int status = 1;

// We do not check purged or end of DIRECTORY ercordss
    if(LIF->DIR.FileType == 0)
    {
        return(1);
    }

    if(LIF->DIR.FileType == 0xffff)
    {
        return(1);
    }

    if( !lif_checkname((char *)LIF->DIR.filename) )
    {
        status = 0;
        if(debuglevel & LIF_DEBUG)
            printf("LIF Directory:[%s] invalid Name\n",LIF->DIR.filename);
    }

    if(LIF->filestart)
    {
        if(LIF->DIR.FileStartSector < LIF->filestart)
        {
            status = 0;
            if(debuglevel & LIF_DEBUG)
                printf("LIF Directory:[%s] invalid start sector:%lXh < fie area start:%lXh\n",
                    LIF->DIR.filename,
                    (long)LIF->DIR.FileStartSector,
                    (long)LIF->filestart);
        }
    }

    if(LIF->sectors)
    {
        if( (LIF->DIR.FileStartSector + LIF->DIR.FileSectors) > (LIF->sectors) )
        {
            status = 0;
            if(debuglevel & LIF_DEBUG)
            {
                printf("LIF Directory:[%s] invalid end sector:%lXh > total sectors:%lXh\n",
                    LIF->DIR.filename,
                    (long)LIF->DIR.FileStartSector + LIF->DIR.FileSectors,
                    (long)LIF->sectors);
            }
        }
    }

    if(LIF->DIR.VolNumber != 0x8001)
    {
        status = 0;
        if(debuglevel & LIF_DEBUG)
            printf("LIF Directory:[%s] invalid Volume Number:%Xh\n", LIF->DIR.filename, (int)LIF->DIR.VolNumber);
    }

// Only inforce file type checks for types we know
// 0xE010 .. 0xE013 are the file types that can use LIF->DIR.FileBytes
    if((LIF->DIR.FileType & 0xFFFC) == 0xE010)
    {
        if(LIF->DIR.FileBytes)
        {
// Error test
            if( lif_bytes2sectors(LIF->DIR.FileBytes) > LIF->DIR.FileSectors )
            {
                status = 0;
                if(debuglevel & LIF_DEBUG)
                    printf("LIF Directory:[%s] invalid FileBytes:%ld as sectors:%ld > FileSectors:%ld\n",
                        LIF->DIR.filename,
                        (long) LIF->DIR.FileBytes,
                        (long) lif_bytes2sectors(LIF->DIR.FileBytes),
                        (long) LIF->DIR.FileSectors);
            }

// Warning test only
// Does this LIF entry have more FileSectors then FileBytes when converted to sectors?
            if( lif_bytes2sectors(LIF->DIR.FileBytes) < LIF->DIR.FileSectors )
            {
                if(debuglevel & LIF_DEBUG)
                    printf("LIF Directory:[%s] warning FileBytes:%ld as sectors:%ld < FileSectors:%ld\n",
                        LIF->DIR.filename,
                        (long) LIF->DIR.FileBytes,
                        (long) lif_bytes2sectors(LIF->DIR.FileBytes),
                        (long) LIF->DIR.FileSectors);
            }
            if(debuglevel & LIF_DEBUG && LIF->DIR.FileBytes == 0)
            {
                status = 0;
                printf("LIF Directory:[%s] invalid FileBytes == 0\n",
                    LIF->DIR.filename);
            }
        }
    }

//FIXME check file types?!
    if(LIF->DIR.SectorSize != LIF_SECTOR_SIZE)
    {
		// MG do we fail this ?
        // status = 0;
        if(debuglevel & LIF_DEBUG)
            printf("LIF Directory:[%s] warning sector size :%ld != %d\n", LIF->name, 
				(long)LIF->DIR.SectorSize, (int) LIF_SECTOR_SIZE);
    }

    return(status);
}


/// @brief Create LIF image with Volume, Directory and optional empty filespace
/// @param[in] imagename:  Image name
/// @param[in] liflabel:   Volume Label
/// @param[in] dirstart:   Directory start sector
/// @param[in] dirsectors: Directory sectors
/// @return pointer to LIF structure
MEMSPACE
lif_t *lif_create_volume(char *imagename, char *liflabel, long dirstart, long dirsectors, long filesectors)
{
    long size;
    long i;
    long offset;
    long count;
    uint8_t buffer[LIF_SECTOR_SIZE];

    time_t t = time(NULL);

    lif_t *LIF = lif_calloc(sizeof(lif_t)+4);
    if(LIF == NULL)
        return(NULL);

    printf("Creating:%s, Label:[%s], Directory Start %ld, Directory Size: %ld, File Sectors:%ld\n",
        imagename, liflabel, dirstart, dirsectors, filesectors );

    if(debuglevel & LIF_DEBUG)
        lif_dump_vol(LIF,"lif_create_volume");

    lif_image_clear(LIF);

// Initialize volume header
    LIF->VOL.LIFid = 0x8000;
    lif_fixname(LIF->VOL.Label, liflabel, 6);
    LIF->VOL.DirStartSector = dirstart;
    LIF->VOL.DirSectors = dirsectors;
    LIF->VOL.System3000LIFid = 0x1000;
    LIF->VOL.tracks_per_side = 0;
    LIF->VOL.sides = 0;
    LIF->VOL.sectors_per_track = 0;
///@brief 0 causes time to return the current date

    lif_time2lifbcd(t, LIF->VOL.date);

// update LIF headers
    LIF->name = lif_stralloc(imagename);
    if(LIF->name == NULL)
    {
        lif_close_volume(LIF);
        return(NULL);
    }

// Initilize all LIF headers
    LIF->filesectors = filesectors;
    LIF->filestart = dirstart + dirsectors;
    LIF->sectors = (LIF->filestart+LIF->filesectors);
    LIF->imagebytes = LIF->sectors * (long)LIF_SECTOR_SIZE;
    LIF->freesectors = LIF->filesectors;
    LIF->usedsectors = 0;
    LIF->files = 0;
    LIF->purged = 0;
    LIF->dirindex = 0;
    LIF->EOFindex = 0;

    memset(buffer,0,LIF_SECTOR_SIZE);

    lif_vol2str(LIF,buffer);

// Write Volume header
    LIF->fp = lif_open(LIF->name,"wb+");
    if(LIF->fp == NULL)
    {
        lif_close_volume(LIF);
        return(NULL);
    }

    offset = 0;
    count = 0;

    size = lif_write(LIF, buffer, offset, LIF_SECTOR_SIZE);

    if(size < LIF_SECTOR_SIZE)
    {
        lif_close_volume(LIF);
        return(NULL);
    }
    offset += size;
    ++count;

    memset(buffer,0,LIF_SECTOR_SIZE);

// Space BETWEEN Volume header and Directory area
    for(i=1;i<dirstart;++i)
    {
        size = lif_write(LIF, buffer, offset, LIF_SECTOR_SIZE);
        if(size < LIF_SECTOR_SIZE)
        {
            lif_close_volume(LIF);
            return(NULL);
        }
        offset += size;
        printf("\tWrote: %ld\r", count);
        ++count;
    }

// Write Directory sectors
    lif_dir_clear(LIF);
    LIF->DIR.FileType = 0xffff;

    for(i=0;i<LIF_SECTOR_SIZE;i+=LIF_DIR_SIZE)
        lif_dir2str(LIF,buffer+i);

    for(i=0;i<dirsectors;++i)
    {
        size = lif_write(LIF, buffer, offset, LIF_SECTOR_SIZE);
        if(size < LIF_SECTOR_SIZE)
        {
            lif_close_volume(LIF);
            return(NULL);
        }
        offset += size;
        if((count % 100) == 0)
            printf("\tWrote: %ld\r", count);
        ++count;
    }

// File area sectors
    memset(buffer,0,LIF_SECTOR_SIZE);
    for(i=0;i<filesectors;++i)
    {
        size = lif_write(LIF, buffer, offset, LIF_SECTOR_SIZE);
        if(size < LIF_SECTOR_SIZE)
        {
            lif_close_volume(LIF);
            return(NULL);
        }
        offset += size;
        if((count % 100) == 0)
            printf("\tWrote: %ld\r", count);
        ++count;
    }
    printf("\tWrote: %ld\n", count);

    lif_rewinddir(LIF);

// As a sanity check verify basic values
    if( !lif_check_volume(LIF) )
    {
        lif_closedir(LIF);
        return(NULL);
    }

// Scan directory and verify values
    if( !lif_updatefree(LIF))
    {
        lif_closedir(LIF);
        return(NULL);
    }

    return(LIF);
}


/// @brief Free LIF structure and close any files
/// @param[in] *LIF: pointer to LIF Volume/Directoy structure
/// @return void
MEMSPACE
void lif_close_volume(lif_t *LIF)
{
    if(LIF)
    {
        if(LIF->fp)
        {
            fseek(LIF->fp, 0, SEEK_END);
            fclose(LIF->fp);
            LIF->fp = NULL;
            sync();
        }

        if(LIF->name)
            lif_free(LIF->name);

        lif_vol_clear(LIF);

        lif_free(LIF);
    }
}


/// @brief Convert bytes into used sectors
/// @param[in] bytes: size in bytes
/// @return sectors
/// FIXME assumes 256 byte sectors
MEMSPACE
uint32_t lif_bytes2sectors(uint32_t bytes)
{
    uint32_t sectors = (bytes/(long)LIF_SECTOR_SIZE);
    if(bytes % (long)LIF_SECTOR_SIZE)
        ++sectors;
    return(sectors);
}


/// @brief Rewind LIF directory
/// Note readdir pre-increments the directory pointer index so we start at -1
/// @param[in] *LIF: pointer to LIF Volume/Directoy structure
/// @return void
MEMSPACE
void lif_rewinddir(lif_t *LIF)
{
// Directory index
    LIF->dirindex = -1;
}


/// @brief Close LIF directory
/// clear and free lif_t structure
/// @param[in] *LIF: pointer to LIF Volume/Directoy structure
/// @return 0 on sucesss, -1 on error
MEMSPACE
void lif_closedir(lif_t *LIF)
{
    return( lif_close_volume(LIF) );
}


/// @brief Check directory index limits
/// @param[in] *LIF: LIF Volume/Diractoy structure
/// @param[in] index: directory index
/// @return 1 inside, 0 outside
MEMSPACE
int lif_checkdirindex(lif_t * LIF, int index)
{
    if(index < 0 || lif_bytes2sectors((long) index * LIF_DIR_SIZE) > LIF->VOL.DirSectors)
    {
        printf("lif_checkdirindex:[%s] direcory index:[%d] out of bounds\n",LIF->name, index);
        if(debuglevel & LIF_DEBUG)
            lif_dump_vol(LIF,"lif_chckdirindex");
        return(0);
    }
    return(1);
}


/// @brief Read LIF directory record number N
/// @param[in] *LIF: to LIF Volume/Diractoy structure
/// @param[in] index: director record number
/// @return 1 on success, 0 if error, bad directory record or outside of directory limits
MEMSPACE
int lif_readdirindex(lif_t *LIF, int index)
{
    uint32_t offset;
    uint32_t size;
    uint8_t dir[LIF_DIR_SIZE];

// Verify that the records is withing directory limits
    if( !lif_checkdirindex(LIF, index) )
    {
        return(0);
    }

// Compute offset
    offset = ((long)index * LIF_DIR_SIZE) + (LIF->VOL.DirStartSector * (long)LIF_SECTOR_SIZE);

// read raw data
    size = lif_read(LIF, dir, offset, sizeof(dir));
    if(size  < (long)sizeof(dir) )
    {
        return(0);
    }

// Convert into directory structure
    lif_str2dir(dir, LIF);

// Update EOF index
    if( LIF->DIR.FileType == 0xffffUL )
        LIF->EOFindex = index;

    if( !lif_check_dir(LIF))
    {
        printf("lif_check_dir: error, index:%d\n", (int)index);
        if(debuglevel & LIF_DEBUG)
        {
            lif_dump_vol(LIF,"lif_readdirindex");
        }
        return(0);
    }
    return(1);
}


/// @brief Write LIF drectory record number N
/// @param[in] *LIF: LIF Volume/Diractoy structure
/// @param[in] index: director record number
/// @return 1 on success, 0 if error, bad directory record or outside of directory limits
MEMSPACE
int lif_writedirindex(lif_t *LIF, int index)
{
    long offset;
    uint8_t dir[LIF_DIR_SIZE];

// Validate the record
    if(!lif_check_dir(LIF))
    {
        if(debuglevel & LIF_DEBUG)
            lif_dump_vol(LIF,"lif_writedirindex");
        return(0);
    }

// check for out of bounds
    if( !lif_checkdirindex(LIF, index))
        return(0);

// Update EOF index
    if( LIF->DIR.FileType == 0xffffUL )
        LIF->EOFindex = index;

    offset = ((long)index * LIF_DIR_SIZE) + (LIF->VOL.DirStartSector * (long)LIF_SECTOR_SIZE);

// store LIF->DIR settings into dir
    lif_dir2str(LIF, dir);

    if( lif_write(LIF, dir, offset, sizeof(dir)) < (int ) sizeof(dir) )
        return(0);

    return(1);
}


/// @brief Write LIF drectory EOF
/// @param[in] *LIF: LIF Volume/Diractoy structure
/// @param[in] index: director record number
/// @return 1 on success, 0 on error and outsize directory limits
MEMSPACE
int lif_writedirEOF(lif_t *LIF, int index)
{
// Create a director EOF
    lif_dir_clear(LIF);
    LIF->DIR.FileType = 0xffff;
    LIF->EOFindex = index;
    return( lif_writedirindex(LIF,index));
}


/// @brief Read a directory records from LIF image advancind directory index
/// @see lif_open_volume()
/// nOte: skip all purged LIF directory records
/// @param[in] *LIF: to LIF Volume/Diractoy structure
/// @return directory structure or NULL
MEMSPACE
lifdir_t *lif_readdir(lif_t *LIF)
{
    while(1)
    {
// Advance index first
// We start initialized at -1 by lif_open_volume() and lif_rewinddir()
        LIF->dirindex++;

        if( !lif_readdirindex(LIF, LIF->dirindex) )
            break;

        if( LIF->DIR.FileType == 0xffffUL )
            break;

        if(LIF->DIR.FileType)
            return( (lifdir_t *) &LIF->DIR );

// Skip purged records
    }
    return( NULL );
}


/// @brief Update free space
/// @parameter[in] *LIF: LIF structure
/// @return: LIF or NULL on error
MEMSPACE
lif_t *lif_updatefree(lif_t *LIF)
{
    int index = 0;
    int purgeindex = -1;

// Start of free space
    uint32_t start = LIF->filestart;
// Free sectors
    LIF->freesectors = LIF->filesectors;
// Used sectors
    LIF->usedsectors = 0;
// Purged files
    LIF->purged= 0;
// Files
    LIF->files = 0;
// Director pointer
    LIF->dirindex = 0;
// Directory EOF record
    LIF->EOFindex = 0;

/// Update free
    while(1)
    {
        if( !lif_readdirindex(LIF,index) )
        {
            return(NULL);
        }

        if(LIF->DIR.FileType == 0xffff)
        {
            LIF->EOFindex = index;
            if(purgeindex != -1)
            {
                LIF->EOFindex = purgeindex;

// Adjust purged file count
                LIF->purged -= (index - purgeindex);
// update EOF
                if(!lif_writedirEOF(LIF,purgeindex))
                {
                    return(NULL);
                }
            }
            break;
        }
        if(LIF->DIR.FileType == 0)
        {
            if(purgeindex == -1)
                purgeindex = index;
            LIF->purged++;
            ++index;
            continue;
        }
        purgeindex = -1;
        if(start > LIF->DIR.FileStartSector)
        {
            if(debuglevel & LIF_DEBUG)
                printf("lif_update_free:[%s] error previous record file area overlaps this one:[%s]\n", LIF->name, LIF->DIR.filename);

        }
        LIF->usedsectors += LIF->DIR.FileSectors;
        LIF->freesectors -= LIF->DIR.FileSectors;
        LIF->files++;
        ++index;
        start = LIF->DIR.FileStartSector + LIF->DIR.FileSectors;
    }
// rewind
    lif_rewinddir(LIF);
    return(LIF);
}


/// @brief Allocate index of free directory record
/// @param[in] *LIF: LIF pointer
/// @param[in] sectors: try to find specified free space
/// @return index or free record or -1 on error
MEMSPACE
int lif_newdir(lif_t *LIF, long sectors)
{
    int index;
    long start;

    int freestate, freeindex;
    long freestart;

// Directory index
    index = 0;

// Start of free space
    start = LIF->filestart;

// Update all file information
    if(lif_updatefree(LIF) == NULL)
    {
        printf("lif_newdir: not enough free space:[%ld] for size:[%ld]\n", (long)LIF->freesectors, (long) sectors);
        return(-1);
    }

// Not enough room ?
    if(sectors > (long)LIF->freesectors)
    {
        printf("lif_newdir: not enough free space:[%ld]\n", (long)LIF->freesectors);
        return(-1);
    }

// 0 = reading valid records
// 1 = found purged records
// 2 = found pured records with enough space to reuse
    freestate = 0;

// Update free space and EOF pointers
    while(1)
    {
// Write new EOF after current one
        if( !lif_readdirindex(LIF,index) )
        {
#if 0
            printf("lif_newdir: index:[%d] failed read at:[%ld] of [%ld] sectors, free:[%ld]\n",
                (int) index, (long) start, (long)sectors, (long)LIF->freesectors);
#endif
            break;
        }

// We hit the EOF record
        if(LIF->DIR.FileType == 0xffff)
        {

// Was enough free space found in purged area ?
// Do NOT need to update EOF!
            if(freestate == 2)
            {
// Update free pace for new file
                lif_dir_clear(LIF);
                LIF->DIR.FileStartSector = freestart;
                LIF->DIR.FileSectors = sectors;
                LIF->usedsectors += sectors;
                LIF->freesectors -= sectors;
                LIF->files++;
                LIF->purged--;
                LIF->dirindex = freeindex;
// Write new record (FileType is still EOF until data is updated by user)
                if( !lif_writedirindex(LIF,freeindex))
                {
                    break;
                }
                return(freeindex);
            }

            if(debuglevel & LIF_DEBUG)
                printf("lif_newdir: index:[%d] adding at:[%ld]to purged space:[%ld] sectors, free:[%ld]\n",
                    (int) index,(long)start,(long) sectors, (long)LIF->freesectors);

// Write new EOF after current one
            if( !lif_writedirEOF(LIF,index+1) )
            {
                break;
            }

            lif_dir_clear(LIF);
            LIF->DIR.FileStartSector = start;
            LIF->DIR.FileSectors = sectors;
            LIF->usedsectors += sectors;
            LIF->freesectors -= sectors;
            LIF->files++;
            LIF->dirindex = index;
// Write new record (FileType is still EOF until data is updated by user )
            if( !lif_writedirindex(LIF,index))
            {
                break;
            }
            return(index);
        }

// PURGED records
        if(LIF->DIR.FileType == 0)
        {
            if(freestate == 0)
            {
                freeindex = index;
                freestart = start;
                freestate = 1;
            }
            ++index;
            continue;
        }

// VALID record
        if(freestate == 1)
        {
            long freesectors;
// Compute sectors of purged space between valid records
// Note: LIF specs prohibit using old purged file start and sectors!
            freesectors = LIF->DIR.FileStartSector - start;
            if(freesectors >= sectors )
                freestate = 2;                    // Found free space in purged record
            else
                freestate = 0;                    // Try again
        }

// Computed start of next record
// Note: MUST be <= actual start of next VALID record or we have overlapping records!
        start = LIF->DIR.FileStartSector + LIF->DIR.FileSectors;
        ++index;
    }
// ERROR
    return(-1);
}


/// @brief Open LIF directory for reading
/// @param[in] *name: file name of LIF image
/// @param[in] *mode: "rb" = read, "rb+" = read/write
/// @return LIF pointer on sucesses or NULL on error
MEMSPACE
lif_t *lif_open_volume(char *name, char *mode)
{
    lif_t *LIF;
    stat_t sb, *sp;
    uint8_t buffer[LIF_SECTOR_SIZE];

    sp = lif_stat(name, (stat_t *)&sb);
    if(sp == NULL)
        return(NULL);

// To read LIF volume we must have at minimum two sectors
// volume header a directory entry
    if(sp->st_size < (long)LIF_SECTOR_SIZE*2)
    {
        if(debuglevel & LIF_DEBUG)
            printf("lif_open_volume:[%s] error volume header area too small:[%ld]\n", name, (long)sp->st_size);
        return(NULL);
    }

// Allocate LIF structure
    LIF = lif_calloc(sizeof(lif_t)+4);
    if(!LIF)
        return(NULL);

    LIF->name = lif_stralloc(name);
    if(!LIF->name)
    {
        lif_closedir(LIF);
        return(NULL);
    }

    LIF->imagebytes = sp->st_size;
    LIF->sectors = lif_bytes2sectors(sp->st_size);

    LIF->fp = lif_open(LIF->name,mode);
    if(!LIF->fp)
    {
        lif_closedir(LIF);
        return(NULL);
    }

// Volume header must be it least one sector
    if( lif_read(LIF, buffer, 0, LIF_SECTOR_SIZE) < LIF_SECTOR_SIZE)
    {
        if(debuglevel & LIF_DEBUG)
            printf("lif_open_volume:[%s] error read volume header failed\n", name);
        lif_closedir(LIF);
        return(NULL);
    }

// Unpack Volumes has the Directory start sector
    lif_str2vol(buffer, LIF);

// Validate basic Volume headers
    if( !lif_check_volume(LIF) )
    {
        if(debuglevel & LIF_DEBUG)
            printf("lif_open_volume:[%s] error volume validate failed\n", LIF->name);
        lif_closedir(LIF);
        return(NULL);
    }

// Initialize remaining LIF headers
    LIF->filestart = LIF->VOL.DirStartSector + LIF->VOL.DirSectors;
    LIF->filesectors = LIF->sectors - LIF->filestart;
    LIF->freesectors = LIF->filesectors;
    LIF->usedsectors = 0;
    LIF->purged = 0;
    LIF->files = 0;
    LIF->dirindex = 0;
    LIF->EOFindex = 0;

    if( lif_updatefree(LIF) == NULL)
    {
        if(debuglevel & LIF_DEBUG)
            printf("lif_open_volume:[%s] error directory check failed\n", LIF->name);
        lif_closedir(LIF);
        return(NULL);
    }

    if(debuglevel &LIF_DEBUG)
        lif_dump_vol(LIF, "Volume Listing");
    return( LIF );
}


/// @brief Display a LIF image file directory
/// @param[in] lifimagename: LIF disk image name
/// @return -1 on error or number of files found
MEMSPACE
void lif_dir(char *lifimagename)
{
    long bytes;
    lif_t *LIF;
    int index = 0;
    char *vol;

    int warn = ' ';

    LIF = lif_open_volume(lifimagename,"rb+");
    if(LIF == NULL)
        return;

    vol = (char *)LIF->VOL.Label;
    if(!vol[0])
        vol = "<EMPTY>";

    printf("Volume:[%s] Date:[%s]\n", vol, lif_lifbcd2timestr(LIF->VOL.date));

    printf("NAME         TYPE   START SECTOR        SIZE    RECSIZE   DATE\n");
    while(1)
    {

        if(!lif_readdirindex(LIF,index))
            break;

        if(LIF->DIR.FileType == 0xffff)
            break;

        bytes = (LIF->DIR.FileSectors * (long)LIF_SECTOR_SIZE);

        if((LIF->DIR.FileType & 0xFFFC) == 0xE010)
        {
            if(LIF->DIR.FileBytes && lif_bytes2sectors(LIF->DIR.FileBytes) == LIF->DIR.FileSectors)
            {
                bytes = LIF->DIR.FileBytes;
            }
            else
            {
                warn = '!';
                if(debuglevel & LIF_DEBUG)
                {
                    printf("LIF Directory:[%s] warning FileBytes:%ld as sectors:%ld != FileSectors:%ld\n",
                        LIF->DIR.filename,
                        (long) LIF->DIR.FileBytes,
                        (long) lif_bytes2sectors(LIF->DIR.FileBytes),
                        (long) LIF->DIR.FileSectors);
                }
            }
        }

// name type start size
        printf("%-10s  %04Xh      %8lXh   %9ld%c      %4d   %s\n",
            (LIF->DIR.FileType ? (char *)LIF->DIR.filename : "<PURGED>"),
            (int)LIF->DIR.FileType,
            (long)LIF->DIR.FileStartSector,
            (long)bytes,
            warn,
            (int)LIF->DIR.SectorSize,
            lif_lifbcd2timestr(LIF->DIR.date));

        ++index;
    }

    printf("\n");
    printf("%8ld Files\n", (long)LIF->files);
    printf("%8ld Purged\n", (long)LIF->purged);
    printf("%8ld Dir  start\n",   (long)LIF->VOL.DirStartSector);
    printf("%8ld Dir  sectors\n", (long)LIF->VOL.DirSectors);
    printf("%8ld Used sectors\n", (long)LIF->usedsectors);
    printf("%8ld Free sectors\n", (long)LIF->freesectors);

    lif_closedir(LIF);
}


/// @brief Find a LIF image file by name
/// @param[in] *LIF: directory pointer
/// @param[in] liflabel: File name in LIF image
/// @return Directory index of record
MEMSPACE
int lif_find_file(lif_t *LIF, char *liflabel)
{
    int index;

    if( !lif_checkname(liflabel) )
    {
        if(debuglevel & LIF_DEBUG)
            printf("lif_find_file:[%s] invalid characters\n", liflabel);
        return(-1);
    }
    if(strlen(liflabel) > 10)
    {
        if(debuglevel & LIF_DEBUG)
            printf("lif_find_file:[%s] liflabel too big\n", liflabel);
        return(-1);
    }

    if(LIF == NULL)
        return(-1);

    index = 0;
    while(1)
    {
        if(!lif_readdirindex(LIF,index))
            return(-1);

        if(LIF->DIR.FileType == 0xffff)
            return(-1);

        if( LIF->DIR.FileType && (strcasecmp((char *)LIF->DIR.filename,liflabel) == 0) )
            break;
        ++index;
    }
    return(index);
}


/** @brief  HP85 E010 ASCII LIF records
    ef [ff]* = no more data in this sector
    df size [ASCII] = data must fit inside this sector
    cf size [ASCII] = split data accross sector boundry, "6f" record continues at start of next sector
           Note: The 6f header is INSIDE the "cf" record and not included in the "cf" size value (yuck!)
    6f size [ASCII] = split continue (always starts at sector boundry)
    df 00 00 ef [ff]* = EOF (df size = 0) pad with "ef" and "ff" until sector end
    size = 16 bits LSB MSB

    Example:
    000080e0 : 4b 7c 22 0d cf 29 00 31 34 20 44 49 53 50 20 22  : K|"..).14 DISP "
000080f0 : 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 5f  :                _
00008100 : 6f 10 00 5f 5f 5f 5f 5f 5f 5f 5f 5f 5f 5f 5f 5f  : o.._____________
00008110 : 5f 22 0d df 2b 00 31 35 20 44 49 53 50 20 22 20  : _"..+.15 DISP "

cf 29 00 (19 is to sector end) (new sector start with 6F 10 00 (10 is remainder)
So 29 = 19 and 10 (yuck!)
*/

///@brief PAD wbuf to sector boundry
/// @param[in] offset: sector offset
/// @param[in] wbuf: E010 PAD data
/// @return size of E010 PAD data
/// FIXME assumes 256 byte secors
MEMSPACE
int lif_e010_pad_sector(long offset, uint8_t *wbuf)
{
    int ind;
    int pos,rem;

// Compute the current offset in this sector
    pos = (offset % (long)LIF_SECTOR_SIZE);
    if(!pos)
        return(0);

// Number of bytes free in this sector
    rem = (long)LIF_SECTOR_SIZE - (long)pos;

// Bytes written to wbuf
    ind = 0;
// PAD
    wbuf[ind++] = 0xEF;
    while(ind<rem)
        wbuf[ind++] = 0xff;

    pos = (offset + (long) ind)  % (long) LIF_SECTOR_SIZE;
// NEW SECTOR
// Debugging make sure we are at sector boundry
    if(pos)
    {
        if(debuglevel & LIF_DEBUG)
            printf("lif_e010_dap_sector: expected sector boundry: offset:%d\n", (int) pos);
        return(-1);
    }
    return(ind);
}


///@brief Convert an ASCII string into HP85 E010 format
/// @param[in] str: ASCII string to write
/// @param[in] offset: E010 data sector offset, only used in formatting wbuf with headers
/// @param[in] wbuf: E010 data result
/// @return size of E010 data
/// FIXME assumes 256 byte secors
MEMSPACE
int lif_ascii_string_to_e010(char *str, long offset, uint8_t *wbuf)
{
    int ind;
    int len;
    int pos,rem;

// String size
    len = strlen(str);

// Output buffer index
    ind = 0;

// Compute the current offset in this sector
    pos = (offset % (long) LIF_SECTOR_SIZE);
// Number of bytes free in this sector
    rem = (long) LIF_SECTOR_SIZE - (long) pos;

/// We ALWAYS pad a sector if:
///   There is no room for a 0xdf single header
///   - OR -
///   Spliting a string takes more space then padding here (extra header)
    if(rem < 6)
    {
        ind = lif_e010_pad_sector(offset, wbuf);
        if(ind < 0)
            return(ind);

// Compute the current offset in this sector
        pos = ((offset + ind) % (long) LIF_SECTOR_SIZE);
// Number of bytes free in this sector
        rem = (long) LIF_SECTOR_SIZE - pos;
    }

// Note: IMPORTANT we have >= 6 bytes!!!

// Do not have to split, there is enough room
    if(rem >= (3 + len))
    {

// Write string in new sector
// The full string + header will fit
        wbuf[ind++] = 0xDF;
        wbuf[ind++] = len & 0xff;
        wbuf[ind++] = (len >> 8) & 0xff;
// Write string
        while(*str)
            wbuf[ind++] = *str++;
    }
    else                                          /* No enough room split string */
    {
// Split strings need at least 6 header bytes
// We KNOW that there are at least 6 bytes in this sector

// CURRENT SECTOR
// String spans a sector , so split the string

// 1st Split string header
        wbuf[ind++] = 0xCF;
        wbuf[ind++] = len & 0xff;
        wbuf[ind++] = (len >>8) & 0xff;
// Write as much of the string as we can in this sector
        while(*str && ind<rem)
            wbuf[ind++] = *str++;

// NEW SECTOR
// Debugging make sure we are at sector boundry
        if(((offset + (long) ind)  % (long) LIF_SECTOR_SIZE))
        {
            if(debuglevel & LIF_DEBUG)
                printf("Expected sector boundry, offset:%d\n", (int) ((offset + ind) % LIF_SECTOR_SIZE) );
            return(-1);
        }

// Update remining string length
        len = strlen(str);
// 2nd Split string header
        wbuf[ind++] = 0x6F;
        wbuf[ind++] = (len & 0xff);
        wbuf[ind++] = (len>>8) & 0xff;
// Write string
        while(*str)
            wbuf[ind++] = *str++;
    }

    return(ind);
}


/// @brief Add ASCII file as E010 data to LIF image - or compute converted data size
/// To find size of formatted result only, without writting, set LIF to NULL
/// @param[in] userfile: User ASCII file source
/// @param[in] *LIF: Where to write file if set (not NULL)
/// @return size of formatted result
/// FIXME assumes 256 byte secors
MEMSPACE
long lif_add_ascii_file_as_e010_wrapper(lif_t *LIF, uint32_t offset, char *username)
{
    long bytes;
    int count;
    int size;
    int len;
    FILE *fi;

// strings are limited to less then this
    char str[LIF_SECTOR_SIZE+1];
// output buffer must be larger then a single sectors because of either headers or padding
    uint8_t obuf[LIF_SECTOR_SIZE*2];

    fi = lif_open(username, "rb");
    if(fi == NULL)
        return(-1);

    bytes = 0;
    count = 0;

// Read user file and write LIF records
// reserve 3 + LIF header bytes + 1 (EOS)
    while( fgets((char *)str,(int)sizeof(str) - 4, fi) != NULL )
    {
        trim_tail((char *)str);

        strcat((char *)str,"\r");                 // HP85 lines end with "\r"

        size = lif_ascii_string_to_e010(str, offset, obuf);
// Write string
// Now Write string
        if(LIF)
        {
            len = lif_write(LIF, obuf, offset, size);
            if(len < size)
            {
                fclose(fi);
                return(-1);
            }
        }

        offset += size;
        bytes += size;
        count += size;

        if(count > 256)
        {
            count = 0;
            if(LIF)
                printf("\tWrote: %8ld\r", (long)bytes);
        }
    }

    fclose(fi);

// Write EOF
    str[0] = 0;
// We only want to return the count of bytes in the file NOT the padding at the end
// Write EOF string with padding
    size = lif_ascii_string_to_e010(str, offset,obuf);

    if(LIF)
    {
        printf("\tWrote: %8ld\r", (long)bytes);
        len = lif_write(LIF, obuf, offset, size);
        if(len < size)
            return(-1);

    }

    offset += size;
    bytes += size;

// PAD
    size = lif_e010_pad_sector(offset, obuf);
    if(LIF)
    {
        len = lif_write(LIF, obuf, offset, size);
        if(len < size)
            return(-1);
    }

    if(LIF)
        printf("\tWrote: %8ld\r",(long)bytes);

    return(bytes);
}


/// @brief Convert and add ASCII file to the LIF image as type E010 format
/// The basename of the lifname, without extensions, is used as the LIF file name
/// @param[in] lifimagename: LIF image name
/// @param[in] lifname: LIF file name
/// @param[in] userfile: userfile name
/// @return size of data written into to LIF image, or -1 on error
/// FIXME assumes 256 byte secors
MEMSPACE
long lif_add_ascii_file_as_e010(char *lifimagename, char *lifname, char *userfile)
{
    long bytes;
    long sectors;
    long offset;
    int index;
    lif_t *LIF;
    stat_t st, *sp;

    if(!*lifimagename)
    {
        printf("lif_add_ascii_file_as_e010: lifimagename is empty\n");
        return(-1);
    }
    if(!*lifname)
    {
        printf("lif_add_ascii_file_as_e010: lifname is empty\n");
        return(-1);
    }
    if(!*userfile)
    {
        printf("lif_add_ascii_file_as_e010: userfile is empty\n");
        return(-1);
    }

//Get size and date info
    sp = lif_stat(userfile, (stat_t *)&st);
    if(!sp)
    {
        printf("lif_add_ascii_file_as_e010: userfile not found\n");
        return(-1);
    }

    if(debuglevel & LIF_DEBUG)
        printf("LIF image:[%s], LIF name:[%s], user file:[%s]\n",
            lifimagename, lifname, userfile);

// Find out how big converted file will be
    bytes = lif_add_ascii_file_as_e010_wrapper(NULL,0,userfile);
    sectors = lif_bytes2sectors(bytes);

    LIF = lif_open_volume(lifimagename,"r+");
    if(LIF == NULL)
        return(-1);

// Now find free record
    index = lif_newdir(LIF, sectors);
    if(index == -1)
    {
        printf("LIF image:[%s], not enough free space for:[%s]\n",
            lifimagename, userfile);
        lif_closedir(LIF);
        return(-1);
    }

// Initialize the free directory entry
    lif_fixname(LIF->DIR.filename, lifname,10);
    LIF->DIR.FileType = 0xe010;                   // 10
    lif_time2lifbcd(sp->st_mtime, LIF->DIR.date);

    LIF->DIR.VolNumber = 0x8001;                  // 26
    LIF->DIR.FileBytes = bytes;                   // 28
    LIF->DIR.SectorSize  = 0x100;                 // 30
    offset = LIF->DIR.FileStartSector * (long) LIF_SECTOR_SIZE;

    if(debuglevel & LIF_DEBUG)
        lif_dump_vol(LIF,"lif_after lif_newdir");

// Write converted file into free space first
    bytes = lif_add_ascii_file_as_e010_wrapper(LIF,offset,userfile);

    if(debuglevel & LIF_DEBUG)
    {
        printf("New Directory Information AFTER write\n");
        printf("Name:              %s\n", LIF->DIR.filename);
        printf("Index:            %4d\n", (int)index);
        printf("First Sector:     %4lxH\n", (long) LIF->DIR.FileStartSector);
        printf("File Sectors:     %4lxH\n", (long)LIF->DIR.FileSectors);
    }

// Write directory record
// Note: lif_newdir alrwady did the new EOF
    if( !lif_writedirindex(LIF,index))
    {
        lif_closedir(LIF);
        return(-1);
    }

    lif_closedir(LIF);

    printf("\tWrote: %8ld\n", bytes);

// Return file size
    return(bytes);
}


/// @brief Extract E010 type file from LIF image and save as user ASCII file
/// @param[in] lifimagename: LIF disk image name
/// @param[in] lifname:  name of file in LIF image
/// @param[in] username: name to call the extracted image
/// @return 1 on sucess or 0 on error
/// FIXME assumes 256 byte secors
MEMSPACE
int lif_extract_e010_as_ascii(char *lifimagename, char *lifname, char *username)
{
    lif_t *LIF;
    uint32_t start, end;                          // sectors
    long offset, bytes;                           // bytes
    int index;
    int i, len,size;
    int status = 1;
    int done = 0;

    time_t t;

    int ind,wind;
    FILE *fo;

// read buffer
    uint8_t buf[LIF_SECTOR_SIZE+4];
// Write buffer, FYI: will ALWAYS be smaller then the read data buffer
    uint8_t wbuf[LIF_SECTOR_SIZE+4];

    LIF = lif_open_volume(lifimagename,"r");
    if(LIF == NULL)
    {
        printf("LIF image not found:%s\n", lifimagename);
        return(0);
    }

    index = lif_find_file(LIF, lifname);
    if(index == -1)
    {
        printf("LIF File not found:%s\n", lifname);
        lif_closedir(LIF);
        return(0);
    }

    if((LIF->DIR.FileType & 0xFFFC) != 0xE010)
    {
        printf("File %s has wrong type:[%04XH] expected 0xE010..0xE013\n", username, (int) LIF->DIR.FileType);
        lif_closedir(LIF);
        return(0);
    }

    start = LIF->DIR.FileStartSector;
    end = start + LIF->DIR.FileSectors;

    t = lif_lifbcd2time(LIF->DIR.date);

    offset = start * (long) LIF_SECTOR_SIZE;

    fo = lif_open(username,"wb");
    if(fo == NULL)
    {
        lif_closedir(LIF);
        return(0);
    }

    printf("Extracting: %s\n", username);

    bytes = 0;
    wind = 0;
    ind = 0;

    while(lif_bytes2sectors(offset) <= end)
    {
// LIF images are always multiples of LIF_SECTOR_SIZE
        size = lif_read(LIF, buf, offset, LIF_SECTOR_SIZE);
        if(size < LIF_SECTOR_SIZE)
        {
            status = 0;
            break;
        }

        ind = 0;
        while(ind < LIF_SECTOR_SIZE && !done)
        {
            if(buf[ind] == 0xDF || buf[ind] == 0xCF || buf[ind] == 0x6F)
            {
                ++ind;
                len = buf[ind++] & 0xff;
                len |= ((buf[ind++] & 0xff) <<8);
// EOF ?
                if(len == 0)
                {
                    done = 1;
                    break;
                }
                if(len >= LIF_SECTOR_SIZE)
                {
                    printf("lif_extract_e010_as_ascii: string too big size = %d\n", (int)len);
                    status = 0;
                    done = 1;
                    break;
                }
            }
            else if(buf[ind] == 0xEF)
            {
// skip remaining bytes in sector
                ind = 0;
                break;
            }
            else
            {
                printf("lif_extract_e010_as_ascii: unexpected control byte:[%02XH] @ offset: %8lx, ind:%02XH\n", (int)buf[ind], offset, (int)ind);
                status = 0;
                done = 1;
                break;
            }
// write string
            for(i=0;i <len && ind < LIF_SECTOR_SIZE;++i)
            {
                if(buf[ind] == '\r' && i == len-1)
                {
                    wbuf[wind++] = '\n';
                    ++ind;
                    break;
                }
                else
                {
                    wbuf[wind++] = buf[ind++];
                }

                if(wind >= LIF_SECTOR_SIZE)
                {
                    size = fwrite(wbuf,1,wind,fo);
                    if(size < wind)
                    {
                        printf("lif_extract_e010_as_ascii: write error\n");
                        status = 0;
                        done = 1;
                        break;
                    }
                    bytes += size;
                    printf("\tWrote: %8ld\r", bytes);
                    wind = 0;
                }

            }                                     // for(i=0;i <len && ind < LIF_SECTOR_SIZE;++i)

        }                                         // while(ind < LIF_SECTOR_SIZE && status)

        offset += (long) LIF_SECTOR_SIZE;

    }                                             // while(offset <= end)

    lif_closedir(LIF);
// Flush any remaining bytes
    if(wind)
    {
        size = fwrite(wbuf,1,wind,fo);
        if(size < wind)
        {
            printf("lif_extract_e010_as_ascii: write error\n");
            status = 0;
        }
        bytes += size;
    }
    fclose(fo);
    if(t)
    {
        struct utimbuf times;
        times.modtime = t;
        times.actime = t;
        utime(username, (struct utimbuf *) &times);
    }
    sync();
    printf("\tWrote: %8ld\n", bytes);
    return(status);
}


/// @brief Extract a file from LIF image entry as standalone LIF image
/// @param[in] lifimagename: LIF disk image name to extract file from
/// @param[in] lifname:  name of file in LIF image we want to extract
/// @param[in] username: new LIF file to create
/// @return 1 on sucess or 0 on error
/// FIXME assumes 256 byte secors
MEMSPACE
int lif_extract_lif_as_lif(char *lifimagename, char *lifname, char *username)
{
// Master image lif_t structure
    lif_t *LIF;
    lif_t *ULIF;

    long offset, uoffset, bytes;
    int index;
    int i, size;
    int sectors;

    uint8_t buf[LIF_SECTOR_SIZE+4];

    LIF = lif_open_volume(lifimagename,"r");
    if(LIF == NULL)
    {
        printf("LIF image not found:%s\n", lifimagename);
        return(0);
    }

    index = lif_find_file(LIF, lifname);
    if(index == -1)
    {
        printf("File not found:%s\n", lifname);
        lif_closedir(LIF);
        return(0);
    }

    sectors = LIF->DIR.FileSectors;

//Initialize the user file lif_t structure
    ULIF = lif_create_volume(username, "HFSLIF",1,1,sectors);
    if(ULIF == NULL)
    {
        lif_closedir(LIF);
        return(0);
    }

// Only the start sector changes

// Copy directory record
    ULIF->DIR = LIF->DIR;

    ULIF->DIR.FileStartSector = 2;
    ULIF->filesectors = LIF->DIR.FileSectors;

    if( !lif_writedirindex(ULIF,0))
    {
        lif_closedir(LIF);
        lif_closedir(ULIF);
        return(0);
    }
    if( !lif_writedirEOF(ULIF,1) )
    {
        lif_closedir(LIF);
        lif_closedir(ULIF);
        return(0);
    }

    uoffset =  ULIF->filestart * (long) LIF_SECTOR_SIZE;

    offset = LIF->DIR.FileStartSector * (long) LIF_SECTOR_SIZE;

    bytes = uoffset;

    for(i=0;i<(int)LIF->DIR.FileSectors;++i)
    {
        size = lif_read(LIF, buf, offset,LIF_SECTOR_SIZE);
        if(size < LIF_SECTOR_SIZE)
        {
            lif_closedir(LIF);
            lif_closedir(ULIF);
            return(0);
        }

        lif_write(ULIF,buf,uoffset,LIF_SECTOR_SIZE);
        if(size < LIF_SECTOR_SIZE)
        {
            lif_closedir(LIF);
            lif_closedir(ULIF);
            return(0);
        }
        bytes += size;
        offset += size;
        uoffset += size;
        printf("\tWrote: %8ld\r", bytes);
    }
    lif_closedir(LIF);
    lif_closedir(ULIF);
    printf("\tWrote: %8ld\n", bytes);
    return(1);
}


/// @brief Add LIF file from another LIF image
/// @param[in] lifimagename: LIF image name
/// @param[in] lifname: LIF file name to copy file to
/// @param[in] userfile: LIF mage name copy file from
/// @return size of data written into to LIF image, or -1 on error
/// FIXME assumes 256 byte secors
MEMSPACE
long lif_add_lif_file(char *lifimagename, char *lifname, char *userfile)
{
// Master image lif_t structure
    lif_t *LIF;
    lif_t *ULIF;
    int index = 0;
    long offset, uoffset, start, bytes;
    int i, size;

    uint8_t buf[LIF_SECTOR_SIZE+4];

    if(!*lifimagename)
    {
        printf("lif_add: lifimagename is empty\n");
        return(-1);
    }
    if(!*lifname)
    {
        printf("lif_add: lifname is empty\n");
        return(-1);
    }
    if(!*userfile)
    {
        printf("lif_add: userfile is empty\n");
        return(-1);
    }

    if(debuglevel & LIF_DEBUG)
        printf("LIF image:[%s], LIF name:[%s], user file:[%s]\n",
            lifimagename, lifname, userfile);

// open  userfile as LIF image
    ULIF = lif_open_volume(userfile,"rb+");
    if(ULIF == NULL)
        return(-1);

// find lif file in user image
    index = lif_find_file(ULIF, lifname);
    if(index == -1)
    {
        printf("File not found:%s\n", lifname);
        lif_closedir(ULIF);
        return(0);
    }

    LIF = lif_open_volume(lifimagename,"rb+");
    if(LIF == NULL)
        return(-1);

// Now find a new free record that is big enough
    index = lif_newdir(LIF, ULIF->DIR.FileSectors);
    if(index == -1)
    {
        printf("LIF image:[%s], not enough free space for:[%s]\n",
            lifimagename, userfile);
        lif_closedir(LIF);
        lif_closedir(ULIF);
        return(-1);
    }

// Save start sector
    start = LIF->DIR.FileStartSector;

// Copy user image directory record to master image directory record
    LIF->DIR = ULIF->DIR;

// Restore FileStartSector
    LIF->DIR.FileStartSector = start;

// Master lif image file start in bytes
    offset  = LIF->DIR.FileStartSector * (long) LIF_SECTOR_SIZE;
// User lif image file start in bytes
    uoffset = ULIF->DIR.FileStartSector * (long) LIF_SECTOR_SIZE;
    bytes = 0;
// Copy file data
    for(i=0;i<(int)LIF->DIR.FileSectors;++i)
    {
// Read
        size = lif_read(ULIF, buf, uoffset, LIF_SECTOR_SIZE);
        if(size < LIF_SECTOR_SIZE)
        {
            lif_closedir(LIF);
            lif_closedir(ULIF);
            return(-1);
        }

// Write
        size = lif_write(LIF, buf, offset, LIF_SECTOR_SIZE);
        if(size < LIF_SECTOR_SIZE)
        {
            lif_closedir(LIF);
            lif_closedir(ULIF);
            return(-1);
        }
        offset += (long) LIF_SECTOR_SIZE;
        uoffset += (long) LIF_SECTOR_SIZE;
        bytes += (long) LIF_SECTOR_SIZE;
        printf("\tWrote: %8ld\r", bytes);
    }
    lif_closedir(ULIF);

// Write directory record
    if( !lif_writedirindex(LIF,index))
    {
        lif_closedir(LIF);
        return(-1);
    }
    lif_closedir(LIF);
    printf("\tWrote: %8ld\n", bytes);
    return(bytes);
}


/// @brief Delete LIF file in LIF image
/// @param[in] lifimagename: LIF image name
/// @param[in] lifname: LIF file name
/// @return 1 if deleted, 0 if not found, -1 error
MEMSPACE
int lif_del_file(char *lifimagename, char *lifname)
{
    lif_t *LIF;
    int index;

    if(!*lifimagename)
    {
        printf("lif_del_file: lifimagename is empty\n");
        return(-1);
    }
    if(!*lifname)
    {
        printf("lif_del_file: lifname is empty\n");
        return(-1);
    }
    if(debuglevel & LIF_DEBUG)
        printf("LIF image:[%s], LIF name:[%s]\n",
            lifimagename, lifname);

    LIF = lif_open_volume(lifimagename,"rb+");
    if(LIF == NULL)
        return(-1);

// Now find file record
    index = lif_find_file(LIF, lifname);
    if(index == -1)
    {
        lif_closedir(LIF);
        printf("LIF image:[%s] lif name:[%s] not found\n", lifimagename, lifname);
        return(0);
    }

// IF the next record is EOF then update EOF
    if(index >= LIF->EOFindex-1)
        LIF->DIR.FileType = 0xffff;
    else
        LIF->DIR.FileType = 0;

// re-Write directory record
    if( !lif_writedirindex(LIF,index) )
    {
        lif_closedir(LIF);
        return(-1);
    }

    lif_updatefree(LIF);

    lif_closedir(LIF);
    printf("Deleted: %10s\n", lifname);

    return(1);
}


/// @brief Rename LIF file in LIF image
/// @param[in] lifimagename: LIF image name
/// @param[in] oldlifname: old LIF file name
/// @param[in] newlifname: new LIF file name
/// @return 1 if renamed, 0 if not found, -1 error
MEMSPACE
int lif_rename_file(char *lifimagename, char *oldlifname, char *newlifname)
{
    int index;
    lif_t *LIF;

    if(!*lifimagename)
    {
        printf("lif_rename_file: lifimagename is empty\n");
        return(-1);
    }
    if(!*oldlifname)
    {
        printf("lif_rename_file: old lifname is empty\n");
        return(-1);
    }
    if(!*newlifname)
    {
        printf("lif_rename_file: new lifname is empty\n");
        return(-1);
    }

    if(!lif_checkname(newlifname))
    {
        printf("lif_rename_file: new lifname contains bad characters\n");
        return(-1);

    }

    LIF = lif_open_volume(lifimagename,"rb+");
    if(LIF == NULL)
        return(-1);

// Now find file record
    index = lif_find_file(LIF, oldlifname);
    if(index == -1)
    {
        printf("lif_rename:[%s] lif name:[%s] not found\n", lifimagename, oldlifname);
        lif_closedir(LIF);
        return(0);
    }
    lif_fixname(LIF->DIR.filename, newlifname, 10);

// re-Write directory record
    if( !lif_writedirindex(LIF,index))
    {
        lif_closedir(LIF);
        return(-1);
    }
    printf("renamed: %10s to %10s\n", oldlifname,newlifname);

    lif_closedir(LIF);

    return(1);
}


/// @brief Create/Format a LIF new disk image
/// This can take a while to run, about 1 min for 10,000,000 bytes
/// @param[in] lifimagename: LIF disk image name
/// @param[in] liflabel: LIF Volume Label name
/// @param[in] dirsectors: Number of LIF directory sectors
/// @param[in] sectors: total disk image size in sectors
///@return bytes writting to disk image
MEMSPACE
long lif_create_image(char *lifimagename, char *liflabel, uint32_t dirsectors, uint32_t sectors)
{
    uint32_t dirstart,filestart,filesectors,end;
    lif_t *LIF;

    if(!*lifimagename)
    {
        printf("lif_create_image: lifimagename is empty\n");
        return(-1);
    }
    if(!*liflabel)
    {
        printf("lif_create_image: liflabel is empty\n");
        return(-1);
    }
    if(!dirsectors)
    {
        printf("lif_create_image: dirsectors is 0\n");
        return(-1);
    }
    if(!sectors)
    {
        printf("lif_create_image: sectors is 0\n");
        return(-1);
    }

    dirstart = 2;
    filestart = dirstart + dirsectors;
    filesectors = sectors - filestart;
    end = filestart + filesectors;

    LIF = lif_create_volume(lifimagename, liflabel, dirstart, dirsectors, filesectors);
    if(LIF == NULL)
        return(-1);
    lif_close_volume(LIF);

    printf("\tFormatting: wrote %ld sectors\n", (long)end);
    return(end);
}
