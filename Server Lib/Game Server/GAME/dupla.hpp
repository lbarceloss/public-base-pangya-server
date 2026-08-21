
#pragma once
#ifndef _STDA_DUPLA_HPP
#define _STDA_DUPLA_HPP

#include "../TYPE/guild_type.hpp"
#include "../SESSION/player.hpp"

namespace stdA {
	struct Dupla {
	public:
		enum eSTATE : unsigned char {
			IN_GAME,
			OUT_GAME,
			OVER_TIME,
		};

	public:
		Dupla(unsigned char _numero, player *_p1, player *_p2);

		unsigned short sumScoreP1();
		unsigned short sumScoreP2();

		unsigned char numero;
		player *p[2];
		unsigned char hole[2];
		uint32_t pang_win[2];
		uint64_t pang[2];
		eSTATE state[2];
		Dados dados[2][18];
	};
}

#endif
