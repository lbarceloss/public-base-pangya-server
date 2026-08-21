
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "pang_battle.hpp"
#include "treasure_hunter_system.hpp"
#include "../PACKET/packet_func_sv.h"
#include "../UTIL/map.hpp"

#define CHECK_SESSION_BEGIN(method) if (!_session.getState()) \
										throw exception("[PangBattle::request" + std::string((method)) +"][Error] player nao esta connectado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANG_BATTLE, 1, 0)); \

#define REQUEST_BEGIN(method) CHECK_SESSION_BEGIN(std::string("request") + (method)) \
							  if (_packet == nullptr) \
									throw exception("[PangBattle::request" + std::string((method)) +"][Error] _packet is nullptr", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANG_BATTLE, 6, 0)); \

#define INIT_PLAYER_INFO(_method, _msg, __session) auto pgi = getPlayerInfo((__session)); \
	if (pgi == nullptr) \
		throw exception("[PangBattle::" + std::string((_method)) + "][Error] player[UID=" + std::to_string((__session)->m_pi.uid) + "] " + std::string((_msg)) + ", mas o game nao tem o info dele guardado. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANG_BATTLE, 1, 4)); \

using namespace stdA;

PangBattle::PangBattle(std::vector< player* >& _players, RoomInfoEx& _ri, RateValue _rv, unsigned char _channel_rookie)
	: VersusBase(_players, _ri, _rv, _channel_rookie), m_pang_battle_state(false), m_pbd() {

	if (!sTreasureHunterSystem::getInstance().isLoad())
		sTreasureHunterSystem::getInstance().load();

	auto course = sTreasureHunterSystem::getInstance().findCourse((m_ri.course & 0x7F));

	if (course == nullptr)
		_smp::message_pool::getInstance().push(new message("[PangBattle::PangBattle][Error] tentou pegar o course do Treasure Hunter System, mas o course[COURSE="
				+ std::to_string((unsigned short)(m_ri.course & 0x7F)) + "] nao existe no sistema", CL_FILE_LOG_AND_CONSOLE));
	else
		sTreasureHunterSystem::getInstance().updateCoursePoint(*course, -1);

	initAllPlayerInfo();

	for (auto& el : m_players) {

		INIT_PLAYER_INFO("PangBattle", "tentou inicializar o counter item do Versus", el);

		initAchievement(*el);
	}

	init_pang_battle_data();

	m_pang_battle_state = init_game();
}

PangBattle::~PangBattle() {

	stopTime();

	for (auto& el : m_players)
		finish_game(*el);

	deleteAllPlayer();

	if (!m_player_order_pb.empty()) {
		m_player_order_pb.clear();
		m_player_order_pb.shrink_to_fit();
	}
}

bool PangBattle::deletePlayer(player* _session, int _option) {

	if (_session == nullptr)
		throw exception("[PangBattle::deletePlayer][Error] tentou deletar um player, mas o seu endereco eh nullptr.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::VERSUS, 50, 0));

	bool ret = false;

	try {

		m_state_vs.lock();

#if defined(_WIN32)
		EnterCriticalSection(&m_cs);
#elif defined(__linux__)
		pthread_mutex_lock(&m_cs);
#endif

		auto it = std::find(m_players.begin(), m_players.end(), _session);

		if (it != m_players.end()) {
			unsigned char opt = 3;

			INIT_PLAYER_INFO("deletePlayer", "tentou sair do jogo", _session);

			if (m_game_init_state == 1 ) {

				packet p;

				if (m_player_turn == pgi)
					stopTime();

				auto sessions = getSessions(*it);

				requestFinishItemUsedGame(*(*it));

				if (m_pbd.m_hole > 0 && m_pbd.m_hole <= (short)m_pbd.v_player_win.size()) {

					pgi->data.pang_pang_battle -= (m_pbd.v_player_win[m_pbd.m_hole - 1].pang * m_pbd.v_player_win[m_pbd.m_hole - 1].vezes);

					if (pgi->data.pang_pang_battle > 0 && m_players.size() > 1) {

						uint64_t div = pgi->data.pang_pang_battle / (m_players.size() - 1);

						pgi->data.pang_pang_battle = 0ll;

						for (auto& ell : m_players) {

							if (ell != nullptr && ell->m_pi.uid != pgi->uid) {

								try {

									auto _pgi = getPlayerInfo(ell);

									if (_pgi == nullptr) \
										throw exception("[PangBattle::deletePlayer][Error] player[UID=" + std::to_string(ell->m_pi.uid)
												+ "] tentou pegar o player para add os pangs do pang battle que o player saiu, mas o game nao tem o info dele guardado. Bug",
												STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANG_BATTLE, 1, 4));

									_pgi->data.pang_pang_battle += div;
									_pgi->data.pang_battle_run_hole = 1;

								}catch (exception& e) {

									_smp::message_pool::getInstance().push(new message("[PangBattle::deletePlayer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
								}
							}
						}
					}

					m_pbd.v_player_win[m_pbd.m_hole - 1].pang_extra += (m_pbd.v_player_win[m_pbd.m_hole - 1].pang * m_pbd.v_player_win[m_pbd.m_hole - 1].vezes);
				}

				pgi->data.pang_battle_run_hole = -1;

				_smp::message_pool::getInstance().push(new message("[PangBattle::deletePlayer][Log] Player[UID=" + std::to_string((*it)->m_pi.uid)
						+ "] Correu do Pang Battle, perdeu " + std::to_string(pgi->data.pang_pang_battle) + " Pang(s).", CL_FILE_LOG_AND_CONSOLE));

				requestSaveInfo(*(*it), (_option == 0x800) ? 5  : 1);

				setGameFlag(pgi, PlayerGameInfo::eFLAG_GAME::QUIT);

				p.init_plain((unsigned short)0x61);

				p.addUint32((*it)->m_oid);

				packet_func::vector_send(p, sessions, 1);

				p.init_plain((unsigned short)0x40);

				p.addUint8(8);

				p.addString((*it)->m_pi.nickname);

				p.addUint16(0);

				packet_func::vector_send(p, sessions, 1);

				sendUpdateInfoAndMapStatistics(*_session, -1);

				ret = checkNextStepGame(*_session);

			}else if (m_game_init_state == 2 && !pgi->finish_game) {

				requestSaveInfo(*(*it), 0);
			}

			if (m_game_init_state == 1  && pgi->data.bad_condute >= 3 && (pgi->data.time_out >= 3 || pgi->data.giveup >= 3)) {

				rain_hole_consecutivos_count(*_session);

				score_consecutivos_count(*_session);

				rain_count(*_session);

				pgi->sys_achieve.incrementCounter(0x6C400004u );

				pgi->sys_achieve.finish_and_update(*_session);

				packet p((unsigned short)0x244);

				p.addUint32(0);

				packet_func::session_send(p, _session, 1);

				p.init_plain((unsigned short)0x24F);

				p.addUint32(0);

				packet_func::session_send(p, _session, 1);
			}

			m_players.erase(it);
		}else
			_smp::message_pool::getInstance().push(new message("[PangBattle::deletePlayer][WARNING] player ja foi excluido do game.", CL_FILE_LOG_AND_CONSOLE));

#if defined(_WIN32)
		LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
		pthread_mutex_unlock(&m_cs);
#endif

		m_state_vs.unlock();

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[PangBattle::deletePlayer][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

#if defined(_WIN32)
		LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
		pthread_mutex_unlock(&m_cs);
#endif

		m_state_vs.unlock();
	}

	return ret;
}

void PangBattle::deleteAllPlayer() {

	while (!m_players.empty())
		deletePlayer(*m_players.begin(), 0);

}

void PangBattle::requestInitHole(player& _session, packet *_packet) {
	REQUEST_BEGIN("InitHole");

	try {

		VersusBase::requestInitHole(_session, _packet);

		INIT_PLAYER_INFO("requestInitHole", "tentou inicializar o hole", &_session);

		auto seq_hole = m_course->findHoleSeq(pgi->hole);

		if (m_pbd.m_hole == -1 || m_pbd.m_count_finish_hole < m_pbd.v_player_win.size())
			m_pbd.m_hole = seq_hole;
		else {

			m_pbd.m_hole_extra = seq_hole;

			m_pbd.m_hole_extra_flag = true;
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[PangBattle::requestInitHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void PangBattle::changeHole() {

	if (m_players.size() <= 0 || checkEndGame(**m_players.begin()))
		finish_pang_battle(0);
	else if (m_players.size() > 0)

		updateFinishHole();
}

void PangBattle::finishHole() {

	m_pbd.m_count_finish_hole++;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	for (auto& el : m_players) {

		requestFinishHole(*el, 0);

		requestUpdateItemUsedGame(*el);
	}

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif
}

void PangBattle::requestInitShot(player& _session, packet *_packet) {
	REQUEST_BEGIN("InitShot");

	packet p;

	try {

		INIT_PLAYER_INFO("requestInitShot", "tentou iniciar tacada no jogo", &_session);

#if defined(_WIN32)
		EnterCriticalSection(&m_cs_sync_shot);
#elif defined(__linux__)
		pthread_mutex_lock(&m_cs_sync_shot);
#endif

		if (pgi->init_shot == 1u) {

			_smp::message_pool::getInstance().push(new message("[PangBattle::requestInitShot][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] o server ja recebeu o pacote12 Init Shot. ignora esse.", CL_FILE_LOG_AND_CONSOLE));

#if defined(_WIN32)
			LeaveCriticalSection(&m_cs_sync_shot);
#elif defined(__linux__)
			pthread_mutex_unlock(&m_cs_sync_shot);
#endif

			return;

		}else
			pgi->init_shot = 1u;

#if defined(_WIN32)
		LeaveCriticalSection(&m_cs_sync_shot);
#elif defined(__linux__)
		pthread_mutex_unlock(&m_cs_sync_shot);
#endif

		pgi->bar_space.setState(0);

		stopTime();

		pgi->tempo = 0;

		ShotDataEx sd{ 0 };

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[PangBattle::requestInitShot][Log] Hex: " + hex_util::BufferToHexString((unsigned char*)_packet->getBuffer(), _packet->getSize()), CL_FILE_LOG_AND_CONSOLE));
#endif

		sd.option = _packet->readUint16();

		if (sd.option == 1)
			_packet->readBuffer(&sd.power_shot, sizeof(sd.power_shot));

		_packet->readBuffer(&sd, sizeof(ShotDataBase));

		pgi->shot_data = sd;

		m_state_vs.setStateWithLock(STATE_VERSUS::SHOTING);

#ifdef _DEBUG

		_smp::message_pool::getInstance().push(new message("Log Shot Data Ex:\n\r" + sd.toString(), CL_FILE_LOG_AND_CONSOLE));
#endif

		p.init_plain((unsigned short)0x55);

		p.addUint32(_session.m_oid);

		p.addBuffer(&sd, sizeof(ShotData));

		packet_func::game_broadcast(*this, p, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[PangBattle::requestInitShot][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void PangBattle::finish_pang_battle(int _option) {

	if (m_players.size() > 0 && m_game_init_state == 1) {

		for (auto& el : m_players) {

			INIT_PLAYER_INFO("finish_pang_battle", "tentou terminar o versus", el);

			pgi->sys_achieve.incrementCounter(0x6C400004u );

			requestCalculePang(*el);

			updatePlayerAssist(*el);
		}

		finish();
	}
}

void PangBattle::timeIsOver(void* _quem) {

	VersusBase::timeIsOver(_quem);

	if (_quem != nullptr) {

		player* p = reinterpret_cast< player* >(_quem);

		INIT_PLAYER_INFO("timeIsOver", "tentou acabar o tempo do turno no jogo", p);

		pgi->tempo = 1u;

		if (pgi->bar_space.getState() == 0 && pgi == m_player_turn) {

			pgi->tempo = 0u;

			if (++pgi->data.time_out >= 3)

				pgi->data.bad_condute = 3;

			packet p((unsigned short)0x5C);

			p.addUint32(pgi->oid);

			packet_func::game_broadcast(*this, p, 1);
		}

	}else
		_smp::message_pool::getInstance().push(new message("[PangBattle::timeIsOver][WARNING] time is over executed without _quem, _quem is invalid(nullptr). Bug" , CL_FILE_LOG_AND_CONSOLE));
}

bool PangBattle::init_game() {

	auto lixo = VersusBase::init_game();

	if (m_players.size() > 0) {

		initGameTime();

		m_game_init_state = 1;

		m_pang_battle_state = true;
	}

	return true;
}

void PangBattle::requestTranslateFinishHoleData(player& _session, UserInfoEx& _ui) {
	CHECK_SESSION_BEGIN("requestTranslateFinishHole");

	try {

		INIT_PLAYER_INFO("requestTranslateFinishHoleData", "tentou finalizar hole dados no jogo", &_session);

		pgi->ui = _ui;

		if (!pgi->shot_sync.state_shot.display.stDisplay.acerto_hole && (m_pbd.m_hole <= 0 || m_pbd.v_player_win[m_pbd.m_hole -1].player_win == -3)) {

			auto hole = m_course->findHole(pgi->hole);

			if (hole == nullptr)
				throw exception("[PangBattle::requestFinishHoleData][Error] player[UID=" + std::to_string(pgi->uid) + "] tentou finalizar os dados do hole no jogo, mas o hole[NUMERO="
						+ std::to_string(pgi->hole) + "] nao existe no course. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::VERSUS, 400, 0));

			if (pgi->data.tacada_num < hole->getPar().total_shot)
				pgi->data.tacada_num++;

			if (!pgi->data.giveup) {
				pgi->data.giveup = 1;

				pgi->data.bad_condute++;
			}
		}

		pgi->progress.best_chipin = _ui.best_chip_in;
		pgi->progress.best_long_puttin = _ui.best_long_putt;
		pgi->progress.best_drive = _ui.best_drive;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[PangBattle::requestTranslateFinishHoleData][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

bool PangBattle::checkEndGame(player& _session) {

	INIT_PLAYER_INFO("checkEndGame", "tentou verificar se eh o final do jogo", &_session);

	return (( m_pbd.m_count_finish_hole >= m_ri.qntd_hole && (m_pbd.m_hole <= 0 || m_pbd.v_player_win[m_pbd.m_hole - 1].player_win >= 0)) || m_players.size() == 1);
}

void PangBattle::requestSaveInfo(player& _session, int option) {

	INIT_PLAYER_INFO("requestSaveInfo", "tentou salvar o info dele no jogo", &_session);

	try {

		auto tmp_holein = pgi->ui.hole_in;

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[PangBattle::requestSaveInfo][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "] UserInfo[" + pgi->ui.toString() + "]", CL_FILE_LOG_AND_CONSOLE));
#endif

		pgi->ui.hole_in = pgi->ui.mad_conduta;
		pgi->ui.mad_conduta = tmp_holein;

		int32_t score = 0;

		for (auto it = 0u; it < (unsigned short)m_pbd.m_hole && it < m_ri.qntd_hole ; ++it)
			score += pgi->progress.score[it];

		pgi->ui.skin_run_hole = pgi->data.pang_battle_run_hole;

		pgi->ui.skin_pang = pgi->data.pang_pang_battle;

		if (option == 0) {

			if (m_ri.angel_event) {

				pgi->ui.quitado = -1;

				_smp::message_pool::getInstance().push(new message("[PangBattle::requestSaveInfo][Log][AngelEvent] Player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] vai reduzir o quit em " + std::to_string(pgi->ui.quitado * -1) + " unidade(s).", CL_FILE_LOG_AND_CONSOLE));
			}

			pgi->ui.exp = 0;
			pgi->ui.combo = 1;
			pgi->ui.jogado = 1;

			pgi->ui.media_score = score;

			pgi->ui.jogados_disconnect = 1;

			auto diff = getLocalTimeDiff(m_start_time);

			if (diff > 0)
				diff /= STDA_10_MICRO_PER_SEC;

			pgi->ui.tempo = (uint32_t)diff;

		}else if (option == 1) {

			pgi->data.pang = 0u;
			pgi->data.bonus_pang = 0u;

			pgi->ui.exp = 0;
			pgi->ui.combo = DECREASE_COMBO_VALUE * -1;
			pgi->ui.jogado = 1;

			if (!_session.m_connection_timeout)
				pgi->ui.quitado = 1;
			else
				pgi->ui.disconnect = 1;

			pgi->ui.jogados_disconnect = 1;

			pgi->ui.media_score = score;

			auto diff = getLocalTimeDiff(m_start_time);

			if (diff > 0)
				diff /= STDA_10_MICRO_PER_SEC;

			pgi->ui.tempo = (uint32_t)diff;

		}else if (option == 2) {

			pgi->data.pang = 0u;
			pgi->data.bonus_pang = 0u;

			pgi->ui.exp = 0;
			pgi->ui.jogado = 1;

			pgi->ui.jogados_disconnect = 1;

			auto diff = getLocalTimeDiff(m_start_time);

			if (diff > 0)
				diff /= STDA_10_MICRO_PER_SEC;

			pgi->ui.tempo = (uint32_t)diff;

		}else if (option == 4) {

			pgi->ui.clear();

			if (m_ri.angel_event) {

				pgi->ui.quitado = -1;

				_smp::message_pool::getInstance().push(new message("[PangBattle::requestSaveInfo][Log][AngelEvent] Player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] vai reduzir o quit em " + std::to_string(pgi->ui.quitado * -1) + " unidade(s).", CL_FILE_LOG_AND_CONSOLE));
			}

			pgi->ui.exp = 0;
			pgi->ui.combo = 1;
			pgi->ui.jogado = 1;
			pgi->ui.media_score = 0;

			pgi->ui.jogados_disconnect = 1;

			auto diff = getLocalTimeDiff(m_start_time);

			if (diff > 0)
				diff /= STDA_10_MICRO_PER_SEC;

			pgi->ui.tempo = (uint32_t)diff;

		}else if (option == 5 ) {

			pgi->data.pang = 0u;
			pgi->data.bonus_pang = 0u;

			pgi->ui.exp = 0;
			pgi->ui.jogado = 1;

			pgi->ui.media_score = score;

			pgi->ui.jogados_disconnect = 1;

			auto diff = getLocalTimeDiff(m_start_time);

			if (diff > 0)
				diff /= STDA_10_MICRO_PER_SEC;

			pgi->ui.tempo = (uint32_t)diff;
		}

		records_player_achievement(_session);

		int64_t total_pang = pgi->data.pang + pgi->data.bonus_pang + (m_pbd.m_player_win_pb == pgi->oid ? (int64_t)(pgi->data.pang_pang_battle * 0.95f) : pgi->data.pang_pang_battle);

		_session.m_pi.addUserInfo(pgi->ui, total_pang);

		if (total_pang > 0)
			_session.m_pi.addPang(total_pang);
		else if (total_pang < 0)
			_session.m_pi.consomePang(total_pang * -1);

		if (_session.m_pi.ui.combo > 0)
			pgi->sys_achieve.incrementCounter(0x6C40004Bu , _session.m_pi.ui.combo);

	}catch (exception& e) {
		_smp::message_pool::getInstance().push(new message("[PangBattle::requestSaveInfo][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void PangBattle::finish() {

	m_pang_battle_state = false;

	m_game_init_state = 2;

	requestCalculeRankPlace();

	calculePlayerWinPangBattle();

	requestDrawTreasureHunterItem();

	for (auto& el : m_players) {

		INIT_PLAYER_INFO("finish", "tentou finalizar os dados do jogador no jogo", el);

		if (pgi->flag != PlayerGameInfo::eFLAG_GAME::QUIT)
			requestFinishData(*el);
	}
}

void PangBattle::requestFinishData(player& _session) {

	requestFinishItemUsedGame(_session);

	requestSaveDrop(_session);

	rain_hole_consecutivos_count(_session);

	score_consecutivos_count(_session);

	rain_count(_session);

	sendTreasureHunterItemDrawGUI(_session);

	sendDropItem(_session);

	sendPlacar(_session);
}

void PangBattle::requestFinishHole(player& _session, int option) {

	INIT_PLAYER_INFO("requestFinishHole", "tentou finalizar o dados do hole do player no jogo", &_session);

	auto hole = m_course->findHole(pgi->hole);

	if (hole == nullptr)
		throw exception("[PangBattle::finishHole][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou finalizar hole[NUMERO="
				+ std::to_string((unsigned short)pgi->hole) + "] no jogo, mas o numero do hole is invalid. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANG_BATTLE, 20, 0));

	char score_hole = 0;
	uint32_t tacada_hole = 0u;

	if (option == 0) {

		pgi->data.total_tacada_num += pgi->data.tacada_num;

		score_hole = (char)(pgi->data.tacada_num - hole->getPar().par);

		tacada_hole = pgi->data.tacada_num;

		auto tmp_counter_typeid = SysAchievement::getScoreCounterTypeId(tacada_hole, hole->getPar().par);

		if (tmp_counter_typeid > 0)
			pgi->sys_achieve.incrementCounter(tmp_counter_typeid);

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[PangBattle::requestFinishHole][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] terminou o hole[COURSE="
				+ std::to_string(hole->getCourse()) + ", NUMERO=" + std::to_string(hole->getNumero()) + ", PAR="
				+ std::to_string(hole->getPar().par) + ", SHOT=" + std::to_string(tacada_hole) + ", SCORE=" + std::to_string(score_hole) + ", TOTAL_SHOT="
				+ std::to_string(pgi->data.total_tacada_num) + ", TOTAL_SCORE=" + std::to_string(pgi->data.score) + "]", CL_FILE_LOG_AND_CONSOLE));
#endif

		pgi->data.time_out = 0u;

		pgi->data.giveup = 0u;

		pgi->data.penalidade = 0u;

	}else if (option == 1) {

		auto pair = m_course->findRange(pgi->hole);

		for (auto it = pair.first; it != pair.second && it->first <= m_ri.qntd_hole ; ++it) {
			pgi->data.total_tacada_num += it->second.getPar().total_shot;

			pgi->data.score += it->second.getPar().range_score[1];
		}

		pgi->data.time_out = 0u;

		pgi->data.tacada_num = 0u;

		pgi->data.giveup = 0u;

		pgi->data.penalidade = 0u;
	}

	pgi->progress.hole = (short)m_course->findHoleSeq(pgi->hole);

	if (option == 0) {

		if (pgi->progress.hole > 0) {

			if (pgi->shot_sync.state_shot.display.stDisplay.acerto_hole)
				pgi->progress.finish_hole[pgi->progress.hole - 1] = 1;

			pgi->progress.par_hole[pgi->progress.hole - 1] = hole->getPar().par;
			pgi->progress.score[pgi->progress.hole - 1] = score_hole;
			pgi->progress.tacada[pgi->progress.hole - 1] = tacada_hole;
		}

	}else {

		auto pair = m_course->findRange(pgi->hole);

		for (auto it = pair.first; it != pair.second && it->first <= m_ri.qntd_hole ; ++it) {

			pgi->progress.finish_hole[it->first - 1] = 0;

			pgi->progress.par_hole[it->first - 1] = it->second.getPar().par;

			pgi->progress.score[it->first - 1] = it->second.getPar().range_score[1];

			pgi->progress.tacada[it->first - 1] = it->second.getPar().total_shot;
		}
	}
}

bool PangBattle::checkNextStepGame(player& _session) {

	auto ret = false;

	try {

		INIT_PLAYER_INFO("checkNextStepGame", "tentou verificar o proximo passo do jogo", &_session);

		auto seq = m_course->findHoleSeq(pgi->hole);

		if (seq == 0 || seq == (unsigned short)~0)
			throw exception("[PangBattle::checkNextStepGame][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou pegar sequencia do hole[NUMERO="
					+ std::to_string(pgi->hole) + ", SEQ=" + std::to_string(seq) + "], mas nao encontrou course. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANG_BATTLE, 500, 0));

		if (m_players.size() == 2) {

			if (m_player_turn == nullptr) {

				m_state_vs.setStateWithLock(STATE_VERSUS::WAIT_END_GAME);

				ret = true;

			}else if (m_player_turn == pgi) {

				m_state_vs.setStateWithLock(STATE_VERSUS::WAIT_END_GAME);

				ret = true;

			}else if (!checkPlayerTurnExistOnGame()) {

				m_state_vs.setStateWithLock(STATE_VERSUS::WAIT_END_GAME);

				ret = true;

			}else
				m_flag_next_step_game = 2;

		}else if (m_players.size() == 1) {

			if (m_player_turn == nullptr) {

				m_state_vs.setStateWithLock(STATE_VERSUS::WAIT_END_GAME);

				ret = true;

			}else if (m_player_turn == pgi) {

				m_state_vs.setStateWithLock(STATE_VERSUS::WAIT_END_GAME);

				ret = true;

			}else if (!checkPlayerTurnExistOnGame()) {

				m_state_vs.setStateWithLock(STATE_VERSUS::WAIT_END_GAME);

				ret = true;

			}else
				m_flag_next_step_game = 2;

		}else if(m_player_turn == nullptr) {

			m_state_vs.setStateWithLock(STATE_VERSUS::WAIT_END_GAME);

			ret = true;

		}else
			m_flag_next_step_game = 3;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[PangBattle::checkNextStepGame][ErroSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	return ret;
}

bool PangBattle::checkAllClearHole() {

	bool draw = false;
	uint32_t best_shot = 100u;
	PlayerGameInfo *p = nullptr;

	if (m_pbd.m_hole <= (short)m_pbd.v_player_win.size() && m_pbd.v_player_win[m_pbd.m_hole - 1].player_win >= 0
			|| (m_pbd.v_player_win[m_pbd.m_hole - 1].player_win == -1  && !m_pbd.m_hole_extra_flag))
		return true;

	if (m_pbd.m_hole == m_pbd.v_player_win.size() && m_pbd.m_hole_extra_flag) {

		auto v_player_order = init_player_order_top_shot_approach();

		if (v_player_order.empty()) {

			_smp::message_pool::getInstance().push(new message("[PangBattle::checkAllClearHole][Error][WARNING] nao conseguiu inicializar os players order top shot approach, terminando o jogo. Bug",
					CL_FILE_LOG_AND_CONSOLE));

			return true;
		}

		try {

			uint32_t count_player_shot = 0u;

#if defined(_WIN32)
			EnterCriticalSection(&m_cs_sync_shot);
#elif defined(__linux__)
			pthread_mutex_lock(&m_cs_sync_shot);
#endif

			std::for_each(v_player_order.begin(), v_player_order.end(), [&](auto& _el) {

#ifdef _DEBUG
				_smp::message_pool::getInstance().push(new message("[PangBattle::checkAllClearHole][Log] Player[UID=" + std::to_string(_el.pgi->uid)
						+ "] dados\n\tMH: " + (_el.pgi->shot_sync.isMakeHole() ? "true" : "false") + "\tGU: " + (_el.pgi->data.giveup ? "true" : "false")
						+ "\tSN: " + std::to_string(_el.pgi->data.tacada_num), CL_FILE_LOG_AND_CONSOLE));
#endif

				try {

					if (_el.pgi->data.tacada_num > 0)
						count_player_shot++;

					if (_el.pgi->shot_sync.isMakeHole() && p == nullptr)
						draw = true;
					else if (!_el.pgi->shot_sync.isMakeHole() && p == nullptr)
						p = _el.pgi;

				}catch (exception& e) {

					_smp::message_pool::getInstance().push(new message("[PangBattle::checkAllClearHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
				}
			});

			if (m_pbd.m_hole <= 0)
				_smp::message_pool::getInstance().push(new message("[PangBattle::checkAllClearHole][Error][WARNING] nao conseguiu atualizar os dados do pang battle por que o hole is invalid("
						+ std::to_string(m_pbd.m_hole) + "). Bug.", CL_FILE_LOG_AND_CONSOLE));
			else {

				auto& ph = m_pbd.v_player_win[m_pbd.m_hole - 1];

				if (count_player_shot == v_player_order.size() && draw) {

					ph.player_win = -1;

#ifdef _DEBUG
					_smp::message_pool::getInstance().push(new message("[PangBattle::checkAllClearHole][Log] Hole[NUMERO=" + std::to_string(m_pbd.m_hole) + "][EXTRA] empatou.", CL_FILE_LOG_AND_CONSOLE));
#endif

					if (m_pbd.m_hole < (short)m_pbd.v_player_win.size() && !m_pbd.m_hole_extra_flag) {

						m_pbd.v_player_win[m_pbd.m_hole].pang += ph.pang;
						m_pbd.v_player_win[m_pbd.m_hole].vezes = (ph.vezes < 8) ? ph.vezes * 2u : 8u;
						m_pbd.v_player_win[m_pbd.m_hole].pang_extra = ph.pang_extra;
					}

				}else if (count_player_shot == v_player_order.size() && p != nullptr || (m_players.size() == 1 && (p = getPlayerInfo(m_players[0])) != nullptr)) {

					p->data.score++;
					p->data.pang_pang_battle += ph.pang * ph.vezes * ((m_players.size() > 0) ? m_players.size() - 1 : 0) + ph.pang_extra;

					ph.player_win = p->oid;

	#ifdef _DEBUG
					_smp::message_pool::getInstance().push(new message("[PangBattle::checkAllClearHole][Log] Player[UID=" + std::to_string(p->uid)
							+ "] Ganhou o hole[NUMERO=" + std::to_string(p->hole) + "][EXTRA]", CL_FILE_LOG_AND_CONSOLE));
	#endif

					for (auto& el : m_players) {

						if (el != nullptr && el->m_pi.uid != p->uid) {

							try {

								INIT_PLAYER_INFO("checkAllClearHole", "tentou atualizar os dados do Pang Battle", el);

								if (pgi->flag != PlayerGameInfo::eFLAG_GAME::QUIT)
									pgi->data.pang_pang_battle -= ph.pang * ph.vezes;

							}catch (exception& e) {

								_smp::message_pool::getInstance().push(new message("[PangBattle::checkAllClearHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
							}
						}
					}

				}else if (count_player_shot != v_player_order.size() && p != nullptr) {

					p = nullptr;
					draw = false;
				}
			}

#if defined(_WIN32)
			LeaveCriticalSection(&m_cs_sync_shot);
#elif defined(__linux__)
			pthread_mutex_unlock(&m_cs_sync_shot);
#endif

		}catch (exception& e) {

#if defined(_WIN32)
			LeaveCriticalSection(&m_cs_sync_shot);
#elif defined(__linux__)
			pthread_mutex_unlock(&m_cs_sync_shot);
#endif

			_smp::message_pool::getInstance().push(new message("[PangBattle::checkAllClearHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		}

	}else {

		init_player_order_top_shot();

		try {

#if defined(_WIN32)
			EnterCriticalSection(&m_cs_sync_shot);
#elif defined(__linux__)
			pthread_mutex_lock(&m_cs_sync_shot);
#endif

			std::for_each(m_player_order_pb.begin(), m_player_order_pb.end(), [&](auto& _el) {

#ifdef _DEBUG
				_smp::message_pool::getInstance().push(new message("[PangBattle::checkAllClearHole][Log] Player[UID=" + std::to_string(_el->uid)
						+ "] dados\n\tMH: " + (_el->shot_sync.isMakeHole() ? "true" : "false") + "\tGU: " + (_el->data.giveup ? "true" : "false")
						+ "\tSN: " + std::to_string(_el->data.tacada_num), CL_FILE_LOG_AND_CONSOLE));
#endif

				try {

					if (_el->shot_sync.isMakeHole() || _el->data.giveup) {

						if (p != nullptr) {

							if (p->data.tacada_num == _el->data.tacada_num && _el->data.tacada_num <= best_shot)
								draw = true;

						}else if (best_shot == 100u || _el->data.tacada_num <= best_shot)
							p = _el;

					}else {

						if (best_shot > _el->data.tacada_num)
							best_shot = _el->data.tacada_num;

						if (p != nullptr) {

							if ((_el->data.tacada_num + 1) <= p->data.tacada_num)
								p = nullptr;

						}
					}

				}catch (exception& e) {

					_smp::message_pool::getInstance().push(new message("[PangBattle::checkAllClearHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
				}
			});

			if (m_pbd.m_hole <= 0)
				_smp::message_pool::getInstance().push(new message("[PangBattle::checkAllClearHole][Error][WARNING] nao conseguiu atualizar os dados do pang battle por que o hole is invalid("
						+ std::to_string(m_pbd.m_hole) + "). Bug.", CL_FILE_LOG_AND_CONSOLE));
			else {

				auto& ph = m_pbd.v_player_win[m_pbd.m_hole - 1];

				if (draw) {

					ph.player_win = -1;

#ifdef _DEBUG
					_smp::message_pool::getInstance().push(new message("[PangBattle::checkAllClearHole][Log] Hole[NUMERO=" + std::to_string(m_pbd.m_hole) + "] empatou.", CL_FILE_LOG_AND_CONSOLE));
#endif

					if (m_pbd.m_hole < (short)m_pbd.v_player_win.size()) {

						m_pbd.v_player_win[m_pbd.m_hole].pang += ph.pang;
						m_pbd.v_player_win[m_pbd.m_hole].vezes = (ph.vezes < 8) ? ph.vezes * 2u : 8u;
						m_pbd.v_player_win[m_pbd.m_hole].pang_extra = ph.pang_extra;
					}

				}else if (p != nullptr || (m_players.size() == 1 && (p = getPlayerInfo(m_players[0])) != nullptr)) {

					p->data.score++;
					p->data.pang_pang_battle += ph.pang * ph.vezes * ((m_players.size() > 0) ? m_players.size() - 1 : 0) + ph.pang_extra;

					ph.player_win = p->oid;

	#ifdef _DEBUG
					_smp::message_pool::getInstance().push(new message("[PangBattle::checkAllClearHole][Log] Player[UID=" + std::to_string(p->uid)
							+ "] Ganhou o hole[NUMERO=" + std::to_string(p->hole) + "]", CL_FILE_LOG_AND_CONSOLE));
	#endif

					for (auto& el : m_players) {

						if (el != nullptr && el->m_pi.uid != p->uid) {

							try {

								INIT_PLAYER_INFO("checkAllClearHole", "tentou atualizar os dados do Pang Battle", el);

								if (pgi->flag != PlayerGameInfo::eFLAG_GAME::QUIT)
									pgi->data.pang_pang_battle -= ph.pang * ph.vezes;

							}catch (exception& e) {

								_smp::message_pool::getInstance().push(new message("[PangBattle::checkAllClearHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
							}
						}
					}

				}
			}

#if defined(_WIN32)
			LeaveCriticalSection(&m_cs_sync_shot);
#elif defined(__linux__)
			pthread_mutex_unlock(&m_cs_sync_shot);
#endif

		}catch (exception& e) {

#if defined(_WIN32)
			LeaveCriticalSection(&m_cs_sync_shot);
#elif defined(__linux__)
			pthread_mutex_unlock(&m_cs_sync_shot);
#endif

			_smp::message_pool::getInstance().push(new message("[PangBattle::checkAllClearHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		}
	}

	return (draw || p != nullptr);
}

void PangBattle::updateFinishHole() {

	packet p((unsigned short)0x65);

	int32_t player_win = (m_pbd.m_hole <= 0 || (short)m_pbd.v_player_win.size() < m_pbd.m_hole ? -1 : m_pbd.v_player_win[m_pbd.m_hole - 1].player_win);

	p.addInt32(player_win);

	packet_func::game_broadcast(*this, p, 1);
}

void PangBattle::sendPlayerTurn() {

	if (m_player_turn == nullptr)
		throw exception("[PangBattle::sendPlayerTurn][Error] PlayerGameInfo *m_player_turn is invalid(nullptr). Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::VERSUS_BASE, 100, 1));

	auto hole = m_course->findHole(m_player_turn->hole);

	if (hole == nullptr)
		throw exception("[PangBattle::sendPlayerTurn][Error] player[UID=" + std::to_string(m_player_turn->uid) + "] tentou encontrar o hole[NUMERO="
				+ std::to_string(m_player_turn->hole) + "] do course no jogo, mas nao foi encontrado. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::VERSUS_BASE, 101, 0));

	auto wind = m_course->shuffleWind((uint32_t)(size_t)m_player_turn * 777u);

	m_player_turn->degree = wind.degree.getShuffleDegree();

	auto wind_flag = initCardWindPlayer(m_player_turn, wind.wind);

	packet p((unsigned short)0x5B);

	p.addUint8(wind.wind + wind_flag);
	p.addUint8(1u);
	p.addUint16(m_player_turn->degree);
	p.addUint8(0u);

	packet_func::game_broadcast(*this, p, 1);

	p.init_plain((unsigned short)0x63);

	if (m_player_turn == nullptr) {
		_smp::message_pool::getInstance().push(new message("[PangBattle::sendPlayerTurn][Error] player_turn is invalid(nullptr)", CL_FILE_LOG_AND_CONSOLE));

		p.addUint32(0);
	}else
		p.addUint32(m_player_turn->oid);

	p.addUint16(calcMsgToPlayerMakeHole(m_player_turn));

	packet_func::game_broadcast(*this, p, 1);
}

void PangBattle::changeTurn() {

	if (m_player_turn == nullptr)
		throw exception("[PangBattle::changeTurn][Error] PlayerGameInfo *m_player_turn is invalid(nullptr). Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::VERSUS_BASE, 100, 0));

	stopTime();

	if (m_player_turn->shot_sync.state_shot.display.stDisplay.acerto_hole || m_player_turn->data.giveup) {

		if (m_course->findHoleSeq(m_player_turn->hole) == m_ri.qntd_hole) {

			packet p((unsigned short)0x199);

			packet_func::game_broadcast(*this, p, 1);

			if (m_player_turn->shot_sync.state_shot.display.stDisplay.clear_bonus) {

				if (!sMap::getInstance().isLoad())
					sMap::getInstance().load();

				auto map = sMap::getInstance().getMap(m_ri.course & 0x7F);

				if (map == nullptr)
					_smp::message_pool::getInstance().push(new message("[PangBattle::changeTurn][Error][WARNING] tentou pegar o Map dados estaticos do course[COURSE="
							+ std::to_string((unsigned short)(m_ri.course & 0x7F)) + "], mas nao conseguiu encontra na classe do Server.", CL_FILE_LOG_AND_CONSOLE));
				else
					m_player_turn->data.bonus_pang += sMap::getInstance().calculeClearVS(*map, (uint32_t)m_players.size(), m_ri.qntd_hole);
			}
		}
	}

	clearDataEndShot(m_player_turn);

	if (checkAllClearHole()) {

		clear_all_flag_sync();

		finishHole();

		clearAllClearHole();

		changeHole();

	}else if (m_players.size() == 1  ) {

		clear_all_flag_sync();

		finishHole();

		changeHole();

	}else {

		clear_all_flag_sync();

		requestCalculePlayerTurn();

		sendPlayerTurn();
	}
}

void PangBattle::sendPlacar(player& _session) {

	packet p((unsigned short)0x66);

	p.addUint8((unsigned char)m_players.size());

	auto last_hole_index = (m_pbd.m_hole < (short)m_pbd.v_player_win.size() ? m_pbd.m_hole - 1 : m_pbd.v_player_win.size() - 1);

	if (m_pbd.v_player_win[last_hole_index].player_win != -1)
		p.addUint32(m_pbd.v_player_win[last_hole_index].player_win);
	else
		p.addUint32(m_pbd.m_player_win_pb);

	p.addUint32(m_pbd.m_player_win_pb);

	for (auto& el : m_players) {

		INIT_PLAYER_INFO("sendPlacar", "tentou enviar o placar do jogo", el);

		p.addUint32(el->m_oid);
		p.addUint8((unsigned char)getRankPlace(*el) + 1);
		p.addInt8((char)pgi->data.score);
		p.addInt8((unsigned char)pgi->data.total_tacada_num);

		p.addUint16((unsigned short)pgi->data.exp);
		p.addUint64(pgi->data.pang);
		p.addUint64(pgi->data.bonus_pang);

		p.addUint64(pgi->data.pang_pang_battle);
	}

	packet_func::session_send(p, &_session, 1);
}

void PangBattle::requestCalculeRankPlace() {

	if (!m_player_order.empty())
		m_player_order.clear();

	for (auto& el : m_player_info)
		if (el.second->flag != PlayerGameInfo::eFLAG_GAME::QUIT)
			m_player_order.push_back(el.second);

	std::sort(m_player_order.begin(), m_player_order.end(), PangBattle::sort_player_rank_place);
}

eMSG_MAKE_HOLE PangBattle::calcMsgToPlayerMakeHole(PlayerGameInfo* _pgi) {

	if (_pgi == nullptr) {

		_smp::message_pool::getInstance().push(new message("[PangBattle::calcMsgToPlayerMakeHole][Error] PlayerGameInfo _pgi is invalid(nullptr). Bug.", CL_FILE_LOG_AND_CONSOLE));

		return eMSG_MAKE_HOLE::MMH_PERDEU;
	}

	eMSG_MAKE_HOLE ret = eMSG_MAKE_HOLE::MMH_PERDEU;

	try {

#if defined(_WIN32)
		EnterCriticalSection(&m_cs);
#elif defined(__linux__)
		pthread_mutex_lock(&m_cs);
#endif

		if (m_player_info.size() > 0) {

			uint32_t c_perdeu = 0, c_ganhou = 0, c_empatou = 0;

			for (auto& el : m_player_info) {

				if (el.first != nullptr) {

					if ((_pgi->data.tacada_num + 1) < el.second->data.tacada_num || ((_pgi->data.tacada_num + 1) == el.second->data.tacada_num && !el.second->shot_sync.state_shot.display.stDisplay.acerto_hole))
						c_ganhou++;
					else if ((_pgi->data.tacada_num + 1) == el.second->data.tacada_num && el.second->shot_sync.state_shot.display.stDisplay.acerto_hole)
						c_empatou++;
					else
						c_perdeu++;

				}else
					_smp::message_pool::getInstance().push(new message("[PangBattle::calcMsgToPlayerMakeHole][WARNING] PlayerGameInfo[UID=" + std::to_string(el.second->uid)
							+ "] _session is invalid(nullptr). Bug.", CL_FILE_LOG_AND_CONSOLE));
			}

			if (c_ganhou == (m_player_info.size() - 1))
				ret = eMSG_MAKE_HOLE::MMH_GANHOU;
			else if (c_perdeu == m_player_info.size())
				ret = eMSG_MAKE_HOLE::MMH_PERDEU;
			else if (c_perdeu == 1 && (m_player_info.size() - c_ganhou - 1) == c_empatou)
				ret = eMSG_MAKE_HOLE::MMH_EMPATOU;
		}

#if defined(_WIN32)
		LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
		pthread_mutex_unlock(&m_cs);
#endif

	}catch (exception& e) {

#if defined(_WIN32)
		LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
		pthread_mutex_unlock(&m_cs);
#endif

		_smp::message_pool::getInstance().push(new message("[PangBattle::calcMsgToPlayerMakeHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	return ret;
}

void PangBattle::init_pang_battle_data() {

	uint32_t valor_pang = 10;

	for (int i = 0; i < m_ri.qntd_hole; i++) {

		if ((i + 1) == m_ri.qntd_hole)
			m_pbd.v_player_win.push_back(PangBattleHolePang(valor_pang * 2));
		else {

			if ((i % 3) == 0) {

				valor_pang += 10;

				m_pbd.v_player_win.push_back(PangBattleHolePang(valor_pang ));

			}else
				m_pbd.v_player_win.push_back(PangBattleHolePang(valor_pang ));
		}
	}
}

void PangBattle::calculePlayerWinPangBattle() {

	m_pbd.m_player_win_pb = (uint32_t)-1;

	if (m_players.size() == 0) {

		m_pbd.m_player_win_pb = m_players[0]->m_oid;

		return;
	}

	if (m_player_order.empty())
		requestCalculeRankPlace();

	int64_t pang_win = -1ll;

	for (auto& el : m_player_order) {

		if (pang_win == -1ll || el->data.pang_pang_battle > pang_win) {

			m_pbd.m_player_win_pb = el->oid;

			pang_win = el->data.pang_pang_battle;
		}
	}
}

void PangBattle::savePangBattleDados(player& _session) {

	if (!_session.getState()) {

		_smp::message_pool::getInstance().push(new message("[PangBattle::savePangBattleDados][Error] Player[UID=" + std::to_string(_session.m_pi.uid) +"] is invalid session.", CL_FILE_LOG_AND_CONSOLE));

		return;
	}

	try {

		INIT_PLAYER_INFO("savePangBattleDados", "tentou salvar os dados do Pang Battle", &_session);

		if (m_pbd.m_player_win_pb == (uint32_t)-1 || m_pbd.m_player_win_pb == _session.m_oid)
			pgi->ui.skin_win = 1;
		else
			pgi->ui.skin_lose = 1;

		pgi->ui.skin_all_in_count = 1;

		_smp::message_pool::getInstance().push(new message("[PangBattle::savePangBattleDados][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] " + (m_pbd.m_player_win_pb == _session.m_oid ? "Ganhou" : "Perdeu")
				+ " Pang Battle, com Pang: " + std::to_string(pgi->data.pang_pang_battle), CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[PangBattle::savePangBattleDados][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

bool PangBattle::sort_player_top_shot(PlayerGameInfo* _pgi1, PlayerGameInfo* _pgi2) {

	if ((_pgi1->shot_sync.isMakeHole() || _pgi1->data.giveup) && _pgi1->data.tacada_num <= _pgi2->data.tacada_num && (!_pgi2->shot_sync.isMakeHole() && !_pgi2->data.giveup))
		return true;
	else if ((_pgi1->shot_sync.isMakeHole() || _pgi1->data.giveup) && _pgi1->data.tacada_num < _pgi2->data.tacada_num && (_pgi2->shot_sync.isMakeHole() || _pgi2->data.giveup))
		return true;
	else if ((!_pgi1->shot_sync.isMakeHole() && _pgi1->data.giveup) && (_pgi1->data.tacada_num + 1 ) <= _pgi2->data.tacada_num && (_pgi2->shot_sync.isMakeHole() || _pgi2->data.giveup))
		return true;
	else if ((!_pgi1->shot_sync.isMakeHole() && !_pgi1->data.giveup) && _pgi1->data.tacada_num < _pgi2->data.tacada_num && (!_pgi2->shot_sync.isMakeHole() && !_pgi2->data.giveup))
		return true;

	return _pgi1->data.tacada_num < _pgi2->data.tacada_num;
}

#define APPROACH_IS_MAKE_HOLE(_pp) (bool)((_pp) != nullptr ? ((_pp)->shot_sync.isMakeHole() || (_pp)->shot_sync.state == ShotSyncData::SHOT_STATE::INTO_HOLE) : false)
#define CHECK_PGI_IS_NULL(__pgi) (bool)((__pgi == nullptr))

bool PangBattle::sort_player_top_shot_approach(PlayerOrderTurnCtx& _potc1, PlayerOrderTurnCtx& _potc2) {

	if (CHECK_PGI_IS_NULL(_potc1.pgi) || CHECK_PGI_IS_NULL(_potc2.pgi))
		return false;

	auto diff = _potc1.hole->getPinLocation().diffXZ(_potc1.pgi->location);
	auto diff2 = _potc1.hole->getPinLocation().diffXZ(_potc2.pgi->location);

	if (!APPROACH_IS_MAKE_HOLE(_potc1.pgi) && APPROACH_IS_MAKE_HOLE(_potc2.pgi))
		return true;
	else if (diff < diff2 && !APPROACH_IS_MAKE_HOLE(_potc1.pgi) && !APPROACH_IS_MAKE_HOLE(_potc2.pgi))
		return true;
	else if (diff == diff2 && _potc1.pgi->shot_data.time_shot < _potc2.pgi->shot_data.time_shot && !APPROACH_IS_MAKE_HOLE(_potc1.pgi) && !APPROACH_IS_MAKE_HOLE(_potc2.pgi))
		return true;
	else if (diff == diff2 && _potc1.pgi->shot_data.time_shot == _potc2.pgi->shot_data.time_shot && _potc1.pgi->data.tacada_num < _potc2.pgi->data.tacada_num
		&& !APPROACH_IS_MAKE_HOLE(_potc1.pgi) && !APPROACH_IS_MAKE_HOLE(_potc2.pgi))
		return true;

	return false;
}

bool PangBattle::sort_player_rank_place(PlayerGameInfo* _pgi1, PlayerGameInfo* _pgi2) {

	if (_pgi1->data.score == _pgi2->data.score)
		return _pgi1->data.pang_pang_battle > _pgi2->data.pang_pang_battle;

	return _pgi1->data.score > _pgi2->data.score;
}

inline void PangBattle::init_player_order_top_shot() {

	if (!m_player_order_pb.empty())
		m_player_order_pb.clear();

	for (auto& el : m_players) {

		if (el != nullptr) {

			try {

				INIT_PLAYER_INFO("init_player_order_top_shot", " tentou calcular o player que tem a melhor colocacao no jogo", el);

				if (pgi->flag != PlayerGameInfo::eFLAG_GAME::QUIT)
					m_player_order_pb.push_back(pgi);

			}catch (exception& e) {

				_smp::message_pool::getInstance().push(new message("[PangBattle::init_player_order_top_shot][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
			}
		}
	}

	std::sort(m_player_order_pb.begin(), m_player_order_pb.end(), PangBattle::sort_player_top_shot);
}

inline std::vector< PlayerOrderTurnCtx > PangBattle::init_player_order_top_shot_approach() {

	std::vector< PlayerOrderTurnCtx > v_player_order_turn;

	auto hole = m_course->findHoleBySeq((m_pbd.m_hole_extra >= 0 ?  m_pbd.m_hole_extra : m_pbd.m_hole));

	if (hole == nullptr)
		return v_player_order_turn;

	for (auto& el : m_players) {

		if (el != nullptr) {

			try {

				INIT_PLAYER_INFO("init_player_order_top_shot_approach", " tentou calcular o player order top shot approach", el);

				if (pgi->flag != PlayerGameInfo::eFLAG_GAME::QUIT)
					v_player_order_turn.push_back({ pgi, hole });

			}catch (exception& e) {

				_smp::message_pool::getInstance().push(new message("[PangBattle::init_player_order_top_shot_approach][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
			}
		}
	}

	if (v_player_order_turn.empty())
		return v_player_order_turn;

	std::sort(v_player_order_turn.begin(), v_player_order_turn.end(), PangBattle::sort_player_top_shot_approach);

	return v_player_order_turn;
}

bool PangBattle::finish_game(player& _session, int option) {

	if (
#if defined(_WIN32)
		_session.m_sock != INVALID_SOCKET
#elif defined(__linux__)
		_session.m_sock.fd != INVALID_SOCKET
#endif
	&& _session.getState() && _session.isConnected() && m_players.size() > 0) {

		INIT_PLAYER_INFO("finish_game", "tentou finalizar o jogo", &_session);

		if (pgi->shot_sync.state_shot.display.stDisplay.acerto_hole || pgi->data.giveup) {

			requestFinishHole(_session, 0);

			requestUpdateItemUsedGame(_session);
		}

		pgi->finish_game = 1u;

		if (PlayersCompleteGameAndClear() || option == 2 ) {

			packet p;

			if (m_course->findHoleSeq(pgi->hole) == 1 && !checkAllClearHole() && (pgi->progress.hole <= 0 || pgi->progress.finish_hole[pgi->progress.hole - 1] == 0 )) {

				for (auto& el : m_players) {

					INIT_PLAYER_INFO("finish_game", "tentou finalizar o versus", el);

					if (pgi->flag == PlayerGameInfo::eFLAG_GAME::PLAYING) {

						requestSaveInfo(*el, 2);

						if (pgi->finish_item_used == 0u)
							requestFinishItemUsedGame(*el);

						p.init_plain((unsigned short)0x67);

						packet_func::session_send(p, el, 1);

						setGameFlag(pgi, PlayerGameInfo::eFLAG_GAME::END_GAME);
					}
				}

				m_game_init_state = 2;

				return true;

			}else {

				if (m_pang_battle_state) {

					if (option == 2)
						auto lixo_ret = checkAllClearHole();

					finish_pang_battle(1);

				}else {

					for (auto& el : m_players) {

						INIT_PLAYER_INFO("finish_game", "tentou finalizar o versus", el);

						if (pgi->flag == PlayerGameInfo::eFLAG_GAME::PLAYING) {

							requestSaveRecordCourse(*el, 0 , (m_ri.qntd_hole == 18 && m_course->findHoleSeq(pgi->hole) == 18) ? 1 : 0);

							savePangBattleDados(*el);

							requestSaveInfo(*el, 0);

							if (pgi->data.exp > 0) {

								el->addExp(pgi->data.exp, false );

								if (el->m_pi.ei.cad_info != nullptr)
									el->addCaddieExp(pgi->data.exp);

								if (el->m_pi.ei.mascot_info != nullptr)
									el->addMascotExp(pgi->data.exp);
							}

							sendUpdateInfoAndMapStatistics(*el, 0);

							requestSendTreasureHunterItem(*el);

							if (el->m_pi.ei.mascot_info != nullptr) {
								packet_func::pacote06B(p, el, &el->m_pi, 8);

								packet_func::session_send(p, el, 1);
							}

							pgi->sys_achieve.finish_and_update(*el);

							p.init_plain((unsigned short)0x244);

							p.addUint32(0);

							packet_func::session_send(p, &_session, 1);

							p.init_plain((unsigned short)0x24F);

							p.addUint32(0);

							packet_func::session_send(p, &_session, 1);

							p.init_plain((unsigned short)0xC8);

							p.addUint64(el->m_pi.ui.pang);

							p.addUint64(0ull);

							packet_func::session_send(p, el, 1);

							setGameFlag(pgi, PlayerGameInfo::eFLAG_GAME::FINISH);
						}
					}

					m_game_init_state = 2;

					return true;
				}
			}
		}
	}

	return m_players.size() == 0;
}
