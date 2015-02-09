/**
 @file fatfs/fatfs_tests.c

 @brief FatFs utilities and tests for HP85 disk emulator project.
 @par Credit: part of FatFs avr example project (C)ChaN, 2013.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.
 @par Copyright &copy; 2013 ChaN.

*/

#include "hardware/hardware.h"
#include "fatfs_tests.h"


/// @brief FatFS size lookup table
static  const BYTE ft[] = {0,12,16,32};

/// @brief  List files under a specified directory
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] ptr: pathname of directory to list
///
/// @see fatfs_filinfo_list().
/// @return  void.

void fatfs_ls(char *ptr)
{
    long p1;
    UINT s1, s2;
    int res;
    FILINFO *fno;
    DIR dirs;                                     /* Directory object */
    FATFS *fs;

    fno = fatfs_alloc_finfo(0);
    if(fno == NULL)
    {
        return;
    }

    while(*ptr == ' ' || *ptr == '\t')
        ++ptr;

    myprintf("Listing:[%s]\n",ptr);

    res = f_opendir(&dirs, ptr);
    if (res) { put_rc(res); return; }
    p1 = s1 = s2 = 0;
    while(1)
    {
        res = f_readdir(&dirs, fno);
        if ((res != FR_OK) || !fno->fname[0]) break;
        if (fno->fattrib & AM_DIR)
        {
            s2++;
        }
        else
        {
            s1++; p1 += fno->fsize;
        }
        fatfs_filinfo_list(fno);
    }
    myprintf("%4u File(s),%10lu bytes total\n%4u Dir(s)", s1, p1, s2);
    if (f_getfree(ptr, (DWORD*)&p1, &fs) == FR_OK)
        myprintf(", %10luK bytes free\n", p1 * fs->csize / 2);
}

/// @brief Rename a file
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] oldpath: old name.
/// @param[in] newpath: new name.
///
/// @return  void

void fatfs_rename(const char *oldpath, const char *newpath)
{
/* Rename an object */
    int rc;
    rc = f_rename(oldpath, newpath);
    if(rc)
    {
        put_rc(rc);
    }
}

/// @brief  Display the contents of a file
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: file name.
///
/// @return  void.

void fatfs_cat(char *name)
{
    UINT s1;
    int res;
    FIL fp;
    int8_t i;
	long size =0;
	char *ptr;

    myprintf("Reading[%s]\n", name);
    res = f_open(&fp, name, FA_OPEN_EXISTING | FA_READ);
    if (res)
    {
        myprintf("cat error\n");
        put_rc(res);
        f_close(&fp);
        return;
    }

	ptr = safecalloc(64,1);
	if(!ptr)
	{
		myprintf("Calloc failed!\n");
		f_close(&fp);
		return;
	}
    while(1)
    {
/// @todo FIXME
        if(uart_keyhit(0))
            break;
        res = f_read(&fp, ptr, 64, &s1);
        if(res)
        {
            myprintf("cat read error\n");
            put_rc(res);
            break;
        }
        if (!s1)
		{
            break;
		}
		size += s1;
        for(i=0;i<(int8_t)s1;++i)
            putchar(ptr[i]);
    }
    myprintf("\n");
    f_close(&fp);
	safefree(ptr);
    myprintf("%lu bytes\n", size);
}


/// @brief  Copy a file.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] from: source file.
/// @param[in] to:   destination file.
///
/// @return  void.

void fatfs_copy(char *from,char *to)
{
    UINT s1, s2;
    FIL file1,file2;
    int res;
    long p1;
	char *ptr;
	

    myprintf("Opening %s\n", from);
    res = f_open(&file1, from, FA_OPEN_EXISTING | FA_READ);
    if (res)
    {
        put_rc(res);
        return;
    }
    myprintf("Creating %s\n", to);
    res = f_open(&file2, to, FA_CREATE_ALWAYS | FA_WRITE);
    if (res)
    {
        put_rc(res);
        f_close(&file1);
        return;
    }
	ptr = safecalloc(512,1);
	if(!ptr)
	{
		myprintf("Calloc failed!\n");
        f_close(&file1);
        f_close(&file2);
		return;
	}
    myprintf("\nCopying...\n");
    p1 = 0;
    for (;;)
    {
        res = f_read(&file1, ptr, 512, &s1);
        if (res || s1 == 0) break;                /* error or eof */
        res = f_write(&file2, ptr, s1, &s2);
        p1 += s2;
        myprintf("Copied: %08ld\r", p1);
        if (res || s2 < s1) break;                /* error or disk full */
    }
    if (res)
        put_rc(res);
    myprintf("%lu bytes copied.\n", p1);
	safefree(ptr);
    f_close(&file1);
    f_close(&file2);
}


/// @brief  Create a new file from a user supplied string.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: name of file to create.
/// @param[in] str: string containing file contents.
///
/// @return  void.

void fatfs_create(char *name,char *str)
{
    UINT s1;
    UINT len;
    int res;
    FIL fp;
    len = strlen(str);
    myprintf("Creating [%s]\n", name);
    myprintf("Text[%s]\n", str);
    res = f_open(&fp, name, FA_CREATE_ALWAYS | FA_WRITE);
    if (res)
    {
        myprintf("Create error\n");
        put_rc(res);
        f_close(&fp);
        return;
    }
    res = f_write(&fp, str, (UINT)len, &s1);
    if (res)
    {
        myprintf("Write error\n");
        put_rc(res);
        return;
    }
    if (len != s1)
    {
        myprintf("Write error - wanted(%d) got(%d)\n",s1,len);
        put_rc(res);
        return;
    }
    f_close(&fp);
}


/// @brief  Delete a file by name.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: File name to delete.
///
/// @return  void.

void fatfs_rm(char *name)
{
    myprintf("rm [%s]\n", name);
    put_rc(f_unlink(name));
}


/// @brief  Make a directory.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: Directory name.
///
/// @return  void

void fatfs_mkdir(char *name)
{
    myprintf("mkdir [%s]\n", name);
    put_rc(f_mkdir(name));

}


/// @brief  Delete a directory.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: Directory name.
///
/// @return  void.

void fatfs_rmdir(char *name)
{
    myprintf("rmdir [%s]\n", name);
    put_rc(f_unlink(name));
}

/// @brief Display FILINFO status of a file by name.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: File name.
///
/// @return  void

void fatfs_stat(char *name)
{
    FILINFO info;

    myprintf("stat [%s]\n", name);
    f_stat(name, &info);
    fatfs_filinfo_list(&info);
}

/// @brief  Change directory.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: Directory name.
///
/// @return  void.

void fatfs_cd(char *name)
{
    myprintf("cd [%s]\n", name);
    put_rc(f_chdir(name));
}


/// @brief  Display current working directory.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @return  void.

void fatfs_pwd(void)
{
#if _FS_RPATH >= 2
    int res;
    char str[128];
    res = f_getcwd(str, sizeof(str)-2);
    if (res)
        put_rc(res);
    else
        myprintf("pwd [%s]\n", str);
#endif
}
