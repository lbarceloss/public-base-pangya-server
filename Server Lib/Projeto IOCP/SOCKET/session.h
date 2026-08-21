
#if defined(_WIN32)
#pragma pack(1)
#endif

#pragma once
#ifndef STDA_SESSION_H
#define STDA_SESSION_H

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include "../UTIL/WinPort.h"
#include <memory.h>

#define SESSION_CONNECT_TIME_CLOCK CLOCK_MONOTONIC_RAW

#endif

#include <ctime>
#include "../UTIL/buffer.h"
#include "../PACKET/packet.h"

#include "../UTIL/message_pool.h"

namespace stdA {
	class threadpool_base;

#define STDA_TIME_LIMIT_NON_AUTHORIZED	10000ul

#define STDA_SESSION_MAX_SEND_QUEUE		(512ul * 1024ul)

	struct ctx_check_packet {
#define CHK_PCKT_INTERVAL_LIMIT 1000
#define CHK_PCKT_COUNT_LIMIT	5
#define CHK_PCKT_NUM_PCKT_MRY	3
		ctx_check_packet() {
			clear();
		};
		void clear() { memset(this, 0, sizeof(ctx_check_packet)); };
#if defined(_WIN32)
		LARGE_INTEGER gettick() {
			LARGE_INTEGER r;

			QueryPerformanceCounter(&r);
#elif defined(__linux__)
		timespec gettick() {
			timespec r;

			clock_gettime(CLOCK_MONOTONIC_RAW, &r);
#endif

			return r;
		};
		bool checkPacketId(unsigned short _packet_id) {

#if defined(_WIN32)
			#define DIFF_TICK(a, b, c) (INT64)(((INT64)((a.QuadPart) - (b.QuadPart)) * 1000000 / (c.QuadPart)) / 1000)

			LARGE_INTEGER frequency;
			QueryPerformanceFrequency(&frequency);
#elif defined(__linux__)
			#define TIMESPEC_TO_NANO_UI64(_timespec) (uint64_t)((uint64_t)(_timespec).tv_sec * (uint64_t)1000000000 + (uint64_t)(_timespec).tv_nsec)
			#define DIFF_TICK(a, b, c) (int64_t)(((int64_t)(TIMESPEC_TO_NANO_UI64((a)) - TIMESPEC_TO_NANO_UI64((b))) / TIMESPEC_TO_NANO_UI64((c))) / 1000000)

			timespec frequency;
			clock_getres(CLOCK_MONOTONIC_RAW, &frequency);
#endif

			auto tick = gettick();

			for (auto i = 0u; i < CHK_PCKT_NUM_PCKT_MRY; ++i) {
				if (ctx[i].packet_id == _packet_id) {
					if (DIFF_TICK(tick, ctx[i].tick, frequency) <= CHK_PCKT_INTERVAL_LIMIT) {

						last_index = i;

						return true;
					}else {
						ctx[i].tick = tick;
						ctx[i].count = 0;

						return false;
					}
				}else if ((i + 1) == CHK_PCKT_NUM_PCKT_MRY) {

					std::rotate(ctx, ctx + 1, ctx + CHK_PCKT_NUM_PCKT_MRY);

					ctx[CHK_PCKT_NUM_PCKT_MRY - 1].packet_id = _packet_id;
					ctx[CHK_PCKT_NUM_PCKT_MRY - 1].tick = tick;
					ctx[CHK_PCKT_NUM_PCKT_MRY - 1].count = 0;
				}
			}

			return false;
		};
		uint32_t incrementCount() {
			return ctx[last_index].count++;
		};
		void clearLast() {
			ctx[last_index].clear();
		};
	protected:
		struct stCtx {
			void clear() { memset(this, 0, sizeof(stCtx)); };

#if defined(_WIN32)
			LARGE_INTEGER tick;
#elif defined(__linux__)
			timespec tick;
#endif

			uint32_t count;
			unsigned short packet_id;
		};
		stCtx ctx[CHK_PCKT_NUM_PCKT_MRY];

	private:
		unsigned char last_index;
	};

	struct stUseCtx {
		stUseCtx() {

#if defined(_WIN32)
			InitializeCriticalSection(&m_cs);
#elif defined(__linux__)
			INIT_PTHREAD_MUTEXATTR_RECURSIVE;
			INIT_PTHREAD_MUTEX_RECURSIVE(&m_cs);
			DESTROY_PTHREAD_MUTEXATTR_RECURSIVE;
#endif

			clear();
		};
		~stUseCtx() {

			clear();

#if defined(_WIN32)
			DeleteCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_destroy(&m_cs);
#endif
		}
		void clear() {

#if defined(_WIN32)
			EnterCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_lock(&m_cs);
#endif

			m_active = 0l;
			m_quit = false;

#if defined(_WIN32)
			LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_unlock(&m_cs);
#endif
		};
		bool isQuit() {

			auto quit = false;

#if defined(_WIN32)
			EnterCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_lock(&m_cs);
#endif

			quit = m_quit;

#if defined(_WIN32)
			LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_unlock(&m_cs);
#endif

			return quit;
		};
		int32_t usa() {

			auto spin = 0l;

#if defined(_WIN32)
			EnterCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_lock(&m_cs);
#endif

			spin = ++m_active;

#if defined(_WIN32)
			LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_unlock(&m_cs);
#endif

			return spin;
		};
		bool devolve() {

			auto spin = 0l;
			auto quit = false;

#if defined(_WIN32)
			EnterCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_lock(&m_cs);
#endif

			spin = --m_active;
			quit = m_quit;

#if defined(_WIN32)
			LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_unlock(&m_cs);
#endif

			return spin <= 0 && quit;
		};

		bool checkCanQuit() {

			auto can = false;

#if defined(_WIN32)
			EnterCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_lock(&m_cs);
#endif

			if (m_active <= 0)
				can = true;
			else
				m_quit = true;

#if defined(_WIN32)
			LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_unlock(&m_cs);
#endif

			return can;
		};
	protected:
#if defined(_WIN32)
		CRITICAL_SECTION m_cs;
#elif defined(__linux__)
		pthread_mutex_t m_cs;
#endif

		int32_t m_active;
		bool m_quit;
	};

    class session {
		public:
			class buff_ctx {
				public:
					buff_ctx();
					~buff_ctx();

					void init();
					void destroy();

					void clear();

					void lock();
					void unlock();

					bool isWritable();
					bool readyToWrite();

					bool isSendable();
					bool readyToSend();

					bool isSetedToSend();
					bool isSetedToWrite();
					bool isSetedToPartial();
					bool isSetedToSendOrPartialSend();

					void setWrite();
					void setSend();
					void setPartialSend();

					void releaseWrite();
					void releaseSend();
					void releasePartial();
					void releaseSendAndPartialSend();

					int64_t increseRequestSendCount();
					int64_t decreaseRequestSendCount();

					size_t tryWriteOut(const void* _data, size_t _size, bool _raw);

					bool enqueueOut(const void* _data, size_t _size, bool _raw);

					bool pumpOut();

					DWORD getOutOperation();

					size_t getOutPendingBytes();

					void clearSendPosted();

					void clearOut();

					bool isSendInFlight();

				public:
					Buffer buff;

				protected:

					struct out_chunk {
						out_chunk*     next;
						unsigned char* data;
						size_t         size;
						size_t         off;
						bool           raw;
					};

					out_chunk* out_head;
					out_chunk* out_tail;
					size_t     out_bytes;
					bool       out_raw;
					bool       send_posted;

#if defined(_WIN32)
					CRITICAL_SECTION cs;

					CONDITION_VARIABLE cv_send;
					CONDITION_VARIABLE cv_write;
#elif defined(__linux__)
					pthread_mutex_t cs;

					pthread_cond_t cv_send;
					pthread_cond_t cv_write;
#endif

					int64_t request_send_count;

					unsigned char state_send : 1, : 0;
					unsigned char state_write : 1, : 0;
					unsigned char state_wr_send : 1, : 0;
			};

        public:
			session(threadpool_base& _threadpool);
			session(threadpool_base& _threadpool, SOCKET _sock, SOCKADDR_IN _addr, unsigned char _key);
            virtual ~session();

			virtual bool clear();
			const char* getIP();

			void lock();
			void unlock();

			void lockSync();
			void unlockSync();

			void requestSendBuffer(void* _buff, size_t _size, bool _raw = false);
			void requestRecvBuffer();

			void setRecv();
			void releaseRecv();

			void setSend();
			void setSendPartial();
			void releaseSend();

			bool isConnected();
			bool isCreated();

			int getConnectTime();

			packet* getPacketS();
			void setPacketS(packet *_packet);
			packet* getPacketR();
			void setPacketR(packet *_packet);

			int32_t usa();
			bool devolve();
			bool isQuit();

			bool getState();
			void setState(bool _state);

			void setConnected(bool _connected);
			void setConnectedToSend(bool _connected_to_send);

			virtual unsigned char getStateLogged() = 0;

			virtual uint32_t getUID() = 0;
			virtual uint32_t getCapability() = 0;
			virtual char* getNickname() = 0;
			virtual char* getID() = 0;

		private:
			void make_ip();

			bool isConnectedToSend();

		public:
			std::clock_t m_time_start;
			std::clock_t m_tick;
			std::clock_t m_tick_bot;

			ctx_check_packet m_check_packet;

			unsigned char m_is_authorized;

			bool m_connection_timeout;

            SOCKET m_sock;
			SOCKADDR_IN m_addr;
            unsigned char m_key;

			char m_ip[32];
			bool m_ip_maked;

			uint32_t m_oid;

			buff_ctx m_buff_s;
			buff_ctx m_buff_r;

			threadpool_base& m_threadpool;

			packet *m_packet_s;
			packet *m_packet_r;

		private:
#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
			CRITICAL_SECTION m_cs_lock_other;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
			pthread_mutex_t m_cs_lock_other;
#endif

			int64_t m_request_recv;

			bool m_state;
			bool m_connected;
			bool m_connected_to_send;

			stUseCtx m_use_ctx;
	};
}

#endif
