/**
 @file vector.c
 @brief Convert between values and byte arrays
 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.
 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details
*/

#include "user_config.h"

#include "vector.h"

///@brief Convert Value into byte array
/// bytes are MSB ... LSB order
///@param B: byte array
///@param index: offset into byte array
///@param size: number of bytes to process
///@param val: Value to convert
///@return void
void V2B_MSB(uint8_t *B, int index, int size, uint32_t val)
{
    int i;
    for(i=size-1;i>=0;--i)
    {
        B[index+i] = val & 0xff;
        val >>= 8;
    }
}
// =============================================
///@brief Convert Value into byte array
/// bytes are LSB ... MSB order
///@param B: byte array
///@param index: offset into byte array
///@param size: number of bytes to process
///@param val: Value to convert
///@return void
void V2B_LSB(uint8_t *B, int index, int size, uint32_t val)
{
    int i;
    for(i=0;i<size;++i)
    {
        B[index+i] = val & 0xff;
        val >>= 8;
    }
}



///@brief Convert a byte array into a value
/// bytes are MSB ... LSB order
///@param B: byte array
///@param index: offset into byte array
///@param size: number of bytes to process
///@return value
uint32_t B2V_MSB(uint8_t *B, int index, int size)
{
    int i;
    uint32_t val = 0;

    for(i=0;i<size;++i)
    {
        val <<= 8;
        val |= (uint8_t) (B[i+index] & 0xff);
    }
        return(val);
}

///@brief Convert a byte array into a value
/// bytes are LSB ... MSB order
///@param B: byte array
///@param index: offset into byte array
///@param size: number of bytes to process
///@return value
uint32_t B2V_LSB(uint8_t *B, int index, int size)
{
    int i;
    uint32_t val = 0;

    for(i=size-1;i>=0;--i)
    {
        val <<= 8;
        val |= (uint8_t) (B[i+index] & 0xff);
    }
        return(val);
}

/// @brief Create a string from data that has no EOS but known size
/// @param[in] *B: source
/// @param[in] index: index offset into source data
/// @param[out] *name: target string
/// @param[in] size: size of string to write - not including EOS
/// @return void
void B2S(uint8_t *B, int index, uint8_t *name, int size)
{
    int i;
    for(i=0;i<size;++i)
        name[i] = B[index+i];
    name[i] = 0;
}
