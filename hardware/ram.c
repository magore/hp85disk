/**
 @file hardware/ram.c

 @brief Memory Utilities and safe free for AVR.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2020 Mike Gore, Inc. All rights reserved.

*/

#include "user_config.h"
/// @brief calloc may be aliased to safecalloc
#undef calloc
/// @brief free may be aliased to safefree
#undef free
/// @brief malloc may be aliased to safecalloc
#undef malloc

/// @brief Return top of heap
///
/// - avr-libc dependent code.
///
/// @return top of heap
/// @see malloc().
size_t heaptop()
{
	volatile size_t heap_end;

	// I looked at the malloc source code to figure these all out
	// if malloc_heap_end and breakval are 0 then we use the stack bootom plus a margin

	if( (size_t) __malloc_heap_end )
		heap_end = (size_t) __malloc_heap_end;
	else if(__brkval)
        heap_end = (size_t) __brkval;
    else
        heap_end = (size_t) SP - (size_t) __malloc_margin;

	return(heap_end);

}
/// @brief Return AVR Free memory for Malloc.
///
/// - avr-libc dependent code.
///
/// @return free memory in bytes.
/// @see malloc().
size_t freeRam ()
{
    size_t total;
	size_t heap_end;

	heap_end = heaptop();

	total = (unsigned long) SP - (unsigned long) heap_end;

    return ( total );
}


/// @brief  Display AVR free memory of each type>
///
/// - avr-libc dependent code.
/// - Stack, BSS, Data, Heap, Malloc.
///
/// @return  void
void PrintFree()
{
	// See https://www.nongnu.org/avr-libc/user-manual/malloc.html
    // static initial values for __malloc_heap_start and end

	size_t ram;
    size_t heap_end;

	// I looked at the malloc source code to figure these all out

    ram = freeRam();
	heap_end = heaptop();

#ifdef AVR
    printf("Free Ram:        %5u\n", (unsigned int) ram);

    printf("  Stack Top:     %5u, End: %5u\n", (unsigned int*) &__stack, (unsigned int) SP);
    printf("  Stack Used:    %5u\n", (unsigned int*) &__stack - (unsigned int*) SP);

	printf("  Heap  Start:   %5u, End: %5u\n",
		(unsigned int) __malloc_heap_start, (unsigned int)heap_end);

    printf("  BSS   Start:   %5u, End: %5u\n",
        (unsigned int)&__bss_start, (unsigned int)&__bss_end);

    printf("  Data  Start:   %5u, End: %5u\n",
        (unsigned int)&__data_start, (unsigned int)&__data_end);
#endif
#ifdef ESP8266
    printf("Free Ram:        %8lu\n", (unsigned int) ram);

    printf("  Stack Top:     %8lu, End: %8lu\n", (unsigned int*) &__stack, (unsigned int) SP);
    printf("  Stack Used:    %8lu\n", (unsigned int*) &__stack - (unsigned int*) SP);

	printf("  Heap  Start:   %8lu, End: %8lu\n",
		(unsigned int) __malloc_heap_start, (unsigned int)heap_end);

    printf("  BSS   Start:   %8lu, End: %8lu\n",
        (unsigned int)&__bss_start, (unsigned int)&__bss_end);

    printf("  Data  Start:   %8lu, End: %8lu\n",
        (unsigned int)&__data_start, (unsigned int)&__data_end);
#endif

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
        printf("safemalloc(%d) failed!\n", (int) size);
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
    size_t heap_end;

    if(p == NULL)
        return;

	heap_end = heaptop();

    if( ((size_t) p >= (size_t) __malloc_heap_start) && ((size_t) p <= heap_end) )
    {
        free(p);
        return;
    }
    printf("safefree: FREE ERROR start(%lu), end(%lu)\n", (size_t) p, (size_t) heap_end);
    PrintFree();
}
