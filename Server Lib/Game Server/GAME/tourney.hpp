
#pragma once
#ifndef _STDA_TOURNEY_HPP
#define _STDA_TOURNEY_HPP

#include "tourney_base.hpp"

namespace stdA {
	class Tourney : public TourneyBase {
		public:
			Tourney(std::vector< player* >& _players, RoomInfoEx& _ri, RateValue _rv, unsigned char _channel_rookie);
			virtual ~Tourney();

			virtual bool deletePlayer(player* _session, int _option) override;

			virtual void deleteAllPlayer();

			virtual bool requestFinishLoadHole(player& _session, packet *_packet) override;

			virtual void changeHole(player& _session) override;
			virtual void finishHole(player& _session) override;

			void finish_tourney(player& _session, int _option);

			virtual bool requestUseTicketReport(player& _session, packet *_packet) override;

			virtual void requestStartAfterEnter(job& _job) override;
			virtual void requestEndAfterEnter() override;

			virtual void timeIsOver() override;

		protected:

			virtual bool init_game() override;

			virtual void clear_time_after_enter();

			virtual void requestFinishExpGame();

			virtual void finish();

			virtual void requestMakeMedal();

			virtual void requestMakeTrofel();

			virtual void requestSaveTicketReport();

			virtual void requestSendTicketReport();

			virtual void requestGiveMedalAndItens();

			virtual void requestFinishData(player& _session);

			virtual void requestCalculeShotSpinningCube(player& _session, ShotSyncData& _ssd) override;

			virtual void requestCalculeShotCoin(player& _session, ShotSyncData& _ssd) override;

		protected:
			static bool speediest_sort(PlayerGameInfo* _pgi1, PlayerGameInfo* _pgi2);
			static bool best_drive_sort(PlayerGameInfo* _pgi1, PlayerGameInfo* _pgi2);
			static bool best_chipin_sort(PlayerGameInfo* _pgi1, PlayerGameInfo* _pgi2);
			static bool best_long_puttin_sort(PlayerGameInfo* _pgi1, PlayerGameInfo* _pgi2);
			static bool best_recovery(PlayerGameInfo* _pgi1, PlayerGameInfo* _pgi2);

		public:
			virtual bool finish_game(player& _session, int option = 0) override;

		protected:
			timer *m_pTimer_after_enter;

			bool m_tourney_state;
	};
}

#endif
