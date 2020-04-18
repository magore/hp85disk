#ifndef _VECTOR_H
#define _VECTOR_H

/* vector.c */
void V2B_MSB ( uint8_t *B , int index , int size , uint32_t val );
void V2B_LSB ( uint8_t *B , int index , int size , uint32_t val );
uint32_t B2V_MSB ( uint8_t *B , int index , int size );
uint32_t B2V_LSB ( uint8_t *B , int index , int size );
void B2S ( uint8_t *B , int index , uint8_t *name , int size );

#endif

