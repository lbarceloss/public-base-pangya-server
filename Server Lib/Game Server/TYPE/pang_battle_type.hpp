
#pragma once
#ifndef _STDA_PANG_BATTLE_TYPE_HPP
#define _STDA_PANG_BATTLE_TYPE_HPP

#include <vector>

namespace stdA {

#if defined(__linux__)
#pragma pack(1)
#endif

	enum eMSG_MAKE_HOLE : unsigned short {
		MMH_PERDEU,
		MMH_GANHOU = 2,
		MMH_EMPATOU,
	};

	struct PangBattleHolePang {
		PangBattleHolePang(uint32_t _pang) : pang(_pang), pang_extra(0u), player_win(-3), vezes(1u) {
		};
		void clear() {
			pang = 0u;
			pang_extra = 0u;
			player_win = -3;
			vezes = 1u;
		};
		int32_t player_win;
		uint32_t pang;
		uint32_t pang_extra;
		unsigned char vezes;
	};

	struct PangBattleData {
		PangBattleData(uint32_t _ul = 0u) {
			clear();
		};
		~PangBattleData() {};
		void clear() {

			m_hole = -1;
			m_hole_extra = -1;
			m_hole_extra_flag = false;
			m_count_finish_hole = 0u;
			m_player_win_pb = (uint32_t)-1;

			if (!v_player_win.empty()) {
				v_player_win.clear();
				v_player_win.shrink_to_fit();
			}
		};
		short m_hole;
		bool m_hole_extra_flag;
		short m_hole_extra;
		uint32_t m_count_finish_hole;
		uint32_t m_player_win_pb;
		std::vector< PangBattleHolePang > v_player_win;
	};

#if defined(__linux__)
#pragma pack()
#endif
}

#endif
