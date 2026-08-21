
#pragma once
#ifndef _STDA_CLIENT_INFO_H
#define _STDA_CLIENT_INFO_H

#include "pangya_client_st.h"
#include "../../Projeto IOCP/TIMER/timer.h"

namespace stdA {
	class ClientInfo : public client_info {
		public:
			ClientInfo();
			~ClientInfo();

			void clear();

		public:
			timer *m_timer_ttl;
			timer *m_timer_msg_lobby;

			ServerInfo m_login_server;
			server_list m_list_servers;
			canal_list m_list_canais;

			chat_macro_user m_chat_macro;

			unsigned char m_channel;
			unsigned char m_lobby;
	};
}

#endif
