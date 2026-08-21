
#pragma once
#ifndef _STDA_SESSION_MANAGER_HPP
#define _STDA_SESSION_MANAGER_HPP

#include "session.h"
#include "../THREAD POOL/threadpool.h"
#include "../TYPE/list_async.h"

#if defined(__linux__)
#include <pthread.h>
#endif

#include <vector>
#include <string>

namespace stdA {
    class session_manager {
        public:
			session_manager(threadpool& _threadpool, uint32_t _max_session);
            virtual ~session_manager();

			virtual void clear();

            virtual session* addSession(SOCKET _sock, SOCKADDR_IN _addr, unsigned char _key);
            virtual bool deleteSession(session* _session);

			virtual uint32_t getNumSessionOnline();

            virtual session* findSessionByOID(uint32_t _oid);
			virtual session* findSessionByUID(uint32_t _uid);
			virtual std::vector< session* > findAllSessionByUID(uint32_t _uid);

			virtual session* findSessionByNickname(std::string& _nickname);

			virtual session* getSessionToDelete(DWORD _dwMilliseconds = INFINITE);

            virtual void checkSessionLive();

            virtual bool isFull();

			virtual uint32_t numSessionConnected();

			static bool isInit();

		protected:
			static uint32_t m_count;
			static bool m_is_init;

        protected:
			virtual void config_init();

            virtual int32_t findSessionFree();

		protected:
			std::vector< session* > m_sessions;
			uint32_t m_max_session;
			uint32_t m_TTL;

			threadpool& m_threadpool;

			list_async<session*> m_session_del;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif
    };
}

#endif
