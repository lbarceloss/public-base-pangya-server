#ifndef _GG_AUTH_SERVER_H_
#define _GG_AUTH_SERVER_H_

#include <cstdint>

#ifndef _WIN32
	typedef unsigned short	WORD;
	typedef uint32_t		DWORD;
	typedef void*			LPVOID;
	typedef int            	BOOL;
#endif

#ifdef _EXPORT_DLL
	#define GGAUTHS_API    extern "C" __declspec(dllexport)
	#define GGAUTHS_EXPORT __declspec(dllexport)
	#define __CDECL        __cdecl
#else
  	#define GGAUTHS_API extern "C"

	#define GGAUTHS_EXPORT
	#define __CDECL
#endif

#define	NPGG_INFO_SUCCESS					0
#define	NPGG_INFO_ERROR_NOTENOUGHFMEMORY	1

#define NPGG_CHECKUPDATED_VERIFIED			0
#define NPGG_CHECKUPDATED_NOTREADY			1
#define NPGG_CHECKUPDATED_HIGH				2
#define NPGG_CHECKUPDATED_LOW				3

#define	NPLOG_DEBUG	0x00000001
#define	NPLOG_ERROR	0x00000002

#define NPLOG_ENABLE_DEBUG 0x00000001
#define NPLOG_ENABLE_ERROR 0x00000002

#define	NPGG_USER_AUTH_QUERY	0x00000001
#define	NPGG_USER_AUTH_ANSWER	0x00000002

#define NPGG_USER_AUTH_INDEX	0x00000010
#define NPGG_USER_AUTH_VALUE1	0x00000020
#define NPGG_USER_AUTH_VALUE2	0x00000040
#define NPGG_USER_AUTH_VALUE3	0x00000080

typedef struct _GG_AUTH_DATA
{
	DWORD dwIndex;
	DWORD dwValue1;
	DWORD dwValue2;
	DWORD dwValue3;
} GG_AUTH_DATA, *PGG_AUTH_DATA;

typedef struct _GG_VERSION
{
	DWORD	dwGGVer;
	WORD	wYear;
	WORD	wMonth;
	WORD	wDay;
	WORD	wNum;
} GG_VERSION, *PGG_VERSION;

typedef struct _GG_AUTH_PROTOCOL *PGG_AUTH_PROTOCOL;

GGAUTHS_API DWORD __CDECL InitGameguardAuth(char* sGGPath, DWORD dwNumActive, BOOL useTimer, int useLog);
GGAUTHS_API void  __CDECL CleanupGameguardAuth();
GGAUTHS_API DWORD __CDECL GGAuthUpdateTimer();

GGAUTHS_API DWORD __CDECL AddAuthProtocol(char* sDllName);

GGAUTHS_API void NpLog(int mode, char* msg);

typedef struct _GG_UPREPORT
{
	DWORD	dwBefore;
	DWORD	dwNext;
	int		nType;
} GG_UPREPORT, *PGG_UPREPORT;

GGAUTHS_API void GGAuthUpdateCallback(PGG_UPREPORT report);

GGAUTHS_API int ModuleInfo(char* dest, int length);

class GGAUTHS_EXPORT CCSAuth2
{
public:

	CCSAuth2();

	~CCSAuth2();

protected:
	PGG_AUTH_PROTOCOL m_pProtocol;
	DWORD m_bPrtcRef;
	DWORD m_dwUserFlag;
	GG_VERSION m_GGVer;
	GG_AUTH_DATA m_AuthQueryTmp;

	BOOL m_bNewProtocol;

	BOOL m_bActive;

public:
	GG_AUTH_DATA m_AuthQuery;
	GG_AUTH_DATA m_AuthAnswer;

	void  Init();
	DWORD GetAuthQuery();
	DWORD CheckAuthAnswer();
	void  Close();
	int	  Info(char* dest, int length);
	int	  CheckUpdated();
};

typedef LPVOID          LPGGAUTH;

GGAUTHS_API LPGGAUTH __CDECL GGAuthCreateUser();
GGAUTHS_API DWORD     __CDECL GGAuthDeleteUser(LPGGAUTH pGGAuth);
GGAUTHS_API DWORD	 __CDECL GGAuthInitUser(LPGGAUTH pGGAuth);
GGAUTHS_API DWORD     __CDECL GGAuthCloseUser(LPGGAUTH pGGAuth);
GGAUTHS_API DWORD    __CDECL GGAuthGetQuery(LPGGAUTH pGGAuth, PGG_AUTH_DATA pAuthData);
GGAUTHS_API DWORD    __CDECL GGAuthCheckAnswer(LPGGAUTH pGGAuth, PGG_AUTH_DATA pAuthData);
GGAUTHS_API int      __CDECL GGAuthCheckUpdated(LPGGAUTH pGGAuth);
GGAUTHS_API int      __CDECL GGAuthUserInfo(LPGGAUTH pGGAuth, char* dest, int length);
GGAUTHS_API DWORD	 __CDECL GGAuthGetUserValue(LPGGAUTH pGGAuth, int type);
#endif
