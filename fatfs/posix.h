/**
 @file fatfs/posix.h

 @brief FatFs to POSIX wrapper.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#ifndef _POSIX_H_
#define _POSIX_H_

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/eeprom.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#undef EDOM
#undef ERANGE

///@brief Maximum number of POSIX file handles.
#define MAX_FILES 16

extern int errno;

extern FILE *__iob[MAX_FILES];

///@brief fileno()
#define fileno(a) stream_to_fileno(a)
///@brief compare two strings, 1 = match, 0 mismatch.
#define modecmp(str, pat) (strcmp(str, pat) == 0 ? 1: 0)

enum POSIX_errno
{
    OK,        /*< 	0   NO ERROR */
    EPERM,     /*< 	1   Operation not permitted */
    ENOENT,    /*< 	2   No such file or directory */
    ESRCH,     /*< 	3   No such process */
    EINTR,     /*< 	4   Interrupted system call */
    EIO,       /*< 	5   I/O error */
    ENXIO,     /*< 	6   No such device or address */
    E2BIG,     /*< 	7   Argument list too long */
    ENOEXEC,   /*< 	8   Exec format error */
    EBADF,     /*< 	9   Bad file number */
    ECHILD,    /*< 	10  No child processes */
    EAGAIN,    /*< 	11  Try again */
    ENOMEM,    /*< 	12  Out of memory */
    EACCES,    /*< 	13  Permission denied */
    EFAULT,    /*< 	14  Bad address */
    ENOTBLK,   /*< 	15  Block device required */
    EBUSY,     /*< 	16  Device or resource busy */
    EEXIST,    /*< 	17  File exists */
    EXDEV,     /*< 	18  Cross-device link */
    ENODEV,    /*< 	19  No such device */
    ENOTDIR,   /*< 	20  Not a directory */
    EISDIR,    /*< 	21  Is a directory */
    EINVAL,    /*< 	22  Invalid argument */
    ENFILE,    /*< 	23  File table overflow */
    EMFILE,    /*< 	24  Too many open files */
    ENOTTY,    /*< 	25  Not a typewriter */
    ETXTBSY,   /*< 	26  Text file busy */
    EFBIG,     /*< 	27  File too large */
    ENOSPC,    /*< 	28  No space left on device */
    ESPIPE,    /*< 	29  Illegal seek */
    EROFS,     /*< 	30  Read-only file system */
    EMLINK,    /*< 	31  Too many links */
    EPIPE,     /*< 	32  Broken pipe */
    EDOM,      /*< 	33  Math argument out of domain of func */
    ERANGE,    /*< 	34  Math result not representable */
    EBADMSG    /*< 	35  Bad Message */
};
///@brief POSIX open modes  - no other combination are allowed.
/// - man page open(2)
/// - Note: The POSIX correct test of O_RDONLY is: (mode & O_ACCMODE) == O_RDONLY.
#define O_ACCMODE  00000003 /*< read, write, read-write modes */
#define O_RDONLY   00000000 /*< Read only */
#define O_WRONLY   00000001 /*< Write only */
#define O_RDWR     00000002 /*< Read/Write */
#define O_CREAT    00000100 /*< Create file only if it does not exist */
#define O_EXCL     00000200 /*< O_CREAT option, Create fails if file exists 
*/
#define O_NOCTTY   00000400 /*< @todo */
#define O_TRUNC    00001000 /*< Truncate if exists */
#define O_APPEND   00002000 /*< All writes are to EOF */
#define O_NONBLOCK 00004000 /*< @todo */
#define O_BINARY   00000004 /*< Binary */
#define O_TEXT     00000004 /*< Text End Of Line translation */


///@brief POSIX File types, see fstat and stat.
#define S_IFMT     0170000  /*< These bits determine file type.  */
#define S_IFDIR    0040000  /*< Directory.  */
#define S_IFCHR    0020000  /*< Character device.  */
#define S_IFBLK    0060000  /*< Block device.  */
#define S_IFREG    0100000  /*< Regular file.  */
#define S_IFIFO    0010000  /*< FIFO.  */
#define S_IFLNK    0120000  /*< Symbolic link.  */
#define S_IFSOCK   0140000  /*< Socket.  */
#define S_IREAD    0400     /*< Read by owner.  */
#define S_IWRITE   0200     /*< Write by owner.  */
#define S_IEXEC    0100     /*< Execute by owner.  */

///@brief POSIX Fiel type test macros.
#define S_ISTYPE(mode, mask)  (((mode) & S_IFMT) == (mask))
#define S_ISDIR(mode)    S_ISTYPE((mode), S_IFDIR)
#define S_ISCHR(mode)    S_ISTYPE((mode), S_IFCHR)
#define S_ISBLK(mode)    S_ISTYPE((mode), S_IFBLK)
#define S_ISREG(mode)    S_ISTYPE((mode), S_IFREG)

//@brief POSIX File permissions, see fstat and stat  
#define S_IRUSR S_IREAD                     /*< Read by owner.  */
#define S_IWUSR S_IWRITE                    /*< Write by owner.  */
#define S_IXUSR S_IEXEC                     /*< Execute by owner.  */
#define S_IRWXU (S_IREAD|S_IWRITE|S_IEXEC)	/*< Read,Write,Execute by owner */

#define S_IRGRP (S_IRUSR >> 3)              /*< Read by group.  */
#define S_IWGRP (S_IWUSR >> 3)              /*< Write by group.  */
#define S_IXGRP (S_IXUSR >> 3)              /*< Execute by group.  */
#define S_IRWXG (S_IRWXU >> 3)				/*< Read,Write,Execute by user */

#define S_IROTH (S_IRGRP >> 3)              /*< Read by others.  */
#define S_IWOTH (S_IWGRP >> 3)              /*< Write by others.  */
#define S_IXOTH (S_IXGRP >> 3)              /*< Execute by others.  */
#define S_IRWXO (S_IRWXG >> 3)				/*< Read,Write,Execute by other */

#define FATFS_R (S_IRUSR | S_IRGRP | S_IROTH)	/*< FatFs Read perms */
#define FATFS_W (S_IWUSR | S_IWGRP | S_IWOTH)	/*< FatFs Write perms */
#define FATFS_X (S_IXUSR | S_IXGRP | S_IXOTH)	/*< FatFs Execute perms */

///@brief Standard POSIX typedefs.
///
/// - Using these makes code portable accross many acrchitectures
typedef uint32_t dev_t;		/*< dev_t for this architecture */
typedef uint32_t ino_t;		/*< ino_t for this architecture */
typedef uint32_t mode_t;    /*< mode_t for this architecture */
typedef uint32_t nlink_t;  	/*< nlink_t for this architecture */ 
typedef uint16_t uid_t;     /*< uid_t for this architecture */
typedef uint16_t gid_t;     /*< gid_t for this architecture */
typedef uint32_t off_t;     /*< off_t for this architecture */
typedef uint32_t blkcnt_t;  /*< blkcnt_t for this architecture */
typedef uint32_t blksize_t; /*< blksize_t for this architecture */
typedef uint32_t time_t;    /*< time_t for this architecture */

///@brief POSIX stat structure
///@see stat()
///@see fstat()
struct stat
{
    dev_t     st_dev;    /*<  ID of device containing file */
    ino_t     st_ino;    /*<  inode number */
    mode_t    st_mode;   /*<  protection */
    nlink_t   st_nlink;  /*<  number of hard links */
    uid_t     st_uid;    /*<  user ID of owner */
    gid_t     st_gid;    /*<  group ID of owner */
    dev_t     st_rdev;   /*<  device ID (if special file) */
    off_t     st_size;   /*<  total size, in bytes */
    blksize_t st_blksize;/*<  blocksize for filesystem I/O */
    blkcnt_t  st_blocks; /*<  number of 512B blocks allocated */
    time_t    st_atime;  /*<  time of last access */
    time_t    st_mtime;  /*<  time of last modification */
    time_t    st_ctime;  /*<  time of last status change */
};

///@brief POSIX lstat()
///@see stat()
#define lstat stat

/* posix.c */
FILE *fileno_to_stream ( int fileno );
int stream_to_fileno ( FILE *stream );
FIL *fileno_to_fatfs ( int fileno );
int fatfs_to_fileno ( FIL *fh );
int new_file_descriptor ( void );
int free_file_descriptor ( int fileno );
int isatty ( int fileno );
void perror ( const char *s );
char *strerror ( int errnum );
char *strerror_r ( int errnum , char *buf , size_t buflen );
int fatfs_to_errno ( FRESULT Result );
int posix_fopen_modes_to_open ( const char *mode );
int fatfs_getc ( FILE *stream );
int fatfs_putc ( char c , FILE *stream );
int open ( const char *pathname , int flags );
FILE *fopen ( const char *path , const char *mode );
int close ( int fileno );
int syncfs ( int fd );
void sync ( void );
int fclose ( FILE *stream );
size_t write ( int fd , const void *buf , size_t count );
size_t fwrite ( const void *ptr , size_t size , size_t nmemb , FILE *stream );
size_t read ( int fd , const void *buf , size_t count );
size_t fread ( void *ptr , size_t size , size_t nmemb , FILE *stream );
size_t lseek ( int fileno , size_t position , int whence );
int fseek ( FILE *stream , long offset , int whence );
size_t ftell ( FILE *stream );
void rewind ( FILE *stream );
int fgetpos ( FILE *stream , size_t *pos );
int fsetpos ( FILE *stream , size_t *pos );
int unlink ( const char *pathname );
int rmdir ( const char *pathname );
time_t fat_time_to_unix ( uint16_t date , uint16_t time );
int ftruncate ( int fd , off_t length );
int truncate ( const char *path , off_t length );
int fstat ( int fd , struct stat *buf );
int stat ( char *name , struct stat *buf );
char *mctime ( time_t timev );
void dump_stat ( struct stat *sp );
int rename ( const char *oldpath , const char *newpath );
int dirname ( char *str );
char *basename ( char *str );
char *baseext ( char *str );

#endif                                            //_POSIX_H_
