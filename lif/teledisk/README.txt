 @file README.txt

  @brief  TeleDisk decoder library targetting LIF format images only
  @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved. GPL
  @see http://github.com/magore/hp85disk
  @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

  @help
   * LIF command help
     * lif td02lif image.td0 image.lif
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
