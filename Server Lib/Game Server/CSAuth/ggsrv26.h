#ifndef _GG_AUTH_SERVER_H_
#define _GG_AUTH_SERVER_H_

#pragma pack(push, ggsrv)
#pragma pack( )

#if !defined(_WIN32) && !defined(_WIN64)
    #include <stdint.h>

	typedef int32_t			INT32, *PINT32;
	typedef uint32_t		UINT32, *PUINT32;
	typedef int64_t			INT64, *PINT64;
	typedef uint64_t		UINT64, *PUINT64;

	typedef intptr_t		INT_PTR;
	typedef uintptr_t		UINT_PTR;
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

#ifndef ERROR_SUCCESS
	#define ERROR_SUCCESS								0
#endif
#define NPGG_INFO_SUCCESS								0

#define NPGG_INFO_ERROR_NOTENOUGHFMEMORY			1
#define ERROR_GGAUTH_FAIL_MEM_ALLOC					1
#define ERROR_GGAUTH_FAIL_LOAD_DLL					2
#define ERROR_GGAUTH_FAIL_GET_PROC					3
#define ERROR_GGAUTH_FAIL_BEFORE_INIT				4
#define ERROR_GGAUTH_FAIL_LOAD_CFG					5
#define ERROR_GGAUTH_INVALID_PARAM					10

#define ERROR_GGAUTH_NO_REPLY							11
#define ERROR_GGAUTH_INVALID_PROTOCOL_VERSION	12
#define ERROR_GGAUTH_INVALID_REPLY					13
#define ERROR_GGAUTH_INVALID_GAMEGUARD_VER		14

#define ERROR_GGAUTH_SAME_CLIENT_DETECTED			15
#define ERROR_GGAUTH_CLIENT_STOPPED					16
#define ERROR_GGAUTH_CLIENT_AUTH_ERROR				17
#define ERROR_GGAUTH_INVALID_PACKET					18
#define ERROR_GGAUTH_INVALID_CRC1					19
#define ERROR_GGAUTH_INVALID_CRC2					20
#define ERROR_GGAUTH_CLIENT_HACK_DETECTED			21

#define ERROR_GGAUTH_SETSTATE_ERROR					50

#define ERROR_GGAUTH_INVALID_GAMEMON_VER			101
#define ERROR_GGAUTH_INVALID_GAMEMON_VER_CODE	102

#define ERROR_GGAUTH_RETRY_QUERY						200

#define NPGG_CHECKUPDATED_VERIFIED			0
#define NPGG_CHECKUPDATED_NOTREADY			1
#define NPGG_CHECKUPDATED_HIGH				2
#define NPGG_CHECKUPDATED_LOW					3

#define	NPLOG_DEBUG	0x00000001
#define	NPLOG_ERROR	0x00000002

#define NPLOG_ENABLE_DEBUG 0x00000001
#define NPLOG_ENABLE_ERROR 0x00000002

typedef struct _GG_AUTH_DATA
{
	UINT32 dwIndex;
	UINT32 dwValue1;
	UINT32 dwValue2;
	UINT32 dwValue3;
} GG_AUTH_DATA, *PGG_AUTH_DATA;

typedef struct _GG_VERSION
{
	UINT32	dwGGVer;
	unsigned short	wYear;
	unsigned short	wMonth;
	unsigned short	wDay;
	unsigned short	wNum;
} GG_VERSION, *PGG_VERSION;

typedef struct _GG_CSAUTH_STATE
{
	UINT32	m_PrtcVersion;
	UINT32	m_GGVersion;
	UINT32	m_UserFlag;
} GG_CSAUTH_STATE, *PGG_CSAUTH_STATE;

typedef struct _GG_AUTH_PROTOCOL *PGG_AUTH_PROTOCOL;

GGAUTHS_API UINT32 __CDECL InitGameguardAuth(char* sGGPath, UINT32 dwNumActive, int useTimer, int useLog);
GGAUTHS_API void  __CDECL CleanupGameguardAuth();
GGAUTHS_API UINT32 __CDECL GGAuthUpdateTimer();
GGAUTHS_API UINT32 __CDECL AddAuthProtocol(char* sDllName);
GGAUTHS_API UINT32 __CDECL SetGGVerLimit(UINT32 nLimitVer);
GGAUTHS_API UINT32 __CDECL SetUpdateCondition(int nTimeLimit, int nCondition);
GGAUTHS_API UINT32 __CDECL CheckCSAuth(bool bCheck);
GGAUTHS_API int	 __CDECL DecryptHackData(char* lpszUserKey, LPVOID lpData, DWORD dwLength);
GGAUTHS_API int ModuleInfo(char* dest, int length);
GGAUTHS_API void NpLog(int mode, char* msg);

typedef struct _GG_UPREPORT
{
	UINT32	dwBefore;
	UINT32	dwNext;
	int		nType;
} GG_UPREPORT, *PGG_UPREPORT;

GGAUTHS_API void GGAuthUpdateCallback(PGG_UPREPORT report);

#define MY_CSAuth2 FALSE

#if MY_CSAuth2 != TRUE

class GGAUTHS_EXPORT CCSAuth2
{
public:

	CCSAuth2();

	~CCSAuth2();

public:
	GG_AUTH_DATA m_AuthQuery;
	GG_AUTH_DATA m_AuthAnswer;

	DWORD m_dwUniqValue1;
	DWORD m_dwUniqValue2;
	CCSAuth2 *m_pNext;

protected:
	bool m_bAuth;
	PGG_AUTH_PROTOCOL m_pProtocol;
	UINT32 m_bPrtcRef;
	UINT32 m_dwUserFlag;
	GG_VERSION m_GGVer;
	GG_AUTH_DATA m_AuthQueryTmp;
	bool m_bAllowOldVersion;

	int m_nSequenceNum;
	DWORD m_dwServerKey;
	DWORD m_dwLastValue4;

	BYTE  m_byLastLoop1;
	BYTE  m_byLastLoop2;
	DWORD  m_dwLoop1AuthArray[0x100];
	DWORD  m_dwLoop2AuthArray[0x100];

public:
	void  Init();
	UINT32 GetAuthQuery();
	UINT32 CheckAuthAnswer();
	UINT32 CheckUserCSAuth(bool bCheck);
	inline void InitCSAuthState(PGG_CSAUTH_STATE m_CSAuthState) { memset(m_CSAuthState, 0, sizeof(GG_CSAUTH_STATE)); };
	UINT32 GetCSAuthState(PGG_CSAUTH_STATE m_CSAuthState);
	UINT32 SetCSAuthState(PGG_CSAUTH_STATE m_CSAuthState);
	UINT32 SetSecretOrder();
	void  Close();
	int	  Info(char* dest, int length);
	int	  CheckUpdated();

	void	AllowOldVersion();
};

#endif

#define NPGG_USER_AUTH_QUERY	0x00000001
#define NPGG_USER_AUTH_ANSWER	0x00000002
#define NPGG_USER_AUTH_INDEX	0x00000010
#define NPGG_USER_AUTH_VALUE1	0x00000020
#define NPGG_USER_AUTH_VALUE2	0x00000040
#define NPGG_USER_AUTH_VALUE3	0x00000080

typedef void*          LPGGAUTH;

#if MY_CSAuth2 == TRUE

GGAUTHS_EXPORT LPGGAUTH __CDECL GGAuthCreateUser();
GGAUTHS_EXPORT UINT32   __CDECL GGAuthDeleteUser(LPGGAUTH pGGAuth);
GGAUTHS_EXPORT UINT32	__CDECL GGAuthInitUser(LPGGAUTH pGGAuth);
GGAUTHS_EXPORT UINT32   __CDECL GGAuthCloseUser(LPGGAUTH pGGAuth);
GGAUTHS_EXPORT UINT32   __CDECL GGAuthGetQuery(LPGGAUTH pGGAuth, PGG_AUTH_DATA pAuthData);
GGAUTHS_EXPORT UINT32   __CDECL GGAuthCheckAnswer(LPGGAUTH pGGAuth, PGG_AUTH_DATA pAuthData);
GGAUTHS_API int      __CDECL GGAuthCheckUpdated(LPGGAUTH pGGAuth);
GGAUTHS_API int      __CDECL GGAuthUserInfo(LPGGAUTH pGGAuth, char* dest, int length);
GGAUTHS_EXPORT UINT32	__CDECL GGAuthGetState(LPGGAUTH pGGAuth, PGG_CSAUTH_STATE pAuthState);
GGAUTHS_EXPORT UINT32	__CDECL	GGAuthSetState(LPGGAUTH pGGAuth, PGG_CSAUTH_STATE pAuthState);
GGAUTHS_EXPORT UINT32	__CDECL	GGAuthSetSecureOrder(LPGGAUTH pGGAuth);
GGAUTHS_EXPORT UINT32   __CDECL GGAuthCheckUserCSAuth(LPGGAUTH pGGAuth, bool bCheck);
GGAUTHS_API UINT32	__CDECL GGAuthAllowOldVersion(LPGGAUTH pGGAuth);
GGAUTHS_EXPORT UINT32	__CDECL GGGetCurrentGGVer();

GGAUTHS_EXPORT UINT32	 __CDECL GGAuthGetUserValue(LPGGAUTH pGGAuth, int type);

#endif

#pragma pack(pop, ggsrv)

#endif
