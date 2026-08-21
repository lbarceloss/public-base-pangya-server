
#pragma once
#ifndef _STDA_COIN_CUBE_TYPE_HPP
#define _STDA_COIN_CUBE_TYPE_HPP

#include "game_type.hpp"
#include "hole_type.hpp"

namespace stdA {

#if defined(__linux__)
#pragma pack(1)
#endif

	struct CalculeCoinCubeUpdateOrder {
		public:
			enum eTYPE : unsigned char {
				COIN,
				CUBE,
			};
			eTYPE type;
			uint32_t uid;
			Location last_location;
			Location pin;
			ShotEndLocationData shot_data_for_cube;
			unsigned char course;
			unsigned char hole;
	};

	struct CoinCubeUpdate {
		public:
			enum eTYPE : unsigned char {
				INSERT,
				UPDATE,
			};
			eTYPE type;
			unsigned char course_id;
			unsigned char hole_number;
			CubeEx cube;
	};

#if defined(__linux__)
#pragma pack()
#endif
}

#endif
