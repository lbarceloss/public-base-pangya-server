
#pragma once
#ifndef _STDA_PACKET_FUNC_MS_HPP
#define _STDA_PACKET_FUNC_MS_HPP

#if defined(_WIN32)
#include <Windows.h>
#include <WinSock2.h>
#elif defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#endif

#include "../../Projeto IOCP/TYPE/stdAType.h"
#include "../../Projeto IOCP/PACKET/packet_func.h"
#include "../../Projeto IOCP/PACKET/packet.h"

#include "../TYPE/pangya_message_st.hpp"

#include <string>
#include <vector>

#include "../SESSION/player.hpp"

#define MAKE_BEGIN_SERVER(_arg1) message_server *ms = reinterpret_cast< message_server* >((_arg1));

#define _MAKE_BEGIN_PACKET_SERVER(_arg1, _arg2) MAKE_BEGIN_SERVER(_arg1) _MAKE_BEGIN_PACKET(_arg2)

#define MAKE_BEGIN_PACKET_AUTH_SERVER(_arg1, _arg2) MAKE_BEGIN_SERVER(_arg1) _MAKE_BEGIN_PACKET_AUTH_SERVER(_arg2)

#ifdef _DEBUG
#define MAKE_BEGIN_PACKET_SERVER(_arg1, _arg2) _MAKE_BEGIN_PACKET_SERVER(_arg1, _arg2) _MSG_BEGIN_PACKET
#else
#define MAKE_BEGIN_PACKET_SERVER(_arg1, _arg2) _MAKE_BEGIN_PACKET_SERVER(_arg1, _arg2)
#endif

namespace stdA {
	class packet_func : public packet_func_base {
		public:

			static int packet012(void* _arg1, void* _arg2);
			static int packet013(void* _arg1, void* _arg2);
			static int packet014(void* _arg1, void* _arg2);
			static int packet016(void* _arg1, void* _arg2);
			static int packet017(void* _arg1, void* _arg2);
			static int packet018(void* _arg1, void* _arg2);
			static int packet019(void* _arg1, void* _arg2);
			static int packet01A(void* _arg1, void* _arg2);
			static int packet01B(void* _arg1, void* _arg2);
			static int packet01C(void* _arg1, void* _arg2);
			static int packet01D(void* _arg1, void* _arg2);
			static int packet01E(void* _arg1, void* _arg2);
			static int packet01F(void* _arg1, void* _arg2);
			static int packet023(void* _arg1, void* _arg2);
			static int packet024(void* _arg1, void* _arg2);
			static int packet025(void* _arg1, void* _arg2);
			static int packet028(void* _arg1, void* _arg2);
			static int packet029(void* _arg1, void* _arg2);
			static int packet02A(void* _arg1, void* _arg2);
			static int packet02B(void* _arg1, void* _arg2);
			static int packet02C(void* _arg1, void* _arg2);
			static int packet02D(void* _arg1, void* _arg2);

			static int packet_svFazNada(void* _arg1, void* _arg2);

			static int packet_as001(void* _arg1, void* _arg2);
			static int packet_as002(void* _arg1, void* _arg2);
			static int packet_as003(void* _arg1, void* _arg2);

			static void friend_broadcast(std::map< uint32_t, player* > _m_player, packet& _p, session *_s, unsigned char _debug);
			static void friend_broadcast(std::map< uint32_t, player* > _m_player, std::vector< packet* > _v_p, session *_s, unsigned char _debug);

			static void session_send(packet& _p, session *_s, unsigned char _debug);
			static void session_send(std::vector< packet* > _v_p, session *_s, unsigned char _debug);
	};
}

#endif
