
#pragma once
#ifndef _STDA_PRACTICE_HPP
#define _STDA_PRACTICE_HPP

#include "tourney_base.hpp"

namespace stdA {
	class Practice : public TourneyBase {
		public:
			Practice(std::vector< player* >& _players, RoomInfoEx& _ri, RateValue _rv, unsigned char _channel_rookie);
			virtual ~Practice();

			virtual void changeHole(player& _session) override;
			virtual void finishHole(player& _session) override;

			virtual void requestInitHole(player& _session, packet *_packet) override;

			virtual void requestCalculePang(player& _session) override;

			void finish_practice(player& _session, int _option);

			virtual void requestChangeWindNextHoleRepeat(player& _session, packet *_packet) override;

			virtual void timeIsOver() override;

		protected:

			virtual bool init_game() override;

			virtual void requestReplySyncShotData(player& _session) override;

			virtual void requestSavePang(player& _session);

			virtual void requestFinishExpGame();

			virtual void finish();

			virtual void requestFinishData(player& _session);

			virtual int checkEndShotOfHole(player& _session);

			virtual void requestCalculeShotSpinningCube(player& _session, ShotSyncData& _ssd) override;

			virtual void requestCalculeShotCoin(player& _session, ShotSyncData& _ssd) override;

		public:
			virtual bool finish_game(player& _session, int option = 0) override;

		protected:
			bool m_practice_state;
	};
}

#endif
