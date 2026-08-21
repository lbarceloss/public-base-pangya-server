
#pragma once
#ifndef _STDA_ROOM_GRAND_PRIX_HPP
#define _STDA_ROOM_GRAND_PRIX_HPP

#if defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

#include "room.h"

namespace stdA {

	class RoomGrandPrix : public room {
		public:
			struct RoomGrandPrixInstanciaCtx {
				public:
					enum eSTATE : unsigned char {
						GOOD,
						DESTROYING,
						DESTROYED,
					};

				public:
					RoomGrandPrixInstanciaCtx(RoomGrandPrix* _rgp, eSTATE _state)
						: m_rgp(_rgp), m_state(_state) {
					};

				public:
					RoomGrandPrix* m_rgp;
					eSTATE m_state;
			};

			struct CriticalSectionInstancia {
				CriticalSectionInstancia() : m_state(false), m_lock(false) {

					init();

				};
				~CriticalSectionInstancia() {

					if (m_state)
#if defined(_WIN32)
						DeleteCriticalSection(&m_cs);
#elif defined(__linux__)
						pthread_mutex_destroy(&m_cs);
#endif
				};
				void init() {

					if (!m_state) {
#if defined(_WIN32)
						InitializeCriticalSection(&m_cs);
#elif defined(__linux__)
						INIT_PTHREAD_MUTEXATTR_RECURSIVE;
						INIT_PTHREAD_MUTEX_RECURSIVE(&m_cs);
						DESTROY_PTHREAD_MUTEXATTR_RECURSIVE;
#endif
					}

					m_state = true;
				};
				void lock() {

					if (!m_state)
						init();

#if defined(_WIN32)
					EnterCriticalSection(&m_cs);
#elif defined(__linux__)
					pthread_mutex_lock(&m_cs);
#endif

					m_lock = true;

				};
				void unlock() {

					if (!m_lock)
						return;

					m_lock = false;

#if defined(_WIN32)
					LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
					pthread_mutex_unlock(&m_cs);
#endif
				};
#if defined(_WIN32)
				CRITICAL_SECTION m_cs;
#elif defined(__linux__)
				pthread_mutex_t m_cs;
#endif
				bool m_state;
				bool m_lock;
			};

		public:
			RoomGrandPrix(unsigned char _channel_owner, RoomInfoEx _ri, IFF::GrandPrixData& _gp);
			virtual ~RoomGrandPrix();

			virtual bool isAllReady() override;

			virtual void requestChangePlayerItemRoom(player& _session, ChangePlayerItemRoom& _cpir) override;

			virtual bool requestStartGame(player& _session, packet *_packet) override;

			bool startGame();

			static void initFirstInstance();

		protected:
			static int _count_down_to_start(void* _arg1, void* _arg2);

			virtual void count_down_to_start(int64_t _sec_to_start);

		protected:
			std::vector< IFF::GrandPrixRankReward > reward;

			IFF::GrandPrixData m_gp;

			timer *m_count_down;

		private:

			static void push_instancia(RoomGrandPrix* _rgp);
			static void pop_instancia(RoomGrandPrix* _rgp);

			static void set_instancia_state(RoomGrandPrix* _rgp, RoomGrandPrixInstanciaCtx::eSTATE _state);

			static int get_instancia_index(RoomGrandPrix* _rgp);

			static bool instancia_valid(RoomGrandPrix* _rgp);

			typedef Singleton< CriticalSectionInstancia > m_cs_instancia;
			typedef Singleton< std::vector< RoomGrandPrixInstanciaCtx > > m_instancias;
	};
}

#endif
