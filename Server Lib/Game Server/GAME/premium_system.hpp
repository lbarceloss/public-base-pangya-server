
#pragma once
#ifndef _STDA_PREMIUM_SYSTEM_HPP
#define _STDA_PREMIUM_SYSTEM_HPP

#include "../../Projeto IOCP/TYPE/singleton.h"
#include "../SESSION/player.hpp"
#include "../TYPE/pangya_game_st.h"

namespace stdA {

	class PremiumSystem {

		public:
			PremiumSystem();
			virtual ~PremiumSystem();

			void checkEndTimeTicket(player& _session);

			void addPremiumUser(player& _session, WarehouseItemEx& _ticket, uint32_t _time);
			void removePremiumUser(player& _session);

			stItem addPremiumBall(player& _session);
			stItem addPremiumClubSet(player& _session, uint32_t _time);
			stItem addPremiumMascot(player& _session, uint32_t _time);

			void removePremiumBall(player& _session);

			void updatePremiumUser(player& _session);

			uint32_t getPremiumBallByTicket(uint32_t _typeid);
			uint32_t getPremiumClubSetByTicket(uint32_t _typeid);
			uint32_t getPremiumMascotByTicket(uint32_t _typeid);

			uint32_t getExpPangRateByTicket(uint32_t _typeid);

			bool isPremiumTicket(uint32_t _typeid);
			bool isPremiumBall(uint32_t _typeid);

			bool isPremium1(uint32_t _typeid);
			bool isPremium2(uint32_t _typeid);
	};

	typedef Singleton< PremiumSystem > sPremiumSystem;
}

#endif
