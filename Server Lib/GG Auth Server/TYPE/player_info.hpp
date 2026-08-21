
#pragma once
#ifndef _STDA_PLAYER_INFO_HPP
#define _STDA_PLAYER_INFO_HPP

#include "pangya_gg_auth_st.hpp"

namespace stdA {

	class PlayerInfo : public player_info {
		public:
			PlayerInfo();
			virtual ~PlayerInfo();

			void clear();

		public:
			unsigned char m_state;
	};
}

#endif
