
#pragma once
#ifndef _STDA_PLAYER_INFO_HPP
#define _STDA_PLAYER_INFO_HPP

#include "pangya_rank_st.hpp"

namespace stdA {
	class PlayerInfo : public player_info {
		public:
			PlayerInfo();
			virtual ~PlayerInfo();

			virtual void clear();

		public:
			unsigned char m_state;

			search_dados_ex m_sd;
	};
}

#endif
