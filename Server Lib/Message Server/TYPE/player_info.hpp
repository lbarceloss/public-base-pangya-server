
#pragma once
#ifndef _STDA_PLAYER_INFO_HPP
#define _STDA_PLAYER_INFO_HPP

#include "pangya_message_st.hpp"
#include "../GAME/friend_manager.hpp"

namespace stdA {
	class PlayerInfo : public player_info {
		public:
			PlayerInfo();
			virtual ~PlayerInfo();

			void clear();

		public:
			unsigned char m_state;
			volatile uint32_t m_logout;

			ChannelPlayerInfo m_cpi;

			FriendManager m_friend_manager;
	};
}

#endif
