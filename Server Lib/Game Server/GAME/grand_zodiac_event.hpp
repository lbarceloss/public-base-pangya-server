
#pragma once
#ifndef _STDA_GRAND_ZODIAC_EVENT_HPP
#define _STDA_GRAND_ZODIAC_EVENT_HPP

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

#include "../../Projeto IOCP/TYPE/singleton.h"
#include "../TYPE/grand_zodiac_type.hpp"

#include <vector>

namespace stdA {
	class GrandZodiacEvent {

		public:
			GrandZodiacEvent();
			virtual ~GrandZodiacEvent();

			 void load();

			 bool isLoad();

			 bool checkTimeToMakeRoom();

			 bool messageSended();

			 void setSendedMessage();

			 range_time* getInterval();

		protected:
			 void initialize();

			 void clear();

		private:
			 std::vector< range_time > m_rt;

			 bool m_load;

			SYSTEMTIME m_st;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif
	};

	typedef Singleton< GrandZodiacEvent > sGrandZodiacEvent;
}

#endif
