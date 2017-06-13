#ifndef _FORMAT_H_
#define _FORMAT_H_
/**
 @file gpib/format.h

 @brief LIF disk image formatter for HP85 disk emulator project for AVR.

 @par Edit History - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2017 Mike Gore, Inc. All rights reserved.

*/


#include "user_config.h"


#endif // #ifndef _FORMAT_H_

/* format.c */
uint8_t *LIFPackDir ( uint8_t *B , DirEntryType *D );
void LIFPackVolume ( uint8_t *B , VolumeLabelType *V );
uint8_t BIN2BCD ( uint8_t data );
void time_to_LIF ( uint8_t *bcd , time_t t );
long create_lif_image ( char *name , char *label , long dirsecs , long sectors );

