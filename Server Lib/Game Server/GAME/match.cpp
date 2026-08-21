
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "match.hpp"
#include "../../Projeto IOCP/UTIL/exception.h"
#include "../../Projeto IOCP/TYPE/stda_error.h"
#include "../../Projeto IOCP/UTIL/message_pool.h"

#include "treasure_hunter_system.hpp"

#include "../PACKET/packet_func_sv.h"

#include "../UTIL/map.hpp"

#include "../Game Server/game_server.h"
#include "../UTIL/club3d.hpp"

#if defined(__linux__)
#include <numbers>
#include <cmath>
#endif

#define CHECK_SESSION(method) if (!_session.getState()) \
									throw exception("[Match::" + std::string((method)) +"][Error] player nao esta connectado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MATCH, 1, 0)); \

#define CHECK_SESSION_BEGIN(method) if (!_session.getState()) \
										throw exception("[Match::request" + std::string((method)) +"][Error] player nao esta connectado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MATCH, 1, 0)); \

#define REQUEST_BEGIN(method) CHECK_SESSION_BEGIN(std::string("request") + (method)) \
							  if (_packet == nullptr) \
									throw exception("[Match::request" + std::string((method)) +"][Error] _packet is nullptr", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MATCH, 6, 0)); \

#define INIT_PLAYER_INFO(_method, _msg, __session) auto pgi = getPlayerInfo((__session)); \
	if (pgi == nullptr) \
		throw exception("[Match::" + std::string((_method)) + "][Error] player[UID=" + std::to_string((__session)->m_pi.uid) + "] " + std::string((_msg)) + ", mas o game nao tem o info dele guardado. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MATCH, 1, 4)); \

#define INIT_TEAM_INFO(_method, __session) Team *team = nullptr; \
{ \
	auto __it = std::find_if(m_teans.begin(), m_teans.end(), [&](auto& _el) { \
		return _el.findPlayerByUID((__session)->m_pi.uid) != nullptr; \
	}); \
	if (__it != m_teans.end()) \
		team = &(*__it); \
	else \
		throw exception("[Match::" + std::string((_method)) + "][Error] player[UID=" + std::to_string((__session)->m_pi.uid) + "] tentou encontrar o team(time) dele, mas no game nao tem o team(time) dele. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MATCH, 100, 0)); \
} \

using namespace stdA;

Match::Match(std::vector<player*>& _players, RoomInfoEx & _ri, RateValue _rv, unsigned char _channel_rookie, std::vector< Team >& _teans)
	: VersusBase(_players, _ri, _rv, _channel_rookie), m_team_win(0u), m_match_state(false), m_teans(_teans), m_team_turn(nullptr), m_thi_blue{0} {

	if (!sTreasureHunterSystem::getInstance().isLoad())
		sTreasureHunterSystem::getInstance().load();

	auto course = sTreasureHunterSystem::getInstance().findCourse((m_ri.course & 0x7F));

	if (course == nullptr)
		_smp::message_pool::getInstance().push(new message("[Match::Match][Error] tentou pegar o course do Treasure Hunter System, mas o course[COURSE="
				+ std::to_string((unsigned short)(m_ri.course & 0x7F)) + "] nao existe no sistema", CL_FILE_LOG_AND_CONSOLE));
	else

		sTreasureHunterSystem::getInstance().updateCoursePoint(*course, -1);

	initAllPlayerInfo();

	init_team_player_position();

	for (auto& el : m_players) {

		INIT_PLAYER_INFO("Match", "tentou inicializar o counter item do Match", el);

		initAchievement(*el);

		pgi->sys_achieve.incrementCounter(0x6C40001Eu );
	}

	m_match_state = init_game();
}

Match::~Match() {

	stopTime();

	for (auto& el : m_players)
		finish_game(*el);

	clear_teans();

	m_team_win = 0u;

	deleteAllPlayer();
}

bool Match::deletePlayer(player* _session, int _option) {

	if (_session == nullptr)
		throw exception("[Match::deletePlayer][Error] tentou deletar um player, mas o seu endereco eh nullptr.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MATCH, 50, 0));

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

				INIT_TEAM_INFO("deletePlayer", _session);

				packet p;

				if (m_player_turn == pgi)
					stopTime();

				auto sessions = getSessions(*it);

				requestFinishItemUsedGame(*(*it));

				requestSaveInfo(*(*it), (_option == 0x800) ? 5  : 1);

				setGameFlag(pgi, PlayerGameInfo::eFLAG_GAME::QUIT);

				team->setQuit(1u);

				p.init_plain((unsigned short)0x61);

				p.addUint32((*it)->m_oid);

				packet_func::vector_send(p, sessions, 1);

				p.init_plain((unsigned short)0x40);

				p.addUint8(2);

				p.addString((*it)->m_pi.nickname);

				p.addUint16(0);

				packet_func::vector_send(p, sessions, 1);

				sendUpdateInfoAndMapStatistics(*_session, -1);

				ret = checkNextStepGame(*_session);

				if (!ret && m_players.size() > 0)
					ret = 1;

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
			_smp::message_pool::getInstance().push(new message("[Match::deletePlayer][WARNING] player ja foi excluido do game.", CL_FILE_LOG_AND_CONSOLE));

#if defined(_WIN32)
		LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
		pthread_mutex_unlock(&m_cs);
#endif

		m_state_vs.unlock();

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Match::deletePlayer][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

#if defined(_WIN32)
		LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
		pthread_mutex_unlock(&m_cs);
#endif

		m_state_vs.unlock();
	}

	return ret;
}

void Match::deleteAllPlayer() {

	while (!m_players.empty())
		deletePlayer(*m_players.begin(), 0);
}

void Match::requestInitHole(player& _session, packet *_packet) {
	REQUEST_BEGIN("InitHole");

	packet p;

#if defined(__linux__)
#pragma pack(1)
#endif

	struct stInitHole {
		void clear() { memset(this, 0, sizeof(stInitHole)); };
		unsigned char numero;
		uint32_t option;
		uint32_t ulUnknown;
		unsigned char par;
		stXZLocation tee;
		stXZLocation pin;
	};

#if defined(__linux__)
#pragma pack()
#endif

	try {

		stInitHole ctx_hole{ 0 };

		_packet->readBuffer(&ctx_hole, sizeof(ctx_hole));

		auto hole = m_course->findHole(ctx_hole.numero);

		hole->init(ctx_hole.tee, ctx_hole.pin);

		INIT_PLAYER_INFO("requestInitHole", "tentou inicializar o hole[NUMERO = " + std::to_string(hole->getNumero()) + "] no jogo", &_session);

		INIT_TEAM_INFO("requestInitHole", &_session);

		pgi->location.x = ctx_hole.tee.x;
		pgi->location.z = ctx_hole.tee.z;

		team->setLocation(pgi->location);

		pgi->hole = ctx_hole.numero;

		if (!pgi->init_first_hole)
			pgi->init_first_hole = 1u;

		team->setHole(ctx_hole.numero);

		team->setDegree((m_ri.modo == Hole::M_REPEAT) ? hole->getWind().degree.getDegree() : hole->getWind().degree.getShuffleDegree());

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[Match::requestInitHole][Log] player[UID=" + std::to_string(pgi->uid) + "] Vento[Graus=" + std::to_string(pgi->degree) + "]", CL_FILE_LOG_AND_CONSOLE));
#endif

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Match::requestInitHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

	}
}

void Match::requestMoveBall(player& _session, packet* _packet) {
	REQUEST_BEGIN("MoveBall");

	packet p;

	try {

		float x = _packet->readFloat();
		float y = _packet->readFloat();
		float z = _packet->readFloat();

		INIT_PLAYER_INFO("requestMoveBall", "tentou recolocar a bola no jogo", &_session);

		INIT_TEAM_INFO("requestMoveBall", &_session);

		pgi->location.x = x;
		pgi->location.y = y;
		pgi->location.z = z;

		team->setLocation(pgi->location);

		team->decrementPlayerStartHole();

		stopTime();

		p.init_plain((unsigned short)0x60);

		p.addFloat(pgi->location.x);
		p.addFloat(pgi->location.y);
		p.addFloat(pgi->location.z);

		packet_func::game_broadcast(*this, p, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Match::requestMoveBall][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void Match::changeHole() {

	updateTreasureHunterPoint();

	if (m_player_turn == nullptr)
		throw exception("[Match::changeHole][Error] PlayerGameInfo *m_player_turn is invalid(nullptr). Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MATCH, 100, 0));

	auto hole_seq = m_course->findHoleSeq(m_player_turn->hole);

	int state = 0, hole_diff = m_ri.qntd_hole - hole_seq;

	if ((int)(m_teans[0].getPoint() - m_teans[1].getPoint()) > hole_diff)
		state = 1;
	else if ((int)(m_teans[1].getPoint() - m_teans[0].getPoint()) > hole_diff)
		state = 1;

	if (state || m_players.size() <= 0 || checkEndGame(**m_players.begin())) {

		packet p((unsigned short)0x199);

		packet_func::game_broadcast(*this, p, 1);

		if (!sMap::getInstance().isLoad())
			sMap::getInstance().load();

		auto map = sMap::getInstance().getMap(m_ri.course & 0x7F);

#ifdef _DEBUG
		auto clear_bonus = 0u;
#endif

		if (map == nullptr)
			_smp::message_pool::getInstance().push(new message("[Match::changeHole][Error][WARNING] tentou pegar o Map dados estaticos do course[COURSE="
					+ std::to_string((unsigned short)(m_ri.course & 0x7F)) + "], mas nao conseguiu encontra na classe do Server.", CL_FILE_LOG_AND_CONSOLE));
		else {
			for (auto& team : m_teans) {
#ifdef _DEBUG
				team.incrementBonusPang((clear_bonus = sMap::getInstance().calculeClearMatch(*map, (uint32_t)hole_seq)));

				_smp::message_pool::getInstance().push(new message("[Match::changeHole][Log] player_turn[UID=" + std::to_string(m_player_turn->uid) + "] do Team[ID="
						+ std::to_string(team.getId()) + "] fez o ultimo* hole do Match e ganhou " + std::to_string(clear_bonus) + " Clear Bonus", CL_FILE_LOG_AND_CONSOLE));
#else
				team.incrementBonusPang(sMap::getInstance().calculeClearMatch(*map, m_ri.qntd_hole));
#endif
			}
		}

		finish_match(0);

	}else if (m_players.size() > 0)

		updateFinishHole();
}

void Match::finishHole() {

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

void Match::finish_match(int _option) {

	if (m_players.size() > 0 && m_game_init_state == 1) {

		requestUpdateTeamPang();

		for (auto& el : m_players) {

			INIT_PLAYER_INFO("finish_match", "tentou finalizar o Match", el);

			pgi->sys_achieve.incrementCounter(0x6C400004u );

			requestCalculePang(*el);

			updatePlayerAssist(*el);

			sendFinishMessage(*el);
		}

		finish();
	}
}

void Match::requestTeamFinishHole(player& _session, packet *_packet) {
	REQUEST_BEGIN("TeamFinishHole");

	try {

		auto state_finish = _packet->readUint16();

		INIT_TEAM_INFO("requestTeamFinishHole", &_session);

		team->setStateFinish(state_finish);

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[Match::requestTeamFinishHole][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] do Team[ID=" + std::to_string(team->getId()) + ", STATE=" + std::to_string(team->getStateFinish()) + "] terminou o hole.", CL_FILE_LOG_AND_CONSOLE));
#endif

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Match::requestTeamFinishHole][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

stGameShotValue Match::getGameShotValueToSmartCalculator(player& _session, unsigned char _club_index, unsigned char _power_shot_index) {
	CHECK_SESSION("getGameShotValueToSmartCalculator");

	stGameShotValue gsv{ 0u };

	try {

		INIT_PLAYER_INFO("getGameShotValueToSmartCalculator", "tentou executar Smart Calculator Command", &_session);

		INIT_TEAM_INFO("getGameShotValueToSmartCalculator", &_session);

		auto hole = m_course->findHole(team->getHole());

		if (hole == nullptr)
			throw exception("[Match::getGameShotValueToSmartCalculator][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou executar Smart Calculator command na sala[NUMERO=" + std::to_string(m_ri.numero)
					+ "], mas nao encontrou o Hole[NUMERO=" + std::to_string((short)team->getHole())
					+ "] no Course.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MATCH, 10000, 0));

		auto wind_flag = initCardWindPlayer(pgi, hole->getWind().wind);

		auto wind = hole->getWind().wind + 1 + wind_flag;
		auto distance = hole->getPinLocation().diffXZ(const_cast<Location&>(team->getLocation())) * 0.3125f;
		auto ground = 1u;
		auto power_range = 230.f;
		auto slope_break = 1.f;
		auto power = _session.m_pi.getSlotPower();
		auto angTo_rad = -std::atan2(hole->getPinLocation().x - team->getLocation().x, hole->getPinLocation().z - team->getLocation().z);
		auto angTo = angTo_rad * 180 /
#if defined(_WIN32)
			std::_Pi;
#elif defined(__linux__)
			std::numbers::pi;
#endif

		auto angEarcuff = pgi->earcuff_wind_angle_shot * 180 /
#if defined(_WIN32)
			std::_Pi;
#elif defined(__linux__)
			std::numbers::pi;
#endif

		auto ang = 0.l;

		if (pgi->effect_flag_shot.stFlag.EARCUFF_DIRECTION_WIND) {

			long double rad_earcuff_hole = pgi->earcuff_wind_angle_shot + -angTo_rad;

			if (rad_earcuff_hole < 0.f)
				rad_earcuff_hole = (2 *
#if defined(_WIN32)
					std::_Pi
#elif defined(__linux__)
					std::numbers::pi
#endif
				) + rad_earcuff_hole;

			ang = rad_earcuff_hole * 180 /
#if defined(_WIN32)
				std::_Pi;
#elif defined(__linux__)
				std::numbers::pi;
#endif

		}else
			ang = fmodl((team->getDegree() / 255.f) * 360.f + -angTo, 360.f);

		bool pwr_by_condition_actived = pgi->effect_flag_shot.stFlag.SWITCH_TWO_EFFECT;

		if (pwr_by_condition_actived && _session.m_pi.ei.char_info != nullptr
			&& _session.m_pi.ei.char_info->isAuxPartEquiped(0x70210001u) && pgi->item_active_used_shot != 0u
			&& pgi->item_active_used_shot == POWER_MILK_TYPEID)
			pwr_by_condition_actived = false;

		auto power_extra = _session.m_pi.getExtraPower(pwr_by_condition_actived);

		if (pgi->effect_flag_shot.stFlag.DECREASE_1M_OF_WIND && wind > 1)
			wind--;

		if (pgi->effect_flag_shot.stFlag.WIND_1M_RANDOM)
			wind = 1;

		if (pgi->effect_flag_shot.stFlag.SAFETY_CLIENT_RANDOM || pgi->effect_flag_shot.stFlag.SAFETY_RANDOM) {

			ground = 100;
			slope_break = 0.f;
		}

		if (pgi->effect_flag_shot.stFlag.GROUND_100_PERCENT_RONDOM)
			ground = 100;

		if (pgi->item_active_used_shot != 0u) {

			if (isSilentWindItem(pgi->item_active_used_shot))
				wind = 1;

			if (isSafetyItem(pgi->item_active_used_shot)) {

				ground = 100;
				slope_break = 0.f;
			}
		}

#ifdef _DEBUG

		_smp::message_pool::getInstance().push(new message("[Match::getGameShotValueToSmartCalculator][Log] Wind=" + std::to_string(wind)
				+ ", Distance=" + std::to_string(distance) + ", Power=" + std::to_string(power) + ", Power_Extra="
				+ std::to_string(power_extra.getTotal(0)) + ", ANGLE[ANG_TO_RAD=" + std::to_string(angTo_rad)
				+ ", ANG_TO=" + std::to_string(angTo) + ", ANG=" + std::to_string(ang) + ", DEGREE=" + std::to_string((pgi->degree / 255.f) * 360.f)
				+ ", ANG_EARCUFF=" + (pgi->effect_flag_shot.stFlag.EARCUFF_DIRECTION_WIND ? std::to_string(angEarcuff) : "NONE") + "]", CL_FILE_LOG_AND_CONSOLE));
#endif

		if (_club_index < sAllClubInfo3D::getInstance().m_clubs.size()) {

			Club3D club(sAllClubInfo3D::getInstance().m_clubs[_club_index], calculeTypeDistance((float)distance));

			power_range = (float)club.getRange(power_extra, (float)power, ePOWER_SHOT_FACTORY(_power_shot_index));
		}

		gsv.gm = (_session.m_pi.m_cap.stBit.gm_normal || _session.m_pi.m_cap.stBit.game_master) ? true : false;

		gsv.safety = (slope_break == 0.f) ? true : false;
		gsv.ground = (ground == 100u) ? true : false;

		gsv.power_slot = (unsigned char)power;

		gsv.auxpart_pwr = (char)power_extra.getPowerDrive().m_auxpart;
		gsv.mascot_pwr = (char)power_extra.getPowerDrive().m_mascot;
		gsv.card_pwr = (char)power_extra.getPowerDrive().m_card;
		gsv.ps_card_pwr = (char)power_extra.getPowerShot().m_card;

		gsv.distance = (float)distance;
		gsv.wind = (float)wind;
		gsv.degree = (float)ang;

		gsv.mira_rad = angTo_rad;
		gsv.power_range = power_range;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Match::getGameShotValueToSmartCalculator][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	return gsv;
}

void Match::startTime(void* _quem) {

	try {

		INIT_PLAYER_INFO("startTime", "tentou comecar o tempo do player turno no jogo", (player*)_quem);

		INIT_TEAM_INFO("startTime", (player*)_quem);

		pgi->data.tacada_num++;

		team->incrementTacadaNum();

		team->incrementPlayerStartHole();

		if (m_timer != nullptr)
			stopTime();

		job j(VersusBase::end_time, this, _quem);

			m_timer = sgs::gs::getInstance().makeTime(m_ri.time_vs , j);

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[Match::startTime][Log] Criou o Timer[Tempo=" + std::to_string((m_ri.time_30s > 0) ? m_ri.time_30s / 60000 : 0) + "min, STATE=" + std::to_string(m_timer->getState()) + "]", CL_FILE_LOG_AND_CONSOLE));
#endif
	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Match::startTime][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void Match::timeIsOver(void* _quem) {

	VersusBase::timeIsOver(_quem);

	if (_quem != nullptr) {

		player* p = reinterpret_cast< player* >(_quem);

		INIT_PLAYER_INFO("timeIsOver", "tentou acabar o tempo do turno no jogo", p);

		INIT_TEAM_INFO("timeIsOver", p);

		team->setTimeout(1u);

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
		_smp::message_pool::getInstance().push(new message("[Match::timeIsOver][WARNING] time is over executed without _quem, _quem is invalid(nullptr). Bug" , CL_FILE_LOG_AND_CONSOLE));
}

bool Match::init_game() {

	auto lixo = VersusBase::init_game();

	if (m_players.size() > 0) {

		initGameTime();

		m_game_init_state = 1;

		m_match_state = true;
	}

	return true;
}

void Match::requestTranslateSyncShotData(player& _session, ShotSyncData& _ssd) {
	CHECK_SESSION_BEGIN("requestTransateSyncShotData");

	try {

		auto s = findSessionByOID(_ssd.oid);

		if (s == nullptr)
			throw exception("[Match::requestTranslateSyncShotData][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou sincronizar tacada do player[OID="
					+ std::to_string(_ssd.oid) + "], mas o player nao existe nessa jogo. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::VERSUS_BASE, 200, 0));

		if (_session.m_pi.uid == s->m_pi.uid) {

			INIT_PLAYER_INFO("requestTranslateSyncShotData", "tentou sincronizar a tacada no jogo", &_session);

			INIT_TEAM_INFO("requestTranslateSyncShotData", &_session);

			pgi->shot_sync = _ssd;

			auto last_location = team->getLocation();

			pgi->location.x = _ssd.location.x;
			pgi->location.z = _ssd.location.z;

			team->setLocation((const Location&)_ssd.location);

			pgi->data.pang = _ssd.pang;
			pgi->data.bonus_pang = _ssd.bonus_pang;

			team->setPang(_ssd.pang);
			team->setBonusPang(_ssd.bonus_pang);

			if (_ssd.state == ShotSyncData::OUT_OF_BOUNDS || _ssd.state == ShotSyncData::UNPLAYABLE_AREA) {
				pgi->data.tacada_num++;

				team->incrementTacadaNum();
			}

			auto hole = m_course->findHole(pgi->hole);

			if (hole == nullptr)
				throw exception("[Match::requestTranslateSyncShotData][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou sincronizar tacada no hole[NUMERO="
						+ std::to_string((unsigned short)pgi->hole) + "], mas o numero do hole is invalid. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::VERSUS_BASE, 12, 0));

			team->setAcertoHole(_ssd.state_shot.display.stDisplay.acerto_hole);

			if (!_ssd.state_shot.display.stDisplay.acerto_hole && hole->getPar().total_shot <= (pgi->data.tacada_num + 1)) {

				if (pgi->data.tacada_num < hole->getPar().total_shot) {
					pgi->data.tacada_num++;

					team->incrementTacadaNum();
				}

				pgi->data.giveup = 1;

				team->setGiveUp(1u);

				pgi->data.bad_condute++;

				team->incrementBadCondute();
			}

			update_sync_shot_achievement(_session, last_location);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Match::requestTranslateSyncShotData][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void Match::requestTranslateFinishHoleData(player& _session, UserInfoEx& _ui) {
	CHECK_SESSION_BEGIN("requestTranslateFinishHole");

	try {

		INIT_PLAYER_INFO("requestTranslateFinishHoleData", "tentou finalizar hole dados no jogo", &_session);

		pgi->ui = _ui;

		if (!pgi->shot_sync.state_shot.display.stDisplay.acerto_hole
			&& !m_teans[0].getAcertoHole() && !m_teans[1].getAcertoHole()) {

			INIT_TEAM_INFO("requestTranslateFinishHoleData", &_session);

			auto hole = m_course->findHole(pgi->hole);

			if (hole == nullptr)
				throw exception("[Match::requestFinishHoleData][Error] player[UID=" + std::to_string(pgi->uid) + "] tentou finalizar os dados do hole no jogo, mas o hole[NUMERO="
						+ std::to_string(pgi->hole) + "] nao existe no course. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::VERSUS, 400, 0));

			if (pgi->data.tacada_num < hole->getPar().total_shot) {
				pgi->data.tacada_num++;

				team->setTacadaNum(pgi->data.tacada_num);
			}

			if (!pgi->data.giveup) {
				pgi->data.giveup = 1;

				team->setGiveUp(1);
			}

		}

		pgi->progress.best_chipin = _ui.best_chip_in;
		pgi->progress.best_long_puttin = _ui.best_long_putt;
		pgi->progress.best_drive = _ui.best_drive;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Match::requestTranslateFinishHoleData][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

bool Match::checkEndGame(player& _session) {

	INIT_PLAYER_INFO("checkEndGame", "tentou verificar se eh o final do jogo", &_session);

	return (m_course->findHoleSeq(pgi->hole) == m_ri.qntd_hole || ((m_players.size() % 2) == 1 ));
}

bool Match::checkAllClearHole() {

	uint32_t count = 0u;
	bool ret = false;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs_sync_shot);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs_sync_shot);
#endif

	std::for_each(m_teans.begin(), m_teans.end(), [&](auto& _el) {

		if (_el.getAcertoHole() || _el.getGiveUp() || _el.isQuit())
			count++;
	});

	ret = (count == m_teans.size());

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs_sync_shot);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs_sync_shot);
#endif

	return ret;
}

bool Match::checkAllClearHoleAndClear() {

	uint32_t count = 0u;
	bool ret = false;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs_sync_shot);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs_sync_shot);
#endif

	std::for_each(m_teans.begin(), m_teans.end(), [&](auto& _el) {

		if (_el.getAcertoHole() || _el.getGiveUp())
			count++;
	});

	ret = (count == m_teans.size());

	if (ret)
		clear_all_clear_hole();

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs_sync_shot);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs_sync_shot);
#endif

	return ret;
}

void Match::clearAllClearHole() {

#if defined(_WIN32)
	EnterCriticalSection(&m_cs_sync_shot);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs_sync_shot);
#endif

	clear_all_clear_hole();

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs_sync_shot);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs_sync_shot);
#endif
}

void Match::clear_all_clear_hole() {

	std::for_each(m_teans.begin(), m_teans.end(), [&](auto& _el) {

		_el.setAcertoHole(0u);
		_el.setGiveUp(0u);

	});
}

void Match::clear_teans() {

	if (!m_teans.empty()) {
		m_teans.clear();
		m_teans.shrink_to_fit();
	}
}

void Match::updateTreasureHunterPoint() {

	if (!sTreasureHunterSystem::getInstance().isLoad())
		sTreasureHunterSystem::getInstance().load();

	if (m_teans[0].getAcertoHole()) {

		auto hole = m_course->findHole(m_teans[0].getHole());

		if (hole == nullptr)
			throw exception("[VersusBase::updateTreasureHunterPoint][Error] tentou atualizar os pontos do Treasure Hunter no hole[NUMERO="
					+ std::to_string((unsigned short)m_teans[0].getHole()) + "], mas o hole nao existe. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::TOURNEY_BASE, 30, 0));

		m_thi.treasure_point += (sTreasureHunterSystem::getInstance().calcPointNormal(m_teans[0].getTacadaNum(), hole->getPar().par) + m_thi.getPoint(m_teans[0].getTacadaNum(), hole->getPar().par)) * 2;
	}

	if (m_teans[1].getAcertoHole()) {

		auto hole = m_course->findHole(m_teans[1].getHole());

		if (hole == nullptr)
			throw exception("[VersusBase::updateTreasureHunterPoint][Error] tentou atualizar os pontos do Treasure Hunter no hole[NUMERO="
					+ std::to_string((unsigned short)m_teans[1].getHole()) + "], mas o hole nao existe. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::TOURNEY_BASE, 30, 0));

		m_thi_blue.treasure_point += (sTreasureHunterSystem::getInstance().calcPointNormal(m_teans[1].getTacadaNum(), hole->getPar().par) + m_thi_blue.getPoint(m_teans[1].getTacadaNum(), hole->getPar().par)) * 2;
	}

	packet p((unsigned short)0x132);

	p.addUint32(m_thi.treasure_point);

	packet_func::game_broadcast(*this, p, 1);

	p.init_plain((unsigned short)0x132);

	p.addUint32(m_thi.treasure_point);

	p.addUint32(m_thi_blue.treasure_point);

	packet_func::game_broadcast(*this, p, 1);
}

bool Match::checkNextStepGame(player& _session) {

	auto ret = false;

	try {

		INIT_PLAYER_INFO("checkNextStepGame", "tentou verificar o proximo passo do jogo", &_session);

		if (m_players.size() > 0) {

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
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Match::checkNextStepGame][ErroSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	return ret;
}

void Match::requestSaveInfo(player& _session, int _option) {
	CHECK_SESSION_BEGIN("SaveInfo");

	try {

		INIT_PLAYER_INFO("requestSaveInfo", "tentou salvar o info do player no Match", &_session);

		INIT_TEAM_INFO("requestSaveInfo", &_session);

		if (_option == 0) {
			pgi->ui.team_game = 1l;
			pgi->ui.team_win = (m_team_win == team->getId()) ? 1l : 0l;
		}else {
			pgi->ui.team_game = 0l;
			pgi->ui.team_win = 0l;
		}

		auto hole_seq = m_course->findHoleSeq(pgi->hole);

		if (hole_seq == (unsigned short)~0)
			_smp::message_pool::getInstance().push(new message("[Match::requestSaveInfo][Error] player[UID=" + std::to_string(_session.m_pi.uid) +"] tentou pegar sequencia do hole[NUMERO="
					+ std::to_string(pgi->hole) + "] no course, mas nao encontrou o hole no course. Bug", CL_FILE_LOG_AND_CONSOLE));
		else
			pgi->ui.team_hole = hole_seq;

		Game::requestSaveInfo(_session, _option);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Match::requestSaveInfo][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void Match::requestFinishExpGame() {

	if (m_players.size() > 0) {

		player *_session = nullptr;
		float stars = m_course->getStar(), temp = 0.f;
		int32_t exp = 0, hole_seq = 0;

		for (auto& team : m_teans) {

			temp = 1.f;

			if (m_team_win != 2 && team.getId() != m_team_win)
				temp -= 0.4f;

			for (auto& el : team.getPlayers()) {

				INIT_PLAYER_INFO("requestFinishExpGame", "tentou finalizar experiencia do jogo", el);

				hole_seq = (int)m_course->findHoleSeq(pgi->hole);

				if (hole_seq == 1 && !team.getAcertoHole())
					hole_seq = 0;

				if ((_session = findSessionByUID(pgi->uid)) != nullptr) {

					exp = (int)(1 * m_players.size() * (hole_seq > 0 ? hole_seq : 0) * stars);
					exp = (int)(exp * TRANSF_SERVER_RATE_VALUE(pgi->used_item.rate.exp) * TRANSF_SERVER_RATE_VALUE(m_rv.exp));
					exp = (int)((float)exp * temp);

					if (pgi->level < 70 )
						pgi->data.exp = exp;
				}

				_smp::message_pool::getInstance().push(new message("[Match::requestFinishExpGame][Log] player[UID=" + std::to_string(pgi->uid) + "] ganhou " + std::to_string(pgi->data.exp) + " de experience.", CL_FILE_LOG_AND_CONSOLE));

			}
		}
	}
}

void Match::requestCalculeTeamWin() {

	m_team_win = 0;

	if (m_teans[0].getPoint() == m_teans[1].getPoint()) {

		if (m_teans[0].getPang() == m_teans[1].getPang())
			m_team_win = 2;
		else if (m_teans[1].getPang() > m_teans[0].getPang())
			m_team_win = 1;

	}else if (m_teans[1].getPoint() > m_teans[0].getPoint())
		m_team_win = 1;
}

void Match::requestUpdateTeamPang() {

	for (auto& team : m_teans) {

		for (auto& el : team.getPlayers()) {

			INIT_PLAYER_INFO("requestUpdateTeamPang", "tentou atualizar pangs do player[UID=" + std::to_string(el->m_pi.uid) + "] no Match", el);

			pgi->data.pang = team.getPang();
			pgi->data.bonus_pang = team.getBonusPang();
		}
	}
}

void Match::finish() {

	m_match_state = false;

	m_game_init_state = 2;

	requestCalculeRankPlace();

	requestCalculeTeamWin();

	requestFinishExpGame();

	requestDrawTreasureHunterItem();

	for (auto& el : m_players) {

		INIT_PLAYER_INFO("finish", "tentou finalizar os dados do jogador no jogo", el);

		if (pgi->flag != PlayerGameInfo::eFLAG_GAME::QUIT)
			requestFinishData(*el);
	}
}

void Match::requestFinishTeamHole() {

	finishHole();

	if (m_player_turn == nullptr)
		throw exception("[Match::requestFinishTeamHole][Error] m_player_turn is invalid(nullptr). Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MATCH, 100, 0));

	auto hole = m_course->findHole(m_player_turn->hole);

	if (hole == nullptr)
		throw exception("[Match::requestFinishTeamHole][Error] player[UID=" + std::to_string(m_player_turn->uid) + "] tentou finalizar hole[NUMERO="
				+ std::to_string((unsigned short)m_player_turn->hole) + "] no jogo, mas o numero do hole is invalid. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MATCH, 20, 0));

	m_teans[0].setLastWin(0u);
	m_teans[1].setLastWin(0u);

	if (m_teans[0].getAcertoHole() && m_teans[0].getStateFinish() > 0 && m_teans[1].getStateFinish() == 0
			&& m_teans[0].getTacadaNum() < (m_teans[1].getTacadaNum() + 1)) {
		m_teans[0].setLastWin(1u);
		m_teans[0].incrementPoint();
	}else if (m_teans[1].getAcertoHole() && m_teans[1].getStateFinish() > 0 && m_teans[0].getStateFinish() == 0
			&& m_teans[1].getTacadaNum() < (m_teans[0].getTacadaNum() + 1)) {
		m_teans[1].setLastWin(1u);
		m_teans[1].incrementPoint();
	}

	m_teans[0].setStateFinish(0u);
	m_teans[1].setStateFinish(0u);

	for (auto& el : m_teans) {

		el.setStateFinish(0u);

		el.incrementTotalTacadaNum(el.getTacadaNum());

		el.setScore((el.getTacadaNum() - hole->getPar().par));

		el.setPlayerStartHole(0u);

	}
}

void Match::requestFinishData(player& _session) {

	requestFinishItemUsedGame(_session);

	requestSaveDrop(_session);

	rain_hole_consecutivos_count(_session);

	score_consecutivos_count(_session);

	rain_count(_session);

	sendTreasureHunterItemDrawGUI(_session);

	sendDropItem(_session);

	sendPlacar(_session);
}

void Match::sendPlacar(player& _session) {

	packet p((unsigned short)0x91);

	p.addUint8((unsigned char)m_players.size());

	for (auto& el : m_players) {

		INIT_PLAYER_INFO("sendPlacar", "tentou enviar o placar do jogo", el);

		INIT_TEAM_INFO("sendPlacar", el);

		p.addUint32(el->m_oid);
		p.addUint8((unsigned char)getRankPlace(*el));
		p.addInt8(0x7F );
		p.addInt8((unsigned char)pgi->data.total_tacada_num);

		p.addUint16((unsigned short)pgi->data.exp);
		p.addUint64(team->getPang() );
		p.addUint64(team->getBonusPang() );

		p.addUint64(0ull);
	}

	p.addUint8((const unsigned char)m_teans[0].getPoint());
	p.addUint8((const unsigned char)m_teans[1].getPoint());
	p.addUint8(m_team_win);

	packet_func::session_send(p, &_session, 1);
}

void Match::sendFinishMessage(player& _session) {

	INIT_PLAYER_INFO("sendFinishMessage", "tentou enviar message no chat que o player terminou o jogo", &_session);

	INIT_TEAM_INFO("sendFinishMessage", &_session);

	packet p((unsigned short)0x40);

	p.addUint8(16);

	p.addString(_session.m_pi.nickname);
	p.addUint16(0);

	p.addInt32(team->getPoint());
	p.addUint64(team->getPang());
	p.addUint8(pgi->assist_flag);

	packet_func::game_broadcast(*this, p, 1);
}

void Match::sendReplyFinishLoadHole() {

	try {

		PlayerGameInfo *pgi = requestCalculePlayerTurn();

		auto hole = m_course->findHole(pgi->hole);

		if (hole == nullptr)
			throw exception("[Match::requestFinishLoadHole][Error] player[UID=" + std::to_string(pgi->uid) + "] tentou finalizar carregamento do hole[NUMERO="
					+ std::to_string(pgi->hole) + "], mas nao conseguiu encontrar o hole no course. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::VERSUS_BASE, 201, 0));

		INIT_TEAM_INFO("requestFinishLoadHole", findSessionByPlayerGameInfo(pgi));

		packet p((unsigned short)0x9E);

		p.addUint16(hole->getWeather());
		p.addUint8(0);

		packet_func::game_broadcast(*this, p, 1);

		auto wind_flag = initCardWindPlayer(m_player_turn, hole->getWind().wind);

		p.init_plain((unsigned short)0x5B);

		p.addUint8(hole->getWind().wind + wind_flag);
		p.addUint8((wind_flag < 0) ? 1 : 0);
		p.addUint16(team->getDegree() );
		p.addUint8(1 );

		packet_func::game_broadcast(*this, p, 1);

		p.init_plain((unsigned short)0x53);

		if (m_player_turn == nullptr) {
			_smp::message_pool::getInstance().push(new message("[Match::requestFinishLoadHole][Error] player_turn is invalid(nullptr)", CL_FILE_LOG_AND_CONSOLE));

			p.addUint32(0);
		}else
			p.addUint32(m_player_turn->oid);

		packet_func::game_broadcast(*this, p, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Match::sendReplyFinishLoadHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void Match::sendReplyFinishCharIntro() {

	packet p;

	try {

		for (auto& el : m_teans) {

			el.setTacadaNum(0u);

			el.setGiveUp(0u);
		}

		p.init_plain((unsigned short)0x90);

		packet_func::game_broadcast(*this, p, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Match::sendReplyFinishCharIntro][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

}

int Match::checkEndShotOfHole(player& _session) {
	CHECK_SESSION_BEGIN("checkEndShotOfHole");

	try {

		INIT_PLAYER_INFO("checkEndShotOfHole", "tentou verificar a ultima tacada do hole no jogo", &_session);

		if (pgi->data.bad_condute >= 3)
			return 2;
		else
			setFinishShot(pgi);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Match::checkEndShotOfHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	return 0;
}

void Match::changeTurn() {

	try {

		if (m_player_turn == nullptr)
			throw exception("[Match::changeTurn][Error] PlayerGameInfo *m_player_turn is invalid(nullptr). Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MATCH, 100, 0));

		INIT_TEAM_INFO("changeTurn", findSessionByPlayerGameInfo(m_player_turn));

		stopTime();

		auto hole_seq = m_course->findHoleSeq(m_player_turn->hole);

		int state = 0, hole_diff = m_ri.qntd_hole - hole_seq;

		if (checkAllClearHole())
			state = 1;
		else if ((m_players.size() % 2) == 1 )
			state = 1;
		else if (m_teans[0].getAcertoHole() && !m_teans[1].getTimeout() && (m_teans[0].getTacadaNum() < (m_teans[1].getTacadaNum() + 1)))
			state = 1;
		else if (m_teans[1].getAcertoHole() && !m_teans[0].getTimeout() && (m_teans[1].getTacadaNum() < (m_teans[0].getTacadaNum() + 1)))
			state = 1;
		else if (m_teans[0].getAcertoHole() && !m_teans[1].getTimeout() && (m_teans[0].getTacadaNum() == (m_teans[1].getTacadaNum() + 1)) && (int)(m_teans[0].getPoint() - m_teans[1].getPoint()) > hole_diff)
			state = 1;
		else if (m_teans[1].getAcertoHole() && !m_teans[0].getTimeout() && (m_teans[1].getTacadaNum() == (m_teans[0].getTacadaNum() + 1)) && (int)(m_teans[1].getPoint() - m_teans[0].getPoint()) > hole_diff)
			state = 1;

		clearDataEndShot(m_player_turn);

		if (state) {

			clear_all_flag_sync();

			for (auto& el : m_teans)
				el.setTimeout(0u);

			requestFinishTeamHole();

			changeHole();

			clearAllClearHole();

		}else {

			clear_all_flag_sync();

			for (auto& el : m_teans)
				el.setTimeout(0u);

			requestCalculePlayerTurn();

			if (m_player_turn == nullptr)
				throw exception("[Match::changeTurn][Error] PlayerGameInfo *m_player_turn is invalid(nullptr). Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MATCH, 100, 1));

			auto hole = m_course->findHole(m_player_turn->hole);

			if (hole == nullptr)
				throw exception("[Match::changeTurn][Error] player[UID=" + std::to_string(m_player_turn->uid) + "] tentou encontrar o hole[NUMERO="
						+ std::to_string(m_player_turn->hole) + "] do course no jogo, mas nao foi encontrado. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MATCH, 101, 0));

			INIT_TEAM_INFO("changeTurn", findSessionByPlayerGameInfo(m_player_turn));

			auto wind_flag = initCardWindPlayer(m_player_turn, hole->getWind().wind);

			packet p((unsigned short)0x5B);

			p.addUint8(hole->getWind().wind + wind_flag);
			p.addUint8((wind_flag < 0) ? 1 : 0);
			p.addUint16(team->getDegree() );
			p.addUint8(1 );

			packet_func::game_broadcast(*this, p, 1);

			p.init_plain((unsigned short)0x63);

			if (m_player_turn == nullptr) {
				_smp::message_pool::getInstance().push(new message("[Match::changeTurn][Error] player_turn is invalid(nullptr)", CL_FILE_LOG_AND_CONSOLE));

				p.addUint32(0);
			}else
				p.addUint32(m_player_turn->oid);

			packet_func::game_broadcast(*this, p, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Match::changeTurn][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void Match::CCGChangeWind(player& _gm, unsigned char _wind, unsigned short _degree) {

	try {

		if (m_player_turn == nullptr)
			throw exception("[Match::CCGChangeWind][Error] player[UID=" + std::to_string(_gm.m_pi.uid) + "] tentou executar o comando de troca de vento no versus na sala[NUMERO="
					+ std::to_string(m_ri.numero) + "], mas o player_turn do versus eh invalido. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MATCH, 1, 0x5700100));

		auto hole = m_course->findHole(m_player_turn->hole);

		if (hole == nullptr)
			throw exception("[Match::CCGChangeWind][Error] player[UID=" + std::to_string(_gm.m_pi.uid) + "] tentou executar o comando de troca de vento no versus na sala[NUMERO="
					+ std::to_string(m_ri.numero) + "], mas o nao encontrou o hole[VALUE=" + std::to_string((short)m_player_turn->hole) + "] no course. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MATCH, 2, 0x5700100));

		auto wind = hole->getWind();

		wind.wind = _wind;

		hole->setWind(wind);

		m_team_turn->setDegree(_degree % LIMIT_DEGREE);

		_smp::message_pool::getInstance().push(new message("[Match::CCGChangeWind][Log] [GM] player[UID=" + std::to_string(_gm.m_pi.uid) + "] trocou o vento e graus da sala[NUMERO="
				+ std::to_string(m_ri.numero) + ", VENTO=" + std::to_string((unsigned short)_wind + 1) + ", GRAUS=" + std::to_string(_degree) + "]", CL_FILE_LOG_AND_CONSOLE));

		auto wind_flag = initCardWindPlayer(m_player_turn, hole->getWind().wind);

		packet p((unsigned short)0x5B);

		p.addUint8(hole->getWind().wind + wind_flag);
		p.addUint8((wind_flag < 0) ? 1 : 0);
		p.addUint16(m_team_turn->getDegree());
		p.addUint8(1);

		packet_func::game_broadcast(*this, p, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[Match::CCGChangeWind][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		throw;
	}

}

PlayerGameInfo* Match::requestCalculePlayerTurn() {

	auto team = requestCalculeTeamTurn();

	INIT_PLAYER_INFO("requestCalculePlayerTurn", "tentou calcular o player turno no Match", team->requestCalculePlayerTurn(m_course->findHoleSeq(team->getHole())));

	m_player_turn = pgi;

	return m_player_turn;
}

Team* Match::requestCalculeTeamTurn() {

	if (!m_player_info.empty()) {

		auto hole = m_course->findHole(m_player_info.begin()->second->hole);

		if (hole == nullptr) {
			_smp::message_pool::getInstance().push(new message("[Match::requestCalculeTeamTurn][Error] player[UID=" + std::to_string(m_player_info.begin()->second->uid) + "] o hole[NUMERO="
					+ std::to_string(m_player_info.begin()->second->hole) + "] nao foi encontrado no course. Bug", CL_FILE_LOG_AND_CONSOLE));

			m_player_turn = nullptr;

			return nullptr;
		}

		std::vector< TeamOrderTurnCtx > v_team_order_turn;

		for (auto& el : m_teans)
			v_team_order_turn.push_back({ &el, hole });

		if (v_team_order_turn.empty()) {
			_smp::message_pool::getInstance().push(new message("[Match::requestCalculeTeamTurn][Error] nao tem players, para calcular o turno. Bug", CL_FILE_LOG_AND_CONSOLE));

			m_player_turn = nullptr;

			return nullptr;
		}

		std::sort(v_team_order_turn.begin(), v_team_order_turn.end(), Match::sort_team_turn);

		m_team_turn = v_team_order_turn.begin()->team;
	}

	return m_team_turn;
}

void Match::init_team_player_position() {

	unsigned char red_flag = 0u, blue_flag = 0u;

	for (auto& el : m_players) {

		INIT_TEAM_INFO("init_team_player_position", el);

		if (team->getId() == 0  && !red_flag) {

			team->sort_player(el->m_pi.uid);

			red_flag = 1u;

		}else if (team->getId() == 1  && !blue_flag) {

			team->sort_player(el->m_pi.uid);

			blue_flag = 1u;
		}
	}

}

bool Match::finish_game(player& _session, int option) {

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

			if (m_course->findHoleSeq(pgi->hole) == 1 && !checkAllClearHole()) {

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

				if (m_match_state)
					finish_match(1);
				else {

					for (auto& el : m_players) {

						INIT_PLAYER_INFO("finish_game", "tentou finalizar o versus", el);

						if (pgi->flag == PlayerGameInfo::eFLAG_GAME::PLAYING) {

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

	return false;
}

bool Match::sort_team_turn(TeamOrderTurnCtx& _totc1, TeamOrderTurnCtx& _totc2) {

	auto diff = _totc1.hole->getPinLocation().diffXZ(const_cast<Location&>(_totc1.team->getLocation()));
	auto diff2 = _totc1.hole->getPinLocation().diffXZ(const_cast<Location&>(_totc2.team->getLocation()));

	if ((_totc1.team->getAcertoHole() || _totc1.team->getGiveUp()) && (_totc2.team->getAcertoHole() || _totc2.team->getGiveUp()))
		return false;

	if (!_totc1.team->getAcertoHole() && (_totc2.team->getAcertoHole() || _totc2.team->getGiveUp()))
		return true;
	else if (_totc1.team->getTacadaNum() == 0 && _totc2.team->getTacadaNum() > 0)
		return true;
	else if (diff > diff2 && !_totc1.team->getAcertoHole() && !_totc1.team->getGiveUp())
		return true;
	else if (diff == diff2 && _totc1.team->getTacadaNum() < _totc2.team->getTacadaNum() && !_totc1.team->getAcertoHole() && !_totc1.team->getGiveUp())
		return true;
	else if (diff == diff2 && _totc1.team->getTacadaNum() == _totc2.team->getTacadaNum() && _totc1.team->getLastWin() > _totc2.team->getLastWin() && !_totc1.team->getAcertoHole() && !_totc1.team->getGiveUp())
		return true;
	else if (diff == diff2 && _totc1.team->getTacadaNum() == _totc2.team->getTacadaNum() && _totc1.team->getLastWin() == _totc2.team->getLastWin()
			&& _totc1.team->getPoint() > _totc2.team->getPoint() && !_totc1.team->getAcertoHole() && !_totc1.team->getGiveUp())
		return true;
	else if (diff == diff2 && _totc1.team->getTacadaNum() == _totc2.team->getTacadaNum() && _totc1.team->getLastWin() == _totc2.team->getLastWin()
			&& _totc1.team->getPoint() == _totc2.team->getPoint() && _totc1.team->getPang() > _totc2.team->getPang() && !_totc1.team->getAcertoHole() && !_totc1.team->getGiveUp())
		return true;

	return false;
}

void Match::SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg) {

	if (_arg == nullptr) {
		_smp::message_pool::getInstance().push(new message("[Match::SQLDBResponse][WARNING] _arg is nullptr com msg_id = " + std::to_string(_msg_id), CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	if (_pangya_db.getException().getCodeError() != 0) {
		_smp::message_pool::getInstance().push(new message("[Match::SQLDBResponse][Error] " + _pangya_db.getException().getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	auto *game = reinterpret_cast<Game*>(_arg);

	switch (_msg_id) {
	case 0:
	default:
		break;
	}
}
