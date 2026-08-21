
#pragma once
#ifndef _STDA_COIN_CUBE_LOCATION_UPDATE_SYSTEM_HPP
#define _STDA_COIN_CUBE_LOCATION_UPDATE_SYSTEM_HPP

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

#include "../../Projeto IOCP/TYPE/singleton.h"
#include "../../Projeto IOCP/TYPE/list_async.h"
#include "../TYPE/coin_cube_type.hpp"

#include "../../Projeto IOCP/THREAD POOL/thread.h"

#include "../TYPE/hole_type.hpp"

#include <map>
#include <vector>
#include <ctime>

namespace stdA {

	typedef std::map< unsigned char , std::map< unsigned char , std::vector< CubeEx > > > MAP_COURSE_COIN_CUBE;

	constexpr time_t UPDATE_TIME_INTERVALE_HOUR = 24u;

	class CoinCubeLocationUpdateSystem {

		public:
			CoinCubeLocationUpdateSystem();
			virtual ~CoinCubeLocationUpdateSystem();

			void load();

			bool isLoad();

			void pushOrderToCalcule(CalculeCoinCubeUpdateOrder _cccuo);

#ifdef _DEBUG
			void forceUpdate();
#endif

		protected:
			void initialize();

			void clear();

			void calculeShotCube(CalculeCoinCubeUpdateOrder& _cccuo );
			void calculeShotCoin(CalculeCoinCubeUpdateOrder& _cccuo);

			void checkAndAddCoinCube(unsigned char _course_id, unsigned char _hole_number, CubeEx _cube);

			void update_spwan_location();

			uint32_t getLimitCoinCubePerParHole(unsigned char _par_hole);

		protected:
#if defined(_WIN32)
			static DWORD WINAPI _translateOrder(LPVOID lpParameter);
#elif defined(__linux__)
			static void* _translateOrder(LPVOID lpParameter);
#endif

#if defined(_WIN32)
			DWORD translateOrder();
#elif defined(__linux__)
			void* translateOrder();
#endif

		private:
			list_async< CalculeCoinCubeUpdateOrder > m_pedidos;

			bool m_load;

			thread* m_thread;

			uint32_t volatile m_continue_translate;

			time_t m_update_location_time;

			MAP_COURSE_COIN_CUBE m_course_coin_cube;
			MAP_COURSE_COIN_CUBE m_course_coin_cube_current;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif

	};

	typedef Singleton< CoinCubeLocationUpdateSystem > sCoinCubeLocationUpdateSystem;
}

#endif
