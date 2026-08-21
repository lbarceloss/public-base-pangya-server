
#pragma once
#ifndef STDA_GUILD_ROOM_MANAGER_HPP
#define _STDA_GUILD_ROOM_MANGER_HPP

#if defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

#include "dupla_manager.hpp"
#include "guild.hpp"

#include <vector>

namespace stdA {

	class GuildRoomManager {
		public:
			enum eGUILD_WIN : unsigned char {
				RED,
				BLUE,
				DRAW,
			};

		public:
			GuildRoomManager();
			~GuildRoomManager();

			Guild* addGuild(Guild::eTEAM _team, uint32_t _uid);
			Guild* addGuild(Guild& _guild);

			void deleteGuild(Guild* _guild);

			uint32_t getNumGuild();

			eGUILD_WIN getGuildWin();

			Guild* findGuildByTeam(Guild::eTEAM _team);
			Guild* findGuildByUID(uint32_t _uid);
			Guild* findGuildByPlayer(player& _session);

			Dupla* findDupla(player& _session);

			void init_duplas();

			int isGoodToStart();

			bool oneGuildRest();

			void update();

			void calcGuildWin();

			void saveGuildsData();

			void initPacketDuplas(packet& _p);

			bool finishHoleDupla(PlayerGameInfo& _pgi, unsigned short _seq_hole);

		protected:
			static void SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg);

		protected:
			std::vector< Guild > v_guilds;

			DuplaManager m_dupla_manager;

			eGUILD_WIN m_guild_win;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif
	};
}
#endif
