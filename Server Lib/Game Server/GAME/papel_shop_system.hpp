
#pragma once
#ifndef _STDA_PAPEL_SHOP_SYSTEM_HPP
#define _STDA_PAPEL_SHOP_SYSTEM_HPP

#if defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

#include "../TYPE/papel_shop_type.hpp"
#include <vector>
#include <map>

#include "../SESSION/player.hpp"

#include "../../Projeto IOCP/TYPE/singleton.h"

#define PAPEL_SHOP_MIN_BALL 1
#define PAPEL_SHOP_MAX_BALL 5
#define PAPEL_SHOP_BIG_BALL 10

#define PAPEL_SHOP_ITEM_MIN_QNTD 1
#define PAPEL_SHOP_ITEM_MAX_QNTD 3

namespace stdA {
	class PapelShopSystem {
		public:
			PapelShopSystem();
			virtual ~PapelShopSystem();

			  void load();
			  bool isLoad();

			  bool isLimittedPerDay();

			  void init_player_papel_shop_info(player& _session);

			  void updateDia();
			  void updateDiaPlayer(player& _session);

			  void updatePlayerCount(player& _session);

			  void updateConfig(ctx_papel_shop& _ps);

			  uint64_t getPriceNormal();
			  uint64_t getPriceBig();

			  WarehouseItemEx* hasCoupon(player& _session);

			  std::vector< ctx_papel_shop_ball > dropBalls(player& _session);

			  std::vector< ctx_papel_shop_ball > dropBigBall(player& _session);

		protected:
			  void initialize();

			  void clear();

			  bool checkUpdate();
			  bool checkUpdate(SYSTEMTIME& _st);

		protected:
			static void SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg);

		private:
			  std::vector< ctx_papel_shop_item > m_ctx_psi;
			  std::map< uint32_t, ctx_papel_shop_coupon > m_ctx_psc;

			  ctx_papel_shop m_ctx_ps;

			  bool m_load;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif
	};

	typedef Singleton< PapelShopSystem > sPapelShopSystem;
}

#endif
