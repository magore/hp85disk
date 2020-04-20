/**
  @file stringsup.h

 @brief Various string and character functions

 @par Copyright &copy; 2016 Mike Gore, GPL License
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

#include <string.h>
#include "stringsup.h"

// =============================================
// Character functions
// =============================================

// =============================================
/// @brief test if a character is a digit
/// @param[in] c: character
/// @return true or false
MEMSPACE
int
WEAK_ATR
isdigit(int c)
{
    if(c >= '0' && c <= '9')
        return(1);
    return(0);
}


// =============================================
///@brief Is a character upper case
///
/// @param[in] c: character.
///
/// @return 1 of upper case, else 0
MEMSPACE
int
WEAK_ATR
isupper(int c)
{
    if(c >= 'A' && c <= 'Z')
        return(1);
    return(0);
}


// =============================================
//@brief Is a character lower case
///
/// @param[in] c: character.
///
/// @return 1 of lower case, else 0
MEMSPACE
int
WEAK_ATR
islower(int c)
{
    if(c >= 'a' && c <= 'z')
        return(1);
    return(0);
}


// =============================================
///@brief Convert character to lower case, only if it is upper case
///
/// @param[in] c: character.
///
/// @return character or lowercase value or character
MEMSPACE
int
WEAK_ATR
tolower(int c)
{
    if(isupper(c))
        return(c - 'A' + 'a');
    return(c);
}


// =============================================
///@brief Convert character to upper case, only if it is lower case
///
/// @param[in] c: character.
///
/// @return character or upper case value or character
MEMSPACE
int
WEAK_ATR
toupper(int c)
{
    if(islower(c))
        return(c - 'a' + 'A');
    return(c);
}


/// @brief find a character in a string of maximum size
/// @param[in] str: string
/// @param[in] c: character
/// @param[in] size: string length to search
/// @return string length
MEMSPACE
void *memchr(const void *str, int c, size_t size)
{
    const uint8_t *ptr = str;
    while(size--)
    {
        if (*ptr++ == (uint8_t) c)
            return (void *) (ptr - 1);
    }
    return NULL;
}


// =============================================
// String functions
// =============================================
// =============================================
/// @brief String Length
/// @param[in] str: string
/// @return string length
MEMSPACE
size_t
WEAK_ATR
strlen(const char *str)
{
    int len=0;
// String length
    while(*str++)
        ++len;
    return(len);
}


/// @brief copy a string
/// @param[in] dest: destination string
/// @param[in] src: source string
/// @return destination string
MEMSPACE
WEAK_ATR char *
strcpy(char *dest, const char *src)
{
    char *ptr = dest;
    while(*src)
    {
        *ptr++ = *src++;
    }
    *ptr ++ = 0;
    return (ptr);
}


/// @brief copy a string of at most N characters
/// @param[in] dest: destination string
/// @param[in] src: source string
/// @param[in] size: maximum destination size
/// @return destination string
MEMSPACE
WEAK_ATR
char * strncpy(char *dest, const char *src, size_t size)
{
    char *ptr = dest;
    while(*src && size)
    {
        *ptr++ = *src++;
        size--;
    }
    while(size--)
        *ptr++ = 0;
    return (dest);
}


/// @brief Append string
/// @param[in] dest: string
/// @param[in] src: string
/// @return string length
MEMSPACE
WEAK_ATR
char * strcat(char *dest, const char *src)
{
    char *ptr = dest;
    while(*ptr)
        ++ptr;
    strcpy(ptr,src);
    return(dest);
}


/// @brief Append string of at most N bytes from src
/// @param[in] dest: string
/// @param[in] src: string
/// @return string length
MEMSPACE
WEAK_ATR
char * strncat(char *dest, const char *src, size_t max)
{
    char *ptr = dest;
    while(*ptr)
        ++ptr;
    strncpy(ptr,src,max);
    return(dest);
}


// =============================================
/// @brief Reverse a string in place
///  Example: abcdef -> fedcba
/// @param[in] str: string
/// @return string length
MEMSPACE
void
WEAK_ATR
reverse(char *str)
{
    char temp;
    int i;
    int len = strlen(str);
// Reverse
// We only exchange up to half way
    for (i = 0; i < (len >> 1); i++)
    {
        temp = str[len - i - 1];
        str[len - i - 1] = str[i];
        str[i] = temp;
    }
}


// =============================================
/// @brief UPPERCASE a string
/// @param[in] str: string
/// @return void
MEMSPACE
void
WEAK_ATR
strupper(char *str)
{
    while(*str)
    {
        *str = toupper(*str);
        ++str;
    }
}


// =============================================
// String Matching
// =============================================
///@brief Compare two strings
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return 0 on match, < 0 implies str sorts before pat, > 0 implies str sorts after pat
MEMSPACE
int
WEAK_ATR
strcmp(const char *str, const char *pat)
{
    int ret = 0;
    int c1,c2;
    while (1)
    {
        c1 = *str++;
        c2 = *pat++;
        if ( (ret = c1 - c2) != 0 || c2 == 0)
            break;
    }
    return(ret);
}


// =============================================
///@brief Compare two strings maximum len bytes in size
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///@param[in] len: maximum string length for compare
///@return 0 on match, < 0 implies str sorts before pat, > 0 implies str sorts after pat
MEMSPACE
int
WEAK_ATR
strncmp(const char *str, const char *pat, size_t len)
{
    int ret = 0;
    int c1,c2;
    while (len--)
    {
        c1 = *str++;
        c2 = *pat++;
        if ( (ret = c1 - c2) != 0 || c2 == 0)
            break;
    }
    return(ret);
}


// =============================================
///@brief Compare two strings without case
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return 0 on match, < 0 implies str sorts before pat, > 0 implies str sorts after pat
MEMSPACE
int
WEAK_ATR
strcasecmp(const char *str, const char *pat)
{
    int ret = 0;
    int c1,c2;
    while (1)
    {
        c1 = toupper(*str++);
        c2 = toupper(*pat++);
        if ( (ret = c1 - c2) != 0 || c2 == 0)
            break;
    }
    return(ret);
}


///@brief Compare two strings without case maximum len bytes in size
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///@param[in] len: maximum string length for compare
///@return 0 on match, < 0 implies str sorts before pat, > 0 implies str sorts after pat
MEMSPACE
int
WEAK_ATR
strncasecmp(const char *str, const char *pat, size_t len)
{
    int ret = 0;
    int c1,c2;
    while (len--)
    {
        c1 = toupper(*str++);
        c2 = toupper(*pat++);
        if ( (ret = c1 - c2) != 0 || c2 == 0)
            break;
    }
    return(ret);
}


// =============================================
// String memory allocation functions
// =============================================
// =============================================
///@brief Allocate space for string with maximum size.
///
/// - Copies tring into allocated space limited to maximum size.
///
///@param[in] str: user string.
///@param[in] len: maximum string length.
///
///@return pointer to alocated string.

MEMSPACE
char *strnalloc(char *str, int len)
{
    char *ptr;

    if(!str)
        return(NULL);
    ptr = safecalloc(len+1,1);
    if(!ptr)
        return(ptr);
    strncpy(ptr,str,len);
    return(ptr);

}


// =============================================
///@brief Allocate space for string.
///
/// - Copies tring into allocated space.
///
///@param[in] str: user string.
///
///@return pointer to alocated string.
///@return NULL on out of memory.
MEMSPACE
char *stralloc(char *str)
{
    char *ptr;
    int len;

    if(!str)
        return(str);;
    len  = strlen(str);
    ptr = safecalloc(len+1,1);
    if(!ptr)
        return(ptr);
    strcpy(ptr,str);
    return(ptr);
}
