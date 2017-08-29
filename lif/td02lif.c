/**
  @file   td02lif.c
  @brief  TeleDisk decoder library targetting LIF format images only
  @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved. GPL
  @see http://github.com/magore/hp85disk
  @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

  @help
   * LIF command help
     * lif td02lif image.td0 image.lif
       * Convert TeleDisk encoded LIF disk image into to pure LIF image
   * TELEDISK command help
     * td02lif file.td0 file.lif
       * Convert TeleDisk encoded LIF disk image into to pure LIF image

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @Credits
   * lif/teledisk
     * My TELEDISK LIF extracter
       * Note: The TeleDisk image MUST contain a LIF image  - we do NOT translate it
     * README.txt
       * Credits
     * Important Contributions (My converted would not have been possible without these)
       * Dave Dunfield, LZSS Code and TeleDisk documentation
         * Copyright 2007-2008 Dave Dunfield All rights reserved.
         * td0_lzss.h
         * td0_lzss.c
           * LZSS decoder
         * td0notes.txt
           * Teledisk Documentation
       * Jean-Franois DEL NERO, TeleDisk Documentation
         * Copyright (C) 2006-2014 Jean-Franois DEL NERO
           * wteledsk.htm
             * TeleDisk documenation
           * See his github project
             * https://github.com/jfdelnero/libhxcfe
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//#include "types.h"
#include <inttypes.h>

#include <time.h>
#include "lifsup.h"
#include "lifutils.h"
#include "td02lif.h"

/// @brief Dave Dunfiled LZSS expander
#include "td0_lzss.h"


disk_t disk;
   
/// @brief Teledisk liftel analysis and user overrides
liftel_t liftel;


/// @brief Extract TeleDisk image header data in architecture nutral way
/// @param[in] B: source data
/// @param[out] p: TeleDisk image header structure
/// @return TeleDisk image header size (not structure size)
int td0_unpack_disk_header(uint8_t *B, td_header_t *p)
{
    int i;
    uint16_t crc;

    B2S(B, 0, p->Header, 2);
    p->VolNO        = B2V_LSB(B,2,1);
    p->ChkSig       = B2V_LSB(B,3,1);
    p->TDVersion    = B2V_LSB(B,4,1);
    p->Density      = B2V_LSB(B,5,1);
    p->DriveType    = B2V_LSB(B,6,1);
    p->TrackDensity = B2V_LSB(B,7,1);
    p->DosMode      = B2V_LSB(B,8,1);
    p->Sides        = B2V_LSB(B,9,1);
    p->CRC          = B2V_LSB(B,10,2);

    crc = crc16(B,0, 0xA097, 10);
    if(p->CRC != crc)
        printf("TeleDisk error Header CRC16:%04Xh != %04Xh\n", (int)crc, (int)p->CRC);   
    return(TD_HEADER_SIZE);
}

/// @brief Extract TeleDisk comment header data in architecture nutral way
/// @param[in] B: source data
/// @param[out] p: TeleDisk comment header structure
/// @return TeleDisk comment header size (not structure size)
int td0_unpack_comment_header(uint8_t *B, td_comment_t *p)
{
    int i;
    uint16_t crc;

    p->CRC    = B2V_LSB(B,0,2);
    p->Size   = B2V_LSB(B,2,2);
    p->Year   = B2V_LSB(B,4,1);
    p->Month  = B2V_LSB(B,5,1);
    p->Day    = B2V_LSB(B,6,1);
    p->Hour   = B2V_LSB(B,7,1);
    p->Minute = B2V_LSB(B,8,1);
    p->Second = B2V_LSB(B,9,1);

    crc = crc16(B+2,0,0xA097, 8);
    return(crc);
}

/// @brief Extract TeleDisk track header data in architecture nutral way
/// @param[in] B: source data
/// @param[out] p: TeleDisk track header structure
/// @return TeleDisk track header size (not structure size)
int td0_unpack_track_header(uint8_t *B, td_track_t *p)
{
    int i;
    uint16_t crc;

    p->PSectors = B2V_LSB(B,0,1);
    p->PCyl     = B2V_LSB(B,1,1);
    p->PSide    = B2V_LSB(B,2,1);
    p->CRC      = B2V_LSB(B,3,1);
    crc = crc16(B,0,0xA097, 3);

    crc &= 0xff;

    if(p->CRC != crc)
    {
        // EOF ?
        if(p->PSectors != 255) 
        {
            printf("TeleDisk error Track CRC16:%04Xh != %04Xh\n", (int)crc, (int)p->CRC);
            printf("\tCyl:%02d, Side:%02d, Sectors:%d\n",
                (int) p->PCyl, (int) p->PSide, (int) p->PSectors);
        }
    }
    return(crc);
}

/// @brief Extract TeleDisk sector header data in architecture nutral way
/// @param[in] B: source data
/// @param[out] p: TeleDisk sector header structure
/// @return TeleDisk sector header size (not structure size)
int td0_unpack_sector_header(uint8_t *B, td_sector_t *p)
{
    int i;
    uint16_t crc;

    p->Cyl      = B2V_LSB(B,0,1);
    p->Side     = B2V_LSB(B,1,1);
    p->Sector   = B2V_LSB(B,2,1);
    p->SizeExp  = B2V_LSB(B,3,1);
    p->Flags    = B2V_LSB(B,4,1);
    p->CRC      = B2V_LSB(B,5,1);
    return(crc);
}

/// @brief Enable TeleDisk image compression mode
/// @param[in] flag: compression anable/disable
/// @return void
void td0_compressed(int flag)
{
    disk.compressed = flag;
    if(flag)
        init_decompress();
}

/// @brief Read TeleDisk image data block
/// Optionally decompress the data if decompression is enabled
/// @param[out] p: data buffer for read
/// @param[out] osize: size of object
/// @param[out] size: number of objects to read
/// @return 1 on success all bytes read, 0 on failure not all bytes read
int td0_read(void *p, int osize, int size, FILE *fp)
{
    int c;
    long count;
    long ind = 0;

    if(disk.compressed)
    {
        uint8_t *ptr = p;
        count = osize * size;
        while(count--)
        {
            c = lzss_getbyte(fp);
            if(c == EOF)
            {
                if(ind != (osize * size))
                    return(0);
                break;
            }
            *ptr++ = (c & 0xff);
            ++ind;
        }
    }
    else
    {
        ind = fread(p,osize,size,fp);
        if(ind != size)
            return(0);
    }
    return(1);
}

/// @brief Expand a Run Length encoded TeleDisk sector
/// Notes: the source length is encoded in the source data
/// Credits based on work (C) 2006-2014 Jean-François DEL NERO
///    Part of the HxCFloppyEmulator library GNU GPL version 2 or later
/// Rewitten for clarity,to enforce size limits, provide error reporting
/// and to avoid word order dependent assumptions,
/// @param[out] dst: destination expanded data
/// @param[in] src: source data
/// @param[in] max: maximum size of destination data
/// @return size of expanded data, negative if size > max
int td0_rle(uint8_t *dst, uint8_t *src, int max)
{
    int len;  
    int result;     // expanded result size, -expanded result size

    uint8_t type;   // Encoding type, we support 0,1,2
  
    int error = 0;
    uint8_t size;
    uint8_t repeat;

    result = 0;       // expanded size

    len  = B2V_LSB(src,0,2) -1;
    type = B2V_LSB(src,2,1);
    src += 3;

    // printf("td0_rle: len:%3d, type:%d\n", (int) len, (int)type);

    switch ( type )
    {
        case 0:

            if(len > max)
            {
                printf("td0_rle: type 0 len:%d > max:%d\n", len, max);
                memcpy(dst,src,max);
                result = -max;
                error = 1;
            }
            else
            {
                memcpy(dst,src,len);
                result = len;
            }
            break;

        case 1:
            result = 0;
            ///@actual length
            len  = B2V_LSB(src,0,2) << 1;
            if(len > max)
            {
                printf("td0_rle: type 1 len:%d > max:%d\n", len, max);
                error = 1;
                len = max;
            }
            while (len >= 2)
            {
                *dst++ = src[2];
                *dst++ = src[3];
                result += 2;
                len -= 2;
            }
            break;
        case 2:
            result = 0;
            do
            {
                if( !src[0] )
                {
                    size = src[1];
                    src += 2;
                    len -= 2;

                    if((result+size) > max)
                    {
                        printf("td0_rle: type 1 len:%d > max:%d\n", 
                            result+size, max);
                        error = 1;
                        break;
                    }

                    memcpy(dst,src,size);

                    result += size;
                    dst += size;
                    src += size;
                    len -= size;
                }
                else
                {
                    ///@brief block size
                    size = (1 << src[0]);

                    ///@brief repeat count
                    repeat = src[1];

                    src += 2;
                    len -= 2;

                    while(repeat--)
                    {

                        if((result+size) > max)
                        {
                            printf("td0_rle: type 1 len:%d > max:%d\n", 
                                result+size, max);
                            error = 1;
                            break;
                        }

                        memcpy(dst, src, size);

                        result += size;
                        dst += size;
                    }

                    src += size;
                    len -= size;
                }

            }
            while(len > 0);
            if(len)
            {
                printf("td0_rle: type 1 len:%d < 0\n", len);
                error = 1;
            }
            break;

        default:
            printf("td0_rle error unsupported type:%02Xh\n", type);
            result = -1;
            break;
    }
    if(error)
        return(-result);
    return (result);
}

/// @brief Display TeleDisk track sector values
/// @param[in] sector: track sector structure
/// @param[in] index: sector index (not sector number)
/// @return void
void td0_trackinfo(disk_t *disk, int trackind, int index)
{
    printf("cylinder:%02d, head:%02d, sector:%02d, size:%d, index:%d\n", 
        (int) disk->track[trackind].sectors[index].cylinder,
        (int) disk->track[trackind].sectors[index].side,
        (int) disk->track[trackind].sectors[index].sector,
        (int) disk->track[trackind].sectors[index].size,
        (int) index);

}
/// @brief Display TeleDisk sector data
/// @param[in] P: td_sector pointer
/// @return void
void td0_sectorinfo(td_sector_t *P)
{
    printf("cyl: %02d, side: %02d, sector: %02d, size: %03d\n",
        (int) P->Cyl,
        (int) P->Side,
        (int) P->Sector,
        (int) 128 << P->SizeExp);
}

/// @brief Convert Denisity to bitrate
/// @We don't care about the density except as a possible filter
/// @param[in] Density
/// @return bitrate
long td0_density2bitrate(uint8_t density)
{
    long bitrate = 250000;
    switch(density)
    {
        case 0:
            bitrate = 250000;
            break;
        case 1:
            bitrate = 300000;
            break;
        case 2:
            bitrate = 500000;
            break;
        default:
            break;
    }
    return(bitrate);
}

/// @brief Opne TeleDisk image file process header and optional comment block
/// @param[in] *disk: TeleDisk information structure
/// @param[in] *name: image file name
/// @return 1 on success 0 on error
/// Note: on success
/// disk->fi has the file handle
/// disk->td0_name has the file name
/// disk->compresssed if image is compressed
/// disk->td_header has teledisk header
/// disk->td_comment has optional teledisk comment header or zeroed
///    If disk.td_comment.Size > 0
/// disk->comment has optional comment string or empty string
///    If disk.td_comment.Size > 0
int td0_open(disk_t *disk, char *name)
{
    uint16_t crc;
    tm_t tm;
    char timebuf[32];
    uint8_t headerbuf[TD_HEADER_SIZE+1];
    uint8_t commentbuf[TD_COMMENT_SIZE+1];

    // default file time if not in comment
    disk->t = time(0);

    memset((td_header_t *) &disk->td_header,0,sizeof(disk->td_header));
    memset((td_comment_t *) &disk->td_comment,0,sizeof(disk->td_comment));

    // Default is NO comment
    disk->comment = "";
    disk->compressed = 0;

    // TeleDisk Image name
    disk->td0_name = lif_stralloc(name);

    disk->fi = fopen(disk->td0_name,"rb");
    if(!disk->fi)
    {
        printf("Error: Can't open TeleDisk file: %s\n",disk->td0_name);
        return(0);
	}

    ///@process TeleDisk disk image header
    if( fread( headerbuf, 1, TD_HEADER_SIZE, disk->fi ) != TD_HEADER_SIZE)
    {
        printf("Error: Can't read TeleDisk header: %s\n", disk->td0_name);
        return(0);
    }
    td0_unpack_disk_header(headerbuf, (td_header_t *)&disk->td_header);


    ///@brief we expect "TD" (uncompressed) or "td" (compressed) format
	if(MATCH(disk->td_header.Header,"TD"))
	{
        td0_compressed(0);
	}

	else if(MATCH(disk->td_header.Header,"td"))
	{
        td0_compressed(1);
	}
	else
	{
		printf("Error: bad TeleDisk header: %s\n", disk->td_header.Header);
        return(0);
	}

    ///@brief We only accept TeleDisk versions >= 10 && version <= 21
    printf("TeleDisk file:         %s\n",disk->td0_name);
	printf("\tVersion:       %02d\n",disk->td_header.TDVersion);
    if(disk->compressed)
        printf("\tAdvanced Compression\n");
    else
        printf("\tNot Compressed\n");
	printf("\tDensity:       %02Xh\n",disk->td_header.Density);
	printf("\tDriveType:     %02Xh\n",disk->td_header.DriveType);
	printf("\tTrackDensity:  %02Xh\n",disk->td_header.TrackDensity & 0x7f);
	printf("\tDosMode:       %02Xh\n",disk->td_header.DosMode);
	printf("\tSides:         %02d\n",disk->td_header.Sides);

	if((disk->td_header.TDVersion>21) || (disk->td_header.TDVersion<10))
	{
		printf("Error: only TeleDisk versions 10 to 21 supported\n");
        return(0);
	}


    // Do we have a Comment Block ?
    // Note: We have a Comment if bit 7 of TrackDensity is set 
	if(disk->td_header.TrackDensity & 0x80)
	{

		if( !td0_read( commentbuf, 1, TD_COMMENT_SIZE, disk->fi ) )
        {
            printf("Error: reading commment\n");
            return(0);
        }

        crc = td0_unpack_comment_header(commentbuf, (td_comment_t *)&disk->td_comment);

        disk->comment = calloc(disk->td_comment.Size+1,+1);
        if(!disk->comment)
        {
            printf("Can't allocate comment buffer of %d bytes\n", disk->td_comment.Size);
            return(0);
        }
        disk->comment[disk->td_comment.Size] = 0;

		if( !td0_read( disk->comment, 1, disk->td_comment.Size,disk->fi) )
        {
            printf("Error: reading commment\n");
            return(0);
        }

        crc = crc16(disk->comment,crc,0xA097,disk->td_comment.Size);
        if(disk->td_comment.CRC != crc)
        {
            printf("Warning: Comment CRC16:%04Xh != %04Xh\n", (int)crc, (int)disk->td_comment.CRC);
        }

// FIXME convert to LIF date
        ///@brief Time to Unix format
        tm.tm_year = (int)disk->td_comment.Year;
        tm.tm_mon = (int)disk->td_comment.Month;
        tm.tm_mday = (int)disk->td_comment.Day;
        tm.tm_hour = (int)disk->td_comment.Hour;
        tm.tm_min = (int)disk->td_comment.Minute;
        tm.tm_sec = (int)disk->td_comment.Second;
        tm.tm_wday = 0;
        tm.tm_yday = 0;

        // DATE
        printf("\tComment Size:  %d\n", disk->td_comment.Size);
        printf("\tComment Date:  %s\n", asctime_r((tm_t *) &tm, timebuf));

        disk->t = timegm((tm_t *) &tm);

        // COMMENT
        printf("%s\n\n", disk->comment);
    }   // if(disk->td_header.TrackDensity & 0x80)
    else
    {
        disk->td_comment.Size = 0;
        printf("\tNo Comment Block\n");
        // Empty Comment
    }
    return(1);
}


/// @brief Read TeleDisk image file  and save all sector data
/// @param[in] dist: TD0 image data
/// @return 1 on success 0 on error
int td0_read_disk(disk_t *disk)
{
	int i,j;
    int cyl,head,sector,size;
    int t,s;
    int index, tracksectors, found, result;
    int status;
    long count = 0;  
	td_track_t  td_track;
	td_sector_t td_sector;
    uint8_t trackbuf[TD_TRACK_SIZE];
    uint8_t sectorbuf[TD_SECTOR_SIZE];

	uint8_t buffer[8*1024];


    // ==============================================================
    // FIXME
    // Reasons - eliminates the overhead of closing out on errors
    // Open file in main
    // Init data structures in main
    // process TD0 to LIF data from main
    // Close file in main
    // Free data in main
    // Move all disk processing into a function
    // Move all sector processing into a fuction
    // Move all track processing into a function
    // ==============================================================

    ///@brief Process all image Data
	for(t = 0; t < MAXTRACKS; ++t)
	{
        // PROCESS TRACK

        // TRACK HEADER
        if( !td0_read(trackbuf,1, TD_TRACK_SIZE, disk->fi) )
        {
            printf("Error: reading track header\n");
            return (0);
        }
        td0_unpack_track_header(trackbuf, (td_track_t *)&td_track);

        if(td_track.PCyl >= MAXCYL )
        {
            printf("Error: cylinder number %02d >= %02d\n", td_track.PCyl, MAXCYL);
            return (0);

        }

        if(td_track.PSide >= MAXSIDES)
        {
            printf("Error: side number %02d >= %02d\n", td_track.PSide, MAXSIDES);
            return (0);

        }

        disk->track[t].Cyl = td_track.PCyl;
        disk->track[t].Side = td_track.PSide;


        // ====================================================
        /// EOF?  TeleDisk specification says EOF is 255 sectors
        if(td_track.PSectors == 0xff)
            break;
        // ====================================================

        if(debuglevel & 0x400)
            printf("Track: Cyl: %02d, Side: %02d, Sectors: %02d\n",
                (int)td_track.PCyl, (int)td_track.PSide, (int)td_track.PSectors);

        // Initialize track data

        ///@brief Process all track sectors
		for ( s=0; s < td_track.PSectors; s++ )
		{
            ///@brief Sector Header
            if( !td0_read(sectorbuf,1, TD_SECTOR_SIZE,disk->fi) )
            {
                printf("Error: reading sector header\n");
                printf("\t Track: Cyl: %02d, Side: %02d, Sectors: %02d\n",
                    (int)td_track.PCyl, (int)td_track.PSide, (int)td_track.PSectors);
                return (0);
            }

            td0_unpack_sector_header(sectorbuf, (td_sector_t *)&td_sector);


            //FIXME add flag override for this test
            if(td_track.PCyl != td_sector.Cyl )
            {
                printf("Warning: track Cyl:%d != sector Cyl:%d skipping\n",
                    (int)td_track.PCyl, (int) td_sector.Cyl);
                printf("\t");
                td0_sectorinfo((td_sector_t *) &td_sector);
            }

            //FIXME add flag override for this test
            if(td_track.PSide != td_sector.Side)
            {
                printf("Warning: track side:%d != sector side:%d skipping\n",
                    (int)td_track.PSide, (int) td_sector.Side);
                printf("\t");
                td0_sectorinfo((td_sector_t *) &td_sector);
            }

            // We insert sectors using their sector ID as the index offset
            // This "sorts" them in order

			index = td_sector.Sector;


            // FYI as long as MAXSECTORS is 256 this will NEVER happen
            // This test is just here in case someone changes MAXSECTORS
            if(index >= MAXSECTORS)
            {
                printf("Error: sector index: %d >= MAXSECTORS\n",
                    (int)index );
                printf("\t");
                td0_sectorinfo((td_sector_t *) &td_sector);
                return (0);
            }


            //FIXME we can not deal with duplicate sectors so skip them
            if( disk->track[t].sectors[index].sector != -1)
            {
                printf("Warning: skipping duplicate sector: %02d,  skipping\n", index);
                printf("\t");
                td0_sectorinfo((td_sector_t *) &td_sector);
                continue;
            }

			disk->track[t].sectors[index].sector = index;

			disk->track[t].sectors[index].cylinder = td_sector.Cyl;
			disk->track[t].sectors[index].side = td_sector.Side;
			disk->track[t].sectors[index].size = 128<<td_sector.SizeExp;
			disk->track[t].sectors[index].data = 
            calloc(disk->track[t].sectors[index].size,1);

			if(td_sector.Flags & 0x02)
			{
                printf("Warning: alternate CRC flag not implemented\n");
                printf("\t");
                td0_trackinfo(disk,t,index);
			}
			if(td_sector.Flags & 0x04)
			{
                printf("Warning: alternate data mark not implemented\n");
                printf("\t");
                td0_trackinfo(disk,t,index);
			}
			if(td_sector.Flags & 0x20)
			{
                printf("Warning: missing address mark not implemented\n");
                printf("\t");
                td0_trackinfo(disk,t, index);
			}

            ///@brief first test verifies sector < 2K in size
            ///       second test verifies no data or address mark errors
			if  ( !(td_sector.SizeExp & 0xf8) && !(td_sector.Flags & 0x30))
			{
                int len;
                uint16_t crc;

                ///@brief Compute how much data to read
                memset(buffer,0,sizeof(buffer));
                // Read Sector Data
                if( !td0_read(buffer,1, sizeof(uint16_t),disk->fi) )
                {
                    printf("Error: reading sector header\n");
                    printf("\t");
                    td0_trackinfo(disk,t,index);
                    return(0);
                }
                len = B2V_LSB(buffer,0,2);

                if( !td0_read(buffer+2,1, len,disk->fi) )
                {
                    printf("Error: reading sector data\n");
                    printf("\t");
                    td0_trackinfo(disk, t,index);
                    return (0);
                }

                ///@brief td0_rle does not use len
                /// internal data in the buffer does this
                /// We limit the expansion to sector size
				result = td0_rle(disk->track[t].sectors[index].data, buffer, disk->track[t].sectors[index].size);

                if(result != disk->track[t].sectors[index].size)
                {
                    // Very unlikey we can recover from this!
                    printf("Error: RLE result:%d != sector size:%d\n",
                        (int) result , (int) disk->track[t].sectors[index].size);
                    printf("\t");
                    td0_trackinfo(disk,t,index);
                    return (0);
                }

                crc = crc16(disk->track[t].sectors[index].data,0,0xA097,result);
                crc &= 0xff;
                if(td_sector.CRC != crc)
                {
                    printf("Warning: Sector CRC16:%04Xh != %04Xh\n", 
                        (int)crc, (int)td_sector.CRC);
                    printf("\t");
                    td0_trackinfo(disk,t,index);
                }

			}  // if ( !(td_sector.SizeExp & 0xf8) && !(td_sector.Flags & 0x30))
            else
            {
                printf("Warning: unsupported sector flags:%02Xh\n", (int)td_sector.Flags);
                printf("\t");
                td0_trackinfo(disk,t,index);
            }

        }   // for ( i=0;i < td_track.PSectors;i++ )

        // Done processing first sector
        /// =========================================

	}   // for(t = 0; t < MAXTRACKS; ++t)

	return (1);
}


/// @brief Analize TeleDisk disk sector data 
/// Problem: We may a disk reformatted multiple times
///          Each format may have used differing sector sizes, number of tracks, sides
/// For example:
///   First format 80 track disk, two sided, and 9 512 byte sectors per track
///   Second format 35 tracks, single sided, 16 sectors 256 bytes size per track
/// Clearly we only want the 35 tracks on the first side
/// 
/// What we do is examine the first 30 tracks 
///   (note: 30 is just a safe number less then 35)
/// On side 1, on 30 tracks
///   Find first sector and save its size
///   Reject sectors that do not match size
///   Find Last sector matching size
///   Count sectors matching size
/// On side 2, on 30 tracks
///   Reject sectors that do not match size
///   Find first sector and save its size
///   Find Last sector matching size
///   Count sectors matching size
///
/// If the disk had two sides recorded in the header
///      Reject second side if side sector counts do NOT match
///      Reject second side if NO remaining sectors
///      Reject second side if sector ranges do not match or continue in range
/// 
/// @param[in] dist: TD0 image data
/// @retrun 1 if OK, 0 on error
int td0_analize_format(disk_t *disk)
{
    int s,t;
    int i,j;
    int flag;
    int sector,size;
    int maxtracks;
    int maxsectors[2];
    int reject_side_two = 0;
 
 
    // SIDES
    liftel.Sides = disk->td_header.Sides;

    if( disk->td_header.Sides == 2)
    {
        if( liftel.u.sides != -1)
        {
            if(liftel.u.sides == 1)
            {
                printf("Warning: Override number of sides from 2 to 1\n");
                reject_side_two = 1;
                liftel.Sides = 1;
            }
            else if( liftel.u.sides != 2)
            {
                printf("Error: NO sectors detected\n");
                printf("Giving up!\n");
                liftel.error = 1;
                liftel.state = TD0_DONE;
                return(0);
            }
        }
    }
    else if( disk->td_header.Sides == 1)
    {
        if(liftel.u.sides != -1 && liftel.u.sides == 2)
        {
            printf("Error: NO sectors detected\n");
            printf("Giving up!\n");
            liftel.error = 1;
            liftel.state = TD0_DONE;
            return(0);
        }
    }

    // TRACKS
    liftel.Tracks = 0;

    // SIZE
    liftel.Size = 0;

    for(i=0;i<2;++i)
    {
        liftel.s.first[i] = MAXSECTORS-1;
        liftel.s.last[i] = 0;
        liftel.s.size[i] = 0;
        liftel.s.sectors[i] = 0;
    }

    // ===================================================================
    // Analize sector data - see notes at the start of this function

    // Find parameters of smallest possible format 35 tracks less a few
    // Single sided 35 tracks
    maxtracks = 30;
    if(disk->td_header.Sides > 1)
        maxtracks *= 2;

    // find FIRST sector and its SIZE in the search area
    for( t = 0; t < maxtracks; ++ t)
    {
        // Scan side one only
        for ( s=0; s < MAXSECTORS ; s++ )
        {
            sector = disk->track[t].sectors[s].sector;
            if(sector == -1)
                continue;

            size = disk->track[t].sectors[s].size;

            // We only test even tracks to be really safe
            if(sector < liftel.s.first[t & 1] )
            {
                liftel.s.first[t & 1] = sector;
                liftel.s.size[t & 1] = size;
            }
        }       // for ( s=0; s < MAXSECTORS ; s++ )
    }       // for( t = 0; t < maxtracks; ++ t)


    // If size of first sector was NOT found
    if(liftel.s.size[0] == 0)
    {
        printf("Error: NO sectors detected\n");
        printf("Giving up!\n");
        liftel.error = 1;
        liftel.state = TD0_DONE;
        return(0);
    }

    // Find LAST sectors matching SIZE Within the search area 
    // Find maximum NUMBER of sectors on each side matching size
    for( t = 0; t < maxtracks; ++t)
    {
        maxsectors[t & 1] = 0;
        // Scan side one only
        for ( s = 0; s < MAXSECTORS ; s++ )
        {
            sector = disk->track[t].sectors[s].sector;
            if(sector == -1)
                continue;

            size = disk->track[t].sectors[s].size;
            if(liftel.s.size[t & 1] != size )
                continue;

            // Do NOT include replacement sectors in LAST test
            if( sector < 100 && sector > liftel.s.last[t & 1] )
            {
                liftel.s.last[t & 1] = sector;
            }
            // Count ALL sectors remaining regardless of type
            ++maxsectors[t & 1];
        }       // for ( s=0; s < MAXSECTORS ; s++ )


        // Find the largest sector count
        if(maxsectors[t & 1] > liftel.s.sectors[t & 1])
            liftel.s.sectors[t & 1] = maxsectors[t & 1];
    }       // for( t = 0; t < maxtracks; ++ t)

    // ===================================================================


    liftel.Size = liftel.s.size[0];

    // ===================================================================
    // See if SIDE two matches side one formatting

    // Note: We test the number of sides LATER on AFTER these tests
    // Normally first and second side will match
    // But if they do NOT match then we only want FIRST side


    // Sector SIZE of side 1 mismatch ?
    if(liftel.s.size[0] != liftel.s.size[1])
    {
        // Reject side two
        reject_side_two = 1;
    }

    // Sector COUNT mismatch ?
    if(liftel.s.sectors[0] != liftel.s.sectors[1])
    {
        // Reject side two
        reject_side_two = 1;
    }

    if(liftel.s.first[0] != liftel.s.first[1])
    {
        // If the FIRST sector differs on each side
        //   Then the sector numbering MUST be a continuation 
        if( ( liftel.s.last[0] + 1) != liftel.s.first[1] )
        {
            // Reject side two
            reject_side_two = 1;
        }
    }

    // If TeleDisk sides == 1 AND reject_side_two is set something is VERY VERY wrong

    if(disk->td_header.Sides == 1 && reject_side_two)
    {
        printf("Error: Damaged Disk - this should never happen\n");
        printf("Giving up!\n");
        liftel.error = 1;
        liftel.state = TD0_DONE;
        return(0);
    }

    if(reject_side_two)
    {
        // Update sides
        liftel.Sides = 1;
    }
    // ===================================================================

    // ===================================================================
    // DELETE rejected sectors and sides
    // ZERO fill missing sectors
    // REMAP "extra" sectors if they exist

    // Default size from fisrt sector of first side
    if(liftel.u.size != -1)
    {
        for(i=0; i< liftel.Sides; ++i)
        {
            if(liftel.s.size[i] && liftel.u.size != liftel.s.size[i])
            {
                printf("Warning: Side %d user size:[%03d] NOT same as detected:[%03d]\n",
                    i, liftel.u.size, liftel.s.size[i]);
            }
        }
        liftel.Size = liftel.u.size;
    }

    // We just use the sector count from first side as second side count must match
    // (We previously tested sector count mismatch)

    liftel.Sectors = liftel.s.sectors[0];
    liftel.Tracks = 0;

    for( t = 0; t < MAXTRACKS; ++ t)
    {
        disk->track[t].Sectors = 0;
        disk->track[t].First = 0;
        disk->track[t].Last = 0;
        disk->track[t].Size = 0;


        for ( s=0; s < MAXSECTORS ; s++ )
        {
            sector = disk->track[t].sectors[s].sector;
            if(sector == -1)
                continue;

            flag = 0;

            // Delete SIZE mismatch
            size = disk->track[t].sectors[s].size;
            if(size != liftel.Size)
                flag = 1;

            if(disk->td_header.Sides == 2 && (t & 1) == 1)
            {
                // Delete unused SIDE two
                if(liftel.u.sides == 1)
                    flag = 1;

                // Delete SIDE two - previously rejected
                if( reject_side_two)
                    flag = 1;
            }

            // Delete and FREE rejected sectors
            if(flag )
            {
                disk->track[t].sectors[s].sector = -1;
                disk->track[t].sectors[s].size = 0;
                if(disk->track[t].sectors[s].data != NULL)
                {
                    free(disk->track[t].sectors[s].data);
                    disk->track[t].sectors[s].data = NULL;
                }
            }
            else
            {
                // Count valid sectors
                disk->track[t].Sectors++;
            }
        }       // for ( s=0; s < MAXSECTORS ; s++ )

        // We have sectors on this side
        if( disk->track[t].Sectors )
        {
#if 0
printf("track: [%d]\n", t);
printf("sector count :[%d]\n", maxsectors[t & 1]);
printf("Sectors:%d\n", disk->track[t].Sectors);
printf("flag:%d\n", flag);
printf("liftel.Size:%d\n", liftel.Size);
#endif
            disk->track[t].Sectors = liftel.s.sectors[t & 1]; 
            disk->track[t].First = liftel.s.first[t & 1];
            disk->track[t].Last  = liftel.s.last[t & 1];
            disk->track[t].Size  = liftel.Size;
            liftel.Tracks++;
        }
    }

    // Done deleting sectors
    // Track 0, Side 0 MUST have sectors at this point or FAIL
    if(disk->track[0].Sectors == 0)
    {
        printf("Warning: track:0 has zero sectors\n");
        liftel.error = 1;
        liftel.state = TD0_DONE;
        return(0);
    }
    
    for( t = 0; t < MAXTRACKS; ++ t)
    {
        // We have sectors on this side
        if( disk->track[t].Sectors == 0)
            continue;

        // Test if we have an missing sectors in a track
        // If we do have missing sectors on a track then
        // 1) Then look for sectors numbered >= 100 as replacements
        // 2) If none is found then ZERO fill the missing sector
        for (s = disk->track[t].First; s <= disk->track[t].Last; s++ )
        {
            // Sector is NOT missing
            if( disk->track[t].sectors[s].sector != -1)
                continue;
            
            // First look for a replacement >= 100
            for(j=100;j<MAXSECTORS;++j)
            {
                // Look for a replacement
                if(disk->track[t].sectors[j].sector == -1)
                   continue;

                // We also must match the default sector size
                if(disk->track[t].sectors[j].size != liftel.Size)
                   continue;

                // REMAP this on to the missing one
                disk->track[t].sectors[s].cylinder = disk->track[t].sectors[j].cylinder;
                disk->track[t].sectors[s].side = disk->track[t].sectors[j].side;
                disk->track[t].sectors[s].sector = s;
                disk->track[t].sectors[s].size = disk->track[t].sectors[j].size;
                disk->track[t].sectors[s].data = disk->track[t].sectors[j].data;

                // Delete the REMAP sector so it will not get reused !!
                disk->track[t].sectors[j].cylinder = 0;
                disk->track[t].sectors[j].side = 0;
                disk->track[t].sectors[j].sector = -1;
                disk->track[t].sectors[j].size = 0;
                disk->track[t].sectors[j].data = NULL;

                printf("Warning: Sector:%02d missing - found alternate sector:%02d\n", 
                    s, j);
                printf("\t Location: ");

                td0_trackinfo(disk,t,s);
                break;

            }   // for(j=100;j<MAXSECTOR;++j)

            // Did we find a replacement ?
            if(disk->track[t].sectors[s].sector != -1)
                continue;

            // NO replacement 

            // ZERO fill a new one with default size
            disk->track[t].sectors[s].data = calloc(liftel.Size,1);
            disk->track[t].sectors[s].size = liftel.Size;
            // mark sector as in use
            disk->track[t].sectors[s].sector = s;

            printf("Warning: Sector:%02d missing - zero filling\n", s);
            printf("\t Location: ");
            printf("Track: Cyl: %02d, Side: %02d\n",
               (int)disk->track[t].Cyl, (int)disk->track[t].Side);

        }       // for (s = disk->track[t].First; s <= disk->track[t].Last; s++ )

    }       // for( t = 0; t < MAXTRACKS; ++ t)
    // ============================================


    // ============================================
 
    liftel.state = TD0_START;
 
    printf("\n");
    printf("Disk Layout\n");

    printf("\t Sides:             %2d\n", liftel.Sides);
    printf("\t Tracks:            %2d\n", liftel.Tracks);
    printf("\t Sectors Per Track: %2d\n", liftel.Sectors);
    printf("\t Sector Size:      %3d\n", liftel.Size);
    
    // Always display all sides as discovered
    for(i=0; i< disk->td_header.Sides; ++i)
    {
        printf("Side: %d\n", i);
        printf("\t Sector numbering: %2d to %2d\n", 
            liftel.s.first[i], (liftel.s.first[i] + liftel.s.sectors[i] - 1) );
        printf("\t Sector size:     %3d\n", liftel.s.size[i]);
        printf("\t Sector count:     %2d\n", liftel.s.sectors[i]);
    }
    printf("\n");

    // If we rejected sides or sectors display a summary
    if(reject_side_two)
    {
        printf("Warning: rejecting side two\n");
        if(liftel.s.size[1] && liftel.s.size[0] != liftel.s.size[1])
            printf("\t Warning: Sector size:  NOT the same on both sides\n");

        if(liftel.s.sectors[1] && liftel.s.sectors[0] != liftel.s.sectors[1])
            printf("\t Warning: Sector count: NOT the same on both sides\n");

        if(liftel.s.first[1] && liftel.s.first[0] != liftel.s.first[1])
        {
            // If the FIRST sector differs then it MUST be a continuation - or - FAIL
            if( ( liftel.s.last[0] + 1) != liftel.s.first[1] )
                printf("\t Warning: Sector range: NOT a continuation of side 0\n");
        }
        printf("\n");
    }

    return(1);
}

/// @brief save remaining sectors as LIF data
/// @param[in] data: sector data
/// @param[in] LIF: LIF structure
/// @retrun 1 if OK, 0 on error
int td0_save_lif(disk_t *disk, lif_t *LIF)
{

    int t,s;
    uint8_t *ptr;
    long sectors = 0;

    for(t=0; t<MAXTRACKS; ++t)
    {
        if(liftel.Sectors == disk->track[t].Sectors)
        {
            for (s = disk->track[t].First; s <= disk->track[t].Last; s++ )
            {
                ptr = disk->track[t].sectors[s].data;
                if(!ptr || disk->track[t].sectors[s].sector == -1)
                {
                    printf("ERROR: track:%02d, sector:%02d == NULL\n", t, s);
                    printf("\t Program error - should never happen\n");
                    printf("\tGiving up!\n");
                    liftel.error = 1;
                    liftel.state = TD0_DONE;
                    return(0);
                }
                ++sectors;
            }
        }
    }

    if(!sectors)
    {
        printf("LIF - no sectors left after processing\n");
        return(0);
    }

    sectors = 0;
    for(t=0; t<MAXTRACKS; ++t)
    {
        if(liftel.Sectors == disk->track[t].Sectors)
        {
            for (s = disk->track[t].First; s <= disk->track[t].Last; s++ )
            {
                ptr = disk->track[t].sectors[s].data;
                // PROCESS LIF SECTORS
                if( !td0_save_lif_sector(disk, ptr, liftel.Size, LIF) )
                {
                    printf("LIF sectors processed: %ld\n", sectors);
                    return (0);
                }
                ++sectors;
            }
        }
    }

    printf("LIF sectors processed: %ld\n", sectors);
    return(1);
}

 

/// @brief Process all sectors on a track from TeleDisk image
/// @param[in] data: sector data
/// @param[in] size: sector size
/// @param[in] LIF: LIF structure
/// @retrun 1 if OK, 0 on error
int td0_save_lif_sector(disk_t *disk, uint8_t *data, int size, lif_t *LIF)
{
    int i;
    int dir;
    int tmp;
    int count = 0;
 
 
    if(liftel.error)
    {
        printf("Error: exit\n");
        liftel.state = TD0_DONE;
        return(0);
    }
 
    if(liftel.state == TD0_DONE)
        return(1);
 
 
    // =======================================
    switch(liftel.state)
    {
 
        case TD0_START:
             // hexdump(data,size);
            lif_str2vol(data, LIF);
            LIF->filestart = LIF->VOL.DirStartSector + LIF->VOL.DirSectors;
            LIF->sectors = LIF->filestart;
 
            if(lif_check_volume(LIF) == 0)
            {
                printf("LIF: [%s] position:%ld\n",
                    LIF->name,(long)liftel.writeindex);
                printf("\t Error: Not a LIF image!\n");
 
                hexdump(data, size);
                lif_dump_vol(LIF,"debug");
 
                liftel.error = 1;
                liftel.state = TD0_DONE;
                return(0);
            }
 
            if(debuglevel & 0x400)
            {
                printf("LIF VOL Label:     [%10s]\n", LIF->VOL.Label);
                printf("LIF VOL Date:      %s\n", lif_lifbcd2timestr(LIF->VOL.date));
                printf("LIF DIR start:     [%04lXh]\n", (long)LIF->VOL.DirStartSector);
                printf("LIF DIR sectors:   [%04lXh]\n", (long)LIF->VOL.DirSectors);
                printf("LIF file start:    [%04lXh]\n", (long)LIF->filestart);
            }
 
            liftel.sectorindex = 0;
            liftel.writeindex = 0;
 
            liftel.state = TD0_WAIT_DIRECTORY;
            break;
 
 
        // Wait for director sectors
        case TD0_WAIT_DIRECTORY:
            if( liftel.sectorindex < LIF->VOL.DirStartSector )
                break;
 
            // Fall through
            liftel.state = TD0_DIRECTORY;
 
        case TD0_DIRECTORY:
            if( liftel.sectorindex < LIF->filestart)
            {
                // Process Directory entries in this sector
                for(dir=0; dir < size; dir+=32)
                {
                    lif_str2dir(data+dir, LIF);
        
                    // Directory EOF
                    if(LIF->DIR.FileType == 0xffff)
                    {
                        if(debuglevel & 0x400)
                        {
                            printf("LIF file sectors:  [%04lXh]\n", (long) LIF->filesectors);
                            printf("LIF image sectors: [%04lXh]\n", (long) LIF->sectors);
                        }
                        liftel.state = TD0_WAIT_FILE;
                        break;
                    }
        
                    // The offical LIF specification says we must not 
                    // trust file size or start if purged!
                    if(LIF->DIR.FileType == 0)
                        continue;
        
                    // Adjust total sectors and used sectors
                    if( (LIF->DIR.FileStartSector+LIF->DIR.FileSectors) 
                            > LIF->sectors )
                    {
                        LIF->sectors = (LIF->DIR.FileStartSector + LIF->DIR.FileSectors);
                    }
                    else
                    {
                        printf("LIF: [%s] position:%ld\n",
                            LIF->name,(long)liftel.writeindex);
                        printf("\t Warning: directory entry is out of order\n");
                        printf("\t Treating as DirectoryEOF\n");
                        hexdump(data, size);
        
                        // Update sector data
                        LIF->DIR.FileType = 0xffff;
                        lif_dir2str(LIF,data+dir);
        
                        liftel.state = TD0_WAIT_FILE;
                        break;
                    }
        
                    LIF->filesectors = LIF->sectors - LIF->filestart;
                    LIF->usedsectors = LIF->filesectors;
        
                    if(!lif_check_dir(LIF))
                    {
                        printf("LIF: [%s] position:%ld\n",
                            LIF->name,(long)liftel.writeindex);
                        printf("\t Warning: bad director entry\n");
                        printf("\t Treating as DirectoryEOF\n");
                        hexdump(data, size);
        
                        // Update sector data
                        LIF->DIR.FileType = 0xffff;
                        lif_dir2str(LIF,data+dir);
        
                        liftel.state = TD0_WAIT_FILE;
                        break;
                    }
        
                    if(debuglevel & 0x400)
                    {
                        printf("LIF: [%s] start:%6lXh size:%6lXh %s\n", 
                            LIF->DIR.filename, 
                            (long)LIF->DIR.FileStartSector, 
                            (long)LIF->DIR.FileSectors,
                            lif_lifbcd2timestr(LIF->DIR.date) );
                    }
        
                }   // for(dir=0; dir < size; dir+=32)
                break;
            }   
 
            // Fall through
            liftel.state = TD0_WAIT_FILE;
 
        case TD0_WAIT_FILE:
 //hexdump(data,size);
            if( liftel.sectorindex < LIF->filestart)
                break;
 
            // Fall through
            LIF->filesectors = LIF->sectors - LIF->filestart;
            liftel.state = TD0_FILE;
 
        case TD0_FILE:
 //hexdump(data,size);
            if( liftel.sectorindex < LIF->sectors)
                break;
 
            // Fall through
            liftel.state = TD0_DONE;
 
        case TD0_DONE:
            return(1);
    }   // switch(liftel.state)
 
    /// ====================================================
    /// @brief Write sectors from Teledisk track to LIF file
    ///        We continue until the last last file sector 
    /// ====================================================
 
    tmp = fwrite(data, 1, size, LIF->fp);
    if(size != tmp)
    {
        printf("LIF: [%s] position:%ld\n",
            LIF->name,(long)liftel.writeindex);
        printf("\t Write error\n");
 
        liftel.state = TD0_DONE;
        liftel.error = 1;
        return(0);
    }
 
    liftel.sectorindex++;
    liftel.writeindex++;
    return(1);
}


void td0_help(int full)
{
    printf("td02lif help\n");
    if(full)
    {
        printf( 
            "Usage: td02lif [options] file.td0 file.lif\n"
            "       td02lif help\n"
            "tdo2lif options:\n"
            "Notes: for any option that is NOT specified it is automattically detected\n"
            "\t -s256|512 | -s 256|512 - force sector size\n"
            "\t -h1|2 | -h 1|2 - force heads/serfaces\n"
            "\t -tNN | -t NN  - force tracks\n"
            "\n"
        );
    }
}

/// @brief TeleDisk image Analisis structure
/// Find attributes of LIF image stored in TeleDisk image
void td0_init_liftel()
{
    int i;
    // State Machine
    liftel.error = 0;
    liftel.state = TD0_INIT;
    liftel.sectorindex = 0;
    liftel.writeindex = 0;

    // Initialize liftel data
    liftel.Size = 0;
    liftel.Sides = 0;
    liftel.Sectors = 0;
    liftel.Tracks = 0;
    liftel.Cylinders = 0;

    for(i=0;i<2;++i)
    {
        liftel.s.first[i] = 0;
        liftel.s.size[i] = 0;
        liftel.s.last[i] = 0;
        liftel.s.sectors[i] = 0;
    }

    // User overrride
    liftel.u.size = -1;
    liftel.u.sides = -1;
    liftel.u.tracks = -1;
}

/// @brief Initialize track sector information
/// Optionally decompress the data if decompression is enabled
/// @param[out] p: data buffer for read
/// @param[out] osize: size of object
/// @param[out] size: number of objects to read
/// @return size of dataactually  read
void td0_init_sectors(disk_t *disk)
{
    int t,s;

    disk->fi = NULL;
    disk->td0_name = NULL;
    disk->compressed = 0;
    disk->t = 0;

    memset((td_header_t *) & disk->td_header, 0, sizeof(td_header_t));
    memset((td_comment_t *) & disk->td_comment,0,sizeof(td_comment_t));


    for(t = 0; t<MAXTRACKS; ++t)
    {
        for(s=0;s<MAXSECTORS;++s)
        {
            disk->track[t].Cyl = 0;
            disk->track[t].Side = 0;
            disk->track[t].Sectors = 0;
            disk->track[t].First = MAXSECTORS-1;
            disk->track[t].Last = 0;

            disk->track[t].sectors[s].cylinder = 0;
            disk->track[t].sectors[s].side = 0;
            disk->track[t].sectors[s].sector = -1;
            disk->track[t].sectors[s].size = 0;
            disk->track[t].sectors[s].data = NULL;
        }
    }
}



/// @brief Convert a Teledisk LIF formatted disk image into a pure LIF image
/// @param[in] telediskname: TELEDISK image name
/// @param[in] lifname: LIF file name to write
/// @return 1 on success, 0 on error
int td02lif(int argc, char *argv[])
{
    FILE *fo;
    int status;
    int tmp;
    time_t t;
    char *ptr;
    char *telediskname = NULL;
    char *lifname = 0;
    int i;

    // Analisis Data
    td0_init_liftel();

    // Sectir data
    td0_init_sectors( (disk_t *) &disk  );

    ptr = argv[0];
    if(!ptr)
        return(0);

    if(argc <= 1)
    {
        td0_help(1);
        return(1);
    }
    
    for(i=1;i<argc;++i)
    {
        ptr = argv[i];
        if(!ptr)
            break;

        if(!*ptr)
            continue;


        if(*ptr == '-')
        {
            ++ptr;
            // Sector size
            if(*ptr == 's')
            {
                ++ptr;
                if(*ptr || (ptr = argv[++i]) )
                {
                    tmp = atoi(ptr);
                    if(tmp != 256 && tmp != 512)
                    {
                        printf("ERROR: size:[%d] != 256 or 512\n", tmp);
                        td0_help(1);
                        return(1);
                    }
                    liftel.u.size = tmp;
                }
                continue;
            }

            // Heads
            if(*ptr == 'h')
            {
                ++ptr;
                if(*ptr || (ptr = argv[++i]) )
                {
                    tmp = atoi(ptr);
                    if(tmp != 1 && tmp != 2)
                    {
                        printf("ERROR: sides:[%d] != 1 or 2\n", tmp);
                        td0_help(1);
                        return(1);

                    }
                    liftel.u.sides = tmp;
                }
                continue;
            }

            // Tracks
            if(*ptr == 't')
            {
                ++ptr;
                if(*ptr || (ptr = argv[++i]) )
                {
                    tmp = atoi(ptr);
                    if(tmp >= MAXTRACKS)
                    {
                        printf("ERROR: tracks:[%d] >= %d\n", tmp, MAXTRACKS);
                        td0_help(1);
                        return(1);

                    }
                    liftel.u.tracks = atoi(ptr);
                }
                continue;
            }

            if(*ptr == '?' || MATCH(ptr,"help") )
            {
                td0_help(1);
                return(0);
                continue;
            }
            printf("ERROR: bad options:[%s]\n", ptr);
            td0_help(1);
            return(0);
            continue;
        }
        else if(telediskname == NULL)
        {
            telediskname = ptr;
        }
        else if(lifname == NULL)
        {
            lifname = ptr;
        }
        else if(MATCH(ptr,"help") )
        {
            td0_help(1);
            return(1);
        }
        else
        {
            printf("ERROR: bad options:[%s]\n", ptr);
            return(0);
        }
    }

    if( liftel.u.size != -1)
        printf("\tUser Override: sector size = %d\n", liftel.u.size);

    if( liftel.u.sides != -1)
        printf("\tUser Override: sides = %d\n", liftel.u.sides);

    if( liftel.u.tracks != -1)
        printf("\tUser Override: tracks = %d\n", liftel.u.tracks);

    if(!lifname|| !strlen(lifname))
    {
        printf("Expected LIF filename\n");
        return(0);
    }

    if(!telediskname|| !strlen(telediskname))
    {
        printf("Expected TeleDisk filename\n");
        return(0);
    }

    if(strcasecmp(telediskname,lifname) == 0)
    {
        printf("TeleDIsk and LIF names can not be the same\n");
        return(0);
    }

    lif_t *LIF = lif_calloc(sizeof(lif_t)+4);
    if(LIF == NULL)
        return(0);

    lif_image_clear(LIF);

    // LIF file name
    LIF->name = lif_stralloc(lifname);
    if(LIF->name == NULL)
    {
        lif_close_volume(LIF);
        return(0);
    }
    ///@brief Write LIF file
    LIF->fp=fopen(LIF->name,"wb");
    if(LIF->fp==NULL)
    {
        printf("TeleDisk Can't open: %s\n",LIF->name);
        lif_close_volume(LIF);
        return (0);
    }

    if( !td0_open((disk_t *) &disk, telediskname) )
    {
        if(disk.fi)
            fclose(disk.fi);
        lif_close_volume(LIF);
        return(0);
    }

    if( !td0_read_disk( (disk_t *) &disk ) )
    {
        if(disk.fi)
            fclose(disk.fi);
        lif_close_volume(LIF);
        return(0);
    }

    if( !td0_analize_format((disk_t *) &disk) )
    {
        if(disk.fi)
            fclose(disk.fi);
        lif_close_volume(LIF);
        return(0);
    }

    if( !td0_save_lif((disk_t *) &disk, LIF) )
    {
        if(disk.fi)
            fclose(disk.fi);
        lif_close_volume(LIF);
        return(0);
    }

    ///@brief  LIF summary
    printf("\n");
    printf("Done LIF image: [%s] wrote: [%04lXh] sectors\n\n", 
            LIF->name, (long)liftel.writeindex);

    ///@brief  Close LIF file
    lif_close_volume(LIF);

    lif_dir(lifname);
    return(1);
}
