/**
 @file lib/util.c

 @brief Misc. pattern matching and string utilities for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

///@brief Skip white space in a string - tabs and spaces.
///
///@param[in] ptr: input string
///
///@return pointer to first non white space character 
char *skipspaces(char *ptr)
{
    if(!ptr)
        return(ptr);

    while(*ptr == ' ' || *ptr == '\t')
        ++ptr;
    return(ptr);
}

///@brief Skip to first white space in a string - tabs and spaces.
///
///@param[in] ptr: input string
///
///@return pointer to first white space character 

char *nextspace(char *ptr)
{
    if(!ptr)
        return(ptr);

    while(*ptr)
    {
        if(*ptr == ' ' || *ptr == '\t')
            break;
        ++ptr;
    }
    return(ptr);
}

///@brief Skip characters defined in user string.
///
///@param[in] str: string
///@param[in] pat: pattern string
///
///@return pointer to string after skipped characters.

char *skipchars(char *str, char *pat)
{
    char *base;
    if(!str)
        return(str);

    while(*str)
    {
        base = pat;
        while(*base)
        {
            if(*base == *str)
                break;
            ++base;
        }
        if(*base != *str)
            return(str);
        ++str;
    }
    return(str);
}


///@brief Trim White space and control characters from end of string.
///
///@param[in] str: string
///
///@return void
///@warning Overwrites White space and control characters with EOS.
void trim_tail(char *str)
{
    int len = strlen(str);
    while(len--)
    {
        if(str[len] > ' ')
            break;
        str[len] = 0;
    }
}



///@brief Allocate space for string with maximum size.
///
/// - Copies tring into allocated space limited to maximum size.
///
///@param[in] str: user string.
///@param[in] len: maximum string length.
///
///@return pointer to alocated string.

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


///@brief Allocate space for string.
///
/// - Copies tring into allocated space.
///
///@param[in] str: user string.
///
///@return pointer to alocated string.
///@return NULL on out of memory.
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


///@brief Search for token in a string matching user pattern.
///
/// - Skips all non printable ASCII characters before trying match.
///
///@param[in] str: string to search.
///@param[in] pat: pattern to search for.
///
///@return string lenth on match.
///@return 0 on no match.

int token(char *str, char *pat)
{
    int patlen;
    int len;
    char *ptr;

    len = 0;
    ptr = str;
    while(*ptr > ' ' && *ptr <= 0x7e )
    {
        ++len;
        ++ptr;
    }

    if(!len)
        return(0);

    patlen = strlen(pat);

    if(len < patlen)
        return(0);

    if(strncmp(str,pat,patlen) == 0)
        return(len);
    return(0);
}


///@brief Compare two strings.
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return string lenth on match.
///@return 0 on no match.
int MATCH(char *str, char *pat)
{
    int len;
    len = strlen(pat);
    if(strcmp(str,pat) == 0 )
        return(len);
    return(0);
}


///@brief Compare two strings without case.
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return string lenth on match.
///@return 0 on no match.
int MATCHI(char *str, char *pat)
{
    int len;
    len = strlen(pat);
    if(strcasecmp(str,pat) == 0 )
        return(len);
    return(0);
}


///@brief Compare two strings limted to length of pattern.
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return string lenth on match.
///@return 0 on no match.
///@warning Matches sub strings so be caeful.
int MATCH_LEN(char *str, char *pat)
{
    int len;

    if(!str || !pat)
        return(0);
    len = strlen(pat);

    if( len )
    {
        if(strncmp(str,pat,len) == 0 )
            return(len);
    }
    return(0);
}


///@brief Compare two strings without case limted to length of pattern.
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return string lenth on match.
///@return 0 on no match.
///@warning Matches sub strings so be caeful.
int MATCHI_LEN(char *str, char *pat)
{
    int len;

    if(!str || !pat)
        return(0);
    len = strlen(pat);

    if( len )
    {
        if(strncasecmp(str,pat,len) == 0 )
            return(len);
    }
    return(0);
}


///@brief Convert Character into HEX digit.
///
/// @param[in] c: character.
///
/// @return HEX digit [0..F] on success.
/// @return 0xff on fail.
uint8_t hexd( char c )
{
    if( c >= '0' && c <= '9' )
        return(c - '0');
    if( c >= 'a' && c <= 'f')
        return(c - 'a' + 10);
    if( c >= 'A' && c <= 'F')
        return(c - 'A' + 10);
    return(0xff);                                 // error
}


///@brief Convert ASCII hex string to long integer.
///
///@param[in] p: String to convert.
///
///@return long integer result.
///@see strtol() for return overflow and underflow results.
long atoh(const char *p)
{
    return strtol(p, (char **) NULL, 16);
}
