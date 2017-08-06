
/**
 @file td02lif.c

 @brief  Teledisk LIF extractor wrapper for HxCFloppyEmulator project

 @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

 @Credits
     We use the external HxCFloppyEmulator library to decode TELEDISK format
     The HxCFloppyEmulator library is Copyright (C) 2006-2014 Jean-Fran¿ois DEL NERO
     See: https://github.com/jfdelnero/libhxcfe/tree/master/sources/loaders/teledisk_loader

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

   * LIF command help
     * lif td02lif image.td0 image.lif
       * Convert TeleDisk encoded LIF disk image into to pure LIF image
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//#include "types.h"
#include <inttypes.h>
#include "crc.h"

#include <time.h>
#include "lifsup.h"
#include "lifutils.h"
#include "td02lif.h"

#include "teledisk_loader.h"
#include "teledisk_format.h"
#include "td0_lzss.h"


///@brief Teledisk to LIF state
liftel_t liftel;


enum 
{
    TD0_INIT,
    TD0_VOLUME,
    TD0_DIRECTORY,
    TD0_FILE,
    TD0_DONE
};


int td02lif_sector_info(sector_config_t *sectorconfig, int index)
{
    printf("cylinder:%02d, head:%02d, sector:%02d, size:%d, index:%d\n", 
        (int) sectorconfig[index].cylinder,
        (int) sectorconfig[index].head,
        (int) sectorconfig[index].sector,
        (int) sectorconfig[index].sectorsize,
        (int) index);

#if 0
    printf("\t \ttrackencoding:%02d\n", (int) sectorconfig[index].trackencoding);
    printf("\t \tuse_alternate_datamark:%04x\n", (int) (int) sectorconfig[index].use_alternate_datamark);
    printf("\t \talternate_datamark:%04x\n", (int) sectorconfig[index].alternate_datamark);
    printf("\t \tuse_alternate_data_crc:%04x\n", (int) sectorconfig[index].use_alternate_data_crc);
    printf("\t \tmissingdataaddressmark:%04x\n", (int) sectorconfig[index].missingdataaddressmark);
#endif
}

/// @brief Process all sectors on a track from teledisk_loader.c
/// @param[in] *sectorconfig: TeleDisk track data structure
/// @param[in] track_sectors: sectors in this track
/// @retrun 1 if OK, 0 on error
int td02lif_track(sector_config_t *sectorconfig, int track_sectors, void *data)
{
    int i;
    int dir;


    int cyl;
    int head;
    int size;
    int sector;
    int count = 0;

    uint8_t *ptr;
    lif_t *LIF = data;

    int sectormap[MAXSECTOR+1];
    char liflabel[6+1];

    // Save cylinder and head for comparision or remaining track sectors
    cyl = sectorconfig[0].cylinder;
    head = sectorconfig[0].head;

    if(liftel.error)
    {
        printf("Error: exit\n");
        liftel.state = TD0_DONE;
        return(0);
    }
    if(liftel.state == TD0_DONE)
        return(0);


    if(track_sectors >= MAXSECTOR)
    {
        printf("Sectors Per Track Exceeded sector map table!\n");
        liftel.error = 1;
        liftel.state = TD0_DONE;
        return(0);
    }

    // Save sector size from first physical sector in image
    if(liftel.state == TD0_INIT)
        liftel.sectorsize = sectorconfig[0].sectorsize;

    /// ===========================================
    /// @brief CREATE SECTOR MAP
    /// Sort sectors by creating a sector mapping table
    /// ===========================================

    // CLEAR sector number mapping table for this track
    for(i=0;i<MAXSECTOR;++i)
        sectormap[i] = -1;


    // Count valid sectors in this track
    count = 0;
    for (i=0; i<track_sectors; i++ )
    {
        sector = sectorconfig[i].sector;
        size = sectorconfig[i].sectorsize;

        if(size != liftel.sectorsize)
        {
#if 0
            if(debuglevel & 0x400)
            {
                td02lif_sector_info(sectorconfig, i);
                printf("\t Warning: size: %d != %d\n", (int) size, (int) liftel.sectorsize);
            }
#endif
            continue;
        }

        if(cyl != sectorconfig[i].cylinder)
        {
            // fatal error
            td02lif_sector_info(sectorconfig, i);
            printf("\t Error: cylinder: %d != %d\n", (int) sectorconfig[i].cylinder, (int)cyl);
            liftel.error = 1;
            liftel.state = TD0_DONE;
            return(0);
        }

        if( head != sectorconfig[i].head )
        {
            // fatal error
            td02lif_sector_info(sectorconfig, i);
            printf("\t Error: head: %d != %d\n", (int) sectorconfig[i].head, (int)head);
            liftel.error = 1;
            liftel.state = TD0_DONE;
            return(0);
        }

        if(sector >= MAXSECTOR)
        {
            td02lif_sector_info(sectorconfig, i);
            printf("\t Error: sector:%02d >= %02d (MAXSECTOR)\n", (int) sector, (int) MAXSECTOR);
            liftel.error = 1;
            liftel.state = TD0_DONE;
            return(0);
        }

// HACK FIX for damaged 85-SS80.TD0 SS80 Diagnostics
#if 0
        if(cyl == 11 && head == 0 && sector == 116)
        {
            if(debuglevel & 0x400)
            {
                sector = 8;
                td02lif_sector_info(sectorconfig, i);
                printf("\t Warning: mapping sector 116 to 8\n");
            }
        }
        if(cyl == 13 && head == 0 && sector == 116)
        {
            if(debuglevel & 0x400)
            {
                sector = 11;
                td02lif_sector_info(sectorconfig, i);
                printf("\t Warning: mapping sector 116 to 11\n");
            }
        }
#endif

        if(sectormap[sector] != -1)
        {
            if(debuglevel & 0x400)
            {
                td02lif_sector_info(sectorconfig, i);
                printf("\t Warning: skipping: duplicate sector: %d\n", sector);
            }
            continue;
        }

#if 0
        if(sector >= track_sectors)
        {
            if(debuglevel & 0x400))
            {
                td02lif_sector_info(sectorconfig, i);
                printf("\t Warning: skipping: sector:%02d >= %02d (track_sectors)\n", (int) sector, (int) track_sectors);
            }
            continue;
        }
#endif

        if(liftel.state == TD0_INIT)
        {
            if(sector > liftel.sectorlast)
                liftel.sectorlast = sector;
            liftel.sectorspertrack++;
        }
#if 0
        else if(sector > liftel.sectorlast)
        {
            if(debuglevel & 0x400)
            {
                td02lif_sector_info(sectorconfig, i);
                printf("\t Warning: skipping: sector:%02d > %02d (liftel.sectorlast)\n", (int) sector, (int) liftel.sectorlast);
            }
            continue;
        }
#endif
        sectormap[sector] = i;
        ++count;

    }   // for (i=0; i<track_sectors; i++ )

    if(count && count != liftel.sectorspertrack)
    {
        if(debuglevel & 0x400)
        {
            printf("cylinder:%02d, head:%02d\n", (int)cyl, (int)head);
            printf("\t Warning: unexpected sector count:%d != %d\n", (int) count, liftel.sectorspertrack);
        }
    }


    // =======================================
    // Decode Volume Header 
    if(liftel.state == TD0_INIT && cyl == 0 && head == 0)
    {
        // Get sector 0
        sector = sectormap[0];
        if(sector == -1)
        {
            printf("Volume header not found!\n");
            liftel.error = 1;
            liftel.state = TD0_DONE;
            return(0);
        }

        ptr = sectorconfig[sector].input_data;
        lif_str2vol(ptr, LIF);

        LIF->filestart = LIF->VOL.DirStartSector + LIF->VOL.DirSectors;
        LIF->sectors = LIF->filestart;

        if(lif_check_volume(LIF) == 0)
        {
            td02lif_sector_info(sectorconfig, sector);
            printf("\t Error: Not a LIF image!\n");
            lif_dump_vol(LIF,"debug");
            hexdump(ptr, liftel.sectorsize);
            liftel.error = 1;
            liftel.state = TD0_DONE;
            return(0);
        }

        liftel.state = TD0_VOLUME;
        printf("LIF label:      [%s]\n", LIF->VOL.Label);
        printf("LIF Dir start:  [%04lXh]\n", (long)LIF->VOL.DirStartSector);
        printf("LIF Dir sectors:[%04lXh]\n", (long)LIF->VOL.DirSectors);
        printf("LIF File start: [%04lXh]\n", (long)LIF->filestart);
    }   // if(liftel.state == TD0_INIT && cyl == 0 && head == 0)

    ///@brief Display sector map summary
    if(debuglevel & 0x400)
    {
        count = 0;

        for(i=0;i<MAXSECTOR;++i)
        {
            if(sectormap[i] != -1)
                ++count;
        }
        if(count )
        {
            printf("Map: ");
            for(i=0;i<MAXSECTOR;++i)
            {
                if(sectormap[i] == -1)
                    continue;
                printf("%3d ", (int) sectormap[i] );
            }
        }
        else
        {
            printf("EMPTY\n");
            return(0);
        }
        printf("\n");
    }

    /// ===========================================
    /// @brief Process sectors 
    /// ===========================================
    for (i=0; i<MAXSECTOR; i++ )
    {

        sector = sectormap[i];
        if(sector == -1)
            continue;

        // program error
        if(sector >= track_sectors)
        {
            td02lif_sector_info(sectorconfig, 0);
            printf("\t Error: sector:%02d >= %02d (track_sectors)\n", (int) sector, (int) track_sectors);
            liftel.error = 1;
            liftel.state = TD0_DONE;
            return(0);
        }

        if(sector > liftel.sectorlast)
        {
            if(debuglevel & 0x400)
            {
                td02lif_sector_info(sectorconfig, sector);
                printf("\t Warning: sector:%02d > %02d (liftel.sectorlast)\n", (int) sector, (int) liftel.sectorlast);
            }
        }

        ptr = sectorconfig[sector].input_data;


        // Once the VOLUME has been processed we move on to Directory Sectors
        if(liftel.state == TD0_VOLUME)
        {
            // Only process directory sectors
            if( liftel.sectorindex >= LIF->VOL.DirStartSector && liftel.sectorindex < LIF->filestart )
            {
                // Process Directory entries
                for(dir=0;dir<liftel.sectorsize;dir+=32)
                {
                    lif_str2dir(ptr+dir, LIF);

                    // Directory EOF
                    if(LIF->DIR.FileType == 0xffff)
                    {
                        td02lif_sector_info(sectorconfig, sector);
                        printf("\t Directory EOF\n");
                        liftel.state = TD0_DIRECTORY;
                        break;
                    }

                    // Skip purged records - offical LIF specification says we must not use file size values
                    if(LIF->DIR.FileType == 0)
                        continue;

                    // Adjust total sectors and used sectors
                    if( (LIF->DIR.FileStartSector+LIF->DIR.FileSectors) > LIF->sectors )
                    {
                        LIF->sectors = (LIF->DIR.FileStartSector + LIF->DIR.FileSectors);
                    }
                    else
                    {
                        td02lif_sector_info(sectorconfig, sector);
                        printf("\t Warning: directory file position/size is out of order - assume last record\n");
                        liftel.state = TD0_DIRECTORY;
                        hexdump(ptr, liftel.sectorsize);
                        break;
                    }

                    LIF->filesectors = LIF->sectors - LIF->filestart;
                    LIF->usedsectors = LIF->filesectors;

                    if(!lif_check_dir(LIF))
                    {
                        td02lif_sector_info(sectorconfig, sector);
                        printf("\t Warning: directory entry out of order - assume last record\n");
                        liftel.state = TD0_DIRECTORY;
                        hexdump(ptr, liftel.sectorsize);
                        break;
                    }

                    if(debuglevel & 0x400)
                    {
                        printf("LIF name: %-10s start:%6lXh size:%6lXh %s\n", 
                            LIF->DIR.filename, 
                            (long)LIF->DIR.FileStartSector, 
                            (long)LIF->DIR.FileSectors,
                            lif_lifbcd2timestr(LIF->DIR.date) );
                    }

                }   // Processing directory entries

            }   // Processing directory sectors

        }   // for (i=0;i < liftel.sectorspertrack; i++ )

        liftel.sectorindex++;

        // Once we are done processing directory sectors wait for FILE area
        if( liftel.state != TD0_FILE && liftel.sectorindex >= LIF->filestart )
        {
            liftel.state = TD0_FILE;
            LIF->filesectors = LIF->sectors - LIF->filestart;
            printf("LIF image size in sectors:%lXh\n", (long) LIF->sectors);
            printf("LIF image file sectors:%lXh\n", (long) LIF->filesectors);
        }
    }   // Done processing sectors in this track


    /// ====================================================
    /// @brief Write sectors from Teledisk track to LIF file
    ///        We continue until the last last file sector 
    /// ====================================================
    count = 0;
    for (i=0; i<MAXSECTOR; i++ )
    {
        sector = sectormap[i];
        if(sector == -1)
            continue;

        // Done writting file
        if( liftel.writeindex >= LIF->sectors)
        {
            printf("LIF IMAGE Done\n");
            liftel.state = TD0_DONE;
            return(1);
        }

        // program error
        if(sector >= track_sectors)
        {
            printf("\t Error: sector:%02d >= %02d (track_sectors)\n", (int) sector, (int) track_sectors);
            td02lif_sector_info(sectorconfig, 0);
            liftel.error = 1;
            liftel.state = TD0_DONE;
            return(0);
        }

        // program error
        if(sector > liftel.sectorlast)
        {
            if(debuglevel & 0x400)
            {
                td02lif_sector_info(sectorconfig, sector);
                printf("\t Warning: sector:%02d > %02d (liftel.sectorlast)\n", (int) sector, (int) liftel.sectorlast);
            }
        }

        ptr = sectorconfig[sector].input_data;
        if(debuglevel & 0x400)
        {
            //td02lif_sector_info(sectorconfig, sector);
            //hexdump(ptr, liftel.sectorsize);
        }

        size=fwrite(ptr, 1, liftel.sectorsize, LIF->fp);
        if(size != liftel.sectorsize)
        {
            printf("LIF: %s write error\n", LIF->name);
            liftel.state = TD0_DONE;
            liftel.error = 1;
            return(0);
        }

        liftel.writeindex++;
        ++count;
    }   // for (i=0; i<MAXSECTOR; i++ )

    
    if(count < liftel.sectorspertrack)
    {
        memset(ptr,0, liftel.sectorsize);
        printf("\t Warning: adding blank sector at: %04lXh\n", liftel.writeindex);

        while(count < liftel.sectorspertrack)
        {
            size=fwrite(ptr, 1, liftel.sectorsize, LIF->fp);
            if(size != liftel.sectorsize)
            {
                printf("LIF: %s write error\n", LIF->name);
                liftel.state = TD0_DONE;
                liftel.error = 1;
                return(0);
            }
            ++count;
        }
    }

    // not done 
    return(0); 
}


/// @brief Convert a Teledisk LIF formatted disk image into a pure LIF image
/// @param[in] telediskname: TELEDISK image name
/// @param[in] lifname: LIF file name to write
/// @return 1 if renamed, 0 if not found, -1 error
int lif_td02lif(char *telediskname, char *lifname)
{
    floppy_t floppy;
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

    status = TeleDisk_libLoad_DiskFile((floppy_t *) &floppy, telediskname, LIF);

    ///@brief  LIF summary
    printf("LIF image:[%s] wrote:[%ld] sectors\n", LIF->name, (long)liftel.writeindex);
    printf("Done\n\n");

    ///@brief  Close LIF file
    lif_close_volume(LIF);

    lif_dir(lifname);
}
