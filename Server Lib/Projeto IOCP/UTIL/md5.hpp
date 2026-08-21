
#pragma once
#ifndef _STDA_MD5_HPP
#define _STDA_MD5_HPP

#if defined(_WIN32)
#include <Windows.h>
#include <wincrypt.h>
#elif defined(__linux__)

#include <openssl/md5.h>
#endif

#include <string>

namespace stdA {

#define MD5LEN 16

    class md5 {
        public:
            md5();
            ~md5();

            static bool isInit();
            static void init();

            static void destroy();

            static void processData(unsigned char* _data, uint32_t _size);

            static std::string getHash();

            static std::string hash(const unsigned char* _data, uint32_t _size);

        private:
            static bool m_is_init;
            static bool m_is_processed;

#if defined(_WIN32)
            static HCRYPTPROV m_hProv;
            static HCRYPTHASH m_hHash;
#elif defined(__linux__)
            static MD5_CTX m_hHash;
#endif
    };
}

#endif
