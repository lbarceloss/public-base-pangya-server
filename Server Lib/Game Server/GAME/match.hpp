
#pragma once
#ifndef _STDA_MATCH_HPP
#define _STDA_MATCH_HPP

#include "versus_base.hpp"
#include "team.hpp"
#include <vector>

namespace stdA {
	class Match : public VersusBase {
		public:

			struct TeamOrderTurnCtx {
				TeamOrderTurnCtx(uint32_t _ul = 0u) {
					clear();
				};
				TeamOrderTurnCtx(Team* _team, Hole *_hole)
					: team(_team), hole(_hole) {
				}
				void clear() {
					team = nullptr;
					hole = nullptr;
				};
				Team *team;
				Hole *hole;
			};

		public:
			Match(std::vector< player* >& _players, RoomInfoEx& _ri, RateValue _rv, unsigned char _channel_rookie, std::vector< Team >& _teans);
			virtual ~Match();

			virtual bool deletePlayer(player* _session, int _option) override;

			virtual void deleteAllPlayer();

			virtual void requestInitHole(player& _session, packet *_packet) override;

			virtual void requestMoveBall(player& _session, packet *_packet) override;

			virtual void changeHole() override;
			virtual void finishHole() override;

			void finish_match(int _option);

			virtual void requestTeamFinishHole(player& _session, packet* _packet) override;

			virtual stGameShotValue getGameShotValueToSmartCalculator(player& _session, unsigned char _club_index, unsigned char _power_shot_index) override;

			virtual void startTime(void* _quem) override;

			virtual void timeIsOver(void* _quem) override;

		protected:

			virtual bool init_game() override;

			virtual void requestTranslateSyncShotData(player& _session, ShotSyncData& _ssd) override;
			virtual void requestTranslateFinishHoleData(player& _session, UserInfoEx& _ui) override;

			virtual bool checkEndGame(player& _session) override;

			virtual bool checkAllClearHole() override;
			virtual bool checkAllClearHoleAndClear() override;

			virtual void clearAllClearHole() override;

			virtual void clear_all_clear_hole() override;

			virtual void clear_teans();

			virtual void updateTreasureHunterPoint() override;

			virtual bool checkNextStepGame(player& _session) override;

			virtual void requestSaveInfo(player& _session, int _option) override;

			virtual void requestFinishExpGame();

			virtual void requestCalculeTeamWin();

			virtual void requestUpdateTeamPang();

			virtual void finish();

			virtual void requestFinishTeamHole();

			virtual void requestFinishData(player& _session);

			virtual void sendPlacar(player& _session) override;

			virtual void sendFinishMessage(player& _session) override;

			virtual void sendReplyFinishLoadHole() override;
			virtual void sendReplyFinishCharIntro() override;

			virtual int checkEndShotOfHole(player& _session) override;

			virtual void changeTurn() override;

			virtual void CCGChangeWind(player& _gm, unsigned char _wind, unsigned short _degree) override;

			virtual PlayerGameInfo* requestCalculePlayerTurn() override;
			virtual Team* requestCalculeTeamTurn();

			virtual void init_team_player_position();

		public:
			virtual bool finish_game(player& _session, int option = 0) override;

		protected:
			static bool sort_team_turn(TeamOrderTurnCtx& _totc1, TeamOrderTurnCtx& _totc2);

			static void SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg);

		protected:
			bool m_match_state;

			unsigned char m_team_win;

			Team* m_team_turn;

			TreasureHunterVersusInfo m_thi_blue;

			std::vector< Team > m_teans;
	};
}

#endif
