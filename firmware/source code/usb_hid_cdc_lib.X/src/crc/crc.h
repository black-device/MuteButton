#ifndef _CRC_H_
#define _CRC_H_

#include <stdint.h>

/*
 * Obtiene el CRC16 de un bloque de datos, pasándole el CRC inicial
 */
void get_crc16(uint16_t* crc, const void* data, uint16_t size);



/*
 * Obtiene el CRC16 de un bloque de datos
 */
uint16_t get_new_crc16(const void* data, uint16_t size);


/**
 * Calculates the CRC32
 * @param crc
 * @param data
 * @param size
 */
void get_crc32(uint32_t* crc, const void* data, uint16_t size);


/**
 * Obtains the CRC32 value of the block of data
 * @param data
 * @param size
 * @return 
 */
uint32_t get_new_crc32(const void* data, uint16_t size);


#endif