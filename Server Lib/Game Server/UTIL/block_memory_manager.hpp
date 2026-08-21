
#pragma once
#ifndef _STDA_BLOCK_MEMORY_MANAGER_HPP
#define _STDA_BLOCK_MEMORY_MANAGER_HPP

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

#include <map>

namespace stdA {
	class BlockMemoryManager {
		public:
			struct BlockCtx {
				BlockCtx(uint32_t _ul = 0u) {

#if defined(_WIN32)
					InitializeCriticalSection(&cs);
#elif defined(__linux__)
					INIT_PTHREAD_MUTEXATTR_RECURSIVE;
					INIT_PTHREAD_MUTEX_RECURSIVE(&cs);
					DESTROY_PTHREAD_MUTEXATTR_RECURSIVE;
#endif
				};
				~BlockCtx() {

#if defined(_WIN32)
					DeleteCriticalSection(&cs);
#elif defined(__linux__)
					pthread_mutex_destroy(&cs);
#endif
				};
#if defined(_WIN32)
				CRITICAL_SECTION cs;
#elif defined(__linux__)
				pthread_mutex_t cs;
#endif
			};

		public:
			BlockMemoryManager();
			virtual ~BlockMemoryManager();

			static void blockUID(uint32_t _uid);
			static void unblockUID(uint32_t _uid);

		protected:
			static void clear();

		protected:
			static std::map< uint32_t , BlockCtx > mp_block;
	};
}

#endif
