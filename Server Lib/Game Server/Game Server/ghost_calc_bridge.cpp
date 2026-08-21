
#if defined(_WIN32)

#pragma pack(1)

#include "ghost_calc_bridge.hpp"
#include "../../Projeto IOCP/UTIL/message_pool.h"

#include <ws2tcpip.h>
#include <vector>
#include <set>
#include <cstdlib>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

using namespace stdA;

namespace {

	const char* GHOST_CALC_DLL = "GhostCalc.dll";

	void gcLog(const std::string& _msg, bool _console = true) {
		_smp::message_pool::getInstance().push(
			new message("[GhostCalcBridge] " + _msg, _console ? CL_FILE_LOG_AND_CONSOLE : CL_ONLY_FILE_LOG));
	}

	std::vector<std::string> splitPipe(const std::string& _s) {
		std::vector<std::string> out;
		size_t start = 0, pos;
		while ((pos = _s.find('|', start)) != std::string::npos) {
			out.push_back(_s.substr(start, pos - start));
			start = pos + 1;
		}
		out.push_back(_s.substr(start));
		return out;
	}
}

namespace ghostcalc {

GhostCalcBridge& GhostCalcBridge::getInstance() {
	static GhostCalcBridge inst;
	return inst;
}

GhostCalcBridge::~GhostCalcBridge() {
	stop();
	if (m_calc_dll) { FreeLibrary(m_calc_dll); m_calc_dll = nullptr; }
	if (m_cs_init) { DeleteCriticalSection(&m_cs); m_cs_init = false; }
}

bool GhostCalcBridge::loadCalcDll() {
	if (m_calc != nullptr)
		return true;

	m_calc_dll = LoadLibraryA(GHOST_CALC_DLL);
	if (m_calc_dll == nullptr) {
		gcLog(std::string("[Error] nao carregou ") + GHOST_CALC_DLL + " (err=" + std::to_string(GetLastError()) + ")");
		return false;
	}

	FnInit init = (FnInit)GetProcAddress(m_calc_dll, "GhostCalc_Init");
	if (init) init();

	m_calc = (FnCalculate)GetProcAddress(m_calc_dll, "GhostCalc_Calculate");
	if (m_calc == nullptr) {
		gcLog("[Error] nao achou export GhostCalc_Calculate");
		FreeLibrary(m_calc_dll); m_calc_dll = nullptr;
		return false;
	}

	gcLog("[Log] GhostCalc.dll carregada.");
	return true;
}

bool GhostCalcBridge::start(unsigned short _port) {
	if (InterlockedCompareExchange(&m_running, 1, 0) == 1)
		return true;

	if (!m_cs_init) { InitializeCriticalSection(&m_cs); m_cs_init = true; }

	m_port = _port;

	loadCalcDll();

	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	m_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_listen == INVALID_SOCKET) {
		gcLog("[Error] socket() falhou (err=" + std::to_string(WSAGetLastError()) + ")");
		m_running = 0;
		return false;
	}

	BOOL yes = TRUE;
	setsockopt(m_listen, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

	sockaddr_in addr; memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(m_port);

	if (bind(m_listen, (sockaddr*)&addr, sizeof(addr)) != 0) {
		gcLog("[Error] bind() na porta " + std::to_string(m_port) + " falhou (err=" + std::to_string(WSAGetLastError()) + ")");
		closesocket(m_listen); m_listen = INVALID_SOCKET; m_running = 0;
		return false;
	}

	if (listen(m_listen, 16) != 0) {
		gcLog("[Error] listen() falhou (err=" + std::to_string(WSAGetLastError()) + ")");
		closesocket(m_listen); m_listen = INVALID_SOCKET; m_running = 0;
		return false;
	}

	m_thread = CreateThread(nullptr, 0, &GhostCalcBridge::threadProc, this, 0, nullptr);
	if (m_thread == nullptr) {
		gcLog("[Error] CreateThread falhou");
		closesocket(m_listen); m_listen = INVALID_SOCKET; m_running = 0;
		return false;
	}

	gcLog("[Log] listener ON na porta " + std::to_string(m_port) + ".");
	return true;
}

void GhostCalcBridge::stop() {
	if (InterlockedCompareExchange(&m_running, 0, 1) == 0)
		return;

	if (m_listen != INVALID_SOCKET) { closesocket(m_listen); m_listen = INVALID_SOCKET; }
	if (m_thread != nullptr) {
		WaitForSingleObject(m_thread, 2000);
		CloseHandle(m_thread); m_thread = nullptr;
	}
}

DWORD WINAPI GhostCalcBridge::threadProc(LPVOID _arg) {
	reinterpret_cast<GhostCalcBridge*>(_arg)->listenLoop();
	return 0;
}

void GhostCalcBridge::listenLoop() {

	std::map<SOCKET, std::string> bufs;

	while (m_running) {
		fd_set rfds; FD_ZERO(&rfds);
		FD_SET(m_listen, &rfds);
		SOCKET maxfd = m_listen;
		for (auto& kv : bufs) { FD_SET(kv.first, &rfds); if (kv.first > maxfd) maxfd = kv.first; }

		timeval tv; tv.tv_sec = 1; tv.tv_usec = 0;
		int n = select((int)maxfd + 1, &rfds, nullptr, nullptr, &tv);
		if (n <= 0) continue;
		if (!m_running) break;

		if (FD_ISSET(m_listen, &rfds)) {
			SOCKET c = accept(m_listen, nullptr, nullptr);
			if (c != INVALID_SOCKET) bufs[c] = std::string();
		}

		std::vector<SOCKET> dead;
		for (auto& kv : bufs) {
			SOCKET c = kv.first;
			if (!FD_ISSET(c, &rfds)) continue;
			char tmp[1024];
			int r = recv(c, tmp, sizeof(tmp), 0);
			if (r <= 0) { dead.push_back(c); continue; }
			kv.second.append(tmp, r);
			size_t pos;
			while ((pos = kv.second.find('\n')) != std::string::npos) {
				std::string line = kv.second.substr(0, pos);
				kv.second.erase(0, pos + 1);
				if (!line.empty() && line.back() == '\r') line.pop_back();
				if (!line.empty()) handleLine(line, c);
			}
		}
		for (SOCKET c : dead) {

				EnterCriticalSection(&m_cs);
				for (auto it = m_login_sock.begin(); it != m_login_sock.end(); ) {
					if (it->second == c) it = m_login_sock.erase(it); else ++it;
				}
				LeaveCriticalSection(&m_cs);
				closesocket(c); bufs.erase(c);
			}
	}

	for (auto& kv : bufs) closesocket(kv.first);
}

void GhostCalcBridge::handleLine(const std::string& _line, SOCKET _sock) {

	auto p = splitPipe(_line);
	if (p.size() < 12 || p[0] != "GC") return;

	ReaderData d;
	d.dist    = atof(p[2].c_str());
	d.alt     = atof(p[3].c_str());
	d.vento   = atof(p[4].c_str());
	d.dir     = atof(p[5].c_str());
	d.terreno = atof(p[6].c_str());
	d.spin    = atof(p[7].c_str());
	d.curva   = atof(p[8].c_str());
	d.slope   = atof(p[9].c_str());
	strncpy_s(d.club, sizeof(d.club), p[10].c_str(), _TRUNCATE);
	strncpy_s(d.ps,   sizeof(d.ps),   p[11].c_str(), _TRUNCATE);
	d.ts_ms = GetTickCount64();

	EnterCriticalSection(&m_cs);
	m_data[p[1]] = d;
	m_login_sock[p[1]] = _sock;
	LeaveCriticalSection(&m_cs);

	static std::set<std::string> seen;
	if (seen.insert(p[1]).second)
		gcLog("[Log] 1o push recebido de login='" + p[1] + "' (dist=" + std::to_string(d.dist)
				+ ", club=" + std::string(d.club) + ")");
}

bool GhostCalcBridge::get(const std::string& _login, ReaderData& _out, uint32_t _max_age_ms) {
	bool ok = false;
	EnterCriticalSection(&m_cs);
	auto it = m_data.find(_login);
	if (it != m_data.end()) {
		uint64_t now = GetTickCount64();
		if (now - it->second.ts_ms <= _max_age_ms) {
			_out = it->second;
			ok = true;
		}
	}
	LeaveCriticalSection(&m_cs);
	return ok;
}

bool GhostCalcBridge::sendCommand(const std::string& _login, const std::string& _cmd) {

	SOCKET s = INVALID_SOCKET;
	EnterCriticalSection(&m_cs);
	auto it = m_login_sock.find(_login);
	if (it != m_login_sock.end()) s = it->second;
	LeaveCriticalSection(&m_cs);

	if (s == INVALID_SOCKET) {
		gcLog("[Log] sendCommand: login='" + _login + "' sem conexao do reader (cmd=" + _cmd + ")");
		return false;
	}

	std::string line = _cmd + "\n";
	int sent = send(s, line.c_str(), (int)line.size(), 0);
	if (sent == SOCKET_ERROR) {
		gcLog("[Log] sendCommand: send falhou pra login='" + _login + "' (err=" + std::to_string(WSAGetLastError()) + ")");
		return false;
	}
	return true;
}

bool GhostCalcBridge::calculate(const GhostCalcInput& _in, GhostCalcOutput& _out) {
	if (m_calc == nullptr && !loadCalcDll())
		return false;
	return m_calc(&_in, &_out) != 0;
}

}

#endif
