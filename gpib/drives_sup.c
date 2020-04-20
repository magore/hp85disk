/**
 @file gpib/drives_sup.c
 @brief drive definition parsing for HP85 disk emulator project for AVR.
 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.
 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details
*/

#include "user_config.h"

#include "drives_sup.h"

hpdir_t hpdir;
/// ===========================================================
///@brief hpdir.ini file processing

///@brief Init Disk Parameter Structure
///
/// This structure will containes the parsed and computed
/// disk parameters from a single hpdir disk entry
///
///@return void
void hpdir_init()
{
    memset(hpdir.model,0,sizeof(hpdir.model) -1); // 1
                                                  // 2
    memset(hpdir.comment,0,sizeof(hpdir.comment) -1);
    memset(hpdir.TYPE,0,sizeof(hpdir.TYPE) -1);   // 3
    hpdir.ID = 0;                                 // 4
    hpdir.mask_stat2 = 0;                         // 5
    hpdir.id_stat2 = 0;                           // 6
    hpdir.DEVICE_NUMBER = 0;                      // 7
    hpdir.UNITS_INSTALLED = 0x8001;               // 8
    hpdir.CYLINDERS = 0;                          // 9
    hpdir.HEADS= 0;                               // 10
    hpdir.SECTORS= 0;                             // 11
    hpdir.BYTES_PER_SECTOR = 0;                   // 12
    hpdir.INTERLEAVE = 0;                         // 13
    hpdir.FIXED = 1;                              // 14 ALWAYS 1

// Computed values
    hpdir.BLOCKS = 0;
}


// =============================================
///@brief LIF Directory blocks ~= sqrt(blocks);
///
/// We simplify
///  BITS = MSB bit number of block count
///  Directory size = BITS / 2
///@param[in] blocks: size of LIF image in total
///
///@return Size of LIF directory in blocks
long lif_dir_count(long blocks)
{
    int scale = 0;
    long num = 1;
    while(blocks)
    {
        scale++;
        blocks >>= 1;
    }
    scale>>=1;
    while(scale--)
        num <<=1;
    return(num);
}


/// ===============================================
///@brief Find drive parameters in hpdir.ini file
///
///@param[in] model: model string
///
///@return 1 on sucess or 0 on fail
int hpdir_find_drive(char *model, int list, int verbose)
{
    int len;
    int errors = 0;
    int driveinfo=0;
    int found = 0;
    FILE *cfg;
    char *ptr;
    char str[256];
    char token[128];

    hpdir_init();

    cfg = fopen("hpdir.ini","rb");

#ifndef LIF_STAND_ALONE
    if(cfg == NULL)
        cfg = fopen("/hpdir.ini","rb");
#else
    if(cfg == NULL)
    {
        char name[2048];
        len = readlink("/proc/self/exe", name, sizeof(name) -2);
        dirname (name);
        strcat  (name, "/hpdir.ini");
        cfg = fopen(name, "rb");
    }
#endif

    if(cfg == NULL)
    {
        if(verbose)
            printf("Error: hpdir.ini not found!\n");
        return(0);
    }

// printf("Searching /hpdir.ini for model:%s\n", model);

    while( (ptr = fgets(str, sizeof(str)-2, cfg)) != NULL)
    {
        errors = 0;
        ptr = str;

        trim_tail(ptr);
        ptr = skipspaces(ptr);

        len = strlen(ptr);
        if(!len)
            continue;

// Skip comments
        if(*ptr == ';' || *ptr == '#' )
            continue;

        if(*ptr == '[' && driveinfo == 1 )
            break;

// MODEL something else
        ptr = get_token(ptr, token,     sizeof(token)-2);

        if(MATCHI(token,"[driveinfo]"))
        {
            driveinfo = 1;
            continue;
        }

        if( driveinfo != 1)
            continue;

        if(list)
        {
            printf("%s %s\n", token, ptr);
            continue;
        }

        if ( ! MATCHI(model,token) )
            continue;

        hpdir_init();

        if(verbose)
            printf("Found Model: %s\n", model);

                                                  // 1 Model
        strncpy(hpdir.model,token,sizeof(hpdir.model)-2);

                                                  // =
        ptr = get_token(ptr, token,     sizeof(token)-2);

                                                  // 2 Comment
        ptr = get_token(ptr, hpdir.comment, sizeof(hpdir.comment)-2);

                                                  // 3 AMIGO/SS80/CS80
        ptr = get_token(ptr, hpdir.TYPE,  sizeof(hpdir.TYPE)-2);

                                                  // 4 Identify ID
        ptr = get_token(ptr, token,         sizeof(token)-2);
        hpdir.ID = get_value(token);

                                                  // 5 MASK STAT 2
        ptr = get_token(ptr, token,         sizeof(token)-2);
        hpdir.mask_stat2 = get_value(token);

                                                  // 6 STAT2
        ptr = get_token(ptr, token,         sizeof(token)-2);
        hpdir.id_stat2 = get_value(token);

                                                  // 7 BCD include model number
        ptr = get_token(ptr, token,         sizeof(token)-2);
        hpdir.DEVICE_NUMBER = get_value(token);

                                                  // 8 Units installed
        ptr = get_token(ptr, token,         sizeof(token)-2);
        hpdir.UNITS_INSTALLED = get_value(token);

                                                  // 9 Cylinders
        ptr = get_token(ptr, token,         sizeof(token)-2);
        hpdir.CYLINDERS = get_value(token);

                                                  // 10 Heads
        ptr = get_token(ptr, token,         sizeof(token)-2);
        hpdir.HEADS = get_value(token);

                                                  // 11 Sectors
        ptr = get_token(ptr, token,         sizeof(token)-2);
        hpdir.SECTORS = get_value(token);

                                                  // 12 Bytes Per Block/Sector
        ptr = get_token(ptr, token,         sizeof(token)-2);
        hpdir.BYTES_PER_SECTOR = get_value(token);

                                                  // 13 Interleave
        ptr = get_token(ptr, token,         sizeof(token)-2);
        hpdir.INTERLEAVE = get_value(token);

// Computed values
        hpdir.BLOCKS = ( hpdir.CYLINDERS * hpdir.HEADS * hpdir.SECTORS );

        if(errors)
        {
            if(verbose)
                printf("Error /hpdir.ini parsing\n");
            break;
        }
        found = 1;
        break;

    }                                             // while
    fclose(cfg);
    return(found);
}
