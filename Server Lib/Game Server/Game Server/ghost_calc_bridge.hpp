
#pragma once

#if defined(_WIN32)

#include <WinSock2.h>
#include <windows.h>
#include <cstdint>
#include <string>
#include <map>

#include "ghostcalc.h"

namespace ghostcalc {

	struct ReaderData {
		double dist = 0, alt = 0, vento = 0, dir = 0, terreno = 0;
		double spin = 0, curva = 0, slope = 0;
		char   club[16] = { 0 };
		char   ps[24] = { 0 };
		uint64_t ts_ms = 0;
	};

	class GhostCalcBridge {
	public:
		static GhostCalcBridge& getInstance();

		bool start(unsigned short _port = 13360);
		void stop();

		bool get(const std::string& _login, ReaderData& _out, uint32_t _max_age_ms = 5000);

		bool calculate(const GhostCalcInput& _in, GhostCalcOutput& _out);

		bool isCalcLoaded() const { return m_calc != nullptr; }

		bool sendCommand(const std::string& _login, const std::string& _cmd);

		bool armWind(const std::string& _login, bool _on) {
			return sendCommand(_login, _on ? "WIND_ON" : "WIND_OFF");
		}

	private:
		GhostCalcBridge() = default;
		~GhostCalcBridge();
		GhostCalcBridge(const GhostCalcBridge&) = delete;
		GhostCalcBridge& operator=(const GhostCalcBridge&) = delete;

		bool loadCalcDll();
		void listenLoop();
		void handleLine(const std::string& _line, SOCKET _sock);
		static DWORD WINAPI threadProc(LPVOID _arg);

		typedef int (*FnCalculate)(const GhostCalcInput*, GhostCalcOutput*);
		typedef int (*FnInit)();

		HMODULE      m_calc_dll = nullptr;
		FnCalculate  m_calc = nullptr;

		SOCKET       m_listen = INVALID_SOCKET;
		HANDLE       m_thread = nullptr;
		volatile LONG m_running = 0;
		unsigned short m_port = 13360;

		CRITICAL_SECTION m_cs;
		bool          m_cs_init = false;
		std::map<std::string, ReaderData> m_data;
		std::map<std::string, SOCKET> m_login_sock;
	};

}

#define sGhostCalcBridge ghostcalc::GhostCalcBridge::getInstance()

#endif
