
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "chip_in_practice.hpp"

#include "../PACKET/packet_func_sv.h"

#define CHECK_SESSION_BEGIN(method) if (!_session.getState()) \
										throw exception("[ChipInPractice" + std::string((method)) +"][Error] player nao esta connectado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHIP_IN_PRACTICE, 1, 0)); \

#define REQUEST_BEGIN(method) CHECK_SESSION_BEGIN(std::string("request") + (method)) \
							  if (_packet == nullptr) \
									throw exception("[ChipInPractice::request" + std::string((method)) +"][Error] _packet is nullptr", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHIP_IN_PRACTICE, 6, 0)); \

#define INIT_PLAYER_INFO(_method, _msg, __session) PlayerGrandZodiacInfo *pgi = reinterpret_cast< PlayerGrandZodiacInfo* >(getPlayerInfo((__session))); \
	if (pgi == nullptr) \
		throw exception("[ChipInPractice::" + std::string((_method)) + "][Error] player[UID=" + std::to_string((__session)->m_pi.uid) + "] " + std::string((_msg)) + ", mas o game nao tem o info dele guardado. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHIP_IN_PRACTICE, 1, 4)); \

using namespace stdA;

ChipInPractice::ChipInPractice(std::vector< player* >& _players, RoomInfoEx& _ri, RateValue _rv, unsigned char _channel_rookie)
	: GrandZodiacBase(_players, _ri, _rv, _channel_rookie), m_chip_in_practice_state(false) {

	for (auto& el : m_players) {

		INIT_PLAYER_INFO("ChipInPractice", "tentou inicializar o counter item do Chip-in Practice", el);

		initAchievement(*el);

		pgi->sys_achieve.incrementCounter(0x6C40003Fu );
	}

	m_state = init_game();
}

ChipInPractice::~ChipInPractice() {

	m_chip_in_practice_state = false;

	if (m_game_init_state != 2)
		finish(2);

	while (!PlayersCompleteGameAndClear())
#if defined(_WIN32)
		Sleep(500);
#elif defined(__linux__)
		usleep(500000);
#endif

	deleteAllPlayer();

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("[ChipInPractice::~ChipInPractice][Log] ChipInPractice destroyed on Room[Number=" + std::to_string(m_ri.numero) + "]", CL_FILE_LOG_AND_CONSOLE));
#endif
}

void ChipInPractice::changeHole(player& _session) {

	try {

		nextHole(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[ChipInPractice::changeHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void ChipInPractice::finishHole(player& _session) {

	requestFinishHole(_session, 0);
}

void ChipInPractice::finish_chip_in_practice(player& _session, int _option) {

	if (m_players.size() > 0 && m_game_init_state == 1) {

		packet p;

		INIT_PLAYER_INFO("finish_chip_in_practice", "tentou terminar o Chip-in Practice no jogo", &_session);

		if (pgi->flag == PlayerGameInfo::eFLAG_GAME::PLAYING) {

			requestCalculePang(_session);

			updatePlayerAssist(_session);

			if (m_game_init_state == 1 && _option == 0) {

			}
		}

		setGameFlag(pgi, (_option == 0) ? PlayerGameInfo::eFLAG_GAME::FINISH : PlayerGameInfo::eFLAG_GAME::END_GAME);

		GetLocalTime(&pgi->time_finish);

		setEndGame(pgi);

		setFinishGameFlag(pgi, 1u);

		p.init_plain((unsigned short)0x1F2);

		packet_func::session_send(p, &_session, 1);

		if (AllCompleteGameAndClear() && m_game_init_state == 1)
			finish(_option);
	}
}

void ChipInPractice::timeIsOver() {

	if (m_game_init_state == 1 && m_players.size() > 0) {

		player* _session = nullptr;

		for (auto& el : m_player_info) {

			if (el.second->flag == PlayerGameInfo::eFLAG_GAME::PLAYING && (_session = findSessionByUID(el.second->uid)) != nullptr) {

				packet p((unsigned short)0x8D);

				p.addUint32(m_ri.time_30s);

				packet_func::session_send(p, _session, 1);
			}
		}

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[ChipInPractice::timeIsOver][Log] Tempo Acabou no Chip-in Practice. na sala[NUMERO=" + std::to_string(m_ri.numero) + "]", CL_FILE_LOG_AND_CONSOLE));
#endif

	}
}

bool ChipInPractice::init_game() {

	if (m_players.size() > 0) {

		initGameTime();

		m_game_init_state = 1;

		m_chip_in_practice_state = true;
	}

	return true;
}

void ChipInPractice::requestFinishExpGame() {

	int32_t exp = 0l;
	player* _session = nullptr;

	for (auto& el : m_player_info) {

		if (el.second != nullptr) {

			if (el.second->flag == PlayerGameInfo::eFLAG_GAME::FINISH) {

				if ((_session = findSessionByUID(el.second->uid)) != nullptr) {

					exp = 45;
					exp = (int)(exp * TRANSF_SERVER_RATE_VALUE(el.second->used_item.rate.exp) * TRANSF_SERVER_RATE_VALUE(m_rv.exp));

					if (el.second->level < 70 )
						el.second->data.exp = exp;
				}

			}else if (el.second->flag == PlayerGameInfo::eFLAG_GAME::END_GAME) {

				exp = (int)(reinterpret_cast< PlayerGrandZodiacInfo* >(el.second)->m_gz.hole_in_one / 2);
				exp = (int)(exp * TRANSF_SERVER_RATE_VALUE(el.second->used_item.rate.exp) * TRANSF_SERVER_RATE_VALUE(m_rv.exp));

				if (el.second->level < 70 )
					el.second->data.exp = exp;
			}
		}
	}
}

void ChipInPractice::finish(int option) {

	m_game_init_state = 2;

	requestCalculeRankPlace();

	requestMakeTrofel();

	requestFinishExpGame();

	for (auto& el : m_players) {

		INIT_PLAYER_INFO("finish", "tentou finalizar os dados do jogador no jogo", el);

		if (pgi->flag != PlayerGameInfo::eFLAG_GAME::QUIT)
			requestFinishData(*el, option);
	}
}

void ChipInPractice::drawDropItem(player& _session) {
	return;
}

void ChipInPractice::requestFinishData(player& _session, int option) {

	packet p;

	try {

		INIT_PLAYER_INFO("requestFinishData", "tentou finalizar os dado do player no jogo", &_session);

		requestSaveInfo(_session, 0 );

		requestUpdateItemUsedGame(_session);

		requestFinishItemUsedGame(_session);

		requestSaveDrop(_session);

		sendTimeIsOver(_session);

		sendPlacar(_session);

		p.init_plain((unsigned short)0xC8);

		p.addUint64(_session.m_pi.ui.pang);

		p.addUint64(0ull);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0xA7);

		p.addUint8(0u);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0xAA);

		p.addUint16(0u);

		p.addUint64(_session.m_pi.ui.pang);
		p.addUint64(_session.m_pi.cookie);

		packet_func::session_send(p, &_session, 1);

		if (_session.m_pi.ei.mascot_info != nullptr) {
			packet_func::pacote06B(p, &_session, &_session.m_pi, 8);

			packet_func::session_send(p, &_session, 1);
		}

		pgi->sys_achieve.finish_and_update(_session);

		p.init_plain((unsigned short)0x24F);

		p.addUint32(0);

		packet_func::session_send(p, &_session, 1);

		if (pgi->data.exp > 0)
			_session.addExp(pgi->data.exp, true);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[ChipInPractice::requestFinishData][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void ChipInPractice::updateFinishHole(player& _session, int _option) {

	try {

		INIT_PLAYER_INFO("updateFinishHole", "tentou atualizar o finish hole do grand zodiac", &_session);

		packet p((unsigned short)0x1EE);

		p.addUint32(_session.m_oid);
		p.addFloat(pgi->location.x);
		p.addFloat(pgi->location.z);

		packet_func::game_broadcast(*this, p, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[ChipInPractice::updateFinishHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void ChipInPractice::requestMakeTrofel() {
	return;
}

void ChipInPractice::startGoldenBeam() {
	return;
}

void ChipInPractice::endGoldenBeam() {
	return;
}

bool ChipInPractice::finish_game(player& _session, int option) {

	if (
#if defined(_WIN32)
		_session.m_sock != INVALID_SOCKET
#elif defined(__linux__)
		_session.m_sock.fd != INVALID_SOCKET
#endif
	&& _session.getState() && _session.isConnected() && m_players.size() > 0) {

		packet p;

		if (option == 0x12C  || option == 2 ) {

			bool is_hacker_or_bug = false;

			if (m_timer != nullptr) {

				is_hacker_or_bug = ((int)(m_ri.time_30s - m_timer->getElapsed()) / (60 * 1000 )) >= 1 ? true : false;

				if (is_hacker_or_bug && option == 0x12C)
					_smp::message_pool::getInstance().push(new message("[ChipInPractice::finish_game][WARNING] Player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] na sala[NUMERO=" + std::to_string(m_ri.numero) + "] TEMPO[FINISH=" + std::to_string(m_timer->getElapsed()) + ", FINISH_CORRETO=" + std::to_string(m_ri.time_30s)
							+ "] pediu para terminar o Chip-in Practice com tempo menor que o do sala, pelo pacote normal, ele que ganhar exp, com menos tempo. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
			}

			if (m_chip_in_practice_state)
				finish_chip_in_practice(_session, (option == 0x12C && !is_hacker_or_bug) ? 0 : 1);
		}
	}

	return (PlayersCompleteGameAndClear() && m_chip_in_practice_state);
}
