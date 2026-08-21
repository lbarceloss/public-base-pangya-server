
#pragma once
#ifndef _STDA_HOLE_TYPE_HPP
#define _STDA_HOLE_TYPE_HPP

#include <memory>
#include <algorithm>

#include <memory.h>

#include "../../Projeto IOCP/UTIL/random_gen.hpp"

namespace stdA {

#if defined(__linux__)
#pragma pack(1)
#endif

	constexpr unsigned char LIMIT_DEGREE = 255u;
#define CALC_MIN_DEGREE(__degree) (unsigned short)(((short)(__degree) - 20) < 0) ? LIMIT_DEGREE + (__degree) - 20 : (__degree) - 20

	struct stHoleWind {
		stHoleWind(uint32_t _ul = 0u) : degree(0u) {
			clear();
		};
		stHoleWind(unsigned char _wind, unsigned short _degree) : wind(_wind), degree(_degree) {
		}
		void clear() { memset(this, 0, sizeof(stHoleWind)); };
		unsigned char wind;
		struct stDegree {
			stDegree(unsigned short _degree) : degree(_degree), min_degree(0u) {
				min_degree = CALC_MIN_DEGREE(degree);
			};
			void clear() { memset(this, 0, sizeof(stDegree)); };
			void setDegree(unsigned short _degree) {
				degree = _degree;

				min_degree = CALC_MIN_DEGREE(degree);
			};
			unsigned short getDegree() { return degree; };
			unsigned short getShuffleDegree() {

				degree = (unsigned short)((min_degree + (sRandomGen::getInstance().rIbeMt19937_64_chrono() % LIMIT_RANGE)) % LIMIT_DEGREE);

				return degree;
			};
		protected:
			unsigned short degree;
			unsigned short min_degree;
			static const unsigned char LIMIT_RANGE = 40u;
		};
		stDegree degree;
	};

	struct stHolePar {
		void clear() { memset(this, 0, sizeof(stHolePar)); };
		unsigned char par;
		char range_score[2];
		unsigned char total_shot;
	};

	struct stXZLocation {
		void clear() { memset(this, 0, sizeof(stXZLocation)); };
		float x;
		float z;
	};

	union uCubeCoinFlag {
		void clear() { memset(this, 0, sizeof(uCubeCoinFlag)); };
		unsigned char ucFlag[2];
		struct {
			unsigned char type : 2, : 0;
			unsigned char enable : 1;
			unsigned char enable_cube : 1;
			unsigned char enable_coin : 1, : 0;
		}stFlag;
	};

	struct Cube {
		public:
			enum eFLAG_LOCATION : uint32_t {
				EDGE_GREEN,
				CARPET,
				AIR,
				GROUND,
			};

			enum eTYPE : uint32_t {
				COIN,
				CUBE,
			};

		public:
			Cube(uint32_t _ul = 0u) {
				clear();
			};
			Cube(uint32_t _id, eTYPE _tipo, uint32_t _flag_unknown, eFLAG_LOCATION _flag_location, float _x, float _y, float _z)
				: id(_id), tipo(_tipo), flag_unknown(_flag_unknown), flag_location(_flag_location), location{ _x, _y, _z } {
			};
			void clear() { memset(this, 0, sizeof(Cube)); };
			struct stLocation {
				void clear() { memset(this, 0, sizeof(stLocation)); };
				float x;
				float y;
				float z;
			};

		public:
			stLocation location;
			uint32_t id;
			eTYPE tipo;
			uint32_t flag_unknown;
			eFLAG_LOCATION flag_location;
	};

	struct CubeEx : public Cube {
		public:
			CubeEx(uint32_t _ul = 0u) : Cube(_ul), rate(1u) {
			};
			CubeEx(uint32_t _id, eTYPE _tipo, uint32_t _flag_unknown, eFLAG_LOCATION _flag_location, float _x, float _y, float _z, uint32_t _rate)
				: Cube(_id, _tipo, _flag_unknown, _flag_location, _x, _y, _z), rate(_rate) {
			};
			void clear() {

				Cube::clear();

				rate = 1u;
			};

		public:
			uint32_t rate;
	};

#if defined(__linux__)
#pragma pack()
#endif
}

#endif
