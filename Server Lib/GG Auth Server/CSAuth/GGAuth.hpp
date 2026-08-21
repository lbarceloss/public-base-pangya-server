
#pragma once
#ifndef _STDA_GGAUTH_HPP

#include <Windows.h>

#include <cstdint>

#if INTPTR_MAX == INT64_MAX

#elif INTPTR_MAX == INT32_MAX
#include "ggsrv26.h"
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

#if INTPTR_MAX == INT64_MAX
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

	unsigned long InitGameguardAuth(const char* _path, unsigned long _numActiveSession, bool _check, unsigned long _flag);

	void CleanupGameguardAuth(void);

	unsigned long GGAuthUpdateTimer();

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
		inline void InitCSAuthState(PGG_CSAUTH_STATE m_CSAuthState);
		void  Close();
		int	  Info(char* dest, int length);
		int	  CheckUpdated();

		void	AllowOldVersion();
	};
#endif

	class GGAuth {
		public:
			GGAuth(unsigned long _numActiveSession = 1000ul);
			~GGAuth();

		protected:
			static DWORD WINAPI updateTimerProc(LPVOID lpParameter);

		private:
			bool m_state;

			HANDLE m_quit_update_timer;
			HANDLE m_thread_update_timer;

			const unsigned long m_time_sleep = (5000 * 60);
	};

	typedef Singleton< GGAuth > SGGAuth;
}

#endif
