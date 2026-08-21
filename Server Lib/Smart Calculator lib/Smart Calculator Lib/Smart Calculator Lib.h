
#if defined(_WIN32)
    #ifdef SMARTCALCULATORLIB_EXPORTS
        #define SMARTCALCULATORLIB_API __declspec(dllexport)
    #else
        #define SMARTCALCULATORLIB_API __declspec(dllimport)
    #endif
#elif defined(__linux__)
    #ifdef SMARTCALCULATORLIB_EXPORTS

        #define SMARTCALCULATORLIB_API __attribute__((visibility("default")))
    #else

        #define SMARTCALCULATORLIB_API __attribute__((visibility("hidden")))
        #define SMARTCALCULATORLIB_API_EXP __attribute__((visibility("default")))
    #endif
#endif

#if defined(_WIN32)
#pragma pack(push, 1)
#endif

#include <string>
#include "../../Projeto IOCP/TYPE/smart_calculator_type.hpp"
#include "../../Projeto IOCP/TYPE/smart_calculator_player_base.hpp"

using namespace stdA;

extern "C" SMARTCALCULATORLIB_API stContext* makePlayerContext(uint32_t _uid, eTYPE_CALCULATOR_CMD _type);

extern "C" SMARTCALCULATORLIB_API stContext* getPlayerContext(uint32_t _uid, eTYPE_CALCULATOR_CMD _type);

extern "C" SMARTCALCULATORLIB_API void removeAllPlayerContext(uint32_t _uid);

extern "C" SMARTCALCULATORLIB_API int initSmartCalculatorLib(const uint32_t _server_id);

extern "C" SMARTCALCULATORLIB_API void sendCommandServer(const char* _cmd);

#ifdef SMARTCALCULATORLIB_EXPORTS
extern "C" SMARTCALCULATORLIB_API void scLog(const char* _log, const eTYPE_LOG _type);
extern "C" SMARTCALCULATORLIB_API void responseCallBack(const uint32_t _id, const eTYPE_CALCULATOR_CMD _type, const char* _response, const eTYPE_RESPONSE _server);
#else
extern "C" SMARTCALCULATORLIB_API_EXP void scLog(const char* _log, const eTYPE_LOG _type);
extern "C" SMARTCALCULATORLIB_API_EXP void responseCallBack(const uint32_t _id, const eTYPE_CALCULATOR_CMD _type, const char* _response, const eTYPE_RESPONSE _server);
#endif

#if defined(_WIN32)
#pragma pack(pop, 1)
#endif
