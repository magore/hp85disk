## What
 * LIF utilities to add,extract,translate to and from LIF image fies
 * TELEDISK to to convert TELEDISK LIF archives into pure LIF files
___
## Copyrights
 * Copyright &copy; 2014-2017 *Mike Gore*, All rights reserved. GPL
   * http://github.com/magore/hp85disk
   * http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

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

___
  
## OS Requirements for software building
  * I used Ubuntu 16.04LTS and 14.04LTS when developing the code
    * It should be easy to setup the same build with Windows gcc tools.
___

## Install Ubuntu Packages required for Building
  * sudo bash
  * *apt-get update*
  * *apt-get install aptitude make build-essential binutils gcc*
  * *aptitude --with-recommends install minicom avr-libc avra avrdude avrdude-doc avrp binutils-avr gcc-avr gdb-avr*


___
## Compiling Standlone LIF tools
  * *make clean*
  * *make*
___
## LIF command help
  * ./lif
  * ./lif help
  * ./td02lif

___
## FILES
  * hp85disk/lif                LIF utilities
  * Code by *Mike Gore*
    * Makefile
      * Build stand alone Linux / Windows versions of LIF tools and TELEDISK LIF extractor
    * lifsup.c            
    * lifsup.h  
      * Stand alone libraries for LIF utility - from various hp85disk libraries
    * lifutils.c          
    * lifutils.h  
      * LIF image functions, directory listing and file adding.extracting,renaming,deleting
    * td02lif.c           
    * td02lif.h           
      * My TeleDisk LIF extractor
    * lif-notes.txt       
      * My notes on decoding E010 format LIF images for HP-85
    * README.txt          
      * This file
  * hpmuseum.org
    * 85-SS80.TD0 from hpmuseum.org
      * Damaged SS80 Excersizer from HP Musium
    * 85-SS80.LIF
      * The current converter automaticcal did these repairs
        *  cyl 11, side 0 sector 116 mapped to 8
        *  cyl 13, side 0 sector 116 mapped to 11
        *  cyl 15, side 0 sector 10  missing - zero filled

hp85disk/lif/teledisk       Teledisk LIF format to pure LIF support code
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
___

## LIF HELP
   Run *lif* or *td02lif* without options to get help summary
  * Code by Mike Gore
    * lif
      * Stand alone LIF utilities - part of the HP85disk project for linux
    * td02lif
      * A version of LIF utility that just does TeleoDIsk to LIF conversion

    To get help type: ./lif - OR - ./td02lif

<pre>
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
</pre>
___

## LIF command help
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
___
## TELEDISK command help
* Convert TeleDIsk LIF encoded disk into pure LIF file
  * *lif td02lif image.td0 image.lif*
___

