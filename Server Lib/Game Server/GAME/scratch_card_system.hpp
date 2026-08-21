
#pragma once
#ifndef _STDA_SCRATCH_CARD_SYSTEM_HPP
#define _STDA_SCRATCH_CARD_SYSTEM_HPP

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#endif

#include "../TYPE/scratch_card_type.hpp"
#include "../SESSION/player.hpp"

#include "../../Projeto IOCP/TYPE/singleton.h"

#include <vector>

namespace stdA {
	class ScratchCardSystem {
		public:
			ScratchCardSystem();
			virtual ~ScratchCardSystem();

			void load();
			bool isLoad();

			WarehouseItemEx* hasCoupon(player& _session);

			std::vector< ctx_scratch_card_item_win > Play(player& _session);

		private:
			std::vector< ctx_scratch_card_item > m_ctx_psi;
			bool m_load;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif
	};

	typedef Singleton< ScratchCardSystem > sScratchCardSystem;
}

#endif
