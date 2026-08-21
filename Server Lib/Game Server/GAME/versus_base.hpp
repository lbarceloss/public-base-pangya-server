
#pragma once
#ifndef _STDA_VERSUS_BASE_HPP
#define _STDA_VERSUS_BASE_HPP

#if defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#include "../../Projeto IOCP/UTIL/event.hpp"
#endif

#include "game.hpp"
#include "../../Projeto IOCP/THREAD POOL/thread.h"

namespace stdA {
	class VersusBase : public Game {
		public:
			enum STATE_VERSUS : unsigned char {
				WAIT_HIT_SHOT,
				SHOTING,
				END_SHOT,
				LOAD_HOLE,
				WAIT_END_GAME,
			};

			struct stStateVersus {
				stStateVersus() : m_state(STATE_VERSUS::WAIT_HIT_SHOT) {

#if defined(_WIN32)
					InitializeCriticalSection(&m_cs);
#elif defined(__linux__)
					INIT_PTHREAD_MUTEXATTR_RECURSIVE;
					INIT_PTHREAD_MUTEX_RECURSIVE(&m_cs);
					DESTROY_PTHREAD_MUTEXATTR_RECURSIVE;
#endif
				};

				~stStateVersus() {

					m_state = STATE_VERSUS::WAIT_HIT_SHOT;

#if defined(_WIN32)
					DeleteCriticalSection(&m_cs);
#elif defined(__linux__)
					pthread_mutex_destroy(&m_cs);
#endif
				};

				void lock() {
#if defined(_WIN32)
					EnterCriticalSection(&m_cs);
#elif defined(__linux__)
					pthread_mutex_lock(&m_cs);
#endif
				};

				void unlock() {
#if defined(_WIN32)
					LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
					pthread_mutex_unlock(&m_cs);
#endif
				};

				STATE_VERSUS& getState() {
					return m_state;
				};

				void setState(STATE_VERSUS _state) {

					m_state = _state;
				};

				void setStateWithLock(STATE_VERSUS _state) {

					lock();

					m_state = _state;

					unlock();
				}

			protected:

				STATE_VERSUS m_state;

#if defined(_WIN32)
				CRITICAL_SECTION m_cs;
#elif defined(__linux__)
				pthread_mutex_t m_cs;
#endif
			};

		public:
			VersusBase(std::vector< player* >& _players, RoomInfoEx& _ri, RateValue _rv, unsigned char _channel_rookie);
			virtual ~VersusBase();

			virtual void sendInitialData(player& _session) override;
			virtual void sendInitialDataAfter(player& _session) override;

			void makeGameDataInit(packet& p);

			virtual void requestSendTimeGame(player& _session) override;

			virtual void requestInitHole(player& _session, packet *_packet) override;
			virtual bool requestFinishLoadHole(player& _session, packet *_packet) override;
			virtual void requestFinishCharIntro(player& _session, packet *_packet) override;
			virtual void requestFinishHoleData(player& _session, packet *_packet) override;

			virtual void changeHole() = 0;
			virtual void finishHole() = 0;

			virtual void requestInitShotSended(player& _session, packet *_packet) override;

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

			virtual void requestMarkerOnCourse(player& _session, packet *_packet) override;
			virtual void requestLoadGamePercent(player& _session, packet *_packet) override;
			virtual void requestStartTurnTime(player& _session, packet *_packet) override;
			virtual void requestUnOrPause(player& _session, packet *_packet) override;
			virtual void requestReplyContinue() override;

			virtual void requestExecCCGChangeWind(player& _session, packet *_packet) override;
			virtual void requestExecCCGChangeWeather(player& _session, packet *_packet) override;

			virtual bool requestFinishGame(player& _session, packet *_packet) override;

			virtual void startTime(void* _quem);

			virtual void timeIsOver(void* _quem);

		protected:

			virtual bool init_game() override;

			virtual void requestTranslateSyncShotData(player& _session, ShotSyncData& _ssd) override;
			virtual void requestReplySyncShotData(player& _session) override;

			virtual void requestTranslateFinishHoleData(player& _session, UserInfoEx& _ui);

			virtual bool checkNextStepGame(player& _session);

			virtual bool checkEndGame(player& _session) override;

			virtual bool checkPlayerTurnExistOnGame();

			virtual bool checkAllClearHole();

			virtual void clearAllClearHole();

			virtual bool checkAllClearHoleAndClear();

			virtual void setLoadHole(PlayerGameInfo* _pgi);

			virtual bool checkAllLoadHole();

			virtual void clearLoadHole();

			virtual bool setLoadHoleAndCheckAllLoadHoleAndClear(PlayerGameInfo* _pgi);

			virtual void setFinishCharIntro(PlayerGameInfo* _pgi);

			virtual bool checkAllFinishCharIntro();

			virtual void clearFinishCharIntro();

			virtual bool setFinishCharIntroAndCheckAllFinishCharIntroAndClear(PlayerGameInfo* _pgi);

			virtual void setFinishShot(PlayerGameInfo* _pgi);

			virtual bool checkAllFinishShot();

			virtual void clearFinishShot();

			virtual bool setFinishShotAndCheckAllFinishShotAndClear(PlayerGameInfo* _pgi);

			virtual void setFinishShot2(PlayerGameInfo* _pgi);

			virtual bool checkAllFinishShot2();

			virtual void clearFinishShot2();

			virtual bool setFinishShot2AndCheckAllFinishShot2AndClear(PlayerGameInfo* _pgi);

			virtual void setSyncShot(PlayerGameInfo* _pgi);

			virtual bool checkAllSyncShot();

			virtual void clearSyncShot();

			virtual bool setSyncShotAndCheckAllSyncShotAndClear(PlayerGameInfo* _pgi);

			virtual void setSyncShot2(PlayerGameInfo* _pgi);

			virtual bool checkAllSyncShot2();

			virtual void clearSyncShot2();

			virtual bool setSyncShot2AndCheckAllSyncShot2AndClear(PlayerGameInfo* _pgi);

			virtual void setInitShot(PlayerGameInfo* _pgi);

			virtual void clearInitShot();

			virtual void clear_all_clear_hole();
			virtual void clear_all_load_hole();
			virtual void clear_all_finish_char_intro();
			virtual void clear_all_finish_shot();
			virtual void clear_all_finish_shot2();
			virtual void clear_all_sync_shot();
			virtual void clear_all_sync_shot2();
			virtual void clear_all_init_shot();

			virtual void clear_all_flag_sync();

			virtual void init_treasure_hunter_info();

			virtual void updateFinishHole();

			virtual void updateTreasureHunterPoint();

			virtual void requestDrawTreasureHunterItem();

			virtual void sendSyncShot();

			virtual void sendEndShot(player& _session, DropItemRet& _cube);

			virtual void sendDropItem(player& _session);

			virtual void sendPlacar(player& _session);

			virtual void sendTreasureHunterItemDrawGUI(player& _session);

			virtual void sendReplyFinishLoadHole();

			void sendCatchUpToReconnected(player& _session, PlayerGameInfo* _pgi);

			void sendScoreCatchUpToReconnected(player& _session, PlayerGameInfo* _pgi);

			void sendBallRestoreTo(player& _session, PlayerGameInfo* _pgi, const char* _origem);

			void checkPendingBallResend();

			virtual void sendRatesOfVersusBase();

			virtual void sendReplyFinishCharIntro();

			virtual void sendPlayerTurn();

			virtual void changeTurn();

			virtual void CCGChangeWind(player& _gm, unsigned char _wind, unsigned short _degree);

			virtual int checkEndShotOfHole(player& _session);

			virtual void drawDropItem(player& _session);

			virtual void clear_treasure_hunter();

			virtual void init_turn_hole_start();
			virtual PlayerGameInfo* getNextPlayerTurnHole();
			virtual PlayerGameInfo* requestCalculePlayerTurn();

			bool jogaNesteHole(PlayerGameInfo* _pgi);

			bool hasLateJoinHold();
			void holdGameForLateJoin(player& _session, PlayerGameInfo* _pgi);
			void releaseGameAfterLateJoin(player& _session, PlayerGameInfo* _pgi);

		public:
			virtual bool finish_game(player& _session, int option = 0) override = 0;

		protected:
			static bool sort_player_turn_hole_start(PlayerGameInfo* _pgi1, PlayerGameInfo* _pgi2);
			static bool sort_player_turn(PlayerOrderTurnCtx& _potc1, PlayerOrderTurnCtx& _potc2);

			static int end_time(void* _arg1, void* _arg2);

#if defined(_WIN32)
			static DWORD WINAPI CALLBACK _checkVersusTurn(LPVOID lpParameter);
#elif defined(__linux__)
			static void* _checkVersusTurn(LPVOID lpParameter);
#endif

#if defined(_WIN32)
			DWORD checkVersusTurn();
#elif defined(__linux__)
			void* checkVersusTurn();
#endif

		protected:
			uint32_t m_max_player = 4;

			PlayerGameInfo *m_player_turn;

			uint32_t m_count_pause;

			uint32_t m_seed_mascot_effect;

			uint32_t m_flag_next_step_game;

			uint32_t m_reball_uid;
			time_t m_reball_deadline;

			TreasureHunterVersusInfo m_thi;

			stStateVersus m_state_vs;

			thread *m_thread_chk_turn;

#if defined(_WIN32)
			HANDLE m_hEvent_chk_turn;
			HANDLE m_hEvent_chk_turn_pulse;
#elif defined(__linux__)
			Event *m_hEvent_chk_turn;
			Event *m_hEvent_chk_turn_pulse;
#endif

#if defined(_WIN32)
			CRITICAL_SECTION m_cs_sync_shot;
#elif defined(__linux__)
			pthread_mutex_t m_cs_sync_shot;
#endif
	};
}

#endif
