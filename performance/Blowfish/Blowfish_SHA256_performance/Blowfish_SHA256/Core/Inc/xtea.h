// xtea.h
#ifndef XTEA_H
#define XTEA_H

#include <stdint.h>
void xtea_enc(uint32_t dest[2], const uint32_t v[2], const uint32_t k[4]);
void xtea_dec(uint32_t dest[2], const uint32_t v[2], const uint32_t k[4]);

#endif
