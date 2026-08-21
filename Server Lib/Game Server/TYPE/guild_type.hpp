
#pragma once
#ifndef _STDA_GUILD_TYPE_HPP
#define _STDA_GUILD_TYPE_HPP

#include <memory.h>
#include <cstdint>

namespace stdA {

#if defined(__linux__)
#pragma pack(1)
#endif

	struct Dados {
		unsigned short score;
		uint32_t tacada;
		unsigned char finish : 1;
	};

	struct GuildMatch {
		void clear() {
			memset(this, 0, sizeof(GuildMatch));
		};
		uint32_t uid[2];
		uint32_t point[2];
		uint32_t pang[2];
	};

	struct GuildPoints {
		enum eGUILD_WIN : unsigned char {
			WIN,
			LOSE,
			DRAW,
		};
		void clear() {
			memset(this, 0, sizeof(GuildPoints));
		};
		uint32_t uid;
		uint64_t point;
		uint64_t pang;
		eGUILD_WIN win;
	};

	struct GuildMemberPoints {
		void clear() {
			memset(this, 0, sizeof(GuildMemberPoints));
		};
		uint32_t guild_uid;
		uint32_t member_uid;
		uint32_t point;
		uint32_t pang;
	};

#if defined(__linux__)
#pragma pack()
#endif
}

#endif
