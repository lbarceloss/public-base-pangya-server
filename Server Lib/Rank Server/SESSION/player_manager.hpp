
#pragma once
#ifndef _STDA_PLAYER_MANAGER_HPP
#define _STDA_PLAYER_MANAGER_HPP

#include <vector>
#include "player.hpp"
#include "../../Projeto IOCP/SOCKET/session_manager.hpp"
#include "../TYPE/pangya_rank_st.hpp"

namespace stdA {
	class player_manager : public session_manager {
		public:
			player_manager(threadpool& _threadpool, uint32_t _max_session);
			virtual ~player_manager();

			virtual player *findPlayer(uint32_t _uid, bool _oid = false);
	};
}

#endif
