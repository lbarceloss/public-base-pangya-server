
#pragma once
#ifndef _STDA_PANGYA_CLIENT_ST_H
#define _STDA_PANGYA_CLIENT_ST_H

#include "../../Projeto IOCP/TYPE/pangya_st.h"

namespace stdA {

    struct client_info  {
        void clear() { memset(this, 0, sizeof(client_info)); };

		char m_client_version[20];
		unsigned long m_packet_version;

		unsigned long m_uid;
		unsigned long m_oid;
		unsigned long m_guid;
		unsigned long m_luid;
		unsigned long m_muid;
		unsigned long m_ruid;
		unsigned long m_cuid;
		unsigned long m_cap;
		unsigned long m_TTL;
		char m_nickname[22];
		char m_id[22];
		char m_pass[32];
		unsigned char m_level;
		char m_keys[2][8];
		char m_web_key[8];
		unsigned char m_TRWK;

		char link_gacha[1024];
		char link_guild[1024];
		char link_point[1024];
		char link_entrance[1024];
		char link_weblink[3][1024];

		long volatile m_RCLOP;
		long volatile m_RC;
    };

	struct server_list {
		void clear() {
			if (a_servers != nullptr)
				delete[] a_servers;

			memset(this, 0, sizeof(server_list));
		};
		size_t num_servers;
		ServerInfo *a_servers;
	};

	struct ChannelInfo {
		void clear() {
			memset(this, 0, sizeof(ChannelInfo));
		};
		char name[64];
		short max_user;
		short curr_user;
		unsigned char id;
		long flag;
		long flag2;
		long min_level_allow;
		long max_level_allow;
	};

	struct canal_list {
		void clear() {
			if (a_canais != nullptr)
				delete[] a_canais;

			memset(this, 0, sizeof(canal_list));
		};
		size_t num_canais;
		ChannelInfo *a_canais;
	};

}

#endif !_STDA_PANGYA_CLIENT_ST_H
