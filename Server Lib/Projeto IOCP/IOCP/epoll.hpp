
#pragma once
#ifndef _STDA_IOCP_HPP
#define _STDA_IOCP_HPP

#include <cstdint>
#include <sys/types.h>
#include <sys/epoll.h>
#include <signal.h>

#ifndef INFINITE
	#define INFINITE -1l
#endif

namespace stdA {

	class iocp {
		public:
			explicit iocp(int32_t _flag = 0l);
			virtual ~iocp();

			void postStatus();
			int32_t getStatus(epoll_event* _events, uint32_t _max_events, int32_t _timeout = INFINITE);

			bool associaDeviceToPort(uint32_t _fd, epoll_event& _event);
			bool deleteFD(uint32_t _fd, epoll_event& _event);

			bool isExitEpollEvent(epoll_event& _event);

		protected:
			uint32_t m_epollfd;

			int m_pipefds[2];

	};
}

#endif
