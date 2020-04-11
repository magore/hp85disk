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
#include "parsing.h"

// =============================================
///@brief print seperator
MEMSPACE
void sep()
{
    printf("==============================\n");
}

// =============================================
///@brief Trim White space and control characters from end of string.
///
///@param[in] str: string
///
///@return void
///@warning Overwrites White space and control characters with EOS.
MEMSPACE
void trim_tail(char *str)
{
	if(!str)
		return;
    int len = strlen(str);
    while(len--)
    {
        if(str[len] > ' ')
            break;
        str[len] = 0;
    }
}

// =============================================
///@brief Skip white space in a string - tabs and spaces.
///
///@param[in] ptr: input string
///
///@return pointer to first non white space character
MEMSPACE
char *skipspaces(char *ptr)
{
    if(!ptr)
        return(ptr);

    while(*ptr == ' ' || *ptr == '\t')
        ++ptr;
    return(ptr);
}

// =============================================
///@brief Skip to first white space in a string - tabs and spaces.
///
///@param[in] ptr: input string
///
///@return pointer to first white space character
MEMSPACE
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

// =============================================
///@brief Skip characters defined in user string.
///
///@param[in] str: string
///@param[in] pat: pattern string
///
///@return pointer to string after skipped characters.
MEMSPACE
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

// =============================================
///@brief Compare two strings.
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return string lenth on match.
///@return 0 on no match.
MEMSPACE
int MATCH(char *str, char *pat)
{
    int len;
    len = strlen(pat);
    if(strcmp(str,pat) == 0 )
        return(len);
    return(0);
}
///@brief Match two strings and compare argument index
/// Display  message if the number of arguments is too few
///@param str: string to test
///@param pat: pattern to match
///@param min: minumum number or arguments
///@param argc: actual number of arguments
///@return 1 on match, 0 on no match or too few arguments
MEMSPACE
int MATCHARGS(char *str, char *pat, int min, int argc)
{
    if(MATCHI(str,pat))
    {
        if(argc >= min)
            return(1);
        else
            printf("%s expected %d arguments only got %d\n", pat, min,argc);
    }
    return(0);
}


// =============================================
///@brief Compare two strings without case.
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return string lenth on match.
///@return 0 on no match.
MEMSPACE
int MATCHI(char *str, char *pat)
{
    int len;
    len = strlen(pat);
    if(strcasecmp(str,pat) == 0 )
        return(len);
    return(0);
}
// =============================================
///@brief Compare two strings limted to length of pattern.
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return string lenth on match.
///@return 0 on no match.
///@warning Matches sub strings so be caeful.
MEMSPACE
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

// =============================================
///@brief Compare two strings without case limted to length of pattern.
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return string lenth on match.
///@return 0 on no match.
///@warning Matches sub strings so be caeful.
MEMSPACE
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

// =============================================
///@brief Split string into arguments stored in argv[]
///   We split source string into arguments
/// Warning: source string is modified!
///   To save memory each gap in the source string is terminated with an EOS
///   This becomes the end of string for each argument returned
/// Warning: Do NOT modify the source string or argument contents while using them
///   You can reassign new pointers to the arguments if you like
///@param[in|out] str: string to break up into arguments
///@param[out] *argv[]: token array
///@param[in] max: maximum argument count
///@return count
MEMSPACE
int split_args(char *str, char *argv[], int max)
{
    int i;
    int count = 0;
    // NULL ?

    for(i=0;i<max;++i)
        argv[i] = NULL; 

    // You may replace argv[0]
    // argv[count++] = "main";

    if(!max)
        return(0);

    if(!str)
        return(0);

    while(*str && count < max)
    {
        str = skipspaces(str);
        if(!*str)
            break;

        // string processing
        if(*str == '"')
        {
            ++str;
            // Save string pointer
            argv[count++] = str;
            while(*str && *str != '"')
                ++str;
            if(*str == '"')
                *str++ = 0;
            continue;
        }

        argv[count++] = str;
        // Find size of token
        while(*str > ' ' && *str <= 0x7e)
            ++str;
        if(!*str)
            break;
        *str  = 0;
        ++str;
    }
    return(count);
}

// =============================================
///@brief return next token
///
/// - Skips all non printable ASCII characters before token
/// - Token returns only printable ASCII
///
///@param[in] str: string to search.
///@param[out] token: token to return
///@param[in] max: maximum token size
///
///@return pointer past token on success .
///@return NULL if no token found
MEMSPACE
char *get_token(char *str, char *token, int max)
{

	*token = 0;

    if(!str || *str == 0)
        return(str);

    // Skip beginning spaces
    str = skipspaces(str);
    // Delete all trailing spaces
    trim_tail(str);

    while(*str > ' ' && max > 0) {

        // String processing
        // A token can be a quoted string
        if(*str == '"')
        {
            ++str;
            // We are pointing at the body of the quoted string now
            while(*str && *str != '"' && max > 0)
            {
                *token++ = *str++;
                --max;
            }
            if(*str == '"')
            {
                ++str;
                *token = 0;
                --max;
                break;
            }
            break;
        }

        // If we have a comma, outside of a string, break
        if(*str == ',' )
            break;

        // copy token
        *token++ = *str++;
        --max;
    }

    // Skip trailing spaces
    str = skipspaces(str);
    // If we had a trailing comma skip it
    if(*str == ',' )
        ++str;

    *token = 0;
    return(str);
}


// =============================================
///@brief Search for token in a string matching user pattern.
///
/// - Skips all non printable ASCII characters before trying match.
///
///@param[in] str: string to search.
///@param[in] pat: pattern to search for.
///
///@return string lenth on match.
///@return 0 on no match.

MEMSPACE
int token(char *str, char *pat)
{
    int patlen;
    int len;
    char *ptr;

    if(!str || *str == 0)
        return(0);

    ptr = skipspaces(str);
    len = 0;
    while(*ptr > ' ' && *ptr <= 0x7e )
    {
        ++len;
        ++ptr;
    }

    if(!len)
        return(0);

    patlen = strlen(pat);

    if(len != patlen)
        return(0);

    if(strncmp(str,pat,patlen) == 0)
        return(len);
    return(0);
}
// =============================================

/// @brief get a number
///
/// - Used only for debugging
/// @param[in] str: string to examine
///
/// @return  value
MEMSPACE
int32_t get_value(char *str)
{ 
    int base;
    int ret;
    char *ptr;
    char *endptr;

    if(!str || *str == 0)
        return(0);

    ptr = skipspaces(str);
    base = 10;

    // convert number base 10, 16, 8 and 2
    if( (ret = MATCHI_LEN(ptr,"0x")) )
    {
        base = 16;
        ptr += ret;
    }
    else if( (ret = MATCHI_LEN(ptr,"0o")) )
    {
        base = 8;
        ptr += ret;
    }
    else if( (ret = MATCHI_LEN(ptr,"0b")) )
    {
        base = 2;
        ptr += ret;
    }
    return(strtol(ptr, (char **)&endptr, base));
}

// =============================================
