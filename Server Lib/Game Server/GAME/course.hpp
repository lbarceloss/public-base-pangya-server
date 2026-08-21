
#pragma once
#ifndef _STDA_COURSE_HPP
#define _STDA_COURSE_HPP

#include "hole.hpp"
#include <map>
#include <vector>

#include "../TYPE/game_type.hpp"

#include "../../Projeto IOCP/PACKET/packet.h"

namespace stdA {
	class Course {
		public:
			struct Sequencia {
				Sequencia(uint32_t _ul = 0u) {
					clear();
				};
				Sequencia(unsigned short _hole) {
					clear();

					m_hole = _hole;
				};
				Sequencia(unsigned char _course, unsigned short _hole)
					: m_course(_course), m_hole(_hole) {
				};
				void clear() {
					memset(this, 0, sizeof(Sequencia));
				};
				unsigned char m_course;
				unsigned short m_hole;
			};

		public:
			Course(RoomInfoEx& _ri, unsigned char _channel_rookie, float _star, uint32_t _rate_rain, unsigned char _rain_persist_flag);
			virtual ~Course();

			uint32_t getSeedRandGame();

			unsigned short getFlagCubeCoin();

			float getStar();

			Hole* findHole(unsigned short _number);
			Hole* findHoleBySeq(unsigned short _seq);

			unsigned short findHoleSeq(unsigned short _number);

			std::pair< std::map< unsigned short, Hole >::iterator, std::map< unsigned short, Hole >::iterator > findRange(unsigned short _number);

			stHoleWind shuffleWind(uint32_t _seed = 777u);

			void shuffleWindNextHole(unsigned short _number);

			void makePacketHoleInfo(packet& _p, int _option = 0, unsigned short _from_seq = 1u);

			void makePacketHoleSpinningCubeInfo(packet& _p, unsigned short _from_seq = 1u);

			uint32_t countHolesRain();
			uint32_t countHolesRainBySeq(uint32_t _seq);

			float getMediaAllParHoles();
			float getMediaAllParHolesBySeq(uint32_t _seq);

			ConsecutivosHolesRain& getConsecutivesHolesRain();

		protected:
			void init_seq();
			void init_hole();

			void init_dados_rain();

		protected:
			std::map< unsigned short, Hole > m_hole;
			std::vector< Sequencia > m_seq;

			unsigned char m_channel_rookie;
			uint32_t m_rate_rain;
			unsigned char m_rain_persist_flag;

			float m_star;

			unsigned short m_wind_range[2];
			unsigned short m_wind_flag;

			uint32_t m_seed_rand_game;

			RoomInfoEx& m_ri;

			HolesRain m_holes_rain;
			ConsecutivosHolesRain m_chr;

			bool m_grand_prix_special_hole;

		private:
			unsigned short m_flag_cube_coin;
	};
}

#endif
