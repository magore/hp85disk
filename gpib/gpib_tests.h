/**
 @file gpib/gpib_tests.h

 @brief GPIB diagnostic tests for HP85 disk emulator project for AVR.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

*/


#ifndef _GPIB_TESTS_H_
#define _GPIB_TESTS_H_


/* gpib_tests.c */
void gpib_help ( int full );
int gpib_tests ( int argc , char *argv []);

#endif // #ifndef _GPIB_TESTS_H_

