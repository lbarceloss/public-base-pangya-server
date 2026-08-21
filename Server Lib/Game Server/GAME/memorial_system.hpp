
#pragma once
#ifndef _STDA_MEMORIAL_SYSTEM_HPP
#define _STDA_MEMORIAL_SYSTEM_HPP

#if defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

#include "../TYPE/memorial_type.hpp"
#include "../SESSION/player.hpp"

#include "../../Projeto IOCP/TYPE/singleton.h"

#include <map>
#include <vector>

#define MEMORIAL_LEVEL_MAX 24

namespace stdA {
	class MemorialSystem {
		public:
			MemorialSystem();
			virtual ~MemorialSystem();

			  bool isLoad();

			  void load();

			  ctx_coin* findCoin(uint32_t _typeid);

			  std::vector< ctx_coin_item_ex > drawCoin(player& _session, ctx_coin& _ctx_c);

		protected:
			  void initialize();

			  void clear();

			  uint32_t calculeMemorialLevel(uint32_t _achievement_pontos);

		private:
			  std::map< uint32_t, ctx_coin > m_coin;
			  std::map< uint32_t, ctx_memorial_level > m_level;
			  std::map< uint32_t, ctx_coin_set_item > m_consolo_premio;

			  bool m_load;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif
	};

	typedef Singleton< MemorialSystem > sMemorialSystem;
}

#endif
