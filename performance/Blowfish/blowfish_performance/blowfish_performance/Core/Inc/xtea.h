// BLOWFISH3.h
#ifndef BLOWFISH3_H
#define BLOWFISH3_H

#include <stdint.h>
void BLOWFISH3_enc(uint32_t dest[2], const uint32_t v[2], const uint32_t k[4]);
void BLOWFISH3_dec(uint32_t dest[2], const uint32_t v[2], const uint32_t k[4]);

#endif
