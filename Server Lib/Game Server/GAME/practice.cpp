
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "practice.hpp"
#include "../../Projeto IOCP/UTIL/exception.h"
#include "../../Projeto IOCP/TYPE/stda_error.h"
#include "../../Projeto IOCP/UTIL/message_pool.h"

#include "../GAME/treasure_hunter_system.hpp"

#include "../PACKET/packet_func_sv.h"

#include "../UTIL/map.hpp"

#define CHECK_SESSION_BEGIN(method) if (!_session.getState()) \
										throw exception("[Practice::" + std::string((method)) +"][Error] player nao esta connectado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PRACTICE, 1, 0)); \

#define REQUEST_BEGIN(method) CHECK_SESSION_BEGIN(std::string("request") + (method)) \
							  if (_packet == nullptr) \
									throw exception("[Practice::request" + std::string((method)) +"][Error] _packet is nullptr", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PRACTICE, 6, 0)); \

#define INIT_PLAYER_INFO(_method, _msg, __session) auto pgi = getPlayerInfo((__session)); \
	if (pgi == nullptr) \
		throw exception("[Practice::" + std::string((_method)) + "][Error] player[UID=" + std::to_string((__session)->m_pi.uid) + "] " + std::string((_msg)) + ", mas o game nao tem o info dele guardado. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PRACTICE, 1, 4)); \

using namespace stdA;

Practice::Practice(std::vector< player* >& _players, RoomInfoEx& _ri, RateValue _rv, unsigned char _channel_rookie)
	: TourneyBase(_players, _ri, _rv, _channel_rookie), m_practice_state(false) {

	initAllPlayerInfo();

	m_state = init_game();
}

Practice::~Practice() {

	m_practice_state = false;

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("[Practice::~Practice][Log] Practice destroyed on Room[Number=" + std::to_string(m_ri.numero) + "]", CL_FILE_LOG_AND_CONSOLE));
#endif
}

void Practice::changeHole(player& _session) {

	updateTreasureHunterPoint(_session);

	if (checkEndGame(_session))
		finish_practice(_session, 0);
	else

		updateFinishHole(_session, 1);
}

void Practice::finishHole(player& _session) {

	requestFinishHole(_session, 0);

	requestUpdateItemUsedGame(_session);

}

void Practice::requestInitHole(player& _session, packet *_packet) {
	REQUEST_BEGIN("InitHole");

	try {

		TourneyBase::requestInitHole(_session, _packet);

		INIT_PLAYER_INFO("requestInitHole", "tentou inicializar o hole do jogo", &_session);

		pgi->sys_achieve.incrementCounter(0x6C400005u );

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Practice::requestInitHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void Practice::requestCalculePang(player& _session) {

	Game::requestCalculePang(_session);

	INIT_PLAYER_INFO("requestCalculePang", "tentou calcular o pang do player no jogo", &_session);

	if (m_ri.modo == RoomInfo::MODO::M_REPEAT) {

		pgi->data.pang = (uint64_t)(pgi->data.pang * (1.f / 6.f));
		pgi->data.bonus_pang = (uint64_t)(pgi->data.bonus_pang * (1.f / 6.f));

	}else {

		pgi->data.pang = (uint64_t)(pgi->data.pang * (1.f / 3.f));
		pgi->data.bonus_pang = (uint64_t)(pgi->data.bonus_pang * (1.f / 3.f));
	}
}

void Practice::finish_practice(player& _session, int _option) {

	if (m_players.size() > 0 && m_game_init_state == 1) {

		INIT_PLAYER_INFO("finish_practice", "tentou terminar o practice no jogo", &_session);

		if (pgi->flag == PlayerGameInfo::eFLAG_GAME::PLAYING) {

			requestCalculePang(_session);

			updatePlayerAssist(_session);

			if (m_game_init_state == 1 && _option == 0) {

				sendFinishMessage(_session);

				updateFinishHole(_session, 1);

				sendUpdateState(_session, 2);

				pgi->sys_achieve.incrementCounter((m_ri.modo == RoomInfo::MODO::M_REPEAT) ? 0x6C40003Du  : 0x6C40003Eu );
			}
		}

		setGameFlag(pgi, (_option == 0) ? PlayerGameInfo::eFLAG_GAME::FINISH : PlayerGameInfo::eFLAG_GAME::END_GAME);

		GetLocalTime(&pgi->time_finish);

		if (AllCompleteGameAndClear() && m_game_init_state == 1)
			finish();
	}

}

void Practice::requestChangeWindNextHoleRepeat(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChangeWindNextHoleRepeat");

	try {

		INIT_PLAYER_INFO("requestChangeWindNextHoleRepeat", "tentou trocar o vento dos proximos holes repeat no jogo", &_session);

		m_course->shuffleWindNextHole(pgi->hole);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Practice::requestChangeWindNextHoleRepeat][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void Practice::timeIsOver() {

	try {

#if defined(_WIN32)
		EnterCriticalSection(&m_cs_sync_finish_game);
#elif defined(__linux__)
		pthread_mutex_lock(&m_cs_sync_finish_game);
#endif

		if (m_game_init_state == 1 && m_players.size() > 0) {

			m_game_init_state = 2;

			packet p;

			auto s = m_players.front();

			INIT_PLAYER_INFO("end_time", "tentou acabar o tempo no jogo", s);

			requestCalculePang(*s);

			sendFinishMessage(*s);

			updateFinishHole(*s, 0);

			requestFinishHole(*s, 1);

			sendTimeIsOver(*s);

			finish();

	#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("[Practice::timeIsOver][Log] Tempo Acabou no Practice. na sala[NUMERO=" + std::to_string(m_ri.numero) + "]", CL_FILE_LOG_AND_CONSOLE));
	#endif

		}

#if defined(_WIN32)
		LeaveCriticalSection(&m_cs_sync_finish_game);
#elif defined(__linux__)
		pthread_mutex_unlock(&m_cs_sync_finish_game);
#endif

	}catch (exception& e) {

#if defined(_WIN32)
		LeaveCriticalSection(&m_cs_sync_finish_game);
#elif defined(__linux__)
		pthread_mutex_unlock(&m_cs_sync_finish_game);
#endif

		_smp::message_pool::getInstance().push(new message("[Practice::timeIsOver][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

bool Practice::init_game() {

	if (m_players.size() > 0) {

		startTime();

		initGameTime();

		m_game_init_state = 1;

		m_practice_state = true;
	}

	return true;
}

void Practice::requestReplySyncShotData(player& _session) {
	CHECK_SESSION_BEGIN("requestReplySyncShotData");

	try {

		sendSyncShot(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Practice::requestReplySyncShotData][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void Practice::requestSavePang(player& _session) {

	INIT_PLAYER_INFO("requestSavePang", "tentou salvar os pang ganho no jogo", &_session);

	if (pgi->data.pang > 0 || pgi->data.bonus_pang > 0)
		_session.m_pi.addPang(pgi->data.pang + pgi->data.bonus_pang);
}

void Practice::requestFinishExpGame() {

	if (m_players.size() > 0) {

		float stars = m_course->getStar();

		auto hole_seq = 0l;

		for (auto& el : m_players) {

			INIT_PLAYER_INFO("requestFinishExpGame", "tentou finalizar exp do jogo", el);

			hole_seq = (int)m_course->findHoleSeq(pgi->hole);

			if (hole_seq == 1 && !pgi->shot_sync.state_shot.display.stDisplay.acerto_hole)
				hole_seq = 0;

			if (el->m_pi.level < 70 )
				pgi->data.exp = (uint32_t)((hole_seq > 0 ? hole_seq : 0) * TRANSF_SERVER_RATE_VALUE(m_rv.exp) * TRANSF_SERVER_RATE_VALUE(pgi->used_item.rate.exp));

			_smp::message_pool::getInstance().push(new message("[Practice::requestFinishExpGame][Log] player[UID=" + std::to_string(el->m_pi.uid) + "] ganhou " + std::to_string(pgi->data.exp) + " de experience.", CL_FILE_LOG_AND_CONSOLE));

		}
	}
}

void Practice::finish() {

	m_game_init_state = 2;

	requestCalculeRankPlace();

	requestFinishExpGame();

	for (auto& el : m_players) {

		INIT_PLAYER_INFO("finish", "tentou finalizar os dados do jogador no jogo", el);

		if (pgi->flag != PlayerGameInfo::eFLAG_GAME::QUIT)
			requestFinishData(*el);
	}
}

void Practice::requestFinishData(player& _session) {

	requestFinishItemUsedGame(_session);

	requestSaveDrop(_session);

	INIT_PLAYER_INFO("requestFinishData", "tentou finalizar dados do jogo", &_session);

	sendDropItem(_session);

	sendPlacar(_session);
}

int Practice::checkEndShotOfHole(player& _session) {

	INIT_PLAYER_INFO("checkEndShotOfHole", "tentou verificar a ultima tacada do hole no jogo", &_session);

	if (pgi->shot_sync.state_shot.display.stDisplay.acerto_hole || pgi->data.giveup) {

		if (m_course->findHoleSeq(pgi->hole) == m_ri.qntd_hole) {

			packet p((unsigned short)0x199);

			packet_func::session_send(p, &_session, 1);

			if (pgi->shot_sync.state_shot.display.stDisplay.clear_bonus) {

				if (!sMap::getInstance().isLoad())
					sMap::getInstance().load();

				auto map = sMap::getInstance().getMap(m_ri.course & 0x7F);

				if (map == nullptr)
					_smp::message_pool::getInstance().push(new message("[TourneyBase::checkEndShotOfHole][Error][WARNING] tentou pegar o Map dados estaticos do course[COURSE="
						+ std::to_string((unsigned short)(m_ri.course & 0x7F)) + "], mas nao conseguiu encontra na classe do Server.", CL_FILE_LOG_AND_CONSOLE));
				else
					pgi->data.bonus_pang += sMap::getInstance().calculeClear30s(*map, m_ri.qntd_hole);
			}
		}

		finishHole(_session);

		changeHole(_session);
	}

	return 0;
}

void Practice::requestCalculeShotSpinningCube(player& _session, ShotSyncData& _ssd) {
	CHECK_SESSION_BEGIN("requestCalculeShotSpinningCube");

	try {

#ifdef _DEBUG

		calcule_shot_to_spinning_cube(_session, _ssd);
#endif

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Practice::requestCalculeShotSpinningCube][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void Practice::requestCalculeShotCoin(player& _session, ShotSyncData& _ssd) {
	CHECK_SESSION_BEGIN("requestCalculeShotCoin");

	try {

#ifdef _DEBUG

		calcule_shot_to_coin(_session, _ssd);
#endif

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Practice::requestCalculeShotCoin][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

bool Practice::finish_game(player& _session, int option) {

	if (
#if defined(_WIN32)
		_session.m_sock != INVALID_SOCKET
#elif defined(__linux__)
		_session.m_sock.fd != INVALID_SOCKET
#endif
	&& _session.getState() && _session.isConnected() && m_players.size() > 0) {

		if (option == 6 ) {

			packet p;

			if (m_practice_state)
				finish_practice(_session, 1);

			INIT_PLAYER_INFO("finish_game", "tentou terminar o jogo", &_session);

			requestSavePang(_session);

			if (pgi->data.exp > 0)
				_session.addExp(pgi->data.exp, false );

			sendUpdateInfoAndMapStatistics(_session, 0);

			requestSendTreasureHunterItem(_session);

			pgi->sys_achieve.finish_and_update(_session);

			p.init_plain((unsigned short)0xC8);

			p.addUint64(_session.m_pi.ui.pang);

			p.addUint64(0ull);

			packet_func::session_send(p, &_session, 1);

			pgi->finish_game = 1;

			m_game_init_state = 2;
		}
	}

	return (PlayersCompleteGameAndClear() && m_practice_state);
}
