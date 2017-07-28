 @file README.txt

 @brief  LIF utilities to add,extract,translate to and from LIF image fies
         We also created a wrapper to extract LIF sectors from Teledisk Images
         See credits for support code used
 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

 @Credits
     * We use stripped down version of teledisk_loader reader code to decode TELEDISK format
       * See: https://github.com/jfdelnero/libhxcfe/tree/master/sources/loaders/teledisk_loader
       * Part of HxCFloppyEmulator project Copyright (C) 2006-2014 Jean-Franois DEL NERO
     * LZSS and Teledisk Documention Copyright 2007-2008 Dave Dunfield All rights reserved.
     * CRC code By Ashley Roll Digital Nemesis Pty Ltd www.digitalnemesis.com

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
        wteledsk.htm        Jean-Framois Combined TeleDisk documenation from David Dunfiled and other sources
