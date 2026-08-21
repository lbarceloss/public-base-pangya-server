
#pragma once
#ifndef _STDA_PKT_TAP_H
#define _STDA_PKT_TAP_H

#if defined(_WIN32)

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <stdint.h>

namespace pkttap {

	enum { TAP_PORT = 9931 };
	enum { TAP_MAGIC = 0x50545950u };
	enum { TAP_VERSION = 1 };

	enum { RING_SIZE = 32u * 1024u * 1024u };
	enum { MAX_PKT = 512u * 1024u };

	enum Dir { DIR_OUT = 0, DIR_IN = 1 };

	enum Src { SRC_AUTH = 1, SRC_LOGIN = 2, SRC_GAME = 3, SRC_MESSAGE = 4, SRC_GGAUTH = 5 };

#pragma pack(push, 1)
	struct Frame {
		uint32_t magic;
		uint16_t ver;
		uint16_t src;
		uint8_t  dir;
		uint8_t  pad;
		uint64_t ts;
		uint32_t uid;
		uint32_t oid;
		uint16_t opcode;
		uint32_t len;
		uint32_t drops;
	};
#pragma pack(pop)

	class Tap {
		public:
			static Tap& getInstance() {
				static Tap s_inst;
				return s_inst;
			}

			void record(uint16_t _src, uint8_t _dir, uint32_t _uid, uint32_t _oid, uint16_t _opcode,
					const void* _data, uint32_t _len) {

				if (m_connected == 0)
					return;

				if (_data == nullptr || _len == 0 || _len > MAX_PKT)
					return;

				Frame f;

				f.magic  = TAP_MAGIC;
				f.ver    = TAP_VERSION;
				f.src    = _src;
				f.dir    = _dir;
				f.pad    = 0;
				f.ts     = nowFileTime();
				f.uid    = _uid;
				f.oid    = _oid;
				f.opcode = _opcode;
				f.len    = _len;
				f.drops  = (uint32_t)m_drops;

				const uint64_t total = (uint64_t)sizeof(Frame) + _len;

				EnterCriticalSection(&m_cs);

				if ((uint64_t)RING_SIZE - (m_head - m_tail) < total) {

					m_drops++;

					LeaveCriticalSection(&m_cs);
					return;
				}

				put(&f, sizeof(Frame));
				put(_data, _len);

				LeaveCriticalSection(&m_cs);
			}

		private:
			Tap() : m_head(0), m_tail(0), m_drops(0), m_connected(0), m_stop(0), m_sock(INVALID_SOCKET),
					m_ring(nullptr), m_thread(nullptr) {

				WSADATA wsa;

				WSAStartup(MAKEWORD(2, 2), &wsa);

				InitializeCriticalSection(&m_cs);

				m_ring = (uint8_t*)VirtualAlloc(nullptr, RING_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

				if (m_ring != nullptr)
					m_thread = CreateThread(nullptr, 0, &Tap::threadEntry, this, 0, nullptr);
			}

			~Tap() {

				InterlockedExchange(&m_stop, 1);

				if (m_thread != nullptr) {
					WaitForSingleObject(m_thread, 2000);
					CloseHandle(m_thread);
				}

				closeSock();

				if (m_ring != nullptr)
					VirtualFree(m_ring, 0, MEM_RELEASE);

				DeleteCriticalSection(&m_cs);
			}

			Tap(const Tap&);
			Tap& operator=(const Tap&);

			static uint64_t nowFileTime() {
				FILETIME ft;
				GetSystemTimeAsFileTime(&ft);
				return ((uint64_t)ft.dwHighDateTime << 32) | (uint64_t)ft.dwLowDateTime;
			}

			void put(const void* _src, size_t _n) {

				const size_t off = (size_t)(m_head & (uint64_t)(RING_SIZE - 1));
				const size_t ate_fim = (size_t)RING_SIZE - off;

				if (_n <= ate_fim)
					memcpy(m_ring + off, _src, _n);
				else {
					memcpy(m_ring + off, _src, ate_fim);
					memcpy(m_ring, (const uint8_t*)_src + ate_fim, _n - ate_fim);
				}

				m_head += _n;
			}

			static DWORD WINAPI threadEntry(LPVOID _p) {
				((Tap*)_p)->threadLoop();
				return 0;
			}

			void threadLoop() {

				uint8_t buf[64 * 1024];

				while (InterlockedCompareExchange(&m_stop, 0, 0) == 0) {

					if (m_sock == INVALID_SOCKET) {

						if (!tryConnect()) {
							Sleep(1000);
							continue;
						}
					}

					size_t n = 0;

					EnterCriticalSection(&m_cs);

					const uint64_t disp = m_head - m_tail;

					if (disp > 0) {

						n = (size_t)((disp < sizeof(buf)) ? disp : sizeof(buf));

						const size_t off = (size_t)(m_tail & (uint64_t)(RING_SIZE - 1));
						const size_t ate_fim = (size_t)RING_SIZE - off;

						if (n <= ate_fim)
							memcpy(buf, m_ring + off, n);
						else {
							memcpy(buf, m_ring + off, ate_fim);
							memcpy(buf + ate_fim, m_ring, n - ate_fim);
						}

						m_tail += n;
					}

					LeaveCriticalSection(&m_cs);

					if (n == 0) {
						Sleep(2);
						continue;
					}

					if (!sendAll(buf, n))
						closeSock();
				}
			}

			bool tryConnect() {

				SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

				if (s == INVALID_SOCKET)
					return false;

				sockaddr_in sa;

				memset(&sa, 0, sizeof(sa));

				sa.sin_family = AF_INET;
				sa.sin_port = htons((u_short)TAP_PORT);
				sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

				if (connect(s, (sockaddr*)&sa, sizeof(sa)) != 0) {
					closesocket(s);
					return false;
				}

				BOOL nodelay = TRUE;
				setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char*)&nodelay, sizeof(nodelay));

				m_sock = s;

				InterlockedExchange(&m_connected, 1);

				return true;
			}

			void closeSock() {

				InterlockedExchange(&m_connected, 0);

				if (m_sock != INVALID_SOCKET) {
					closesocket(m_sock);
					m_sock = INVALID_SOCKET;
				}

				EnterCriticalSection(&m_cs);
				m_tail = m_head;
				LeaveCriticalSection(&m_cs);
			}

			bool sendAll(const uint8_t* _buf, size_t _n) {

				size_t enviado = 0;

				while (enviado < _n) {

					const int r = send(m_sock, (const char*)_buf + enviado, (int)(_n - enviado), 0);

					if (r <= 0)
						return false;

					enviado += (size_t)r;
				}

				return true;
			}

		private:
			CRITICAL_SECTION m_cs;

			uint8_t* m_ring;
			uint64_t m_head;
			uint64_t m_tail;

			volatile LONG m_drops;
			volatile LONG m_connected;
			volatile LONG m_stop;

			SOCKET m_sock;
			HANDLE m_thread;
	};

	inline void record(uint16_t _src, uint8_t _dir, uint32_t _uid, uint32_t _oid, uint16_t _opcode,
			const void* _data, uint32_t _len) {

		Tap::getInstance().record(_src, _dir, _uid, _oid, _opcode, _data, _len);
	}
}

#endif

#endif
