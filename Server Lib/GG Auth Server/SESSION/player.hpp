
#pragma once
#ifndef _STDA_PLAYER_HPP
#define _STDA_PLAYER_HPP

#include "../../Projeto IOCP/SOCKET/session.h"
#include "../TYPE/player_info.hpp"
#include "../../Projeto IOCP/THREAD POOL/threadpool_base.hpp"

#include "../TYPE/game_guard_type.hpp"

#include <map>

namespace stdA {

	class player : public session {
		public:
			player(threadpool_base& _threadpool);
			virtual ~player();

			virtual bool clear() override;

			virtual unsigned char getStateLogged() override;

			virtual uint32_t getUID() override;
			virtual uint32_t getCapability() override;
			virtual char* getNickname() override;
			virtual char* getID() override;

			void addPlayerToGameGuard(unsigned long _uid);
			void removePlayerToGameGuard(unsigned long _uid);

			PlayerGameGuard* getPlayerGameGuard(unsigned long _uid);

		public:
			PlayerInfo m_pi;

			std::map< unsigned long , PlayerGameGuard* > m_gg;

	};
}

#endif
