
#pragma once
#ifndef _STDA_GM_INFO_HPP
#define _STDA_GM_INFO_HPP

#include <map>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

namespace stdA {
	class GMInfo {
		public:
			GMInfo();
			virtual ~GMInfo();

			void clear();

			void openPlayerWhisper(uint32_t _uid);
			void closePlayerWhisper(uint32_t _uid);

			bool isOpenPlayerWhisper(uint32_t _uid);

			void setGMUID(uint32_t _uid);

		public:
			unsigned char visible : 1, : 0;
			unsigned char whisper : 1, : 0;
			unsigned char channel : 1, : 0;

		private:
			uint32_t m_uid;

			std::map< uint32_t, bool > map_open;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif
	};
}

#endif
