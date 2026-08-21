
#pragma once
#ifndef _STDA_GRAND_ZODIAC_HPP
#define _STDA_GRAND_ZODIAC_HPP

#include "grand_zodiac_base.hpp"

namespace stdA {
	class GrandZodiac : public GrandZodiacBase {
		public:
			GrandZodiac(std::vector< player* >& _players, RoomInfoEx& _ri, RateValue _rv, unsigned char _channel_rookie);
			virtual ~GrandZodiac();

			virtual void changeHole(player& _session) override;
			virtual void finishHole(player& _session) override;

			void finish_grand_zodiac(player& _session, int _option);

			virtual void timeIsOver() override;

		protected:

			virtual bool init_game() override;

			virtual void requestFinishExpGame() override;

			virtual void finish(int option) override;

			virtual void drawDropItem(player& _session) override;

			virtual void requestFinishData(player& _session, int option) override;

			virtual void updateFinishHole(player& _session, int _option) override;

			virtual void requestMakeTrofel() override;

			virtual void startGoldenBeam() override;
			virtual void endGoldenBeam() override;

			virtual void requestCalculePontos();

			virtual void sendTrofel(player& _session);

		public:
			virtual bool finish_game(player& _session, int option = 0) override;

		protected:
			bool m_grand_zodiac_state;
	};
}

#endif
