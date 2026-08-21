
#pragma once
#ifndef _STDA_CARD_SYSTEM_HPP
#define _STDA_CARD_SYSTEM_HPP

#if defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

#include <vector>
#include <map>

#include "../TYPE/card_type.hpp"
#include "../../Projeto IOCP/TYPE/singleton.h"

namespace stdA {
	class CardSystem {
		public:
			CardSystem();
			virtual ~CardSystem();

			  void load();

			  bool isLoad();

			  CardPack* findCardPack(uint32_t _typeid);
			  CardPack* findBoxCardPack(uint32_t _typeid);
			  Card* findCard(uint32_t _typeid);

			  std::vector< Card > draws(CardPack& _cp);
			  Card drawsLoloCardCompose(LoloCardComposeEx& _lcc);

		protected:
			  void initialize();

			  void clear();

		private:
			  std::vector< Card > m_card;
			  std::map< uint32_t, CardPack > m_card_pack;
			  std::map< uint32_t, CardPack > m_box_card_pack;

			  bool m_load;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif
	};

	typedef Singleton< CardSystem > sCardSystem;
}

#endif
