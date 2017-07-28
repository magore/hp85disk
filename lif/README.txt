 @file README.txt

 @brief  LIF utilities to add,extract,translate to and from LIF image fies
         We also created a wrapper to extract LIF sectors from Teledisk Images 
         See credits for support code used
 @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL
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
     * ./lif
     * ./lif help
     * ./td02lif


hp85disk/lif                LIF utilities
    Code by Mike Gore
        Makefile            Make stand alone Linux versions of LIF utility and optionaly TeleDisk to LIF converter
        lifsup.c            Stand alone libraries for LIF utility - from various hp85disk libraries
        lifsup.h  
        lifutils.c          LIF image functions, directory listing and file adding.extracting,renaming,deleting
        lifutils.h  
        td02lif.c           My TeleDisk decoder wrapper to convert Teledisk LIF images into pure LIF files
        lif-notes.txt       My notes on decoding E010 format LIF images for HP-85
        README.txt          This file
    hpmuseum.org
        85-SS80.TD0         Damaged SS80 Excersizer from HP Musium
        85-SS80.LIF         My (hopefully) corrected LIF version (fixed two sectors addresses in TeleDisk image)
                            My TeleDisk fix was to correct two sectors that had sector ID address of 116:
                            I added the following to td02lif.c
                                if(cyl == 11 && side == 0 && sector == 116)
                                    sector = 8;
                                if(cyl == 13 && side == 0 && sector == 116)
                                    sector = 11;

hp85disk/lif/teledisk       Teledisk LIF format to pure LIF support code
    Code by Mike Gore
        td02lif.h           Headers to make teledisk_loader.c stand alone
        README.txt
        Makefile            Make library from teledisk_loader.c crc.c td0_lzss.c

    Code by Ashley Roll
        crc.c               CRC16 code 
        crc.h  

    Code and Documenation by David Dunfield
        td0_lzss.h          LZSS decoder 
        td0_lzss.c  
        td0notes.txt        David Dunfield Teledisk Documentation

    Code by Jean-Franois DEL NERO
        teledisk_loader.c  
        teledisk_format.h  
        teledisk_loader.h  
        wteledsk.htm        Combined TeleDisk documenation for various sources


hp85disk/lif
    Code by Mike Gore
        lif                 Stand alone LIF utilities - part of the HP85disk project for linux
        td02lif             A version of LIF utility that just does TeleoDIsk to LIF conversion

        To get help type: ./lif - OR - ./td02lif

        ./lif

        Stand alone version of LIF utilities for linux
        HP85 Disk and Device Emulator
         (c) 2014-2017 by Mike Gore
         GNU version 3
        -> https://github.com/magore/hp85disk
           GIT last pushed:   2017-07-17 17:03:59.102151709 -0400
           Last updated file: 2017-07-27 00:34:18.512415597 -0400

        Usage: td02lif file.td0 file.lif
         - OR -

        lif help
        lif add lifimage lifname from_ascii_file
        lif addbin lifimage lifname from_lif_file
        lif create lifimage label directory_sectors sectors
        lif del lifimage name
        lif dir lifimage
        lif extract lifimage lifname to_ascii_file
        lif extractbin lifimage lifname to_lif_file
        lif rename lifimage oldlifname newlifname
        lif td02lif image.td0 image.lif

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

     * lif extractbin lifimage llifname to_lif_image
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
       * The HxCFloppyEmulator library is Copyright (C) 2006-2014 Jean-Fran▒ois DEL NERO
       * See: https://github.com/jfdelnero/libhxcfe/tree/master/sources/loaders/teledisk_loader
    
