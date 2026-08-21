
#pragma once
#ifndef _STDA_SOCKETSERVER_H_
#define _STDA_SOCKETSERVER_H_

#if defined(_WIN32)
#include "wsa.h"
#elif defined(__linux__)
#include "../UTIL/WinPort.h"
#endif

namespace stdA {
    class socketserver
#ifdef _WIN32
        : public wsa
#endif
    {
        public:
            socketserver();
            ~socketserver();

			SOCKET getsocklistener();

    };
}

#endif
