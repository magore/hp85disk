## What
 * LIF utilities to add,extract,translate to and from LIF image fies
 * TELEDISK to to convert TELEDISK LIF archives into pure LIF files
___

## **HP85** Disk Emulator Copyright (C) 2014-2020 Mike Gore
  * This Project Emulates **AMIGO** and **SS80** disk drives used by the **HP85** series computers.
  * The images are stored in **LIF** format used by the **HP85** series computers
  * See [COPYRIGHT](COPYRIGHT.md) for a full copyright notice for the project
  * Documentation created using Doxygen available at
    * https://rawgit.com/magore/hp85disk/master/doxygen/html/index.html
    * All emulated disk images are just regular files stored on a standard FAT32 formatted SD Card

 * Copyright (C) 2014-2017 *Mike Gore*, All rights reserved. GPL
   * http://github.com/magore/hp85disk
   * http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

___


## Credits for software used in my **LIF** and **TeleDisk to LIF** conversion utilities
## Credits
 * Important Contributions (My converted would not have been possible without these)
 * I used the lzss libraries and documentation by **Dave Dunfield**
   * Copyright 2007-2008 Dave Dunfield All rights reserved.
 * Documentation from **Jean-Franois DEL NERO**
   * Copyright (C) 2006-2014 Jean-Franois DEL NERO
 * [lif/teledisk](lif/teledisk)
   * My **TELEDISK LIF** extractor
   * Important Contributions (My converted would not have been possible without these)
     * **Dave Dunfield**, LZSS Code and *TeleDisk* documentation
       * Copyright 2007-2008 Dave Dunfield All rights reserved.
       * [td0_lzss.h](lif/teledisk/td0_lzss.h)
       * [td0_lzss.c](lif/teledisk/td0_lzss.c)
         * *LZSS decoder*
       * [td0notes.txt](lif/teledisk/td0notes.txt)
         * *TeleDisk Documentation*
     * **Jean-Franois DEL NERO**, **TeleDisk Documentation**
       * Copyright (C) 2006-2014 Jean-Franois DEL NERO
         * [wteledsk.htm](lif/teledisk/wteledsk.htm)
           * *TeleDisk documentation*
         * See his github project
             * https://github.com/jfdelnero/libhxcfe

___
  

## OS Requirements for software building
  * See main project [README.md](README.md)

## Install Ubuntu Packages required for Building
  * See main project [README.md](README.md)

## Compiling Standalone LIF and TeleDisk tools
  * **cd lif**
  * **make clean**
  * **make**
## LIF and TeleDisk standalone tool help
  * **lif**
  * ** lif help**
  * ** td02lif**

___


## **LIF** files
  * [lif](lif)
    * My **LIF** disk image utilities
    * [lif/lifutils.c](lif/lifutils.c)
    * [lif/lifutils.c](lif/lifutils.c)
      * Functions that allow the emulator to import and export files from **LIF** images
    * [Makefile](lif/Makefile)
      * Permits creating a standalone Linux version of the **LIF** emulator tools
    * Code by Mike Gore
      * [Makefile](lif/Makefile)
        * Make stand alone Linux versions of **LIF** utility and optionaly TeleDisk to *LIF* converter
      * [lifsup.c](lif/lifsup.c)
      * [lifsup.h](lif/lifsup.h)
        * Stand alone libraries for **LIF** and TELEDISK utility
          * These functions are also part of various hp85disk libraries
      * [lifutils.c](lif/lifutils.c)
      * [lifutils.h](lif/lifutils.h)
        * **LIF** image functions, directory listing and file adding. extracting,renaming,deleting
      * [td02lif.c](lif/td02lif.c)
      * [td02lif.h](lif/td02lif.h)
        * My TeleDisk to **LIF** translator
      * [lif-notes.txt](lif/lif-notes.txt)
        * My notes on decoding E010 format **LIF** images for HP-85
      * [README.txt](lif/README.txt)
        * Notes on each file under **LIF** and lif/teledisk
      * [85-SS80.TD0](lif/85-SS80.TD0) from hpmuseum.org
        * Damaged SS80 Exerciser from HP Museum
      * [85-SS80.LIF](lif/85-SS80.LIF)
        * The current converter automatically did these repairs
          *  cyl 11, side 0 sector 116 mapped to 8
          *  cyl 13, side 0 sector 116 mapped to 11
          *  cyl 15, side 0 sector 10  missing - zero filled


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
    Stand alone version of LIF/TELEDISK utilities for linux
    HP85 Disk and Device Emulator
     (c) 2014-2020 by Mike Gore
     GNU version 3
    -> https://github.com/magore/hp85disk
       GIT last pushed:   2020-04-11 01:13:56.363450958 -0400
       Last updated file: 2020-04-11 00:50:34.784232621 -0400
    
    lif help
    lif add lifimage lifname from_ascii_file
    lif addbin lifimage lifname from_lif_file
    lif create lifimage label directory_sectors sectors
    lif createdisk lifimage label model
    lif del lifimage name
    lif dir lifimage
    lif extract lifimage lifname to_ascii_file
    lif extractbin lifimage lifname to_lif_file
        extracts a file into a sigle file LIF image
    lif rename lifimage oldlifname newlifname
    lif td02lif [options] image.td0 image.lif
    Use -d after first keyword 'lif' above for LIF filesystem debugging
    
    td02lif help
    Usage: td02lif [options] file.td0 file.lif
           td02lif help
    tdo2lif options:
    Notes: for any option that is NOT specified it is automattically detected
             -s256|512 | -s 256|512 - force sector size
             -h1|2 | -h 1|2 - force heads/serfaces
             -tNN | -t NN  - force tracks
    
</pre>

## LIF command summary
  * Built in help
    * **lif help**
      * Gives lif commands
  * Summary - List, Create, import, export, translate, copy, rename, delete, etc
  * NOTE: Each disk image is a single file, encoded in **LIF** format,saved on the SD Card
    * **LIF** format is a common the filesystem on series 80 computers.
    * **LIF** format is also a vary common file interchange format for series 80 computers
      * **LIF** format includes file date,size permissions and other important meta data
  * Translate between DTA8x (type E010) and plain text files
      * You can add a plain text file, and translate it, into a **LIF** image with file format DTA8x (type E010)
      * You can extract and translate DTA8x (type E010) into a plain text files
  * **Key LIF manipulation features**
    * **dir** display a directory of a **LIF** image
      * Directory listing of **LIF** images and SSD Card files
      * If you have an RTC the listing can display file and **LIF** volume date and time
        * Display time stamps if they were set
          * But only if they were created or added with the built in tools
    * **add** an ascii file to **LIF** image
      * This function permits renaming of the translated file
      * They get translated between **HP85** DTA8x (type E010) format and plain text files!!!
    * **extract** ASCII files from **LIF** image
      * This function permits renaming of the translated file
      * They get translated between **HP85** DTA8x (type E010) format and plain text files!!!
    * **addbin** add binary programs from one **LIF** image to another *LIF* image
      * This function permits renaming of the translated file
    * **extractbin** a single binary file or program into a new **LIF** image
      * This function permits renaming of the translated file
      * Extracted **LIF** images contain a single file a 256 byte volume header, 256 byte directory followed by a file.
    * **del** delete file in **LIF** image
    * **rename** file in **LIF** image
    * **create** create a LIF image specifing a label, directory size and overall disk size
    * **createdisk** create a LIF image specifying a label and drive model name
  * [For more **LIF** documentation](lif/README.md)



## LIF command examples
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


## **LIF** teledisk files
   * [lif/teledisk](lif/teledisk)
     * My TELEDISK **LIF** extractor
       * Note: The TeleDisk image MUST contain a **LIF** image  - we do NOT translate it
     * [README.txt](lif/teledisk/README.txt)
       * Credits
     * Important Contributions (My converter would not have been possible without these)
       * Dave Dunfield, LZSS Code and TeleDisk documentation
         * Copyright 2007-2008 Dave Dunfield All rights reserved.
         * [td0_lzss.h](lif/teledisk/td0_lzss.h)
         * [td0_lzss.c](lif/teledisk/td0_lzss.c)
           * LZSS decoder
         * [td0notes.txt](lif/teledisk/td0notes.txt)
           * Teledisk Documentation
       * Jean-Franois DEL NERO, TeleDisk Documentation
         * Copyright (lif/teledisk/C) 2006-2014 Jean-Franois DEL NERO
           * [wteledsk.htm](lif/teledisk/wteledsk.htm)
             * TeleDisk documentation
           * See his github project
             * https://github.com/jfdelnero/libhxcfe
___


## TELEDISK command usage
* Convert TeleDIsk LIF encoded disk into pure LIF file
  * *lif td02lif image.td0 image.lif*

## TELEDISK conversion technical notes - extracting LIF data
  * Unfortunately TeleDisk saves everything in on all sides and tracks. 
    * Question: what happens when we have a disk formatted multiple times with differing parameters ?
    
  * Example 1 
    * Say we start an image that was two sided 80 track disk, 9 sectors, 512 byte sectors
    * Later on someone formats it single sides 35 track disk, 16 sectors, 256 bytes sectors
  * Analisis
    * We examing the first 30 tracks on both sides looking for:
      * FIRST sector on both sides and its size
      * LAST sector on both size that matches the size
      * COUNT of sectors on each side that matches the size
    * We then compare sector COUNT, compare sector SIZE, and FIRSTd 
      * If SIZE or COUNT is not the same reject
      * If only FISRT mismatches we then check FIRST for continuation of numbering on each side
  * Conclusion
    * We can reject side two by comparing the
      * FIRST sector number, Number of sectors and size

  * Eample 2
    * We have a double sided disk that is formated single sided with FIRST,LAST,COUNT and SIZE matching
  * Analisis
    * We have no automated way to reject side two
  * Conclusion
    * The user will have to override heads

___


## TELEDISK extracting LIF data user overrides
  * Number of Sides:
    * You CAN specify single sided override for two sided disks
      * (as defined in the TeleDisk headers)
    * However you can NOT specify 2 sided mode if the TeleDisk image headers say there is one side

  * Sector Size override
    *  Rather then using the SIZE of the FIRST sector as a filter the user can manually pick the size

  * Number of tracks overrides
    *  The LIF decoder stops after the last sector of the last file so this option is ignored
</pre>

___

## TELEDISK LIF conversion example
<pre>
./td02lif 85-SS80.TD0 1.LIF
TeleDisk file:         85-SS80.TD0
        Version:       21
        Not Compressed
        Density:       00h
        DriveType:     04h
        TrackDensity:  00h
        DosMode:       00h
        Sides:         02
        Comment Size:  84
        Comment Date:  Sun Apr  1 12:47:51 2008

HP-85 S/W, SS/80 EXERCISER, 5010-0310/2537. DOES NOT INCLUDE THE SERVC F

Warning: alternate CRC flag not implemented
        cylinder:00, head:00, sector:17, size:128, index:17
Warning: Sector:08 missing - found alternate sector:116
         Location: cylinder:11, head:00, sector:08, size:256, index:8
Warning: Sector:11 missing - found alternate sector:116
         Location: cylinder:13, head:00, sector:11, size:256, index:11
Warning: Sector:10 missing - zero filling
         Location: Track: Cyl: 15, Side: 00
Warning: Sector:01 missing - zero filling
         Location: Track: Cyl: 30, Side: 00
Warning: Sector:09 missing - zero filling
         Location: Track: Cyl: 41, Side: 00
Warning: Sector:11 missing - zero filling
         Location: Track: Cyl: 61, Side: 00
Warning: Sector:01 missing - zero filling
         Location: Track: Cyl: 62, Side: 00
Warning: Sector:10 missing - zero filling
         Location: Track: Cyl: 63, Side: 00

Disk Layout
         Sides:              1
         Tracks:            70
         Sectors Per Track: 16
         Sector Size:      256
Side: 0
         Sector numbering:  0 to 15
         Sector size:     256
         Sector count:     16
Side: 1
         Sector numbering:  1 to  9
         Sector size:     512
         Sector count:      9

Warning: rejecting side two
         Warning: Sector size:  NOT the same on both sides
         Warning: Sector count: NOT the same on both sides
         Warning: Sector range: NOT a continuation of side 0

LIF sectors processed: 1120

Done LIF image: [1.LIF] wrote: [01DAh] sectors

Volume:[<EMPTY>] Date:[<EMPTY>]
NAME         TYPE   START SECTOR        SIZE    RECSIZE   DATE
MANUAL      E020h            10h       26880        256   <EMPTY>
RW-TES      E020h            79h       27648        256   <EMPTY>
OPER        E020h            E5h       28672        256   <EMPTY>
REVID       E020h           155h       27136        256   <EMPTY>
Autost      E020h           1BFh        6912        256   <EMPTY>

       5 Files
       0 Purged
     458 Used sectors
       0 Free sectors

</pre>

___


