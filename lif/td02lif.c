
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


/// @brief Process all sectors on a track from teledisk_loader.c
/// @param[in] *sectorconfig: TeleDisk track data structure
/// @param[in] track_sectors: sectors in this track
/// @retrun 1 if OK, 0 on error
int td02lif_track(sector_config_t *sectorconfig, int track_sectors, void *data)
{
    int i;
    uint8_t *ptr;
    long size;
    uint32_t System3000LIFid;
    uint16_t lifid;
    uint16_t zero1,zero2;
    lif_t *LIF = data;

    int labelstatus;
    int status;
    int cyl;
    int side;
    int sector;
    int count = 0;

    int sectormap[MAXSECTOR];
    char liflabel[6+1];

    if(liftel.done == 1)
        return(1);

    if(liftel.error)
        return(0);

    if(track_sectors >= MAXSECTOR)
    {
        printf("Sectors Per Track Exceeded sector map table!\n");
        liftel.error = 1;
        return(0);
    }

    // CLEAR sector number mapping table for this track
    for(i=0;i<MAXSECTOR;++i)
        sectormap[i] = -1;

    /// ===========================================
    /// @brief Process SECTOR 0 - LIF header
    /// ===========================================
    if(!liftel.sectorzeroprocessed)
    {
        // Process Sector 0 on TRack 0
        // Find lif data on sector 0, cylinder 0, head 0
        for (i=0;i < track_sectors;i++ )
        {

            if( sectorconfig[i].cylinder == 0 && sectorconfig[i].head == 0 && 
                sectorconfig[i].sector == 0)
            {
                ptr = sectorconfig[i].input_data;

                lif_str2vol(ptr, LIF);

                LIF->filestart = LIF->VOL.DirStartSector + LIF->VOL.DirSectors;
                LIF->sectors = LIF->filestart;

                if(!lif_check_volume(LIF))
                {
                    if(debuglevel & (0x400+1))
                    {
                        lif_dump_vol(LIF,"debug");
                        printf("Not a LIF image!\n");
                        hexdump(ptr, sectorconfig[i].sectorsize);
                    }
                    liftel.error = 1;
                    return(0);
                }

                liftel.sectorsize = sectorconfig[i].sectorsize;
                liftel.sectorindex = 0;
                liftel.sectorzeroprocessed = 1;

                break;
            }
        }


        // Find maximum sectors per track  number using side 0 cylinder 0
        for (i=0;i < track_sectors;i++ )
        {
            if( sectorconfig[i].cylinder == 0 && 
                sectorconfig[i].head == 0 && 
                sectorconfig[i].sectorsize == liftel.sectorsize )
                    ++liftel.sectorspertrack;
        }

        if(debuglevel & 0x400)
        {
            printf("LIF Dir start:[%ld]\n", (long)LIF->VOL.DirStartSector);
            printf("LIF Dir sectors:[%ld]\n", (long)LIF->VOL.DirSectors);
            printf("LIF sectors per track:[%ld]\n", (long)liftel.sectorspertrack);
            printf("LIF label:[%s]\n", LIF->VOL.Label);
        }
    }   

    if(!liftel.sectorzeroprocessed || liftel.sectorsize == -1)
    {
        printf("Not a LIF image!\n");
        liftel.error = 1;
        liftel.done = 1;
        return(0);
    }

    /// ===========================================
    ///@brief Done processing SECTOR 0 - LIF header
    /// ===========================================



    /// ===========================================
    /// @brief CREATE SECTOR MAP
    /// Sort sectors by creating a sector mapping table
    /// ===========================================
    for (i=0; i<track_sectors; i++ )
    {
        cyl = sectorconfig[i].cylinder;
        side = sectorconfig[i].head;
        size = sectorconfig[i].sectorsize;
        sector = sectorconfig[i].sector;

        if(size != liftel.sectorsize)
            continue;

#if 1
// HACK FIX for damaged 85-SS80.TD0 SS80 Diagnostics
        if(cyl == 11 && side == 0 && sector == 116)
            sector = 8;

        if(cyl == 13 && side == 0 && sector == 116)
            sector = 11;
#endif

        if(sector > track_sectors )
        {
            printf("skipping: track:%02d, side:%d, sector:%02d, sectorsize:%d\n",
                    (int)cyl,
                    (int)side,
                    (int)sector,
                    (int)size);

            continue;
        }

        if(sectormap[sector] != -1)
        {
            printf("duplicate sector: track:%02d, side:%d, sector:%02d, sectorsize:%d\n",
                    (int)cyl,
                    (int)side,
                    (int)sector,
                    (int)size);
            liftel.done = 1;
            liftel.error = 1;
            return(0);
        }
        else
        {
            sectormap[sector] = i;
        }
    }


    ///@brief sector map summary
    if(debuglevel & 0x400)
    {
        int i;
        int count = 0;
        printf("cyl:%2d,hd:%d map: ", 
            (int)sectorconfig[0].cylinder,
            (int)sectorconfig[0].head);
        for(i=0;i<MAXSECTOR;++i)
        {
            if(sectormap[i] != -1)
                ++count;
        }
        if(count )
        {
            for(i=0;i<MAXSECTOR;++i)
            {
                if(sectormap[i] == -1)
                    continue;
                printf("%2d ", (int) sectormap[i] );
            }
        }
        else
        {
            printf("EMPTY");
        }
        printf("\n");
    }
    /// ===========================================
    /// @brief Done creating SECTOR MAP
    /// ===========================================

    /// ===========================================
    /// @brief Process LIF directory sectors 
    /// We stop after the last valid directory entry
    /// ===========================================
    if(liftel.dirprocessed == 0)
    {
        // Find LAST sector in LIF image directory
        for (i=0;i < liftel.sectorspertrack; i++ )
        {
            int j;
            uint8_t *ptr;
            int sector = sectormap[i];

            if(liftel.sectorindex >= LIF->filestart)
                liftel.dirprocessed = 1;

            if(liftel.dirprocessed)
                break;

            if(sector == -1)
                continue;

            if(sector > liftel.sectorspertrack)
                continue;

            if( (liftel.sectorindex >= LIF->VOL.DirStartSector) 
                && (liftel.sectorindex < LIF->filestart) )
            {
                ptr = sectorconfig[sector].input_data;
                if(debuglevel & 0x400)
                {
                    printf(": cyl:%2d, side:%d, sector:%2d\n",
                        (int)sectorconfig[sector].cylinder,
                        (int)sectorconfig[sector].head,
                        (int)sectorconfig[sector].sector);
                    printf("Sector: %ld\n", liftel.sectorindex);
                    hexdump(ptr,256);
                }

                for(j=0;j<liftel.sectorsize;j += 32)
                {
                    time_t t;


                    lif_str2dir(ptr+j, LIF);

                    if(LIF->DIR.FileType == 0xffff)
                    {
                        liftel.dirprocessed = 1;
                        break;
                    }

                    if( (LIF->DIR.FileStartSector+LIF->DIR.FileSectors) 
                        > LIF->sectors )
                    {
                        LIF->sectors = 
                            (LIF->DIR.FileStartSector + LIF->DIR.FileSectors);
                        LIF->usedsectors = LIF->sectors;
                    }
                    else
                    {
                        printf("LIF image directory entries out of order - or invalid image\n");
                        lif_dump_vol(LIF,"debug");

                        liftel.done = 1;
                        liftel.error = 1;
                        return(0);
                    }

                    if(!lif_check_dir(LIF))
                        break;

                    if(debuglevel & 0x400)
                    {
                        printf("LIF name: %-10s start:%6ld size:%6ld %s\n", 
                            LIF->DIR.filename, 
                            (long)LIF->DIR.FileStartSector, 
                            (long)LIF->DIR.FileSectors,
                            lif_lifbcd2timestr(LIF->DIR.date) );
                    }
                }   // for(j=0)
            }       // if(liftel.dirsectorindex)
            liftel.sectorindex++;
        }   // for (i=0)
    }   // if(!liftel.dirprocessed)

    if(liftel.dirprocessed == 1)
    {
        if(LIF->sectors == 0)
        {
            printf("LIF image is empty!\n");
            // Not really an error
            liftel.done = 1;
            return(0);
        }
        printf("LIF image size in sectors:%ld\n", (long) LIF->sectors);
        liftel.dirprocessed = 2;
    }

    /// ===========================================
    /// @brief Done Processing LIF directory sectors 
    /// ===========================================

    ///@brief only process sectors until the last file sector is copied
    if( liftel.sectorindex > LIF->sectors)
    {
        if(liftel.error)
            return(0);
        return(1);
    }

    /// ===========================================
    /// @brief Write sectors from Teledisk track
    /// ===========================================
    // Write sectors from this TRACK to LIF file 
    for ( i=0;i < MAXSECTOR;i++ )
    {
        int sector;

        if(sectormap[i]  == -1)
            continue;

        sector = sectormap[i];

        if(debuglevel & 0x400)
        {
            if(sector > liftel.sectorspertrack)
                printf("skipping: ");
            printf(": cyl:%2d, side:%d, sector:%2d\n",
                (int)sectorconfig[sector].cylinder,
                (int)sectorconfig[sector].head,
                (int)sectorconfig[sector].sector);
            hexdump(sectorconfig[sector].input_data, sectorconfig[sector].sectorsize);
        }

        if( liftel.writeindex >= LIF->sectors)
        {
            liftel.done = 1;
            printf("LIF IMAGE Done\n");
            return(1);
        }

        size=fwrite(sectorconfig[sector].input_data, 1, liftel.sectorsize, LIF->fp);
        if(size != liftel.sectorsize)
        {
            printf("LIF: %s write error\n", LIF->name);
            liftel.done = 1;
            liftel.error = 1;
            return(0);
        }
        ++liftel.writeindex;
    }   
    /// ===========================================
    /// @brief Done writing sectors from Teledisk track
    /// ===========================================
    return(1); 
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
    liftel.done = 0;
    liftel.sectorzeroprocessed = 0;
    liftel.sectorspertrack = 0;
    liftel.sectorsize = -1;
    liftel.dirprocessed = 0;
    liftel.sectorindex = 0;
    liftel.writeindex = 0;

    status = TeleDisk_libLoad_DiskFile((floppy_t *) &floppy, telediskname, LIF);

    ///@brief  LIF summary
    printf("LIF image:[%s] wrote:[%ld] used sectors\n", LIF->name, (long)liftel.writeindex);

    ///@brief  Close LIF file
    lif_close_volume(LIF);

    lif_dir(lifname);
}
