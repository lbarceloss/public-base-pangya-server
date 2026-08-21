
#pragma once
#ifndef _STDA_PANGYA_GG_AUTH_ST_HPP
#define _STDA_PANGYA_GG_AUTH_ST_HPP

#include <memory>

#include "../../Projeto IOCP/TYPE/pangya_st.h"

namespace stdA {

	struct player_info {
		player_info() {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(player_info));
		};
		unsigned long uid;
		unsigned long tipo;
		char id[22];
		char nickname[22];
		unsigned long auth_key;
	};
}

#endif
