/**
 @file fatfs/fatfs_utils.c

 @brief FatFs utilities and tests for HP85 disk emulator project.
   - Based on FatFs AVR example library (C) ChaN 2013.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.
 @par Copyright &copy; 2013 ChaN.

*/

#include "hardware/hardware.h"

#include "fatfs_tests.h"
#include "fatfs_utils.h"
#include "posix.h"


/// @brief Perform key FatFs diagnostics tests.
///
/// - Perform all basic file tests
/// - Assumes the device is formatted
///
/// @return void
void mmc_test(void)
{
    struct stat p;

	myprintf("==============================\n");
    myprintf("START MMC TEST\n");
    fatfs_status("/");
    myprintf("MMC Directory List\n");
    fatfs_ls("/");
    fatfs_cd("/");
    fatfs_create("test.txt","this is a test");
    fatfs_cat("test.txt");
    fatfs_copy("test.txt","test2.txt");
    fatfs_cat("test2.txt");
    fatfs_mkdir("/tmp");
    fatfs_copy("test.txt","tmp/test3.txt");
    fatfs_cat("tmp/test3.txt");
    fatfs_cd("/tmp");
    fatfs_pwd();
    fatfs_ls("");
    fatfs_cat("test3.txt");
    stat("test3.txt", &p);                        // POSIX test
    dump_stat(&p);

    myprintf("END MMC TEST\n");
	myprintf("==============================\n");
}


/// @brief Display FatFs test diagnostics help menu.
///
/// @see fatfs_tests.c.
///
/// @return  void
void fatfs_help( void )
{
    myprintf("debug N\n"
        "mmc_init\n"
        "mmc_test\n"
        "ls dir\n"
        "create file str\n"
        "cat file\n"
        "status str\n"
        "stat str\n"
        "rm str\n"
        "mkdir str\n"
        "rmdir str\n"
        "attrib p1 p2\n"
        "copy file1 file2\n"
        "rename file1 file2\n"
        "cd path\n"
        "pwd\n"
        "fatfs_help\n");
}


/// @brief FatFs test parser
///
///
/// - Keywords and arguments are matched against fatfs test functions
/// If ther are matched the function along with its argements are called.
///
///
/// @param[in] str: User supplied command line with FatFs test and arguments.
///
/// @see fatfs_tests.c
/// @return 1 The ruturn code indicates a command matched.
/// @return 0 if no rules matched
int fatfs_tests(char *str)
{

    int len;
    char *ptr;
    long p1, p2;

    ptr = skipspaces(str);

    if ((len = token(ptr,"mmc_test")) )
    {
        ptr += len;
        mmc_test();
        return(1);
    }
    else if ((len = token(ptr,"mmc_init")) )
    {
        ptr += len;
        mmc_init(0);
        return(1);
    }
    else if ((len = token(ptr,"mkfs")) )
    {
        FATFS fs;
        ptr += len;
        f_mount(&fs, "0:", 0);                    /* Register work area to the logical drive 0 */
        f_mkfs("0:", 0, 0);                       /* Create FAT volume on the logical drive 0. 2nd argument is ignored. */
        return(1);
    }
    else if ((len = token(ptr,"ls")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        fatfs_ls(ptr);
        return(1);
    }
    else if ((len = token(ptr,"create")) )
    {
        char *name,*end;
        ptr += len;
        name=skipspaces(ptr);
        end=nextspace(name);
        ptr=skipspaces(end);
        *end=0;
        fatfs_create(name,ptr);
        return(1);
    }
    else if ((len = token(ptr,"cat")) )
    {
        ptr += len;
        ptr = skipspaces(ptr);
        fatfs_cat(ptr);
        return(1);
    }
    else if ((len = token(ptr,"status")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        fatfs_status(ptr);
        return(1);
    }
    else if ((len = token(ptr,"stat")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        fatfs_stat(ptr);
        return(1);
    }
    else if ((len = token(ptr,"rm")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        fatfs_rm(ptr);
        return(1);
    }
    else if ((len = token(ptr,"mkdir")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        fatfs_mkdir(ptr);
        return(1);
    }
    else if ((len = token(ptr,"rmdir")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        fatfs_rmdir(ptr);
        return(1);
    }
    else if ((len = token(ptr,"attrib")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        sscanf(ptr,"%lu %lu", &p1,&p2);
        put_rc(f_chmod(ptr, p1, p2));
        return(1);
    }
    else if ((len = token(ptr,"copy")) )
    {
        char name1[128],name2[128];
        ptr += len;
        ptr=skipspaces(ptr);
        sscanf(ptr,"%s %s",name1,name2);
        fatfs_copy(name1,name2);
        return(1);
    }
    else if ((len = token(ptr,"rename")) )
    {
        char name1[128],name2[128];
        ptr += len;
        ptr=skipspaces(ptr);
        sscanf(ptr,"%s %s",name1,name2);
        fatfs_rename(name1,name2);
        return(1);
    }
#if _FS_RPATH
    else if ((len = token(ptr,"cd")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        fatfs_cd(ptr);
        return(1);
    }
#if _FS_RPATH >= 2
    else if ((len = token(ptr,"pwd")) )
    {
        fatfs_pwd();
        return(1);
    }
    else if ( (len = token(ptr,"fatfs_help")) )
    {
        fatfs_help();
        return(1);
    }
    return(0);
#endif
#endif
}
