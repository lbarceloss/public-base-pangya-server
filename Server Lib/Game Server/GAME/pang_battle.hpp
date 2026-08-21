
#pragma once
#ifndef _STDA_PANG_BATTLE_HPP
#define _STDA_PANG_BATTLE_HPP

#include "versus_base.hpp"
#include "../TYPE/pang_battle_type.hpp"

namespace stdA {
	class PangBattle : public VersusBase {
		public:
			PangBattle(std::vector< player* >& _players, RoomInfoEx& _ri, RateValue _rv, unsigned char _channel_rookie);
			virtual ~PangBattle();

			virtual bool deletePlayer(player* _session, int _option) override;

			virtual void deleteAllPlayer();

			virtual void requestInitHole(player& _session, packet *_packet) override;

			virtual void changeHole() override;
			virtual void finishHole() override;

			virtual void requestInitShot(player& _session, packet *_packet) override;

			void finish_pang_battle(int _option);

			virtual void timeIsOver(void* _quem) override;

		protected:

			virtual bool init_game() override;

			virtual void requestTranslateFinishHoleData(player& _session, UserInfoEx& _ui);

			virtual bool checkEndGame(player& _session) override;

			virtual void requestSaveInfo(player& _session, int option) override;

			virtual void finish();

			virtual void requestFinishData(player& _session);

			virtual void requestFinishHole(player& _session, int option) override;

			virtual bool checkNextStepGame(player& _session) override;

			virtual bool checkAllClearHole() override;

			virtual void updateFinishHole() override;

			virtual void sendPlayerTurn() override;

			virtual void changeTurn() override;

			virtual void sendPlacar(player& _session) override;

			virtual void requestCalculeRankPlace() override;

			virtual eMSG_MAKE_HOLE calcMsgToPlayerMakeHole(PlayerGameInfo* _pgi);

			virtual void init_pang_battle_data();

			virtual void calculePlayerWinPangBattle();

			virtual void savePangBattleDados(player& _session);

		protected:
			static bool sort_player_top_shot(PlayerGameInfo* _pgi1, PlayerGameInfo* _pgi2);
			static bool sort_player_top_shot_approach(PlayerOrderTurnCtx& _potc1, PlayerOrderTurnCtx& _potc2);
			static bool sort_player_rank_place(PlayerGameInfo* _pgi1, PlayerGameInfo* _pgi2);

			inline void init_player_order_top_shot();
			inline std::vector< PlayerOrderTurnCtx > init_player_order_top_shot_approach();

		public:
			virtual bool finish_game(player& _session, int option = 0) override;

		protected:
			bool m_pang_battle_state;

			PangBattleData m_pbd;

			std::vector< PlayerGameInfo* > m_player_order_pb;
	};
}

#endif
