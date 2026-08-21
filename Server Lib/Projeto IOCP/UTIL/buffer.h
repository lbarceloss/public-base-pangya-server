
#pragma once
#ifndef _STDA_BUFFER_H
#define _STDA_BUFFER_H

#define MAX_BUFFER_SIZE 0x4000

#if defined(_WIN32)
#include <Windows.h>
#include <WinSock2.h>
#elif defined(__linux__)
#include "WinPort.h"
#include <sys/epoll.h>
#endif

#include "../TYPE/stdAType.h"

namespace stdA {
	class session;

    class Buffer
#if defined(_WIN32)
		: public OVERLAPPED
#elif defined(__linux__)
		: public epoll_event
#endif
	{
        public:
			Buffer();
			Buffer(int seq_mode);

			Buffer( void* buffer, size_t size, int seq_mode);
            ~Buffer();

			void clear();
			void init(int seq_mode);

			void init( void* buffer, size_t size, int seq_mode);

			void reset();

			int addSize(size_t size);
			int write(void* buffer, size_t size);
			int read(void* buffer, size_t size);

			int peek(void* buffer, size_t size);
			void consume(size_t size);

			size_t checkSize(size_t size);

			const unsigned char* getBuffer();
			size_t getSize();
			size_t getUsed();

			WSABUF *getWSABufToRead();
			WSABUF *getWSABufToSend();

			DWORD getOperation();
			void setOperation(DWORD _operation);

			uint32_t getSequence();

			bool isOrder();

			WSABUF& getWSABUF();

		protected:
			void setOverlapped();

		protected:
			DWORD m_operation;

        private:
            size_t m_index_w;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs_wr;
#elif defined(__linux__)
			pthread_mutex_t m_cs_wr;
#endif

		private:
			uint32_t m_sequence;
			int m_seq_mode;
			LPWSABUF m_mwsab;
			unsigned char m_buffer[MAX_BUFFER_SIZE];

		protected:
			static volatile uint32_t static_sequence;
    };
}

#endif
