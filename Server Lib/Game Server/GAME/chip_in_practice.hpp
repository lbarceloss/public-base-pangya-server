
#pragma once
#ifndef _STDA_CHIP_IN_PRACTICE_HPP
#define _STDA_CHIP_IN_PRACTICE_HPP

#include "grand_zodiac_base.hpp"

namespace stdA {
	class ChipInPractice : public GrandZodiacBase {
		public:
			ChipInPractice(std::vector< player* >& _players, RoomInfoEx& _ri, RateValue _rv, unsigned char _channel_rookie);
			virtual ~ChipInPractice();

			virtual void changeHole(player& _session) override;
			virtual void finishHole(player& _session) override;

			void finish_chip_in_practice(player& _session, int _option);

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

		public:
			virtual bool finish_game(player& _session, int option = 0) override;

		protected:
			bool m_chip_in_practice_state;
	};
}

#endif
