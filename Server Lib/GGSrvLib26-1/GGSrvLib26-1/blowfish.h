
#pragma once
#ifndef _BLOWFISH_H
#define _BLOWFISH_H

#define MAXKEYBYTES 56

#define big_endian 1

#define N				16
#define KEYBYTES		8

#if defined(_WIN32)

#pragma pack(push, ggsrv)
#pragma pack( )
#endif

#include <cstdint>

struct _BLOWFISH_CTX {
	uint32_t P[N + 2];
	uint32_t S[4][256];
};

#if defined(_WIN32)

#pragma pack(pop, ggsrv)
#endif

uint32_t F(_BLOWFISH_CTX* _ctx, uint32_t x);

void Blowfish_Init(_BLOWFISH_CTX* _ctx, char _key[], int _keybytes);

void Blowfish_Encrypt(_BLOWFISH_CTX* _ctx, uint32_t *_xl, uint32_t *_xr);

void Blowfish_Decrypt(_BLOWFISH_CTX* _ctx, uint32_t *_xl, uint32_t *_xr);

#endif
