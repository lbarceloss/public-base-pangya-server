
#pragma once
#ifndef _STDA_GGAUTH_HPP

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include "../../Projeto IOCP/UTIL/event.hpp"
#include "../../Projeto IOCP/THREAD POOL/thread.h"
#endif

#include <cstdint>

#define MY_GG_SRV_LIB 1

#if INTPTR_MAX == INT64_MAX
	#if MY_GG_SRV_LIB == 1
		#include "../../GGSrvLib26-1/GGSrvLib26-1/GGSrvLib26-1.h"
	#endif
#elif INTPTR_MAX == INT32_MAX
	#if MY_GG_SRV_LIB == 1
		#include "../../GGSrvLib26-1/GGSrvLib26-1/GGSrvLib26-1.h"
	#else
		#include "ggsrv26.h"
	#endif
#else
#error Unknown pointer size or missing size macros!
#endif

#include "../../Projeto IOCP/TYPE/singleton.h"

namespace stdA {

#if MY_CSAuth2 == TRUE

	class CCSAuth2 {
		public:

			CCSAuth2();

			~CCSAuth2();

		public:
			GG_AUTH_DATA m_AuthQuery;
			GG_AUTH_DATA m_AuthAnswer;

		protected:
			bool m_bAuth;
			LPGGAUTH m_auth;

		public:
			void  Init();
			UINT32 GetAuthQuery();
			UINT32 CheckAuthAnswer();
			UINT32 CheckUserCSAuth(bool bCheck);
			inline void InitCSAuthState(PGG_CSAUTH_STATE m_CSAuthState) { memset(m_CSAuthState, 0, sizeof(GG_CSAUTH_STATE)); };
			void  Close();
			int	  Info(char* dest, int length);
			int	  CheckUpdated();

			void	AllowOldVersion();
	};

#endif

#if INTPTR_MAX == INT64_MAX && MY_GG_SRV_LIB == 0
#define GGAUTHS_API
#define	NPLOG_DEBUG	0x00000001
#define	NPLOG_ERROR	0x00000002

	typedef void*          LPGGAUTH;

	typedef struct _GG_UPREPORT
	{
		UINT32	dwBefore;
		UINT32	dwNext;
		int		nType;
	} GG_UPREPORT, *PGG_UPREPORT;

	typedef struct _GG_AUTH_DATA
	{
		UINT32 dwIndex;
		UINT32 dwValue1;
		UINT32 dwValue2;
		UINT32 dwValue3;
	} GG_AUTH_DATA, *PGG_AUTH_DATA;

	typedef struct _GG_CSAUTH_STATE
	{
		UINT32	m_PrtcVersion;
		UINT32	m_GGVersion;
		UINT32	m_UserFlag;
	} GG_CSAUTH_STATE, *PGG_CSAUTH_STATE;

	class CCSAuth2 {
	public:

		CCSAuth2();

		~CCSAuth2();

	public:
		GG_AUTH_DATA m_AuthQuery;
		GG_AUTH_DATA m_AuthAnswer;

	protected:
		bool m_bAuth;

		uint32_t m_socket_id;

	public:
		void  Init(uint32_t _socket_id);
		UINT32 GetAuthQuery();
		UINT32 CheckAuthAnswer();
		UINT32 CheckUserCSAuth(bool bCheck);
		inline void InitCSAuthState(PGG_CSAUTH_STATE m_CSAuthState);
		void  Close();
		int	  Info(char* dest, int length);
		int	  CheckUpdated();

		void	AllowOldVersion();
	};
#endif

	class GGAuth {
		public:
			GGAuth(uint32_t _numActiveSession = 1000u);
			~GGAuth();

		protected:
#if defined(_WIN32)
			static DWORD WINAPI updateTimerProc(LPVOID lpParameter);
#elif defined(__linux__)
			static void* updateTimerProc(void* lpParameter);
#endif

		private:
			bool m_state;

#if defined(_WIN32)
			HANDLE m_quit_update_timer;
			HANDLE m_thread_update_timer;
#elif defined(__linux__)
			Event *m_quit_update_timer;
			thread *m_thread_update_timer;
#endif

			const uint32_t m_time_sleep = (5000 * 60);
	};

	typedef Singleton< GGAuth > SGGAuth;
}

#endif
