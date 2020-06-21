/**
 @file hardware/portio_tests.h

 @brief PORTIO diagnostic tests for HP85 disk emulator project for AVR.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

*/

#ifndef _PORTIO_TESTS_H_
#define _PORTIO_TESTS_H_

/* portio_tests.c */
void portio_help( int full );
int portio_tests( int argc __attribute__((unused)), char *argv []);
#endif                                            // #ifndef _PORTIO_TESTS_H_
