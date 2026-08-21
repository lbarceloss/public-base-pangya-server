
#pragma once
#ifndef _STDA_HMAC_SHA256_HPP
#define _STDA_HMAC_SHA256_HPP

#include <cstdint>
#include <cstring>

namespace stdA {

	class sha256 {
		public:
			static const size_t DIGEST_SIZE = 32;
			static const size_t BLOCK_SIZE  = 64;

			sha256() { init(); }

			void init() {
				m_len = 0;
				m_tot = 0;
				m_h[0] = 0x6a09e667; m_h[1] = 0xbb67ae85;
				m_h[2] = 0x3c6ef372; m_h[3] = 0xa54ff53a;
				m_h[4] = 0x510e527f; m_h[5] = 0x9b05688c;
				m_h[6] = 0x1f83d9ab; m_h[7] = 0x5be0cd19;
			}

			void update(const uint8_t* data, size_t len) {
				while (len > 0) {
					size_t take = BLOCK_SIZE - m_len;
					if (take > len) take = len;
					memcpy(m_buf + m_len, data, take);
					m_len += take;
					data  += take;
					len   -= take;
					m_tot += take;
					if (m_len == BLOCK_SIZE) {
						transform(m_buf);
						m_len = 0;
					}
				}
			}

			void final_(uint8_t out[DIGEST_SIZE]) {
				uint64_t bits = m_tot * 8;
				m_buf[m_len++] = 0x80;
				if (m_len > BLOCK_SIZE - 8) {
					memset(m_buf + m_len, 0, BLOCK_SIZE - m_len);
					transform(m_buf);
					m_len = 0;
				}
				memset(m_buf + m_len, 0, BLOCK_SIZE - 8 - m_len);
				for (int i = 0; i < 8; ++i)
					m_buf[BLOCK_SIZE - 1 - i] = (uint8_t)(bits >> (i * 8));
				transform(m_buf);
				for (int i = 0; i < 8; ++i) {
					out[i*4+0] = (uint8_t)(m_h[i] >> 24);
					out[i*4+1] = (uint8_t)(m_h[i] >> 16);
					out[i*4+2] = (uint8_t)(m_h[i] >>  8);
					out[i*4+3] = (uint8_t)(m_h[i]);
				}
			}

			static void hash(const uint8_t* data, size_t len, uint8_t out[DIGEST_SIZE]) {
				sha256 h;
				h.update(data, len);
				h.final_(out);
			}

		private:
			static uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }

			void transform(const uint8_t block[BLOCK_SIZE]) {
				static const uint32_t K[64] = {
					0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
					0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
					0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
					0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
					0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
					0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
					0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
					0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
				};
				uint32_t w[64];
				for (int i = 0; i < 16; ++i)
					w[i] = ((uint32_t)block[i*4] << 24) | ((uint32_t)block[i*4+1] << 16)
					     | ((uint32_t)block[i*4+2] << 8) |  (uint32_t)block[i*4+3];
				for (int i = 16; i < 64; ++i) {
					uint32_t s0 = rotr(w[i-15], 7) ^ rotr(w[i-15], 18) ^ (w[i-15] >> 3);
					uint32_t s1 = rotr(w[i-2], 17) ^ rotr(w[i-2], 19)  ^ (w[i-2] >> 10);
					w[i] = w[i-16] + s0 + w[i-7] + s1;
				}
				uint32_t a=m_h[0],b=m_h[1],c=m_h[2],d=m_h[3],e=m_h[4],f=m_h[5],g=m_h[6],h=m_h[7];
				for (int i = 0; i < 64; ++i) {
					uint32_t S1 = rotr(e,6) ^ rotr(e,11) ^ rotr(e,25);
					uint32_t ch = (e & f) ^ ((~e) & g);
					uint32_t t1 = h + S1 + ch + K[i] + w[i];
					uint32_t S0 = rotr(a,2) ^ rotr(a,13) ^ rotr(a,22);
					uint32_t mj = (a & b) ^ (a & c) ^ (b & c);
					uint32_t t2 = S0 + mj;
					h=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
				}
				m_h[0]+=a; m_h[1]+=b; m_h[2]+=c; m_h[3]+=d;
				m_h[4]+=e; m_h[5]+=f; m_h[6]+=g; m_h[7]+=h;
			}

			uint32_t m_h[8];
			uint8_t  m_buf[BLOCK_SIZE];
			size_t   m_len;
			uint64_t m_tot;
	};

	class hmac_sha256 {
		public:
			static const size_t DIGEST_SIZE = sha256::DIGEST_SIZE;

			static void compute(const uint8_t* key, size_t key_len,
			                    const uint8_t* msg, size_t msg_len,
			                    uint8_t out[DIGEST_SIZE]) {
				uint8_t k[sha256::BLOCK_SIZE];
				if (key_len > sha256::BLOCK_SIZE) {
					sha256::hash(key, key_len, k);
					memset(k + sha256::DIGEST_SIZE, 0, sha256::BLOCK_SIZE - sha256::DIGEST_SIZE);
				} else {
					memcpy(k, key, key_len);
					memset(k + key_len, 0, sha256::BLOCK_SIZE - key_len);
				}
				uint8_t ipad[sha256::BLOCK_SIZE], opad[sha256::BLOCK_SIZE];
				for (size_t i = 0; i < sha256::BLOCK_SIZE; ++i) {
					ipad[i] = k[i] ^ 0x36;
					opad[i] = k[i] ^ 0x5c;
				}
				uint8_t inner[DIGEST_SIZE];
				sha256 h1;
				h1.update(ipad, sha256::BLOCK_SIZE);
				h1.update(msg, msg_len);
				h1.final_(inner);
				sha256 h2;
				h2.update(opad, sha256::BLOCK_SIZE);
				h2.update(inner, DIGEST_SIZE);
				h2.final_(out);
			}

			static bool verify(const uint8_t a[DIGEST_SIZE], const uint8_t b[DIGEST_SIZE]) {
				uint8_t d = 0;
				for (size_t i = 0; i < DIGEST_SIZE; ++i) d |= (uint8_t)(a[i] ^ b[i]);
				return d == 0;
			}
	};
}

#endif
