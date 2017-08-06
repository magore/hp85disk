/**
  @file   td02lif.c
  @brief  Header file for stripped done version of TeleDisk decoder library
          See Credits for support code used
  @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved. GPL
  @see http://github.com/magore/hp85disk
  @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

  @Credits
      * We use stripped down version of teledisk_loader reader code to decode TELEDISK format
        * See: https://github.com/jfdelnero/libhxcfe/tree/master/sources/loaders/teledisk_loader
        * Part of HxCFloppyEmulator project Copyright (C) 2006-2014 Jean-Franois DEL NERO
      * LZSS and Teledisk Documention Copyright 2007-2008 Dave Dunfield All rights reserved.
      * CRC code By Ashley Roll Digital Nemesis Pty Ltd www.digitalnemesis.com

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

   * LIF command help
     * lif td02lif image.td0 image.lif
       * Convert TeleDisk encoded LIF disk image into to pure LIF image
*/


///@brief Recreated HxCFloppyEmulator Data structures for stand alone use
typedef struct {
    long floppyBitRate;
    int floppyNumberOfSide;
    int floppyNumberOfTrack;
    int floppySectorPerTrack;
} floppy_t;

enum 
{
    IBMFORMAT_SD,
    IBMFORMAT_DD
};

#ifndef _TD02LIF_H_
#define _TD02LIF_H_

///@brief Recreated HxCFloppyEmulator Data structures for stand alone use
typedef struct {
    int cylinder;
    int head;
    int sector;
    int sectorsize;
    long bitrate;
    int trackencoding;
    int use_alternate_datamark;
    int alternate_datamark;
    int use_alternate_data_crc;
    int missingdataaddressmark;
    uint8_t *input_data;
} sector_config_t;

///@brief td02lif state structure
typedef struct
{
    int  done;
    int  error;
    int state;
    int sectorspertrack;
    int sectorsize;
    int sectorlast;
    long sectorindex;
    long writeindex;
} liftel_t;

typedef struct tm tm_t;


///@brief sector table lookup sfor sorting
#define MAXSECTOR 256

extern liftel_t liftel;
extern int debuglevel;
#endif

/* td02lif.c */
int td02lif_track ( sector_config_t *sectorconfig , int track_sectors , void *data );
int lif_td02lif ( char *telediskname , char *lifname );
