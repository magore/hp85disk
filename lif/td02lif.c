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

///@brief Dave Dunfiled LZSS expander
#include "td0_lzss.h"

///@brief Sectors decoded in this current track
track_data_t trackdata[MAXSECTOR];

///@brief Teledisk to LIF state
liftel_t liftel;

///@brief Teledisk LZSS decompression flag
static int decompress = 0;

///@brief Extract TeleDisk image header data in architecture nutral way
///@param[in] B: source data
///@param[out] p: TeleDisk image header structure
///@return TeleDisk image header size (not structure size)
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

///@brief Extract TeleDisk comment header data in architecture nutral way
///@param[in] B: source data
///@param[out] p: TeleDisk comment header structure
///@return TeleDisk comment header size (not structure size)
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

///@brief Extract TeleDisk track header data in architecture nutral way
///@param[in] B: source data
///@param[out] p: TeleDisk track header structure
///@return TeleDisk track header size (not structure size)
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
        if(p->PSectors != 255)   // EOF ?
            printf("TeleDisk error Track CRC16:%04Xh != %04Xh\n", (int)crc, (int)p->CRC);
        printf("Cyl:%02d, Side:%02d, Sectors:%d\n",
            (int) p->PCyl, (int) p->PSide, (int) p->PSectors);
    }
    return(crc);
}

///@brief Extract TeleDisk sector header data in architecture nutral way
///@param[in] B: source data
///@param[out] p: TeleDisk sector header structure
///@return TeleDisk sector header size (not structure size)
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

///@brief Enable TeleDisk image decompression
///@param[in] flag: decompression enable
///@return void
void td0_enable_decompress(int flag)
{
    decompress = flag;
    if(flag)
        init_decompress();
}

///@brief Read TeleDisk image data block
/// Optionally decompress the data if decompression is enabled
///@param[out] p: data buffer for read
///@param[out] osize: size of object
///@param[out] size: number of objects to read
///@return size of dataactually  read
long td0_read(void *p, int osize, int size, FILE *fp)
{
    int c;
    long count;
    long ind = 0;

    if(decompress)
    {
        uint8_t *ptr = p;
        count = osize * size;
        while(count--)
        {
            c = lzss_getbyte(fp);
            if(c == EOF)
                return(ind);
            *ptr++ = (c & 0xff);
            ++ind;
        }
    }
    else
    {
        ind = fread(p,osize,size,fp);
    }
    return(ind);
}

///@brief Initialize track sector information
/// Optionally decompress the data if decompression is enabled
///@param[out] p: data buffer for read
///@param[out] osize: size of object
///@param[out] size: number of objects to read
///@return size of dataactually  read
void td0_init_trackdata(int freemem)
{
    int i;
    for(i=0;i<MAXSECTOR;++i)
    {
        trackdata[i].cylinder = 0;
        trackdata[i].side = 0;
        trackdata[i].sector = -1;
        trackdata[i].sectorsize = 0;
        trackdata[i].bitrate = 0;
        trackdata[i].encoding = 0;
        if(freemem && trackdata[i].data != NULL);
        {   
            free( trackdata[i].data );
        }
        trackdata[i].data = NULL;
    }
}

///@brief Expand a Run Length encoded TeleDisk sector
/// Notes: the source length is encoded in the source data
/// Credits based on work (C) 2006-2014 Jean-François DEL NERO
///    Part of the HxCFloppyEmulator library GNU GPL version 2 or later
/// Rewitten for clarity,to enforce size limits, provide error reporting
/// and to avoid word order dependent assumptions,
///@param[out] dst: destination expanded data
///@param[in] src: source data
///@param[in] max: maximum size of destination data
///@return size of expanded data, negative if size > max
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

///@brief Display TeleDisk track sector values
///@param[in] trackdata: track sector structure
///@param[in] index: sector index (not sector number)
///@return void
void td0_track(int index)
{
    printf("cylinder:%02d, head:%02d, sector:%02d, size:%d, index:%d\n", 
        (int) trackdata[index].cylinder,
        (int) trackdata[index].side,
        (int) trackdata[index].sector,
        (int) trackdata[index].sectorsize,
        (int) index);

}
///@brief Display TeleDisk sector data
///@param[in] P: td_sector pointer
///@return void
void td0_sector(td_sector_t *P)
{
    printf("cyl: %02d, side: %02d, sector: %02d, size: %03d\n",
        (int) P->Cyl,
        (int) P->Side,
        (int) P->Sector,
        (int) 128 << P->SizeExp);
}


///@brief Convert Denisity to bitrate
///@We don't care about the density except as a possible filter
///@param[in] Density
///@return bitrate
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

///@brief Read TeleDisk image file and call LIF converter
///@param[in] imagefile: TD0 image file
///@param[in] data: LIF data structure pointer
///@return 1 on success 0 on error
int td0_read_image(char * imgfile, lif_t *LIF)
{
	FILE * fi;
	int i,j;
    int cyl,head,sector,size;
    int index, sectors,found, result;
    int status;
    long count = 0;  
	td_header_t td_header;
	td_comment_t td_comment;
	td_track_t td_track;
	td_sector_t  td_sector;
    tm_t tm;

	uint8_t buffer[8*1024];


    if(!imgfile || !strlen(imgfile))
    {
        printf("Error expected TeleDisk filename\n");
        liftel.error = 1;
        liftel.state = TD0_DONE;
        return(0);
    }

    fi = fopen(imgfile,"rb");
    if(!fi)
    {
        printf("Error Can't open TeleDisk file: %s\n",imgfile);
        liftel.error = 1;
        liftel.state = TD0_DONE;
		return (0);
	}


    ///@process TeleDisk disk image header
    if( fread( &buffer, 1, TD_HEADER_SIZE, fi ) != TD_HEADER_SIZE)
    {
        printf("Error Can't read TeleDisk header: %s\n", imgfile);
        fclose(fi);
        liftel.error = 1;
        liftel.state = TD0_DONE;
		return (0);

    }
    td0_unpack_disk_header(buffer, (td_header_t *)&td_header);

    printf("TeleDisk image file: %s\n",imgfile);
    ///@brief we expect "TD" (uncompressed) or "td" (compressed) format
	if(MATCH(td_header.Header,"TD"))
	{
		printf("\tNormal compression\n");
        td0_enable_decompress(0);
	}

	else if(MATCH(td_header.Header,"td"))
	{
		printf("\tAdvanced compression\n");
        td0_enable_decompress(1);
	}
	else
	{
		printf("Error bad TeleDisk header: %s\n", td_header.Header);
        printf("\t file position:%ld\n", (long)ftell(LIF->fp));
        fclose(fi);
        liftel.error = 1;
        liftel.state = TD0_DONE;
		return (0);
	}

    ///@brief We only accept TeleDisk versions >= 10 && version <= 21
	printf("\tVersion: %d\n",td_header.TDVersion);

	if((td_header.TDVersion>21) || (td_header.TDVersion<10))
	{
		printf("Error only TeleDisk versions 10 to 21 supported\n");
        printf("\t file position:%ld\n", (long)ftell(LIF->fp));
        fclose(fi);
        liftel.error = 1;
        liftel.state = TD0_DONE;
		return (0);
	}


    ///@brief Do we have a Comment Block ?
    /// bit 7 of TrackDensity is set 
	if(td_header.TrackDensity & 0x80)
	{

        uint16_t crc;
        char timebuf[32];
		if( td0_read( buffer, 1, sizeof(td_comment), fi ) != sizeof(td_comment) )
        {
            printf("Error reading commment\n");
            printf("\t file position:%ld\n", (long)ftell(LIF->fp));
            fclose(fi);
            return (0);
        }

        crc = td0_unpack_comment_header(buffer, (td_comment_t *)&td_comment);

		if( td0_read( buffer, 1, td_comment.Size,fi) != td_comment.Size)
        {
            printf("Error reading commment\n");
            printf("\t file position:%ld\n", (long)ftell(LIF->fp));
            fclose(fi);
            liftel.error = 1;
            liftel.state = TD0_DONE;
            return (0);
        }


        buffer[td_comment.Size] = 0;


        crc = crc16(buffer,crc,0xA097,td_comment.Size);
        if(td_comment.CRC != crc)
        {
            printf("Warning Comment CRC16:%04Xh != %04Xh\n", (int)crc, (int)td_comment.CRC);
            printf("\t file position:%ld\n", (long)ftell(LIF->fp));
        }

// FIXME convert to LIF date

        ///@brief Time to Unix format
        tm.tm_year = (int)td_comment.Year;
        tm.tm_mon = (int)td_comment.Month;
        tm.tm_mday = (int)td_comment.Day;
        tm.tm_hour = (int)td_comment.Hour;
        tm.tm_min = (int)td_comment.Minute;
        tm.tm_sec = (int)td_comment.Second;

        // DATE
        printf("\tDate: %s", asctime_r((tm_t *) &tm,timebuf));

        liftel.t = timegm((tm_t *) &tm);

        // COMMENT
        printf("%s\n\n", buffer);
	}

    td0_init_trackdata(0);

    ///@brief Process all image Data
	while(1)
	{
        // PROCESS TRACK


        // TRACK HEADER
        if( td0_read(buffer,1, TD_TRACK_SIZE, fi) != TD_TRACK_SIZE)
        {
            printf("Error reading track header\n");
            printf("\t file position:%ld\n", (long)ftell(LIF->fp));
            fclose(fi);
            liftel.error = 1;
            liftel.state = TD0_DONE;
            return (0);
        }
        td0_unpack_track_header(buffer, (td_track_t *)&td_track);

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
		for ( i=0;i < td_track.PSectors;i++ )
		{
            ///@brief Sector Header
            if( td0_read(buffer,1, TD_SECTOR_SIZE,fi) != TD_SECTOR_SIZE )
            {
                printf("Error reading sector header\n");
                printf("\t Track: Cyl: %02d, Side: %02d, Sectors: %02d\n",
                    (int)td_track.PCyl, (int)td_track.PSide, (int)td_track.PSectors);
                fclose(fi);
                liftel.error = 1;
                liftel.state = TD0_DONE;
                return (0);
            }

            td0_unpack_sector_header(buffer, (td_sector_t *)&td_sector);


            //FIXME add flag override for this test
            if(td_track.PCyl != td_sector.Cyl )
            {
                printf("Warning track Cyl:%d != sector Cyl:%d skipping\n",
                    (int)td_track.PCyl, (int) td_sector.Cyl);
                printf("\t");
                td0_sector((td_sector_t *) &td_sector);
                continue;
            }

            //FIXME add flag override for this test
            if(td_track.PSide != td_sector.Side)
            {
                printf("Warning track side:%d != sector side:%d skipping\n",
                    (int)td_track.PSide, (int) td_sector.Side);
                printf("\t");
                td0_sector((td_sector_t *) &td_sector);
                continue;
            }

            // We insert sectors using their dector ID as the index offset
            // This "sorts" them in order
			index = td_sector.Sector;

            //FIXME we can not deal with duplicate sectors so skip them
            if( trackdata[index].sector != -1)
            {
                printf("Warning duplicate sector: %02d,  skipping\n", index);
                printf("\t");
                td0_sector((td_sector_t *) &td_sector);
                continue;
            }

			trackdata[index].cylinder = td_sector.Cyl;
			trackdata[index].side = td_sector.Side;
			trackdata[index].sector = index;
			trackdata[index].sectorsize = 128<<td_sector.SizeExp;
			trackdata[index].encoding = td_track.PSide & 0x80;
            trackdata[index].bitrate = td0_density2bitrate(td_header.Density);
			trackdata[index].data = calloc(trackdata[index].sectorsize,1);

			if(td_sector.Flags & 0x02)
			{
                printf("Warning alternate CRC flag not implemented\n");
                printf("\t");
                td0_track(index);
			}
			if(td_sector.Flags & 0x04)
			{
                printf("Warning alternate data mark not implemented\n");
                printf("\t");
                td0_track(index);
			}
			if(td_sector.Flags & 0x20)
			{
                printf("Warning missing address mark not implemented\n");
                printf("\t");
                td0_track(index);
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
                if( td0_read(buffer,1, sizeof(uint16_t),fi) != sizeof(uint16_t) )
                {
                    printf("Error reading sector header\n");
                    printf("\t");
                    td0_track(index);
                    liftel.error = 1;
                    liftel.state = TD0_DONE;
                    fclose(fi);
                    return(0);
                }
                len = B2V_LSB(buffer,0,2);

                if( td0_read(buffer+2,1, len,fi) != len )
                {
                    printf("Error reading sector data\n");
                    printf("\t");
                    td0_track(index);
                    liftel.error = 1;
                    liftel.state = TD0_DONE;
                    fclose(fi);
                    return (0);
                }

                ///@brief td0_rle does not use len
                /// internal data in the buffer does this
                /// We limit the expansion to sector size
				result = td0_rle(trackdata[index].data, buffer, trackdata[index].sectorsize);

                if(result != trackdata[index].sectorsize)
                {
                    // Very unlikey we can recover from this!
                    printf("Error RLE result:%d != sector size:%d\n",
                        (int) result , (int) trackdata[index].sectorsize);
                    printf("\t");
                    td0_track(index);
                    liftel.error = 1;
                    liftel.state = TD0_DONE;
                    fclose(fi);
                    return (0);
                }

                crc = crc16(trackdata[index].data,0,0xA097,result);
                crc &= 0xff;
                if(td_sector.CRC != crc)
                {
                    printf("Warning Sector CRC16:%04Xh != %04Xh\n", 
                        (int)crc, (int)td_sector.CRC);
                    printf("\t");
                    td0_track(index);
                }

			}  // if ( !(td_sector.SizeExp & 0xf8) && !(td_sector.Flags & 0x30))
            else
            {
                printf("Warning unsupported sector flags:%02Xh\n", (int)td_sector.Flags);
                printf("\t");
                td0_track(index);
            }

        }   // for ( i=0;i < td_track.PSectors;i++ )

        ///@brief Done processing track sectors

        // =========================================
        // FYI we have already eliminated sectors that are duplicates, 
        // or that do not match the track header cylinder, head values
        // =========================================

        // =========================================
        // Find attributes of first data sector in image
        if(liftel.state == TD0_INIT)
        {
            // Find lowest numbered sector and its size
            // FIXME add flag override for first and last sector

            liftel.sectorfirst = -1;
            liftel.sectorlast = 0;

            // Determine sectors per track from first track and side
            // FIXME add flag override for sectorspertrack
            liftel.sectorspertrack = 0;
            for ( i=0;i < MAXSECTOR;i++ )
            {
                if( trackdata[i].sector != -1)
                {
                    // Save size and sector number
                    if( liftel.sectorfirst == -1)
                    {
                        ///@brief update first sector
                        ///FYI sector trackdata[i].sector == i
                        liftel.sectorfirst = i;
                        ///@vrief update size
                        liftel.sectorsize = trackdata[i].sectorsize;
                    }
                }
            }

            ///@brief Count all sectors that match first sector
            for ( i=0; i < MAXSECTOR; i++ )
            {
                if( trackdata[i].sector == -1)
                    continue;

                if( trackdata[i].sectorsize != liftel.sectorsize )
                    continue;

                liftel.sectorspertrack++;
                liftel.sectorlast = trackdata[i].sector;
            }
            liftel.state = TD0_START;

            if(debuglevel & 0x400)
                printf("TeleDisk sectors per track:%02d, First:%02d, Last:%02d\n",
                    liftel.sectorspertrack, liftel.sectorfirst,liftel.sectorlast);

            if(!liftel.sectorspertrack)
            {

                printf("TeleDisk 0 sectors on first track - aborting\n");
                liftel.sectorlast = trackdata[i].sector;
                liftel.error = 1;
                liftel.state = TD0_DONE;
                fclose(fi);
                return(0);
            }

        }       // if(liftel.state == TD0_INIT)
        // Done processing first sector
        /// =========================================


        // Delete / Free ALL sectors with size mismatch
        // This simplifies the tests later on
        sectors = 0;
        for (i = 0; i < MAXSECTOR; i++ )
        {
            if( trackdata[i].sector != -1)
            {
                if(trackdata[i].sectorsize != liftel.sectorsize)
                {
                    trackdata[i].cylinder = 0;
                    trackdata[i].side = 0;
                    trackdata[i].sector = -1;
                    trackdata[i].sectorsize = 0;
                    trackdata[i].encoding = 0;
                    trackdata[i].bitrate = 0;

                    if( trackdata[i].data != NULL)
                    {
                        free(trackdata[i].data);
                        trackdata[i].data = NULL;
                    }
                }
                else
                {
                    ++sectors;
                }
            }
        }


        // Skip tracks with no sectors
        if(!sectors)
        {
            if(debuglevel & 0x400)
                printf("EMPTY TRACK SKIPPING\n");
            // FREE sectors
            td0_init_trackdata(1);
            continue;
        }

        if(debuglevel & 0x400)
            printf("PROCESSING %02d SECTORS\n", sectors);

        /// =========================================
        // PROCESS SECTORS
        sectors = 0;
        for (i = liftel.sectorfirst; i <= liftel.sectorlast; i++ )
        {
            if( trackdata[i].sector == -1)
            {
                // See if we have an alternate
                for(j=100;j<MAXSECTOR;++j)
                {
                    // Size MUST match if it does

                    if(trackdata[j].sector == -1)
                        continue;

                    // Find alternate sector if we have one
    
                    // Map this on to the missing one
                    trackdata[i].cylinder = trackdata[j].cylinder;
                    trackdata[i].side = trackdata[j].side;
                    trackdata[i].sector = i;
                    trackdata[i].sectorsize = trackdata[j].sectorsize;
                    trackdata[i].bitrate = trackdata[j].bitrate;
                    trackdata[i].encoding = trackdata[j].encoding;
                    trackdata[i].data = trackdata[j].data;
    
                    // Make sure we do not reuse this sector!!!
                    trackdata[j].cylinder = 0;
                    trackdata[j].side = 0;
                    trackdata[j].sector = -1;
                    trackdata[j].sectorsize = 0;
                    trackdata[j].bitrate = 0;
                    trackdata[j].encoding = 0;
                    trackdata[j].data = NULL;

                    printf("Warning Sector:%02d missing - found alternate sector:%02d\n", i, j);
                    printf("\tLocation: ");
                    td0_track(i);
    
                    break;
                }   // for(j=100;j<MAXSECTOR;++j)

            }   // if( trackdata[i].sector == -1)

            // If we did not map an alternate then assign a blank dummy
            if( trackdata[i].sector == -1)
            {
                trackdata[i].data = calloc(liftel.sectorsize,1);
                // mark sector as in use
                trackdata[i].sectorsize = liftel.sectorsize;
                trackdata[i].sector = i;

                printf("Warning Sector:%02d missing - zero filling\n", i);
                printf("\tLocation: ");
                printf("Track: Cyl: %02d, Side: %02d\n",
                    (int)td_track.PCyl, (int)td_track.PSide);
            }

            // PROCESS LIF SECTORS
            status = td02lif_sector(trackdata[i].data, liftel.sectorsize, LIF);
            if(!status)
            {
                td0_init_trackdata(1);
                fclose(fi);
                return (status);
            }
        }

        // FREE sectors
        td0_init_trackdata(1);

	}  // while(1)

    td0_init_trackdata(1);

    fclose(fi);
	return (1);
}

/// @brief Process all sectors on a track from TeleDisk image
/// @param[in] data: sector data
/// @param[in] size: sector size
/// @param[in] LIF: LIF structure
/// @retrun 1 if OK, 0 on error
int td02lif_sector(uint8_t *data, int size, lif_t *LIF)
{
    int i;
    int dir;

    int count = 0;


    if(liftel.error)
    {
        printf("Error: exit\n");
        liftel.state = TD0_DONE;
        return(0);
    }
    if(liftel.state == TD0_DONE)
        return(0);


    // =======================================
    switch(liftel.state)
    {

        // Decode Volume Header 
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

    size=fwrite(data, 1, size, LIF->fp);
    if(size != liftel.sectorsize)
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


/// @brief Convert a Teledisk LIF formatted disk image into a pure LIF image
/// @param[in] telediskname: TELEDISK image name
/// @param[in] lifname: LIF file name to write
/// @return 1 on success, 0 on error
int lif_td02lif(char *telediskname, char *lifname)
{
    FILE *fo;
    int status;
    time_t t;

    if(!lifname|| !strlen(lifname))
    {
        printf("Expected LIF filename\n");
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

    liftel.error = 0;
    liftel.state = TD0_INIT;
    liftel.sectorspertrack = 0;
    liftel.sectorsize = -1;
    liftel.sectorlast = 0;
    liftel.sectorindex = 0;
    liftel.writeindex = 0;
    liftel.t = time(0);

    status = td0_read_image(telediskname, LIF);

    ///@brief  LIF summary
    printf("Done LIF image: [%s] wrote: [%ld] sectors\n\n", 
            LIF->name, (long)liftel.writeindex);
    ///@brief  Close LIF file
    lif_close_volume(LIF);

    lif_dir(lifname);
    return(1);
}
