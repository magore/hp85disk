/**
 @file fatfs/fatfs_utils.c

 @brief fatfs test utilities with user interface

 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL  License
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for specific Copyright details

 @par Credit: part of FatFs avr example project (C)ChaN, 2013.
 @par Copyright &copy; 2013 ChaN.

@par You are free to use this code under the terms of GPL
please retain a copy of this notice in any code you use it in.

This is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option)
any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "user_config.h"
#include "fatfs.h"

#ifdef AVR
//#include <stdio.h>
#include <stdlib.h>
#endif

#include "time.h"
#include "stringsup.h"


/// @brief Display FatFs test diagnostics help menu.
/// @return  void
MEMSPACE
void fatfs_help( int full)
{
    if(full)
    {
        printf(
#ifdef POSIX_TESTS
            "Note: fatfs tests MUST start with \"fatfs\" keyword\n"
#else
            "Note: fatfs prefix is optional\n"
#endif
            "fatfs help\n"
#ifdef FATFS_UTILS_FULL
            "fatfs attrib file p1 p2\n"
            "fatfs cat file\n"
            "fatfs cd dir\n"
            "fatfs copy file1 file2\n"
            "fatfs create file str\n"
#endif
            "fatfs mmc_test\n"
            "fatfs mmc_init\n"
            "fatfs ls directory\n"

#ifdef FATFS_UTILS_FULL
            "fatfs mkdir dir\n"
            "fatfs mkfs\n"
            "fatfs pwd\n"
#endif
            "fatfs status\n"

#ifdef FATFS_UTILS_FULL
            "fatfs stat file\n"
            "fatfs rm file\n"
            "fatfs rmdir dir\n"
            "fatfs rename old new\n"
#endif
            "\n"
            );
    }
	else
	{
		printf("fatfs     help\n");
	}

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
/// @return 1 The ruturn code indicates a command matched.
/// @return 0 if no rules matched
MEMSPACE
int fatfs_tests(int argc,char *argv[])
{
    char *ptr;
    int ind;

    char buff[MAX_NAME_LEN+1];

    ind = 0;
    ptr = argv[ind];

    if(!ptr)
        return(0);

// If we have POSIX_TESTS we MUST prefix each test with "fatfs" keyword to avoid name clashing

    if( MATCHI(ptr,"fatfs") )
    {
        ptr = argv[++ind];
        if ( !ptr || MATCHI(ptr,"help") )
        {
            fatfs_help(1);
            return(1);
        }
    }

#ifdef POSIX_TESTS
    else
    {
        return(0);
    }
#endif

    if (MATCHI(ptr,"ls") || MATCHI(ptr,"dir") )
    {
        int i;
        int args = 0;
        for(i=ind+1;i<argc;++i)
        {
            if(fatfs_ls(argv[i]) == 0)
			{
				return(-1);
			}
            ++args;
        }
        if(!args)
        {
            if(fatfs_ls("") == 0)
			{
				return(-1);
			}
        }
        return(1);
    }

    else if (MATCHARGS(ptr,"mmc_test",(ind+0),argc ))
    {
        mmc_test();
        return(1);
    }

    else if (MATCHARGS(ptr,"mmc_init",(ind+0),argc))
    {
        mmc_init(1);
        return(1);
    }

    else if (MATCHARGS(ptr,"status", (ind + 1), argc))
    {
        strcpy(buff,"/");
        if(fatfs_status(buff)== 0)
		{
			return(-1);
		}
        return(1);
    }

#ifdef FATFS_UTILS_FULL
    else if (MATCHARGS(ptr,"attrib",(ind+3),argc))
    {
		int res;
        res = f_chmod(argv[ind],atol(argv[ind+1]),atol(argv[ind+2]));
        if(res)
		{
			printf("fatfs attribute failed\n");
			return(-1);
		}
        return(1);
    }

    else if (MATCHARGS(ptr,"cat", (ind + 1), argc))
    {
        if( fatfs_cat(argv[ind]) < 0)
		{
			return(-1);
		}
        return(1);
    }

#if FF_FS_RPATH
    else if (MATCHARGS(ptr,"cd", (ind + 1), argc))
    {
        if( fatfs_cd(argv[ind]) == 0)
		{
			printf("fatfs cd %s failed\n", argv[ind]);
			return(-1);
		}
        return(1);
    }
#endif

    else if (MATCHARGS(ptr,"copy", (ind + 2), argc))
    {
        if( fatfs_copy(argv[ind],argv[ind+1]) < 0)
		{
			printf("fatfs copy failed\n");
			return(-1);
		}
        return(1);
    }

    else if (MATCHARGS(ptr,"create", (ind + 2), argc))
    {
        if( fatfs_create(argv[ind],argv[ind+1]) < 0)
		{
			printf("fatfs create failed\n");
			return(-1);
		}
        return(1);
    }

    else if (MATCHARGS(ptr,"mkdir", (ind + 1), argc))
    {
        if( fatfs_mkdir(argv[ind]) ==  0)
		{
			printf("fatfs mkdir %s failed\n", argv[ind]);
			return(-1);
		}
        return(1);
    }

    else if (MATCHARGS(ptr,"mkfs", (ind + 0), argc))
    {
        FATFS fs;
        uint8_t *mem;
        int res;
/* Register work area to the logical drive 0 */
        res = f_mount(&fs, "0:", 0);
        if (res)
		{
			printf("fatfs mkfs f_mount failed\n");
            return(-1);
		}
        mem = safemalloc(1024);
/* Create FAT volume on the logical drive 0. 2nd argument is ignored. */
        res = f_mkfs("0:", FM_FAT32, 0, mem, 1024);
        safefree(mem);
		if(res)
		{
			printf("fatfs mkfs f_mkfs failed\n");
			return(-1);
		}
        return(1);
    }

#if FF_FS_RPATH
#if FF_FS_RPATH >= 2
    else if (MATCHARGS(ptr,"pwd", (ind + 0), argc))
    {
        if( fatfs_pwd() == 0)
		{
			printf("fatfs pwd failed\n");
			return(-1);
		}
        return(1);
    }
#endif                                        // #if FF_FS_RPATH >= 2
#endif                                        // #if FF_FS_RPATH

    else if (MATCHARGS(ptr,"rename", (ind + 2), argc))
    {
        if( fatfs_rename(argv[ind],argv[ind+1]) == 0)
		{
			printf("fatfs mkdir %s failed\n", argv[ind]);
			return(-1);
		}
        return(1);
    }

    else if (MATCHARGS(ptr,"rmdir", (ind + 1), argc))
    {
        if( fatfs_rmdir(argv[ind]) == 0)
		{
			printf("fatfs rmdir %s failed\n", argv[ind]);
			return(-1);
		}
        return(1);
    }

    else if (MATCHARGS(ptr,"rm", (ind + 1), argc))
    {
        if( fatfs_rm(argv[ind]) == 0)
		{
			printf("fatfs rm %s failed\n", argv[ind]);
			return(-1);
		}
        return(1);
    }

    else if (MATCHARGS(ptr,"stat", (ind + 1), argc))
    {
        if( fatfs_stat(argv[ind]) == 0)
		{
			printf("fatfs stat %s failed\n", argv[ind]);
			return(-1);
		}
        return(1);
    }
#endif

    return(0);
}


/// @brief Perform key FatFs diagnostics tests.
///
/// - Perform all basic file tests
/// - Assumes the device is formatted
///
/// @return void
MEMSPACE
void mmc_test(void)
{
    char buff[MAX_NAME_LEN+1];

    sep();
    printf("START MMC TEST\n");
    strcpy(buff,"/");
    fatfs_status(buff);
    printf("MMC Directory List\n");
    fatfs_ls("/");

#ifdef FATFS_UTILS_FULL
#if FF_FS_RPATH
    fatfs_cd("/");
#endif
    fatfs_create("test.txt","this is a test");
    fatfs_cat("test.txt");
    fatfs_ls("/");
    fatfs_copy("test.txt","test2.txt");
    fatfs_cat("test2.txt");
#if FF_FS_RPATH
    fatfs_mkdir("/tmp");
    fatfs_copy("test.txt","tmp/test3.txt");
    fatfs_cat("tmp/test3.txt");
    fatfs_cd("/tmp");
    fatfs_pwd();
    fatfs_cat("test3.txt");
    fatfs_ls("");
#endif
#endif

    printf("END MMC TEST\n");
    sep();
}


///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] ptr: pathname of directory to list
///
/// @see fatfs_filinfo_list().
/// @return  1 on success or 0 on error
MEMSPACE
int fatfs_ls(char *name)
{
    long p1;
    UINT s1, s2;
    int res;
    FILINFO fno;
    DIR dirs;                                     /* Directory object */
    FATFS *fs;
    char buff[MAX_NAME_LEN+1];

	memset(buff,0,sizeof(buff)-1);

    if(!name || !*name)
    {
        strcpy(buff,".");
    }
    else
    {
        strcpy(buff,name);
    }
    printf("Listing:[%s]\n",buff);

    res = f_opendir(&dirs, buff);
    if (res != FR_OK) 
	{ 
		return(0); 
	}
    p1 = s1 = s2 = 0;
    while(1)
    {
        res = f_readdir(&dirs, &fno);
        if (res != FR_OK) 
			break;
		if(!fno.fname[0]) 
			break;
        if (fno.fattrib & AM_DIR)
        {
            s2++;
        }
        else
        {
            s1++; p1 += fno.fsize;
        }
        fatfs_filinfo_list(&fno);
#ifdef ESP8266
        optimistic_yield(1000);
        wdt_reset();
#endif
    }
	f_closedir(&dirs);
    printf("%4u File(s),%10lu bytes total\n%4u Dir(s)", s1, p1, s2);
    if (f_getfree(buff, (DWORD*)&p1, &fs) == FR_OK)
        printf(", %10luK bytes free\n", p1 * fs->csize / 2);
	if(res)
		return(-1);
	return(1);
}


#ifdef FATFS_UTILS_FULL
/// @brief  Display the contents of a file
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: file name.
///
/// @return  sizae  on sucess -1 on error
MEMSPACE
long fatfs_cat(char *name)
{
    UINT s1;
    FIL fp;
    int res;
    int i;
    int ret;
    long size;
    char *ptr;

    printf("Reading[%s]\n", name);
    res = f_open(&fp, name, FA_OPEN_EXISTING | FA_READ);
    if (res)
    {
        printf("cat error\n");
        f_close(&fp);
		return(0);
    }

    ptr = safecalloc(512,1);
    if(!ptr)
    {
        printf("Calloc failed!\n");
        f_close(&fp);
		return(0);
    }

    size = 0;
    while(1)
    {
/// @todo FIXME
        res = f_read(&fp, ptr, 512, &s1);
        if(res)
        {
            printf("cat read error\n");
			return(0);
            break;
        }
        ret = s1;
        if (!s1)
        {
            break;
        }
        size += ret;
        for(i=0;i<ret;++i)
        {
//FIXME putchar depends on fdevopen having been called
            if(stdout)
                putchar(ptr[i]);
            else
                uart_putc(0,ptr[i]);
        }
#ifdef ESP8266
        optimistic_yield(1000);
        wdt_reset();
#endif
    }
    printf("\n");
    f_close(&fp);
    safefree(ptr);
    printf("%lu bytes\n", size);
	return(size);
}


/// @brief  Copy a file.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] from: source file.
/// @param[in] to:   destination file.
///
/// @return  size on success -1 on error

MEMSPACE
long fatfs_copy(char *from,char *to)
{
    UINT s1, s2;
    FIL file1,file2;
    int res;
    long size;
    char *ptr;
#ifdef ESP8266
#define MSIZE 4096
#else
#define MSIZE 512
#endif
    printf("Opening %s\n", from);
    res = f_open(&file1, from, FA_OPEN_EXISTING | FA_READ);
    if (res)
    {
        printf("f_open %s failed\n", from);
        return(-1);
    }
    printf("Creating %s\n", to);
    res = f_open(&file2, to, FA_CREATE_ALWAYS | FA_WRITE);
    if (res)
    {
        printf("f_open %s failed\n", to);
        f_close(&file1);
        return(-1);
    }
    ptr = safecalloc(MSIZE,1);
    if(!ptr)
    {
        printf("f_open calloc failed!\n");
        f_close(&file1);
        f_close(&file2);
        return(-1);
    }
    printf("\nCopying...\n");
    size = 0;
    for (;;)
    {
        res = f_read(&file1, ptr, MSIZE, &s1);
        if (res )	// error
		{
			printf("fatfs copy read %s error\n",from);
			break;
		}
		if(s1 == 0) 
			break;                /* eof */
        res = f_write(&file2, ptr, s1, &s2);
        if (res )	// error
		{
			printf("fatfs copy %s write\n",to);
			break;
		}
        size += s2;
        printf("Copied: %08ld\r", size);
        if (res )	// error
			break;
		if(s2 == 0) 
			break;                /* write error */
    }
    safefree(ptr);
    f_close(&file1);
    f_close(&file2);
    printf("%lu bytes copied.\n", size);
    if (res)
		return(-1);
	return(size);
}


/// @brief  Create a new file from a user supplied string.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: name of file to create.
/// @param[in] str: string containing file contents.
///
/// @return  size on success -1 on error
MEMSPACE
int fatfs_create(char *name, char *str)
{
    UINT s1;
    UINT len;
    int res;
    FIL fp;
    printf("Creating [%s]\n", name);
    printf("Text[%s]\n", str);
    res = f_open(&fp, name, FA_CREATE_ALWAYS | FA_WRITE);
    if (res)
    {
        printf("fatfs_create f_open error\n");
        f_close(&fp);
        return(-1);
    }

    len = strlen(str);
    res = f_write(&fp, str, (UINT)len, &s1);

    if (res)
    {
        printf("fatfs_create f_write error\n");
        return(-1);
    }
    if (len != s1)
    {
        printf("fatfs_create f_write error - wanted(%d) got(%d)\n",s1,len);
        return(-1);
    }

    f_close(&fp);
	return((long) s1);
}


#if FF_FS_RPATH >= 2
/// @brief  Change directory.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: Directory name.
///
/// @return   on success 0 on fail
MEMSPACE
int fatfs_cd(char *name)
{
	int res;
    printf("fatfs cd [%s]\n", name);
    res = f_chdir(name);
	if(res)
	{
		printf("fatfs_cd %s failed\n",name);
		return(0);
	}
	return(1);
}
#endif

/// @brief  Make a directory.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: Directory name.
///
/// @return  void
MEMSPACE
int fatfs_mkdir(char *name)
{
	int res;

    printf("fatfs mkdir [%s]\n", name);
    res = f_mkdir(name);
	if(res)
	{
		printf("mkdir %s failed\n",name);
		return(0);
	}
	return(1);
}


#if FF_FS_RPATH >= 2
/// @brief  Display current working directory.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @return  1 on success 0 on fail
MEMSPACE
int fatfs_pwd(void)
{
    int res;
    char str[128];
    res = f_getcwd(str, sizeof(str)-2);
    if (res)
	{
		printf("fatfs pwd failed\n");
		return(0);
	}
	printf("fatfs pwd [%s]\n", str);
	return(1);
}
#endif

/// @brief Rename a file
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] oldpath: old name.
/// @param[in] newpath: new name.
///
/// @return  1 on success 0 on ERROR
MEMSPACE
int fatfs_rename(const char *oldpath, const char *newpath)
{
/* Rename an object */
    int res;
    printf("fatfs rename [%s] to [%s]\n", oldpath,newpath);
    res = f_rename(oldpath, newpath);
    if (res)
	{
		printf("fatfs rename failed\n");
		return(0);
	}
	return(1);
}


/// @brief  Delete a file by name.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: File name to delete.
///
/// @return  void.

MEMSPACE
int fatfs_rm(char *name)
{
    int res;
    printf("fatfs rm [%s]\n", name);
    res = f_unlink(name);
    if (res)
	{
		printf("fatfs_rm %s failed\n", name);
		return(0);
	}
	return(1);
}


/// @brief  Delete a directory.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: Directory name.
///
/// @return  void.
MEMSPACE
int fatfs_rmdir(char *name)
{
    int res;
    printf("fatfs rmdir [%s]\n", name);
    res = f_unlink(name);
    if (res)
	{
		printf("fatfs rmdir %s failed\n", name);
		return(0);
	}
	return(1);
}


/// @brief Display FILINFO status of a file by name.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: File name.
///
/// @return  void

MEMSPACE
int fatfs_stat(char *name)
{
    FILINFO info;
    int res;

    printf("fatfs stat [%s]\n", name);
    res = f_stat(name, &info);
    if(res != FR_OK)
    {
		printf("fatfs f_stat %s failed\n",name);
		return(0);
	}
	fatfs_filinfo_list(&info);
	return(1);
}
#endif
