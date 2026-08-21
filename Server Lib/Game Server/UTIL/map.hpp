
#pragma once
#ifndef _STDA_MAP_HPP
#define _STDA_MAP_HPP

#include "../../Projeto IOCP/TYPE/singleton.h"

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

#include <string>
#include <map>

#include <memory.h>
#include <cstdint>

namespace stdA {
	class Map {
		public:
			struct stCtx {
				stCtx(uint32_t _ul = 0u) {
					clear();
				};
				void clear() {

					name.clear();
					range_score.clear();

					clear_bonus = 0u;
					star = 0.f;
				};
				struct stParRangeScore {
					void clear() { memset(this, 0, sizeof(stParRangeScore)); };
					char par[18];
					char min[18];
					char max[18];
				};
				std::string name;
				uint32_t clear_bonus;
				float star;
				stParRangeScore range_score;
			};

		public:
			Map();
			~Map();

			  bool isLoad();
			  void load();

			  stCtx* getMap(unsigned char _course);

			  uint32_t calculeClearVS(stCtx& _ctx, uint32_t _num_player, uint32_t _qntd_hole);
			  uint32_t calculeClearMatch(stCtx& _ctx, uint32_t _qntd_hole);
			  uint32_t calculeClear30s(stCtx& _ctx, uint32_t _qntd_hole);
			  uint32_t calculeClearSSC(stCtx& _ctx);

		protected:
			  void initialize();

			  void clear();

		private:
			  std::map< unsigned char, stCtx > m_map;

			  bool m_load;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif
	};

	typedef Singleton< Map > sMap;
}

#endif
