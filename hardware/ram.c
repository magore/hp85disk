/**
 @file hardware/ram.c

 @brief Memory Utilities and safe free for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#include "user_config.h"
/// @brief calloc may be aliased to safecalloc
#undef calloc
/// @brief free may be aliased to safefree
#undef free
/// @brief malloc may be aliased to safecalloc
#undef malloc

/// @brief Return AVR Free memory for Malloc.
///
/// - avr-libc dependent code.
///
/// @return free memory in bytes.
/// @see malloc().
uint16_t freeRam ()
{
    int v;
    extern void * __heap_start;
    extern void * __brkval;
    int ret;
    uint16_t top;

    if(__brkval)
        top = (uint16_t) __brkval;
    else
        top = (uint16_t) & __heap_start;

    ret = (uint16_t) &v - top;
    return ( ret );
}


/// @brief  Display AVR free memory of each type>
///
/// - avr-libc dependent code.
/// - Stack, BSS, Data, Heap, Malloc.
///
/// @return  void
void PrintFree()
{
    extern void * __brkval;
    extern void * __heap_start;
    extern void * __heap_end;
    extern void * __bss_start;
    extern void * __bss_end;
    extern void * __data_start;
    extern void * __data_end;
    extern void * __stack;
    uint16_t ram;

    ram = freeRam();

    printf("Free Ram:%u\n", ram);
    printf("  Stack Top:   %u\n", (unsigned int) &__stack);
    printf("  Stack Free:  %u\n", (unsigned int) &ram - 0);

    printf("  BSS   start: %5u, end: %5u\n",
        (unsigned int)&__bss_start, (unsigned int)&__bss_end);

    printf("  Data  start: %5u, end: %5u\n",
        (unsigned int)&__data_start, (unsigned int)&__data_end);

    printf("  Heap: start: %5u, end: %5u\n",
        (unsigned int)&__heap_start, (unsigned int)&__heap_end);

    printf("  Malloc start %5u  end: %5u\n",
        (unsigned int)__malloc_heap_start, (unsigned int) __malloc_heap_end );

    printf("  __brkval:    %5u\n", (unsigned int) __brkval);

}

/// @brief Safe Alloc -  Display Error message if Calloc fails
///
///  - We check if the pointer was in the heap.
///  - Otherwise it may have been statically defined - display error.
///
/// @return  void.
void *safecalloc(int size, int elements)
{
	void *p = calloc(size, elements);
	if(!p)
	{
		printf("safecalloc(%d,%d) failed!\n", size, elements);
	}
	return(p);
}

/// @brief Safe Malloc -  Display Error message if Malloc fails
///
///  - We check if the pointer was in the heap.
///  - Otherwise it may have been statically defined - display error.
/// @param[in] size:  size
/// @return  void.
void *safemalloc(size_t size)
{
    void *p = calloc(size, 1);
    if(!p)
    {
        printf("safemalloc(%d) failed!\n", size);
    }
    return(p);
}

/// @brief Safe free -  Only free a pointer if it is in malloc memory range.
///
///  - We check if the pointer was in the heap.
///  - Otherwise it may have been statically defined - display error.
///
/// @param[in] p: pointer to free.
///
/// @return  void.
void safefree(void *p)
{
    extern void *__brkval;
    extern void * __heap_start;
    extern void * __heap_end;
    uint16_t top;

    if(p == NULL)
        return;

    if(__brkval)
        top = (uint16_t) __brkval;
    else
        top = (uint16_t) & __heap_start;

    if( ((uint16_t) p >= (uint16_t) &__heap_start) &&
        ((uint16_t) p <= top) )
    {
        free(p);
        return;
    }
    printf("safefree: FREE ERROR (%u), top:(%u)\n", (uint16_t) p, (uint16_t) top);
    PrintFree();
}
