
#pragma once
#ifndef _STDA_RANK_REGISTRY_MANAGER_HPP
#define _STDA_RANK_REGISTRY_MANAGER_HPP

#if defined(__linux__)
#include <pthread.h>
#include <unistd.h>
#endif

#include "rank_registry.hpp"
#include "rank_character.hpp"

#include "../../Projeto IOCP/PACKET/packet.h"
#include "../SESSION/player.hpp"

#include <fstream>

namespace stdA {

	constexpr uint32_t LIMIT_REGISTRY_FOR_PAGE = 12u;

	typedef std::pair< uint32_t  , int32_t   > FoundPlayer;

	class RankRegistryManager {
		public:
			RankRegistryManager();
			virtual ~RankRegistryManager();

			void load();

			bool isLoad();

			void pageToPacket(packet& _packet, search_dados& _sd);

			void playerPositionToPacket(packet& _packet, player& _session, search_dados& _sd);

			void sendPlayerFullInfo(player& _session, uint32_t _uid );

			void sendPageFoundPlayer(player& _session, FoundPlayer& _fp, search_dados& _sd);

			void searchPlayerByNicknameAndSendPage(player& _session, std::string _nickname, search_dados& _sd);

			void searchPlayerByRankAndSendPage(player& _session, uint32_t _position, search_dados& _sd);

			FoundPlayer searchPlayerByNickname(std::string _nickname, search_dados& _sd);

			FoundPlayer searchPlayerByRank(uint32_t _position, search_dados& _sd);

			void makeLog();

		protected:
			void initialize();

			void clear();

			std::map< eRANK_OVERALL, RankRegistry > getAllOverallInfoFromPlayer(uint32_t _uid);

			std::pair< RankEntryValueRange, bool > getPage(RankEntry::iterator _registrys, unsigned int & _page);

		protected:
			std::ofstream log;

			std::string prex;
			std::string dir;

			void init_log();
			void close_log();

			inline void putLog(std::string _str_log);

		protected:
			RankEntry m_entry;
			RankCharacterEntry m_character_entry;

			bool m_state;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif
	};
}

#endif
