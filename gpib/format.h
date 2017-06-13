#ifndef _FORMAT_H_
#define _FORMAT_H_
/**
 @file gpib/format.h

 @brief LIF disk image formatter for HP85 disk emulator project for AVR.

 @par Edit History - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2017 Mike Gore, Inc. All rights reserved.

*/


#include "user_config.h"
#include "defines.h"



/* format.c */
int lif_closedir ( lifdir_t *DIR );
lifdir_t *lif_opendir ( char *name );
DirEntryType *lif_readdir ( lifdir_t *DIR );
void lif_filename ( char *name , DirEntryType *DE );
long lif_filelength ( DirEntryType *DE );
int lif_dir ( char *name );
long lif_create_image ( char *name , char *label , long dirsecs , long sectors );

#endif // #ifndef _FORMAT_H_
