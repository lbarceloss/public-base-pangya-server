
#pragma once
#ifndef _STDA_TOURNEY_BASE_HPP
#define _STDA_TOURNEY_BASE_HPP

#include "game.hpp"
#include "../TYPE/tourney_base_type.hpp"

namespace stdA {
	class TourneyBase : public Game {
		public:
			TourneyBase(std::vector< player* >& _players, RoomInfoEx& _ri, RateValue _rv, unsigned char _channel_rookie);
			virtual ~TourneyBase();

			virtual void sendInitialData(player& _session) override;
			virtual void sendInitialDataAfter(player& _session) override;

			virtual void requestInitHole(player& _session, packet *_packet) override;
			virtual bool requestFinishLoadHole(player& _session, packet *_packet) override;
			virtual void requestFinishCharIntro(player& _session, packet *_packet) override;
			virtual void requestFinishHoleData(player& _session, packet *_packet) override;

			virtual void changeHole(player& _session) = 0;
			virtual void finishHole(player& _session) = 0;

			virtual void requestInitShot(player& _session, packet *_packet) override;
			virtual void requestSyncShot(player& _session, packet *_packet) override;
			virtual void requestInitShotArrowSeq(player& _session, packet *_packet) override;
			virtual void requestShotEndData(player& _session, packet *_packet) override;
			virtual RetFinishShot requestFinishShot(player& _session, packet *_packet) override;

			virtual void requestChangeMira(player& _session, packet *_packet) override;
			virtual void requestChangeStateBarSpace(player& _session, packet *_packet) override;
			virtual void requestActivePowerShot(player& _session, packet *_packet) override;
			virtual void requestChangeClub(player& _session, packet  *_packet) override;
			virtual void requestUseActiveItem(player& _session, packet *_packet) override;
			virtual void requestChangeStateTypeing(player& _session, packet *_packet) override;
			virtual void requestMoveBall(player& _session, packet *_packet) override;
			virtual void requestChangeStateChatBlock(player& _session, packet *_packet) override;
			virtual void requestActiveBooster(player& _session, packet *_packet) override;
			virtual void requestActiveReplay(player& _session, packet *_packet) override;
			virtual void requestActiveCutin(player& _session, packet *_packet) override;

			virtual void requestActiveRing(player& _session, packet *_packet) override;
			virtual void requestActiveRingGround(player& _session, packet *_packet) override;
			virtual void requestActiveRingPawsRainbowJP(player& _session, packet *_packet) override;
			virtual void requestActiveRingPawsRingSetJP(player& _session, packet *_packet) override;
			virtual void requestActiveRingPowerGagueJP(player& _session, packet *_packet) override;
			virtual void requestActiveRingMiracleSignJP(player& _session, packet *_packet) override;
			virtual void requestActiveWing(player& _session, packet *_packet) override;
			virtual void requestActivePaws(player& _session, packet *_packet) override;
			virtual void requestActiveGlove(player& _session, packet *_packet) override;
			virtual void requestActiveEarcuff(player& _session, packet *_packet) override;

			virtual void requestUpdateTrofel() override;

			virtual void requestSendTimeGame(player& _session) override;
			virtual void requestUpdateEnterAfterStartedInfo(player& _session, EnterAfterStartInfo& _easi) override;

			virtual bool requestFinishGame(player& _session, packet *_packet) override;

			virtual void startTime();

			virtual void timeIsOver() = 0;

		protected:

			virtual bool init_game() override = 0;

			virtual void requestTranslateSyncShotData(player& _session, ShotSyncData& _ssd) override;
			virtual void requestReplySyncShotData(player& _session) override;

			virtual void sendRemainTime(player& _session);

			virtual void updateFinishHole(player& _session, int _option);

			virtual void updateTreasureHunterPoint(player& _session);

			virtual void requestDrawTreasureHunterItem(player& _session);

			virtual void sendSyncShot(player& _session);

			virtual void sendEndShot(player& _session, DropItemRet& _cube);

			virtual void sendUpdateState(player& _session, int _option);

			virtual void sendDropItem(player& _session);

			virtual void sendPlacar(player& _session);

			virtual void sendTreasureHunterItemDrawGUI(player& _session);

			virtual void sendTimeIsOver(player& _session);

			virtual int checkEndShotOfHole(player& _session);

			virtual void drawDropItem(player& _session);

			virtual void achievement_top_3_1st(player& _session);

			virtual void calcule_shot_to_spinning_cube(player& _session, ShotSyncData& _ssd);

			virtual void calcule_shot_to_coin(player& _session, ShotSyncData& _ssd);

			virtual void requestCalculeShotSpinningCube(player& _session, ShotSyncData& _ssd);

			virtual void requestCalculeShotCoin(player& _session, ShotSyncData& _ssd);

		public:
			virtual bool finish_game(player& _session, int option = 0) override = 0;

		protected:
			static int end_time(void* _arg1, void* _arg2);

		protected:
			uint32_t m_max_player;
			int32_t m_entra_depois_flag;

			TicketReportInfo m_tri;

			Medal m_medal[12];
	};
}

#endif
