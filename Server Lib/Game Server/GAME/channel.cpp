
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "channel.h"
#include "../../Projeto IOCP/UTIL/exception.h"
#include "../PACKET/packet_func_sv.h"
#include "../../Projeto IOCP/UTIL/message_pool.h"
#include "../../Projeto IOCP/TYPE/stdAType.h"
#include "../../Projeto IOCP/UTIL/hex_util.h"
#include "../../Projeto IOCP/UTIL/iff.h"

#include "../GAME/item_manager.h"
#include "../GAME/mail_box_manager.hpp"

#include <algorithm>

#include "../../Projeto IOCP/PANGYA_DB/cmd_server_list.hpp"

#include "../../Projeto IOCP/PANGYA_DB/cmd_update_character_equiped.hpp"

#include "../PANGYA_DB/cmd_email_info.hpp"
#include "../PANGYA_DB/cmd_item_left_from_email.hpp"

#include "../PANGYA_DB/cmd_update_dolfini_locker_pass.hpp"
#include "../PANGYA_DB/cmd_update_dolfini_locker_mode.hpp"
#include "../PANGYA_DB/cmd_update_dolfini_locker_pang.hpp"

#include "../PANGYA_DB/cmd_add_dolfini_locker_item.hpp"
#include "../PANGYA_DB/cmd_equip_card.hpp"
#include "../PANGYA_DB/cmd_remove_equiped_card.hpp"

#include "../PANGYA_DB/cmd_delete_dolfini_locker_item.hpp"

#include "../PANGYA_DB/cmd_extend_rental.hpp"
#include "../PANGYA_DB/cmd_delete_rental.hpp"

#include "../PANGYA_DB/cmd_tuto_event_clear.hpp"
#include "../PANGYA_DB/cmd_update_tutorial.hpp"

#include "../PANGYA_DB/cmd_update_item_slot.hpp"
#include "../PANGYA_DB/cmd_update_caddie_equiped.hpp"
#include "../PANGYA_DB/cmd_update_ball_equiped.hpp"
#include "../PANGYA_DB/cmd_update_clubset_equiped.hpp"
#include "../PANGYA_DB/cmd_update_mascot_equiped.hpp"
#include "../PANGYA_DB/cmd_update_character_cutin_equiped.hpp"
#include "../PANGYA_DB/cmd_update_skin_equiped.hpp"
#include "../PANGYA_DB/cmd_update_poster_equiped.hpp"
#include "../PANGYA_DB/cmd_update_character_all_part_equiped.hpp"
#include "../PANGYA_DB/cmd_update_character_pcl.hpp"
#include "../PANGYA_DB/cmd_update_clubset_stats.hpp"
#include "../PANGYA_DB/cmd_update_character_mastey.hpp"
#include "../PANGYA_DB/cmd_update_clubset_workshop.hpp"
#include "../PANGYA_DB/cmd_update_mascot_info.hpp"
#include "../PANGYA_DB/cmd_update_legacy_tiki_shop_point.hpp"

#include "../PANGYA_DB/cmd_update_papel_shop_info.hpp"
#include "../PANGYA_DB/cmd_insert_papel_shop_rare_win_log.hpp"

#include "../PANGYA_DB/cmd_insert_box_rare_win_log.hpp"
#include "../PANGYA_DB/cmd_insert_spinning_cube_super_rare_win_broadcast.hpp"
#include "../PANGYA_DB/cmd_insert_memorial_rare_win_log.hpp"

#include "../PANGYA_DB/cmd_coupon_gacha.hpp"
#include "../PANGYA_DB/cmd_mail_box_info.hpp"
#include "../PANGYA_DB/cmd_delete_email.hpp"

#include "../PANGYA_DB/cmd_use_item_buff.hpp"
#include "../PANGYA_DB/cmd_update_item_buff.hpp"

#include "../PANGYA_DB/cmd_update_card_special_time.hpp"

#include "../PANGYA_DB/cmd_pay_caddie_holy_day.hpp"
#include "../PANGYA_DB/cmd_set_notice_caddie_holy_day.hpp"

#include "../PANGYA_DB/cmd_ticket_report_dados_info.hpp"

#include "../PANGYA_DB/cmd_attendance_reward_info.hpp"

#include "../PANGYA_DB/cmd_guild_update_activity_info.hpp"
#include "../PANGYA_DB/cmd_update_guild_update_activity.hpp"
#include "../PANGYA_DB/cmd_member_info.hpp"
#include "../PANGYA_DB/cmd_guild_info.hpp"

#include "../PANGYA_DB/cmd_grand_zodiac_pontos.hpp"

#include "card_system.hpp"
#include "comet_refill_system.hpp"
#include "papel_shop_system.hpp"
#include "box_system.hpp"
#include "memorial_system.hpp"
#include "../UTIL/sys_achievement.hpp"
#include "attendance_reward_system.hpp"
#include "premium_system.hpp"
#include "bot_gm_event.hpp"
#include "scratch_card_system.hpp"

#include "../UTIL/mgr_daily_quest.hpp"

#include "../../Projeto IOCP/DATABASE/normal_manager_db.hpp"

#include "../UTIL/lottery.hpp"
#include "../../Projeto IOCP/UTIL/random_gen.hpp"

#include "../Game Server/game_server.h"

#include "../../Projeto IOCP/UTIL/string_util.hpp"

#include "../UTIL/block_exec_one_per_time.hpp"

#define CHECK_SESSION_BEGIN(method) if (!_session.getState()) \
										throw exception("[channel::" + std::string((method)) +"][Error] player nao esta connectado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0)); \

#define REQUEST_BEGIN(method) CHECK_SESSION_BEGIN(std::string("request") + (method)) \
								if (_packet == nullptr) \
									throw exception("[channel::request" + std::string((method)) +"][Error] _packet is nullptr", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 6, 0)); \
								M_SMART_BLOCK_PACKET_ONE_TIME \

#define CHECK_SESSION_IS_AUTHORIZED(method) if (!_session.m_is_authorized) \
												throw exception("[channel::request" + std::string((method)) + "][Error] Player[UID=" + std::to_string(_session.m_pi.uid) \
														+ "] Nao esta autorizado a fazer esse request por que ele ainda nao fez o login com o Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x7000501)); \

#define BEGIN_FIND_ROOM_C(__number) \
	room *r = nullptr; \
	try { \
		r = c->m_rm.findRoom((__number)); \

#define BEGIN_FIND_ROOM(__number) \
	room *r = nullptr; \
	try { \
		r = m_rm.findRoom((__number)); \

#define END_FIND_ROOM \
		if (r != nullptr) \
			m_rm.unlockRoom(r); \
	} catch (exception& e) { \
		UNREFERENCED_PARAMETER(e); \
		if (r != nullptr) \
			m_rm.unlockRoom(r); \
		throw;  \
	} \

#define END_FIND_ROOM_C \
		if (r != nullptr) \
			c->m_rm.unlockRoom(r); \
	} catch (exception& e) { \
		UNREFERENCED_PARAMETER(e); \
		if (r != nullptr) \
			c->m_rm.unlockRoom(r); \
		throw;  \
	} \

using namespace stdA;

channel::channel(ChannelInfo _ci, uProperty _type) : m_ci(_ci), m_type(_type), m_state(ESTADO::UNITIALIZED), m_rm(_ci.id) {

#if defined(_WIN32)
	InitializeCriticalSection(&m_cs);

	InitializeCriticalSection(&m_cs_invite);
#elif defined(__linux__)
	INIT_PTHREAD_MUTEXATTR_RECURSIVE;
	INIT_PTHREAD_MUTEX_RECURSIVE(&m_cs);
	INIT_PTHREAD_MUTEX_RECURSIVE(&m_cs_invite);
	DESTROY_PTHREAD_MUTEXATTR_RECURSIVE;
#endif

	m_state = ESTADO::INITIALIZED;
};

channel::~channel() {

	if (m_state == ESTADO::INITIALIZED) {
		while (!v_sessions.empty()) {
			v_sessions.erase(v_sessions.begin());
			v_sessions.shrink_to_fit();
		}

#if defined(_WIN32)
		DeleteCriticalSection(&m_cs);
#elif defined(__linux__)
		pthread_mutex_destroy(&m_cs);
#endif

		clear_invite_time();

#if defined(_WIN32)
		DeleteCriticalSection(&m_cs_invite);
#elif defined(__linux__)
		pthread_mutex_destroy(&m_cs_invite);
#endif
	}

	m_state = ESTADO::UNITIALIZED;
};

void channel::enterChannel(player& _session) {

	if (!_session.getState())
		throw exception("[channel::enterChannel][Error] player nao esta conectado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 1));

	if (_session.m_pi.channel != INVALID_CHANNEL)
		throw exception("[channel::enterChannel][Error] player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] ja esta conectado em outro canal.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 2));

	addSession(&_session);

	packet p;

	packet_func::pacote095(p, &_session, 0x102);
	packet_func::session_send(p, &_session, 0);

	packet_func::pacote04E(p, &_session, 1);
	packet_func::session_send(p, &_session, 0);

	sPremiumSystem::getInstance().checkEndTimeTicket(_session);
};

void channel::leaveChannel(player& _session, int _option) {

	try {

		if (_session.m_pi.lobby != (unsigned char)~0)
			leaveLobby(_session, _option);
		else
			leaveRoom(_session, _option);

		removeSession(&_session);

	}catch (exception& e) {

		removeSession(&_session);

		_smp::message_pool::getInstance().push(new message("[channel::leaveChannel][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (!STDA_ERROR_CHECK_SOURCE_AND_ERROR(e.getCodeError(), STDA_ERROR_TYPE::CHANNEL, 1))
			throw;
	}
};

void channel::checkEnterChannel(player& _session) {

	if (!_session.getState())
		throw exception("[channel::checkEnterChannel][Error] player nao esta conectado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 1));

	if (!_session.m_pi.m_cap.stBit.game_master) {

		if (_session.m_pi.level < m_ci.min_level_allow || _session.m_pi.level > m_ci.max_level_allow)
			throw exception("[channel::checkEnterChannel][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + ", LEVEL=" + std::to_string(_session.m_pi.level)
					+ "] nao tem o level necessario para entrar no canal[ID=" + std::to_string((unsigned short)m_ci.id) + ", MIN=" + std::to_string(m_ci.min_level_allow)
					+ ", MAX=" + std::to_string(m_ci.max_level_allow) + "].", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 3));

		if (m_ci.flag.stBit.only_rookie && _session.m_pi.level > PlayerInfo::enLEVEL::ROOKIE_A)
			throw exception("[channel::checkEnterChannel][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + ", LEVEL=" + std::to_string(_session.m_pi.level)
					+ "] nao tem o level necessario para entrar no canal[ID=" + std::to_string((unsigned short)m_ci.id) + ", MIN=" + std::to_string(m_ci.min_level_allow)
					+ ", MAX=" + std::to_string(m_ci.max_level_allow) + "] com a flag So Rookie.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 3));

		if (m_ci.flag.stBit.junior_bellow && _session.m_pi.level > PlayerInfo::enLEVEL::JUNIOR_A)
			throw exception("[channel::checkEnterChannel][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + ", LEVEL=" + std::to_string(_session.m_pi.level)
					+ "] nao tem o level necessario para entrar no canal[ID=" + std::to_string((unsigned short)m_ci.id) + ", MIN=" + std::to_string(m_ci.min_level_allow)
					+ ", MAX=" + std::to_string(m_ci.max_level_allow) + "] com a flag Junior A pra baixo.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 3));

		if (m_ci.flag.stBit.junior_above && _session.m_pi.level < PlayerInfo::enLEVEL::JUNIOR_E)
			throw exception("[channel::checkEnterChannel][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + ", LEVEL=" + std::to_string(_session.m_pi.level)
					+ "] nao tem o level necessario para entrar no canal[ID=" + std::to_string((unsigned short)m_ci.id) + ", MIN=" + std::to_string(m_ci.min_level_allow)
					+ ", MAX=" + std::to_string(m_ci.max_level_allow) + "] com a flag Junior E pra cima.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 3));

		if (m_ci.flag.stBit.junior_between_senior && (_session.m_pi.level < PlayerInfo::enLEVEL::JUNIOR_E || _session.m_pi.level > PlayerInfo::enLEVEL::SENIOR_A))
			throw exception("[channel::checkEnterChannel][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + ", LEVEL=" + std::to_string(_session.m_pi.level)
					+ "] nao tem o level necessario para entrar no canal[ID=" + std::to_string((unsigned short)m_ci.id) + ", MIN=" + std::to_string(m_ci.min_level_allow)
					+ ", MAX=" + std::to_string(m_ci.max_level_allow) + "] com a flag junior E a Senior A.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 3));

		if (m_ci.flag.stBit.beginner_between_junior && (_session.m_pi.level < PlayerInfo::enLEVEL::BEGINNER_E || _session.m_pi.level > PlayerInfo::enLEVEL::JUNIOR_A))
			throw exception("[channel::checkEnterChannel][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + ", LEVEL=" + std::to_string(_session.m_pi.level)
					+ "] nao tem o level necessario para entrar no canal[ID=" + std::to_string((unsigned short)m_ci.id) + ", MIN=" + std::to_string(m_ci.min_level_allow)
					+ ", MAX=" + std::to_string(m_ci.max_level_allow) + "] com a flag Beginner E a Junior A.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 3));
	}
};

ChannelInfo* channel::getInfo() {
	ChannelInfo* ci = nullptr;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	ci = &m_ci;

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif

	return ci;
};

unsigned char channel::getId() {
	return m_ci.id;
};

PlayerCanalInfo* channel::getPlayerInfo(player *_session) {

	if (_session == nullptr)
		throw exception("[channel::getPlayerInfo][Error] _session is nullptr.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 12, 1));

	PlayerCanalInfo *pci = nullptr;
	std::map< player*, PlayerCanalInfo >::iterator i;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	if ((i = m_player_info.find(_session)) != m_player_info.end())
		pci = &i->second;

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif

	return pci;
};

void channel::checkInviteTime() {

#if defined(_WIN32)
	EnterCriticalSection(&m_cs_invite);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs_invite);
#endif

	try {

		for (auto i = 0u; i < v_invite.size(); ++i) {

			if (getLocalTimeDiff(v_invite[i].time) >= (STDA_INVITE_TIME_MILLISECONDS * STDA_10_MICRO_PER_MILLI)) {

				if (send_time_out_invite(v_invite[i])) {

					v_invite.erase(v_invite.begin() + i-- );
				}
			}
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::checkInviteTime][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs_invite);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs_invite);
#endif
};

bool channel::isFull() {

	bool ret = false;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	ret = m_ci.curr_user >= m_ci.max_user;

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif

	return ret;
};

void channel::enterLobby(player& _session, unsigned char _lobby) {

	if (!_session.getState())
		throw exception("[channel::enterLobby][Error] player[UID_TRASH=" + std::to_string(_session.m_pi.uid)
				+ "] nao esta conectado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0));

	if (_session.m_pi.lobby != (unsigned char)~0)
		throw exception("[channel::enterLobby][Error] player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] ja esta na lobby.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 5, 0));

	_session.m_pi.lobby = (_lobby == (unsigned char)~0 || _lobby == 0) ? 1  : _lobby;
	_session.m_pi.place = 0u;

	updatePlayerInfo(_session);

	packet p;

	std::vector< PlayerCanalInfo > v_pci;
	PlayerCanalInfo *pci = nullptr;

	std::vector< RoomInfo > v_ri = m_rm.getRoomsInfo();

	std::vector< player* > v_sessions = getSessions(_session.m_pi.lobby);

	for (auto i = 0u; i < v_sessions.size(); ++i) {
		if ((pci = getPlayerInfo(v_sessions[i])) != nullptr)
			v_pci.push_back(*pci);
	}

	pci = getPlayerInfo(&_session);

	if (packet_func::pacote046(p, &_session, std::vector< PlayerCanalInfo > { v_pci.front() }, 4))
		packet_func::session_send(p, &_session, 0);

	v_pci.erase(v_pci.begin());
	v_pci.shrink_to_fit();

	if (v_pci.size() > 0)
		if (packet_func::pacote046(p, &_session, v_pci, 5))
			packet_func::session_send(p, &_session, 0);

	if (packet_func::pacote047(p, v_ri, 0))
		packet_func::session_send(p, &_session, 0);

	if (packet_func::pacote046(p, &_session, std::vector< PlayerCanalInfo > { (pci == nullptr) ? PlayerCanalInfo() : *pci }, 1))
		packet_func::channel_broadcast(*this, p, 0);

	v_pci.clear();
	v_pci.shrink_to_fit();
};

void channel::leaveLobby(player& _session, int _option) {

	try {
		leaveRoom(_session, _option);
	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::leaveLobby][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	_session.m_pi.lobby = ~0;
	_session.m_pi.place = 0u;

	updatePlayerInfo(_session);

	sendUpdatePlayerInfo(_session, 2);
};

void channel::enterLobbyMultiPlayer(player& _session) {
	CHECK_SESSION_BEGIN("enterLobbyMultiPlayer");

	try {

		enterLobby(_session, 1 );

		packet p;

		packet_func::pacote0F5(p, &_session);
		packet_func::session_send(p, &_session, 0);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::enterLobbyMultiPlayer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

};

void channel::leaveLobbyMultiPlayer(player& _session) {
	CHECK_SESSION_BEGIN("leaveLobbyMultiPlayer");

	try {

		leaveLobby(_session);

		if (
#if defined(_WIN32)
			_session.m_sock != INVALID_SOCKET
#elif defined(__linux__)
			_session.m_sock.fd != INVALID_SOCKET
#endif
		) {

			packet p;

			packet_func::pacote0F6(p, &_session);
			packet_func::session_send(p, &_session, 0);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::leaveLobbyMultiPlayer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::enterLobbyGrandPrix(player& _session) {

	packet p;

	try {

		if (!sgs::gs::getInstance().getInfo().propriedade.stBit.grand_prix)
			throw exception("[channel::enterLobbyGrandPrix][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou entrar na lobby Grand Prix, mas ele esta desativo. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x750001));

		enterLobby(_session, 176u );

		p.init_plain((unsigned short)0x250);

		p.addUint32(0u);

		p.addUint32(sgs::gs::getInstance().getInfo().rate.countBitGrandPrixEvent());

		for (auto& el : sgs::gs::getInstance().getInfo().rate.getValueBitGrandPrixEvent())
			p.addUint32(el);

		p.addUint32((uint32_t)_session.m_pi.v_gpc.size());

		for (auto& el : _session.m_pi.v_gpc)
			p.addBuffer(&el, sizeof(GrandPrixClear));

		p.addFloat(_session.m_pi.ui.getMediaScore());

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::enterLobbyGrandPrix][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x250);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x750000);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::leaveLobbyGrandPrix(player& _session) {
	CHECK_SESSION_BEGIN("leaveLobbyGrandPrix");

	try {

		leaveLobby(_session);

		if (
#if defined(_WIN32)
			_session.m_sock != INVALID_SOCKET
#elif defined(__linux__)
			_session.m_sock.fd != INVALID_SOCKET
#endif
		) {

			packet p((unsigned short)0x251);

			p.addUint32(0u);

			packet_func::session_send(p, &_session, 0);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::leaveLobbyGrandPrix][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

std::vector< stPlayerReward > channel::getAllEligibleToGoldenTime() {

	auto getRoomInfoLambda = [&](player* _p) -> std::pair< bool , RoomInfoEx > {

		std::pair< bool , RoomInfoEx > ret{ false, RoomInfoEx{ 0u } };

		BEGIN_FIND_ROOM(_p->m_pi.mi.sala_numero);

		if (r != nullptr) {

			ret.second = *r->getInfo();

			ret.first = r->isGaming();

		}else
			_smp::message_pool::getInstance().push(new message("[channel::isGoldenTimeGood::lambda(getRoomInfo)][Error][WARNNING] player[UID=" + std::to_string(_p->m_pi.uid) + "] esta na sala[NUMERO="
					+ std::to_string(_p->m_pi.mi.sala_numero) + "], mas ela nao existe. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

		END_FIND_ROOM;

		return ret;
	};

	std::pair< bool , RoomInfoEx > pair_ri;

	std::vector< stPlayerReward > players;

	for (auto& p : v_sessions) {

		if (p == nullptr)
			continue;

		if (p->m_pi.lobby == INVALID_LOBBY)
			continue;

		if (p->m_pi.mi.sala_numero == -1)
			continue;

		pair_ri = getRoomInfoLambda(p);

		if (pair_ri.second.numero == -1)
			continue;

		if (pair_ri.second.tipo == RoomInfo::TIPO::PRACTICE || pair_ri.second.tipo == RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE)
			continue;

		if (pair_ri.second.tipo == RoomInfo::TIPO::GRAND_PRIX && sIff::getInstance().getGrandPrixAba(pair_ri.second.grand_prix.dados_typeid) == IFF::GrandPrixData::GP_ABA::ROOKIE
				&& sIff::getInstance().isGrandPrixNormal(pair_ri.second.grand_prix.dados_typeid))
			continue;

		if (pair_ri.second.tipo != RoomInfo::TIPO::LOUNGE && !pair_ri.first)
			continue;

		players.push_back({ p->m_pi.uid, p->m_pi.m_cap.stBit.premium_user == 1u, pair_ri.first });
	}

	return players;
};

void channel::sendFireWorksWinnerGoldenTime(std::vector< stPlayerReward >& _winners) {

	player* p = nullptr;
	packet pckt;

	for (auto& el : _winners) {

		try {

			if ((p = findSessionByUID(el.uid)) == nullptr)
				continue;

			if (p->m_pi.mi.sala_numero == -1 || p->m_pi.lobby == INVALID_LOBBY)
				continue;

			BEGIN_FIND_ROOM(p->m_pi.mi.sala_numero);

			if (r != nullptr) {

				if (r->getInfo()->tipo == RoomInfo::TIPO::LOUNGE) {

					packet_func::pacote04B(pckt, p, ChangePlayerItemRoom::TYPE_CHANGE::TC_ITEM_EFFECT_LOUNGE, 0 , ChangePlayerItemRoom::stItemEffectLounge::TYPE_EFFECT::TE_TWILIGHT);
					packet_func::room_broadcast(*r, pckt, 1);
				}

			}else
				_smp::message_pool::getInstance().push(new message("[channel::sendFireWorksWinnerGoldenTime][Error][WARNNING] player[UID=" + std::to_string(p->m_pi.uid) + "] esta na sala[NUMERO="
						+ std::to_string(p->m_pi.mi.sala_numero) + "], mas ela nao existe. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

			END_FIND_ROOM;

		}catch (exception& e) {

			_smp::message_pool::getInstance().push(new message("[channel::sendFireWorksWinnerGoldenTime][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		}
	}
};

channel::LEAVE_ROOM_STATE channel::leaveRoom(player& _session, int _option) {

	LEAVE_ROOM_STATE state = LEAVE_ROOM_STATE::DO_NOTHING;

	BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

	if (r != nullptr) {

			packet p;
			int opt = 0;

			try {

				if (r->isInvited(_session)) {

					auto ici = r->deleteInvited(_session);

					deleteInviteTimeRequest(ici);

				}else
					opt = r->leave(_session, _option);

				auto& all_invite = r->getAllInvite();

				if (r->getNumPlayers() == all_invite.size()) {

					player *s = nullptr;
					InviteChannelInfo ici{ 0 };

					while (all_invite.size() > 0) {

						s = nullptr;
						ici.clear();

						if ((s = sgs::gs::getInstance().findPlayer(all_invite.begin()->invited_uid)) == nullptr) {

							ici = r->deleteInvited(all_invite.begin()->invited_uid);

						}else {

							ici = r->deleteInvited(*s);
						}

						if (ici.room_number >= 0 && ici.invited_uid > 0u && ici.invite_uid > 0u)
							deleteInviteTimeRequest(ici);
					}
				}

			}catch (exception& e) {

				_smp::message_pool::getInstance().push(new message("[channel::leaveRoom][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
			}

			updatePlayerInfo(_session);

			if (r->getNumPlayers() > 0 || opt == 0 ) {

				r->sendUpdate();

				try {
					r->sendCharacter(_session, 2);
				}catch (exception& e) {
					_smp::message_pool::getInstance().push(new message("[channel::leaveRoom][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
				}

				sendUpdatePlayerInfo(_session, 3);

				sendUpdateRoomInfo(*const_cast< RoomInfoEx* >(r->getInfo()), 3);

				try {

					if (opt == 0x801 && r->getNumPlayers() > 0)
						while (r->getNumPlayers() > 0)
							if (leaveRoom(*r->getSessions().front(), 0x800) == LEAVE_ROOM_STATE::ROOM_DESTROYED)
								break;

				}catch (exception& e) {

					_smp::message_pool::getInstance().push(new message("[channel::leaveRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
				}

			}else {

				RoomInfoEx ri = *r->getInfo();

				r->setDestroying();

				m_rm.destroyRoom(r);

				sendUpdatePlayerInfo(_session, 3);

				sendUpdateRoomInfo(ri, 2);

				state = LEAVE_ROOM_STATE::ROOM_DESTROYED;
			}

			if (state < LEAVE_ROOM_STATE::ROOM_DESTROYED)
				state = LEAVE_ROOM_STATE::SEND_UPDATE_CLIENT;

	}else if (_option == 1)
		_smp::message_pool::getInstance().push(new message("[channel::leaveRoom][Error][WARNNING] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou sair da sala[NUMERO="
				+ std::to_string(_session.m_pi.mi.sala_numero) + "], mas ela nao existe. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

	END_FIND_ROOM;

	return state;
}

channel::LEAVE_ROOM_STATE channel::leaveRoomMultiPlayer(player& _session, int _option) {

	auto state = leaveRoom(_session, _option);

	if (state > LEAVE_ROOM_STATE::DO_NOTHING &&
#if defined(_WIN32)
		_session.m_sock != INVALID_SOCKET
#elif defined(__linux__)
		_session.m_sock.fd != INVALID_SOCKET
#endif
	) {

		packet p;

		packet_func::pacote04C(p, &_session, -1 );
		packet_func::session_send(p, &_session, 0);
	}

	return state;
}

channel::LEAVE_ROOM_STATE channel::leaveRoomGrandPrix(player& _session, int _option) {

	auto state = leaveRoom(_session, _option);

	if (state > LEAVE_ROOM_STATE::DO_NOTHING &&
#if defined(_WIN32)
		_session.m_sock != INVALID_SOCKET
#elif defined(__linux__)
		_session.m_sock.fd != INVALID_SOCKET
#endif
	) {

		packet p((unsigned short)0x254);

		p.addUint32(0u);

		p.addInt16(-1);

		packet_func::session_send(p, &_session, 1);
	}

	return state;
}

channel::LEAVE_ROOM_STATE channel::kickPlayerRoom(player& _session, unsigned char force) {

	auto state = leaveRoom(_session, (force == 1u) ? 3  : 0x800  );

	if (state > LEAVE_ROOM_STATE::DO_NOTHING &&
#if defined(_WIN32)
		_session.m_sock != INVALID_SOCKET
#elif defined(__linux__)
		_session.m_sock.fd != INVALID_SOCKET
#endif
	) {

		packet p((unsigned short)0x7E);

		p.addUint32(0x800);

		packet_func::session_send(p, &_session, 1);
	}

	return state;
}

std::vector< player* > channel::getSessions(unsigned char _lobby) {
	std::vector< player* > v_session;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	for (auto i = 0u; i < v_sessions.size(); ++i)
		if (v_sessions[i] != nullptr && v_sessions[i]->getState() && v_sessions[i]->m_pi.channel != INVALID_CHANNEL
				&& (_lobby == (unsigned char)~0 || v_sessions[i]->m_pi.lobby != (unsigned char)~0))
			v_session.push_back(v_sessions[i]);

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif

	return v_session;
};

void channel::makeGrandZodiacEventRoom(range_time& _rt) {

	try {

		constexpr char GRAND_ZODIAC_EVENT_INT_NAME[] = "HIO Event (Itermediare)";
		constexpr char GRAND_ZODIAC_EVENT_ADV_NAME[] = "HIO Event (Advance)";

		RoomInfoEx ri{ 0 };
		RoomGrandZodiacEvent *r = nullptr;

		auto num_rooms = ((v_sessions.size() % 200) == 0) ? v_sessions.size() / 200 : v_sessions.size() / 200 + 1;

		if (num_rooms > 0) {

			auto rooms = m_rm.getAllRoomsGrandZodiacEvent();

			if (rooms.empty()) {

				if (_rt.m_type == range_time::eTYPE_MAKE_ROOM::TMR_MAKE_ALL || _rt.m_type == range_time::eTYPE_MAKE_ROOM::TMR_MAKE_INTERMEDIARE) {

					ri.time_30s = 7 * 60000;
					ri.tipo = RoomInfo::TIPO::GRAND_ZODIAC_INT;
					ri.qntd_hole = 1;
					ri.course = RoomInfo::eCOURSE::GRAND_ZODIAC;
					ri.max_player = 100;

					if (m_ci.flag.stBit.junior_bellow  || m_ci.flag.stBit.only_rookie )
						ri.channel_rookie = 1;

#if defined(_WIN32)
					memcpy_s(ri.nome, sizeof(ri.nome), GRAND_ZODIAC_EVENT_INT_NAME, (sizeof(GRAND_ZODIAC_EVENT_INT_NAME) > sizeof(ri.nome) ? sizeof(ri.nome) : sizeof(GRAND_ZODIAC_EVENT_INT_NAME)));
#elif defined(__linux__)
					memcpy(ri.nome, GRAND_ZODIAC_EVENT_INT_NAME, (sizeof(GRAND_ZODIAC_EVENT_INT_NAME) > sizeof(ri.nome) ? sizeof(ri.nome) : sizeof(GRAND_ZODIAC_EVENT_INT_NAME)));
#endif

					for (auto i = 0u; i < num_rooms; ++i) {

						try {

							r = m_rm.makeRoomGrandZodiacEvent(m_ci.id, ri);

							if (r == nullptr)
								throw exception("[channel::makeGrandZodiacEventRoom][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
										+ "] tentou criar a sala Grand Zodiac Event, mas deu erro na criacao. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 8, 0));

#ifdef _DEBUG
							_smp::message_pool::getInstance().push(new message("[channel::makeGrandZodiacEventRoom][Log] New Room Maked.", CL_FILE_LOG_AND_CONSOLE));
#endif

							sendUpdateRoomInfo(*const_cast<RoomInfoEx*>(r->getInfo()), 1);

							if (r != nullptr)
								m_rm.unlockRoom(r);

						}catch (exception& e) {

							if (r != nullptr)
								m_rm.unlockRoom(r);

							_smp::message_pool::getInstance().push(new message("[channel::makeGrandZodiacEventRoom][ErrorSystem][make] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
						}
					}
				}

				if (_rt.m_type == range_time::eTYPE_MAKE_ROOM::TMR_MAKE_ALL || _rt.m_type == range_time::eTYPE_MAKE_ROOM::TMR_MAKE_ADVANCED) {

					ri.clear();

					if (m_ci.flag.stBit.junior_bellow  || m_ci.flag.stBit.only_rookie )
						ri.channel_rookie = 1;

					ri.time_30s = 7 * 60000;
					ri.tipo = RoomInfo::TIPO::GRAND_ZODIAC_ADV;
					ri.qntd_hole = 1;
					ri.course = RoomInfo::eCOURSE::GRAND_ZODIAC;
					ri.max_player = 100;

#if defined(_WIN32)
					memcpy_s(ri.nome, sizeof(ri.nome), GRAND_ZODIAC_EVENT_ADV_NAME, (sizeof(GRAND_ZODIAC_EVENT_ADV_NAME) > sizeof(ri.nome) ? sizeof(ri.nome) : sizeof(GRAND_ZODIAC_EVENT_ADV_NAME)));
#elif defined(__linux__)
					memcpy(ri.nome, GRAND_ZODIAC_EVENT_ADV_NAME, (sizeof(GRAND_ZODIAC_EVENT_ADV_NAME) > sizeof(ri.nome) ? sizeof(ri.nome) : sizeof(GRAND_ZODIAC_EVENT_ADV_NAME)));
#endif

					for (auto i = 0u; i < num_rooms; ++i) {

						try {

							r = m_rm.makeRoomGrandZodiacEvent(m_ci.id, ri);

							if (r == nullptr)
								throw exception("[channel::makeGrandZodiacEventRoom][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
										+ "] tentou criar a sala Grand Zodiac Event, mas deu erro na criacao. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 8, 0));

#ifdef _DEBUG
							_smp::message_pool::getInstance().push(new message("[channel::makeGrandZodiacEventRoom][Log] New Room Maked.", CL_FILE_LOG_AND_CONSOLE));
#endif

							sendUpdateRoomInfo(*const_cast<RoomInfoEx*>(r->getInfo()), 1);

							if (r != nullptr)
								m_rm.unlockRoom(r);

						}catch (exception& e) {

							if (r != nullptr)
								m_rm.unlockRoom(r);

							_smp::message_pool::getInstance().push(new message("[channel::makeGrandZodiacEventRoom][ErrorSystem][make] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
						}
					}
				}

			}else {

				size_t count = 0ull;

				if (_rt.m_type == range_time::eTYPE_MAKE_ROOM::TMR_MAKE_ALL || _rt.m_type == range_time::eTYPE_MAKE_ROOM::TMR_MAKE_INTERMEDIARE) {

					if ((count = std::count_if(rooms.begin(), rooms.end(), [](auto& _el) {
						return _el->getInfo()->tipo == RoomInfo::TIPO::GRAND_ZODIAC_INT;
					})) < num_rooms) {

						ri.time_30s = 7 * 60000;
						ri.tipo = RoomInfo::TIPO::GRAND_ZODIAC_INT;
						ri.qntd_hole = 1;
						ri.course = RoomInfo::eCOURSE::GRAND_ZODIAC;
						ri.max_player = 100;

						if (m_ci.flag.stBit.junior_bellow  || m_ci.flag.stBit.only_rookie )
							ri.channel_rookie = 1;

#if defined(_WIN32)
						memcpy_s(ri.nome, sizeof(ri.nome), GRAND_ZODIAC_EVENT_INT_NAME, (sizeof(GRAND_ZODIAC_EVENT_INT_NAME) > sizeof(ri.nome) ? sizeof(ri.nome) : sizeof(GRAND_ZODIAC_EVENT_INT_NAME)));
#elif defined(__linux__)
						memcpy(ri.nome, GRAND_ZODIAC_EVENT_INT_NAME, (sizeof(GRAND_ZODIAC_EVENT_INT_NAME) > sizeof(ri.nome) ? sizeof(ri.nome) : sizeof(GRAND_ZODIAC_EVENT_INT_NAME)));
#endif

						for (auto i = count; i < num_rooms; ++i) {

							try {

								r = m_rm.makeRoomGrandZodiacEvent(m_ci.id, ri);

								if (r == nullptr)
									throw exception("[channel::makeGrandZodiacEventRoom][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
											+ "] tentou criar a sala Grand Zodiac Event, mas deu erro na criacao. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 8, 0));

#ifdef _DEBUG
								_smp::message_pool::getInstance().push(new message("[channel::makeGrandZodiacEventRoom][Log] New Room Maked.", CL_FILE_LOG_AND_CONSOLE));
#endif

								sendUpdateRoomInfo(*const_cast<RoomInfoEx*>(r->getInfo()), 1);

								if (r != nullptr)
									m_rm.unlockRoom(r);

							}catch (exception& e) {

								if (r != nullptr)
									m_rm.unlockRoom(r);

								_smp::message_pool::getInstance().push(new message("[channel::makeGrandZodiacEventRoom][ErrorSystem][make] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
							}
						}
					}
				}

				if (_rt.m_type == range_time::eTYPE_MAKE_ROOM::TMR_MAKE_ALL || _rt.m_type == range_time::eTYPE_MAKE_ROOM::TMR_MAKE_ADVANCED) {

					if ((count = std::count_if(rooms.begin(), rooms.end(), [](auto& _el) {
						return _el->getInfo()->tipo == RoomInfo::TIPO::GRAND_ZODIAC_ADV;
					})) < num_rooms) {

						ri.clear();

						ri.time_30s = 7 * 60000;
						ri.tipo = RoomInfo::TIPO::GRAND_ZODIAC_ADV;
						ri.qntd_hole = 1;
						ri.course = RoomInfo::eCOURSE::GRAND_ZODIAC;
						ri.max_player = 100;

						if (m_ci.flag.stBit.junior_bellow  || m_ci.flag.stBit.only_rookie )
							ri.channel_rookie = 1;

#if defined(_WIN32)
						memcpy_s(ri.nome, sizeof(ri.nome), GRAND_ZODIAC_EVENT_ADV_NAME, (sizeof(GRAND_ZODIAC_EVENT_ADV_NAME) > sizeof(ri.nome) ? sizeof(ri.nome) : sizeof(GRAND_ZODIAC_EVENT_ADV_NAME)));
#elif defined(__linux__)
						memcpy(ri.nome, GRAND_ZODIAC_EVENT_ADV_NAME, (sizeof(GRAND_ZODIAC_EVENT_ADV_NAME) > sizeof(ri.nome) ? sizeof(ri.nome) : sizeof(GRAND_ZODIAC_EVENT_ADV_NAME)));
#endif

						for (auto i = count; i < num_rooms; ++i) {

							try {

								r = m_rm.makeRoomGrandZodiacEvent(m_ci.id, ri);

								if (r == nullptr)
									throw exception("[channel::makeGrandZodiacEventRoom][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
											+ "] tentou criar a sala Grand Zodiac Event, mas deu erro na criacao. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 8, 0));

#ifdef _DEBUG
								_smp::message_pool::getInstance().push(new message("[channel::makeGrandZodiacEventRoom][Log] New Room Maked.", CL_FILE_LOG_AND_CONSOLE));
#endif

								sendUpdateRoomInfo(*const_cast<RoomInfoEx*>(r->getInfo()), 1);

								if (r != nullptr)
									m_rm.unlockRoom(r);

							}catch (exception& e) {

								if (r != nullptr)
									m_rm.unlockRoom(r);

								_smp::message_pool::getInstance().push(new message("[channel::makeGrandZodiacEventRoom][ErrorSystem][make] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
							}
						}
					}
				}
			}
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::makeGrandZodiacEventRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::makeBotGMEventRoom(stRangeTime& _rt, std::vector< stReward > _reward) {

	try {

		constexpr char BOT_GM_EVENT_NAME[] = "Bot GM Event";

		RoomInfoEx ri{ 0 };
		RoomBotGMEvent *r = nullptr;

		auto rooms = m_rm.getAllRoomsBotGMEvent();

		if (rooms.empty()) {

			ri.clear();

			if (m_ci.flag.stBit.junior_bellow  || m_ci.flag.stBit.only_rookie )
				ri.channel_rookie = 1;

			ri.time_30s = 35 * 60000;
			ri.tipo = RoomInfo::TIPO::TOURNEY;
			ri.qntd_hole = 18;
			ri.course = RoomInfo::eCOURSE::RANDOM;
			ri.max_player = 250;
			ri.modo = RoomInfo::MODO::M_SHUFFLE;

#if defined(_WIN32)
			memcpy_s(ri.nome, sizeof(ri.nome), BOT_GM_EVENT_NAME, (sizeof(BOT_GM_EVENT_NAME) > sizeof(ri.nome) ? sizeof(ri.nome) : sizeof(BOT_GM_EVENT_NAME)));
#elif defined(__linux__)
			memcpy(ri.nome, BOT_GM_EVENT_NAME, (sizeof(BOT_GM_EVENT_NAME) > sizeof(ri.nome) ? sizeof(ri.nome) : sizeof(BOT_GM_EVENT_NAME)));
#endif

			try {

				r = m_rm.makeRoomBotGMEvent(m_ci.id, ri, _reward);

				if (r == nullptr)
					throw exception("[channel::makeBotGMEventRoom][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
							+ "] tentou criar a sala Bot GM Event, mas deu erro na criacao. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 8, 0));

#ifdef _DEBUG
				_smp::message_pool::getInstance().push(new message("[channel::makeBotGMEventRoom][Log] New Room Maked.", CL_FILE_LOG_AND_CONSOLE));
#endif

				sendUpdateRoomInfo(*const_cast<RoomInfoEx*>(r->getInfo()), 1);

				if (r != nullptr)
					m_rm.unlockRoom(r);

			}catch (exception& e) {

				if (r != nullptr)
					m_rm.unlockRoom(r);

				_smp::message_pool::getInstance().push(new message("[channel::makeBotGMEventRoom][ErrorSystem][make] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
			}

		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::makeBotGMEventRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

bool channel::execSmartCalculatorCmd(player& _session, std::string& _msg, eTYPE_CALCULATOR_CMD _type) {
	CHECK_SESSION_BEGIN("execSmartCalculatorCmd");

	bool ret = false;

	try {

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::execSmartCalculatorCmd][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou executar Smart Calculator Cmd " + (_session.m_pi.mi.sala_numero != -1
						? "na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "], mas ela nao existe."
						: "mas ele nao esta em nenhum sala."), STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0));

		if (!_session.m_pi.m_cap.stBit.game_master && !_session.m_pi.m_cap.stBit.gm_normal || r->getInfo()->tipo == RoomInfo::TIPO::LOUNGE)
			if (r->getInfo()->tipo == RoomInfo::TIPO::LOUNGE || r->getInfo()->tipo == RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE || r->getInfo()->tipo == RoomInfo::TIPO::PRACTICE
					|| (r->getInfo()->tipo == RoomInfo::TIPO::GRAND_PRIX
						&& sIff::getInstance().getGrandPrixAba(r->getInfo()->grand_prix.dados_typeid) == IFF::GrandPrixData::GP_ABA::ROOKIE
						&& sIff::getInstance().isGrandPrixNormal(r->getInfo()->grand_prix.dados_typeid)))
				throw exception("[channel::execSmartCalculatorCmd][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou executar Smart Calculator Cmd na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + ", TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo)
						+ "], mas ele nao pode executar esse comando nesse tipo de sala.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10000, 0));

		ret = r->execSmartCalculatorCmd(_session, _msg, _type);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::execSmartCalculatorCmd][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		ret = false;
	}

	return ret;
};

void channel::requestEnterLobby(player& _session, packet *_packet) {
	REQUEST_BEGIN("EnterLobby");

	try {

		CHECK_SESSION_IS_AUTHORIZED("EnterLobby");

		enterLobbyMultiPlayer(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestEnterLobby][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestExitLobby(player& _session, packet *_packet) {
	REQUEST_BEGIN("ExitLobby");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ExitLobby");

		leaveLobbyMultiPlayer(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestExitLobby][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestEnterLobbyGrandPrix(player& _session, packet *_packet) {
	REQUEST_BEGIN("EnterLobbyGrandPrix");

	try {

		CHECK_SESSION_IS_AUTHORIZED("EnterLobbyGranPrix");

		enterLobbyGrandPrix(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestEnterLobbyGrandPrix][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestExitLobbyGrandPrix(player& _session, packet *_packet) {
	REQUEST_BEGIN("ExitLobbyGrandPrix");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ExitLobbyGrandPrix");

		leaveLobbyGrandPrix(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestExitLobbyGrandPrix][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestEnterSpyRoom(player& _session, packet* _packet) {
	REQUEST_BEGIN("EnterSpyRoom");

	try {

		CHECK_SESSION_IS_AUTHORIZED("EnterSpyRoom");

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[channel::requestEnterSpyRoom][Log] Packet Hex: " + hex_util::BufferToHexString(_packet->getBuffer(), _packet->getSize()), CL_FILE_LOG_AND_CONSOLE));
#endif

		requestEnterRoom(_session, _packet);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestEnterSpyRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestMakeRoom(player& _session, packet* _packet) {
	REQUEST_BEGIN("MakeRoom");

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("Packet 0x08.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("MakeRoom");

		int option;
		RoomInfoEx ri = { 0 };
		std::string s_tmp;

		option = _packet->readUint8();

		ri.time_vs = _packet->readUint32();
		ri.time_30s = _packet->readUint32();
		ri.max_player = _packet->readUint8();
		ri.tipo = _packet->readUint8();
		ri.qntd_hole = _packet->readUint8();
		ri.course = RoomInfo::eCOURSE(_packet->readUint8());
		ri.modo = _packet->readUint8();

		if (ri.modo == RoomInfo::M_REPEAT) {
			ri.hole_repeat = _packet->readUint8();
			ri.fixed_hole = _packet->readUint32();
		}

		ri.natural.ulNaturalAndShortGame = _packet->readUint32();

		s_tmp = _packet->readString();

		if (s_tmp.empty())
			throw exception("[channel::requestMakeRoom][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] Nome da sala vazio, Hacker, por que o cliente nao deixa enviar esse pacote sem um nome da sala.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 7, 0));

		{
			size_t _n = s_tmp.size() < sizeof(ri.nome) ? s_tmp.size() : sizeof(ri.nome) - 1;
			memset(ri.nome, 0, sizeof(ri.nome));
			memcpy(ri.nome, s_tmp.data(), _n);
		}

		ri.bet_crazy_club = 0u;

		s_tmp = _packet->readString();

		if (!s_tmp.empty()) {
			ri.senha_flag = 0;

			{
				size_t _n = s_tmp.size() < sizeof(ri.senha) ? s_tmp.size() : sizeof(ri.senha) - 1;
				memset(ri.senha, 0, sizeof(ri.senha));
				memcpy(ri.senha, s_tmp.data(), _n);
			}
		}

		ri.artefato = _packet->readUint32();

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[channel::requestMakeRoom][Log] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
				+ "] Player[UID=" + std::to_string(_session.m_pi.uid) + "] Room Maked. Tipo: " + std::to_string((unsigned)ri.tipo), CL_FILE_LOG_AND_CONSOLE));
#endif

		if (ri.natural.stBit.short_game && ri.tipo != RoomInfo::TIPO::TOURNEY
				&& ri.tipo != RoomInfo::TIPO::SPECIAL_SHUFFLE_COURSE && ri.tipo != RoomInfo::TIPO::GRAND_PRIX)
			ri.natural.stBit.short_game = 0u;

		if (m_type.stBit.natural)
			ri.natural.stBit.natural = 1;

		uFlag flag =   _session.m_pi.block_flag.m_flag.ullFlag;

		if (flag.stBit.all_game && (ri.tipo != RoomInfo::TIPO::LOUNGE || flag.stBit.lounge))
			throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou criar um sala, mas ele nao pode criar nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x780001));

		switch (ri.tipo) {
		case RoomInfo::TIPO::STROKE:
			if (flag.stBit.stroke)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar sala[TIPO="
						+ std::to_string((unsigned short)ri.tipo) + "], mas ele nao pode criar Stroke. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0x770001));
			break;
		case RoomInfo::TIPO::MATCH:
			if (flag.stBit.match)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar sala[TIPO="
						+ std::to_string((unsigned short)ri.tipo) + "], mas ele nao pode criar Match. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0x770001));
			break;
		case RoomInfo::TIPO::TOURNEY:
			if (flag.stBit.tourney)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar sala[TIPO="
						+ std::to_string((unsigned short)ri.tipo) + "], mas ele nao pode criar Tourney. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4, 0x770001));
			break;
		case RoomInfo::TIPO::TOURNEY_TEAM:
			if (flag.stBit.team_tourney)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar sala[TIPO="
						+ std::to_string((unsigned short)ri.tipo) + "], mas ele nao pode criar Team Tourney. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 5, 0x770001));
			break;
		case RoomInfo::TIPO::GUILD_BATTLE:
			if (flag.stBit.guild_battle)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar sala[TIPO="
						+ std::to_string((unsigned short)ri.tipo) + "], mas ele nao pode criar Guild Battle. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 6, 0x770001));
			break;
		case RoomInfo::TIPO::PANG_BATTLE:
			if (flag.stBit.pang_battle)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar sala[TIPO="
						+ std::to_string((unsigned short)ri.tipo) + "], mas ele nao pode criar Pang Battle. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 7, 0x770001));
			break;
		case RoomInfo::TIPO::APPROCH:
			if (flag.stBit.approach)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar sala[TIPO="
						+ std::to_string((unsigned short)ri.tipo) + "], mas ele nao pode criar Approach. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 8, 0x770001));
			break;
		case RoomInfo::TIPO::LOUNGE:
			if (flag.stBit.lounge)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar sala[TIPO="
						+ std::to_string((unsigned short)ri.tipo) + "], mas ele nao pode criar Lounge. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 9, 0x770001));
			break;
		case RoomInfo::TIPO::GRAND_ZODIAC_INT:
		case RoomInfo::TIPO::GRAND_ZODIAC_ADV:
		case RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE:
			if (flag.stBit.grand_zodiac)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar sala[TIPO="
						+ std::to_string((unsigned short)ri.tipo) + "], mas ele nao pode criar Grand Zodiac. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0x770001));
			break;
		case RoomInfo::TIPO::GRAND_PRIX:
			if (flag.stBit.grand_prix)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar sala[TIPO="
						+ std::to_string((unsigned short)ri.tipo) + "], mas ele nao pode criar Grand Prix. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 11, 0x770001));
			break;
		case RoomInfo::TIPO::SPECIAL_SHUFFLE_COURSE:
			if (flag.stBit.ssc)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar sala[TIPO="
						+ std::to_string((unsigned short)ri.tipo) + "], mas ele nao pode criar Special Shuffle Course. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 12, 0x770001));
			break;
		case RoomInfo::TIPO::PRACTICE:
			if (flag.stBit.single_play)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar sala[TIPO="
						+ std::to_string((unsigned short)ri.tipo) + "], mas ele nao pode criar Practice. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 13, 0x770001));
			break;
		}

		if (ri.natural.stBit.short_game && (flag.stBit.team_tourney || flag.stBit.short_game))
			throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar a sala[TIPO="
					+ std::to_string((unsigned short)ri.tipo) + "], mas ele nao pode criar sala Short Game. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 770001));

		if (ri.tipo == RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE && ri.time_30s != (30 * 60000))
			throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar a sala[TIPO="
					+ std::to_string((unsigned short)ri.tipo) + "], mas o tempo eh diferente do tempo do Chip-in Practice[CERTO="
					+ std::to_string(30 * 60000) + ", HACKER=" + std::to_string(ri.time_30s) + "]. Hacker.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 780002));

		if ((ri.tipo >= RoomInfo::TIPO::GRAND_ZODIAC_INT && ri.tipo <= RoomInfo::TIPO::GRAND_ZODIAC_ADV) && !_session.m_pi.m_cap.stBit.game_master)
			throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar a sala[TIPO="
					+ std::to_string((unsigned)ri.tipo) + "], mas ele nao eh GM para poder criar sala de Grand Zodiac Event. Hacker.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 760001));

		if (ri.tipo == RoomInfo::GRAND_PRIX)
			throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar a sala[TIPO="
					+ std::to_string((unsigned short)ri.tipo) + "], mas nao pode criar sala Grand Prix com esse pacote. Hacker.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 15, 0x770001));

		if (m_ci.flag.stBit.junior_bellow  || m_ci.flag.stBit.only_rookie )
			ri.channel_rookie = 1;

		if (ri.tipo == RoomInfo::TIPO::SPECIAL_SHUFFLE_COURSE) {

			auto pWi = _session.m_pi.findWarehouseItemByTypeid(SPECIAL_SHUFFLE_COURSE_TICKET_TYPEID);

			if (pWi == nullptr)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou criar a sala Special Shuffle Course, mas ele nao tem o Ticket[TYPEID="
						+ std::to_string(SPECIAL_SHUFFLE_COURSE_TICKET_TYPEID) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 9, 0));

			if (pWi->STDA_C_ITEM_QNTD < 1)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou criar a sala Special Shuffle Course, mas ele nao tem quantidade suficiente do Ticket[TYPEID=" + std::to_string(SPECIAL_SHUFFLE_COURSE_TICKET_TYPEID)
						+ ", QNTD=" + std::to_string(pWi->STDA_C_ITEM_QNTD) + ", QNTD_REQ=1]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0));

			stItem item{ 0 };

			item.type = 2;
			item.id = pWi->id;
			item._typeid = pWi->_typeid;
			item.qntd = 1;
			item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

			if (item_manager::removeItem(item, _session) <= 0)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou criar a sala Special Shuffle Course, mas nao conseguiu deletar o Ticket[TYPEID=" + std::to_string(item._typeid)
						+ ", ID=" + std::to_string(item.id) + "]. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 11, 0));

			if (ri.natural.stBit.short_game)
				ri.time_30s = 20 * 60000;

#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("[channel::requestMakeRoom][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] deletou 1 SSC Ticket para poder criar uma sala Special Shuffle Course", CL_FILE_LOG_AND_CONSOLE));
#else
			_smp::message_pool::getInstance().push(new message("[channel::requestMakeRoom][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] deletou 1 SSC Ticket para poder criar uma sala Special Shuffle Course", CL_ONLY_FILE_LOG));
#endif

		}

		room *r = nullptr;

		try {

#ifdef _DEBUG

#endif

			deleteInviteTimeResquestByInvited(_session);

			r = m_rm.makeRoom(m_ci.id, ri, &_session);

			if (r == nullptr)
				throw exception("[channel::requestMakeRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou criar a sala, mas deu erro na criacao. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 8, 0));

		#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("New Room Maked.", CL_FILE_LOG_AND_CONSOLE));
		#endif

			updatePlayerInfo(_session);

			r->sendUpdate();

			r->sendMake(_session);

			r->sendCharacter(_session, 0);

			r->sendCharacterStateLounge(_session);

			r->sendWeatherLounge(_session);

			sendUpdateRoomInfo(*const_cast<RoomInfoEx*>(r->getInfo()), 1);

			if (r->getInfo()->tipo != RoomInfo::TIPO::PRACTICE && r->getInfo()->tipo != RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE)
				sendUpdatePlayerInfo(_session, 3);

			if (r->getInfo()->tipo == RoomInfo::TIPO::GUILD_BATTLE)
				r->sendCharacter(_session, 0);

			if (r->getInfo()->tipo == RoomInfo::TIPO::TOURNEY  || r->getInfo()->tipo == RoomInfo::TIPO::SPECIAL_SHUFFLE_COURSE) {

				try {

					if (r->isLocked() && r->checkPass("bot"))
						r->makeBot(_session);

				}catch (exception& e) {
					UNREFERENCED_PARAMETER(e);

				}
			}

			if (r != nullptr)
				m_rm.unlockRoom(r);

		}catch (exception& e) {
			UNREFERENCED_PARAMETER(e);

			if (r != nullptr)
				m_rm.unlockRoom(r);

			throw;
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestMakeRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x49);

		p.addUint16(2);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestEnterRoom(player& _session, packet* _packet) {
	REQUEST_BEGIN("EnterRoom");

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("Packet 0x09.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("EnterRoom");

		short numero = _packet->readInt16();
		std::string senha = _packet->readString();

		BEGIN_FIND_ROOM(numero);

		if (r == nullptr)
			throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou entrar na sala[NUMERO=" + std::to_string(numero) + "], mas ela nao existe.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0));

		uFlag flag =   _session.m_pi.block_flag.m_flag.ullFlag;

		if (flag.stBit.all_game && (r->getInfo()->tipo != RoomInfo::TIPO::LOUNGE || flag.stBit.lounge))
			throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou entrar um sala[NUMERO=" + std::to_string(r->getNumero()) + "], mas ele nao pode entrar em nenhuma sala. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x780001));

		switch (r->getInfo()->tipo) {
		case RoomInfo::TIPO::STROKE:
			if (flag.stBit.stroke)
				throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo) + ", NUMERO=" + std::to_string(r->getNumero())
						+ "], mas ele nao pode entrar Stroke. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0x770001));
			break;
		case RoomInfo::TIPO::MATCH:
			if (flag.stBit.match)
				throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo) + ", NUMERO=" + std::to_string(r->getNumero())
						+ "], mas ele nao pode entrar Match. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0x770001));
			break;
		case RoomInfo::TIPO::TOURNEY:
			if (flag.stBit.tourney)
				throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo) + ", NUMERO=" + std::to_string(r->getNumero())
						+ "], mas ele nao pode entrar Tourney. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4, 0x770001));
			break;
		case RoomInfo::TIPO::TOURNEY_TEAM:
			if (flag.stBit.team_tourney)
				throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo) + ", NUMERO=" + std::to_string(r->getNumero())
						+ "], mas ele nao pode entrar Team Tourney. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 5, 0x770001));
			break;
		case RoomInfo::TIPO::GUILD_BATTLE:
			if (flag.stBit.guild_battle)
				throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo) + ", NUMERO=" + std::to_string(r->getNumero())
						+ "], mas ele nao pode entrar Guild Battle. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 6, 0x770001));
			break;
		case RoomInfo::TIPO::PANG_BATTLE:
			if (flag.stBit.pang_battle)
				throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo) + ", NUMERO=" + std::to_string(r->getNumero())
						+ "], mas ele nao pode entrar Pang Battle. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 7, 0x770001));
			break;
		case RoomInfo::TIPO::APPROCH:
			if (flag.stBit.approach)
				throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo) + ", NUMERO=" + std::to_string(r->getNumero())
						+ "], mas ele nao pode entrar Approach. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 8, 0x770001));
			break;
		case RoomInfo::TIPO::LOUNGE:
			if (flag.stBit.lounge)
				throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo) + ", NUMERO=" + std::to_string(r->getNumero())
						+ "], mas ele nao pode entrar Lounge. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 9, 0x770001));
			break;
		case RoomInfo::TIPO::GRAND_ZODIAC_INT:
		case RoomInfo::TIPO::GRAND_ZODIAC_ADV:
		case RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE:
			if (flag.stBit.grand_zodiac)
				throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo) + ", NUMERO=" + std::to_string(r->getNumero())
						+ "], mas ele nao pode entrar Grand Zodiac. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0x770001));
			break;
		case RoomInfo::TIPO::GRAND_PRIX:
			if (flag.stBit.grand_prix)
				throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo) + ", NUMERO=" + std::to_string(r->getNumero())
						+ "], mas ele nao pode entrar Grand Prix. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 11, 0x770001));
			break;
		case RoomInfo::TIPO::SPECIAL_SHUFFLE_COURSE:
			if (flag.stBit.ssc)
				throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo) + ", NUMERO=" + std::to_string(r->getNumero())
						+ "], mas ele nao pode entrar Special Shuffle Course. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 12, 0x770001));
			break;
		case RoomInfo::TIPO::PRACTICE:
			if (flag.stBit.single_play)
				throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo) + ", NUMERO=" + std::to_string(r->getNumero())
						+ "], mas ele nao pode entrar Practice. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 13, 0x770001));
			break;
		}

		if (r->getInfo()->natural.stBit.short_game && (flag.stBit.team_tourney || flag.stBit.short_game))
			throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou entrar na sala[TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo) + ", NUMERO=" + std::to_string(r->getNumero())
					+ "], mas ele nao pode entrar sala Short Game. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 770001));

		if (r->getInfo()->tipo == RoomInfo::GRAND_PRIX)
			throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou entrar na sala[TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo) + ", NUMERO=" + std::to_string(r->getNumero())
					+ "], mas nao pode entrar na sala Grand Prix com esse pacote. Hacker.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 15, 0x770001));

		if (r->isGaming() && (_session.m_pi.m_cap.stBit.game_master ))
			r->requestSendTimeGame(_session);
		else if (r->isGaming())
			throw exception("[channel::requestEnterRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou entrar na sala[NUMERO=" + std::to_string(numero) + "], mas a sala ja comecou o jogo. Hacker ou Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0));
		else {

			if (!r->isLocked() || r->isInvited(_session) || (_session.m_pi.m_cap.stBit.game_master ) || (!senha.empty() && r->checkPass(senha))) {

				if (r->isInvited(_session)) {

					auto ici = r->deleteInvited(_session);

					deleteInviteTimeRequest(ici);

					if (!r->isFull())
						r->enter(_session);

				}else if (!r->isFull()) {

					deleteInviteTimeResquestByInvited(_session);

					r->enter(_session);

				}else
					throw exception("[channel::requestEnterRoom][WARNING] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
							+ "] tentou entrar na sala[NUMERO=" + std::to_string(numero) + "], mas a sala esta cheia.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0));
			}else
				throw exception("[channel::requestEnterRoom][WARNING] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[NUMERO=" + std::to_string(numero) + "], mas a senha nao eh igual a da sala.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4, 0));

			updatePlayerInfo(_session);

			r->sendUpdate();

			r->sendMake(_session);

			r->sendCharacter(_session, 0);

			r->sendCharacter(_session, 1);

			r->sendCharacterStateLounge(_session);

			r->sendWeatherLounge(_session);

			sendUpdateRoomInfo(*const_cast< RoomInfoEx* >(r->getInfo()), 3);

			if (r->getInfo()->tipo != RoomInfo::TIPO::PRACTICE && r->getInfo()->tipo != RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE)
				sendUpdatePlayerInfo(_session, 3);

			if (r->getInfo()->tipo == RoomInfo::TIPO::GUILD_BATTLE)
				r->sendCharacter(_session, 0);
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestEnterRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x49);

		p.addUint16(2);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestChangeInfoRoom(player& _session, packet* _packet) {
	REQUEST_BEGIN("ChangeInfoRoom");

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("Packet 0x0A.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

	try {

		CHECK_SESSION_IS_AUTHORIZED("ChangeInfoRoom");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestChangeInfoRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou trocar info da sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "], mas a sala nao existe.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0));

		if (r->requestChangeInfoRoom(_session, _packet))
			sendUpdateRoomInfo(*const_cast<RoomInfoEx*>(r->getInfo()), 3);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestChangeInfoRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::ROOM)
			throw;
	}
};

void channel::requestExitRoom(player& _session, packet* _packet) {
	REQUEST_BEGIN("ExitRoom");

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("Packet 0x0F.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

	unsigned char option;
	short flag;
	char senhaEncriptSala[16];

	try {

		CHECK_SESSION_IS_AUTHORIZED("ExitRoom");

		option = _packet->readUint8();
		flag = _packet->readInt16();
		_packet->readBuffer(senhaEncriptSala, sizeof(senhaEncriptSala));

		leaveRoomMultiPlayer(_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestExitRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestShowInfoRoom(player& _session, packet* _packet) {
	REQUEST_BEGIN("ShowInfoRoom");

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("Packet 0x2D.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

	try {

		CHECK_SESSION_IS_AUTHORIZED("ShowInfoRoom");

		short sala_numero = _packet->readInt16();

		BEGIN_FIND_ROOM(sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestShowInfoRoom][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "], Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] pediu info da sala[NUMERO=" + std::to_string(sala_numero) + "] nao existe.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0));

		packet p((unsigned short)0x86);

		RoomInfoEx *ri = const_cast< RoomInfoEx* >(r->getInfo());

		p.addUint32(ri->num_player);
		p.addUint8(ri->qntd_hole);
		p.addUint32((ri->tipo == RoomInfo::TIPO::STROKE || ri->tipo == RoomInfo::TIPO::MATCH || ri->tipo == RoomInfo::TIPO::PANG_BATTLE) ? ri->time_vs : ((ri->tipo == RoomInfo::TIPO::GUILD_BATTLE) ? 0 : ri->time_30s));
		p.addUint8(ri->course);
		p.addUint8(ri->tipo);
		p.addUint8(ri->modo);
		p.addUint32(ri->trofel);

		std::vector< player* > v_session = r->getSessions();
		PlayerCanalInfo *pci = nullptr;

		for (auto i = 0u; i < v_session.size(); ++i) {
			pci = getPlayerInfo(v_session[i]);

			if (pci == nullptr)
				throw exception("[channel::requestShowInfoRoom][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "], Player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] nao tem o info do player na sala[NUMERO=" + std::to_string(sala_numero) + "].", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 11, 0));

			p.addUint32(pci->oid);
			p.addUint8(pci->level);
			p.addUint8(r->requestPlace(*v_session[i]));

			p.addUint32(pci->capability.ulCapability);
			p.addUint32(pci->title);
			p.addUint32(pci->team_point);
		}

		packet_func::session_send(p, &_session, 0);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestShowInfoRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestPlayerLocationRoom(player& _session, packet* _packet) {
	REQUEST_BEGIN("PlayerLocationRoom");

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("Packet 0x63.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

	try {

		CHECK_SESSION_IS_AUTHORIZED("PlayerLocationRoom");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestPlayerLocationRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou trocar localizacao na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "], mas ela nao existe. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0));

		unsigned char type = _packet->readUint8();

		packet p;

		switch (type) {
		case 0:
		{
			_session.m_pi.location.r = _packet->readFloat();

			r->updatePlayerInfo(_session);

			p.init_plain((unsigned short)0xC4);

			p.addUint32(_session.m_oid);
			p.addUint8(type);

			p.addFloat(_session.m_pi.location.r);

			packet_func::room_broadcast(*r, p, 0);

			break;
		}
		case 1:
		{

			p.init_plain((unsigned short)0xC4);

			p.addUint32(_session.m_oid);
			p.addUint8(type);

			p.addString(_packet->readString());

			packet_func::room_broadcast(*r, p, 0);

			break;
		}
		case 4:
		{
			PlayerRoomInfo::stLocation location;

			_packet->readBuffer(&location, sizeof(location));

			_session.m_pi.location = { location.x, location.z, location.r };

			r->updatePlayerInfo(_session);

			p.init_plain((unsigned short)0xC4);

			p.addUint32(_session.m_oid);
			p.addUint8(type);

			p.addBuffer(&location, sizeof(location));

			packet_func::room_broadcast(*r, p, 0);

	#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("X: " + std::to_string(location.x) + "\tZ: " + std::to_string(location.z) + "\tR: " + std::to_string(location.r), CL_FILE_LOG_AND_CONSOLE));
	#endif

			break;
		}
		case 5:
		{
			_session.m_pi.state = _packet->readUint32();

			r->updatePlayerInfo(_session);

			p.init_plain((unsigned short)0xC4);

			p.addUint32(_session.m_oid);
			p.addUint8(type);

			p.addUint32(_session.m_pi.state);

			packet_func::room_broadcast(*r, p, 0);

			break;
		}
		case 6:
		{
			PlayerRoomInfo::stLocation location_add;

			_packet->readBuffer(&location_add, sizeof(location_add));

			_session.m_pi.location.x += location_add.x;
			_session.m_pi.location.z += location_add.z;
			_session.m_pi.location.r = location_add.r;

			r->updatePlayerInfo(_session);

			p.init_plain((unsigned short)0xC4);

			p.addUint32(_session.m_oid);
			p.addUint8(type);

			p.addBuffer(&location_add, sizeof(location_add));

			packet_func::room_broadcast(*r, p, 0);

	#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("X: " + std::to_string(_session.m_pi.location.x) + "\tZ: " + std::to_string(_session.m_pi.location.z) + "\tR: " + std::to_string(_session.m_pi.location.r), CL_FILE_LOG_AND_CONSOLE));
	#endif

			break;
		}
		case 7:
		{

			p.init_plain((unsigned short)0xC4);

			p.addUint32(_session.m_oid);
			p.addUint8(type);

			p.addString(_packet->readString());

			packet_func::room_broadcast(*r, p, 0);

			break;
		}
		case 8:
		{
			_session.m_pi.state_lounge = _packet->readUint32();

			r->updatePlayerInfo(_session);

			p.init_plain((unsigned short)0xC4);

			p.addUint32(_session.m_oid);
			p.addUint8(type);

			p.addUint32(_session.m_pi.state_lounge);

			packet_func::room_broadcast(*r, p, 0);

			break;
		}
		case 10:
		{

			p.init_plain((unsigned short)0xC4);

			p.addUint32(_session.m_oid);
			p.addUint8(type);

			p.addString(_packet->readString());

			packet_func::room_broadcast(*r, p, 0);

			break;
		}
		default:
			throw exception("[channel::requestPlayerLocationRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou trocar localizacao na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "], mas o type desconhecido. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 11, 0));
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestPlayerLocationRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestChangePlayerStateReadyRoom(player& _session, packet* _packet) {
	REQUEST_BEGIN("ChangePlayerStateReadyRoom");

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("Packet 0x0D.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

	try {

		CHECK_SESSION_IS_AUTHORIZED("ChangePlayerStateReadyRoom");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestChangePlayerStateReadyRoom][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "] nao existe.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0));

		unsigned char ready = _packet->readUint8();

		PlayerRoomInfoEx *pri = r->getPlayerInfo(&_session);

		if (pri == nullptr)
			throw exception("[channel::requestChangePlayerStateReadyRoom][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] nao tem o info do player na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "].", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 11, 0));

		pri->state_flag.uFlag.stFlagBit.ready = !ready;

		packet p((unsigned short)0x78);

		p.addUint32(_session.m_oid);
		p.addUint8(ready);

		packet_func::room_broadcast(*r, p, 1);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerStateReadyRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestKickPlayerOfRoom(player& _session, packet* _packet) {
	REQUEST_BEGIN("KickPlayerOfRoom");

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("Packet 0x26.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

	try {

		CHECK_SESSION_IS_AUTHORIZED("KickPlayerOfRoom");

		uint32_t uid = _packet->readUint32();

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestKickPlayerOfRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou chutar um player[UID=" + std::to_string(uid) + "] da sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas sala nao existe. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0));

		if (r->getMaster() != _session.m_pi.uid)
			throw exception("[channel::requestKickPlayerOfRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou chutar um player[UID=" + std::to_string(uid) + "] da sala[NUMERO=" + std::to_string(r->getNumero())
					+ "], mas o player nao eh master da sala para poder chutar(kick) o player. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 11, 0));

		if (!_session.m_pi.m_cap.stBit.game_master && r->isGaming())
			throw exception("[channel::requestKickPlayerOfRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou chutar um player[UID=" + std::to_string(uid) + "] da sala[NUMERO=" + std::to_string(r->getNumero())
					+ "], mas o player eh GM para poder chutar o player da sala com o jogo em andamento.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 13, 0));

		player *kick = r->findSessionByUID(uid);

		if (kick == nullptr)
			throw exception("[channel::requestKickPlayerOfRoom][Error] player[UID=" + std::to_string(uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou chutar um player[UID=" + std::to_string(uid) + "] da sala[NUMERO=" + std::to_string(r->getNumero())
					+ "], mas o player nao existe na sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 12, 0));

		leaveRoomMultiPlayer(*kick, 3);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel:requestKickPlayerOfRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void channel::requestChangePlayerTeamRoom(player& _session, packet* _packet) {
	REQUEST_BEGIN("ChangePlayerTeamRoom");

	try {

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("Packet 0x10.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

		CHECK_SESSION_IS_AUTHORIZED("ChangePlayerTeamRoom");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestChangePlayerTeamRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou trocar de team(time) na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas a sala nao existe. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0));

		r->requestChangeTeam(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerTeamRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestChangePlayerStateAFKRoom(player& _session, packet* _packet) {
	REQUEST_BEGIN("ChangePlayerStateAFKRoom");

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("Packet 0x32.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

	try {

		CHECK_SESSION_IS_AUTHORIZED("ChangePlayerStateAFKRoom");

		unsigned char state = _packet->readUint8();

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestChangePlayerStateAFKRoom][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "] nao existe.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0));

		PlayerRoomInfoEx *pri = r->getPlayerInfo(&_session);
		PlayerCanalInfo *pci = getPlayerInfo(&_session);

		if (pri == nullptr)
			throw exception("[channel::requestChangePlayerStateAFKRoom][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] nao tem o info do player na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "].", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 11, 0));

		if (pci == nullptr)
			throw exception("[channel::requestChangePlayerStateAFKRoom][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] nao tem o info do player no canal.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 12, 0));

		pci->state_flag.sBit.away = pri->state_flag.uFlag.stFlagBit.away = state;

		packet p((unsigned short)0x8E);

		p.addUint32(_session.m_oid);

		p.addUint8(state);

		packet_func::room_broadcast(*r, p, 0);

		if (packet_func::pacote046(p, &_session, std::vector< PlayerCanalInfo > { (pci == nullptr) ? PlayerCanalInfo() : *pci }, 3))
			packet_func::channel_broadcast(*this, p, 0);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[requestChangePlayerStateAFKRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestPlayerStateCharacterLounge(player& _session, packet* _packet) {
	REQUEST_BEGIN("PlayerStateCharacterLounge");

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("Packet 0xEB.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

	try {

		CHECK_SESSION_IS_AUTHORIZED("PlayerStateCharacterLounge");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestPlayerStateCharacterLounge][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "] nao existe.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0));

		if (r->getInfo()->tipo != RoomInfo::TIPO::LOUNGE)
			throw exception("[channel::requestPlayerStateCharacterLounge][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "] nao eh um lounge.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 12, 0));

		auto it = (_session.m_pi.ei.char_info == nullptr) ? _session.m_pi.mp_scl.end() : _session.m_pi.mp_scl.find(_session.m_pi.ei.char_info->id);

		if (it == _session.m_pi.mp_scl.end())
			throw exception("[channel::requestPlayerStateCharacterLounge][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "] nao tem os estados do character na lounge.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 13, 0));

		packet p((unsigned short)0x196);

		p.addUint32(_session.m_oid);

		p.addBuffer(&it->second, sizeof(StateCharacterLounge));

		packet_func::room_broadcast(*r, p, 0);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestPlayerStateCharacterLounge][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestToggleAssist(player& _session, packet *_packet) {
	REQUEST_BEGIN("ToggleAssist");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ToggleAssist");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestToggleAssist][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou alterna Assist Modo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5800101));

		r->requestToggleAssist(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestToggleAssist][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestInvite(player& _session, packet *_packet) {
	REQUEST_BEGIN("Invite");

	packet p;

	try {

		std::string nickname = _packet->readString();
		uint32_t uid = _packet->readUint32();

		CHECK_SESSION_IS_AUTHORIZED("Invite");

		auto s = findSessionByNickname(nickname);

		if (s == nullptr || s->m_pi.uid != uid)
			throw exception("[channel::requestInvite][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou convidar o player[UID=" + std::to_string(uid) + ", NICKNAME=" + nickname + "] para Sala[NUMERO="
					+ std::to_string(_session.m_pi.mi.sala_numero) + "], mas o player nao esta nesse canal. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3000, 23 ));

		if (s->m_pi.mi.sala_numero != -1)
			throw exception("[channel::requestInvite][Warning] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou convidar o player[UID=" + std::to_string(uid) + ", NICKNAME=" + nickname + "] para Sala[NUMERO="
					+ std::to_string(_session.m_pi.mi.sala_numero) + "], mas o player ja esta em outra sala.",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3002, 23 ));

		if (s->m_pi.place != 0)
			throw exception("[channel::requestInvite][Warning] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou convidar o player[UID=" + std::to_string(uid) + ", NICKNAME=" + nickname + "] para Sala[NUMERO="
					+ std::to_string(_session.m_pi.mi.sala_numero) + "], mas o player nao pode ser convidado no momento.",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3002, 23 ));

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestInvite][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou convidar o player[UID=" + std::to_string(uid) + ", NICKNAME=" + nickname + "] para Sala[NUMERO="
					+ std::to_string(_session.m_pi.mi.sala_numero) + "], mas ele nao esta em nenhuma sala para poder convidar. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3001, 23 ));

		auto ici = r->addInvited(_session.m_pi.uid, *s);

		addInviteTimeRequest(ici);

		sendUpdateRoomInfo(*const_cast< RoomInfoEx* >(r->getInfo()), 3);

		p.init_plain((unsigned short)0x12F);

		p.addUint16(0);

		p.addUint32(sgs::gs::getInstance().getUID());

		p.addUint8(m_ci.id);
		p.addInt16(r->getNumero());

		p.addUint32(_session.m_pi.uid);
		p.addString(_session.m_pi.nickname);

		p.addUint32(s->m_pi.uid);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x83);

		p.addUint16(0);

		p.addUint32(sgs::gs::getInstance().getUID());

		p.addUint8(m_ci.id);
		p.addInt16(r->getNumero());

		p.addUint32(_session.m_pi.uid);
		p.addString(_session.m_pi.nickname);

		p.addUint32(s->m_pi.uid);

		packet_func::session_send(p, s, 1);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestInvite][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x12F);

		p.addUint16((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? (unsigned short)STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 23u );

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestCheckInvite(player& _session, packet *_packet) {
	REQUEST_BEGIN("CheckInvite");

	try {

		CHECK_SESSION_IS_AUTHORIZED("CheckInvite");

		uint32_t uid = _packet->readUint32();

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[channel::requestCheckInvite][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] enviou convite para o player[UID="
				+ std::to_string(uid) + "]", CL_FILE_LOG_AND_CONSOLE));
#endif

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestCheckInvite][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestChatTeam(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChatTeam");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ChatTeam");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestChatTeam][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou mandar message no chat do team na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5900201));

		r->requestChatTeam(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestChatTeam][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestExitedFromWebGuild(player& _session, packet *_packet) {
	REQUEST_BEGIN("ExitedFromWebGuild");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ExitedFromWebGuild");

		auto old_pang = _session.m_pi.ui.pang;

		_session.m_pi.updatePang();

		if (old_pang != _session.m_pi.ui.pang) {

			packet p((unsigned short)0xC8);

			p.addUint64(_session.m_pi.ui.pang);
			p.addUint64(0);

			packet_func::session_send(p, &_session, 1);
		}

		if (_session.m_pi.gi.uid > 0) {

			CmdGuildUpdateActivityInfo cmd_guai(_session.m_pi.gi.uid, _session.m_pi.uid, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_guai, nullptr, nullptr);

			cmd_guai.waitEvent();

			if (cmd_guai.getException().getCodeError() != 0)
				throw cmd_guai.getException();

			auto v_info = cmd_guai.getInfo();

			if (!v_info.empty()) {

				packet p;

				for (auto& el : v_info) {

					switch (el.type) {
						case GuildUpdateActivityInfo::TYPE_UPDATE::TU_ACCEPTED_MEMBER:
						{

							p.init_plain((unsigned short)0x01 );

							p.addUint32(el.club_uid);
							p.addUint32(el.player_uid);

							sgs::gs::getInstance().sendCommandToOtherServerWithAuthServer(p, 3 );

							auto s = sgs::gs::getInstance().findPlayer(el.player_uid);

							if (s != nullptr &&
#if defined(_WIN32)
								s->m_sock != INVALID_SOCKET
#elif defined(__linux__)
								s->m_sock.fd != INVALID_SOCKET
#endif
							) {

								CmdMemberInfo cmd_mi(s->m_pi.uid, true);

								snmdb::NormalManagerDB::getInstance().add(0, &cmd_mi, nullptr, nullptr);

								cmd_mi.waitEvent();

								if (cmd_mi.getException().getCodeError() != 0)
									throw cmd_mi.getException();

								auto mi = cmd_mi.getInfo();

								if (mi.guild_uid > 0u) {

									s->m_pi.mi.guild_mark_img_no = mi.guild_mark_img_no;
									s->m_pi.mi.guild_uid = mi.guild_uid;
									s->m_pi.mi.guild_pang = mi.guild_pang;
									s->m_pi.mi.guild_point = mi.guild_point;
#if defined(_WIN32)
									memcpy_s(s->m_pi.mi.guild_name, sizeof(s->m_pi.mi.guild_name), mi.guild_name, sizeof(mi.guild_name));
									memcpy_s(s->m_pi.mi.guild_mark_img, sizeof(s->m_pi.mi.guild_mark_img), mi.guild_mark_img, sizeof(mi.guild_mark_img));
#elif defined(__linux__)
									memcpy(s->m_pi.mi.guild_name, mi.guild_name, sizeof(mi.guild_name));
									memcpy(s->m_pi.mi.guild_mark_img, mi.guild_mark_img, sizeof(mi.guild_mark_img));
#endif

									CmdGuildInfo cmd_gi(s->m_pi.uid, 0, true);

									snmdb::NormalManagerDB::getInstance().add(0, &cmd_gi, nullptr, nullptr);

									cmd_gi.waitEvent();

									if (cmd_gi.getException().getCodeError() != 0)
										throw cmd_gi.getException();

									s->m_pi.gi = cmd_gi.getInfo();

									if (s->m_pi.lobby != (unsigned char)~0) {

										auto c = sgs::gs::getInstance().findChannel(s->m_pi.channel);

										if (c != nullptr) {

											c->updatePlayerInfo(*s);

											c->sendUpdatePlayerInfo(*s, 3);
										}
									}
								}
							}

							break;
						}
						case GuildUpdateActivityInfo::TYPE_UPDATE::TU_EXITED_MEMBER:
						{

							p.init_plain((unsigned short)0x02 );

							p.addUint32(el.club_uid);
							p.addUint32(el.player_uid);

							sgs::gs::getInstance().sendCommandToOtherServerWithAuthServer(p, 3 );

							_session.m_pi.gi.clear();

							_session.m_pi.mi.guild_mark_img_no = 0u;
							_session.m_pi.mi.guild_uid = 0u;
							_session.m_pi.mi.guild_pang = 0ull;
							_session.m_pi.mi.guild_point = 0;
							memset(_session.m_pi.mi.guild_name, 0, sizeof(_session.m_pi.mi.guild_name));
							memset(_session.m_pi.mi.guild_mark_img, 0, sizeof(_session.m_pi.mi.guild_mark_img));

							if (_session.m_pi.lobby != (unsigned char)~0) {

								auto c = sgs::gs::getInstance().findChannel(_session.m_pi.channel);

								if (c != nullptr) {

									c->updatePlayerInfo(_session);

									c->sendUpdatePlayerInfo(_session, 3);
								}
							}

							break;
						}
						case GuildUpdateActivityInfo::TYPE_UPDATE::TU_KICKED_MEMBER:
						{

							p.init_plain((unsigned short)0x03 );

							p.addUint32(el.club_uid);
							p.addUint32(el.player_uid);

							sgs::gs::getInstance().sendCommandToOtherServerWithAuthServer(p, 3 );

							auto s = sgs::gs::getInstance().findPlayer(el.player_uid);

							if (s != nullptr &&
#if defined(_WIN32)
								s->m_sock != INVALID_SOCKET
#elif defined(__linux__)
								s->m_sock.fd != INVALID_SOCKET
#endif
							) {

								s->m_pi.gi.clear();

								s->m_pi.mi.guild_mark_img_no = 0u;
								s->m_pi.mi.guild_uid = 0u;
								s->m_pi.mi.guild_pang = 0ull;
								s->m_pi.mi.guild_point = 0;
								memset(s->m_pi.mi.guild_name, 0, sizeof(s->m_pi.mi.guild_name));
								memset(s->m_pi.mi.guild_mark_img, 0, sizeof(s->m_pi.mi.guild_mark_img));

								if (s->m_pi.lobby != (unsigned char)~0) {

									auto c = sgs::gs::getInstance().findChannel(s->m_pi.channel);

									if (c != nullptr) {

										c->updatePlayerInfo(*s);

										c->sendUpdatePlayerInfo(*s, 3);
									}
								}
							}

							break;
						}
					}

					snmdb::NormalManagerDB::getInstance().add(27, new CmdUpdateGuildUpdateActiviy(el.index), channel::SQLDBResponse, this);
				}
			}
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestExitedFromWebGuild][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestStartGame(player& _session, packet *_packet) {
	REQUEST_BEGIN("StartGame");

	try {

		CHECK_SESSION_IS_AUTHORIZED("StartGame");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestStartGame][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou comecar o jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5900201));

		if (r->requestStartGame(_session, _packet)) {

			if (r->getInfo()->tipo != RoomInfo::TIPO::PRACTICE && r->getInfo()->tipo != RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE) {
				packet p;

				sendUpdateRoomInfo(*const_cast< RoomInfoEx* >(r->getInfo()), 3);
			}
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestStartGame][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

};

void channel::requestInitHole(player& _session, packet *_packet) {
	REQUEST_BEGIN("InitHole");

	try {

		CHECK_SESSION_IS_AUTHORIZED("InitHole");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestInitHole][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou inicializar o hole, no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5900301));

		r->requestInitHole(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestInitHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestFinishLoadHole(player& _session, packet *_packet) {
	REQUEST_BEGIN("FinishLoadHole");

	try {

		CHECK_SESSION_IS_AUTHORIZED("FinishLoadHole");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestFinishLoadHole][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou finalizar carregamento do hole do jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5900401));

		if (r->requestFinishLoadHole(_session, _packet)) {

			job j(channel::_enter_left_time_is_over, this, (void*)(intptr_t)r->getNumero());

			r->setState(1);
			r->setFlag(1);

			r->requestStartAfterEnter(j);

			packet p;

			sendUpdateRoomInfo(*const_cast< RoomInfoEx* >(r->getInfo()), 3);
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestFinishLoadHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestFinishCharIntro(player& _session, packet *_packet) {
	REQUEST_BEGIN("FinishCharIntro");

	try {

		CHECK_SESSION_IS_AUTHORIZED("FinishCharIntro");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestFinishCharIntro][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou finalizar Char Intro do jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5900501));

		r->requestFinishCharIntro(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestFinishCharIntro][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestFinishHoleData(player& _session, packet *_packet) {
	REQUEST_BEGIN("FinishHoleData");

	try {

		CHECK_SESSION_IS_AUTHORIZED("FinishHoleData");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestFinishHoleData][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou finalizar dados do hole, no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5902101));

		r->requestFinishHoleData(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestFinishHoleData][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestInitShotSended(player& _session, packet *_packet) {
	REQUEST_BEGIN("InitShotSended");

	try {

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestInitShotSended][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] o server enviou o pacote de InitShot para o cliente, mas a sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "] nao existe mais. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5905001));

		r->requestInitShotSended(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestInitShotSended][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestInitShot(player& _session, packet *_packet) {
	REQUEST_BEGIN("InitShot");

	try {

		CHECK_SESSION_IS_AUTHORIZED("InitShot");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestInitShot][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou inicializar shot no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5901501));

		r->requestInitShot(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestInitShot][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

};

void channel::requestSyncShot(player& _session, packet *_packet) {
	REQUEST_BEGIN("SyncShot");

	try {

		CHECK_SESSION_IS_AUTHORIZED("SyncShot");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestSyncShot][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou sincronizar tacada no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5901601));

		r->requestSyncShot(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestSyncShot][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestInitShotArrowSeq(player& _session, packet *_packet) {
	REQUEST_BEGIN("InitShotArrowSeq");

	try {

		CHECK_SESSION_IS_AUTHORIZED("InitShotArrowSeq");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestInitShotArrowSeq][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou inicializar a sequencia de setas no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5901701));

		r->requestInitShotArrowSeq(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestInitShotArrowSeq][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestShotEndData(player& _session, packet *_packet) {
	REQUEST_BEGIN("ShotEndData");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ShotEndData");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestShotEndData][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou finalizar local da tacada no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5901801));

		r->requestShotEndData(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestShotEndData][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestFinishShot(player& _session, packet *_packet) {
	REQUEST_BEGIN("FinishShot");

	try {

		CHECK_SESSION_IS_AUTHORIZED("FinishShot");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestFinishShot][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou finalizar tacada no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5901901));

		auto rfs = r->requestFinishShot(_session, _packet);

		if (rfs.ret > 0) {

			if (rfs.ret == 2  && rfs.p != nullptr)
				leaveRoom(*rfs.p, 2);

		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestFinishShot][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestChangeMira(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChangMira");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ChangMira");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestChangeMira][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou trocar a mira no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5900601));

		r->requestChangeMira(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestChangeMira][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestChangeStateBarSpace(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChangeStateBarSpace");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ChangeStateBarSpace");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestChangeStateBarSpace][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou trocar state da barra de espaco no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], nas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5900701));

		r->requestChangeStateBarSpace(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestChangeStateBarSpace][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestActivePowerShot(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActivePowerShot");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActivePowerShot");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActivePowerShot][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativat power shot no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5900801));

		r->requestActivePowerShot(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActivePowerShot][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestChangeClub(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChangeClub");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ChangeClub");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestChangeClub][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou trocar taco no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5900901));

		r->requestChangeClub(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestChangeClub][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestUseActiveItem(player& _session, packet *_packet) {
	REQUEST_BEGIN("UseActiveItem");

	try {

		CHECK_SESSION_IS_AUTHORIZED("UseActiveItem");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestUseActiveItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou usar active item no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5901001));

		r->requestUseActiveItem(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestUseActiveItem][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestChangeStateTypeing(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChangeStateTypeing");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ChangeStateTypeing");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestChangeStateTypeing][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou mudar estado de escrevendo icon no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5901101));

		r->requestChangeStateTypeing(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestChangeStateTypeing][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestMoveBall(player& _session, packet *_packet) {
	REQUEST_BEGIN("MoveBall");

	try {

		CHECK_SESSION_IS_AUTHORIZED("MoveBall");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestMoveBall][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou recolocar a bola no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5901201));

		r->requestMoveBall(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestMoveBall][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestChangeStateChatBlock(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChangeStateChatBlock");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ChangeStateChatBlock");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestChangeStateChatBlock][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou mudar estado so chat block no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5901301));

		r->requestChangeStateChatBlock(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestChangeStateChatBlock][ErrorSysttem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestActiveBooster(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActiveBooster");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActiveBooster");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActiveBooster][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativar time booster no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5901401));

		r->requestActiveBooster(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActiveBooster][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestActiveReplay(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActiveReplay");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActiveReplay");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActiveReplay][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativar Replay no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6001001));

		r->requestActiveReplay(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActiveReplay][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestActiveCutin(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActiveCutin");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActiveCutin");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActiveCutin][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativar cutin no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5901801));

		r->requestActiveCutin(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActiveCutin][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestActiveAutoCommand(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActiveAutoCommand");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActiveAutoCommand");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActiveAutoCommand][Error] player[UID=" + std::to_string(_session.m_pi.uid) +"] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativar Auto Command no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x550001));

		r->requestActiveAutoCommand(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActiveAutoCommand][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestActiveAssistGreen(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActiveAssistGreen");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActiveAssistGreen");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActiveAssistGreen][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativar Assist Green no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5901901));

		r->requestActiveAssistGreen(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActiveAssistGreen][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

	}
};

void channel::requestLoadGamePercent(player& _session, packet *_packet) {
	REQUEST_BEGIN("LoadGamePercent");

	try {

		CHECK_SESSION_IS_AUTHORIZED("LoadGamePercent");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestLoadGamePercent][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou mandar a porcentagem do jogo carregado na sala[NUMEROR=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x551001));

		r->requestLoadGamePercent(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestLoadGamePercent][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestMarkerOnCourse(player& _session, packet *_packet) {
	REQUEST_BEGIN("MarkerOnCourse");

	try {

		CHECK_SESSION_IS_AUTHORIZED("MarkerOnCourse");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestMarkerOnCourse][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou marcar no course no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x552001));

		r->requestMarkerOnCourse(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestMarkerOnCourse][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestStartTurnTime(player& _session, packet *_packet) {
	REQUEST_BEGIN("StartTurnTime");

	try {

		CHECK_SESSION_IS_AUTHORIZED("StartTurnTime");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestStartTurnTime][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou comecar o tempo do turno no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x553001));

		r->requestStartTurnTime(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestStartTurnTime][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestUnOrPauseGame(player& _session, packet *_packet) {
	REQUEST_BEGIN("UnOrPauseGame");

	try {

		CHECK_SESSION_IS_AUTHORIZED("UnOrPauseGame");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestUnOrPauseGame][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou pausar ou despausar o jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x554001));

		r->requestUnOrPauseGame(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestUnOrPauseGame][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestLastPlayerFinishVersus(player& _session, packet *_packet) {
	REQUEST_BEGIN("LastPlayerFinishVersus");

	try {

		CHECK_SESSION_IS_AUTHORIZED("LastPlayerFinishVersus");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestLastPlayerFinishVersus][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou finalizar o Versus na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x555001));

		if (r->requestLastPlayerFinishVersus(_session, _packet)) {

			if (r->getInfo()->tipo != RoomInfo::TIPO::PRACTICE && r->getInfo()->tipo != RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE) {
				packet p;

				packet_func::pacote047(p, std::vector< RoomInfo > { *r->getInfo() }, 3);
				packet_func::channel_broadcast(*this, p, 1);
			}
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestLastPlayerFinishVersus][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestReplyContinueVersus(player& _session, packet *_packet) {
	REQUEST_BEGIN("ReplyContinueVersus");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ReplyContinueVersus");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestReplyContinueVersus][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou responder se quer continuar o versus ou nao na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "]. mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x556001));

		if (r->requestReplyContinueVersus(_session, _packet)) {

			if (r->getInfo()->tipo != RoomInfo::TIPO::PRACTICE && r->getInfo()->tipo != RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE) {
				packet p;

				packet_func::pacote047(p, std::vector< RoomInfo > { *r->getInfo() }, 3);
				packet_func::channel_broadcast(*this, p, 1);
			}
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestReplyContinueVersus][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestTeamFinishHole(player& _session, packet *_packet) {
	REQUEST_BEGIN("TeamFinishHole");

	try {

		CHECK_SESSION_IS_AUTHORIZED("TeamFinishHole");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestTeamFinishHole][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou finalizar o hole no Match na sala[NUMERO=" + std::to_string(_session.m_pi.uid)
					+ "], mas ele nao esta em nenhum sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x562001));

		r->requestTeamFinishHole(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestTeamFinishHole][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestLeavePractice(player& _session, packet *_packet) {
	REQUEST_BEGIN("LeavePractice");

	try {

		CHECK_SESSION_IS_AUTHORIZED("LeavePractice");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestLeavePractice][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou sair do practice na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6202001));

		if (r->getInfo()->tipo != RoomInfo::TIPO::PRACTICE)
			throw exception("[channel::requestLeavePratice][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou sair do practice na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + ", TIPO=" + std::to_string(r->getInfo()->tipo)
					+ "], mas a sala nao eh um tipo de sala do practice. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0x6202002));

		r->requestLeavePractice(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestLeavePractice][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestUseTicketReport(player& _session, packet *_packet) {
	REQUEST_BEGIN("UseTicketReport");

	try {

		CHECK_SESSION_IS_AUTHORIZED("UseTicketReport");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestUseTicketReport][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou sair do Tourney com Ticket Report no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6301001));

		if (r->requestUseTicketReport(_session, _packet))
			leaveRoom(_session, 10 );

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestUseTicketReport][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestLeaveChipInPractice(player& _session, packet *_packet) {
	REQUEST_BEGIN("LeaveChipInPractice");

	try {

		CHECK_SESSION_IS_AUTHORIZED("LeaveChipInPractice");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestLeaveChipInPractice][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou sair do Chip-in Practice na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhum sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6207701));

		r->requestLeaveChipInPractice(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestLeaveChipInPractice][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestStartFirstHoleGrandZodiac(player& _session, packet *_packet) {
	REQUEST_BEGIN("StartFirstHoleGrandZodiac");

	try {

		CHECK_SESSION_IS_AUTHORIZED("StartFirstHoleGrandZodiac");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestStartFirstHoleGrandZodiac][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou comecar o primeiro hole do Grand Zodiac game na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhum sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6207801));

		r->requestStartFirstHoleGrandZodiac(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestStartFirstHoleGrandZodiac][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestReplyInitialValueGrandZodiac(player& _session, packet *_packet) {
	REQUEST_BEGIN("ReplyInitialValueGrandZodiac");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ReplyInitialValueGrandZodiac");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::ReplyInitialValueGrandZodiac][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou responder o valor inicial do Grand Zodiac game na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhum sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6207901));

		r->requestReplyInitialValueGrandZodiac(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestReplyInitialValueGrandZodiac][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestActiveRing(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActiveRing");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActiveRing");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActiveRing][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativar Anel no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhum sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6201001));

		r->requestActiveRing(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActiveRing][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void channel::requestActiveRingGround(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActiveRingGround");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActiveRingGround");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActiveRingGround][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativar Anel de Terreno no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6201101));

		r->requestActiveRingGround(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActiveRingGround][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void channel::requestActiveRingPawsRainbowJP(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActiveRingPawsRainbowJP");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActiveRingPawsRainbowJP");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActiveRingPawsRainbowJP][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativar Anel de Patinha Arco-iris JP no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6201201));

		r->requestActiveRingPawsRainbowJP(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActiveRingPawsRainbowJP][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void channel::requestActiveRingPawsRingSetJP(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActiveRingPawsRingSetJP");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActiveRingPawsRingSetJP");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActiveRingPawsRingSetJP][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativar Anel de Patinha de Conjunto de Aneis [JP] no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6201301));

		r->requestActiveRingPawsRingSetJP(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActiveRingPawsRingSetJP][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void channel::requestActiveRingPowerGagueJP(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActiveRingPowerGagueJP");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActiveRingPowerGagueJP");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActiveRingPowerGagueJP][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativar Anel Barra de PS [JP] no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhum sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6201401));

		r->requestActiveRingPowerGagueJP(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActiveRingPowerGagueJP][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void channel::requestActiveRingMiracleSignJP(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActiveRingMiracleSignJP");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActiveRingMiracleSignJP");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActiveRingMiracleSignJP][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativar Anel Olho Magico [JP] no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6201501));

		r->requestActiveRingMiracleSignJP(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActiveRingMiracleSignJP][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void channel::requestActiveWing(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActiveWing");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActiveWing");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActiveWing][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativar Asa no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6201601));

		r->requestActiveWing(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActiveWing][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void channel::requestActivePaws(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActivePaws");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActivePaws");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActivePaws][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativar Patinha no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6201701));

		r->requestActivePaws(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActivePaws][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void channel::requestActiveGlove(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActiveGlove");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActiveGlove");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActiveGlove][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativar Luva 1m no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6201801));

		r->requestActiveGlove(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActiveGlove][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void channel::requestActiveEarcuff(player& _session, packet *_packet) {
	REQUEST_BEGIN("ActiveEarcuff");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ActiveEarcuff");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestActiveEarcuff][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou ativar Earcuff no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6201901));

		r->requestActiveEarcuff(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestActiveEarcuff][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

uint32_t channel::findReconnectOID(uint32_t _uid) {

	return m_rm.findReconnectOID(_uid);
};

bool channel::execGhostCalcCmd(player& _session, std::string& _msg) {
	CHECK_SESSION_BEGIN("execGhostCalcCmd");

	bool ret = false;

	try {

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::execGhostCalcCmd][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou executar Ghost Calc Cmd " + (_session.m_pi.mi.sala_numero != -1
						? "na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "], mas ela nao existe."
						: "mas ele nao esta em nenhum sala."), STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0));

		if (!_session.m_pi.m_cap.stBit.game_master && !_session.m_pi.m_cap.stBit.gm_normal || r->getInfo()->tipo == RoomInfo::TIPO::LOUNGE)
			if (r->getInfo()->tipo == RoomInfo::TIPO::LOUNGE || r->getInfo()->tipo == RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE || r->getInfo()->tipo == RoomInfo::TIPO::PRACTICE
					|| (r->getInfo()->tipo == RoomInfo::TIPO::GRAND_PRIX
						&& sIff::getInstance().getGrandPrixAba(r->getInfo()->grand_prix.dados_typeid) == IFF::GrandPrixData::GP_ABA::ROOKIE
						&& sIff::getInstance().isGrandPrixNormal(r->getInfo()->grand_prix.dados_typeid)))
				throw exception("[channel::execGhostCalcCmd][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou executar Ghost Calc Cmd na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + ", TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo)
						+ "], mas ele nao pode executar esse comando nesse tipo de sala.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10000, 0));

		ret = r->execGhostCalcCmd(_session, _msg);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::execGhostCalcCmd][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		ret = false;
	}

	return ret;
};

short channel::criarSalaAposta(uint32_t _criador_uid, unsigned char _course,
		unsigned char _holes, const char* _pin) {

	RoomBet* r = nullptr;
	short numero = -1;

	try {

		RoomInfoEx ri = { 0 };

		ri.tipo       = RoomInfo::TIPO::STROKE;
		ri.course     = RoomInfo::eCOURSE(_course);
		ri.qntd_hole  = _holes;
		ri.max_player = 2;
		ri.modo       = 0u;
		ri.time_vs    = 30000u;
		ri.time_30s   = 0u;
		ri.artefato   = 0u;

		if (m_ci.flag.stBit.junior_bellow || m_ci.flag.stBit.only_rookie)
			ri.channel_rookie = 1;

		{
			const char* nome = "[BET] Versus Apostado";
			size_t _n = strlen(nome) < sizeof(ri.nome) ? strlen(nome) : sizeof(ri.nome) - 1;
			memset(ri.nome, 0, sizeof(ri.nome));
			memcpy(ri.nome, nome, _n);
		}

		{
			size_t _n = strlen(_pin) < sizeof(ri.senha) ? strlen(_pin) : sizeof(ri.senha) - 1;
			memset(ri.senha, 0, sizeof(ri.senha));
			memcpy(ri.senha, _pin, _n);
			ri.senha_flag = 0;
		}

		r = m_rm.makeRoomBet(m_ci.id, ri);

		if (r == nullptr)
			throw exception("[channel::criarSalaAposta][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou criar a sala da aposta do player[UID=" + std::to_string(_criador_uid)
					+ "], mas deu erro na criacao. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10002, 0));

		r->setBetCrazyClub(true);

		numero = r->getNumero();

		_smp::message_pool::getInstance().push(new message("[CRAZY][channel::criarSalaAposta][Log] Channel[ID="
				+ std::to_string((unsigned short)m_ci.id) + "] sala[NUMERO=" + std::to_string(numero)
				+ "] criada para a aposta do player[UID=" + std::to_string(_criador_uid) + "] com PIN.",
				CL_FILE_LOG_AND_CONSOLE));

		sendUpdateRoomInfo(*const_cast<RoomInfoEx*>(r->getInfo()), 1);

		m_rm.unlockRoom(r);

	}catch (exception& e) {

		if (r != nullptr)
			m_rm.unlockRoom(r);

		numero = -1;

		_smp::message_pool::getInstance().push(new message("[channel::criarSalaAposta][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	return numero;
};

bool channel::salaApostaViva(short _numero) {

	bool viva = false;

	try {

		BEGIN_FIND_ROOM(_numero);

		viva = (r != nullptr && r->isBetCrazyClub());

		END_FIND_ROOM;

	}catch (exception& e) {

		UNREFERENCED_PARAMETER(e);

		viva = false;
	}

	return viva;
};

bool channel::execBetCmd(player& _session, std::string& _msg) {
	CHECK_SESSION_BEGIN("execBetCmd");

	bool ret = false;

	try {

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::execBetCmd][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou executar o #BET " + (_session.m_pi.mi.sala_numero != -1
						? "na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "], mas ela nao existe."
						: "mas ele nao esta em nenhum sala."), STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0));

		if (r->getInfo()->tipo == RoomInfo::TIPO::LOUNGE)
			throw exception("[channel::execBetCmd][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou executar o #BET na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas e um Lounge.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10001, 0));

		std::string arg;

		for (auto c : _msg)
			if (!isspace((unsigned char)c))
				arg += (char)tolower((unsigned char)c);

		bool uso_errado = false;

		if (arg.compare("on") == 0 || arg.compare("1") == 0)
			r->setBetCrazyClub(true);
		else if (arg.compare("off") == 0 || arg.compare("0") == 0)
			r->setBetCrazyClub(false);
		else if (!arg.empty())
			uso_errado = true;

		if (uso_errado)
			ssv::sv->sendSmartCalculatorReplyToPlayer(_session.m_pi.uid, "#BET", "Use: #BET on  /  #BET off");
		else

			ssv::sv->sendSmartCalculatorReplyToPlayer(_session.m_pi.uid, "#BET",
					std::string("Sala[") + std::to_string(_session.m_pi.mi.sala_numero) + "] BET_CRAZY_CLUB = "
					+ (r->isBetCrazyClub() ? "LIGADO (vale do proximo hole em diante)" : "DESLIGADO"));

		ret = true;

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::execBetCmd][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		ret = false;
	}

	return ret;
};

void channel::requestPlayScratchCard(player& _session, packet* _packet)
{
	REQUEST_BEGIN("requestPlayScratchCard");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("requestPlayScratchCard");

		if (_session.m_pi.block_flag.m_flag.stBit.scratchy)
			throw exception("[channel::requestPlayScratchCard][Error] player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] tentou jogar no Scratch Card, mas ele nao pode. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0x790001));

		if (!sScratchCardSystem::getInstance().isLoad())
			sScratchCardSystem::getInstance().load();

		auto coupon = sScratchCardSystem::getInstance().hasCoupon(_session);

		if ((coupon == nullptr || coupon->STDA_C_ITEM_QNTD < 1))
			throw exception("[channel::requestPlayScratchCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Scratch Card, ele nao tem Coupon. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0x5900102));

		auto balls = sScratchCardSystem::getInstance().Play(_session);

		if (balls.empty())
			throw exception("[channel::requestPlayScratchCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Scratch Card Normal, mas nao conseguiu sortear os items. Bug",
				STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0x5900103));

		std::vector< stItem > v_item;
		stItem item{ 0 };
		BuyItem bi{ 0 };

		SysAchievement sys_achieve;

		v_item.reserve(balls.size() + 1 );

		for (auto& el : balls) {

			bi.clear();
			item.clear();

			bi.id = -1;
			bi._typeid = el.ctx_psi._typeid;
			bi.qntd = el.ctx_psi.qntd;

			item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1);

			if (item._typeid == 0)
				throw exception("[channel::requestPlayScratchCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Scratch Card Normal, mas nao conseguiu inicializar o Item[TYPEID="
					+ std::to_string(bi._typeid) + "]. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4, 0x5900104));

			auto it = std::find_if(v_item.begin(), v_item.end(), [&](auto& el2) {
				return el2._typeid == item._typeid;
				});

			if (it != v_item.end()) {
				it->qntd += item.qntd;
				it->STDA_C_ITEM_QNTD = (short)it->qntd;
			}
			else
				it = v_item.insert(v_item.end(), item);

#if defined(_WIN32)
			el.item = it._Ptr;
#elif defined(__linux__)
			el.item = &(*it);
#endif
		}

		std::string ids = "";

		for (auto i = 0u; i < v_item.size(); ++i)
			ids += ((i == 0) ? std::string("") : std::string(", ")) + "TYPEID=" + std::to_string(v_item[i]._typeid) + ", ID=" + std::to_string(v_item[i].id) + ", QNTD=" + std::to_string(v_item[i].STDA_C_ITEM_QNTD);

		auto rai = item_manager::addItem(v_item, _session, 0, 0);

		if (rai.fails.size() > 0 && rai.type != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH)
			throw exception("[channel::requestPlayScratchCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Scratch Card, mas nao conseguiu adicionar o(s) Item(ns){"
				+ ids + "}", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 6, 0x5900106));

		if (coupon != nullptr) {

			WarehouseItemEx card_scratch{ 0 };

			auto it = _session.m_pi.findWarehouseItemItByTypeid(coupon->_typeid);
			item.clear();
			auto qtnd = it->second.c[0];
			item.type = 2;
			item.id = coupon->id;
			item._typeid = coupon->_typeid;
			item.qntd = 1;
			item.stat.qntd_dep = qtnd - 1;
			item.STDA_C_ITEM_QNTD = (short)(item.qntd * 1);

			if (item_manager::removeItem(item, _session) <= 0)
				throw exception("[channel::requestPlayScratchCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Scratch Card Normal, mas nao conseguiu deletar o Coupon[TYPEID="
					+ std::to_string(coupon->_typeid) + ", ID=" + std::to_string(coupon->id) + "]. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 5, 0x5900105));
		}
		else
		{

			throw exception("[channel::requestPlayScratchCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Scratch Card Normal, mas nao conseguiu inicializar o Item[TYPEID="
				+ std::to_string(bi._typeid) + "]. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4, 0x5900104));

		}
		p.init_plain((unsigned short)0xD5);
		p.addUint32(coupon->id);
		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addUint32(el.stat.qntd_ant);
			p.addUint32(el.stat.qntd_dep);
			p.addUint32((el.STDA_C_ITEM_TIME > 0 ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD));
			p.addZeroByte(25);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0xDD);
		p.addUint32(0);
		p.addUint32((uint32_t)balls.size());
		for (auto& el : balls) {
			p.addUint32(0);
			p.addUint32(el.ctx_psi._typeid);
			p.addInt32((el.item != 0) ? ((stItem*)el.item)->id : 0);
			p.addUint32(el.qntd);
			p.addUint32(el.ctx_psi.tipo);
		}
		packet_func::session_send(p, &_session, 1);
	}
	catch (exception) {

		_smp::message_pool::getInstance().push(new message("[channel::requestPlayScratchCard][ErrorSystem] ", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x00DD);

		p.addUint32(0x5900100);

		packet_func::session_send(p, &_session, 1);
	}

};

void channel::requestEnterGameAfterStarted(player& _session, packet *_packet) {
	REQUEST_BEGIN("EnterGameAfterStarted");

	packet p;

	try {

		unsigned char option = _packet->readUint8();

		CHECK_SESSION_IS_AUTHORIZED("EnterGameAfterStarted");

		if (option == 0 || option == 1) {

			short numero = _packet->readUint16();

			BEGIN_FIND_ROOM(numero);

			if (r == nullptr)
				throw exception("[channel::requestEnterGameAfterStarted][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[NUMERO=" + std::to_string(numero) + "] ja em jogo, mas ela nao existe. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2700, 1));

			if (r->getInfo()->tipo != RoomInfo::TOURNEY && r->getInfo()->tipo != RoomInfo::STROKE
					&& !r->isReconnecting(_session.m_pi.uid))
				throw exception("[channel::requestEnterGameAfterStarted][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou entrar na sala[TIPO=" + std::to_string((unsigned short)r->getInfo()->tipo) + ", NUMERO=" + std::to_string(r->getNumero())
					+ "] ja em jogo, mas o tipo da sala nao eh Tourney. Hacker.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 15, 0x770001));

			if (r->isLocked() && !r->isReconnecting(_session.m_pi.uid))
				throw exception("[channel::requestEnterGameAfterStarted][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[NUMERO=" + std::to_string(numero) + "] ja em jogo, mas a sala eh privada. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2710, 1));

			if (!r->isGaming())
				throw exception("[channel::requestEnterGameAfterStarted][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[NUMERO=" + std::to_string(numero) + "] ja em jogo, mas a sala nao esta em jogo ainda. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2701, 1));

			if (r->isFull())
				throw exception("[channel::requestEnterGameAfterStarted][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[NUMERO=" + std::to_string(numero) + "] ja em jogo, mas a sala ja esta no seu limite de jogadores.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2702, 1));

			if (option == 0)
				r->requestSendTimeGame(_session);
			else if (option == 1) {

				try {

					deleteInviteTimeResquestByInvited(_session);

					if (r->requestEnterGameAfterStarted(_session)) {

						sendUpdateRoomInfo(*const_cast<RoomInfoEx*>(r->getInfo()), 3);

						updatePlayerInfo(_session);

						if (r->getInfo()->tipo != RoomInfo::TIPO::PRACTICE && r->getInfo()->tipo != RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE)
							sendUpdatePlayerInfo(_session, 3);

					}

				}catch (exception& e) {
					UNREFERENCED_PARAMETER(e);

					throw;
				}

			}

			END_FIND_ROOM;

		}else if (option == 2) {

			EnterAfterStartInfo easi{ 0 };

			_packet->readBuffer(&easi, sizeof(EnterAfterStartInfo));

			BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

			if (r == nullptr)
				throw exception("[channel::requestEnterGameAfterStarted][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "] ja em jogo, mas ela nao existe. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2700, 1));

			if (!r->isGaming())
				throw exception("[channel::requestEnterGameAfterStarted][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "] ja em jogo, mas a sala nao esta em jogo ainda. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2701, 1));

			r->requestUpdateEnterAfterStartedInfo(_session, easi);

			END_FIND_ROOM;
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestEnterGameAfterStarted][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x113);

		p.addUint8(6);

		p.addUint8((unsigned char)((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 1 ));

		packet_func::session_send(p, &_session, 1);
	}
}

void channel::requestFinishGame(player& _session, packet *_packet) {
	REQUEST_BEGIN("FinishGame");

	try {

		CHECK_SESSION_IS_AUTHORIZED("FinishGame");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestFinishGame][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou finalizar o jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhum sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5902201));

		if (r->requestFinishGame(_session, _packet)) {

			if (r->getInfo()->tipo != RoomInfo::TIPO::PRACTICE && r->getInfo()->tipo != RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE) {
				packet p;

				packet_func::pacote047(p, std::vector< RoomInfo > { *r->getInfo() }, 3);
				packet_func::channel_broadcast(*this, p, 1);
			}
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestFinishGame][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestChangeWindNextHoleRepeat(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChangeWindNextHoleRepeat");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ChangeWindNextHoleRepeat");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestChangeWindNextHoleRepeat][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou trocar vento dos proximos holes repeat no jogo na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5902301));

		r->requestChangeWindNextHoleRepeat(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestChangeWindNextHoleRepeat][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestEnterRoomGrandPrix(player& _session, packet *_packet) {
	REQUEST_BEGIN("EnterRoomGrandPrix");

	packet p;

	try {

		uint32_t _typeid_gp = _packet->readUint32();

		CHECK_SESSION_IS_AUTHORIZED("EnterRoomGrandPrix");

		uFlag flag = _session.m_pi.block_flag.m_flag.ullFlag;

		if (flag.stBit.all_game)
			throw exception("[channel::requestEnterRoomGrandPrix][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou criar ou entrar sala Grand Prix[TYPEID=" + std::to_string(_typeid_gp)
					+ "], mas ele nao pode criar ou entrar nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x6700003, 0x6700003));

		if (flag.stBit.grand_prix)
			throw exception("[channel::requestEnterRoomGrandPrix][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou criar ou entrar sala Grand Prix[TYPEID=" + std::to_string(_typeid_gp) + ", TIPO=" + std::to_string(RoomInfo::GRAND_PRIX)
					+ "], mas ele nao pode criar Grand Prix ou entrar(jogar). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x6700004, 0x6700004));

		auto gp = sIff::getInstance().findGrandPrixData(_typeid_gp);

		if (gp == nullptr || !gp->active)
			throw exception("[channel::requestEnterRoomGrandPrix][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] Canal[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou entrar na sala Grand Prix[TYPEID=" + std::to_string(_typeid_gp)
					+ "] mas nao existe esse grand prix no IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x6700001, 0x6700001));

		SYSTEMTIME local{ 0 };

		GetLocalTime(&local);

		local.wDay = local.wDayOfWeek = local.wMonth = local.wYear = 0u;

		if (gp->open.wHour >= 23 && gp->start.wHour <= 1)
			gp->start.wDay = 1u;

		if ((!isEmpty(gp->open) && getHourDiff(local, gp->open) < 0ll) || (!isEmpty(gp->start) && getHourDiff(local, gp->start) > 0ll))
			throw exception("[channel::requestEnterRoomGrandPrix][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] Canal[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou entrar na sala Grand Prix[TYPEID=" + std::to_string(gp->_typeid)
					+ ", OPEN={HORA=" + std::to_string(gp->open.wHour) + ", MIN=" + std::to_string(gp->open.wMinute)
					+ "}, START={HORA=" + std::to_string(gp->start.wHour) + ", MIN=" + std::to_string(gp->start.wMinute)
					+ "}] mas ainda nao esta na hora[HORA=" + std::to_string(local.wHour) + ", MIN=" + std::to_string(local.wMinute)
					+ "] em que pode entrar na sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x670000C, 0x670000C));

		if (_session.m_pi.level < gp->level_min || (gp->level_max > 0u && _session.m_pi.level > gp->level_max))
			throw exception("[channel::requestEnterRoomGrandPrix][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + ", LEVEL=" + std::to_string(_session.m_pi.level)
					+ "] Canal[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou entrar na sala Grand Prix[TYPEID=" + std::to_string(gp->_typeid)
					+ ", LVL_MIN=" + std::to_string(gp->level_min) + ", LVL_MAX=" + std::to_string(gp->level_max)
					+ "] mas ele nao tem o level necessario para entrar na sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x6700006, 0x6700006));

		auto gp_condition = sIff::getInstance().findGrandPrixConditionEquip(gp->typeid_link);

		if (gp_condition != nullptr && !_session.m_pi.checkEquipedItem(gp_condition->item_typeid))
			throw exception("[channel::requestEnterRoomGrandPrix][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] Canal[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou entrar na sala Grand Prix[TYPEID=" + std::to_string(gp->_typeid)
					+ "] mas ele nao esta equipado com o item[TYPEID=" + std::to_string(gp_condition->item_typeid) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x6700007, 0x6700007));

		if (_session.m_pi.ui.getMediaScore() < gp->condition[0] || (gp->condition[1] > 0u && _session.m_pi.ui.getMediaScore() > gp->condition[1]))
			throw exception("[channel::requestEnterRoomGrandPrix][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + ", AVG_SCORE=" + std::to_string(_session.m_pi.ui.getMediaScore())
					+ "] Canal[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou entrar na sala Grand Prix[TYPEID=" + std::to_string(gp->_typeid)
					+ ", AVG_MIN=" + std::to_string(gp->condition[0]) + ", AVG_MAX=" + std::to_string(gp->condition[1])
					+ "] mas ele nao tem o Avg. Score necessario para entrar na sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x6700008, 0x6700008));

		if (gp->ticket.qntd > 0u && gp->ticket._typeid) {

			auto pWi = _session.m_pi.findWarehouseItemByTypeid(gp->ticket._typeid);

			if (pWi == nullptr || (unsigned short)pWi->STDA_C_ITEM_QNTD < gp->ticket.qntd)
				throw exception("[channel::requestEnterRoomGrandPrix][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
						+ ", TICKET_QNTD=" + std::to_string((pWi == nullptr ? 0 : pWi->STDA_C_ITEM_QNTD)) + "] Canal[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala Grand Prix[TYPEID=" + std::to_string(gp->_typeid) + "] mas ele nao tem ticket[TYPEID=" + std::to_string(gp->ticket._typeid)
						+ ", QNTD=" + std::to_string(gp->ticket.qntd) + "] suficiente. Hacker ou Bug ", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x6700009, 0x6700009));

		}

		if (gp->lock_yn > 0u && gp->clear_gp_typeid != 0u && _session.m_pi.findGrandPrixClear(gp->clear_gp_typeid) == nullptr)
			throw exception("[channel::requestEnterRoomGrandPrix][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] Canal[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou entrar na sala Grand Prix[TYPEID=" + std::to_string(gp->_typeid)
					+ "] mas nao concluiu o Grand Prix[TYPEID=" + std::to_string(gp->clear_gp_typeid) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x670000A, 0x670000A));

		if (gp->type > 0u && !sgs::gs::getInstance().getInfo().rate.checkBitGrandPrixEvent(gp->type))
			throw exception("[channel::requestEnterRoomGrandPrix][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] Canal[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou entrar na sala Grand Prix[TYPEID=" + std::to_string(gp->_typeid)
					+ ", TYPE=" + std::to_string(gp->type) + "] mas esse type nao esta ativo no server[GP_TYPE=" + std::to_string(sgs::gs::getInstance().getInfo().rate.grand_prix_event)
					+ "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x670000B, 0x670000B));

		room *r = nullptr;

		if (sIff::getInstance().isGrandPrixNormal(gp->_typeid) && sIff::getInstance().getGrandPrixAba(gp->_typeid) == IFF::GrandPrixData::GP_ABA::ROOKIE
				|| (r = m_rm.findRoomGrandPrix(gp->_typeid)) == nullptr) {

			RoomInfoEx ri{ 0 };

			ri.time_vs = 0;
			ri.time_30s = 0;
			ri.max_player = 30;
			ri.tipo = RoomInfo::TIPO::GRAND_PRIX;
			ri.qntd_hole = gp->course_info.qntd_hole;
			ri.course = RoomInfo::eCOURSE(gp->course_info.course);
			ri.modo = (unsigned char)gp->course_info.modo;

			ri.natural.stBit.natural = gp->flag.natural;
			ri.natural.stBit.short_game = gp->flag.short_game;

			ri.artefato = gp->rule;

			ri.grand_prix.active = 1u;
			ri.grand_prix.dados_typeid = gp->_typeid;
			ri.grand_prix.rank_typeid = gp->typeid_link;
			ri.grand_prix.tempo = gp->time_hole * 1000 ;

#if defined(_WIN32)
			memcpy_s(ri.nome, sizeof(ri.nome), gp->name, sizeof(ri.nome));
#elif defined(__linux__)
			memcpy(ri.nome, gp->name, sizeof(ri.nome));
#endif

			RoomGrandPrix *r = nullptr;

			try {

				deleteInviteTimeResquestByInvited(_session);

				r = m_rm.makeRoomGrandPrix(m_ci.id, ri, &_session, *gp, 1 );

				if (r == nullptr)
					throw exception("[channel::requestEnterRoomGrandPrix][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] Canal[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou entrar na sala Grand Prix[TYPEID=" + std::to_string(_typeid_gp)
						+ "] mas nao conseguiu criar a sala. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x6700002, 0x6700002));

#ifdef _DEBUG
				_smp::message_pool::getInstance().push(new message("New Room Grand Prix Maked.", CL_FILE_LOG_AND_CONSOLE));
#endif

				updatePlayerInfo(_session);

				r->sendUpdate();

				r->sendMake(_session);

				r->sendCharacter(_session, 0);

				sendUpdateRoomInfo(*const_cast<RoomInfoEx*>(r->getInfo()), 1);

				if (r->getInfo()->tipo != RoomInfo::TIPO::PRACTICE && r->getInfo()->tipo != RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE)
					sendUpdatePlayerInfo(_session, 3);

				if (r != nullptr)
					m_rm.unlockRoom(r);

			}catch (exception& e) {
				UNREFERENCED_PARAMETER(e);

				if (r != nullptr)
					m_rm.unlockRoom(r);

				throw;
			}

		}else {

			try {

				if (!r->isFull()) {

					deleteInviteTimeResquestByInvited(_session);

					r->enter(_session);

				}else
					throw exception("[channel::requestEnterRoomGrandPrix][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou entrar na sala[NUMERO=" + std::to_string(r->getNumero()) + "], mas a sala esta cheia.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0x6700005));

				updatePlayerInfo(_session);

				r->sendUpdate();

				r->sendMake(_session);

				r->sendCharacter(_session, 0);

				r->sendCharacter(_session, 1);

				sendUpdateRoomInfo(*const_cast<RoomInfoEx*>(r->getInfo()), 3);

				if (r->getInfo()->tipo != RoomInfo::TIPO::PRACTICE && r->getInfo()->tipo != RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE)
					sendUpdatePlayerInfo(_session, 3);

				if (r != nullptr)
					m_rm.unlockRoom(r);

			}catch (exception& e) {
				UNREFERENCED_PARAMETER(e);

				if (r != nullptr)
					m_rm.unlockRoom(r);

				throw;
			}
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestEnterRoomGrandPrix][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x253);

		p.addUint32(STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x6700000);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestExitRoomGrandPrix(player& _session, packet *_packet) {
	REQUEST_BEGIN("ExitRoomGrandPrix");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("ExitRoomGrandPrix");

		unsigned char opt = _packet->readUint8();

		short value = _packet->readInt16();

		char key[16];

		_packet->readBuffer(key, sizeof(key));

		leaveRoomGrandPrix(_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestExitRoomGrandPrix][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestPlayerReportChatGame(player& _session, packet *_packet) {
	REQUEST_BEGIN("PlayerReportChatGame");

	try {

		CHECK_SESSION_IS_AUTHORIZED("requestPlayerReportChatGame");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestPlayerReportChatGame][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] nao esta em nenhum sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "] para reportar o char da sala. Hacker ou Bug.",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x580200, 0));

		r->requestPlayerReportChatGame(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestPlayerReportChatGame][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::ROOM)
			throw;
	}
};

void channel::requestExecCCGVisible(player& _session, packet *_packet) {
	REQUEST_BEGIN("ExecCCGVisible");

	try {

		unsigned short visible = _packet->readUint16();

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr && _session.m_pi.mi.sala_numero != -1)
			throw exception("[channel::requestExecCCGVisible][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou executar o comando visible, mas nao encontrou a sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "] que esta nos dados dele. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0x5700100));

		_session.m_gi.visible = _session.m_pi.mi.state_flag.stFlagBit.visible = (visible & 1);

		updatePlayerInfo(_session);

		if (r != nullptr)
			r->updatePlayerInfo(_session);

		_smp::message_pool::getInstance().push(new message("[channel::requestExecCCGVisible][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] trocou VISIBLE STATUS[STATE="
				+ (visible & 1 ? std::string("ON") : std::string("OFF")) + "]", CL_FILE_LOG_AND_CONSOLE));

		sendUpdatePlayerInfo(_session, 3);

		if (r != nullptr)
			r->sendCharacter(_session, 3);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestExecCCGVisible][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		throw;
	}
}

void channel::requestExecCCGChangeWindVersus(player& _session, packet *_packet) {
	REQUEST_BEGIN("ExecCCGChangeWindVersus");

	try {

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestExecCCGChangeWindVersus][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] tentou executar o comando de troca de vento do versus, mas ele nao esta em nenhuma sala[NUMERO="
					+ std::to_string(_session.m_pi.mi.sala_numero) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5700100));

		r->requestExecCCGChangeWindVersus(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestExecCCGChangeWindVersus][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		throw;
	}
};

void channel::requestExecCCGChangeWeather(player& _session, packet *_packet) {
	REQUEST_BEGIN("ExecCCGChangeWeather");

	try {

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestExecCCGChangeWeather][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou executar o comando de troca de tempo(weather) da sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0x5700100));

		r->requestExecCCGChangeWeather(_session, _packet);

		END_FIND_ROOM;

	}catch (exception&e ) {

		_smp::message_pool::getInstance().push(new message("[channel::requestExecCCGChangeWeather][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		throw;
	}
};

void channel::requestExecCCGGoldenBell(player& _session, packet *_packet) {
	REQUEST_BEGIN("ExecCCGGoldenBell");

	try {

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestExecCCGGoldenBell][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou executar o comando goldenbell na sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "], mas ele nao esta em nenhum sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 7, 0x5700100));

		r->requestExecCCGGoldenBell(_session, _packet);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestExecCCGGoldenBell][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		throw;
	}
};

void channel::requestExecCCGIdentity(player& _session, packet *_packet) {
	REQUEST_BEGIN("ExecCCGIdentity");

	try {

		uCapability cap = _packet->readUint32();
		std::string nick = _packet->readString();

		CHECK_SESSION_IS_AUTHORIZED("ExecCCGIdentity");

		if (nick.empty())
			throw exception("[channel::requestExecCCGIdentity][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou executar o comando identity, mas o nick is empty. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 11, 0x5700100));

		if (nick.compare(_session.m_pi.nickname) != 0)
			throw exception("[channel::requestExecCCGIdentity][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou executar o comando identity, mas o nick[NICK="
					+ nick + "] nao bate com o do player[NICK=" + std::string(_session.m_pi.nickname) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 12, 0x5700100));

		if (!_session.m_pi.m_cap.stBit.gm_normal && !_session.m_pi.m_cap.stBit.game_master)
			throw exception("[channel::requestExecCCGIdentity][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou executar o comando identity, mas ele nao eh gm e nunca foi. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 13, 0x5700100));

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr && _session.m_pi.mi.sala_numero != -1)
			throw exception("[channel::requestExecCCGIdentity][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou executar o comando identity, mas nao encontrou a sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "] que esta nos dados dele. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 14, 0x5700100));

		packet p;

		if (cap.ulCapability == ~0u) {

			if (_session.m_pi.m_cap.stBit.gm_normal) {

				_session.m_pi.m_cap.stBit.game_master = 1u;
				_session.m_pi.m_cap.stBit.title_gm = 1u;

				_session.m_pi.m_cap.stBit.gm_normal = 0u;

				updatePlayerInfo(_session);

				if (r != nullptr)
					r->updatePlayerInfo(_session);

				_smp::message_pool::getInstance().push(new message("[channel::requestExecCCGIdentity][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] trocou a capacidade dele, para GM Total(Admin)", CL_FILE_LOG_AND_CONSOLE));

				p.init_plain((unsigned short)0x9A);

				p.addUint32(_session.m_pi.m_cap.ulCapability);

				packet_func::session_send(p, &_session, 1);

				sendUpdatePlayerInfo(_session, 3);

				if (r != nullptr)
					r->sendCharacter(_session, 3);
			}

		}else {

			if (cap.stBit.gm_normal) {

				_session.m_pi.m_cap.stBit.game_master = 0u;
				_session.m_pi.m_cap.stBit.title_gm = 0u;

				_session.m_pi.m_cap.stBit.gm_normal = 1u;

				updatePlayerInfo(_session);

				if (r != nullptr)
					r->updatePlayerInfo(_session);

				_smp::message_pool::getInstance().push(new message("[channel::requestExecCCGIdentity][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] trocou a capacidade dele, para GM Normal(user normal)", CL_FILE_LOG_AND_CONSOLE));

				p.init_plain((unsigned short)0x9A);

				p.addUint32(_session.m_pi.m_cap.ulCapability);

				packet_func::session_send(p, &_session, 1);

				sendUpdatePlayerInfo(_session, 3);

				if (r != nullptr)
					r->sendCharacter(_session, 3);
			}
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestExecCCGIdentity][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		throw;
	}
};

void channel::requestExecCCGKick(player& _session, packet *_packet) {
	REQUEST_BEGIN("ExecCCGKick");

	try {

		int32_t oid = _packet->readInt32();
		unsigned char force = _packet->readUint8();

		auto s = findSessionByOID(oid);

		if (s == nullptr)
			throw exception("[channel::requestExecCCGKick][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou executar o comando /kick mas nao encontrou o player[OID=" + std::to_string(oid) + "] do oid fornecido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 8, 0));

		BEGIN_FIND_ROOM(s->m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestExecCCGKick][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou chutar um player[UID=" + std::to_string(s->m_pi.uid) + "] da sala[NUMERO=" + std::to_string(s->m_pi.mi.sala_numero)
					+ "], mas sala nao existe. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0));

		_smp::message_pool::getInstance().push(new message("[channel::requestExecCCGKick][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
				+ "] kikou o player[UID=" + std::to_string(s->m_pi.uid) + ", NICKNAME=" + std::string(s->m_pi.nickname)
				+ "] FORCE[QUIT=" + std::to_string((unsigned short)force) + "]", CL_FILE_LOG_AND_CONSOLE));

		kickPlayerRoom(*s, force);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestExecCCGKick][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		throw;
	}
};

void channel::requestExecCCGDestroy(player& _session, packet *_packet) {
	REQUEST_BEGIN("ExecCCGDestroy");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ExecCCGDestroy");

		if (_session.m_pi.m_cap.stBit.game_master) {
			short number = _packet->readInt16();

			BEGIN_FIND_ROOM(number);

			if (r == nullptr)
				throw exception("[channel::requestExecCCGDestroy][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
						+ "] tentou executar o comando destroy, para destruir a sala[NUMERO=" + std::to_string(number)
						+ "], mas a sala nao existe.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 16, 0x5700100));

			auto v_sessions = r->getSessions();

			if (v_sessions.empty()) {

				RoomInfoEx ri = *r->getInfo();

				m_rm.destroyRoom(r);

				sendUpdateRoomInfo(ri, 2);

			}else {

				for (auto& el : v_sessions)
					kickPlayerRoom(*el, 0 );
			}

			_smp::message_pool::getInstance().push(new message("[channel::requestExecCCGDestroy][Log] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] destruiu a sala[NUMERO=" + std::to_string(number)
					+ "] no canal[NOME=" + std::string(m_ci.name) + "].", CL_FILE_LOG_AND_CONSOLE));

			END_FIND_ROOM;

		}else
			throw exception("[channel::requestExecCCGDestroy][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] nao tem a capacidade de um GM. hacker ou bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 17, 0x5700101));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestExecCCGDestroy][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		packet p((unsigned short)0x40);

		p.addUint8(7);

		p.addString(_session.m_pi.nickname);
		p.addString("Nao conseguiu executar o comando.");

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestChangePlayerItemMyRoom(player& _session, packet* _packet) {
	REQUEST_BEGIN("ChangePlayerItemMyRoom");

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("Packet 0x20.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

	unsigned char type = 0u;
	uint32_t item_id;
	int error = 4;

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("ChangePlayerItemMyRoom");

		type = _packet->readUint8();

		switch (type) {
		case 0:
		{
			CharacterInfo ci{ 0 }, *pCe = nullptr;

			_packet->readBuffer(&ci, sizeof(CharacterInfo));

			if (ci.id != 0 && (pCe = _session.m_pi.findCharacterById(ci.id)) != nullptr
					&& (sIff::getInstance().getItemGroupIdentify(pCe->_typeid) == iff::CHARACTER && sIff::getInstance().getItemGroupIdentify(ci._typeid) == iff::CHARACTER)) {

				_session.checkCharacterEquipedPart(ci);

				_session.checkCharacterEquipedAuxPart(ci);

				*pCe = ci;

				snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateCharacterAllPartEquiped(_session.m_pi.uid, ci), channel::SQLDBResponse, this);

			}else {

				error = (ci.id == 0) ? 1  : (pCe == nullptr ? 2  : 3 );

				_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou Atualizar os Parts do Character[ID=" + std::to_string(ci.id) + "], mas deu Error[VALUE=" + std::to_string(error)
						+ "]. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
			}

			BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

			if (r != nullptr) {
				r->updatePlayerInfo(_session);

				PlayerRoomInfoEx *pri = r->getPlayerInfo(&_session);

				if (packet_func::pacote048(p, &_session, std::vector< PlayerRoomInfoEx > { (pri == nullptr) ? PlayerRoomInfoEx() : *pri }, 0x103))
					packet_func::room_broadcast(*r, p, 0);

				packet_func::pacote04B(p, &_session, 4 );
				packet_func::room_broadcast(*r, p, 0);
			}

			END_FIND_ROOM;

			packet_func::pacote06B(p, &_session, &_session.m_pi, type, error);
			packet_func::session_send(p, &_session, 1);
			break;
		}
		case 1:
		{
			if ((item_id = _packet->readUint32()) != 0) {
				auto pCi = _session.m_pi.findCaddieById(item_id);

				if (pCi != nullptr && sIff::getInstance().getItemGroupIdentify(pCi->_typeid) == iff::CADDIE) {

					_session.m_pi.ei.cad_info = pCi;
					_session.m_pi.ue.caddie_id = item_id;

					if (_session.checkCaddieEquiped(_session.m_pi.ue))
						item_id = _session.m_pi.ue.caddie_id;

					snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateCaddieEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

				}else {

					error = (pCi == nullptr ? 2  : 3 );

					_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] tentou equipar Caddie[ID=" + std::to_string(item_id) + "], mas deu Error[VALUE=" + std::to_string(error)
							+ "]. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
				}

			}else if (_session.m_pi.ue.caddie_id > 0 && _session.m_pi.ei.char_info != nullptr) {

				_session.m_pi.ei.cad_info = nullptr;
				_session.m_pi.ue.caddie_id = 0;

				snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateCaddieEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

			}

			packet_func::pacote06B(p, &_session, &_session.m_pi, type, error);
			packet_func::session_send(p, &_session, 1);
			break;
		}
		case 2:
		{

			UserEquip ue;
			_packet->readBuffer(&ue.item_slot, sizeof(ue.item_slot));

	#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message(hex_util::BufferToHexString((unsigned char*)ue.item_slot, sizeof(ue.item_slot)), CL_ONLY_FILE_LOG));
	#endif

			try {

				std::map< uint32_t , uint32_t  > mp_same_item_count;
				std::map< uint32_t , uint32_t  >::iterator c_it;

				for (size_t i = 0; i < (sizeof(ue.item_slot) / sizeof(int)); ++i) {

					if (ue.item_slot[i] != 0) {

						if (!sIff::getInstance().ItemEquipavel(ue.item_slot[i]))
							throw exception("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou atualizar item[TYPEID="
									+ std::to_string(ue.item_slot[i]) + "] equipaveis, mas nao eh um item equipavel. Hacker ou Bug", 0);

						if (sIff::getInstance().findItem(ue.item_slot[i]) == nullptr)
							throw std::out_of_range("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou atualizar item[TYPEID="
									+ std::to_string(ue.item_slot[i]) + "] equipaveis, mas nao tem o item no iff Item do IFF_STRUCT do Server. Hacker ou Bug");

						auto pWi = _session.m_pi.findWarehouseItemByTypeid(ue.item_slot[i]);

						if (pWi == nullptr)
							throw exception("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou atualizar item[TYPEID="
									+ std::to_string(ue.item_slot[i]) + "] equipaveis, mas ele nao tem esse item. Hacker ou Bug", 2);
						else {

							if ((c_it = mp_same_item_count.find(pWi->_typeid)) != mp_same_item_count.end()) {

								if (std::find(active_item_cant_have_2_inveroty, LAST_ELEMENT_IN_ARRAY(active_item_cant_have_2_inveroty), pWi->_typeid) != LAST_ELEMENT_IN_ARRAY(active_item_cant_have_2_inveroty)) {

									throw exception("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar item[TYPEID=" + std::to_string(ue.item_slot[i])
											+ "] mas ele ja tem 1 item desse equipado, so e permitido equipar 1, nao pode equipar mais do que 1. Hacker ou Bug", 2);

								}else if (pWi->STDA_C_ITEM_QNTD < (int)(c_it->second + 1) )
									throw exception("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou atualizar item[TYPEID="
											+ std::to_string(ue.item_slot[i]) + "] equipaveis, mas ele nao tem quantidade dele. Hacker ou Bug", 2);
								else
									c_it->second++;

							}else {

								if (pWi->STDA_C_ITEM_QNTD < 1 )
									throw exception("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou atualizar item[TYPEID="
											+ std::to_string(ue.item_slot[i]) + "] equipaveis, mas ele nao tem quantidade dele. Hacker ou Bug", 2);

								mp_same_item_count.insert(std::make_pair(pWi->_typeid, 1 ));
							}

						}
					}
				}

#if defined(_WIN32)
				memcpy_s(_session.m_pi.ue.item_slot, sizeof(_session.m_pi.ue.item_slot), ue.item_slot, sizeof(_session.m_pi.ue.item_slot));
#elif defined(__linux__)
				memcpy(_session.m_pi.ue.item_slot, ue.item_slot, sizeof(_session.m_pi.ue.item_slot));
#endif

				snmdb::NormalManagerDB::getInstance().add(25, new CmdUpdateItemSlot(_session.m_pi.uid, (uint32_t*)ue.item_slot), channel::SQLDBResponse, this);

			}catch (exception& e) {

				_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemMyRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

				if (e.getCodeError() == 0)
					error = 0;
				else if (e.getCodeError() == 1)
					error = 1;
				else
					error = 10;

			}catch (std::out_of_range& e) {

				_smp::message_pool::getInstance().push(new message(std::string("[channel::requestChangePlayerItemMyRoom][ErrorSystem] ") + e.what(), CL_FILE_LOG_AND_CONSOLE));

				error = 1;

			}catch (std::exception& e) {

				_smp::message_pool::getInstance().push(new message(std::string("[channel::requestChangePlayerItemMyRoom][ErrorSystem] ") + e.what(), CL_FILE_LOG_AND_CONSOLE));

				error = 10;

			}catch (...) {

				_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemMyRoom][ErrorSystem] Unknown Error", CL_FILE_LOG_AND_CONSOLE));

				error = 10;
			}

			packet_func::pacote06B(p, &_session, &_session.m_pi, type, error);
			packet_func::session_send(p, &_session, 1);
			break;
		}
		case 3:
		{

			WarehouseItemEx *pWi = nullptr;

			if ((item_id = _packet->readUint32()) != 0 && (pWi = _session.m_pi.findWarehouseItemByTypeid(item_id)) != nullptr
					&& sIff::getInstance().getItemGroupIdentify(pWi->_typeid) == iff::BALL) {

				_session.m_pi.ei.comet = pWi;
				_session.m_pi.ue.ball_typeid = item_id;

				if (_session.checkBallEquiped(_session.m_pi.ue))
					item_id = _session.m_pi.ue.ball_typeid;

				snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateBallEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

			}else if (item_id == 0) {

				_session.m_pi.ei.comet = nullptr;
				_session.m_pi.ue.ball_typeid = 0l;

				if (_session.checkBallEquiped(_session.m_pi.ue))
					item_id = _session.m_pi.ue.ball_typeid;

				snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateBallEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

			}else {

				error = (pWi == nullptr ? 2  : 3 );

				_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou equipar Ball[TYPEID=" + std::to_string(item_id) + "], mas deu Error[VALUE=" + std::to_string(error)
						+ "]. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
			}

			if ((item_id = _packet->readUint32()) != 0 && (pWi = _session.m_pi.findWarehouseItemById(item_id)) != nullptr
					&& sIff::getInstance().getItemGroupIdentify(pWi->_typeid) == iff::CLUBSET) {

				_session.m_pi.ei.clubset = pWi;

				_session.m_pi.ei.csi = { pWi->id, pWi->_typeid, pWi->c };

				IFF::ClubSet *cs = sIff::getInstance().findClubSet(pWi->_typeid);

				if (cs != nullptr) {

					for (auto j = 0u; j < (sizeof(_session.m_pi.ei.csi.enchant_c) / sizeof(short)); ++j)
						_session.m_pi.ei.csi.enchant_c[j] = cs->slot[j] + pWi->clubset_workshop.c[j];

					_session.m_pi.ue.clubset_id = item_id;

					if (_session.checkClubSetEquiped(_session.m_pi.ue))
						item_id = _session.m_pi.ue.clubset_id;

					snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateClubsetEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

				}else
					_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou atualizar o ClubSet[TYPEID="
							+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "] equipado, mas ClubSet Not exists on IFF_STRUCT do Server. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

			}else {

				error = (item_id == 0) ? 1  : (pWi == nullptr ? 2  : 3 );

				_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou equipar ClubSet[ID=" + std::to_string(item_id) + "], mas deu Error[VALUE=" + std::to_string(error)
						+ "]. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
			}

			packet_func::pacote06B(p, &_session, &_session.m_pi, type, error);
			packet_func::session_send(p, &_session, 1);
			break;
		}
		case 4:
		{
			for (auto i = 0u; i < (sizeof(UserEquip::skin_typeid) / sizeof(UserEquip::skin_typeid[0])); ++i) {

				if ((item_id = _packet->readUint32()) != 0) {

					auto pWi = _session.m_pi.findWarehouseItemByTypeid(item_id);

					if (pWi != nullptr && sIff::getInstance().getItemGroupIdentify(pWi->_typeid) == iff::SKIN) {

						_session.m_pi.ue.skin_id[i] = pWi->id;
						_session.m_pi.ue.skin_typeid[i] = pWi->_typeid;

						snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateSkinEquiped(_session.m_pi.uid, _session.m_pi.ue), channel::SQLDBResponse, this);

					}else {

						error = (pWi == nullptr ? 2  : 3 );

						_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid)
								+ "] tentou equipar SKIN[TYPEID=" + std::to_string(item_id) + ", SLOT=" + std::to_string(i)
								+ "], mas deu Error[VALUE=" + std::to_string(error) + "]. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
					}

				}else {

					_session.m_pi.ue.skin_id[i] = 0u;
					_session.m_pi.ue.skin_typeid[i] = 0u;

					snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateSkinEquiped(_session.m_pi.uid, _session.m_pi.ue), channel::SQLDBResponse, this);
				}
			}

			if (_session.checkSkinEquiped(_session.m_pi.ue))

				snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateSkinEquiped(_session.m_pi.uid, _session.m_pi.ue), channel::SQLDBResponse, this);

			packet_func::pacote06B(p, &_session, &_session.m_pi, type, error);
			packet_func::session_send(p, &_session, 1);
			break;
		}
		case 5:
		{
			CharacterInfo *pCe = nullptr;

			if ((item_id = _packet->readUint32()) != 0 && (pCe = _session.m_pi.findCharacterById(item_id)) != nullptr
					&& sIff::getInstance().getItemGroupIdentify(pCe->_typeid) == iff::CHARACTER) {

				_session.m_pi.ei.char_info = pCe;
				_session.m_pi.ue.character_id = item_id;

				updatePlayerInfo(_session);

				PlayerCanalInfo *pci = getPlayerInfo(&_session);

				packet_func::pacote06B(p, &_session, &_session.m_pi, type, error);
				packet_func::session_send(p, &_session, 1);

				snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateCharacterEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

			}else {

				error = (item_id == 0) ? 1  : (pCe == nullptr ? 2  : 3 );

				_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou equipar o Character[ID=" + std::to_string(item_id) + "], mas deu Error[VALUE=" + std::to_string(error)
						+ "]. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
			}

			packet_func::pacote06B(p, &_session, &_session.m_pi, type, error);
			packet_func::session_send(p, &_session, 1);
			break;
		}
		case 8:
		{
			if ((item_id = _packet->readUint32()) != 0) {

				auto pMi = _session.m_pi.findMascotById(item_id);

				if (pMi != nullptr && sIff::getInstance().getItemGroupIdentify(pMi->_typeid) == iff::MASCOT) {

					_session.m_pi.ei.mascot_info = pMi;
					_session.m_pi.ue.mascot_id = item_id;

					if (_session.checkMascotEquiped(_session.m_pi.ue))
						item_id = _session.m_pi.ue.mascot_id;

					snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateMascotEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

				}else {

					error = (pMi == nullptr ? 2  : 3 );

					_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] tentou equipar Mascot[ID=" + std::to_string(item_id) + "], mas deu Error[VALUE=" + std::to_string(error)
							+ "]. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
				}

			}else if (_session.m_pi.ue.mascot_id > 0 && _session.m_pi.ei.mascot_info != nullptr) {

				_session.m_pi.ei.mascot_info = nullptr;
				_session.m_pi.ue.mascot_id = 0;

				snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateMascotEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

			}

			packet_func::pacote06B(p, &_session, &_session.m_pi, type, error);
			packet_func::session_send(p, &_session, 1);
			break;
		}
		case 9:
		{
			CharacterInfo *pCe = nullptr;

			if ((item_id = _packet->readUint32()) != 0 && (pCe = _session.m_pi.findCharacterById(item_id)) != nullptr
					&& (sIff::getInstance().getItemGroupIdentify(pCe->_typeid) == iff::CHARACTER && _session.m_pi.ei.char_info != nullptr && _session.m_pi.ei.char_info->id == pCe->id)) {

				int32_t cc[4]{ 0 };

				_packet->readBuffer(&cc, sizeof(cc));

				for (auto i = 0u; i < (sizeof(cc) / sizeof(cc[0])); i++) {

					if (cc[i] != 0) {

						auto pWi = _session.m_pi.findWarehouseItemById(cc[i]);

						if (pWi != nullptr && sIff::getInstance().getItemGroupIdentify(pWi->_typeid) == iff::SKIN)
							pCe->cut_in[i] = pWi->id;
						else {

							error = (pWi == nullptr ? 2  : 3 );

							_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid)
									+ "] tentou equipar Equipar Cutin do Character[ID=" + std::to_string(item_id) + ", CUTTIN_TYPEID=" + std::to_string(cc[i])
									+ ", SLOT=" + std::to_string(i) + "], mas deu Error[VALUE=" + std::to_string(error) + "]. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
						}

					}else
						pCe->cut_in[i] = cc[i];
				}

				_session.checkCharacterEquipedCutin(*pCe);

				snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateCharacterCutinEquiped(_session.m_pi.uid, *pCe), channel::SQLDBResponse, this);

			}else {

				error = 1;

				if (item_id == 0)
					error = 1;
				else if (pCe == nullptr)
					error = 2;
				else if (_session.m_pi.ei.char_info == nullptr)
					error = 4;
				else if (_session.m_pi.ei.char_info->id != pCe->id)
					error = 5;
				else
					error = 3;

				_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou equipar Equipar Cutin do Character[ID=" + std::to_string(item_id) + "], mas deu Error[VALUE=" + std::to_string(error)
						+ "]. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
			}

			packet_func::pacote06B(p, &_session, &_session.m_pi, type, error);
			packet_func::session_send(p, &_session, 1);
			break;
		}
		case 10:
			{
				for (auto i = 0u; i < 2u; ++i) {

					if ((item_id = _packet->readUint32()) != 0) {

						auto pMri = _session.m_pi.findMyRoomItemByTypeid(item_id);

						if (pMri != nullptr && sIff::getInstance().getItemGroupIdentify(pMri->_typeid) == iff::FURNITURE)
							_session.m_pi.ue.poster[i] = item_id;
						else {

							error = (pMri == nullptr ? 2  : 3 );

							_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid)
									+ "] tentou equipar Poster[TYPEID=" + std::to_string(item_id) + ", SLOT=" + std::to_string(i)
									+ "], mas deu Error[VALUE=" + std::to_string(error) + "]. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
						}
					}else
						_session.m_pi.ue.poster[i] = 0;
				}

				if (_session.checkPosterEquiped(_session.m_pi.ue) || error == 4 )
					snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdatePosterEquiped(_session.m_pi.uid, _session.m_pi.ue), channel::SQLDBResponse, this);

				packet_func::pacote06B(p, &_session, &_session.m_pi, type, error);
				packet_func::session_send(p, &_session, 1);
				break;
			}
		}

		updatePlayerInfo(_session);

	}catch (exception& e) {

		packet_func::pacote06B(p, &_session, &_session.m_pi, type, 1);
		packet_func::session_send(p, &_session, 1);

		_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemMyRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestOpenTicketReportScroll(player& _session, packet *_packet) {
	REQUEST_BEGIN("OpenTicketReportScroll");

	packet p;

	try {

		int32_t ticket_scroll_item_id = _packet->readInt32();
		int32_t ticket_scroll_id = _packet->readInt32();

		CHECK_SESSION_IS_AUTHORIZED("OpenTicketReportScroll");

		item_manager::openTicketReportScroll(_session, ticket_scroll_item_id, ticket_scroll_id, true );

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestOpenTicketReportScroll][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x11A);

		p.addInt32(-1);
		p.addZeroByte(16);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestChangeMascotMessage(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChangeMascotMessage");

	packet p;

	try {

		int32_t mascot_id = _packet->readInt32();
		std::string msg = _packet->readString();

		CHECK_SESSION_IS_AUTHORIZED("ChangeMascotMessage");

		if (msg.empty())
			throw exception("[channel::requestChangeMascotMessage][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "], tentou trocar a message["
					+ msg + "] do Mascot[ID=" + std::to_string(mascot_id) + "], mas a message esta vazia. Hacker ou Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x6200100, 0));

		if (msg.length() > 30)
			throw exception("[channel::requestChangeMascotMessage][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "], tentou trocar a message["
					+ msg + "] do Mascot[ID=" + std::to_string(mascot_id) + "], mas o comprimento da message ultrapassa os 30 caracteres permitido. Hacker ou Bug.",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x6200101, 0));

		auto pMi = _session.m_pi.findMascotById(mascot_id);

		if (pMi == nullptr)
			throw exception("[channel::requestChangeMascotMessage][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "], tentou trocar a message["
					+ msg + "] do Mascot[ID=" + std::to_string(mascot_id) + "], mas ele nao tem esse mascot. Hacker ou Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x6200102, 0));

		if (!sIff::getInstance().isLoad())
			sIff::getInstance().load();

		auto mascot = sIff::getInstance().findMascot(pMi->_typeid);

		if (mascot == nullptr || !mascot->active)
			throw exception("[channel::requestChangeMascotMessage][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "], tentou trocar a message["
					+ msg + "] do Mascot[TYPEID=" + std::to_string(pMi->_typeid) + " ID=" + std::to_string(pMi->id)
					+ "], mas nao existe ou nao esta ativado esse mascot no IFF_STRUCT do server. Hacker ou Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x6200103, 0));

		if (!mascot->msg.active)
			throw exception("[channel::requestChangeMascotMessage][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "], tentou trocar a message["
					+ msg + "] do Mascot[TYPEID=" + std::to_string(pMi->_typeid) + " ID=" + std::to_string(pMi->id) + "], mas a message do mascot nao esta ativado. Hacker ou Bug.",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x6200104, 0));

		try {

			if (mascot->msg.change_price > 0)
				_session.m_pi.consomePang(mascot->msg.change_price);

		}catch (exception& e) {

			_smp::message_pool::getInstance().push(new message("[channel::requestChangeMascotMessage][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

			throw exception("[channel::requestChangeMascotMessage][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "], tentou trocar a message["
					+ msg + "] do Mascot[TYPEID=" + std::to_string(pMi->_typeid) + " ID=" + std::to_string(pMi->id) + "], mas o player nao tem Pang[HAVE="
					+ std::to_string(_session.m_pi.ui.pang) + ", REQ=" + std::to_string(mascot->msg.change_price)
					+ "] suficiente para trocar a mensagem do mascot. Hacker ou Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x6200105, 0));
		}

		memset(pMi->message, 0, sizeof(pMi->message));

#if defined(_WIN32)
		memcpy_s(pMi->message, sizeof(pMi->message), msg.data(), msg.length());
#elif defined(__linux__)
		memcpy(pMi->message, msg.data(), msg.length());
#endif

		snmdb::NormalManagerDB::getInstance().add(26, new CmdUpdateMascotInfo(_session.m_pi.uid, *pMi), channel::SQLDBResponse, this);

		p.init_plain((unsigned short)0xE2);

		p.addInt8(4);

		p.addInt32(pMi->id);

		p.addString(pMi->message);

		p.addUint64(_session.m_pi.ui.pang);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestChangeMascotMessage][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0xE2);

		p.addInt8(-1);

		p.addInt32(-1);

		p.addUint16(0);

		p.addUint64(_session.m_pi.ui.pang);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestPayCaddieHolyDay(player& _session, packet *_packet) {
	REQUEST_BEGIN("PayCaddieHolyDay");

	packet p;

	try {

		int32_t caddie_id = _packet->readInt32();

		CHECK_SESSION_IS_AUTHORIZED("PayCaddieHolyDay");

		if (caddie_id <= 0)
			throw exception("[channel::requestPayCaddieHolyDay][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou pagar as ferias do Caddie[ID="
					+ std::to_string(caddie_id) + "], mas o caddie_id eh invalido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6100101));

		auto pCi = _session.m_pi.findCaddieById(caddie_id);

		if (pCi == nullptr)
			throw exception("[channel::requestPayCaddieHolyDay][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou pagar as ferias do Caddie[ID="
					+ std::to_string(caddie_id) + "], mas o ele nao possui esse Caddie. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0x6100102));

		auto caddie = sIff::getInstance().findCaddie(pCi->_typeid);

		if (caddie == nullptr)
			throw exception("[channel::requestPayCaddieHolyDay][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou pagar as ferias do Caddie[TYPEID="
					+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "], mas nao tem esse caddie no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0x6100103));

		if ((!caddie->shop.flag_shop.uFlagShop.stFlagShop.is_cash && caddie->valor_mensal <= 0) || pCi->rent_flag != 2)
			throw exception("[channel::requestPayCaddieHolyDay][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou pagar as ferias do Caddie[TYPEID="
					+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "], mas nao eh um caddie valido para pagar as verias. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4, 0x6100104));

		if (caddie->valor_mensal > _session.m_pi.ui.pang)
			throw exception("[channel::requestPayCaddieHolyDay][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou pagar as ferias do Caddie[TYPEID="
					+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "], mas o ele nao tem pangs suficiente[value="
					+ std::to_string(_session.m_pi.ui.pang) + ", request=" + std::to_string(caddie->valor_mensal) + "] para pagar as ferias do caddie. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 5, 0x6100105));

		auto end_date_unix = GetLocalTimeAsUnix() + (30 * 24 * 3600 );

		pCi->end_date = UnixToSystemTime(end_date_unix);

		pCi->updateEndDate();

		auto end_dt = _formatDate(pCi->end_date);

		_session.m_pi.consomePang(caddie->valor_mensal);

		snmdb::NormalManagerDB::getInstance().add(20, new CmdPayCaddieHolyDay(_session.m_pi.uid, pCi->id, end_dt), channel::SQLDBResponse, this);

		auto v_it = _session.m_pi.findUpdateItemByTypeidAndId(pCi->_typeid, pCi->id);

		if (!v_it.empty()) {

			for (auto& el : v_it)
				if (el->second.type == UpdateItem::CADDIE)

					_session.m_pi.mp_ui.erase(el);

		}

		_smp::message_pool::getInstance().push(new message("[PayCaddieHolyDay][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] pagou as ferias do Caddie[TYPEID="
				+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + ", PRICE=" + std::to_string(caddie->valor_mensal) + "] ate " + end_dt , CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x93);

		p.addUint8(2);

		p.addInt32(pCi->id);
		p.addUint64(_session.m_pi.ui.pang);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestPayCaddieHolyDay][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x93);

		p.addUint8(1);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestSetNoticeBeginCaddieHolyDay(player& _session, packet *_packet) {
	REQUEST_BEGIN("SetNoticeBeginCaddieHolyDay");

	try {

		int32_t caddie_id = _packet->readInt32();
		unsigned char check = _packet->readUint8();

		CHECK_SESSION_IS_AUTHORIZED("SetNoticeBeginCaddieHolyDay");

		if (caddie_id <= 0)
			throw exception("[channel::requestSetNoticeBeginCaddieHolyDay][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou setar ou desetar o Aviso de ferias do Caddie[ID="
					+ std::to_string(caddie_id) + "], mas o caddie_id is invalid. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6200101));

		auto pCi = _session.m_pi.findCaddieById(caddie_id);

		if (pCi == nullptr)
			throw exception("[channel::requestSetNoticeBeginCaddieHolyDay][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou setar ou desetar o Aviso de ferias do Caddie[ID="
					+ std::to_string(caddie_id) + "], mas ele nao tem esse caddie. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0x6200102));

		auto caddie = sIff::getInstance().findCaddie(pCi->_typeid);

		if (caddie == nullptr)
			throw exception("[channel::requestSetNoticeBeginCaddieHolyDay][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou setar ou desetar o Aviso de ferias do Caddie[TYPEID="
					+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "], mas nao tem esse caddie no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0x6200103));

		if ((!caddie->shop.flag_shop.uFlagShop.stFlagShop.is_cash && caddie->valor_mensal <= 0) || pCi->rent_flag != 2)
			_smp::message_pool::getInstance().push(new message("[channel::requestSetNoticeBeginCaddieHolyDay][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou setar ou desetar o Aviso de ferias do Caddie[TYPEID="
					+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "], mas esse nao eh um caddie valido para setar aviso de ferias. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

		if (pCi->check_end != check) {

			pCi->check_end = check;

			snmdb::NormalManagerDB::getInstance().add(21, new CmdSetNoticeCaddieHolyDay(_session.m_pi.uid, pCi->id, pCi->check_end), channel::SQLDBResponse, this);
		}

		_smp::message_pool::getInstance().push(new message("[Caddie::SetNoticeHolyDay][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] setou o avisou de ferias do Caddie[TYPEID="
				+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + ", CHECK=" + (pCi->check_end ? std::string("ON") : std::string("OFF")) + "]", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[requestSetNoticeBeginCaddieHolyDay][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

	}
};

void channel::requestBuyItemShop(player& _session, packet* _packet) {
	REQUEST_BEGIN("BuyItemShop");

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("Packet 0x1D.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

	try {

		CHECK_SESSION_IS_AUTHORIZED("BuyItemShop");

		if (_session.m_pi.block_flag.m_flag.stBit.buy_and_gift_shop)
			throw exception("[channel::requestBuyItemShop][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+"] tentou comprar item no shop, mas ele nao pode. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x790001));

		CPLog cp_log;

		cp_log.setType(CPLog::TYPE::BUY_SHOP);

		BuyItem bi{ 0 };
		unsigned char option = _packet->readUint8();
		unsigned short qntd = _packet->readUint16();

		stItem coupon{ 0u };
		std::string coupon_msg = "";

		uint64_t pang = 0ull, cookie = 0ull;

		if (qntd > 0) {

			stItem item{ 0 };
			std::vector< stItem > v_item;

			for (auto i = 0u; i < qntd; ++i) {

				_packet->readBuffer(&bi, sizeof(BuyItem));

				if (sIff::getInstance().IsBuyItem(bi._typeid) && !sIff::getInstance().IsOnlyGift(bi._typeid)) {

					if (bi.pang > 0)
						pang += bi.pang;

					if (bi.cookie > 0)
						cookie += bi.cookie;

					item.clear();

					item_manager::initItemFromBuyItem(_session.m_pi, item, bi, true, option);

					if (item._typeid == 0) {

						_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Error] ao inicializar item from buyItem, item typeid: " + std::to_string(bi._typeid)
								+ " bug. para o Player[UID=" + std::to_string(_session.m_pi.uid) + "]", CL_FILE_LOG_AND_CONSOLE));

						packet p((unsigned short)0x68);

						p.addUint32(1);

						packet_func::session_send(p, &_session, 0);

						return;
					}

					if (item.is_cash ? (option != 1  && item.desconto != 0 ? bi.cookie != (item.desconto * item.qntd) : bi.cookie != (item.price * item.qntd)) :
						(option != 1  && item.desconto != 0 ? bi.pang != (item.desconto * item.qntd) : bi.pang != (item.price * item.qntd))) {

						_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou comprar um item com preco[server="
								+ std::to_string((item.desconto != 0 ? (item.desconto * item.qntd) : (item.price * item.qntd))) + ", cliente="
								+ std::to_string((item.is_cash ? bi.cookie : bi.pang)) + "] diferente, item typeid: " + std::to_string(bi._typeid) + ". Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

						packet p((unsigned short)0x68);

						p.addUint32(2);

						packet_func::session_send(p, &_session, 0);

						return;
					}

					if (!item_manager::isTimeItem(item.date) || item_manager::betweenTimeSystem(item.date)) {

						if ((sIff::getInstance().IsCanOverlapped(item._typeid) && sIff::getInstance().getItemGroupIdentify(item._typeid) != iff::CAD_ITEM) || !_session.m_pi.ownerItem(item._typeid)) {

							if (item_manager::isSetItem(item._typeid)) {

								auto v_stItem = item_manager::getItemOfSetItem(_session, item._typeid, true, 1 );

								if (item.is_cash && bi.cookie > 0)
									cp_log.putItem(item._typeid, (item.STDA_C_ITEM_TIME > 0 ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD), bi.cookie);

								if (!v_stItem.empty()) {

									for (auto& el : v_stItem)
										if ((sIff::getInstance().IsCanOverlapped(el._typeid) && sIff::getInstance().getItemGroupIdentify(el._typeid) != iff::CAD_ITEM) || !_session.m_pi.ownerItem(el._typeid))
											v_item.push_back(el);

								}else {

									_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
											+ "] tentou comprar um set item que nao tem item, item typeid: " + std::to_string(bi._typeid) + ". Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

									packet p((unsigned short)0x68);

									p.addUint32(3);

									packet_func::session_send(p, &_session, 0);

									return;
								}

							}else {

								v_item.push_back(item);

								if (item.is_cash && bi.cookie > 0)
									cp_log.putItem(item._typeid, (item.STDA_C_ITEM_TIME > 0 ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD), bi.cookie);
							}

						}else if (sIff::getInstance().getItemGroupIdentify(item._typeid) == iff::CAD_ITEM) {

							_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
									+ "] tentou comprar um CaddieItem que ele nao tem o caddie, item typeid: " + std::to_string(bi._typeid) + ". Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

							packet p((unsigned short)0x68);

							p.addUint32(11);

							packet_func::session_send(p, &_session, 0);

							return;
						}else {

							_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
									+ "] tentou comprar um item que ele ja tem, item typeid: " + std::to_string(bi._typeid) + ". Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

							packet p((unsigned short)0x68);

							p.addUint32(4);

							packet_func::session_send(p, &_session, 0);

							return;
						}

					}else {

						_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
								+ "] tentou comprar um item que nao pode comprar, nao esta na data, item typeid: " + std::to_string(bi._typeid) + ". Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

						packet p((unsigned short)0x68);

						p.addUint32(5);

						packet_func::session_send(p, &_session, 0);

						return;
					}

				}else {

					_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] tentou comprar um item que nao pode ser comprado, item typeid: " + std::to_string(bi._typeid) + ". Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

					packet p((unsigned short)0x68);

					p.addUint32(6);

					packet_func::session_send(p, &_session, 0);

					return;
				}
			}

			coupon.id = _packet->readUint32();

			if (coupon.id != 0) {

				auto wi_coupon = _session.m_pi.findWarehouseItemById(coupon.id);

				if (wi_coupon == nullptr) {

					_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] tentou comprar um item com coupon de descontou, mas ele nao tem o coupon[ID=" + std::to_string(coupon.id)
							+ "], item typeid: " + std::to_string(bi._typeid) + ". Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

					packet p((unsigned short)0x68);

					p.addUint32(6);

					packet_func::session_send(p, &_session, 0);

					return;
				}

				coupon.type = 2;
				coupon._typeid = wi_coupon->_typeid;
				coupon.qntd = 1;
				coupon.STDA_C_ITEM_QNTD = (short)coupon.qntd * -1;

				uint64_t old_price = cookie;
				std::string type_desconto = "5%";

				if (wi_coupon->_typeid == 0x1A00001Eu
					|| wi_coupon->_typeid == 0x1A000015u
					|| wi_coupon->_typeid == 0x1A00001Du) {

					if (((int64_t)cookie - 5ll) < 0)
						cookie = 0ull;
					else
						cookie -= 5ull;

					type_desconto = "5CP";

				}else if (wi_coupon->_typeid == 0x1A00003Cu) {

					cookie = (uint64_t)(cookie * 0.80f);

					type_desconto = "20%";

				}else
					cookie = (uint64_t)(cookie * 0.95f);

				coupon_msg = " e usou Coupon[TYPEID=" + std::to_string(wi_coupon->_typeid) + ", ID=" + std::to_string(wi_coupon->id)
						+ ", DESCONTO=" + type_desconto + ", TOTAL_CP=" + std::to_string(old_price)
						+ ", TOTAL_CP_COM_DESCONTO=" + std::to_string(cookie) + "]";
			}

			if (_session.m_pi.cookie < cookie || _session.m_pi.ui.pang < pang) {

				_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou comprar um item, mas nao tem moedas(Pang ou Cookie) suficiente, item typeid: " + std::to_string(bi._typeid) + ". Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

				packet p((unsigned short)0x68);

				p.addUint32(7);

				packet_func::session_send(p, &_session, 0);

				return;
			}

			try {

				_session.m_pi.consomeMoeda(pang, cookie);

			}catch (exception& e) {

				_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

				if (STDA_ERROR_CHECK_SOURCE_AND_ERROR(e.getCodeError(), STDA_ERROR_TYPE::PLAYER_INFO, 200 )) {

					packet p((unsigned short)0x68);

					p.addUint32(2);

					packet_func::session_send(p, &_session, 0);

					return;

				}else
					throw;
			}

			if (coupon.id != 0 && coupon._typeid != 0u) {

				if (item_manager::removeItem(coupon, _session) <= 0) {

					_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] tentou comprar um item com coupon de descontou, mas nao conseguiu remove o coupon[TYPEID=" + std::to_string(coupon._typeid)
							+ ", ID=" + std::to_string(coupon.id) + "], item typeid: " + std::to_string(bi._typeid) + ". Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

					_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Log] devolve as moedas gasta deu erro no add itens no db para o player.", CL_FILE_LOG_AND_CONSOLE));

					_session.m_pi.addMoeda(pang, cookie);

					packet p((unsigned short)0x68);

					p.addUint32(8);

					packet_func::session_send(p, &_session, 0);

					return;
				}

				packet p((unsigned short)0x216);

				p.addUint32((const unsigned int)GetSystemTimeAsUnix());
				p.addUint32(1u);

				p.addUint8(coupon.type);
				p.addInt32(coupon._typeid);
				p.addInt32(coupon.id);
				p.addInt32(coupon.flag_time);
				p.addUint32(coupon.stat.qntd_ant);
				p.addUint32(coupon.stat.qntd_dep);
				p.addUint32((coupon.STDA_C_ITEM_TIME > 0 ? coupon.STDA_C_ITEM_TIME : coupon.STDA_C_ITEM_QNTD));
				p.addZeroByte(25);

				packet_func::session_send(p, &_session, 1);
			}

			auto rai = item_manager::addItem(v_item, _session, 0, 1);

			if (rai.fails.size() > 0 && rai.type != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH) {

				std::string str = "";

				for (auto i = 0u; i < rai.fails.size(); ++i) {

					if (i == 0)
						str += "[TYPEID=" + std::to_string(rai.fails[i]._typeid) + ", ID=" + std::to_string(rai.fails[i].id) + ", QNTD=" + std::to_string((rai.fails[i].qntd > 0xFFu) ? rai.fails[i].qntd : rai.fails[i].STDA_C_ITEM_QNTD)
							+ (rai.fails[i].STDA_C_ITEM_TIME > 0 ? ", TEMPO=" + std::to_string(rai.fails[i].STDA_C_ITEM_TIME) : std::string("")) + "]";
					else
						str += ", [TYPEID=""" + std::to_string(rai.fails[i]._typeid) + ", ID=" + std::to_string(rai.fails[i].id) + ", QNTD=" + std::to_string((rai.fails[i].qntd > 0xFFu) ? rai.fails[i].qntd : rai.fails[i].STDA_C_ITEM_QNTD)
							+ (rai.fails[i].STDA_C_ITEM_TIME > 0 ? ", TEMPO=" + std::to_string(rai.fails[i].STDA_C_ITEM_TIME) : std::string("")) + "]";
				}

				_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Error] Itens que falhou ao add os itens que o Player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] comprou item(ns){" + str + "}. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

				_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Log] devolve as moedas gasta deu erro no add itens no db para o player.", CL_FILE_LOG_AND_CONSOLE));

				_session.m_pi.addMoeda(pang, cookie);

				packet p((unsigned short)0x68);

				p.addUint32(8);

				packet_func::session_send(p, &_session, 0);

				return;
			}

			std::string log_itens = "";

			for (auto el : v_item) {

				if (!log_itens.empty())
					log_itens += ", ";

				log_itens += "[TYPEID=" + std::to_string(el._typeid) + ", ID=" + std::to_string(el.id) + ", FLAG_TIME="
						+ std::to_string((unsigned short)el.flag_time) + ", QNTD="
						+ std::to_string((el.STDA_C_ITEM_TIME > 0 ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD))
						+ ", QNTD_DEPOIS=" + std::to_string(el.stat.qntd_dep) + "]";
			}

#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] comprou " + std::to_string(v_item.size()) + " item(ns), Moedas(CP=" + std::to_string(cookie)
					+ ", PANG=" + std::to_string(pang) + ")," + coupon_msg + " no Shop{ " + log_itens + " }", CL_FILE_LOG_AND_CONSOLE));
#else
			_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] comprou " + std::to_string(v_item.size()) + " item(ns), Moedas(CP=" + std::to_string(cookie)
					+ ", PANG=" + std::to_string(pang) + ")," + coupon_msg + " no Shop{ " + log_itens + " }", CL_ONLY_FILE_LOG));
#endif

			packet p;

			if (pang > 0) {
				p.init_plain((unsigned short)0xC8);

				p.addUint64(_session.m_pi.ui.pang);
				p.addUint64(pang);

				packet_func::session_send(p, &_session, 1);
			}

			if (cookie > 0) {

				_session.saveCPLog(cp_log);

				p.init_plain((unsigned short)0x96);

				p.addUint64(_session.m_pi.cookie);

				packet_func::session_send(p, &_session, 1);
			}

			packet_func::pacote0AA(p, &_session, v_item);
			packet_func::session_send(p, &_session, 1);

			p.init_plain((unsigned short)0x68);

			p.addUint32(0);
			p.addUint64(_session.m_pi.ui.pang);
			p.addUint64(_session.m_pi.cookie);

			packet_func::session_send(p, &_session, 0);

		}else {

			_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou comprar um item, mas nao enviou nenhum item no request. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

			packet p((unsigned short)0x68);

			p.addUint32(9);

			packet_func::session_send(p, &_session, 0);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "] error desconhecido: " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		packet p((unsigned short)0x68);

		p.addUint32(10);

		packet_func::session_send(p, &_session, 0);
	}
};

void channel::requestGiftItemShop(player& _session, packet *_packet) {
	REQUEST_BEGIN("GiftItemShop");

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("Packet 0x1F.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("GiftItemShop");

		CPLog cp_log;

		cp_log.setType(CPLog::TYPE::GIFT_SHOP);

		BuyItem bi{ 0 };

		unsigned short option = _packet->readUint16();
		uint32_t uid_to_send = _packet->readUint32();
		std::string msg = _packet->readString();
		unsigned char opt2 = _packet->readUint8();
		unsigned short qntd = _packet->readUint16();

		uint64_t pang = 0ull, cookie = 0ull;

		if (_session.m_pi.block_flag.m_flag.stBit.gift_shop || _session.m_pi.block_flag.m_flag.stBit.buy_and_gift_shop)
			throw exception("[channel::requestGiftItemShop][Error] player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] tentou presentear player[UID=" + std::to_string(uid_to_send) + "], mas ele nao pode. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0x790001));

		if (_session.m_pi.level < _session.m_pi.enLEVEL::BEGINNER_E)
			throw exception("[channel::requestGiftItemShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + ", LEVEL=" + std::to_string(_session.m_pi.level)
					+ "] tentou presentear o player[UID=" + std::to_string(uid_to_send) + "], mas o level dele eh menor que Beginner E.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3500, 1));

		if (qntd > 0) {

			stItem item{ 0 };
			std::vector< stItem > v_item;

			for (auto i = 0u; i < qntd; ++i) {

				_packet->readBuffer(&bi, sizeof(BuyItem));

				if (sIff::getInstance().IsGiftItem(bi._typeid)) {

					if (bi.pang > 0)
						pang += bi.pang;

					if (bi.cookie > 0)
						cookie += bi.cookie;

					item.clear();

					item_manager::initItemFromBuyItem(_session.m_pi, item, bi, true, option, 1 );

					if (item._typeid == 0) {

						_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][Error] ao inicializar item from buyItem, item typeid: "
								+ std::to_string(bi._typeid) + " bug. para o Player[UID=" + std::to_string(_session.m_pi.uid) + "]", CL_FILE_LOG_AND_CONSOLE));

						p.init_plain((unsigned short)0x6A);

						p.addUint32(1);

						p.addUint64(_session.m_pi.ui.pang);
						p.addUint64(_session.m_pi.cookie);

						packet_func::session_send(p, &_session, 0);

						return;
					}

					if (item.is_cash ? (item.desconto != 0 ? bi.cookie != (item.desconto * item.qntd) : bi.cookie != (item.price * item.qntd)) :
						(item.desconto != 0 ? bi.pang != (item.desconto * item.qntd) : bi.pang != (item.price * item.qntd))) {

						_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou presentear para o player[UID="
								+ std::to_string(uid_to_send) + "] um item com preco[server="
								+ std::to_string((item.desconto != 0 ? (item.desconto * item.qntd) : (item.price * item.qntd))) + ", cliente="
								+ std::to_string((item.is_cash ? bi.cookie : bi.pang)) + "] diferente, item typeid: " + std::to_string(bi._typeid) + ". Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

						p.init_plain((unsigned short)0x6A);

						p.addUint32(2);

						p.addUint64(_session.m_pi.ui.pang);
						p.addUint64(_session.m_pi.cookie);

						packet_func::session_send(p, &_session, 0);

						return;
					}

					if (!item_manager::isTimeItem(item.date) || item_manager::betweenTimeSystem(item.date)) {

						if ((sIff::getInstance().IsCanOverlapped(item._typeid) && sIff::getInstance().getItemGroupIdentify(item._typeid) != iff::CAD_ITEM)
								|| !item_manager::ownerItem(uid_to_send, item._typeid) ) {

							if (item_manager::isSetItem(item._typeid)) {

								if (item.is_cash && bi.cookie > 0)
									cp_log.putItem(item._typeid, (item.STDA_C_ITEM_TIME > 0 ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD), bi.cookie);

								auto v_stItem = item_manager::getItemOfSetItem(_session, item._typeid, true, 1 );

								if (!v_stItem.empty()) {

									v_item.push_back(item);

								}else {

									_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
											+ "] tentou presentear para o player[UID=" + std::to_string(uid_to_send) + "] um set item que nao tem item, item typeid: "
											+ std::to_string(bi._typeid) + ". Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

									p.init_plain((unsigned short)0x6A);

									p.addUint32(3);

									p.addUint64(_session.m_pi.ui.pang);
									p.addUint64(_session.m_pi.cookie);

									packet_func::session_send(p, &_session, 0);

									return;
								}

							}else {

								v_item.push_back(item);

								if (item.is_cash && bi.cookie > 0)
									cp_log.putItem(item._typeid, (item.STDA_C_ITEM_TIME > 0 ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD), bi.cookie);
							}

						}else if (sIff::getInstance().getItemGroupIdentify(item._typeid) == iff::CAD_ITEM) {

							_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
									+ "] tentou presentear um CaddieItem que o player[UID=" + std::to_string(uid_to_send) + "] nao tem o caddie, item typeid: "
									+ std::to_string(bi._typeid), CL_FILE_LOG_AND_CONSOLE));

							p.init_plain((unsigned short)0x6A);

							p.addUint32(11);

							p.addUint64(_session.m_pi.ui.pang);
							p.addUint64(_session.m_pi.cookie);

							packet_func::session_send(p, &_session, 0);

							return;

						}else {

							_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
									+ "] tentou presentear um item que o player[UID=" + std::to_string(uid_to_send) + "] ja tem, item typeid: " + std::to_string(bi._typeid), CL_FILE_LOG_AND_CONSOLE));

							p.init_plain((unsigned short)0x6A);

							p.addUint32(4);

							p.addUint64(_session.m_pi.ui.pang);
							p.addUint64(_session.m_pi.cookie);

							packet_func::session_send(p, &_session, 0);

							return;
						}

					}else {

						_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
								+ "] tentou presentear para o player[UID=" + std::to_string(uid_to_send) + "] um item que nao esta na data para esta disponivel no shop, item typeid: "
								+ std::to_string(bi._typeid) + ". Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

						p.init_plain((unsigned short)0x6A);

						p.addUint32(5);

						p.addUint64(_session.m_pi.ui.pang);
						p.addUint64(_session.m_pi.cookie);

						packet_func::session_send(p, &_session, 0);

						return;
					}

				}else {

					_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] tentou presentear para o player[UID=" + std::to_string(uid_to_send) + "] um item que nao pode ser comprado[indisponivel no shop], item typeid: "
							+ std::to_string(bi._typeid) + ". Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

					p.init_plain((unsigned short)0x6A);

					p.addUint32(6);

					p.addUint64(_session.m_pi.ui.pang);
					p.addUint64(_session.m_pi.cookie);

					packet_func::session_send(p, &_session, 0);

					return;
				}
			}

			if (_session.m_pi.cookie < cookie || _session.m_pi.ui.pang < pang) {

				_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou presentear para o player[UID=" + std::to_string(uid_to_send) + "] um item, mas nao tem moedas(Pang ou Cookie) suficiente, item typeid: "
						+ std::to_string(bi._typeid) + ". Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

				p.init_plain((unsigned short)0x6A);

				p.addUint32(7);

				p.addUint64(_session.m_pi.ui.pang);
				p.addUint64(_session.m_pi.cookie);

				packet_func::session_send(p, &_session, 0);

				return;
			}

			try {

				_session.m_pi.consomeMoeda(pang, cookie);

			}catch (exception& e) {

				_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

				if (STDA_ERROR_CHECK_SOURCE_AND_ERROR(e.getCodeError(), STDA_ERROR_TYPE::PLAYER_INFO, 200 )) {

					p.init_plain((unsigned short)0x6A);

					p.addUint32(2);

					p.addUint64(_session.m_pi.ui.pang);
					p.addUint64(_session.m_pi.cookie);

					packet_func::session_send(p, &_session, 0);

					return;

				}else
					throw;
			}

			int32_t mail_id = 0;

			try {

				if ((mail_id = MailBoxManager::sendMessageWithItem(_session.m_pi.uid, uid_to_send, msg, v_item)) <= 0)
					throw exception("[channel::requestGiftItemShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou presentear um player[UID="
							+ std::to_string(uid_to_send) + "] com o Item[TYPEID=" + std::to_string(bi._typeid)+ "], mas nao conseguiu colocar o item no mail box do player. Bug",
							STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5800101));

			}catch (exception& e) {

				_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

				_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][Error] ao add os itens que o Player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] presenteou para o player[UID=" + std::to_string(uid_to_send) + "]. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

				_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][Log] devolve as moedas gasta deu erro no add itens no db para o player.", CL_FILE_LOG_AND_CONSOLE));

				_session.m_pi.addMoeda(pang, cookie);

				p.init_plain((unsigned short)0x6A);

				p.addUint32(8);

				p.addUint64(_session.m_pi.ui.pang);
				p.addUint64(_session.m_pi.cookie);

				packet_func::session_send(p, &_session, 0);

				return;

			}

			std::string log_itens = "";

			for (auto& el : v_item) {

				if (!log_itens.empty())
					log_itens += ", ";

				log_itens += "[TYPEID=" + std::to_string(el._typeid) + ", ID=" + std::to_string(el.id)
						+ ", FLAG_TIME=" + std::to_string((unsigned short)el.flag_time) + ", QNTD="
						+ std::to_string((el.STDA_C_ITEM_TIME > 0 ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD))
						+ ", QNTD_DEPOIS=" + std::to_string(el.stat.qntd_dep) + "]";
			}

#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] MailBox[MAIL_ID=" + std::to_string(mail_id) + "] mandou " + std::to_string(v_item.size())
					+ " presente(s), Moedas(CP=" + std::to_string(cookie) + ", PANG=" + std::to_string(pang)
					+ "), do Shop para o Player[UID=" + std::to_string(uid_to_send) + "]. Item(ns) { " + log_itens + " }", CL_FILE_LOG_AND_CONSOLE));
#else
			_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] MailBox[MAIL_ID=" + std::to_string(mail_id) + "] mandou " + std::to_string(v_item.size())
					+ " presente(s), Moedas(CP=" + std::to_string(cookie) + ", PANG=" + std::to_string(pang)
					+ "), do Shop para o Player[UID=" + std::to_string(uid_to_send) + "]. Item(ns) { " + log_itens + " }", CL_ONLY_FILE_LOG));
#endif

			if (pang > 0) {
				p.init_plain((unsigned short)0xC8);

				p.addUint64(_session.m_pi.ui.pang);
				p.addUint64(pang);

				packet_func::session_send(p, &_session, 1);
			}

			if (cookie > 0) {

				cp_log.setMailId(mail_id);

				_session.saveCPLog(cp_log);

				p.init_plain((unsigned short)0x96);

				p.addUint64(_session.m_pi.cookie);

				packet_func::session_send(p, &_session, 1);
			}

			p.init_plain((unsigned short)0x6A);

			p.addUint32(0);
			p.addUint64(_session.m_pi.ui.pang);
			p.addUint64(_session.m_pi.cookie);

			packet_func::session_send(p, &_session, 0);

		}else {

			_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou presentear para o player[UID=" + std::to_string(uid_to_send) + "] um item, mas nao enviou nenhum item no request. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x6A);

			p.addUint32(9);

			p.addUint64(_session.m_pi.ui.pang);
			p.addUint64(_session.m_pi.cookie);

			packet_func::session_send(p, &_session, 0);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestGiftItemShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] error desconhecido: " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x6A);

		p.addUint32(STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 10);

		p.addUint64(_session.m_pi.ui.pang);
		p.addUint64(_session.m_pi.cookie);

		packet_func::session_send(p, &_session, 0);
	}
};

void channel::requestExtendRental(player& _session, packet *_packet) {
	REQUEST_BEGIN("ExtendRental");

	packet p;

	try {

		int32_t item_id = _packet->readInt32();

		CHECK_SESSION_IS_AUTHORIZED("ExtendRental");

		if (item_id <= 0)
			throw exception("[channel::requestExtendRental][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou extend rental, mas o item[ID=" + std::to_string(item_id) + "] is invalid. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 350, 5200351));

		auto pWi = _session.m_pi.findWarehouseItemById(item_id);

		if (pWi == nullptr)
			throw exception("[channel::requestExtendRental][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou extend rental, mas o player nao tem o item[ID=" + std::to_string(item_id) + "]. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 351, 5200352));

		if (sIff::getInstance().getItemGroupIdentify(pWi->_typeid) != iff::PART)
			throw exception("[channel::requestExtendRental][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou extend rental, mas o item[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "] nao eh um Part. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 352, 5200353));

		auto part = sIff::getInstance().findPart(pWi->_typeid);

		if (part == nullptr)
			throw exception("[channel::requestExtendRental][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou extender um rental Item[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "] que nao esta no IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 353, 5200354));

		if (part->valor_rental <= 0)
			throw exception("[channel::requestExtendRental][Error] player[UID=" + std::to_string(_session.m_pi.uid) +"] tentou extender um rental Item[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "] que nao eh um rental no IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 354, 5200355));

		pWi->end_date_unix_local = (uint32_t)GetLocalTimeAsUnix() + (7 * 24 * 3600) ;

		pWi->end_date = TzLocalUnixToUnixUTC(pWi->end_date_unix_local);

		auto end_date = formatDateLocal(pWi->end_date_unix_local);

		snmdb::NormalManagerDB::getInstance().add(5, new CmdExtendRental(_session.m_pi.uid, pWi->id, end_date), channel::SQLDBResponse, this);

		_session.m_pi.consomePang(part->valor_rental);

		auto v_it = _session.m_pi.findUpdateItemByTypeidAndId(pWi->_typeid, pWi->id);

		if (!v_it.empty()) {

			for (auto& el : v_it)
				if (el->second.type == UpdateItem::WAREHOUSE)

					_session.m_pi.mp_ui.erase(el);

		}

		_smp::message_pool::getInstance().push(new message("[Rental::Extend][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] extendeu o Rental Item[TYPEID="
				+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0xC8);

		p.addUint64(_session.m_pi.ui.pang);
		p.addUint64(part->valor_rental);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x18F);

		p.addUint8(0);

		p.addUint32(pWi->_typeid);
		p.addInt32(pWi->id);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestExtendRental][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x18F);

		p.addUint8(1);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestDeleteRental(player& _session, packet* _packet) {
	REQUEST_BEGIN("DeleteRental");

	packet p;

	try {

		int32_t item_id = _packet->readInt32();

		CHECK_SESSION_IS_AUTHORIZED("DeleteRental");

		if (item_id <= 0)
			throw exception("[channel::requestDeleteRental][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar um Rental item[ID="
					+ std::to_string(item_id) + "] invalid. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 400, 5200401));

		auto pWi = _session.m_pi.findWarehouseItemById(item_id);

		if (pWi == nullptr)
			throw exception("[channel::requestDeleteRental][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar um Rental item[ID="
					+ std::to_string(item_id) + "] que ele nao tem. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 401, 5200402));

		if (sIff::getInstance().getItemGroupIdentify(pWi->_typeid) != iff::PART)
			throw exception("[channel::requestDeleteRental][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar um Rental Item[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "] que nao eh um Part. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 402, 5200403));

		auto part = sIff::getInstance().findPart(pWi->_typeid);

		if (part == nullptr)
			throw exception("[channel::requestDeleteRental][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar um rental Item[TYPEID="
				+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "] que nao esta no IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 403, 5200404));

		if (part->valor_rental <= 0)
			throw exception("[channel::requestDeleteRental][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar um rental Item[TYPEID="
				+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "] que nao eh um rental no IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 404, 5200404));

		auto tmp_wi = *pWi;

		auto it = _session.m_pi.findWarehouseItemItById(pWi->id);

		if (it != _session.m_pi.mp_wi.end())
			_session.m_pi.mp_wi.erase(it);

		snmdb::NormalManagerDB::getInstance().add(6, new CmdDeleteRental(_session.m_pi.uid, tmp_wi.id), channel::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[Rental::Delete][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] deletou Rental Item[TYPEID="
				+ std::to_string(tmp_wi._typeid) + ", ID=" + std::to_string(tmp_wi.id) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x190);

		p.addUint8(0);

		p.addUint32(tmp_wi._typeid);
		p.addInt32(tmp_wi.id);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestDeleteRental][ErroSytem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x190);

		p.addUint8(1);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestCheckAttendanceReward(player& _session, packet *_packet) {
	REQUEST_BEGIN("CheckAttendanceReward");

	try {

		CHECK_SESSION_IS_AUTHORIZED("CheckAttendanceReward");

		if (!sAttendanceRewardSystem::getInstance().isLoad())
			sAttendanceRewardSystem::getInstance().load();

		sAttendanceRewardSystem::getInstance().requestCheckAttendance(_session, _packet);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestCheckAttendanceReward][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestAttendanceRewardLoginCount(player& _session, packet *_packet) {
	REQUEST_BEGIN("AttendanceRewardLoginCount");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("AttendanceRewardLoginCount");

		if (!sAttendanceRewardSystem::getInstance().isLoad())
			sAttendanceRewardSystem::getInstance().load();

		sAttendanceRewardSystem::getInstance().requestUpdateCountLogin(_session, _packet);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestAttendanceRewardLoginCount][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestDailyQuest(player& _session, packet *_packet) {
	REQUEST_BEGIN("DailyQuest");

	try {

		CHECK_SESSION_IS_AUTHORIZED("DailyQuest");

		MgrDailyQuest::requestCheckAndSendDailyQuest(_session, _packet);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestDailyQuest][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestAcceptDailyQuest(player& _session, packet *_packet) {
	REQUEST_BEGIN("AcceptDailyQuest");

	try {

		CHECK_SESSION_IS_AUTHORIZED("AcceptDailyQuest");

		MgrDailyQuest::requestAcceptQuest(_session, _packet);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestAcceptDailyQuest][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestTakeRewardDailyQuest(player& _session, packet *_packet) {
	REQUEST_BEGIN("TakeRewardDailyQuest");

	try {

		CHECK_SESSION_IS_AUTHORIZED("TakeRewardDailyQuest");

		MgrDailyQuest::requestTakeRewardQuest(_session, _packet);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestTakeRewardDailyQuest][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestLeaveDailyQuest(player& _session, packet *_packet) {
	REQUEST_BEGIN("LeaveDailyQuest");

	try {

		CHECK_SESSION_IS_AUTHORIZED("LeaveDailyQuest");

		MgrDailyQuest::requestLeaveQuest(_session, _packet);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestLeaveDailyQuest][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestCadieCauldronExchange(player& _session, packet* _packet) {
	REQUEST_BEGIN("CadieCauldronExchange");

	packet p;

#if defined(__linux__)
#pragma pack(1)
#endif

	struct CadieExchangeItem {
		void clear() { memset(this, 0, sizeof(CadieExchangeItem)); };
		uint32_t _typeid;
		int32_t id;
	};

#if defined(__linux__)
#pragma pack()
#endif

	try {

		CHECK_SESSION_IS_AUTHORIZED("CadieCauldronExchange");

		if (_session.m_pi.block_flag.m_flag.stBit.cadie_recycle)
			throw exception("[channel::requestCaddieCauldronExchange][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou trocard item no Cadie Cauldron Exchange, mas ele nao pode. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 8, 0x790001));

		unsigned short seq = _packet->readUint16();
		uint32_t item_exchange_qntd = _packet->readUint32();
		unsigned char count = _packet->readUint8();

		if (count > 4)
			throw exception("[channel::requestCadieCauldronExchange][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tryed exchange item on CadieCauldron, but the count[value="
					+ std::to_string((unsigned short)count) + "] of item(ns) is wrong. Hacker or Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 450, 5200451));

		CadieExchangeItem cei[4]{ 0 };

		_packet->readBuffer(cei, sizeof(CadieExchangeItem) * count);

		auto cmb = sIff::getInstance().findCadieMagicBox(seq );

		if (cmb == nullptr)
			throw exception("[channel::requestCadieCauldronExchange][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar item no CadieCauldron, mas o Item[Seq="
					+ std::to_string(seq + 1) + "] que ele tentou trocar nao tem no IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 451, 5200452));

		if (cmb->seq != (seq + 1))
			throw exception("[channel::requestCadieCauldronExchange][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou trocar item no CadieCauldron, mas o Item[Seq_Player=" + std::to_string(seq + 1) + ", Seq_Srv=" + std::to_string(cmb->seq)
					+ "] que ele tentou trocar nao combina com a Seq do IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 462, 5200463));

		if (!cmb->active)
			throw exception("[channel::requestCadieCauldronExchange][Error] player[UID=" + std::to_string(_session.m_pi.uid) +"] tentou trocar item no CadieCauldron, mas o Item[Seq="
					+ std::to_string(seq + 1) + "] esta inativo. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 452, 5200453));

		for (auto i = 0u; i < (sizeof(cmb->item_trade._typeid) / sizeof(cmb->item_trade._typeid[0])); ++i)
			if (cmb->item_trade._typeid[i] != 0 && cmb->item_trade._typeid[i] != cei[i]._typeid)
				throw exception("[channel::requestCadieCauldronExchange][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar item no CadieCauldron, mas o Item[Seq="
						+ std::to_string(seq + 1) + "], item_trade[0-4] nao esta combinando. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 453, 5200454));

		if (_session.m_pi.level < cmb->level)
			throw exception("[channel::requestCadieCauldronExchange][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar item no CadieCauldron, mas ele nao tem level para o Item[Seq="
					+ std::to_string(seq + 1) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 454, 5200455));

		if (item_manager::isTimeItem(*(stItem::stDate::stDateSys*)&cmb->date) && !item_manager::betweenTimeSystem(*(stItem::stDate::stDateSys*)&cmb->date))
			throw exception("[channel::requestCadieCauldronExchange][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar item no CadieCauldron, mas o item[Seq="
					+ std::to_string(seq + 1) + "] nao esta mais na data[temporario]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 455, 5200456));

		if (cmb->box_random_id == 0 && !sIff::getInstance().IsCanOverlapped(cmb->item_receive._typeid) && _session.m_pi.ownerItem(cmb->item_receive._typeid))
			throw exception("[channel::requestCadieCauldronExchange][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar item[Seq="
					+ std::to_string(seq + 1) + ", TYPEID_RCV=" + std::to_string(cmb->item_receive._typeid) + "] no Cauldron que ele ja possui e nao pode ter duplicata", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 458, 5200459));

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		for (auto i = 0u; i < count; ++i) {

			if (item_manager::exchangeCadieMagicBox(_session, cei[i]._typeid, cei[i].id, cmb->item_trade.qntd[i] * item_exchange_qntd) <= 0)
				throw exception("[channel::requestCadieCauldronExchange][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar um item[Seq="
						+ std::to_string(seq + 1) + ", TYPEID=" + std::to_string(cei[i]._typeid) + "] que nao pode ser trocado no CadieCauldron. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 457, 5200458));

			if (r != nullptr && r->checkPersonalShopItem(_session, cei[i].id))
				throw exception("[channel::requestCadieCauldronExchange][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar o item[Seq=" + std::to_string(seq + 1) + ", TYPEID="
						+ std::to_string(cei[i]._typeid) + ", ID=" + std::to_string(cei[i].id) + "] no CadieCauldron, mas o item esta sendo vendido no Personal shop dele. Hacker ou Bug.",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1010, 0x5201010));
		}

		END_FIND_ROOM;

		std::vector< stItem > v_remove;
		std::vector< stItem > v_item;
		stItem item{ 0 };
		BuyItem bi{ 0 };

		SysAchievement sys_achieve;

		for (auto i = 0u; i < count; ++i) {
			item.clear();

			item.type = 2;
			item.id = cei[i].id;
			item._typeid = cei[i]._typeid;
			item.qntd = (short)cmb->item_trade.qntd[i] * item_exchange_qntd ;
			item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

			v_remove.push_back(item);
		}

		if (item_manager::removeItem(v_remove, _session) <= 0)
			throw exception("[channel::requestCadieCauldronExchange][Error] problemas ao remover(s) item(ns) do player[UID=" + std::to_string(_session.m_pi.uid) + "]",
				STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 461, 5200462));

		if (cmb->box_random_id > 0) {

			_smp::message_pool::getInstance().push(new message("[channel::requestCadieCauldronExchange][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] vai trocar um Item[Seq="
					+ std::to_string(seq + 1) + "] Random[ID=" + std::to_string(cmb->box_random_id) + "] Box[LootBox]", CL_FILE_LOG_AND_CONSOLE));

			auto cmbr_iff = sIff::getInstance().findCadieMagicBoxRandom(cmb->box_random_id);

			if (cmbr_iff.empty())
				throw exception("[channel::requestCadieCauldronExchange][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] CadieMagicBoxRandom[ID="
						+ std::to_string(cmb->box_random_id) + "] empty", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 456, 5200457));

			Lottery lottery((uint64_t)this);

			for (auto& el : cmbr_iff)
				lottery.push(el.item_random.rate, (size_t)&el);

			auto lc = lottery.spinRoleta();

			if (lc == nullptr)
				throw exception("[channel::requestCadieCauldronExchange][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao conseguiu sortear um item do caddie magic box random[ID="
						+ std::to_string(cmb->box_random_id) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 461, 5200462));

			auto cmbr = (IFF::CadieMagicBoxRandom*)lc->value;

			if (cmbr == nullptr)
				throw exception("[channel::requestCadieCauldronExchange][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] valor retornado do sorteio is invalid(nullptr)",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 462, 5200463));

			_smp::message_pool::getInstance().push(new message("[channel::requestCadieCauldronExchange][CadieMagicBoxRandom::Lotery][Log] Item[INDEX=" + std::to_string(lc->offset[0]) + ", TYPEID="
					+ std::to_string(cmbr->item_random._typeid) + ", QNTD=" + std::to_string(cmbr->item_random.qntd) + "] dropped", CL_FILE_LOG_AND_CONSOLE));

			auto item_random = sIff::getInstance().findCommomItem(cmbr->item_random._typeid);

			if (item_random == nullptr)
				throw exception("[channel::requestCadieCauldronExchange][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "] o item random[TYPEID=" +
						std::to_string(cmbr->item_random._typeid) + "] que esta no IFF_STRUCT do server nao existe no IFF do server, nao foi encontrado. Tem que colocar o item "
						+ "no IFF ou foi colocado o TYPEID errado no IFF de random item CadieCauldronExchange. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 463, 5200464));

			bi.id = -1;
			bi._typeid = cmbr->item_random._typeid;
			bi.qntd = cmbr->item_random.qntd;

			if (item_random->shop.flag_shop.time_shop.active ) {

				if (item_random->shop.flag_shop.time_shop.dia > 0 )
					bi.time = item_random->shop.flag_shop.time_shop.dia;
				else {

					bi.time = (cmb->box_random_id == cadie_cauldron_Hermes_random_id
						|| cmb->box_random_id == cadie_cauldron_Jester_random_id
						|| cmb->box_random_id == cadie_cauldron_Twilight_random_id) ? 10   : 0;
				}

			}else {

				bi.time = (cmb->box_random_id == cadie_cauldron_Hermes_random_id
					|| cmb->box_random_id == cadie_cauldron_Jester_random_id
					|| cmb->box_random_id == cadie_cauldron_Twilight_random_id) ? 10   : 0;
			}

		}else {

			bi.id = -1;
			bi._typeid = cmb->item_receive._typeid;
			bi.qntd = cmb->item_receive.qntd * item_exchange_qntd ;
		}

		item.clear();

		item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1 );

		if (item_manager::isSetItem(item._typeid)) {
			auto v_stItem = item_manager::getItemOfSetItem(_session, item._typeid, false, 1 );

			if (!v_stItem.empty()) {

				for (auto& el : v_stItem)
					if ((sIff::getInstance().IsCanOverlapped(el._typeid) && sIff::getInstance().getItemGroupIdentify(el._typeid) != iff::CAD_ITEM) || !_session.m_pi.ownerItem(el._typeid))
						v_item.push_back(el);

			}else
				throw exception("[channel::requestCadieCauldronExchange][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou trocar um set item que nao tem item, item typeid: " + std::to_string(bi._typeid) + ". Hacker ou bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 461, 0x5200062));
		}else
			v_item.push_back(item);

		if (v_item.empty())
			throw exception("[channel::requestCadieCauldronExchange][Error] problemas ao inicializar o item[TYPEID=" + std::to_string(bi._typeid) + "] para o player[UID="
					+ std::to_string(_session.m_pi.uid) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 459, 5200460));

		auto rai = item_manager::addItem(v_item, _session, 0, 0);

		if (rai.fails.size() > 0 && rai.type != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH)
			throw exception("[channel::requestCadieCauldronExchange][Error] problemas ao adicionar o item[TYPEID=" + std::to_string(bi._typeid) + "] para o player[UID="
					+ std::to_string(_session.m_pi.uid) + "] ", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 460, 5200461));

		if (item._typeid == 0x1A000083 )
			_session.m_pi.cg.partial_ticket += item.STDA_C_ITEM_QNTD;

		item.clear();

		item = v_item[0];

		if (cmb->box_random_id <= 0 && item._typeid != cmb->item_receive._typeid)
			item._typeid = cmb->item_receive._typeid;

		_smp::message_pool::getInstance().push(new message("[channel::requestCadieCauldronExchange][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] trocou item[Seq="
				+ std::to_string(seq + 1) + ", Aba=" + std::to_string(cmb->setor) + ", TYPEID_RCV=" + std::to_string(item._typeid)
				+ ", ID=" + std::to_string(item.id) + ", QNTD(exchange*item_qntd)=" + std::to_string(item.qntd) + "]", CL_FILE_LOG_AND_CONSOLE));

		if (rai.type != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH)
			v_remove.insert(v_remove.end(), v_item.begin(), v_item.end());

		p.init_plain((unsigned short)0x216);

		p.addUint32((const unsigned int)GetSystemTimeAsUnix());
		p.addInt32((uint32_t)v_remove.size());

		for (auto& el : v_remove) {
			p.addUint8(el.type);
			p.addInt32(el._typeid);
			p.addInt32(el.id);
			p.addInt32(el.flag_time);
			p.addUint32(el.stat.qntd_ant);
			p.addUint32(el.stat.qntd_dep);
			p.addUint32((el.STDA_C_ITEM_TIME > 0 ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD));
			p.addZeroByte(25);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x22F);

		p.addUint32(0);

		p.addUint32(seq);

		p.addUint32(1);

		p.addUint32(item._typeid);
		p.addInt32(item.id);
		p.addUint32(item.STDA_C_ITEM_QNTD);
		p.addUint32(item.stat.qntd_dep);
		p.addUint32(item.flag_time);

		packet_func::session_send(p, &_session, 1);

		sys_achieve.incrementCounter(0x6C400082u );

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestCadieCauldronExchange][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x22F);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 5200450);

		packet_func::session_send(p, &_session, 0);
	}
};

void channel::requestCharacterStatsUp(player& _session, packet *_packet) {
	REQUEST_BEGIN("CharacterStatsUp");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("CharacterStatsUp");

		if (_session.m_pi.block_flag.m_flag.stBit.char_mastery)
			throw exception("[channel::requestCharacterStatsUp][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou upar Stats do character, mas ele nao pode. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 9, 0x790001));

		uint32_t stat = _packet->readUint32();

		CharacterInfo ci{ 0 };

		_packet->readBuffer(&ci, sizeof(CharacterInfo));

		auto pCi = _session.m_pi.findCharacterById(ci.id);

		if (pCi == nullptr || pCi->_typeid != ci._typeid)
			throw exception("[channel::requestCharacterStatsUp][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar stat[value="
					+ std::to_string(stat) + "] do Character[TYPEID=" + std::to_string(ci._typeid) + ", ID=" + std::to_string(ci.id) + "] que ele nao possui. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 500, 0x5200501));

		auto character = sIff::getInstance().findCharacter(pCi->_typeid);

		if (character == nullptr)
			throw exception("[channel::requestChracterStatsUp][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar stat[value="
					+ std::to_string(stat) + "] do Character[TYPEID=" + std::to_string(pCi->_typeid) + ", ID="
					+ std::to_string(pCi->id) + "], mas ele nao existe no IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 504, 0x5200505));

		unsigned char value = 0u;

		auto value_part = pCi->getSlotOfStatsFromCharEquipedPartItem(CharacterInfo::Stats(stat));
		auto value_auxpart = pCi->getSlotOfStatsFromCharEquipedAuxPart(CharacterInfo::Stats(stat));
		auto value_set_effect_table = pCi->getSlotOfStatsFromSetEffectTable(CharacterInfo::Stats(stat));
		auto value_card = pCi->getSlotOfStatsFromCharEquipedCard(CharacterInfo::Stats(stat));

		if (value_part == -1 || value_card == -1 || value_auxpart == -1 || value_set_effect_table == -1)
			throw exception("[channel::requestCharacterStatsUp][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "], stat[value="
					+ std::to_string(stat) + "] is invalid. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 501, 0x5200502));

		value += value_part;

		value += value_auxpart;

		value += value_set_effect_table;

		value += value_card;

		if (stat == CharacterInfo::S_POWER)
			value += ((_session.m_pi.mi.level - 1 ) / 5 );

		auto mastery = sIff::getInstance().findCharacterMastery(pCi->_typeid);

		if (mastery.empty())
			throw exception("[channel::requestCharacterStatsUp][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar stat[value="
					+ std::to_string(stat) + "] do Character[TYPEID=" + std::to_string(pCi->_typeid) + ", ID="
					+ std::to_string(pCi->id) + "], mas nao tem o Character Mastery no IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 505, 0x5200506));

		if (mastery.size() < (uint32_t)pCi->mastery)
			throw exception("[channel::requestCharacterStatsUp][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar stat[value="
					+ std::to_string(stat) + "] do Character[TYPEID=" + std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "], mas o CharacterMastery[value="
					+ std::to_string(pCi->mastery) + ", vector_size=" + std::to_string(mastery.size()) + "] do player e invalido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 506, 0x5200507));

		for (auto i = 0u; i < (uint32_t)pCi->mastery; ++i)
			if ((mastery[i].stats - 1) == stat)
				value++;

		if ((pCi->pcl[stat] + 1) > value)
			throw exception("[channel::requestCharacterStatsUp][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar stats[stat="
					+ std::to_string(stat) + "] do Character[TYPEID=" + std::to_string(pCi->_typeid) + ", ID="
					+ std::to_string(pCi->id) + "], mas nao tem mais slot para upar. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 502, 0x5200503));

		uint32_t enchant_typeid = ((iff::ENCHANT << 26) | (stat << 20)) + pCi->pcl[stat];

		auto enchant = sIff::getInstance().findEnchant(enchant_typeid);

		if (enchant == nullptr)
			throw exception("[channel::requestCharacterStatsUp][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar stats[stats="
					+ std::to_string(stat) + "] do Character[ID=" + std::to_string(ci.id) + "], mas nao encontrou o enchant[TYPEID="
					+ std::to_string(enchant_typeid) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 503, 0x5200504));

		_session.m_pi.consomePang(enchant->pang);

		pCi->pcl[stat]++;

		snmdb::NormalManagerDB::getInstance().add(7, new CmdUpdateCharacterPCL(_session.m_pi.uid, *pCi), channel::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[CharacterStats::UPGRADE][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "] upou Character[TYPEID="
				+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "] PCL[C0=" + std::to_string((unsigned short)pCi->pcl[CharacterInfo::S_POWER]) + ", C1="
				+ std::to_string((unsigned short)pCi->pcl[CharacterInfo::S_CONTROL]) + ", C2=" + std::to_string((unsigned short)pCi->pcl[CharacterInfo::S_ACCURACY]) + ", C3="
				+ std::to_string((unsigned short)pCi->pcl[CharacterInfo::S_SPIN]) + ", C4=" + std::to_string((unsigned short)pCi->pcl[CharacterInfo::S_CURVE]) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0xC8);

		p.addUint64(_session.m_pi.ui.pang);
		p.addUint64(enchant->pang);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x216);

		p.addUint32((const unsigned int)GetSystemTimeAsUnix());
		p.addUint32(1);

		p.addUint8(0xC9);
		p.addUint32(pCi->_typeid);
		p.addUint32(pCi->id);
		p.addUint32(0);
		p.addUint32(0);
		p.addUint32(0);
		p.addUint32(0);
		p.addUint16(pCi->pcl[CharacterInfo::S_POWER]);
		p.addUint16(pCi->pcl[CharacterInfo::S_CONTROL]);
		p.addUint16(pCi->pcl[CharacterInfo::S_ACCURACY]);
		p.addUint16(pCi->pcl[CharacterInfo::S_SPIN]);
		p.addUint16(pCi->pcl[CharacterInfo::S_CURVE]);
		p.addZeroByte(15);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x26F);

		p.addUint32(0);

		p.addUint32(stat);

		packet_func::session_send(p, &_session, 1);

		SysAchievement sys_achieve;

		sys_achieve.incrementCounter(0x6C400084u );

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestCharacterStatsUp][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x26F);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5200500);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestCharacterStatsDown(player& _session, packet *_packet) {
	REQUEST_BEGIN("CharacterStatsDown");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("CharacterStatsDown");

		if (_session.m_pi.block_flag.m_flag.stBit.char_mastery)
			throw exception("[channel::requestCharacterStatsDown][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou desupar Stats do character, mas ele nao pode. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 9, 0x790001));

		uint32_t stat = _packet->readUint32();

		CharacterInfo ci{ 0 };

		_packet->readBuffer(&ci, sizeof(CharacterInfo));

		auto pCi = _session.m_pi.findCharacterById(ci.id);

		if (pCi == nullptr || pCi->_typeid != ci._typeid)
			throw exception("[channel::requestCharacterStatsDown][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou desupar o stat[value="
					+ std::to_string(stat) + "] do Character[TYPEID=" + std::to_string(ci._typeid) + ", ID=" + std::to_string(ci.id) + "] que ele nao possui. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 550, 0x5200551));

		auto character = sIff::getInstance().findCharacter(pCi->_typeid);

		if (character == nullptr)
			throw exception("[channel::requestChracterStatsDown][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou desupar stat[value="
					+ std::to_string(stat) + "] do Character[TYPEID=" + std::to_string(pCi->_typeid) + ", ID="
					+ std::to_string(pCi->id) + "], mas ele nao existe no IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 553, 0x5200554));

		if (stat > CharacterInfo::S_CURVE)
			throw exception("[channel::requestCharacterStatsDown][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou desupar um stat[value="
					+ std::to_string(stat) + "] invalido do Character[ID=" + std::to_string(pCi->id) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 551, 0x5200552));

		if ((char)(pCi->pcl[stat] - 1) < 0)
			throw exception("[channel::requestCharacterStatsDown][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou desupar um stat[value="
					+ std::to_string(stat) + "] do Character[ID=" + std::to_string(pCi->id) + "] que ele nao tem mais valor upado. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 552, 0x5200553));

		pCi->pcl[stat]--;

		snmdb::NormalManagerDB::getInstance().add(7, new CmdUpdateCharacterPCL(_session.m_pi.uid, *pCi), channel::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[CharacterStats::DOWNGRADE][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "] desupou Character[TYPEID="
				+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "] PCL[C0=" + std::to_string((unsigned short)pCi->pcl[CharacterInfo::S_POWER]) + ", C1="
				+ std::to_string((unsigned short)pCi->pcl[CharacterInfo::S_CONTROL]) + ", C2=" + std::to_string((unsigned short)pCi->pcl[CharacterInfo::S_ACCURACY]) + ", C3="
				+ std::to_string((unsigned short)pCi->pcl[CharacterInfo::S_SPIN]) + ", C4=" + std::to_string((unsigned short)pCi->pcl[CharacterInfo::S_CURVE]) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32(1);

		p.addUint8(0xC9);
		p.addUint32(pCi->_typeid);
		p.addUint32(pCi->id);
		p.addUint32(0);
		p.addUint32(0);
		p.addUint32(0);
		p.addUint32(0);
		p.addUint16(pCi->pcl[CharacterInfo::S_POWER]);
		p.addUint16(pCi->pcl[CharacterInfo::S_CONTROL]);
		p.addUint16(pCi->pcl[CharacterInfo::S_ACCURACY]);
		p.addUint16(pCi->pcl[CharacterInfo::S_SPIN]);
		p.addUint16(pCi->pcl[CharacterInfo::S_CURVE]);
		p.addZeroByte(15);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x270);

		p.addUint32(0);
		p.addUint32(stat);

		packet_func::session_send(p, &_session, 1);

		SysAchievement sys_achieve;

		sys_achieve.incrementCounter(0x6C400085u );

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestCharacterStatsDown][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x270);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5200550);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestCharacterMasteryExpand(player& _session, packet *_packet) {
	REQUEST_BEGIN("CharacterMasteryExpand");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("CharacterMasteryExpand");

		if (_session.m_pi.block_flag.m_flag.stBit.char_mastery)
			throw exception("[channel::requestCharacterMasteryExpand][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou expandir o character mastery, mas ele nao pode. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 9, 0x790001));

		uint32_t char_typeid = _packet->readUint32();
		int32_t char_id = _packet->readInt32();

		auto pCi = _session.m_pi.findCharacterById(char_id);

		if (pCi == nullptr || pCi->_typeid != char_typeid)
			throw exception("[channel::requestCharacterMasteryExpand][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou expandir Character[TYPEID="
					+ std::to_string(char_typeid) + ", ID=" + std::to_string(char_id) + "] mastery, mas ele nao possui o character. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 650, 0x5200651));

		auto mastery = sIff::getInstance().findCharacterMastery(char_typeid);

		if (mastery.empty())
			throw exception("[channel::requestCharacterMasteryExpand][Error] player[UID=" + std::to_string(_session.m_pi.uid) +"] tentou expandir Character[TYPEID="
					+ std::to_string(char_typeid) + ", ID=" + std::to_string(char_id) + "] mastery, mas nao tem o character mastery no IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 651, 0x5200652));

		if ((uint32_t)(pCi->mastery + 1) > mastery.size())
			throw exception("[channel::requestCharacterMasteryExpand][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou expandir Character[TYPEID="
					+ std::to_string(char_typeid) + ", ID=" + std::to_string(char_id) + "] mastery, mas ele ja expandiu todos que eh permitido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 652, 0x5200653));

		if (mastery[pCi->mastery].seq != (pCi->mastery + 1))
			throw exception("[channel::requestCharacterMasteryExpand][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou expandir Character[TYPEID="
					+ std::to_string(char_typeid) + ", ID=" + std::to_string(char_id) + "] mastery, mas a sequencia do mastery no IFF_STRUCT eh diferente. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 653, 0x5200654));

		if ((char)mastery[pCi->mastery].level > _session.m_pi.mi.level)
			throw exception("[channel::requestCharacterMasteryExpand][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou expandir Character[TYPEID="
					+ std::to_string(char_typeid) + ", ID=" + std::to_string(char_id) + "] mastery, mas nao tem level suficiente[have_lvl="
					+ std::to_string(mastery[pCi->mastery].level) + ", req_lvl=" + std::to_string((short)_session.m_pi.mi.level) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 654, 0x5200655));

		std::vector< stItem > v_item;
		stItem item{ 0 };

		auto& condition = mastery[pCi->mastery].condition;

		for (auto i = 0u; i < (sizeof(IFF::CharacterMastery::Condition::condition) / sizeof(IFF::CharacterMastery::Condition::condition[0])); ++i) {

			if (condition.condition[i] > 0) {

				switch (sIff::getInstance().getItemGroupIdentify(condition.condition[i])) {
				case iff::ITEM:
				{
					auto pWi = _session.m_pi.findWarehouseItemByTypeid(condition.condition[i]);

					if (pWi == nullptr)
						throw exception("[channel::requestCharacterMasteryExpand][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao tem o item da condicao.",
								STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 656, 0x5200657));

					if (pWi->STDA_C_ITEM_QNTD < (short)condition.qntd[i])
						throw exception("[channel::requestCharacterMasteryExpand][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] o item nao tem quantidade suficiente para a condicao",
								STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 657, 0x5200658));

					item.clear();

					item.type = 2;
					item._typeid = condition.condition[i];
					item.id = pWi->id;
					item.qntd = condition.qntd[i];
					item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

					v_item.push_back(item);

					break;
				}
				case iff::QUEST_STUFF:
				{
					auto pQsi = _session.m_pi.mgr_achievement.findQuestStuffByTypeId(condition.condition[i]);

					if (pQsi == nullptr)
						throw exception("[channel::requestCharacterMasteryExpand][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao tem o QuestStuff da condicao",
								STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 658, 0x5200659));

					if (!pQsi->isValid())
						throw exception("[channel::requestCharacterMasteryExpand][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] o counter item da condicao esta inativo",
								STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 659, 0x5200660));

					if (pQsi->counter_item_id == 0 || pQsi->clear_date_unix == 0)
						throw exception("[channel::requestCharacterMasteryExpand][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] o QuestStuff[TYPEID=" + std::to_string(pQsi->_typeid) + "] nao foi concluido",
								STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 660, 0x5200661));

					break;
				}
				default:
					throw exception("[channel::requestCharacterMasteryExpand][Error] Unknown Condition[TYPEID=" + std::to_string(condition.condition[i]) + ", QNTD="
							+ std::to_string(condition.qntd[i]) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 655, 0x5200656));
				}
			}
		}

		if (item_manager::removeItem(v_item, _session) <= 0)
			throw exception("[channel::requestCharacterMasteryExpand][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao conseguiu excluir os item(ns) do player",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 661, 0x5200662));

		pCi->mastery++;

		item.clear();

		item._typeid = pCi->_typeid;
		item.id = pCi->id;
		item.type = 0xCD;
		item.flag = (unsigned char)pCi->mastery;

		v_item.push_back(item);

		snmdb::NormalManagerDB::getInstance().add(9, new CmdUpdateCharacterMastery(_session.m_pi.uid, *pCi), channel::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[CharacterMasteryExpand][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "] expandiu Character[TYPEID="
				+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "] Mastery[value=" + std::to_string(pCi->mastery) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addUint32(el.stat.qntd_ant);
			p.addUint32(el.stat.qntd_dep);
			p.addUint32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addZeroByte(25);
			if (el.type == 0xCD)
				p.addUint32(el.flag);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x26E);

		p.addUint32(0);

		packet_func::session_send(p, &_session, 1);

		SysAchievement sys_achieve;

		sys_achieve.incrementCounter(0x6C4000C3u );

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestCharacterMasteryExpand][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x26E);

		p.addUint32(STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5200650);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestCharacterCardEquip(player& _session, packet *_packet) {
	REQUEST_BEGIN("CharacterCardEquip");

	packet p;

#if defined(__linux__)
#pragma pack(1)
#endif

	struct CardEquip {
		void clear() { memset(this, 0, sizeof(CardEquip)); };
		uint32_t char_typeid;
		int32_t char_id;
		uint32_t card_typeid;
		int32_t card_id;
		uint32_t char_card_slot;
	};

#if defined(__linux__)
#pragma pack()
#endif

	try {

		CHECK_SESSION_IS_AUTHORIZED("CharacterCardEquip");

		if (_session.m_pi.block_flag.m_flag.stBit.char_mastery)
			throw exception("[channel::requestCharacterCardEquip][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou equipar card no character, mas ele nao pode. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 9, 0x790001));

		CardEquip ce{ 0 };
		std::vector< stItem > v_item;
		stItem item{ 0 };

		_packet->readBuffer(&ce, sizeof(CardEquip));

		auto card = sIff::getInstance().findCard(ce.card_typeid);

		if (card == nullptr)
			throw exception("[channel::requestCharacterCardEquip][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
					+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
					+ std::to_string(ce.char_id) + "], mas o card nao existe no IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 756, 0x5200757));

		auto pCi = _session.m_pi.findCharacterById(ce.char_id);

		if (pCi == nullptr || pCi->_typeid != ce.char_typeid)
			throw exception("[channel::requestCharacterCardEquip][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID=" + std::to_string(ce.card_typeid) + ", ID="
					+ std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
					+ std::to_string(ce.char_id) + "], mas ele nao possui esse character. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 750, 0x5200751));

		auto pCardInfo = _session.m_pi.findCardById(ce.card_id);

		if (pCardInfo == nullptr || pCardInfo->_typeid != ce.card_typeid)
			throw exception("[channel::requestCharacterCardEquip][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID=" + std::to_string(ce.card_typeid) + ", ID="
					+ std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID=" + std::to_string(ce.char_id) + "], mas ele nao possui esse card", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 751, 0x5200752));

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r != nullptr && r->checkPersonalShopItem(_session, ce.card_id))
			throw exception("[channel::requestCharacterCardEquip][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar o card[TYPEID=" + std::to_string(ce.card_typeid) + ", ID="
					+ std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID=" + std::to_string(ce.char_id) + "], mas o card esta sendo vendido no Personal shop dele. Hacker ou Bug.",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1010, 0x5201010));

		END_FIND_ROOM;

		item.clear();

		item.type = 2;
		item.id = pCardInfo->id;
		item._typeid = pCardInfo->_typeid;
		item.qntd = 1;
		item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

		v_item.push_back(item);

		if (ce.char_card_slot == 4 || ce.char_card_slot == 8)
			throw exception("[channel::requestCharacterCardEquip][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
					+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
					+ std::to_string(ce.char_id) + "], mas o Slot do card requer um Club Patcher e um outro pacote do cliente. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 752, 0x5200753));

		if (ce.char_card_slot == 7 && !pCi->isEquipedPartSlotThirdCaddieCardSlot())
			throw exception("[channel::requestCharacterCardEquip][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
					+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
					+ std::to_string(ce.char_id) + "], mas o Slot que ele tentou equipar precisa de um Part que libere ele, mas o player nao tem", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 753, 0x5200754));

		switch (ce.char_card_slot) {
		case 1:
		case 2:
		case 3:
		case 4:
			if (sIff::getInstance().getItemSubGroupIdentify22(ce.card_typeid) != 0)
				throw exception("[channel::requestCharacterCardEquip][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
						+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
						+ std::to_string(ce.char_id) + "] no Slot do Character[tipo=0], mas o card eh tipo[value=" + std::to_string(sIff::getInstance().getItemSubGroupIdentify22(ce.card_typeid)) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 755, 0x5200756));

			if (pCi->card_character[(ce.char_card_slot - 1) % 4] != 0)
				throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
						+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
						+ std::to_string(ce.char_id) + "] no Slot[value=" + std::to_string(ce.char_card_slot) + "] mas ja tem card equipado[TYPEID="
						+ std::to_string(pCi->card_character[(ce.char_card_slot - 1) % 4]) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 758, 0x5200759));

			pCi->card_character[(ce.char_card_slot - 1) % 4] = ce.card_typeid;
			break;
		case 5:
		case 6:
		case 7:
		case 8:
			if (sIff::getInstance().getItemSubGroupIdentify22(ce.card_typeid) != 1)
				throw exception("[channel::requestCharacterCardEquip][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
						+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
						+ std::to_string(ce.char_id) + "] no Slot do Caddie[tipo=1], mas o card eh tipo[value=" + std::to_string(sIff::getInstance().getItemSubGroupIdentify22(ce.card_typeid)) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 755, 0x5200756));

			if (pCi->card_caddie[(ce.char_card_slot - 1) % 4] != 0)
				throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
						+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
						+ std::to_string(ce.char_id) + "] no Slot[value=" + std::to_string(ce.char_card_slot) + "] mas ja tem card equipado[TYPEID="
						+ std::to_string(pCi->card_caddie[(ce.char_card_slot - 1) % 4]) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 758, 0x5200759));

			pCi->card_caddie[(ce.char_card_slot - 1) % 4] = ce.card_typeid;
			break;
		case 9:
		case 10:
		case 11:
		case 12:
			if (sIff::getInstance().getItemSubGroupIdentify22(ce.card_typeid) != 5)
				throw exception("[channel::requestCharacterCardEquip][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
						+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
						+ std::to_string(ce.char_id) + "] no Slot do NPC[tipo=5], mas o card eh tipo[value=" + std::to_string(sIff::getInstance().getItemSubGroupIdentify22(ce.card_typeid)) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 755, 0x5200756));

			if (pCi->card_NPC[(ce.char_card_slot - 1) % 4] != 0)
				throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
						+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
						+ std::to_string(ce.char_id) + "] no Slot[value=" + std::to_string(ce.char_card_slot) + "] mas ja tem card equipado[TYPEID="
						+ std::to_string(pCi->card_NPC[(ce.char_card_slot - 1) % 4]) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 758, 0x5200759));

			pCi->card_NPC[(ce.char_card_slot - 1) % 4] = ce.card_typeid;
			break;
		default:
			throw exception("[channel::requestCharacterCardEquip][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
					+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
					+ std::to_string(ce.char_id) + "] em um Slot desconhecido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 754, 0x5200755));
		}

		if (item_manager::removeItem(v_item, _session) <= 0)
			throw exception("[channel::requestCharacterCardEquip][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao conseguiu excluiu/(atualizar) item.",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 757, 0x5200758));

		CardEquipInfoEx cei{ 0 };

		cei.index = -1;
		cei._typeid = ce.card_typeid;
		cei.id = ce.card_id;
		cei.efeito = card->efeito.type;
		cei.efeito_qntd = card->efeito.qntd;
		cei.slot = ce.char_card_slot;
		cei.tipo = sIff::getInstance().getItemSubGroupIdentify22(cei._typeid);
		cei.use_yn = 1;
		cei.parts_typeid = ce.char_typeid;
		cei.parts_id = ce.char_id;

		_session.m_pi.v_cei.push_back(cei);

		item.clear();

		item.type = 0xCB;
		item.id = pCi->id;
		item._typeid = pCi->_typeid;
		item.price = cei._typeid;
		item.type_iff = (unsigned char)cei.slot;

		v_item.push_back(item);

		snmdb::NormalManagerDB::getInstance().add(10, new CmdEquipCard(_session.m_pi.uid, cei, 0 ), channel::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[EquipCard][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] equipou card[TYPEID=" + std::to_string(ce.card_typeid) + "] no Character[TYPEID="
				+ std::to_string(ce.char_typeid) + ", ID=" + std::to_string(ce.char_id) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addUint32(el.stat.qntd_ant);
			p.addUint32(el.stat.qntd_dep);
			p.addInt32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addBuffer(el.c, sizeof(el.c));
			p.addZeroByte(10);
			p.addUint32(el.price);
			p.addUint8(el.type_iff);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x271);

		p.addUint32(0);
		p.addUint32(ce.card_typeid);

		packet_func::session_send(p, &_session, 1);

		SysAchievement sys_achieve;

		sys_achieve.incrementCounter(0x6C400087u );

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestCharacterCardEquip][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x271);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5200750);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestCharacterCardEquipWithPatcher(player& _session, packet* _packet) {
	REQUEST_BEGIN("CharacterCardEquipWithPatcher");

	packet p;

#if defined(__linux__)
#pragma pack(1)
#endif

	struct CardEquip {
		void clear() { memset(this, 0, sizeof(CardEquip)); };
		uint32_t char_typeid;
		int32_t char_id;
		uint32_t card_typeid;
		int32_t card_id;
		uint32_t char_card_slot;
	};

#if defined(__linux__)
#pragma pack()
#endif

	try {

		CHECK_SESSION_IS_AUTHORIZED("CharacterCardEquipWithPatcher");

		if (_session.m_pi.block_flag.m_flag.stBit.char_mastery)
			throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou equipar card no character com Patcher, mas ele nao pode. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 9, 0x790001));

		CardEquip ce{ 0 };
		std::vector< stItem > v_item;
		stItem item{ 0 };

		_packet->readBuffer(&ce, sizeof(CardEquip));

		auto pWi = _session.m_pi.findWarehouseItemByTypeid(CLUB_PATCHER_TYPEID);

		if (pWi == nullptr)
			throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
					+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
					+ std::to_string(ce.char_id) + "] com club Patcher mas ele nao tem o item. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 809, 0x5200810));

		if (pWi->STDA_C_ITEM_QNTD < 1)
			throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=""] tentou equipar card[TYPEID="
					+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
					+ std::to_string(ce.char_id) + "] com club Patcher mas ele nao tem quantidade suficiente do item. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 810, 0x5200811));

		item.clear();

		item.type = 2;
		item.id = pWi->id;
		item._typeid = pWi->_typeid;
		item.qntd = 1;
		item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

		v_item.push_back(item);

		auto card = sIff::getInstance().findCard(ce.card_typeid);

		if (card == nullptr)
			throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
					+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
					+ std::to_string(ce.char_id) + "], mas o card nao existe no IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 806, 0x5200807));

		auto pCi = _session.m_pi.findCharacterById(ce.char_id);

		if (pCi == nullptr || pCi->_typeid != ce.char_typeid)
			throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID=" + std::to_string(ce.card_typeid) + ", ID="
					+ std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
					+ std::to_string(ce.char_id) + "], mas ele nao possui esse character. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 800, 0x5200801));

		auto pCardInfo = _session.m_pi.findCardById(ce.card_id);

		if (pCardInfo == nullptr || pCardInfo->_typeid != ce.card_typeid)
			throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID=" + std::to_string(ce.card_typeid) + ", ID="
					+ std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID=" + std::to_string(ce.char_id) + "], mas ele nao possui esse card", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 801, 0x5200802));

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r != nullptr && r->checkPersonalShopItem(_session, ce.card_id))
			throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar o card[TYPEID=" + std::to_string(ce.card_typeid) + ", ID="
					+ std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID=" + std::to_string(ce.char_id) + "], mas o card esta sendo vendido no Personal shop dele. Hacker ou Bug.",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1010, 0x5201010));

		END_FIND_ROOM;

		item.clear();

		item.type = 2;
		item.id = pCardInfo->id;
		item._typeid = pCardInfo->_typeid;
		item.qntd = 1;
		item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

		v_item.push_back(item);

		if (ce.char_card_slot != 4 && ce.char_card_slot != 8)
			throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
					+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
					+ std::to_string(ce.char_id) + "], so pode equipar o card no slot 4 ou 8, reg_value=" + std::to_string(ce.char_card_slot) + ". Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 802, 0x5200803));

		switch (ce.char_card_slot) {
		case 1:
		case 2:
		case 3:
		case 4:
			if (sIff::getInstance().getItemSubGroupIdentify22(ce.card_typeid) != 0)
				throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
						+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
						+ std::to_string(ce.char_id) + "] no Slot do Character[tipo=0], mas o card eh tipo[value=" + std::to_string(sIff::getInstance().getItemSubGroupIdentify22(ce.card_typeid)) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 805, 0x5200806));

			if (pCi->card_character[(ce.char_card_slot - 1) % 4] != 0)
				throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
						+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
						+ std::to_string(ce.char_id) + "] no Slot[value=" + std::to_string(ce.char_card_slot) + "] mas ja tem card equipado[TYPEID="
						+ std::to_string(pCi->card_character[(ce.char_card_slot - 1) % 4]) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 811, 0x5200812));

			pCi->card_character[(ce.char_card_slot - 1) % 4] = ce.card_typeid;
			break;
		case 5:
		case 6:
		case 7:
		case 8:
			if (sIff::getInstance().getItemSubGroupIdentify22(ce.card_typeid) != 1)
				throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
						+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
						+ std::to_string(ce.char_id) + "] no Slot do Caddie[tipo=1], mas o card eh tipo[value=" + std::to_string(sIff::getInstance().getItemSubGroupIdentify22(ce.card_typeid)) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 805, 0x5200806));

			if (pCi->card_caddie[(ce.char_card_slot - 1) % 4] != 0)
				throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
						+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
						+ std::to_string(ce.char_id) + "] no Slot[value=" + std::to_string(ce.char_card_slot) + "] mas ja tem card equipado[TYPEID="
						+ std::to_string(pCi->card_caddie[(ce.char_card_slot - 1) % 4]) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 811, 0x5200812));

			pCi->card_caddie[(ce.char_card_slot - 1) % 4] = ce.card_typeid;
			break;
		case 9:
		case 10:
		case 11:
		case 12:
			throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
						+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
						+ std::to_string(ce.char_id) + "] em no Slot NPC, mas nao pode, o slot do Club Patcher eh so Character e Caddie. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 803, 0x5200804));
			break;
		default:
			throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou equipar card[TYPEID="
						+ std::to_string(ce.card_typeid) + ", ID=" + std::to_string(ce.card_id) + "] no Character[TYPEID=" + std::to_string(ce.char_typeid) + ", ID="
						+ std::to_string(ce.char_id) + "] em um Slot desconhecido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 804, 0x5200805));
		}

		if (item_manager::removeItem(v_item, _session) <= 0)
			throw exception("[channel::requestCharacterCardEquipWithPatcher][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao conseguiu excluiu/(atualizar) item.",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 807, 0x5200808));

		CardEquipInfoEx cei{ 0 };

		cei.index = -1;
		cei._typeid = ce.card_typeid;
		cei.id = ce.card_id;
		cei.efeito = card->efeito.type;
		cei.efeito_qntd = card->efeito.qntd;
		cei.slot = ce.char_card_slot;
		cei.tipo = sIff::getInstance().getItemSubGroupIdentify22(cei._typeid);
		cei.use_yn = 1;
		cei.parts_typeid = ce.char_typeid;
		cei.parts_id = ce.char_id;

		_session.m_pi.v_cei.push_back(cei);

		item.clear();

		item.type = 0xCB;
		item.id = pCi->id;
		item._typeid = pCi->_typeid;
		item.price = cei._typeid;
		item.type_iff = (unsigned char)cei.slot;

		v_item.push_back(item);

		snmdb::NormalManagerDB::getInstance().add(10, new CmdEquipCard(_session.m_pi.uid, cei, 0 ), channel::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[EquipCardWithPatcher][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] equipou card[TYPEID=" + std::to_string(ce.card_typeid) + "] no Character[TYPEID="
				+ std::to_string(ce.char_typeid) + ", ID=" + std::to_string(ce.char_id) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addUint32(el.stat.qntd_ant);
			p.addUint32(el.stat.qntd_dep);
			p.addInt32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addBuffer(el.c, sizeof(el.c));
			p.addZeroByte(10);
			p.addUint32(el.price);
			p.addUint8(el.type_iff);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x272);

		p.addUint32(0);
		p.addUint32(ce.card_typeid);

		packet_func::session_send(p, &_session, 1);

		SysAchievement sys_achieve;

		sys_achieve.incrementCounter(0x6C400087u );

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestCharacterCardEquipWithPatcher][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x272);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5200800);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestCharacterRemoveCard(player& _session, packet* _packet) {
	REQUEST_BEGIN("CharacterRemoveCard");

	packet p;

#if defined(__linux__)
#pragma pack(1)
#endif

	struct CardRemove {
		void clear() { memset(this, 0, sizeof(CardRemove)); };
		uint32_t char_typeid;
		int32_t char_id;
		uint32_t removedor_typeid;
		int32_t removedor_id;
		uint32_t card_slot;
	};

#if defined(__linux__)
#pragma pack()
#endif

	try {

		CHECK_SESSION_IS_AUTHORIZED("CharacterRemoveCard");

		if (_session.m_pi.block_flag.m_flag.stBit.char_mastery)
			throw exception("[channel::requestCharacterRemoveCard][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou remover card do character, mas ele nao pode. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 9, 0x790001));

		CardRemove cr{ 0 };
		std::vector< stItem > v_item;
		stItem item{ 0 };
		BuyItem bi{ 0 };

		_packet->readBuffer(&cr, sizeof(CardRemove));

		auto pCi = _session.m_pi.findCharacterById(cr.char_id);

		if (pCi == nullptr || pCi->_typeid != cr.char_typeid)
			throw exception("[channel::requestCharacterRemoveCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou remover card[Slot="
					+ std::to_string(cr.card_slot) + "] do Character[TYPEID=" + std::to_string(cr.char_typeid) + ", ID=" + std::to_string(cr.char_id) + "], mas o ele nao possui esse character. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 850, 0x5200851));

		auto pWi = _session.m_pi.findWarehouseItemById(cr.removedor_id);

		if (pWi == nullptr || pWi->_typeid != cr.removedor_typeid)
			throw exception("[channel::requestCharacterRemoveCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou remover card[Slot="
					+ std::to_string(cr.card_slot) + "] do Character[TYPEID=" + std::to_string(cr.char_typeid) + ", ID=" + std::to_string(cr.char_id) + "], mas ele nao possui o removedor[TYPEID="
					+ std::to_string(cr.removedor_typeid) + ", ID=" + std::to_string(cr.removedor_id) + "] de card. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 851, 0x5200852));

		if (pWi->STDA_C_ITEM_QNTD < 1)
			throw exception("[channel::requestCharacterRemoveCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou remover card[Slot="
					+ std::to_string(cr.card_slot) + "] do Character[TYPEID=" + std::to_string(cr.char_typeid) + ", ID=" + std::to_string(cr.char_id) + "], mas ele nao quantidade suficiente do removedor[TYPEID="
					+ std::to_string(cr.removedor_typeid) + ", ID=" + std::to_string(cr.removedor_id) + "] de card. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 854, 0x5200855));

		switch (cr.card_slot) {
		case 1:
		case 2:
		case 3:
		case 4:
			if (pCi->card_character[(cr.card_slot - 1) % 4] == 0)
				throw exception("[channel::requestCharacterRemoveCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou remover card[Slot="
						+ std::to_string(cr.card_slot) + "] do Character[TYPEID=" + std::to_string(cr.char_typeid) + ", ID=" + std::to_string(cr.char_id) + "], mas nao tem nenhum card equipado nesse Slot. Hacker ou Bug",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 853, 0x5200854));

			bi.id = -1;
			bi.qntd = 1;
			bi._typeid = pCi->card_character[(cr.card_slot - 1) % 4];

			pCi->card_character[(cr.card_slot - 1) % 4] = 0;
			break;
		case 5:
		case 6:
		case 7:
		case 8:
			if (pCi->card_caddie[(cr.card_slot - 1) % 4] == 0)
				throw exception("[channel::requestCharacterRemoveCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou remover card[Slot="
					+ std::to_string(cr.card_slot) + "] do Character[TYPEID=" + std::to_string(cr.char_typeid) + ", ID=" + std::to_string(cr.char_id) + "], mas nao tem nenhum card equipado nesse Slot. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 853, 0x5200854));

			bi.id = -1;
			bi.qntd = 1;
			bi._typeid = pCi->card_caddie[(cr.card_slot - 1) % 4];

			pCi->card_caddie[(cr.card_slot - 1) % 4] = 0;
			break;
		case 9:
		case 10:
		case 11:
		case 12:
			if (pCi->card_NPC[(cr.card_slot - 1) % 4] == 0)
				throw exception("[channel::requestCharacterRemoveCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou remover card[Slot="
					+ std::to_string(cr.card_slot) + "] do Character[TYPEID=" + std::to_string(cr.char_typeid) + ", ID=" + std::to_string(cr.char_id) + "], mas nao tem nenhum card equipado nesse Slot. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 853, 0x5200854));

			bi.id = -1;
			bi.qntd = 1;
			bi._typeid = pCi->card_NPC[(cr.card_slot - 1) % 4];

			pCi->card_NPC[(cr.card_slot - 1) % 4] = 0;
			break;
		default:
			throw exception("[channel::requestCharacterRemoveCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou remover card[Slot="
					+ std::to_string(cr.card_slot) + "] do Character[TYPEID=" + std::to_string(cr.char_typeid) + ", ID=" + std::to_string(cr.char_id) + "], mas o slot eh deconhecido. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 852, 0x5200853));
		}

		auto pCei = _session.m_pi.findCardEquipedByTypeid(bi._typeid, cr.char_typeid, cr.card_slot);

		if (pCei == nullptr)
			throw exception("[channel::requestCharacterRemoveCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou remover card[Slot="
					+ std::to_string(cr.card_slot) + "] do Character[TYPEID=" + std::to_string(cr.char_typeid) + ", ID=" + std::to_string(cr.char_id) + "], mas nao tem o card equipado no vector de cards equipado. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x857, 0x5200858));

		item.clear();

		item.type = 2;
		item._typeid = pWi->_typeid;
		item.id = pWi->id;
		item.qntd = 1;
		item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

		if (item_manager::removeItem(item, _session) <= 0)
			throw exception("[channel::requestCharacterRemoveCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao conseguiu excluir/(atualizar qntd) item[TYPEID="
					+ std::to_string(item._typeid) + ", ID=" + std::to_string(item.id) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 858, 0x5200859));

		v_item.push_back(item);

		item.clear();

		item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1 );

		if (item._typeid == 0)
			throw exception("[channel::requestCharacterRemoveCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao conseguiu initializar item[TYPEID="
					+ std::to_string(bi._typeid) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 855, 0x5200856));

		auto rt = item_manager::RetAddItem::T_INIT_VALUE;

		if ((rt = item_manager::addItem(item, _session, 0, 0)) < 0)
			throw exception("[channel::requestCharacterRemoveCard][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao conseguiu adicionar item[TYPEID="
					+ std::to_string(item._typeid) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 856, 0x5200857));

		if (rt != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH)
			v_item.push_back(item);

		item.clear();

		item.type = 0xCB;
		item._typeid = pCi->_typeid;
		item.id = pCi->id;
		item.price = 0;
		item.type_iff = (unsigned char)cr.card_slot;

		v_item.push_back(item);

		snmdb::NormalManagerDB::getInstance().add(11, new CmdRemoveEquipedCard(_session.m_pi.uid, *pCei), channel::SQLDBResponse, this);

		auto it = std::find_if(_session.m_pi.v_cei.begin(), _session.m_pi.v_cei.end(), [&](auto _el) {
			return _el._typeid == bi._typeid && _el.parts_id == cr.char_id && _el.slot == cr.card_slot;
		});

		if (it != _session.m_pi.v_cei.end())
			_session.m_pi.v_cei.erase(it);

		_smp::message_pool::getInstance().push(new message("[DesequipaCard][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] desequipou Card[TYPEID="
				+ std::to_string(bi._typeid) + "] do Character[TYPEID=" + std::to_string(cr.char_typeid) + ", ID=" + std::to_string(cr.char_id) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addUint32(el.stat.qntd_ant);
			p.addUint32(el.stat.qntd_dep);
			p.addUint32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addBuffer(el.c, sizeof(el.c));
			p.addZeroByte(10);
			p.addUint32(el.price);
			p.addUint8(el.type_iff);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x273);

		p.addUint32(0);
		p.addUint32(bi._typeid);

		packet_func::session_send(p, &_session, 1);

		SysAchievement sys_achieve;

		sys_achieve.incrementCounter(0x6C400088u );

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestCharacterRemoveCard][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x273);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5200850);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestClubSetStatsUpdate(player& _session, packet *_packet) {
	REQUEST_BEGIN("ClubSetStatsUpdate");

	packet p;

	try {

		unsigned char opt = _packet->readUint8();
		unsigned char stat = _packet->readUint8();
		int32_t item_id = _packet->readUint32();

		CHECK_SESSION_IS_AUTHORIZED("ClubSetStatsUpdate");

		if (opt == 1 || opt == 3) {

			SysAchievement sys_achieve;

			auto pWi = _session.m_pi.findWarehouseItemById(item_id);

			if (pWi == nullptr)
				throw exception("[channel::requestClubSetStatsUpdate][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou " + std::string(opt == 1 ? "updar" : "desupar") + " stat[value="
					+ std::to_string((unsigned short)stat) + "] do ClubSet[ID=" + std::to_string(item_id) + "] que ele nao possui. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 600, 0x5200601));

			if (stat > CharacterInfo::S_CURVE)
				throw exception("[channel::requestClubSetStatsUpdate][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou " + std::string(opt == 1 ? "updar" : "desupar") + " um stat[value="
						+ std::to_string((unsigned short)stat) + "] que nao existe do ClubSet[ID=" + std::to_string(item_id) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 604, 0x5200605));

			auto clubset = sIff::getInstance().findClubSet(pWi->_typeid);

			if (clubset == nullptr)
				throw exception("[channel::requestClubSetStatsUpdate][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou " + std::string(opt == 1 ? "updar" : "desupar") + " stat[value="
					+ std::to_string((unsigned short)stat) + "] do ClubSet[ID=" + std::to_string(item_id) + "] que nao existe no IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 601, 0x5200602));

			if (opt == 1) {

				if (((clubset->slot[stat] - clubset->c[stat]) + pWi->clubset_workshop.c[stat]) < (pWi->c[stat] + 1))
					throw exception("[channel::requestClubSetStatsUpdate][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar stat[value="
							+ std::to_string((unsigned short)stat) + "] do ClubSet[ID=" + std::to_string(item_id) + "], mas ele ja upou todos os slot's disponiveis. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 602, 0x5200603));

				uint32_t enchant_typeid = (iff::ENCHANT << 26) | (stat << 20) + pWi->c[stat];

				auto enchant = sIff::getInstance().findEnchant(enchant_typeid);

				if (enchant == nullptr)
					throw exception("[channel::requestClubSetStatsUpdate][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar stat[value="
							+ std::to_string((unsigned short)stat) + "] do ClubSet[ID=" + std::to_string(item_id) + "], mas nao tem o enchant[TYPEID="
							+ std::to_string(enchant_typeid) + "] no IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 603, 0x5200604));

				_session.m_pi.consomePang(enchant->pang);

				pWi->c[stat]++;

				snmdb::NormalManagerDB::getInstance().add(8, new CmdUpdateClubSetStats(_session.m_pi.uid, *pWi, enchant->pang), channel::SQLDBResponse, this);

				sys_achieve.incrementCounter(0x6C400084u );

				p.init_plain((unsigned short)0xA5);

				p.addUint8(opt / 2 + 1);
				p.addUint8(opt % 2);
				p.addUint8(stat);
				p.addUint32(item_id);
				p.addUint64(enchant->pang);

				packet_func::session_send(p, &_session, 1);

			}else if (opt == 3) {

				if ((pWi->c[stat] - 1) < 0)
					throw exception("[channel::requestClubSetStatsUpdate][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou desupar stat[value="
							+ std::to_string((unsigned short)stat) + "] do ClubSet[ID=" + std::to_string(item_id) + "], mas ele ja desupou tudo que podia. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 605, 0x5200606));

				pWi->c[stat]--;

				snmdb::NormalManagerDB::getInstance().add(8, new CmdUpdateClubSetStats(_session.m_pi.uid, *pWi, 0), channel::SQLDBResponse, this);

				sys_achieve.incrementCounter(0x6C400085u );

				p.init_plain((unsigned short)0xA5);

				p.addUint8(opt / 2 + 1);
				p.addUint8(opt % 2);
				p.addUint8(stat);
				p.addUint32(item_id);
				p.addUint64(0);

				packet_func::session_send(p, &_session, 1);
			}

			sys_achieve.finish_and_update(_session);

			_smp::message_pool::getInstance().push(new message("[ClubSetUpdateStats::" + std::string((opt == 1) ? "UP" : "DOWN") + "GRADE][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "] " + std::string((opt == 1) ? "upou" : "desupou") + " ClubSet[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "] Stats[C0=" + std::to_string(pWi->c[CharacterInfo::S_POWER]) + ", C1="
					+ std::to_string(pWi->c[CharacterInfo::S_CONTROL]) + ", C2=" + std::to_string(pWi->c[CharacterInfo::S_ACCURACY]) + ", C3="
					+ std::to_string(pWi->c[CharacterInfo::S_SPIN]) + ", C4=" + std::to_string(pWi->c[CharacterInfo::S_CURVE]) + "]", CL_FILE_LOG_AND_CONSOLE));

		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestCharacterStatsUpdate][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0xA5);

		p.addUint8(0);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestTikiShopExchangeItem(player& _session, packet* _packet) {
	REQUEST_BEGIN("TikiShopExchangeItem");

	packet p;

#if defined(__linux__)
#pragma pack(1)
#endif

	struct TikiShopExchangeItem {
		void clear() { memset(this, 0, sizeof(TikiShopExchangeItem)); };
		uint32_t _typeid;
		int32_t id;
		uint32_t qntd;
	};

#if defined(__linux__)
#pragma pack()
#endif

	try {

		CHECK_SESSION_IS_AUTHORIZED("TikiShopExchangeItem");

		_smp::message_pool::getInstance().push(new message("[channel::requestTikiShopExchangeItem][Log][Teste] Player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] iniciou a troca de itens no Tiki Shop.", CL_ONLY_FILE_LOG));

		uint64_t pang = 0ull;
		uint32_t milage = 0u;
		uint32_t tiki_pts = 0u;
		uint32_t bonus = 0u;
		uint32_t bonus_prob = 0u;
		uint32_t bonus_minmax[2]{ 0 };

		std::string s_item = "";
		std::string s_ids = "";

		TikiShopExchangeItem tsei{ 0 };

		std::vector< stItem > v_item;
		stItem item{ 0 };

		SysAchievement sys_achieve;

		item.type = 2;
		item.id = -1;
		item._typeid = 0x1A0002A7;
		item.qntd = 0;
		item.STDA_C_ITEM_QNTD = 0;

		auto pWi = _session.m_pi.findWarehouseItemByTypeid(MILAGE_POINT_TYPEID);

		if (pWi != nullptr) {
			item.id = pWi->id;
			item.qntd = item.STDA_C_ITEM_QNTD = pWi->STDA_C_ITEM_QNTD;
		}

		uint32_t count = _packet->readUint32();

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		for (auto i = 0u; i < count; ++i) {
			tsei.clear();

			_packet->readBuffer(&tsei, sizeof(TikiShopExchangeItem));

			auto item = item_manager::exchangeTikiShop(_session, tsei._typeid, tsei.id, tsei.qntd);

			if (item.empty())
				throw exception("[channel::requestTikiShopExchangeItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar item[TYPEID="
						+ std::to_string(tsei._typeid) + ", ID=" + std::to_string(tsei.id) + ", QNTD=" + std::to_string(tsei.qntd) + "] no Tiki's Shop, mas nao conseguiu inicializar o item. Hacker ou Bug.",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 900, 0x52000901));

			if (r != nullptr && r->checkPersonalShopItem(_session, tsei.id))
				throw exception("[channel::requestTikiShopExchangeItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar item[TYPEID=" + std::to_string(tsei._typeid) + ", ID="
						+ std::to_string(tsei.id) + ", QNTD=" + std::to_string(tsei.qntd) + "] no Tiki's Shop, mas o item esta sendo vendido no Personal shop dele. Hacker ou Bug.",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1010, 0x5201010));

			auto base = sIff::getInstance().findCommomItem(tsei._typeid);

			if (base == nullptr)
				throw exception("[channel::requestTikiShopExchangeItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar item[TYPEID="
						+ std::to_string(tsei._typeid) + ", ID=" + std::to_string(tsei.id) + "] no Tiki's Shop, mas o item nao existe no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 901, 0x5200902));

			if (!base->tiki.isActived())
				throw exception("[channel::requestTikiShopExchangeItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar item[TYPEID="
						+ std::to_string(tsei._typeid) + ", ID=" + std::to_string(tsei.id) + "] no Tiki's Shop, mas o item nao eh valido para ser trocado. Hacker ou Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 904, 0x5200905));

			pang += base->tiki.tiki_pang;

			milage += base->tiki.milage_pts * tsei.qntd;

			bonus_minmax[0] += base->tiki.bonus[0];
			bonus_minmax[1] += base->tiki.bonus[1];
			bonus_prob = base->tiki.bonus_prob;

			v_item.insert(v_item.end(), item.begin(), item.end());

			switch (base->tiki.tipo_tiki_shop) {
			case 1:
				sys_achieve.incrementCounter(0x6C4000BEu );
				break;
			case 2:
				sys_achieve.incrementCounter(0x6C4000BFu );
				break;
			case 3:
				sys_achieve.incrementCounter(0x6C4000C0u );
				break;
			}

			s_ids = "";

			for (auto ii = 0u; ii < item.size(); ++ii)
				s_ids += std::string((ii == 0) ? "" : ", ") + std::to_string(item[ii].id);

			s_item += std::string((i == 0) ? "" : ", ") + "[TYPEID=" + std::to_string(tsei._typeid) + ", ID(s)={" + s_ids + "}, QNTD=" + std::to_string(tsei.qntd) + ", TIPO(Normal, CP, Rare)=" + std::to_string(base->tiki.tipo_tiki_shop) +"]";
		}

		END_FIND_ROOM;

		uint32_t index = (uint32_t)sRandomGen::getInstance().rIbeMt19937_64_chrono() % (bonus_prob * 3 + 1);

		if (index < bonus_prob) {
			bonus = (((uint32_t)sRandomGen::getInstance().rIbeMt19937_64_chrono()) % (bonus_minmax[1] - bonus_minmax[0])) + bonus_minmax[0];

			sys_achieve.incrementCounter(0x6C4000C1u );
		}

		if (item_manager::removeItem(v_item, _session) <= 0)
			throw exception("[channel::requestTikiShopExchangeItem][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou trocar item(ns)(" + s_item + "), mas nao conseguiu deletar ele(s).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 902, 0x5200903));

		if ((milage + item.qntd + bonus) > 1000)
			tiki_pts = (milage + item.qntd + bonus) / 1000;

		item.STDA_C_ITEM_QNTD = (short)((int)((milage + item.qntd + bonus) % 1000) - (int)item.qntd);
		item.qntd = std::abs(item.STDA_C_ITEM_QNTD);

		if (item.STDA_C_ITEM_QNTD != 0) {

			auto rt = item_manager::RetAddItem::T_INIT_VALUE;

			if ((rt = item_manager::addItem(item, _session, 0, 0)) < 0)
				throw exception("[channel::requestTikiShopExchangeItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou adicionar item[TYPEID="
					+ std::to_string(item._typeid) + ", ID=" + std::to_string(item.id) + ", QNTD=" + std::to_string(item.STDA_C_ITEM_QNTD) + "], mas nao conseguiu.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 903, 0x5200904));

			if (rt != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH)
				v_item.push_back(item);

		}

		if (tiki_pts > 0) {

			item.clear();

			item.type = 2;
			item.id = -1;
			item._typeid = 0x1A0002A6;
			item.qntd = 0;
			item.STDA_C_ITEM_QNTD = 0;

			pWi = _session.m_pi.findWarehouseItemByTypeid(TIKI_POINT_TYPEID);

			if (pWi != nullptr)
				item.id = pWi->id;

			item.qntd += tiki_pts;
			item.STDA_C_ITEM_QNTD = (short)item.qntd;

			auto rt = item_manager::RetAddItem::T_INIT_VALUE;

			if ((rt = item_manager::addItem(item, _session, 0, 0)) < 0)
				throw exception("[channel::requestTikiShopExchangeItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou adicionar item[TYPEID="
						+ std::to_string(item._typeid) + ", ID=" + std::to_string(item.id) + ", QNTD=" + std::to_string(item.STDA_C_ITEM_QNTD) + "], mas nao conseguiu.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 903, 0x5200904));

			if (rt != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH)
				v_item.push_back(item);

			sys_achieve.incrementCounter(0x6C4000C2u , tiki_pts);
		}

		_session.m_pi.consomePang(pang);

		_smp::message_pool::getInstance().push(new message("[TikiShopExchangeItem][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] player trocou item(ns)(" + s_item + ") por Milage[value="
				+ std::to_string(milage) +", bonus=" + std::to_string(bonus) + "] Tiki Pts[value=" + std::to_string(tiki_pts) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0xC8);

		p.addUint64(_session.m_pi.ui.pang);
		p.addUint64(pang);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addUint32(el.stat.qntd_ant);
			p.addUint32(el.stat.qntd_dep);
			p.addUint32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addZeroByte(25);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x274);

		p.addUint32(0);
		p.addUint32(milage);
		p.addUint32(bonus);

		packet_func::session_send(p, &_session, 1);

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestTikiShopExchangeItem][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x274);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5200900);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestChangePlayerItemChannel(player& _session, packet* _packet) {
	REQUEST_BEGIN("ChangePlayerItemChannel");

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("Packet 0x0B.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

	unsigned char type = 255 ;

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("ChangePlayerItemChannel");

		type = _packet->readUint8();
		uint32_t item_id;

		int error = 0 ;

		switch (type) {
		case 1:
		{
			CaddieInfoEx *pCi = nullptr;

			if ((item_id = _packet->readUint32()) != 0 && (pCi = _session.m_pi.findCaddieById(item_id)) != nullptr
					&& sIff::getInstance().getItemGroupIdentify(pCi->_typeid) == iff::CADDIE) {

				auto v_it = _session.m_pi.findUpdateItemByTypeidAndId(pCi->_typeid, pCi->id);

				if (!v_it.empty()) {

					for (auto& el : v_it) {

						if (el->second.type == UpdateItem::CADDIE) {

							_session.m_pi.ei.cad_info = nullptr;
							_session.m_pi.ue.caddie_id = 0;

							item_id = 0;

						}else if (el->second.type == UpdateItem::CADDIE_PARTS) {

							pCi->parts_typeid = 0u;
							pCi->parts_end_date_unix = 0;
							pCi->end_parts_date = { 0 };

							_session.m_pi.ei.cad_info = pCi;
							_session.m_pi.ue.caddie_id = item_id;
						}

						_session.m_pi.mp_ui.erase(el);
					}

				}else {

					_session.m_pi.ei.cad_info = pCi;
					_session.m_pi.ue.caddie_id = item_id;

					if (_session.checkCaddieEquiped(_session.m_pi.ue))
						item_id = _session.m_pi.ue.caddie_id;

				}

				snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateCaddieEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

			}else if (_session.m_pi.ue.caddie_id > 0 && _session.m_pi.ei.cad_info != nullptr) {

				error = (item_id == 0) ? 1  : (pCi == nullptr ? 2  : 3 );

				if (error > 1) {
					_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] tentou trocar o Caddie[ID=" + std::to_string(item_id) + "], mas deu Error[VALUE="
							+ std::to_string(error) + "], desequipando o caddie. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
				}

				auto v_it = _session.m_pi.findUpdateItemByTypeidAndId(_session.m_pi.ei.cad_info->_typeid, _session.m_pi.ei.cad_info->id);

				if (!v_it.empty()) {

					for (auto& el : v_it) {

						if (el->second.type == UpdateItem::CADDIE_PARTS) {

							_session.m_pi.ei.cad_info->parts_typeid = 0u;
							_session.m_pi.ei.cad_info->parts_end_date_unix = 0;
							_session.m_pi.ei.cad_info->end_parts_date = { 0 };
						}

						_session.m_pi.mp_ui.erase(el);
					}

				}

				_session.m_pi.ei.cad_info = nullptr;
				_session.m_pi.ue.caddie_id = 0;

				item_id = 0;

				error = 0;

				snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateCaddieEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

			}

			break;
		}
		case 2:
		{
			WarehouseItemEx *pWi = nullptr;

			if ((item_id = _packet->readUint32()) != 0 && (pWi = _session.m_pi.findWarehouseItemByTypeid(item_id)) != nullptr
					&& sIff::getInstance().getItemGroupIdentify(pWi->_typeid) == iff::BALL) {

				_session.m_pi.ei.comet = pWi;
				_session.m_pi.ue.ball_typeid = item_id;

				if (_session.checkBallEquiped(_session.m_pi.ue))
					item_id = _session.m_pi.ue.ball_typeid;

				snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateBallEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

			}else if (item_id == 0) {

				_session.m_pi.ei.comet = nullptr;
				_session.m_pi.ue.ball_typeid = 0l;

				if (_session.checkBallEquiped(_session.m_pi.ue))
					item_id = _session.m_pi.ue.ball_typeid;

				snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateBallEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

			}else {

				error = (pWi == nullptr ? 2  : 3 );

				_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou trocar Ball[TYPEID=" + std::to_string(item_id) + "], mas deu Error[VALUE=" + std::to_string(error)
						+ "], Equipando Ball Padrao. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

				pWi = _session.m_pi.findWarehouseItemByTypeid(DEFAULT_COMET_TYPEID);

				if (pWi != nullptr) {

					_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] tentou trocar a Ball[TYPEID=" + std::to_string(item_id) + "], mas deu Error[VALUE="
							+ std::to_string(error) + "], colocando a Ball Padrao do player. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

					_session.m_pi.ei.comet = pWi;
					item_id = _session.m_pi.ue.ball_typeid = pWi->_typeid;

					error = 0;

					snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateBallEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

				}else {

					_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] tentou trocar a Ball[TYPEID=" + std::to_string(item_id) + "], mas deu Error[VALUE="
							+ std::to_string(error) + "], ele nao tem a Ball Padrao, adiciona a Ball pardrao para ele. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

					BuyItem bi{ 0 };
					stItem item{ 0 };

					bi.id = -1;
					bi._typeid = DEFAULT_COMET_TYPEID;
					bi.qntd = 1;

					item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1 );

					if (item._typeid != 0) {

						if ((item_id = item_manager::addItem(item, _session, 2 , 0)) != item_manager::RetAddItem::T_ERROR) {

							pWi = _session.m_pi.findWarehouseItemById(item_id);

							if (pWi != nullptr) {

								_session.m_pi.ei.comet = pWi;
								_session.m_pi.ue.ball_typeid = pWi->_typeid;

								error = 0;

								snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateBallEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

								p.init_plain((unsigned short)0x216);

								p.addUint32((const uint32_t)GetSystemTimeAsUnix());
								p.addUint32(1);

								p.addUint8(item.type);
								p.addUint32(item._typeid);
								p.addInt32(item.id);
								p.addUint32(item.flag_time);
								p.addBuffer(&item.stat, sizeof(item.stat));
								p.addUint32((item.STDA_C_ITEM_TIME > 0) ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD);
								p.addZeroByte(25);

								packet_func::session_send(p, &_session, 1);

							}else
								_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
										+ "] nao conseguiu achar a Ball[ID=" + std::to_string(item.id) + "] que acabou de adicionar para ele. Bug", CL_FILE_LOG_AND_CONSOLE));

						}else
							_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
									+ "] nao conseguiu adicionar a Ball[TYPEID=" + std::to_string(item._typeid) + "] para ele. Bug", CL_FILE_LOG_AND_CONSOLE));

					}else
						_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
								+ "] nao conseguiu inicializar a Ball[TYPEID=" + std::to_string(bi._typeid) + "] para ele. Bug", CL_FILE_LOG_AND_CONSOLE));
				}
			}

			break;
		}
		case 3:
		{
			WarehouseItemEx *pWi = nullptr;

			if ((item_id = _packet->readUint32()) != 0 && (pWi = _session.m_pi.findWarehouseItemById(item_id)) != nullptr
					&& sIff::getInstance().getItemGroupIdentify(pWi->_typeid) == iff::CLUBSET) {

				auto c_it = _session.m_pi.findUpdateItemByIdAndType(item_id, UpdateItem::WAREHOUSE);

				if (c_it == _session.m_pi.mp_ui.end()) {

					_session.m_pi.ei.clubset = pWi;

					_session.m_pi.ei.csi = { pWi->id, pWi->_typeid, pWi->c };

					IFF::ClubSet *cs = sIff::getInstance().findClubSet(pWi->_typeid);

					if (cs != nullptr) {

						for (auto j = 0u; j < (sizeof(_session.m_pi.ei.csi.enchant_c) / sizeof(short)); ++j)
							_session.m_pi.ei.csi.enchant_c[j] = cs->slot[j] + pWi->clubset_workshop.c[j];

						_session.m_pi.ue.clubset_id = item_id;

						if (_session.checkClubSetEquiped(_session.m_pi.ue))
							item_id = _session.m_pi.ue.clubset_id;

						snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateClubsetEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

					}else {

						error = 5 ;

						_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou Atualizar Clubset[TYPEID="
								+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id)
								+ "] equipado, mas ClubSet Not exists on IFF structure. Equipa o ClubSet padrao. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

						pWi = _session.m_pi.findWarehouseItemByTypeid(AIR_KNIGHT_SET);

						if (pWi != nullptr) {

							_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
									+ "] tentou trocar o ClubSet[ID=" + std::to_string(item_id) + "], mas acabou o tempo do ClubSet[ID="
									+ std::to_string(item_id) + "], colocando o ClubSet Padrao\"CV1\" do player. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

							_session.m_pi.ei.clubset = pWi;
							item_id = _session.m_pi.ue.clubset_id = pWi->id;

							_session.m_pi.ei.csi = { pWi->id, pWi->_typeid, pWi->c };

							IFF::ClubSet *cs = sIff::getInstance().findClubSet(pWi->_typeid);

							if (cs != nullptr)
								for (auto i = 0u; i < (sizeof(_session.m_pi.ei.csi.enchant_c) / sizeof(_session.m_pi.ei.csi.enchant_c[0])); ++i)
									_session.m_pi.ei.csi.enchant_c[i] = cs->slot[i] + pWi->clubset_workshop.c[i];

							error = 0;

							snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateClubsetEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

						}else {

							_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
									+ "] tentou trocar oClubSet[ID=" + std::to_string(item_id) + "], mas acabou o tempo do ClubSet[ID="
									+ std::to_string(item_id) + "], ele nao tem o ClubSet Padrao\"CV1\", adiciona o ClubSet pardrao\"CV1\" para ele. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

							BuyItem bi{ 0 };
							stItem item{ 0 };

							bi.id = -1;
							bi._typeid = AIR_KNIGHT_SET;
							bi.qntd = 1;

							item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1 );

							if (item._typeid != 0) {

								if ((item_id = item_manager::addItem(item, _session, 2 , 0)) != item_manager::RetAddItem::T_ERROR) {

									pWi = _session.m_pi.findWarehouseItemById(item_id);

									if (pWi != nullptr) {

										_session.m_pi.ei.clubset = pWi;
										_session.m_pi.ue.clubset_id = pWi->id;

										_session.m_pi.ei.csi = { pWi->id, pWi->_typeid, pWi->c };

										IFF::ClubSet *cs = sIff::getInstance().findClubSet(pWi->_typeid);

										if (cs != nullptr)
											for (auto i = 0u; i < (sizeof(_session.m_pi.ei.csi.enchant_c) / sizeof(_session.m_pi.ei.csi.enchant_c[0])); ++i)
												_session.m_pi.ei.csi.enchant_c[i] = cs->slot[i] + pWi->clubset_workshop.c[i];

										error = 0;

										snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateClubsetEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

										p.init_plain((unsigned short)0x216);

										p.addUint32((const uint32_t)GetSystemTimeAsUnix());
										p.addUint32(1);

										p.addUint8(item.type);
										p.addUint32(item._typeid);
										p.addInt32(item.id);
										p.addUint32(item.flag_time);
										p.addBuffer(&item.stat, sizeof(item.stat));
										p.addUint32((item.STDA_C_ITEM_TIME > 0) ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD);
										p.addZeroByte(25);

										packet_func::session_send(p, &_session, 1);

									}else
										_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
												+ "] nao conseguiu achar o ClubSet\"CV1\"[ID=" + std::to_string(item.id) + "] que acabou de adicionar para ele. Bug", CL_FILE_LOG_AND_CONSOLE));

								}else
									_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
											+ "] nao conseguiu adicionar o ClubSet[TYPEID=" + std::to_string(item._typeid) + "] para ele. Bug", CL_FILE_LOG_AND_CONSOLE));

							}else
								_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
										+ "] nao conseguiu inicializar o ClubSet[TYPEID=" + std::to_string(bi._typeid) + "] para ele. Bug", CL_FILE_LOG_AND_CONSOLE));
						}
					}

				}else {

					error = 6;

					pWi = _session.m_pi.findWarehouseItemByTypeid(AIR_KNIGHT_SET);

					if (pWi != nullptr) {

						_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
								+ "] tentou trocar o ClubSet[ID=" + std::to_string(item_id) + "], mas acabou o tempo do ClubSet[ID="
								+ std::to_string(item_id) + "], colocando o ClubSet Padrao\"CV1\" do player. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

						_session.m_pi.ei.clubset = pWi;
						item_id = _session.m_pi.ue.clubset_id = pWi->id;

						_session.m_pi.ei.csi = { pWi->id, pWi->_typeid, pWi->c };

						IFF::ClubSet *cs = sIff::getInstance().findClubSet(pWi->_typeid);

						if (cs != nullptr)
							for (auto i = 0u; i < (sizeof(_session.m_pi.ei.csi.enchant_c) / sizeof(_session.m_pi.ei.csi.enchant_c[0])); ++i)
								_session.m_pi.ei.csi.enchant_c[i] = cs->slot[i] + pWi->clubset_workshop.c[i];

						error = 0;

						snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateClubsetEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

					}else {

						_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
								+ "] tentou trocar oClubSet[ID=" + std::to_string(item_id) + "], mas acabou o tempo do ClubSet[ID="
								+ std::to_string(item_id) + "], ele nao tem o ClubSet Padrao\"CV1\", adiciona o ClubSet pardrao\"CV1\" para ele. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

						BuyItem bi{ 0 };
						stItem item{ 0 };

						bi.id = -1;
						bi._typeid = AIR_KNIGHT_SET;
						bi.qntd = 1;

						item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1 );

						if (item._typeid != 0) {

							if ((item_id = item_manager::addItem(item, _session, 2 , 0)) != item_manager::RetAddItem::T_ERROR) {

								pWi = _session.m_pi.findWarehouseItemById(item_id);

								if (pWi != nullptr) {

									_session.m_pi.ei.clubset = pWi;
									_session.m_pi.ue.clubset_id = pWi->id;

									_session.m_pi.ei.csi = { pWi->id, pWi->_typeid, pWi->c };

									IFF::ClubSet *cs = sIff::getInstance().findClubSet(pWi->_typeid);

									if (cs != nullptr)
										for (auto i = 0u; i < (sizeof(_session.m_pi.ei.csi.enchant_c) / sizeof(_session.m_pi.ei.csi.enchant_c[0])); ++i)
											_session.m_pi.ei.csi.enchant_c[i] = cs->slot[i] + pWi->clubset_workshop.c[i];

									error = 0;

									snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateClubsetEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

									p.init_plain((unsigned short)0x216);

									p.addUint32((const uint32_t)GetSystemTimeAsUnix());
									p.addUint32(1);

									p.addUint8(item.type);
									p.addUint32(item._typeid);
									p.addInt32(item.id);
									p.addUint32(item.flag_time);
									p.addBuffer(&item.stat, sizeof(item.stat));
									p.addUint32((item.STDA_C_ITEM_TIME > 0) ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD);
									p.addZeroByte(25);

									packet_func::session_send(p, &_session, 1);

								}else
									_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
											+ "] nao conseguiu achar o ClubSet\"CV1\"[ID=" + std::to_string(item.id) + "] que acabou de adicionar para ele. Bug", CL_FILE_LOG_AND_CONSOLE));

							}else
								_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
										+ "] nao conseguiu adicionar o ClubSet[TYPEID=" + std::to_string(item._typeid) + "] para ele. Bug", CL_FILE_LOG_AND_CONSOLE));

						}else
							_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
									+ "] nao conseguiu inicializar o ClubSet[TYPEID=" + std::to_string(bi._typeid) + "] para ele. Bug", CL_FILE_LOG_AND_CONSOLE));
					}
				}

			}else {

				error = (item_id == 0) ? 1  : (pWi == nullptr ? 2  : 3 );

				pWi = _session.m_pi.findWarehouseItemByTypeid(AIR_KNIGHT_SET);

				if (pWi != nullptr) {

					_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] tentou trocar o ClubSet[ID=" + std::to_string(item_id) + "], mas deu Error[VALUE="
							+ std::to_string(error) + "], colocando o ClubSet Padrao\"CV1\" do player. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

					_session.m_pi.ei.clubset = pWi;
					item_id = _session.m_pi.ue.clubset_id = pWi->id;

					_session.m_pi.ei.csi = { pWi->id, pWi->_typeid, pWi->c };

					IFF::ClubSet *cs = sIff::getInstance().findClubSet(pWi->_typeid);

					if (cs != nullptr)
						for (auto i = 0u; i < (sizeof(_session.m_pi.ei.csi.enchant_c) / sizeof(_session.m_pi.ei.csi.enchant_c[0])); ++i)
							_session.m_pi.ei.csi.enchant_c[i] = cs->slot[i] + pWi->clubset_workshop.c[i];

					error = 0;

					snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateClubsetEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

				}else {

					_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] tentou trocar o ClubSet[ID=" + std::to_string(item_id) + "], mas deu Error[VALUE="
							+ std::to_string(error) + "], ele nao tem o ClubSet Padrao\"CV1\", adiciona o ClubSet pardrao\"CV1\" para ele. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

					BuyItem bi{ 0 };
					stItem item{ 0 };

					bi.id = -1;
					bi._typeid = AIR_KNIGHT_SET;
					bi.qntd = 1;

					item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1 );

					if (item._typeid != 0) {

						if ((item_id = item_manager::addItem(item, _session, 2 , 0)) != item_manager::RetAddItem::T_ERROR) {

							pWi = _session.m_pi.findWarehouseItemById(item_id);

							if (pWi != nullptr) {

								_session.m_pi.ei.clubset = pWi;
								_session.m_pi.ue.clubset_id = pWi->id;

								_session.m_pi.ei.csi = { pWi->id, pWi->_typeid, pWi->c };

								IFF::ClubSet *cs = sIff::getInstance().findClubSet(pWi->_typeid);

								if (cs != nullptr)
									for (auto i = 0u; i < (sizeof(_session.m_pi.ei.csi.enchant_c) / sizeof(_session.m_pi.ei.csi.enchant_c[0])); ++i)
										_session.m_pi.ei.csi.enchant_c[i] = cs->slot[i] + pWi->clubset_workshop.c[i];

								error = 0;

								snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateClubsetEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

								p.init_plain((unsigned short)0x216);

								p.addUint32((const uint32_t)GetSystemTimeAsUnix());
								p.addUint32(1);

								p.addUint8(item.type);
								p.addUint32(item._typeid);
								p.addInt32(item.id);
								p.addUint32(item.flag_time);
								p.addBuffer(&item.stat, sizeof(item.stat));
								p.addUint32((item.STDA_C_ITEM_TIME > 0) ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD);
								p.addZeroByte(25);

								packet_func::session_send(p, &_session, 1);

							}else
								_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
										+ "] nao conseguiu achar o ClubSet\"CV1\"[ID=" + std::to_string(item.id) + "] que acabou de adicionar para ele. Bug", CL_FILE_LOG_AND_CONSOLE));

						}else
							_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
									+ "] nao conseguiu adicionar o ClubSet[TYPEID=" + std::to_string(item._typeid) + "] para ele. Bug", CL_FILE_LOG_AND_CONSOLE));

					}else
						_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
								+ "] nao conseguiu inicializar o ClubSet[TYPEID=" + std::to_string(bi._typeid) + "] para ele. Bug", CL_FILE_LOG_AND_CONSOLE));
				}
			}

			break;
		}
		case 4:
		{
			CharacterInfo *pCe = nullptr;

			if ((item_id = _packet->readUint32()) != 0 && (pCe = _session.m_pi.findCharacterById(item_id)) != nullptr
					&& sIff::getInstance().getItemGroupIdentify(pCe->_typeid) == iff::CHARACTER) {

				_session.m_pi.ei.char_info = pCe;
				_session.m_pi.ue.character_id = item_id;

				snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateCharacterEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

				updatePlayerInfo(_session);

			}else {

				error = (item_id == 0) ? 1  : (pCe == nullptr ? 2  : 3 );

				if (_session.m_pi.mp_ce.size() > 0) {

					_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] tentou trocar o Character[ID=" + std::to_string(item_id) + "], mas deu Error[VALUE="
							+ std::to_string(error) + "], colocando o primeiro character do player. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

					_session.m_pi.ei.char_info = &_session.m_pi.mp_ce.begin()->second;
					item_id = _session.m_pi.ue.character_id = _session.m_pi.ei.char_info->id;

					error = 0;

					snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateCharacterEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

					updatePlayerInfo(_session);

				}else {

					_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] tentou trocar o Character[ID=" + std::to_string(item_id) + "], mas deu Error[VALUE="
							+ std::to_string(error) + "], ele nao tem nenhum character, adiciona o Nuri para ele. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

					BuyItem bi{ 0 };
					stItem item{ 0 };

					bi.id = -1;
					bi._typeid = iff::CHARACTER << 26;
					bi.qntd = 1;

					item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1 );

					if (item._typeid != 0) {

						if ((item_id = item_manager::addItem(item, _session, 2 , 0)) != item_manager::RetAddItem::T_ERROR) {

							error = 0;

							p.init_plain((unsigned short)0x216);

							p.addUint32((const uint32_t)GetSystemTimeAsUnix());
							p.addUint32(1);

							p.addUint8(item.type);
							p.addUint32(item._typeid);
							p.addInt32(item.id);
							p.addUint32(item.flag_time);
							p.addBuffer(&item.stat, sizeof(item.stat));
							p.addUint32((item.STDA_C_ITEM_TIME > 0) ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD);
							p.addZeroByte(25);

							packet_func::session_send(p, &_session, 1);

							updatePlayerInfo(_session);

						}else
							_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
									+ "] nao conseguiu adicionar o Character[TYPEID=" + std::to_string(item._typeid) + "] para ele. Bug", CL_FILE_LOG_AND_CONSOLE));

					}else
						_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
								+ "] nao conseguiu inicializar o Character[TYPEID=" + std::to_string(bi._typeid) + "] para ele. Bug", CL_FILE_LOG_AND_CONSOLE));
				}
			}

			break;
		}
		case 5:
		{
			MascotInfoEx *pMi = nullptr;

			if ((item_id = _packet->readUint32()) != 0) {

				if ((pMi = _session.m_pi.findMascotById(item_id)) != nullptr && sIff::getInstance().getItemGroupIdentify(pMi->_typeid) == iff::MASCOT) {

					auto m_it = _session.m_pi.findUpdateItemByIdAndType(_session.m_pi.ue.mascot_id, UpdateItem::MASCOT);

					if (m_it != _session.m_pi.mp_ui.end()) {

						_session.m_pi.ei.mascot_info = nullptr;
						_session.m_pi.ue.mascot_id = 0;

						item_id = 0;

					}else {

						_session.m_pi.ei.mascot_info = pMi;
						_session.m_pi.ue.mascot_id = item_id;

						if (_session.checkMascotEquiped(_session.m_pi.ue))
							item_id = _session.m_pi.ue.mascot_id;

					}

					snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateMascotEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

				}else {

					error = (item_id == 0) ? 1  : (pMi == nullptr ? 2  : 3 );

					if (error > 1) {
						_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][Log][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
								+ "] tentou trocar o Mascot[ID=" + std::to_string(item_id) + "], mas deu Error[VALUE="
								+ std::to_string(error) + "], desequipando o Mascot. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
					}

					_session.m_pi.ei.mascot_info = nullptr;
					_session.m_pi.ue.mascot_id = 0;

					item_id = 0;

					snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateMascotEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);
				}

			}else if (_session.m_pi.ue.mascot_id > 0 && _session.m_pi.ei.mascot_info != nullptr) {

				_session.m_pi.ei.mascot_info = nullptr;
				_session.m_pi.ue.mascot_id = 0;

				item_id = 0;

				snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateMascotEquiped(_session.m_pi.uid, item_id), channel::SQLDBResponse, this);

			}

			break;
		}
		default:
			throw exception("[channel::requestChangePlayerItemChannel][Error] type desconhecido.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 13, 1));
		}

		updatePlayerInfo(_session);

		packet_func::pacote04B(p, &_session, type, error);
		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemChannel][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		packet_func::pacote04B(p, &_session, type, (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 1 ));
		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestChangePlayerItemRoom(player& _session, packet* _packet) {
	REQUEST_BEGIN("ChangePlayerItemRoom");

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("Packet 0x0C.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("ChangePlayerItemRoom");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr && _session.m_pi.lobby != (unsigned char)~0u)
			return;

		if (r == nullptr)
			throw exception("[channel::requestChangePlayerItemRoom][Error] o player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] Channel[ID=" + std::to_string((unsigned short)m_ci.id) + "] nao esta[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero)
					+ "] em nenhuma sala. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 17, 1));

		ChangePlayerItemRoom cpir{ 0 };

		cpir.type = ChangePlayerItemRoom::TYPE_CHANGE(_packet->readUint8());

		switch (cpir.type) {
			case ChangePlayerItemRoom::TYPE_CHANGE::TC_CADDIE:
				cpir.caddie = _packet->readUint32();
				break;
			case ChangePlayerItemRoom::TYPE_CHANGE::TC_BALL:
				cpir.ball = _packet->readUint32();
				break;
			case ChangePlayerItemRoom::TYPE_CHANGE::TC_CLUBSET:
				cpir.clubset = _packet->readUint32();
				break;
			case ChangePlayerItemRoom::TYPE_CHANGE::TC_CHARACTER:
				cpir.character = _packet->readUint32();
				break;
			case ChangePlayerItemRoom::TYPE_CHANGE::TC_MASCOT:
				cpir.mascot = _packet->readUint32();
				break;
			case ChangePlayerItemRoom::TYPE_CHANGE::TC_ITEM_EFFECT_LOUNGE:
				_packet->readBuffer(&cpir.effect_lounge, sizeof(cpir.effect_lounge));
				break;
			case ChangePlayerItemRoom::TYPE_CHANGE::TC_ALL:
				cpir.character = _packet->readUint32();
				cpir.caddie = _packet->readUint32();
				cpir.clubset = _packet->readUint32();
				cpir.ball = _packet->readUint32();
				break;
		}

		r->requestChangePlayerItemRoom(_session, cpir);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestChangePlayerItemRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		packet_func::pacote04B(p, &_session, 255, (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 1 ));
		packet_func::session_send(p, &_session, 0);
	}
};

void channel::requestDeleteActiveItem(player& _session, packet *_packet) {
	REQUEST_BEGIN("DeleteActiveItem");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("DeleteActiveItem");

		uint32_t _typeid = _packet->readUint32();
		uint32_t qntd = _packet->readUint32();

		if (sIff::getInstance().getItemGroupIdentify(_typeid) != iff::ITEM)
			throw exception("[channel::requestDeleteActiveItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou excluir um item[TYPEID="
					+ std::to_string(_typeid) + "] que nao pode ser excluido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 703, 0x5200704));

		auto iff_item = sIff::getInstance().findItem(_typeid);

		if (iff_item == nullptr)
			throw exception("[channel::requestDeleteActiveItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou excluir um item[TYPEID="
					+ std::to_string(_typeid) + "] que nao pode ser excluido, por que ele nao tem no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 704, 0x5200705));

		if (sIff::getInstance().IsItemEquipable(_typeid) && iff_item->shop.flag_shop.uFlagShop.stFlagShop.is_cash)
			throw exception("[channel::requestDeleteActiveItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou excluir um item[TYPEID="
					+ std::to_string(_typeid) + "] que nao pode ser excluido, por que ele eh um item equipavel de cash(cookie). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 705, 0x5200706));

		if (!sIff::getInstance().IsItemEquipable(_typeid) && !(iff_item->shop.flag_shop.uFlagShop.stFlagShop.is_giftable && iff_item->c[0] > 0))
			throw exception("[channel::requestDeleteActiveItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou excluir um item[TYPEID="
					+ std::to_string(_typeid) + "] que nao pode ser excluido, por que ele eh um passive item que nao tem a condicao(giftable) e a quantidade no C[0] para deletar esse item. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 706, 0x5200707));

		auto pWi = _session.m_pi.findWarehouseItemByTypeid(_typeid);

		if (pWi == nullptr)
			throw exception("[channel::requestDeleteActiveItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou excluir item[TYPEID="
					+ std::to_string(_typeid) + "] que ele nao possui. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 700, 0x5200701));

		if (pWi->STDA_C_ITEM_QNTD < (short)qntd)
			throw exception("[channel::requestDeleteActiveItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou excluir item[TYPEID="
					+ std::to_string(_typeid) + "] mas ele nao tem quantidade suficiente[have_qntd=" + std::to_string(pWi->STDA_C_ITEM_QNTD) + ", req_qntd="
					+ std::to_string(qntd) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 701, 0x5200702));

		stItem item{ 0 };

		item.type = 2;
		item.id = pWi->id;
		item._typeid = pWi->_typeid;
		item.qntd = qntd;
		item.STDA_C_ITEM_QNTD = (short)qntd * -1;

		if (item_manager::removeItem(item, _session) <= 0)
			throw exception("[channel::requestDeleteActiveItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao conseguiu excluir item[TYPEID="
					+ std::to_string(_typeid) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 702, 0x5200703));

		_smp::message_pool::getInstance().push(new message("[DeleteActiveItem][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "] excluiu/(Atualizou qntd) item[TYPEID="
				+ std::to_string(pWi->_typeid) + ", QNTD=" + std::to_string(qntd) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0xC5);

		p.addUint8(1);

		p.addUint32(pWi->_typeid);
		p.addUint32(qntd);
		p.addInt32(pWi->id);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestDeleteActiveItem][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0xC5);

		p.addInt8(-1);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestClubSetWorkShopTransferMasteryPts(player& _session, packet *_packet) {
	REQUEST_BEGIN("ClubSetWorkShopTransferMasteryPts");

	packet p;

#if defined(__linux__)
#pragma pack(1)
#endif

	struct ClubSetWorkShopTransferMasteryPts {
		void clear() { memset(this, 0, sizeof(ClubSetWorkShopTransferMasteryPts)); };
		uint32_t UCIM_chip_typeid;
		int32_t clubset[2];
		uint32_t qntd;
	};

#if defined(__linux__)
#pragma pack()
#endif

	try {

		ClubSetWorkShopTransferMasteryPts tmp{ 0 };

		std::vector< stItemEx > v_item;
		stItemEx item{ 0 };

		_packet->readBuffer(&tmp, sizeof(ClubSetWorkShopTransferMasteryPts));

		CHECK_SESSION_IS_AUTHORIZED("ClubSetWorkShopTransferMasteryPts");

		auto pUCIM_chip = _session.m_pi.findWarehouseItemByTypeid(tmp.UCIM_chip_typeid);

		if (pUCIM_chip == nullptr)
			throw exception("[channel::requestClubSetWorkShopTransferMasteryPts][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou transferir mastery pts do ClubSet[ID="
					+ std::to_string(tmp.clubset[0]) + "] para ClubSet[ID=" + std::to_string(tmp.clubset[1]) + "], mas ele nao tem UCIM Chip[TYPEID="
					+ std::to_string(tmp.UCIM_chip_typeid) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 103, 0x5300104));

		if (pUCIM_chip->STDA_C_ITEM_QNTD < (short)tmp.qntd)
			throw exception("[channel::requestClubSetWorkShopTransferMasteryPts][Error] player[UID=""] tentou transferir mastery pts do ClubSet[ID="
					+ std::to_string(tmp.clubset[0]) + "] para ClubSet[ID=" + std::to_string(tmp.clubset[1]) + "], mas ele nao tem quantidade suficiente de UCIM Chip[TYPEID="
					+ std::to_string(tmp.UCIM_chip_typeid) + ", QNTD=" + std::to_string(pUCIM_chip->STDA_C_ITEM_QNTD) + ", request=" + std::to_string(tmp.qntd) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 104, 0x5300105));

		auto pClub_src = _session.m_pi.findWarehouseItemById(tmp.clubset[0]);

		if (pClub_src == nullptr)
			throw exception("[channel::requestClubSetWorkShopTransferMasteryPts][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou transferir mastery pts do ClubSet[ID="
					+ std::to_string(tmp.clubset[0]) + "] mas o player nao tem esse ClubSet. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 100, 0x5300101));

		auto pClub_dst = _session.m_pi.findWarehouseItemById(tmp.clubset[1]);

		if (pClub_dst == nullptr)
			throw exception("[channel::requestClubSetWorkShopTransferMasteryPts][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou transferir mastery pts para o ClubSet[ID="
					+ std::to_string(tmp.clubset[1]) + "] mas o player nao tem esse ClubSet. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 100, 0x5300101));

		if (sIff::getInstance().findClubSet(pClub_src->_typeid) == nullptr)
			throw exception("[channel::requestClubSetWorkShopTransferMasteryPts][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou transferir mastery pts do ClubSet[TYPEID="
					+ std::to_string(pClub_src->_typeid) + ", ID=" + std::to_string(pClub_src->id) + "] mas o clubset nao existe no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 101, 0x5300102));

		auto clubset = sIff::getInstance().findClubSet(pClub_dst->_typeid);

		if (clubset == nullptr)
			throw exception("[channel::requestClubSetWorkShopTransferMasteryPts][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou transferir mastery pts para o ClubSet[TYPEID="
					+ std::to_string(pClub_src->_typeid) + ", ID=" + std::to_string(pClub_src->id) + "] mas o clubset nao existe no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 101, 0x5300102));

		if (clubset->work_shop.tipo == -1)
			throw exception("[channel::requestClubSetWorkShopTransferMasteryPts][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou transferir mastery pts para o ClubSet[TYPEID="
					+ std::to_string(pClub_dst->_typeid) + ", ID=" + std::to_string(pClub_dst->id) + "] mas ele nao pode receber mastery de outros ClubSet. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 102, 0x5300103));

		if (pClub_dst->clubset_workshop.calcRank((short*)clubset->slot) == 5 )
			throw exception("[channel::requestClubSetWorkShopTransferMasteryPts][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou transferir mastery pts para o ClubSet[TYPEID="
					+ std::to_string(pClub_dst->_typeid) + ", ID=" + std::to_string(pClub_dst->id) + "] mas o ClubSet eh Rank S nao pode transferir Mastery Pts mais para ele. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 107, 0x5300108));

		if ((tmp.qntd * 300) > (uint32_t)pClub_src->clubset_workshop.mastery && (uint32_t)((pClub_src->clubset_workshop.mastery % 300 == 0) ? pClub_src->clubset_workshop.mastery / 300 : pClub_src->clubset_workshop.mastery / 300 + 1) > tmp.qntd)
			throw exception("[channel::requestClubSetWorkShopTransferMasteryPts][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou transferir mastery pts do ClubSet[ID="
					+ std::to_string(tmp.clubset[0]) + "] para ClubSet[ID=" + std::to_string(tmp.clubset[1]) + "], mas ele tentou usar UCIM chip mais que o necessario. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 105, 0x5300106));

		uint32_t mastery = ((tmp.qntd * 300) > (uint32_t)pClub_src->clubset_workshop.mastery) ? pClub_src->clubset_workshop.mastery : tmp.qntd * 300;

		pClub_dst->clubset_workshop.mastery += mastery;
		pClub_src->clubset_workshop.mastery -= mastery;

		item.clear();

		item.type = 2;
		item.id = pUCIM_chip->id;
		item._typeid = pUCIM_chip->_typeid;
		item.qntd = tmp.qntd;
		item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

		if (item_manager::removeItem(item, _session) <= 0)
			throw exception("[channel::requestClubSetWorkShopTransferMasteryPts][Error] player[UID=""] tentou remover item[TYPEID=" + std::to_string(item._typeid) + ", ID="
					+ std::to_string(item.id) + "] mas nao conseguiu", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 106, 0x5300107));

		v_item.push_back(item);

		item.clear();

		item.type = 0xCC;
		item.id = pClub_src->id;
		item._typeid = pClub_src->_typeid;
#if defined(_WIN32)
		memcpy_s(item.clubset_workshop.c, sizeof(item.clubset_workshop.c), pClub_src->clubset_workshop.c, sizeof(item.clubset_workshop.c));
#elif defined(__linux__)
		memcpy(item.clubset_workshop.c, pClub_src->clubset_workshop.c, sizeof(item.clubset_workshop.c));
#endif
		item.clubset_workshop.level = (char)pClub_src->clubset_workshop.level;
		item.clubset_workshop.mastery = pClub_src->clubset_workshop.mastery;
		item.clubset_workshop.rank = pClub_src->clubset_workshop.rank;
		item.clubset_workshop.recovery = pClub_src->clubset_workshop.recovery_pts;

		v_item.push_back(item);

		item.clear();

		item.type = 0xCC;
		item.id = pClub_dst->id;
		item._typeid = pClub_dst->_typeid;
#if defined(_WIN32)
		memcpy_s(item.clubset_workshop.c, sizeof(item.clubset_workshop.c), pClub_dst->clubset_workshop.c, sizeof(item.clubset_workshop.c));
#elif defined(__linux__)
		memcpy(item.clubset_workshop.c, pClub_dst->clubset_workshop.c, sizeof(item.clubset_workshop.c));
#endif
		item.clubset_workshop.level = (char)pClub_dst->clubset_workshop.level;
		item.clubset_workshop.mastery = pClub_dst->clubset_workshop.mastery;
		item.clubset_workshop.rank = pClub_dst->clubset_workshop.rank;
		item.clubset_workshop.recovery = pClub_dst->clubset_workshop.recovery_pts;

		v_item.push_back(item);

		snmdb::NormalManagerDB::getInstance().add(12, new CmdUpdateClubSetWorkshop(_session.m_pi.uid, *pClub_src, CmdUpdateClubSetWorkshop::F_TRANSFER_MASTERY_PTS), channel::SQLDBResponse, this);
		snmdb::NormalManagerDB::getInstance().add(12, new CmdUpdateClubSetWorkshop(_session.m_pi.uid, *pClub_dst, CmdUpdateClubSetWorkshop::F_TRANSFER_MASTERY_PTS), channel::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[ClubSet Workshop::TransferMasteryPts][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] transferiu mastery pts[value=" + std::to_string(mastery) + "] do ClubSet[TYPEID="
				+ std::to_string(pClub_src->_typeid) + ", ID=" + std::to_string(pClub_src->id) + "] para o ClubSet[TYPEID=" + std::to_string(pClub_dst->_typeid) + ", ID=" + std::to_string(pClub_dst->id) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addBuffer(&el.stat, sizeof(el.stat));
			p.addUint32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addZeroByte(25);
			if (el.type == 0xCC)
				p.addBuffer(&el.clubset_workshop, sizeof(el.clubset_workshop));
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x245);

		p.addUint32(0);

		packet_func::session_send(p, &_session, 1);

		SysAchievement sys_achieve;

		sys_achieve.incrementCounter(0x6C4000A5u );

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestClubSetWorkShopTransferMasteryPts][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x245);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5300100);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestClubSetWorkShopRecoveryPts(player& _session, packet *_packet) {
	REQUEST_BEGIN("ClubSetWorkShopRecoveryPts");

	packet p;

	try {

		uint32_t item_typeid = _packet->readUint32();
		int32_t clubset_id = _packet->readInt32();

		CHECK_SESSION_IS_AUTHORIZED("ClubSetWorkShopRecoveryPts");

		std::vector< stItemEx > v_item;
		stItemEx item{ 0 };

		auto pWi = _session.m_pi.findWarehouseItemByTypeid(item_typeid);

		if (pWi == nullptr)
			throw exception("[requestClubSetWorkShopRecoveryPts][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou recuperar os pontos de recuperacao do ClubSet[ID="
					+ std::to_string(clubset_id) + "], mas ele nao tem o item[TYPEID=" + std::to_string(item_typeid) + "] para isso. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 150, 0x5300151));

		if (pWi->STDA_C_ITEM_QNTD < 1)
			throw exception("[requestClubSetWorkShopRecoveryPts][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou recuperar os pontos de recuperacao do ClubSet[ID="
					+ std::to_string(clubset_id) + "], mas ele nao tem quantidade do item[TYPEID=" + std::to_string(pWi->_typeid) + ", ID="
					+ std::to_string(pWi->id) + ", QNTD=" + std::to_string(pWi->STDA_C_ITEM_QNTD) + ", request=1]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 151, 0x5300152));

		auto pClub = _session.m_pi.findWarehouseItemById(clubset_id);

		if (pClub == nullptr)
			throw exception("[requestClubSetWorkShopRecoveryPts][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou recuperar os pontos de recuperacao do ClubSet[ID="
					+ std::to_string(clubset_id) + "], mas ele nao tem o ClubSet. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 152, 0x5300153));

		auto clubset = sIff::getInstance().findClubSet(pClub->_typeid);

		if (clubset == nullptr)
			throw exception("[requestClubSetWorkShopRecoveryPts][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou recuperar os pontos de recuperacao do ClubSet[ID="
					+ std::to_string(clubset_id) + "], mas nao tem esse ClubSet no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 153, 0x5300154));

		if (clubset->work_shop.tipo == -1)
			throw exception("[requestClubSetWorkShopRecoveryPts][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou recuperar os pontos de recuperacao do ClubSet[ID="
					+ std::to_string(clubset_id) + "], mas esse ClubSet nao pode Recuperar o Recovery Pts. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 154, 0x5300155));

		if (pClub->clubset_workshop.recovery_pts == 0)
			throw exception("[requestClubSetWorkShopRecoveryPts][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou recuperar os pontos de recuperacao do ClubSet[ID="
					+ std::to_string(clubset_id) + "], mas o ClubSet do player ja foi recuperado. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 156, 0x5300157));

		item.clear();

		item.type = 2;
		item.id = pWi->id;
		item._typeid = pWi->_typeid;
		item.qntd = 1;
		item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

		if (item_manager::removeItem(item, _session) <= 0)
			throw exception("[requestClubSetWorkShopRecoveryPts][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou recuperar os pontos de recuperacao do ClubSet[ID="
					+ std::to_string(clubset_id) + "], mas nao conseguiu remover item[TYPEID=" + std::to_string(item._typeid) + ", ID=" + std::to_string(item.id) + "]",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 155, 0x5300156));

		v_item.push_back(item);

		pClub->clubset_workshop.recovery_pts = 0;

		item.clear();

		item.type = 0xCC;
		item.id = pClub->id;
		item._typeid = pClub->_typeid;
#if defined(_WIN32)
		memcpy_s(item.clubset_workshop.c, sizeof(item.clubset_workshop.c), pClub->clubset_workshop.c, sizeof(item.clubset_workshop.c));
#elif defined(__linux__)
		memcpy(item.clubset_workshop.c, pClub->clubset_workshop.c, sizeof(item.clubset_workshop.c));
#endif
		item.clubset_workshop.level = (char)pClub->clubset_workshop.level;
		item.clubset_workshop.mastery = pClub->clubset_workshop.mastery;
		item.clubset_workshop.rank = pClub->clubset_workshop.rank;
		item.clubset_workshop.recovery = pClub->clubset_workshop.recovery_pts;

		v_item.push_back(item);

		snmdb::NormalManagerDB::getInstance().add(12, new CmdUpdateClubSetWorkshop(_session.m_pi.uid, *pClub, CmdUpdateClubSetWorkshop::F_R_RECOVERY_PTS), channel::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[ClubSet WorkShop::RecoveryPts][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] recuperou os pontos do ClubSet[TYPEID="
				+ std::to_string(pClub->_typeid) + ", ID=" + std::to_string(pClub->id) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addBuffer(&el.stat, sizeof(el.stat));
			p.addUint32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addZeroByte(25);
			if (el.type == 0xCC)
				p.addBuffer(&el.clubset_workshop, sizeof(el.clubset_workshop));
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x246);

		p.addUint32(0);

		packet_func::session_send(p, &_session, 1);

		SysAchievement sys_achieve;

		sys_achieve.incrementCounter(0x6C4000A6 );

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestClubSetWorkShopRecoveryPts][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x246);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5300150);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestClubSetWorkShopUpLevel(player& _session, packet* _packet) {
	REQUEST_BEGIN("ClubSetWorkShopUpLevel");

	packet p;

#if defined(__linux__)
#pragma pack(1)
#endif

	struct CWUpLevel {
		void clear() { memset(this, 0, sizeof(CWUpLevel)); };
		uint32_t item_typeid;
		unsigned short qntd;
		int32_t clubset_id;
	};

	struct ProbCardExtra {
		unsigned char active : 1, : 0;
		unsigned char stat;
		uint32_t prob;
	};

#if defined(__linux__)
#pragma pack()
#endif

	try {

		CWUpLevel cwul{ 0 };
		stItem item{ 0 };
		ProbCardExtra pce{ 0 };

		size_t stat = 0;

		_packet->readBuffer(&cwul, sizeof(CWUpLevel));

		CHECK_SESSION_IS_AUTHORIZED("ClubSetWorkShopUpLevel");

		switch (sIff::getInstance().getItemGroupIdentify(cwul.item_typeid)) {
		case iff::ITEM:
		{
			auto pWi = _session.m_pi.findWarehouseItemByTypeid(cwul.item_typeid);

			if (pWi == nullptr)
				throw exception("[channel::requestClubSetWorkShopUpLevel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar ClubSet[ID="
						+ std::to_string(cwul.clubset_id) + "] Level, mas ele nao tem o item[TYPEID=" + std::to_string(cwul.item_typeid) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 201, 0x5300202));

			if (pWi->STDA_C_ITEM_QNTD < (short)cwul.qntd)
				throw exception("[channel::requestClubSetWorkShopUpLevel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar ClubSet[ID="
						+ std::to_string(cwul.clubset_id) + "] Level, mas ele nao tem quantidade suficiente do item[TYPEID=" + std::to_string(pWi->_typeid) + ", ID="
						+ std::to_string(pWi->id) + ", QNTD=" + std::to_string(pWi->STDA_C_ITEM_QNTD) + ", request=" + std::to_string(cwul.qntd) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 202, 0x5300203));

			if (!sIff::getInstance().findItem(pWi->_typeid))
				throw exception("[channel::requestClubSetWorkShopUpLevel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar ClubSet[ID="
						+ std::to_string(cwul.clubset_id) + "] Level, mas o Item nao existe no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 203, 0x5300204));

			item.clear();

			item.type = 2;
			item.id = pWi->id;
			item._typeid = pWi->_typeid;
			item.qntd = cwul.qntd;
			item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

			break;
		}
		case iff::CARD:
		{
			auto pCi = _session.m_pi.findCardByTypeid(cwul.item_typeid);

			if (pCi == nullptr)
				throw exception("[channel::requestClubSetWorkShopUpLevel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar ClubSet[ID="
						+ std::to_string(cwul.clubset_id) + "] Level, mas ele nao tem o item[TYPEID=" + std::to_string(cwul.item_typeid) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 201, 0x5300202));

			if (pCi->qntd < (short)cwul.qntd)
				throw exception("[channel::requestClubSetWorkShopUpLevel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar ClubSet[ID="
						+ std::to_string(cwul.clubset_id) + "] Level, mas ele nao tem quantidade suficiente do Card[TYPEID=" + std::to_string(pCi->_typeid) + ", ID="
						+ std::to_string(pCi->id) + ", QNTD=" + std::to_string(pCi->qntd) + ", request=" + std::to_string(cwul.qntd) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 202, 0x5300203));

			if (!sIff::getInstance().findCard(pCi->_typeid))
				throw exception("[channel::requestClubSetWorkShopUpLevel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar ClubSet[ID="
						+ std::to_string(cwul.clubset_id) + "] Level, mas o Card nao existe no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 203, 0x5300204));

			item.clear();

			item.type = 2;
			item.id = pCi->id;
			item._typeid = pCi->_typeid;
			item.qntd = cwul.qntd;
			item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

			if (cwul.qntd > 0) {
				pce.active = 1;
				pce.stat = (cwul.qntd == 1 ? 2 : (cwul.qntd == 2 ? 4 : (cwul.qntd == 3 ? 0 : (cwul.qntd == 4 ? 3 : (cwul.qntd == 5 ? 1 : 2)))));
				pce.prob = cwul.qntd * 200;
			}

			break;
		}
		default:
			throw exception("[channel::requestClubSetWorkShopUpLevel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar ClubSet[ID="
					+ std::to_string(cwul.clubset_id) + "] Level, mas o item[TYPEID=" + std::to_string(cwul.item_typeid) + "], usado para upar eh desconhecido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 200, 0x5300201));
		}

		auto pClub = _session.m_pi.findWarehouseItemById(cwul.clubset_id);

		if (pClub == nullptr)
			throw exception("[channel::requestClubSetWorkShopUpLevel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar ClubSet[ID="
					+ std::to_string(cwul.clubset_id) + "] Level, mas o ele nao tem o ClubSet. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 204, 0x5300205));

		if (pClub->clubset_workshop.rank == -1)
			throw exception("[channel::requestClubSetWorkShopUpLevel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar ClubSet[ID="
					+ std::to_string(cwul.clubset_id) + "] Level, mas ClubSet dele ja upou todos os levels permitidos. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 209, 0x5300210));

		auto clubset = sIff::getInstance().findClubSet(pClub->_typeid);

		if (clubset == nullptr)
			throw exception("[channel::requestClubSetWorkShopUpLevel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar ClubSet[TYPEID="
					+ std::to_string(pClub->_typeid) + ", ID=" + std::to_string(pClub->id) + "] Level, mas o ClubSet nao existe no IFF_STRUCT so Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 205, 0x5300206));

		if (clubset->work_shop.tipo == -1)
			throw exception("[channel::requestClubSetWorkShopUpLevel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar ClubSet[ID="
					+ std::to_string(cwul.clubset_id) + "] Level, mas esse ClubSet nao pose upar Level. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 206, 0x5300207));

		auto level_up_limit = sIff::getInstance().findClubSetWorkShopLevelUpLimit(clubset->work_shop.tipo);
		auto level_up_prob = sIff::getInstance().findClubSetWorkShopLevelUpProb(clubset->work_shop.tipo);

		if (level_up_limit.empty() || level_up_prob == nullptr)
			throw exception("[channel::requestClubSetWorkShopUpLevel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar ClubSet[ID="
					+ std::to_string(cwul.clubset_id) + "] Level, IFF_STRUCT level_up_limit or level_up_prob not found. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 208, 0x5300209));

		auto limit = std::find_if(level_up_limit.begin(), level_up_limit.end(), [&](auto& el) {
			return el.rank == pClub->clubset_workshop.calcRank((short*)clubset->slot);
		});

		if (limit == level_up_limit.end())
			throw exception("[channel::requestClubSetWorkShopUpLevel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar ClubSet[ID="
					+ std::to_string(cwul.clubset_id) + "] Level, nao encontrou o level para upar no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 210, 0x5300211));

		Lottery lottery((uint64_t)this);

		for (auto ii = 0u; ii < (sizeof(limit->c) / sizeof(limit->c[0])); ++ii)
			if (limit->c[ii] > (unsigned short)(pClub->clubset_workshop.c[ii] + clubset->slot[ii]))
				lottery.push(level_up_prob->c[ii] + (pce.active && ii == pce.stat ? pce.prob : 0), ii);

		auto lc = lottery.spinRoleta();

		if (lc != nullptr)
			stat = lc->value;

		if (item_manager::removeItem(item, _session) <= 0)
			throw exception("[channel::requestClubSetWorkShopUpLevel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar ClubSet[ID="
					+ std::to_string(cwul.clubset_id) + "] Level, nao conseguiu remover item[TYPEID=" + std::to_string(item._typeid) + ", ID=" + std::to_string(item.id) + "]",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 207, 0x5300208));

		_session.m_pi.cwlul.clubset_id = pClub->id;
		_session.m_pi.cwlul.stat = (uint32_t)stat;

		pClub->clubset_workshop.c[stat]++;

		snmdb::NormalManagerDB::getInstance().add(12, new CmdUpdateClubSetWorkshop(_session.m_pi.uid, *pClub, CmdUpdateClubSetWorkshop::F_UP_LEVEL), channel::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[ClubSetWorkshop::UpLevel][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] upou Level[stat="
				+ std::to_string(stat) + "] do ClubSet[TYPEID=" + std::to_string(pClub->_typeid) + ", ID=" + std::to_string(pClub->id) + "] agora aguardando resposta do cliente, se quer ou nao esse stat.", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32(1);

		p.addUint8(item.type);
		p.addUint32(item._typeid);
		p.addInt32(item.id);
		p.addUint32(item.flag_time);
		p.addBuffer(&item.stat, sizeof(item.stat));
		p.addUint32((item.STDA_C_ITEM_TIME > 0) ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD);
		p.addZeroByte(25);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x23D);

		p.addUint32(0);
		p.addUint32((uint32_t)stat);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestClubSetWorkShopUpLevel][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x23D);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5300200);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestClubSetWorkShopUpLevelConfirm(player& _session, packet *_packet) {
	REQUEST_BEGIN("ClubSetWorkShopUpLevelConfirm");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("ClubSetWorkShopUpLevelConfirm");

		stItemEx item{ 0 };

		auto pClub = _session.m_pi.findWarehouseItemById(_session.m_pi.cwlul.clubset_id);

		if (pClub == nullptr)
			throw exception("[channel::requestClubSetWorkShopUpLevelConfirm][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou confirma o Up Level[stat="
					+ std::to_string(_session.m_pi.cwlul.stat) + "] do ClubSet[ID=" + std::to_string(_session.m_pi.cwlul.clubset_id) + "], mas ele nao tem esse ClubSet. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 300, 0x5300301));

		if (_session.m_pi.cwlul.stat > 4)
			throw exception("[channel::requestClubSetWorkShopUpLevelConfirm][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou confirma o Up Level[stat="
					+ std::to_string(_session.m_pi.cwlul.stat) + "] do ClubSet[ID=" + std::to_string(_session.m_pi.cwlul.clubset_id) + "], mas o stat eh desconhecido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 302, 0x5300303));

		auto clubset = sIff::getInstance().findClubSet(pClub->_typeid);

		if (clubset == nullptr)
			throw exception("[channel::requestClubSetWorkShopUpLevelConfirm][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou confirma o Up Level[stat="
					+ std::to_string(_session.m_pi.cwlul.stat) + "] do ClubSet[ID=" + std::to_string(_session.m_pi.cwlul.clubset_id) + "], mas nao existe esse ClubSet no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 301, 0x5300302));

		item.clear();

		item.type = 0xCC;
		item.id = pClub->id;
		item._typeid = pClub->_typeid;
#if defined(_WIN32)
		memcpy_s(item.clubset_workshop.c, sizeof(item.clubset_workshop.c), pClub->clubset_workshop.c, sizeof(item.clubset_workshop.c));
#elif defined(__linux__)
		memcpy(item.clubset_workshop.c, pClub->clubset_workshop.c, sizeof(item.clubset_workshop.c));
#endif
		item.clubset_workshop.level = (char)pClub->clubset_workshop.level;
		item.clubset_workshop.mastery = pClub->clubset_workshop.mastery;
		item.clubset_workshop.rank = pClub->clubset_workshop.rank;
		item.clubset_workshop.recovery = pClub->clubset_workshop.recovery_pts;

		_smp::message_pool::getInstance().push(new message("[ClubSetWorkshop::UpLevelConfirm][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] confirmou o Up Level[stat="
				+ std::to_string(_session.m_pi.cwlul.stat) + "] do ClubSet[TYPEID=" + std::to_string(pClub->_typeid) + ", ID=" + std::to_string(pClub->id) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32(1);

		p.addUint8(item.type);
		p.addUint32(item._typeid);
		p.addInt32(item.id);
		p.addUint32(item.flag_time);
		p.addBuffer(&item.stat, sizeof(item.stat));
		p.addUint32((item.STDA_C_ITEM_TIME > 0) ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD);
		p.addZeroByte(25);
		if (item.type == 0xCC)
			p.addBuffer(&item.clubset_workshop, sizeof(item.clubset_workshop));

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x23E);

		p.addUint32(0);
		p.addUint32(_session.m_pi.cwlul.stat);
		p.addInt32(_session.m_pi.cwlul.clubset_id);

		packet_func::session_send(p, &_session, 1);

		SysAchievement sys_achieve;

		sys_achieve.incrementCounter(0x6C4000A2u );

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestClubSetWorkShopUpLevelConfirm][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x23E);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5300300);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestClubSetWorkShopUpLevelCancel(player& _session, packet* _packet) {
	REQUEST_BEGIN("ClubSetWorkShopUpLevelCancel");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("ClubSetWorkShopUpLevelCancel");

		stItemEx item{ 0 };

		auto pClub = _session.m_pi.findWarehouseItemById(_session.m_pi.cwlul.clubset_id);

		if (pClub == nullptr)
			throw exception("[channel::requestClubSetWorkShopUpLevelCancel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou cancelar o up level[stat="
					+ std::to_string(_session.m_pi.cwlul.stat) + "] do ClubSet[ID=" + std::to_string(_session.m_pi.cwlul.clubset_id) + "], mas ele nao tem esse ClubSet. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 250, 0x5300251));

		if (_session.m_pi.cwlul.stat > 4)
			throw exception("[channel::requestClubSetWorkShopUpLevelCancel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou cancelar o up level[stat="
				+ std::to_string(_session.m_pi.cwlul.stat) + "] do ClubSet[ID=" + std::to_string(_session.m_pi.cwlul.clubset_id) + "], mas o stat eh desconhecido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 251, 0x5300252));

		auto clubset = sIff::getInstance().findClubSet(pClub->_typeid);

		if (clubset == nullptr)
			throw exception("[channel::requestClubSetWorkShopUpLevelCancel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou cancelar o up level[stat="
					+ std::to_string(_session.m_pi.cwlul.stat) + "] do ClubSet[ID=" + std::to_string(_session.m_pi.cwlul.clubset_id) + "], mas o ClubSet nao existe no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 252, 0x5300253));

		if (clubset->work_shop.total_recovery <= (uint32_t)pClub->clubset_workshop.recovery_pts)
			throw exception("[channel::requestClubSetWorkShopUpLevelCancel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou cancelar o up level[stat="
					+ std::to_string(_session.m_pi.cwlul.stat) + "] do ClubSet[ID=" + std::to_string(_session.m_pi.cwlul.clubset_id) + "], mas o ele nao pode mais cancelar ja gastou todos os seus pts de recovery[ClubSet_IFF_recovery="
					+ std::to_string(clubset->work_shop.total_recovery) + ", ClubSet_recovery=" + std::to_string(pClub->clubset_workshop.recovery_pts) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 253, 0x5300254));

		pClub->clubset_workshop.c[_session.m_pi.cwlul.stat]--;
		pClub->clubset_workshop.recovery_pts++;

		item.clear();

		item.type = 0xCC;
		item.id = pClub->id;
		item._typeid = pClub->_typeid;
#if defined(_WIN32)
		memcpy_s(item.clubset_workshop.c, sizeof(item.clubset_workshop.c), pClub->clubset_workshop.c, sizeof(item.clubset_workshop.c));
#elif defined(__linux__)
		memcpy(item.clubset_workshop.c, pClub->clubset_workshop.c, sizeof(item.clubset_workshop.c));
#endif
		item.clubset_workshop.level = (char)pClub->clubset_workshop.level;
		item.clubset_workshop.mastery = pClub->clubset_workshop.mastery;
		item.clubset_workshop.rank = pClub->clubset_workshop.rank;
		item.clubset_workshop.recovery = pClub->clubset_workshop.recovery_pts;

		snmdb::NormalManagerDB::getInstance().add(12, new CmdUpdateClubSetWorkshop(_session.m_pi.uid, *pClub, CmdUpdateClubSetWorkshop::F_UP_LEVEL_CANCEL), channel::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[ClubSetWorkshop::UpLevelCancel][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] cancelou o Up Level[stat="
				+ std::to_string(_session.m_pi.cwlul.stat) + "] do ClubSet[TYPEID=" + std::to_string(pClub->_typeid) + ", ID=" + std::to_string(pClub->id) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32(1);

		p.addUint8(item.type);
		p.addUint32(item._typeid);
		p.addInt32(item.id);
		p.addUint32(item.flag_time);
		p.addBuffer(&item.stat, sizeof(item.stat));
		p.addUint32((item.STDA_C_ITEM_TIME > 0) ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD);
		p.addZeroByte(25);
		if (item.type == 0xCC)
			p.addBuffer(&item.clubset_workshop, sizeof(item.clubset_workshop));

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x23F);

		p.addUint32(0);
		p.addInt32(_session.m_pi.cwlul.clubset_id);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestClubSetWorkShopUpLevelCancel][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x23F);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5300250);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestClubSetWorkShopUpRank(player& _session, packet *_packet) {
	REQUEST_BEGIN("ClubSetWorkShopUpRank");

	packet p;

#if defined(__linux__)
#pragma pack(1)
#endif

	struct CWUpRank {
		void clear() { memset(this, 0, sizeof(CWUpRank)); };
		uint32_t item_typeid;
		unsigned short qntd;
		int32_t clubset_id;
	};

#if defined(__linux__)
#pragma pack()
#endif

	try {

		CWUpRank cwup{ 0 };
		std::vector< stItemEx > v_item;
		stItemEx item{ 0 };

		uint32_t stat = 2 ;

		_packet->readBuffer(&cwup, sizeof(CWUpRank));

		CHECK_SESSION_IS_AUTHORIZED("ClubSetWorkShopUpRank");

		if (cwup.qntd > 0) {
			auto pCi = _session.m_pi.findCardByTypeid(cwup.item_typeid);

			if (pCi == nullptr)
				throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[ID="
						+ std::to_string(cwup.clubset_id) + "], mas ele nao tem o Card[TYPEID=" + std::to_string(cwup.item_typeid) + "] para upar o rank. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 350, 0x5300351));

			if (pCi->qntd < (int)cwup.qntd)
				throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[ID="
						+ std::to_string(cwup.clubset_id) + "], mas ele nao tem quantidade suficiente de Card[TYPEID=" + std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + ", QNTD="
						+ std::to_string(pCi->qntd) + ", request=" + std::to_string(cwup.qntd) + "] para upar o rank. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 351, 0x5300532));

			item.clear();

			item.type = 2;
			item.id = pCi->id;
			item._typeid = pCi->_typeid;
			item.qntd = cwup.qntd;
			item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;
		}

		auto pClub = _session.m_pi.findWarehouseItemById(cwup.clubset_id);

		if (pClub == nullptr)
			throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[ID="
					+ std::to_string(cwup.clubset_id) + "], mas ele nao tem esse ClubSet. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 352, 0x5300353));

		auto clubset = sIff::getInstance().findClubSet(pClub->_typeid);

		if (clubset == nullptr)
			throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[TYPEID=" + std::to_string(pClub->_typeid) + ", ID="
					+ std::to_string(pClub->id) + "], mas esse ClubSet nao existe no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 353, 0x5300354));

		if (clubset->work_shop.tipo == -1)
			throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[TYPEID=" + std::to_string(pClub->_typeid) + ", ID="
					+ std::to_string(pClub->id) + "], mas esse ClubSet nao eh permitido upar de rank. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 354, 0x5300355));

		auto level_up_limit = sIff::getInstance().findClubSetWorkShopLevelUpLimit(clubset->work_shop.tipo);

		if (level_up_limit.empty())
			throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[TYPEID=" + std::to_string(pClub->_typeid) + ", ID="
					+ std::to_string(pClub->id) + "], IFF_STRUCT level_up_limit not found. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 208, 0x5300209));

		auto limit = std::find_if(level_up_limit.begin(), level_up_limit.end(), [&](auto& el) {
			return el.rank == (pClub->clubset_workshop.calcRank((short*)clubset->slot) + 1) ;
		});

		if (limit == level_up_limit.end())
			throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[TYPEID=" + std::to_string(pClub->_typeid) + ", ID="
					+ std::to_string(pClub->id) + "], nao encontrou o level para upar no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 210, 0x5300211));

		if (cwup.qntd > 4)
			throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[TYPEID=" + std::to_string(pClub->_typeid) + ", ID="
					+ std::to_string(pClub->id) + "], mas a quantidade de card[TYPEID=" + std::to_string(cwup.item_typeid) + ", QNTD=" + std::to_string(cwup.qntd) + "] eh desconhecida", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 355, 0x5300356));

		stat = (cwup.qntd == 0 ? 2  : (cwup.qntd == 1 ? 4  : (cwup.qntd == 2 ? 0  : (cwup.qntd == 3 ? 3  : (cwup.qntd == 4 ? 1  : 2 )))));

		if (limit->c[stat] <= (pClub->clubset_workshop.c[stat] + clubset->slot[stat]))
			throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[TYPEID=" + std::to_string(pClub->_typeid) + ", ID="
					+ std::to_string(pClub->id) + "], mas o player nao pode mais upar esse stat[value=" + std::to_string(stat) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 357, 0x5300358));

		auto rank_up_exp = sIff::getInstance().findClubSetWorkShopRankExp(clubset->work_shop.tipo_rank_s);

		if (rank_up_exp == nullptr)
			throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[TYPEID=" + std::to_string(pClub->_typeid) + ", ID="
					+ std::to_string(pClub->id) + "], mas nao encontrou o Rank Up Exp no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 358, 0x5300359));

		int32_t rank = pClub->clubset_workshop.calcRank((short*)clubset->slot) + 1 ;

		if (rank == -1)
			throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[TYPEID=" + std::to_string(pClub->_typeid) + ", ID="
					+ std::to_string(pClub->id) + "], mas pegou um rank desconhecido, System Error", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 360, 0x5300361));

		if ((uint32_t)pClub->clubset_workshop.mastery < rank_up_exp->rank[(uint32_t)rank])
			throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou upar rank[rank=" + std::to_string(rank) + "] do ClubSet[TYPEID=" + std::to_string(pClub->_typeid) + ", ID="
					+ std::to_string(pClub->id) + "], mas ele nao tem mastery[value=" + std::to_string(pClub->clubset_workshop.mastery) + ", request="
					+ std::to_string(rank_up_exp->rank[(uint32_t)rank]) + "] suficiente para upar o rank. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 359, 0x5300360));

		if (item._typeid != 0) {
			if (item_manager::removeItem(item, _session) <= 0)
				throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID = " + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[TYPEID = " + std::to_string(pClub->_typeid) + ", ID = "
						+ std::to_string(pClub->id) + "], mas nao conseguiu remover Card[TYPEID=" + std::to_string(item._typeid) + ", ID=" + std::to_string(item.id) + ", QNTD="
						+ std::to_string(item.qntd) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 356, 0x5300357));

			v_item.push_back(item);
		}

		if (rank == 5 ) {
			if (clubset->work_shop.rank_s_stat > 4)
				throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID = " + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[TYPEID = " + std::to_string(pClub->_typeid) + ", ID = "
						+ std::to_string(pClub->id) + "], mas o ClubSet Stat[value=" + std::to_string(clubset->work_shop.rank_s_stat) + "] Rank S do IFF_STRUCT do Server eh invalido. System Error",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 361, 0x5300362));

			pClub->clubset_workshop.c[clubset->work_shop.rank_s_stat]++;
		}

		pClub->clubset_workshop.c[stat]++;
		pClub->clubset_workshop.recovery_pts = 0;
		pClub->clubset_workshop.rank = rank;
		pClub->clubset_workshop.level = pClub->clubset_workshop.calcLevel((short*)clubset->slot);
		pClub->clubset_workshop.mastery -= rank_up_exp->rank[(uint32_t)rank];

		item.clear();

		item.type = 0xCC;
		item.id = pClub->id;
		item._typeid = pClub->_typeid;
#if defined(_WIN32)
		memcpy_s(item.clubset_workshop.c, sizeof(item.clubset_workshop.c), pClub->clubset_workshop.c, sizeof(item.clubset_workshop.c));
#elif defined(__linux__)
		memcpy(item.clubset_workshop.c, pClub->clubset_workshop.c, sizeof(item.clubset_workshop.c));
#endif
		item.clubset_workshop.level = (char)pClub->clubset_workshop.level;
		item.clubset_workshop.mastery = pClub->clubset_workshop.mastery;
		item.clubset_workshop.rank = pClub->clubset_workshop.rank;
		item.clubset_workshop.recovery = pClub->clubset_workshop.recovery_pts;

		v_item.push_back(item);

		snmdb::NormalManagerDB::getInstance().add(12, new CmdUpdateClubSetWorkshop(_session.m_pi.uid, *pClub, CmdUpdateClubSetWorkshop::F_UP_RANK), channel::SQLDBResponse, this);

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addBuffer(&el.stat, sizeof(el.stat));
			p.addUint32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addZeroByte(25);
			if (el.type == 0xCC)
				p.addBuffer(&el.clubset_workshop, sizeof(el.clubset_workshop));
		}

		packet_func::session_send(p, &_session, 1);

		if (clubset->work_shop.flag_transformar) {
			Lottery lottery((uint64_t)this);

			lottery.push(250, 0x1000005D);
			lottery.push(250, 0x1000005E);
			lottery.push(250, 0x1000005F);
			lottery.push(750 * 14 , 0);

			auto lc = lottery.spinRoleta();

			if (lc != nullptr && lc->value != 0) {
				auto clubset_original = sIff::getInstance().findClubSetOriginal((uint32_t)lc->value);

				if (clubset_original.empty())
					throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID = " + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[TYPEID = " + std::to_string(pClub->_typeid) + ", ID = "
							+ std::to_string(pClub->id) + "], nao encontrou o Special ClubSet Original no IFF_STRUCT do Server. System Error", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 362, 0x5300363));

				if (clubset_original.size() <= (uint32_t)(rank - 1) )
					throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID = " + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[TYPEID = " + std::to_string(pClub->_typeid) + ", ID = "
							+ std::to_string(pClub->id) + "], nao tem o Rank[value=" + std::to_string(rank) + "] do Special ClubSet Original no IFF_STRUCT do Server. System Error", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 363, 0x5300364));

				auto it = std::find_if(clubset_original.begin(), clubset_original.end(), [&](auto& el) {
					return WarehouseItemEx::ClubsetWorkshop::s_calcRank((short*)el.slot) == rank;
				});

				if (it == clubset_original.end())
					throw exception("[channel::requestClubSetWorkShopUpRank][Error] player[UID = " + std::to_string(_session.m_pi.uid) + "] tentou upar rank do ClubSet[TYPEID = " + std::to_string(pClub->_typeid) + ", ID = "
							+ std::to_string(pClub->id) + "], nao encontrou o Rank[value=" + std::to_string(rank) + "] no IFF_STRUCT do Server. System Error", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 364, 0x5300365));

				if (!_session.m_pi.ownerItem(it->_typeid)) {

					_session.m_pi.cwtc.clubset_id = pClub->id;
					_session.m_pi.cwtc.stat = stat;
					_session.m_pi.cwtc.transform_typeid = it->_typeid;

					_smp::message_pool::getInstance().push(new message("[ClubSetWorkshop::UpRank][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] transformou o ClubSet[TYPEID="
								+ std::to_string(pClub->_typeid) + ", ID=" + std::to_string(pClub->id) + "] no ClubSet[TYPEID=" + std::to_string(it->_typeid) + "] Special, aguardando confirmacao do cliente.", CL_FILE_LOG_AND_CONSOLE));

					p.init_plain((unsigned short)0x241);

					packet_func::session_send(p, &_session, 1);

					return;
				}
			}
		}

		_smp::message_pool::getInstance().push(new message("[ClubSetWorkshop::UpRank][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] upou Rank[value="
				+ std::to_string(rank) + "] do ClubSet[TYPEID=" + std::to_string(pClub->_typeid) + ", ID=" + std::to_string(pClub->id) + "] Stat[value="
				+ std::to_string(stat) + "" + ((rank == 5 ) ? std::string(", Rank S bonus=" + std::to_string(clubset->work_shop.rank_s_stat) + "") : std::string("")) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x240);

		p.addUint32(0);
		p.addUint32(stat);
		p.addInt32(pClub->id);

		packet_func::session_send(p, &_session, 1);

		SysAchievement sys_achieve;

		if (rank == 5 )
			sys_achieve.incrementCounter(0x6C4000A7u );

		sys_achieve.incrementCounter(0x6C4000A3u );

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestClubSetWorkShopUpRank][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x240);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5300350);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestClubSetWorkShopUpRankTransformConfirm(player& _session, packet *_packet) {
	REQUEST_BEGIN("ClubSetWorkShopUpRankTransformConfirm");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("ClubSetWorkShopUpRankTransformConfirm");

		std::vector< stItem > v_item;
		stItem item{ 0 };

		auto pClub = _session.m_pi.findWarehouseItemById(_session.m_pi.cwtc.clubset_id);

		if (pClub == nullptr)
			throw exception("[channel::requestClubSetWorkShopUpRankTransformConfirm][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou transformar ClubSet[ID="
					+ std::to_string(_session.m_pi.cwtc.clubset_id) + "] no ClubSet[TYPEID=" + std::to_string(_session.m_pi.cwtc.transform_typeid) + "] Special, mas ele nao tem o ClubSet. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 450, 0x5300451));

		auto clubset = sIff::getInstance().findClubSet(pClub->_typeid);

		if (clubset == nullptr)
			throw exception("[channel::requestClubSetWorkShopUpRankTransformConfirm][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou transformar ClubSet[ID="
					+ std::to_string(_session.m_pi.cwtc.clubset_id) + "] no ClubSet[TYPEID=" + std::to_string(_session.m_pi.cwtc.transform_typeid) + "] Special, mas nao existe o ClubSet no IFF_STRUCT do Server. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 451, 0x5300452));

		auto clubset_transform = sIff::getInstance().findClubSet(_session.m_pi.cwtc.transform_typeid);

		if (clubset_transform == nullptr)
			throw exception("[channel::requestClubSetWorkShopUpRankTransformConfirm][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou transformar ClubSet[ID="
					+ std::to_string(_session.m_pi.cwtc.clubset_id) + "] no ClubSet[TYPEID=" + std::to_string(_session.m_pi.cwtc.transform_typeid) + "] Special, mas o ClubSet Special nao existe no IFF_STRUCT do Server. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 452, 0x5300453));

		item.clear();

		item.type = 2;
		item.id = pClub->id;
		item._typeid = pClub->_typeid;
		item.qntd = 1;
		item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

		if (item_manager::removeItem(item, _session) <= 0)
			throw exception("[channel::requestClubSetWorkShopUpRankTransformConfirm][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou transformar ClubSet[ID="
					+ std::to_string(_session.m_pi.cwtc.clubset_id) + "] no ClubSet[TYPEID=" + std::to_string(_session.m_pi.cwtc.transform_typeid) + "] Special, nao conseguiu deletar o ClubSet[TYPEID="
					+ std::to_string(item._typeid) + ", ID=" + std::to_string(item.id) + "] que vai ser transformado no Special. System Error", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 453, 0x5300454));

		v_item.push_back(item);

		item.clear();

		BuyItem bi{ 0 };

		bi.id = -1;
		bi._typeid = clubset_transform->_typeid;
		bi.qntd = 1;

		item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1 );

		if (item._typeid == 0)
			throw exception("[channel::requestClubSetWorkShopUpRankTransformConfirm][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou transformar ClubSet[ID="
					+ std::to_string(_session.m_pi.cwtc.clubset_id) + "] no ClubSet[TYPEID=" + std::to_string(_session.m_pi.cwtc.transform_typeid) + "] Special, nao conseguiu inicializar o ClubSet[TYPEID="
					+ std::to_string(bi._typeid) + "]. System Error", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 454, 0x5300455));

		auto rt = item_manager::RetAddItem::T_INIT_VALUE;

		if ((rt = item_manager::addItem(item, _session, 0, 0)) < 0)
			throw exception("[channel::requestClubSetWorkShopUpRankTransformConfirm][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou transformar ClubSet[ID="
					+ std::to_string(_session.m_pi.cwtc.clubset_id) + "] no ClubSet[TYPEID=" + std::to_string(_session.m_pi.cwtc.transform_typeid) + "] Special, nao conseguiu adicionar o ClubSet[TYPEID="
					+ std::to_string(item._typeid) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 455, 0x5300456));

		if (rt != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH)
			v_item.push_back(item);

		_smp::message_pool::getInstance().push(new message("[ClubSetWokShop::UpRankTransformConfirm][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] confirmou a transformacao do ClubSet[TYPEID="
				+ std::to_string(clubset->_typeid) + ", ID=" + std::to_string(_session.m_pi.cwtc.clubset_id) + "] no ClubSet[TYPEID=" + std::to_string(item._typeid) + ", ID=" + std::to_string(item.id) + "] Special", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addBuffer(&el.stat, sizeof(el.stat));
			p.addUint32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addZeroByte(25);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x242);

		p.addUint32(0);

		p.addUint32(item._typeid);
		p.addInt32(item.id);

		packet_func::session_send(p, &_session, 1);

		SysAchievement sys_achieve;

		sys_achieve.incrementCounter(0x6C4000A4u );

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestClubSetWorkShopUpRankTransformConfirm][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x242);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5300450);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestClubSetWorkShopUpRankTransformCancel(player& _session, packet *_packet) {
	REQUEST_BEGIN("ClubSetWorkShopUpRankTransformCancel");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("ClubSetWorkShopUpRankTransformCancel");

		auto pClub = _session.m_pi.findWarehouseItemById(_session.m_pi.cwtc.clubset_id);

		if (pClub == nullptr)
			throw exception("[channel::requestClubSetWorkShopUpRankTransformCancel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou cancelar o transformacao do ClubSet[ID="
					+ std::to_string(_session.m_pi.cwtc.clubset_id) +"] no ClubSet[TYPEID="
					+ std::to_string(_session.m_pi.cwtc.transform_typeid) + "] Special, mas ele nao tem o ClubSet. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 400, 0x5300401));

		auto clubset = sIff::getInstance().findClubSet(pClub->_typeid);

		if (clubset == nullptr)
			throw exception("[channel::requestClubSetWorkShopUpRankTransformCancel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou cancelar o transformacao do ClubSet[ID="
					+ std::to_string(_session.m_pi.cwtc.clubset_id) + "] no ClubSet[TYPEID="
					+ std::to_string(_session.m_pi.cwtc.transform_typeid) + "] Special, mas o ClubSet nao existe no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 401, 0x5300402));

		if (_session.m_pi.cwtc.stat > 4)
			throw exception("[channel::requestClubSetWorkShopUpRankTransformCancel][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou cancelar o transformacao do ClubSet[ID="
					+ std::to_string(_session.m_pi.cwtc.clubset_id) + "] no ClubSet[TYPEID="
					+ std::to_string(_session.m_pi.cwtc.transform_typeid) + "] Special, mas o Stat[value=" + std::to_string(_session.m_pi.cwtc.stat) + "] eh invalido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 402, 0x5300403));

		_smp::message_pool::getInstance().push(new message("[ClubSetWorkshop::UpRankTransformCancel][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] cancelou a transformacao do ClubSet[TYPEID="
				+ std::to_string(pClub->_typeid) + ", ID=" + std::to_string(pClub->id) + "] no ClubSet[TYPEID=" + std::to_string(_session.m_pi.cwtc.transform_typeid) + "] Special", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x243);

		p.addUint32(0);

		p.addUint32(_session.m_pi.cwtc.stat);
		p.addUint32(_session.m_pi.cwtc.clubset_id);

		packet_func::session_send(p, &_session, 1);

		SysAchievement sys_achieve;

		sys_achieve.incrementCounter(0x6C4000A3u );

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestClubSetWorkShopUpRankTransformCancel][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x243);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5300400);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestClubSetReset(player& _session, packet *_packet) {
	REQUEST_BEGIN("ClubSetRest");

	packet p;

	try {

		std::vector< stItemEx > v_item;
		stItemEx item{ 0 };

		uint32_t item_typeid = _packet->readUint32();
		int32_t clubset_id = _packet->readInt32();

		CHECK_SESSION_IS_AUTHORIZED("ClubSetRest");

		if (item_typeid != 0x1A00024B  && item_typeid != 0x1A000247 )
			throw exception("[channel::requestClubSetReset][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou resetar ClubSet[ID="
					+ std::to_string(clubset_id) + "], mas o item[TYPEID=" + std::to_string(item_typeid) + "] eh desconhecido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 505, 0x5300506));

		auto pWi = _session.m_pi.findWarehouseItemByTypeid(item_typeid);

		if (pWi == nullptr)
			throw exception("[channel::requestClubSetReset][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou resetar ClubSet[ID="
					+ std::to_string(clubset_id) + "], mas ele nao tem o item[TYPEID=" + std::to_string(item_typeid) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 500, 0x5300501));

		if (pWi->STDA_C_ITEM_QNTD < 1)
			throw exception("[channel::requestClubSetReset][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou resetar ClubSet[ID="
					+ std::to_string(clubset_id) + "], mas ele nao tem quantidade suficiente do item[TYPEID=" + std::to_string(pWi->_typeid) + ", ID="
					+ std::to_string(pWi->id) + ", QNTD=" + std::to_string(pWi->STDA_C_ITEM_QNTD) + ", request=1]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 501, 0x5300502));

		auto pClub = _session.m_pi.findWarehouseItemById(clubset_id);

		if (pClub == nullptr)
			throw exception("[channel::requestClubSetReset][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou resetar ClubSet[ID="
					+ std::to_string(clubset_id) + "], mas ele nao tem o ClubSet. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 502, 0x5300503));

		auto clubset = sIff::getInstance().findClubSet(pClub->_typeid);

		if (clubset == nullptr)
			throw exception("[channel::requestClubSetReset][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou resetar ClubSet[ID="
					+ std::to_string(clubset_id) + "], mas o ClubSet nao existe no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 503, 0x5300504));

		int32_t rank_base = WarehouseItemEx::ClubsetWorkshop::s_calcRank((short*)clubset->slot);
		int32_t rank = pClub->clubset_workshop.calcRank((short*)clubset->slot);

		if (rank_base == -1 || rank == -1)
			throw exception("[channel::requestClubSetReset][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou resetar ClubSet[ID="
					+ std::to_string(clubset_id) + "], nao conseguiu pegar o Rank do ClubSet[TYPEID=" + std::to_string(pClub->_typeid) + ", ID="
					+ std::to_string(pClub->id) + ", rank=" + std::to_string(rank) + ", rank_base=" + std::to_string(rank_base) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 505, 0x5300506));

		auto rank_up_exp = sIff::getInstance().findClubSetWorkShopRankExp(clubset->work_shop.tipo_rank_s);

		if (rank_up_exp == nullptr)
			throw exception("[channel::requestClubSetReset][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou resetar ClubSet[ID="
					+ std::to_string(clubset_id) + "], mas nao encontrou o Rank Up Exp[tipo=" + std::to_string(clubset->work_shop.tipo_rank_s) + "] no IFF_STRUCT do Server. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 504, 0x5300505));

		item.clear();

		item.type = 2;
		item.id = pWi->id;
		item._typeid = pWi->_typeid;
		item.qntd = 1;
		item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

		if (item_manager::removeItem(item, _session) <= 0)
			throw exception("[channel::requestClubSetReset][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou resetar ClubSet[ID="
					+ std::to_string(clubset_id) + "], mas nao conseguiu remover o Item[TYPEID=" + std::to_string(item._typeid) + ", ID=" + std::to_string(item.id) + "]. ErrorSystem", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 506, 0x5300507));

		v_item.push_back(item);

		uint32_t mastery = 0u;
		uint64_t pang = 0u;

		if (item_typeid == 0x1A00024B) {

			IFF::Enchant *enchant = nullptr;

			for (auto i = rank_base + 1; i <= rank; ++i)
				mastery += rank_up_exp->rank[i];

			for (auto i = 0u; i < (sizeof(pClub->c) / sizeof(pClub->c[0])); ++i) {
				for (auto j = 0u; j < (uint32_t)pClub->c[i]; ++j) {
					if ((enchant = sIff::getInstance().findEnchant((iff::ENCHANT << 26) | (i << 20) + j)) != nullptr)
						pang += enchant->pang;
				}
			}

			mastery = (uint32_t)(mastery * 0.5f);
			pang = (uint64_t)(pang * 0.5f);

			pClub->clubset_workshop.mastery += mastery;

			if (pang > 0ull)
				_session.m_pi.addPang(pang);

			p.init_plain((unsigned short)0xC8);

			p.addUint64(_session.m_pi.ui.pang);
			p.addUint64(pang);

			packet_func::session_send(p, &_session, 1);

		}

		memset(pClub->clubset_workshop.c, 0, sizeof(pClub->clubset_workshop.c));

		pClub->clubset_workshop.level = 0;
		pClub->clubset_workshop.rank = 0;
		pClub->clubset_workshop.recovery_pts = 0;

		memset(pClub->c, 0, sizeof(pClub->c));

		item.clear();

		item.type = 0xCC;
		item.id = pClub->id;
		item._typeid = pClub->_typeid;
#if defined(_WIN32)
		memcpy_s(item.clubset_workshop.c, sizeof(item.clubset_workshop.c), pClub->clubset_workshop.c, sizeof(item.clubset_workshop.c));
#elif defined(__linux__)
		memcpy(item.clubset_workshop.c, pClub->clubset_workshop.c, sizeof(item.clubset_workshop.c));
#endif
		item.clubset_workshop.level = (char)pClub->clubset_workshop.level;
		item.clubset_workshop.mastery = pClub->clubset_workshop.mastery;
		item.clubset_workshop.rank = pClub->clubset_workshop.rank;
		item.clubset_workshop.recovery = pClub->clubset_workshop.recovery_pts;

		v_item.push_back(item);

		item.type = 0xC9;
#if defined(_WIN32)
		memcpy_s(item.c, sizeof(item.c), pClub->c, sizeof(item.c));
#elif defined(__linux__)
		memcpy(item.c, pClub->c, sizeof(item.c));
#endif

		v_item.push_back(item);

		snmdb::NormalManagerDB::getInstance().add(12, new CmdUpdateClubSetWorkshop(_session.m_pi.uid, *pClub, CmdUpdateClubSetWorkshop::F_RESET), channel::SQLDBResponse, this);

		snmdb::NormalManagerDB::getInstance().add(8, new CmdUpdateClubSetStats(_session.m_pi.uid, *pClub, 0), channel::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[ClubSet::Reset][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] resetou o ClubSet[TYPEID="
				+ std::to_string(pClub->_typeid) + ", ID=" + std::to_string(pClub->id) + "] "
				+ (item_typeid == 0x1A00024B ? std::string("Hard[Pang=" + std::to_string(pang) + ", Mastery=" + std::to_string(mastery) + "] Item") : std::string("Soft Item")), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addBuffer(&el.stat, sizeof(el.stat));
			p.addUint32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addBuffer(&el.c, sizeof(el.c));
			p.addZeroByte(15);
			if (el.type == 0xCC)
				p.addBuffer(&el.clubset_workshop, sizeof(el.clubset_workshop));
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x247);

		p.addUint32(0);

		p.addUint32(pClub->_typeid);
		p.addInt32(pClub->id);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestClubSetReset][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x247);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5300500);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestMakeTutorial(player& _session, packet *_packet) {
	REQUEST_BEGIN("MakeTutorial");

	packet p;

#if defined(__linux__)
#pragma pack(1)
#endif

	struct RequestMakeTutorial {
		void clear() { memset(this, 0, sizeof(RequestMakeTutorial)); };
		union u1 {
			unsigned short usTipo;
			struct {
				unsigned char finish;
				unsigned char tipo;
			}stTipo;
		}uTipo;
		union u2 {
			uint32_t ulValor;
			struct st1 {
				union uByte {
					struct st2 {
						unsigned char _bit0 : 1;
						unsigned char _bit1 : 1;
						unsigned char _bit2 : 1;
						unsigned char _bit3 : 1;
						unsigned char _bit4 : 1;
						unsigned char _bit5 : 1;
						unsigned char _bit6 : 1;
						unsigned char _bit7 : 1;

						unsigned char whatBit() {

							if (_bit0)
								return 1;
							else if (_bit1)
								return 2;
							else if (_bit2)
								return 3;
							else if (_bit3)
								return 4;
							else if (_bit4)
								return 5;
							else if (_bit5)
								return 6;
							else if (_bit6)
								return 7;
							else if (_bit7)
								return 8;

							return 0;
						};
					}st8bit;
					unsigned char ucbyte;
				};
				uByte rookie;
				uByte beginner;
				uByte advancer;

			}stValor;
		}uValor;
	};

#if defined(__linux__)
#pragma pack()
#endif

	try {

		stItem item{ 0 };

		item.type = 2;
		item.id = -1;

		std::string msg = "";

		RequestMakeTutorial rmt{ 0 };

		_packet->readBuffer(&rmt, sizeof(RequestMakeTutorial));

		CHECK_SESSION_IS_AUTHORIZED("MakeTutorial");

		switch (rmt.uTipo.stTipo.tipo) {
		case 0:
		{
			if (rmt.uTipo.stTipo.tipo == 0  && _session.m_pi.TutoInfo.rookie & rmt.uValor.stValor.rookie.ucbyte)
				throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
						+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], mas ele ja concluiu esse tutorial. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 550, 0x5300551));

			if (rmt.uValor.stValor.rookie.st8bit._bit2 || rmt.uValor.stValor.rookie.st8bit._bit3) {
				if (_session.m_pi.TutoInfo.rookie < 3)
					throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
							+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], mas ele nao concluiu os outros tutoriais para poder completar o Rookie. Hacker ou Bug",
							STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 553, 0x5300554));
			}else if (rmt.uValor.stValor.rookie.st8bit._bit4) {
				if ((_session.m_pi.TutoInfo.rookie & 7) <= 3)
					throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
							+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], mas ele nao concluiu os outros tutoriais para poder completar o Rookie. Hacker ou Bug",
							STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 553, 0x5300554));
			}else if (rmt.uValor.stValor.rookie.st8bit._bit6) {
				if ((_session.m_pi.TutoInfo.rookie & 11) <= 3)
					throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
							+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], mas ele nao concluiu os outros tutoriais para poder completar o Rookie. Hacker ou Bug",
							STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 553, 0x5300554));
			}else if (rmt.uValor.stValor.rookie.st8bit._bit5) {
				if ((_session.m_pi.TutoInfo.rookie & 15) <= 3)
					throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
							+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], mas ele nao concluiu os outros tutoriais para poder completar o Rookie. Hacker ou Bug",
							STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 553, 0x5300554));
			}else if (((rmt.uValor.stValor.rookie.ucbyte - 1) & _session.m_pi.TutoInfo.rookie) != (rmt.uValor.stValor.rookie.ucbyte -1))
				throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
						+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], mas ele nao concluiu os outros tutoriais para poder completar o Rookie. Hacker ou Bug",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 553, 0x5300554));

			_session.m_pi.TutoInfo.rookie |= rmt.uValor.ulValor;

			switch (rmt.uValor.stValor.rookie.st8bit.whatBit()) {
			case 1:
				item._typeid = 0x1A000002;
				item.qntd = item.STDA_C_ITEM_QNTD = 10;
				break;
			case 2:
				item._typeid = 0x1800000B;
				item.qntd = item.STDA_C_ITEM_QNTD = 3;
				break;
			case 3:
				item._typeid = 0x18000025;
				item.qntd = item.STDA_C_ITEM_QNTD = 3;
				break;
			case 4:
				item._typeid = 0x18000005;
				item.qntd = item.STDA_C_ITEM_QNTD = 3;
				break;
			case 5:
				item._typeid = 0x18000004;
				item.qntd = item.STDA_C_ITEM_QNTD = 3;
				break;
			case 6:
				item._typeid = 0x1800000A;
				item.qntd = item.STDA_C_ITEM_QNTD = 3;
				break;
			case 7:
				item._typeid = 0x18000000;
				item.qntd = item.STDA_C_ITEM_QNTD = 3;
				break;
			case 8:
				item._typeid = PANG_POUCH_TYPEID;
				item.qntd = item.STDA_C_ITEM_QNTD = 1000;
				break;
			case 0:
			default:
				throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
						+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], o valor do tutorial eh desconhecido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 555, 0x5300556));
			}

			msg = "NICE TUTORIAL ROOKIE CLEAR";

			MailBoxManager::sendMessageWithItem(0, _session.m_pi.uid, msg, item);

			_smp::message_pool::getInstance().push(new message("[Tutorial][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] Concluiu Tutorial Rookie", CL_FILE_LOG_AND_CONSOLE));

			if (_session.m_pi.TutoInfo.rookie & 0xFF && rmt.uTipo.stTipo.finish) {

				std::vector< stItem > v_item;

				item.clear();

				item.type = 2;
				item.id = -1;
				item._typeid = 0x1C000000;
				item.qntd = item.STDA_C_ITEM_QNTD = 1;

				v_item.push_back(item);

				item.clear();

				item.type = 2;
				item.id = -1;
				item._typeid = 0x10000014;
				item.qntd = item.STDA_C_ITEM_QNTD = 1;

				v_item.push_back(item);

				msg = "NICE ALL TUTORIAL ROOKIE CLEAR";

				MailBoxManager::sendMessageWithItem(0, _session.m_pi.uid, msg, v_item);

				snmdb::NormalManagerDB::getInstance().add(14, new CmdTutoEventClear(_session.m_pi.uid, CmdTutoEventClear::T_ROOKIE), channel::SQLDBResponse, this);

				_smp::message_pool::getInstance().push(new message("[Tutorial][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] Concluiu Todos Tutoriais Rookie", CL_FILE_LOG_AND_CONSOLE));
			}
			break;
		}
		case 1:
		{
			if (rmt.uTipo.stTipo.tipo == 1  && _session.m_pi.TutoInfo.beginner & rmt.uValor.stValor.beginner.ucbyte)
				throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
						+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], mas ele ja concluiu esse tutorial. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 550, 0x5300551));

			if (_session.m_pi.TutoInfo.rookie && 0xFF != 0xFF)
				throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
						+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], mas ele nao concluiu o tutorial rookie. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 554, 0x5300555));

			RequestMakeTutorial::u2 tutu{ _session.m_pi.TutoInfo.beginner };

			if (rmt.uValor.stValor.beginner.st8bit._bit1 || rmt.uValor.stValor.beginner.st8bit._bit2) {
				if (tutu.stValor.beginner.ucbyte < 1)
					throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
							+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], mas ele nao concluiu os outros tutoriais para poder completar o Beginner. Hacker ou Bug",
							STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 553, 0x5300554));
			}else if (rmt.uValor.stValor.beginner.st8bit._bit4 || rmt.uValor.stValor.beginner.st8bit._bit5) {
				if (tutu.stValor.beginner.ucbyte < 15)
					throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
						+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], mas ele nao concluiu os outros tutoriais para poder completar o Beginner. Hacker ou Bug",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 553, 0x5300554));
			}else if (((rmt.uValor.stValor.beginner.ucbyte - 1) & tutu.stValor.beginner.ucbyte) != (rmt.uValor.stValor.beginner.ucbyte - 1))
				throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
						+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], mas ele nao concluiu os outros tutoriais para poder completar o Beginner. Hacker ou Bug",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 553, 0x5300554));

			_session.m_pi.TutoInfo.beginner |= rmt.uValor.ulValor;

			switch (rmt.uValor.stValor.beginner.st8bit.whatBit()) {
			case 1:
				item._typeid = 0x1A000002;
				item.qntd = item.STDA_C_ITEM_QNTD = 10;
				break;
			case 2:
				item._typeid = 0x18000028;
				item.qntd = item.STDA_C_ITEM_QNTD = 1;
				break;
			case 3:
				item._typeid = 0x18000006;
				item.qntd = item.STDA_C_ITEM_QNTD = 1;
				break;
			case 4:
				item._typeid = 0x1800000A;
				item.qntd = item.STDA_C_ITEM_QNTD = 3;
				break;
			case 5:
				item._typeid = 0x18000000;
				item.qntd = item.STDA_C_ITEM_QNTD = 4;
				break;
			case 6:
				item._typeid = 0x18000001;
				item.qntd = item.STDA_C_ITEM_QNTD = 3;
				break;
			case 7:
			case 8:
			case 0:
			default:
				throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
						+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], o valor do tutorial eh desconhecido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 555, 0x5300556));
			}

			msg = "NICE TUTORIAL BEGINNER CLEAR";

			MailBoxManager::sendMessageWithItem(0, _session.m_pi.uid, msg, item);

			_smp::message_pool::getInstance().push(new message("[Tutorial][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] Concluiu Tutorial Beginner", CL_FILE_LOG_AND_CONSOLE));

			if (_session.m_pi.TutoInfo.beginner == (0x3F << 8)) {

				std::vector< stItem > v_item;

				item.clear();

				item.type = 2;
				item.id = -1;
				item._typeid = 0x18000027;
				item.qntd = item.STDA_C_ITEM_QNTD = 10;

				v_item.push_back(item);

				item.clear();

				item.type = 2;
				item.id = -1;
				item._typeid = PANG_POUCH_TYPEID;
				item.qntd = item.STDA_C_ITEM_QNTD = 10000;

				v_item.push_back(item);

				msg = "NICE ALL TUTORIAL BEGINNER CLEAR";

				MailBoxManager::sendMessageWithItem(0, _session.m_pi.uid, msg, v_item);

				snmdb::NormalManagerDB::getInstance().add(14, new CmdTutoEventClear(_session.m_pi.uid, CmdTutoEventClear::T_BEGINNER), channel::SQLDBResponse, this);

				_smp::message_pool::getInstance().push(new message("[Tutorial][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] Concluiu Todos Tutoriais Beginner", CL_FILE_LOG_AND_CONSOLE));
			}
			break;
		}
		case 2:
		{
			if (rmt.uTipo.stTipo.tipo == 2  && _session.m_pi.TutoInfo.advancer & rmt.uValor.stValor.advancer.ucbyte)
				throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
					+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], mas ele ja concluiu esse tutorial. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 550, 0x5300551));

			if (_session.m_pi.TutoInfo.rookie && 0xFF != 0xFF)
				throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
					+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], mas ele nao concluiu o tutorial rookie. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 554, 0x5300555));

			if (_session.m_pi.TutoInfo.beginner && 0x3F != 0x3F)
				throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
						+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], mas ele nao concluiu o tutorial Beginner. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 554, 0x5300555));

			RequestMakeTutorial::u2 tutu{ _session.m_pi.TutoInfo.advancer };

			if (((rmt.uValor.stValor.advancer.ucbyte - 1) & tutu.stValor.advancer.ucbyte) != (rmt.uValor.stValor.advancer.ucbyte - 1))
				throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
					+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], mas ele nao concluiu os outros tutoriais para poder completar o Advancer. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 553, 0x5300554));

			_session.m_pi.TutoInfo.advancer |= rmt.uValor.ulValor;

			switch (rmt.uValor.stValor.advancer.st8bit.whatBit()) {
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 0:
			default:
				throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
						+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], o valor do tutorial eh desconhecido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 555, 0x5300556));
			}

			msg = "NICE TUTORIAL ADVANCER CLEAR";

			MailBoxManager::sendMessageWithItem(0, _session.m_pi.uid, msg, item);

			_smp::message_pool::getInstance().push(new message("[Tutorial][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] Concluiu Tutorial Advancer", CL_FILE_LOG_AND_CONSOLE));

			if (_session.m_pi.TutoInfo.advancer == (0x7 << 16) && rmt.uTipo.stTipo.finish) {

				std::vector< stItem > v_item;

				item.clear();

				item.type = 2;
				item.id = -1;
				item._typeid = 0x18000000;
				item.qntd = item.STDA_C_ITEM_QNTD = 1;

				v_item.push_back(item);

				item.clear();

				item.type = 2;
				item.id = -1;
				item._typeid = PANG_POUCH_TYPEID;
				item.qntd = item.STDA_C_ITEM_QNTD = 30000;

				v_item.push_back(item);

				msg = "NICE ALL TUTORIAL ADVANCER CLEAR";

				MailBoxManager::sendMessageWithItem(0, _session.m_pi.uid, msg, v_item);

				snmdb::NormalManagerDB::getInstance().add(14, new CmdTutoEventClear(_session.m_pi.uid, CmdTutoEventClear::T_ADVANCER), channel::SQLDBResponse, this);

				_smp::message_pool::getInstance().push(new message("[Tutorial][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] Concluiu Todos Tutoriais Advancer", CL_FILE_LOG_AND_CONSOLE));
			}
			break;
		}
		default:
			throw exception("[channel::requestMakeTutorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fazer tutorial[tipo="
					+ std::to_string(rmt.uTipo.stTipo.tipo) + ", value=" + std::to_string(rmt.uValor.ulValor) + "], tipo desconhecido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 551, 0x5300552));
		}

		snmdb::NormalManagerDB::getInstance().add(13, new CmdUpdateTutorial(_session.m_pi.uid, _session.m_pi.TutoInfo), channel::SQLDBResponse, this);

		p.init_plain((unsigned short)0x11F);

		p.addUint8(rmt.uTipo.stTipo.tipo);
		p.addUint8(1);

		if (rmt.uTipo.stTipo.tipo == 0 )
			p.addUint32(_session.m_pi.TutoInfo.rookie);
		else if (rmt.uTipo.stTipo.tipo == 1 )
			p.addUint32(_session.m_pi.TutoInfo.beginner);
		else if (rmt.uTipo.stTipo.tipo == 2 )
			p.addUint32(_session.m_pi.TutoInfo.advancer);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestMakeTutorial][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x44);

		p.addUint8(0xE2);
		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5300550);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestEnterWebLinkState(player& _session, packet *_packet) {
	REQUEST_BEGIN("EnterWebLinkState");

	try {

		CHECK_SESSION_IS_AUTHORIZED("EnterWebLinkState");

		_session.m_pi.place = _packet->readUint8();

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestEnterWebLinkState][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestCookie(player& _session, packet *_packet) {
	REQUEST_BEGIN("Cookie");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("Cookie");

		_session.m_pi.updateCookie();

		p.init_plain((unsigned short)0x96);

		p.addUint64(_session.m_pi.cookie);

		packet_func::session_send(p, &_session, 1);

		CmdGrandZodiacPontos cmd_gzp(_session.m_pi.uid, CmdGrandZodiacPontos::eCMD_GRAND_ZODIAC_TYPE::CGZT_GET, true);

		snmdb::NormalManagerDB::getInstance().add(0, &cmd_gzp, nullptr, nullptr);

		cmd_gzp.waitEvent();

		if (cmd_gzp.getException().getCodeError() != 0)
			throw cmd_gzp.getException();

		_session.m_pi.grand_zodiac_pontos = cmd_gzp.getPontos();

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestCookie][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void channel::requestUpdateGachaCoupon(player& _session, packet *_packet) {
	REQUEST_BEGIN("UpdateGachaCoupon");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("UpdateGachaCoupon");

		CmdCouponGacha cmd_cg(_session.m_pi.uid, true);

		snmdb::NormalManagerDB::getInstance().add(0, &cmd_cg, channel::SQLDBResponse, this);

		cmd_cg.waitEvent();

		if (cmd_cg.getException().getCodeError() != 0)
			throw cmd_cg.getException();

		_session.m_pi.cg = cmd_cg.getCouponGacha();

		unsigned char find_ticket_and_sub = 0;

		for (auto& el : _session.m_pi.mp_wi) {

			switch (el.second._typeid) {
				case 0x1A000080:
					el.second.STDA_C_ITEM_QNTD = (unsigned short)_session.m_pi.cg.normal_ticket;
					find_ticket_and_sub = 1;
					break;
				case 0x1A000083:
					el.second.STDA_C_ITEM_QNTD = (unsigned short)_session.m_pi.cg.partial_ticket;
					find_ticket_and_sub |= 2;
					break;
			}

			if (find_ticket_and_sub == 3)
				break;
		}

		packet_func::pacote102(p, &_session, &_session.m_pi);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestUpdateGachaCoupon][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x44);

		p.addUint8(0xE2);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5300600);

		packet_func::session_send(p, &_session, 1);
	}

};

void channel::requestOpenBoxMail(player& _session, packet *_packet) {
	REQUEST_BEGIN("OpenBoxMail");

	packet p;

	try {

		if (!sBoxSystem::getInstance().isLoad())
			sBoxSystem::getInstance().load();

		uint32_t box_typeid = _packet->readUint32();

		CHECK_SESSION_IS_AUTHORIZED("OpenBoxMail");

		if (box_typeid == 0)
			throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(box_typeid) + "], mas o typeid eh invalido(zero). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6300101));

		auto pWi = _session.m_pi.findWarehouseItemByTypeid(box_typeid);

		if (pWi == nullptr)
			throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(box_typeid) + "], mas ele nao tem essa Box. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0x6300102));

		if (pWi->STDA_C_ITEM_QNTD < 1)
			throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas ele nao tem quantidade suficiente da Box[value="
					+ std::to_string(pWi->STDA_C_ITEM_QNTD) + ", request=1]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0x6300103));

		if (sIff::getInstance().getItemGroupIdentify(pWi->_typeid) != iff::ITEM)
			throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao eh uma Box valida. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4, 0x6300104));

		auto item_iff = sIff::getInstance().findItem(pWi->_typeid);

		if (item_iff == nullptr)
			throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao tem essa Box no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 5, 0x6300105));

		auto box = sBoxSystem::getInstance().findBox(pWi->_typeid);

		if (box == nullptr)
			throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao tem essa Box no Box System do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 6, 0x6300106));

		std::vector< stItem > v_item;
		stItem item{ 0 };

		ctx_box_item *ctx_bi = nullptr;
		IFF::Mascot *mascot = nullptr;

		std::string msg = box->msg;

		switch (pWi->_typeid) {
		case SPINNING_CUBE_TYPEID:
		{

			auto key = _session.m_pi.findWarehouseItemByTypeid(KEY_OF_SPINNING_CUBE_TYPEID);

			if (key == nullptr)
				throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
						+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas o ele nao tem a chave para abrir o spinning cube. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 7, 0x6300107));

			if (key->STDA_C_ITEM_QNTD < 1)
				throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
						+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas ele nao tem quantidade suficiante[value="
						+ std::to_string(key->STDA_C_ITEM_QNTD) + ", request=1] de chave para abrir Spinning Cube. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 8, 0x6300108));

			ctx_bi = sBoxSystem::getInstance().drawBox(_session, *box);

			if (ctx_bi == nullptr)
				throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
						+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu sortear um Spinning Cube Item. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 9, 0x6300109));

			item.clear();

			item.type = 2;
			item.id = pWi->id;
			item._typeid = box->_typeid;
			item.qntd = 1;
			item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

			if (item_manager::removeItem(item, _session) <= 0)
				throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
						+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu deletar o Spinning Cube. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0x6300110));

			v_item.push_back(item);

			item.clear();

			item.type = 2;
			item.id = key->id;
			item._typeid = KEY_OF_SPINNING_CUBE_TYPEID;
			item.qntd = 1;
			item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

			if (item_manager::removeItem(item, _session) <= 0)
				throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
						+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu deletar a Key[TYPEID="
						+ std::to_string(KEY_OF_SPINNING_CUBE_TYPEID) + ", DESC=Spinning Cube]. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 11, 0x6300111));

			v_item.push_back(item);

			if (box->opened_typeid > 0) {

				item.clear();

				item.type = 2;
				item._typeid = box->opened_typeid;
				item.qntd = 1;
				item.STDA_C_ITEM_QNTD = (short)item.qntd;

				auto rt = item_manager::RetAddItem::T_INIT_VALUE;

				if ((rt = item_manager::addItem(item, _session, 0, 0)) < 0)
					throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
							+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu adicionar um  Openned Spinning Cube. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 12, 0x6300112));

				if (rt != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH) {

					p.init_plain((unsigned short)0x216);

					p.addUint32((const uint32_t)GetLocalTimeAsUnix());
					p.addUint32(1);

					p.addUint8(item.type);
					p.addUint32(item._typeid);
					p.addInt32(item.id);
					p.addUint32(item.flag_time);
					p.addBuffer(&item.stat, sizeof(item.stat));
					p.addUint32((item.STDA_C_ITEM_TIME > 0) ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD);
					p.addZeroByte(25);

					packet_func::session_send(p, &_session, 1);
				}

			}

			item.clear();

			item.type = 2;
			item.id = -1;
			item._typeid = ctx_bi->_typeid;

			if (sIff::getInstance().getItemGroupIdentify(ctx_bi->_typeid) == iff::MASCOT && (mascot = sIff::getInstance().findMascot(ctx_bi->_typeid)) != nullptr
				&& mascot->shop.flag_shop.time_shop.dia > 0 && mascot->shop.flag_shop.time_shop.active) {
				item.qntd = 1;
				item.flag_time = 4;
				item.STDA_C_ITEM_QNTD = 1;
				item.STDA_C_ITEM_TIME = (short)ctx_bi->qntd;
			}else {
				item.qntd = ctx_bi->qntd;
				item.STDA_C_ITEM_QNTD = (short)item.qntd;
			}

			if (MailBoxManager::sendMessageWithItem(0, _session.m_pi.uid, msg, item) <= 0)
				throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
						+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu colocar o item ganho no mailbox do player. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 13, 0x6300113));

			if (ctx_bi->raridade == BOX_TYPE_RARETY::R_SUPER_RARE) {
				_smp::message_pool::getInstance().push(new message("[BoxSystem::SpinningCube][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] Spinning Cube[TYPEID=" + std::to_string(pWi->_typeid) + "] ganhou super raro[TYPEID="
						+ std::to_string(ctx_bi->_typeid) + ", QNTD=" + std::to_string(ctx_bi->qntd) + "] no spinning cube.", CL_FILE_LOG_AND_CONSOLE));

				std::string msg = "<PARAMS><BOX_TYPEID>" + std::to_string(box->_typeid) + "</BOX_TYPEID><NICKNAME>" + std::string(_session.m_pi.nickname) + "</NICKNAME><TYPEID>"
						+ std::to_string(ctx_bi->_typeid) + "</TYPEID><QTY>" + std::to_string(ctx_bi->qntd) + "</QTY></PARAMS>";

				msg = verifyAndEncode(msg);

#ifdef _DEBUG
				_smp::message_pool::getInstance().push(new message("[channel::requestOpenBoxMail][Log] Message. Hex: " + hex_util::StringToHexString(msg), CL_FILE_LOG_AND_CONSOLE));
#endif

				unsigned char opt = (ctx_bi->_typeid == PANG_POUCH_TYPEID) ? 2 : 1;

				snmdb::NormalManagerDB::getInstance().add(23, new CmdInsertSpinningCubeSuperRareWinBroadcast(msg, opt), channel::SQLDBResponse, this);
			}

			SysAchievement sys_achieve;

			sys_achieve.incrementCounter(0x6C400054u );

			_smp::message_pool::getInstance().push(new message("[BoxSystem::SpinningCube][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] abriu Spinning Cube[TYPEID=" + std::to_string(pWi->_typeid) + "] e ganhou o Item[TYPEID="
					+ std::to_string(ctx_bi->_typeid) + ", QNTD=" + std::to_string(ctx_bi->qntd) + ", RARIDADE=" + std::to_string((short)ctx_bi->raridade) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0xA7);

			p.addUint8((unsigned char)v_item.size());

			for (auto& el : v_item) {
				p.addUint32(el._typeid);
				p.addInt32(el.id);
				p.addUint16((const unsigned short)el.stat.qntd_dep);
			}

			packet_func::session_send(p, &_session, 1);

			p.init_plain((unsigned short)0xAA);

			p.addUint16(0);

			p.addUint64(_session.m_pi.ui.pang);
			p.addUint64(_session.m_pi.cookie);

			packet_func::session_send(p, &_session, 1);

			p.init_plain((unsigned short)0x19D);

			p.addUint32(0);

			p.addUint32(box->_typeid);
			p.addUint32(ctx_bi->_typeid);
			p.addUint32(ctx_bi->qntd);

			packet_func::session_send(p, &_session, 1);

			sys_achieve.finish_and_update(_session);

			break;
		}
		case PAPEL_BOX_TYPEID:
		{

			ctx_bi = sBoxSystem::getInstance().drawBox(_session, *box);

			if (ctx_bi == nullptr)
				throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
						+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu sortear um Papel Box Item. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 9, 0x6300109));

			item.clear();

			item.type = 2;
			item.id = pWi->id;
			item._typeid = box->_typeid;
			item.qntd = 1;
			item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

			if (item_manager::removeItem(item, _session) <= 0)
				throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
						+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu deletar Papel Box. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0x6300110));

			v_item.push_back(item);

			stItem key{ 0 };

			key.clear();

			key.type = 2;
			key.id = -1;
			key._typeid = KEY_OF_SPINNING_CUBE_TYPEID;
			key.qntd = 30;
			key.STDA_C_ITEM_QNTD = (short)key.qntd;

			auto rt = item_manager::RetAddItem::T_INIT_VALUE;

			if ((rt = item_manager::addItem(key, _session, 0, 0)) < 0)
				throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
						+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], nao conseguiu adicionar Key[TYPEID="
						+ std::to_string(KEY_OF_SPINNING_CUBE_TYPEID) + ", DESC=Spinning Cube]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 14, 0x6300114));

			if (box->opened_typeid > 0) {

				item.clear();

				item.type = 2;
				item._typeid = box->opened_typeid;
				item.qntd = 1;
				item.STDA_C_ITEM_QNTD = (short)item.qntd;

				auto rt = item_manager::RetAddItem::T_INIT_VALUE;

				if ((rt = item_manager::addItem(item, _session, 0, 0)) < 0)
					throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
							+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu adicionar um  Openned Papel Box. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 12, 0x6300112));

				if (rt != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH) {

					p.init_plain((unsigned short)0x216);

					p.addUint32((const uint32_t)GetLocalTimeAsUnix());
					p.addUint32(1);

					p.addUint8(item.type);
					p.addUint32(item._typeid);
					p.addInt32(item.id);
					p.addUint32(item.flag_time);
					p.addBuffer(&item.stat, sizeof(item.stat));
					p.addUint32((item.STDA_C_ITEM_TIME > 0) ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD);
					p.addZeroByte(25);

					packet_func::session_send(p, &_session, 1);
				}

			}

			item.clear();

			item.type = 2;
			item.id = -1;
			item._typeid = ctx_bi->_typeid;

			if (sIff::getInstance().getItemGroupIdentify(ctx_bi->_typeid) == iff::MASCOT && (mascot = sIff::getInstance().findMascot(ctx_bi->_typeid)) != nullptr
				&& mascot->shop.flag_shop.time_shop.dia > 0 && mascot->shop.flag_shop.time_shop.active) {
				item.qntd = 1;
				item.flag_time = 4;
				item.STDA_C_ITEM_QNTD = 1;
				item.STDA_C_ITEM_TIME = (short)ctx_bi->qntd;
			}else {
				item.qntd = ctx_bi->qntd;
				item.STDA_C_ITEM_QNTD = (short)item.qntd;
			}

			if (MailBoxManager::sendMessageWithItem(0, _session.m_pi.uid, msg, item) <= 0)
				throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
						+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu colocar o item ganho no mailbox do player. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 13, 0x6300113));

			_smp::message_pool::getInstance().push(new message("[BoxSystem::PapelBox][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] abriu Papel Box[TYPEID=" + std::to_string(pWi->_typeid) + "] e ganhou o Item[TYPEID="
					+ std::to_string(ctx_bi->_typeid) + ", QNTD=" + std::to_string(ctx_bi->qntd) + ", RARIDADE=" + std::to_string((short)ctx_bi->raridade) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0xA7);

			p.addUint8((unsigned char)v_item.size());

			for (auto& el : v_item) {
				p.addUint32(el._typeid);
				p.addInt32(el.id);
				p.addUint16((const unsigned short)el.stat.qntd_dep);
			}

			packet_func::session_send(p, &_session, 1);

			p.init_plain((unsigned short)0xAA);

			p.addUint16(1);

			p.addUint32(key._typeid);
			p.addInt32(key.id);
			p.addUint16(key.STDA_C_ITEM_TIME);
			p.addUint8(key.flag_time);
			p.addUint16((unsigned short)key.stat.qntd_dep);
			p.addBuffer(&key.date.date.sysDate[1], sizeof(SYSTEMTIME));
			p.addBuffer(&key.ucc.IDX, sizeof(item.ucc.IDX));

			p.addUint64(_session.m_pi.ui.pang);
			p.addUint64(_session.m_pi.cookie);

			packet_func::session_send(p, &_session, 1);

			p.init_plain((unsigned short)0x19D);

			p.addUint32(0);

			p.addUint32(box->_typeid);
			p.addUint32(ctx_bi->_typeid);
			p.addUint32(ctx_bi->qntd);

			packet_func::session_send(p, &_session, 1);

			break;
		}
		default:
		{

			ctx_bi = sBoxSystem::getInstance().drawBox(_session, *box);

			if (ctx_bi == nullptr)
				throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
						+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu sortear um Box Item. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 9, 0x6300109));

			item.clear();

			item.type = 2;
			item.id = pWi->id;
			item._typeid = box->_typeid;
			item.qntd = 1;
			item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

			if (item_manager::removeItem(item, _session) <= 0)
				throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
						+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu deletar Box. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0x6300110));

			v_item.push_back(item);

			if (box->opened_typeid > 0) {

				item.clear();

				item.type = 2;
				item._typeid = box->opened_typeid;
				item.qntd = 1;
				item.STDA_C_ITEM_QNTD = (short)item.qntd;

				auto rt = item_manager::RetAddItem::T_INIT_VALUE;

				if ((rt = item_manager::addItem(item, _session, 0, 0)) < 0)
					throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
							+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu adicionar um  Openned Box. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 12, 0x6300112));

				if (rt != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH) {

					p.init_plain((unsigned short)0x216);

					p.addUint32((const uint32_t)GetLocalTimeAsUnix());
					p.addUint32(1);

					p.addUint8(item.type);
					p.addUint32(item._typeid);
					p.addInt32(item.id);
					p.addUint32(item.flag_time);
					p.addBuffer(&item.stat, sizeof(item.stat));
					p.addUint32((item.STDA_C_ITEM_TIME > 0) ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD);
					p.addZeroByte(25);

					packet_func::session_send(p, &_session, 1);
				}

			}

			item.clear();

			item.type = 2;
			item.id = -1;
			item._typeid = ctx_bi->_typeid;

			if (sIff::getInstance().getItemGroupIdentify(ctx_bi->_typeid) == iff::MASCOT && (mascot = sIff::getInstance().findMascot(ctx_bi->_typeid)) != nullptr
				&& mascot->shop.flag_shop.time_shop.dia > 0 && mascot->shop.flag_shop.time_shop.active) {
				item.qntd = 1;
				item.flag_time = 4;
				item.STDA_C_ITEM_QNTD = 1;
				item.STDA_C_ITEM_TIME = (short)ctx_bi->qntd;
			}else {
				item.qntd = ctx_bi->qntd;
				item.STDA_C_ITEM_QNTD = (short)item.qntd;
			}

			if (MailBoxManager::sendMessageWithItem(0, _session.m_pi.uid, msg, item) <= 0)
				throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
						+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu colocar o item ganho no mailbox do player. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 13, 0x6300113));

			_smp::message_pool::getInstance().push(new message("[BoxSystem::BoxMail][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] abriu Box[TYPEID=" + std::to_string(pWi->_typeid) + "] e ganhou o Item[TYPEID="
					+ std::to_string(ctx_bi->_typeid) + ", QNTD=" + std::to_string(ctx_bi->qntd) + ", RARIDADE=" + std::to_string((short)ctx_bi->raridade) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0xA7);

			p.addUint8((unsigned char)v_item.size());

			for (auto& el : v_item) {
				p.addUint32(el._typeid);
				p.addInt32(el.id);
				p.addUint16((const unsigned short)el.stat.qntd_dep);
			}

			packet_func::session_send(p, &_session, 1);

			p.init_plain((unsigned short)0xAA);

			p.addUint16(0);

			p.addUint64(_session.m_pi.ui.pang);
			p.addUint64(_session.m_pi.cookie);

			packet_func::session_send(p, &_session, 1);

			p.init_plain((unsigned short)0x19D);

			p.addUint32(0);

			p.addUint32(box->_typeid);
			p.addUint32(ctx_bi->_typeid);
			p.addUint32(ctx_bi->qntd);

			packet_func::session_send(p, &_session, 1);

			break;
		}
		}

		if (ctx_bi != nullptr && ctx_bi->raridade > 0)
			snmdb::NormalManagerDB::getInstance().add(22, new CmdInsertBoxRareWinLog(_session.m_pi.uid, box->_typeid, *ctx_bi), channel::SQLDBResponse, this);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestOpenBoxMail][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x19D);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x6300100);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestOpenBoxMyRoom(player& _session, packet *_packet) {
	REQUEST_BEGIN("OpenBoxMyRoom");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("OpenBoxMyRoom");

		if (!sBoxSystem::getInstance().isLoad())
			sBoxSystem::getInstance().load();

		uint32_t box_typeid = _packet->readUint32();

		if (box_typeid == 0)
			throw exception("[channel::requestOpenBoxMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(box_typeid) + "], mas o typeid eh invalido(zero). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6300201));

		auto pWi = _session.m_pi.findWarehouseItemByTypeid(box_typeid);

		if (pWi == nullptr)
			throw exception("[channel::requestOpenBoxMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(box_typeid) + "], mas ele nao tem essa Box. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0x6300202));

		if (pWi->STDA_C_ITEM_QNTD < 1)
			throw exception("[channel::requestOpenBoxMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas ele nao tem quantidade suficiente da Box[value="
					+ std::to_string(pWi->STDA_C_ITEM_QNTD) + ", request=1]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0x6300203));

		if (sIff::getInstance().getItemGroupIdentify(pWi->_typeid) != iff::ITEM)
			throw exception("[channel::requestOpenBoxMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao eh uma Box valida. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4, 0x6300204));

		auto item_iff = sIff::getInstance().findItem(pWi->_typeid);

		if (item_iff == nullptr)
			throw exception("[channel::requestOpenBoxMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao tem essa Box no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 5, 0x6300205));

		auto box = sBoxSystem::getInstance().findBox(pWi->_typeid);

		if (box == nullptr)
			throw exception("[channel::requestOpenBoxMyRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao tem essa Box no Box System do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 6, 0x6300206));

		std::vector< stItem > v_item;
		stItem item{ 0 }, stBox{ 0 };

		ctx_box_item *ctx_bi = nullptr;

		ctx_bi = sBoxSystem::getInstance().drawBox(_session, *box);

		if (ctx_bi == nullptr)
			throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu sortear um Box Item. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 9, 0x6300209));

		BuyItem bi{ 0 };
		IFF::Mascot *mascot = nullptr;

		item.clear();

		bi.id = -1;
		bi._typeid = ctx_bi->_typeid;

		if (sIff::getInstance().getItemGroupIdentify(ctx_bi->_typeid) == iff::MASCOT && (mascot = sIff::getInstance().findMascot(ctx_bi->_typeid)) != nullptr
			&& mascot->shop.flag_shop.time_shop.dia > 0 && mascot->shop.flag_shop.time_shop.active) {
			bi.qntd = 1;
			bi.time = (unsigned short)ctx_bi->qntd;
		}else
			bi.qntd = ctx_bi->qntd;

		item_manager::initItemFromBuyItem(_session.m_pi , item, bi, false, 0, 0, 1);

		if (item._typeid == 0)
			throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu inicializar o Item[TYPEID=" + std::to_string(bi._typeid) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 11, 0x6300211));

		if ((sIff::getInstance().IsCanOverlapped(item._typeid) && sIff::getInstance().getItemGroupIdentify(item._typeid) != iff::CAD_ITEM) || !_session.m_pi.ownerItem(item._typeid)) {
			if (item_manager::isSetItem(item._typeid)) {
				auto v_stItem = item_manager::getItemOfSetItem(_session, item._typeid, false, 1 );

				if (!v_stItem.empty()) {

					for (auto& el : v_stItem)
						if ((sIff::getInstance().IsCanOverlapped(el._typeid) && sIff::getInstance().getItemGroupIdentify(el._typeid) != iff::CAD_ITEM) || !_session.m_pi.ownerItem(el._typeid))
							v_item.push_back(el);
				}else
					throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
							+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas SetItem que ele ganhou da box, nao tem Item[TYPEID=" + std::to_string(bi._typeid) + "]. Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 12, 0x6300212));
			}else
				v_item.push_back(item);

		}else if (sIff::getInstance().getItemGroupIdentify(item._typeid) == iff::CAD_ITEM)
			throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas o CaddieItem que ele ganhou, nao tem o caddie, Item[TYPEID=" + std::to_string(bi._typeid) + "]. Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 13, 0x6300213));
		else
			throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas ele ja tem o Item[TYPEID=" + std::to_string(bi._typeid) + "]. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 14, 0x6300214));

		stBox.clear();

		stBox.type = 2;
		stBox.id = pWi->id;
		stBox._typeid = box->_typeid;
		stBox.qntd = 1;
		stBox.STDA_C_ITEM_QNTD = (short)stBox.qntd * -1;

		if (item_manager::removeItem(stBox, _session) <= 0)
			throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
				+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas nao conseguiu deletar Box. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0x6300210));

		std::string str = "";

		auto rai = item_manager::addItem(v_item, _session, 0, 0);

		if (rai.fails.size() > 0 && rai.type != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH) {

			for (auto i = 0u; i < v_item.size(); ++i) {
				if (i == 0)
					str += "[TYPEID=" + std::to_string(v_item[i]._typeid) + ", ID=" + std::to_string(v_item[i].id) + ", QNTD=" + std::to_string((v_item[i].qntd > 0xFFu) ? v_item[i].qntd : v_item[i].STDA_C_ITEM_QNTD)
					+ (v_item[i].STDA_C_ITEM_TIME > 0 ? ", TEMPO=" + std::to_string(v_item[i].STDA_C_ITEM_TIME) : std::string("")) + "]";
				else
					str += ", [TYPEID=""" + std::to_string(v_item[i]._typeid) + ", ID=" + std::to_string(v_item[i].id) + ", QNTD=" + std::to_string((v_item[i].qntd > 0xFFu) ? v_item[i].qntd : v_item[i].STDA_C_ITEM_QNTD)
					+ (v_item[i].STDA_C_ITEM_TIME > 0 ? ", TEMPO=" + std::to_string(v_item[i].STDA_C_ITEM_TIME) : std::string("")) + "]";
			}

			throw exception("[channel::requestOpenBoxMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Box[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "], mas ele nao conseguiu adicionar os item(ns){" + str + "}. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 15, 0x6300215));
		}else {

			for (auto i = 0u; i < v_item.size(); ++i) {
				if (i == 0)
					str += "[TYPEID=" + std::to_string(v_item[i]._typeid) + ", ID=" + std::to_string(v_item[i].id) + ", QNTD=" + std::to_string((v_item[i].qntd > 0xFFu) ? v_item[i].qntd : v_item[i].STDA_C_ITEM_QNTD)
					+ (v_item[i].STDA_C_ITEM_TIME > 0 ? ", TEMPO=" + std::to_string(v_item[i].STDA_C_ITEM_TIME) : std::string("")) + "]";
				else
					str += ", [TYPEID=""" + std::to_string(v_item[i]._typeid) + ", ID=" + std::to_string(v_item[i].id) + ", QNTD=" + std::to_string((v_item[i].qntd > 0xFFu) ? v_item[i].qntd : v_item[i].STDA_C_ITEM_QNTD)
					+ (v_item[i].STDA_C_ITEM_TIME > 0 ? ", TEMPO=" + std::to_string(v_item[i].STDA_C_ITEM_TIME) : std::string("")) + "]";
			}
		}

		if (ctx_bi != nullptr && ctx_bi->raridade > 0)
			snmdb::NormalManagerDB::getInstance().add(22, new CmdInsertBoxRareWinLog(_session.m_pi.uid, box->_typeid, *ctx_bi), channel::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[BoxSystem::BoxMyRoom][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] abriu Box e ganhou o Item(ns){" + str + "} [TYPEID="
				+ std::to_string(ctx_bi->_typeid) + ", QNTD=" + std::to_string(ctx_bi->qntd) + ", RARIDADE=" + std::to_string((short)ctx_bi->raridade) + "]", CL_FILE_LOG_AND_CONSOLE));

		for (auto& el : v_item) {
			p.init_plain((unsigned short)0xAA);

			p.addUint16(1);

			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint16(el.STDA_C_ITEM_TIME);
			p.addUint8(el.flag_time);
			p.addUint16((unsigned short)el.stat.qntd_dep);
			p.addBuffer(&el.date.date.sysDate[1], sizeof(SYSTEMTIME));
			p.addBuffer(&el.ucc.IDX, sizeof(item.ucc.IDX));

			p.addUint64(_session.m_pi.ui.pang);
			p.addUint64(_session.m_pi.cookie);

			packet_func::session_send(p, &_session, 1);
		}

		p.init_plain((unsigned short)0x129);

		p.addUint8(0);

		p.addUint32(box->_typeid);
		p.addUint32(stBox.stat.qntd_dep);

		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addZeroByte(8);
		}

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestOpenBoxMyRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x129);

		p.addUint8(1);

		p.addZeroByte(12);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestPlayMemorial(player& _session, packet *_packet) {
	REQUEST_BEGIN("PlayMemorial");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("PlayMemorial");

		if (_session.m_pi.block_flag.m_flag.stBit.memorial_shop)
			throw exception("[channel::requestPlayerMemorial][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou jogar no Memorial Shop, mas ele nao pode. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 6, 0x790001));

		if (!sMemorialSystem::getInstance().isLoad())
			sMemorialSystem::getInstance().load();

		uint32_t coin_typeid = _packet->readUint32();

		if (coin_typeid == 0)
			throw exception("[channel::requestPlayMemorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar Memorial com a coin[TYPEID="
					+ std::to_string(coin_typeid) + "], mas o coin_typeid is invalid(zero). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x6300301));

		if (sIff::getInstance().getItemGroupIdentify(coin_typeid) != iff::ITEM)
			throw exception("[channel::requestPlayMemorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar Memorial com a coin[TYPEID="
					+ std::to_string(coin_typeid) + "], mas a coin is not Item Valid. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0x6300302));

		auto pWi = _session.m_pi.findWarehouseItemByTypeid(coin_typeid);

		if (pWi == nullptr)
			throw exception("[channel::requestPlayMemorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar Memorial com a coin[TYPEID="
					+ std::to_string(coin_typeid) + "], mas o ele nao possui a coin. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0x6300303));

		auto coin = sIff::getInstance().findItem(pWi->_typeid);

		if (coin == nullptr || !coin->active)
			throw exception("[channel::requestPlayMemorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar Memorial com a coin[TYPEID="
					+ std::to_string(coin_typeid) + "], mas nao tem a coin na IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4, 0x6300304));

		SysAchievement sys_achieve;

		auto c = sMemorialSystem::getInstance().findCoin(coin->_typeid);

		if (c == nullptr)
			throw exception("[channel::requestPlayMemorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar Memorial com a coin[TYPEID="
					+ std::to_string(coin_typeid) + "], mas nao tem essa coin no Memorial System do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 5, 0x6300305));

		if (c->tipo == MEMORIAL_COIN_TYPE::MCT_NORMAL)
			sys_achieve.incrementCounter(0x6C4000B2u );
		else if (c->tipo == MEMORIAL_COIN_TYPE::MCT_SPECIAL)
			sys_achieve.incrementCounter(0x6C4000B3u );

		auto win_item = sMemorialSystem::getInstance().drawCoin(_session, *c);

		if (win_item.empty())
			throw exception("[channel::requestPlayMemorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar Memorial com a coin[TYPEID="
					+ std::to_string(coin_typeid) + "], mas não conseguiu sortear um item do memorial shop. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 6, 0x6300306));

		std::vector< stItem > v_item;
		stItem item{ 0 };

		BuyItem bi{ 0 };
		IFF::Mascot *mascot = nullptr;

		for (auto& el : win_item) {
			bi.clear();
			item.clear();

			bi.id = -1;
			bi._typeid = el._typeid;

			if (sIff::getInstance().getItemGroupIdentify(el._typeid) == iff::MASCOT && (mascot = sIff::getInstance().findMascot(el._typeid)) != nullptr
				&& mascot->shop.flag_shop.time_shop.dia > 0 && mascot->shop.flag_shop.time_shop.active) {
				bi.qntd = 1;
				bi.time = (unsigned short)el.qntd;
			}else
				bi.qntd = el.qntd;

			item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1);

			if (item._typeid == 0)
				throw exception("[channel::requestPlayMemorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar Memorial com a coin[TYPEID="
						+ std::to_string(coin_typeid) + "], mas nao conseguiu inicializar o Item[TYPEID=" + std::to_string(bi._typeid) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 7, 0x6300307));

			if ((sIff::getInstance().IsCanOverlapped(item._typeid) && sIff::getInstance().getItemGroupIdentify(item._typeid) != iff::CAD_ITEM) || !_session.m_pi.ownerItem(item._typeid)) {
				if (item_manager::isSetItem(item._typeid)) {
					auto v_stItem = item_manager::getItemOfSetItem(_session, item._typeid, false, 1 );

					if (!v_stItem.empty()) {

						for (auto& el : v_stItem)
							if ((sIff::getInstance().IsCanOverlapped(el._typeid) && sIff::getInstance().getItemGroupIdentify(el._typeid) != iff::CAD_ITEM) || !_session.m_pi.ownerItem(el._typeid))
								v_item.push_back(el);
					}else
						throw exception("[channel::requestPlayMemorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar Memorial com a coin[TYPEID="
								+ std::to_string(coin_typeid) + "], mas SetItem que ele ganhou no Memorial Shop, nao tem Item[TYPEID=" + std::to_string(bi._typeid) + "]. Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 8, 0x6300308));
				}else
					v_item.push_back(item);

			}else if (sIff::getInstance().getItemGroupIdentify(item._typeid) == iff::CAD_ITEM)
				throw exception("[channel::requestPlayMemorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar Memorial com a coin[TYPEID="
						+ std::to_string(coin_typeid) + "], mas o CaddieItem que ele ganhou, nao tem o caddie, Item[TYPEID=" + std::to_string(bi._typeid) + "]. Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 9, 0x6300309));
			else
				throw exception("[channel::requestPlayMemorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar Memorial com a coin[TYPEID="
						+ std::to_string(coin_typeid) + "], mas ele ja tem o Item[TYPEID=" + std::to_string(bi._typeid) + "]. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 10, 0x6300310));

			if (el.tipo > 0 && el.tipo < 3)
				sys_achieve.incrementCounter(0x6C4000B5u );
			else if (el.tipo >= 3)
				sys_achieve.incrementCounter(0x6C4000B4u );
		}

		item.clear();

		item.type = 2;
		item.id = pWi->id;
		item._typeid = c->_typeid;
		item.qntd = 1;
		item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

		if (item_manager::removeItem(item, _session) <= 0)
			throw exception("[channel::requestPlayMemorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar Memorial com a coin[TYPEID="
					+ std::to_string(coin_typeid) + "], mas nao conseguiu deletar Coin. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 11, 0x6300311));

		std::string str = "";

		auto rai = item_manager::addItem(v_item, _session, 0, 0);

		if (rai.fails.size() > 0 && rai.type != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH) {

			for (auto i = 0u; i < v_item.size(); ++i) {
				if (i == 0)
					str += "[TYPEID=" + std::to_string(v_item[i]._typeid) + ", ID=" + std::to_string(v_item[i].id) + ", QNTD=" + std::to_string((v_item[i].qntd > 0xFFu) ? v_item[i].qntd : v_item[i].STDA_C_ITEM_QNTD)
						+ (v_item[i].STDA_C_ITEM_TIME > 0 ? ", TEMPO=" + std::to_string(v_item[i].STDA_C_ITEM_TIME) : std::string("")) + "]";
				else
					str += ", [TYPEID=""" + std::to_string(v_item[i]._typeid) + ", ID=" + std::to_string(v_item[i].id) + ", QNTD=" + std::to_string((v_item[i].qntd > 0xFFu) ? v_item[i].qntd : v_item[i].STDA_C_ITEM_QNTD)
						+ (v_item[i].STDA_C_ITEM_TIME > 0 ? ", TEMPO=" + std::to_string(v_item[i].STDA_C_ITEM_TIME) : std::string("")) + "]";
			}

			throw exception("[channel::requestPlayMemorial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar Memorial com a coin[TYPEID="
					+ std::to_string(coin_typeid) + "], mas ele nao conseguiu adicionar os item(ns){" + str + "}. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 12, 0x6300312));
		}else {

			for (auto i = 0u; i < v_item.size(); ++i) {
				if (i == 0)
					str += "[TYPEID=" + std::to_string(v_item[i]._typeid) + ", ID=" + std::to_string(v_item[i].id) + ", QNTD=" + std::to_string((v_item[i].qntd > 0xFFu) ? v_item[i].qntd : v_item[i].STDA_C_ITEM_QNTD)
						+ (v_item[i].STDA_C_ITEM_TIME > 0 ? ", TEMPO=" + std::to_string(v_item[i].STDA_C_ITEM_TIME) : std::string("")) + "]";
				else
					str += ", [TYPEID=""" + std::to_string(v_item[i]._typeid) + ", ID=" + std::to_string(v_item[i].id) + ", QNTD=" + std::to_string((v_item[i].qntd > 0xFFu) ? v_item[i].qntd : v_item[i].STDA_C_ITEM_QNTD)
						+ (v_item[i].STDA_C_ITEM_TIME > 0 ? ", TEMPO=" + std::to_string(v_item[i].STDA_C_ITEM_TIME) : std::string("")) + "]";
			}
		}

		v_item.push_back(item);

		if (!win_item.empty() && win_item.begin()->tipo > 0 && win_item.size() == 1)
			snmdb::NormalManagerDB::getInstance().add(24, new CmdInsertMemorialRareWinLog(_session.m_pi.uid, c->_typeid, *win_item.begin()), channel::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[MemorialSystem::Play][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] jogou Coin[TYPEID="
				+ std::to_string(c->_typeid) + "] no Memorial Shop e ganhou o Item(ns){" + str + "}", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addBuffer(&el.stat, sizeof(el.stat));
			p.addUint32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addZeroByte(25);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x264);

		p.addUint32(0);

		p.addUint32((uint32_t)win_item.size());

		for (auto& el : win_item) {
			p.addInt32(el.tipo);
			p.addUint32(el._typeid);
			p.addUint32(el.qntd);
		}

		packet_func::session_send(p, &_session, 1);

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestPlayMemorial][ErrorSystem]" + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x264);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x6300300);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestOpenCardPack(player& _session, packet* _packet) {
	REQUEST_BEGIN("OpenCardPack");

	packet p;

	try {

		SysAchievement sys_achieve;

		std::vector< stItem > v_item_add;
		std::vector< stItem* > v_item;
		stItem item{ 0 }, item_rm{ 0 };

		uint32_t _typeid = _packet->readUint32();
		uint32_t id = _packet->readInt32();

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[CardSystem::requestOpenCardPack][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] pedindo para abrir Card Pack[TYPEID="
				+ std::to_string(_typeid) + ", ID=" + std::to_string(id) + "]", CL_FILE_LOG_AND_CONSOLE));
#endif

		CHECK_SESSION_IS_AUTHORIZED("OpenCardPack");

		if (!sCardSystem::getInstance().isLoad())
			sCardSystem::getInstance().load();

		auto pCi = _session.m_pi.findCardById(id);

		if (pCi == nullptr)
			throw exception("[channel::requestOpenCardPack][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Card Pack[TYPEID="
				+ std::to_string(_typeid) + ", ID=" + std::to_string(id) + "], mas ele nao tem o Card Pack. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 102, 0x5400103));

		if (pCi->qntd < 1)
			throw exception("[channel::requestOpenCardPack][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Card Pack[TYPEID="
				+ std::to_string(_typeid) + ", ID=" + std::to_string(id) + "], mas ele nao tem quantidade[value=" + std::to_string(pCi->qntd) + ", request=1] suficiente para abrir Card Pack.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 103, 0x5400104));

		CardPack *cp = nullptr;

		cp = (sIff::getInstance().getItemSubGroupIdentify22(_typeid) == 4  ? sCardSystem::getInstance().findBoxCardPack(_typeid) : sCardSystem::getInstance().findCardPack(_typeid));

		if (cp == nullptr)
			throw exception("[channel::requestOpenCardPack][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Card Pack[TYPEID="
					+ std::to_string(_typeid) + ", ID=" + std::to_string(id) + "], mas nao tem esse Card Pack no Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 100, 0x5400101));

		auto cards = sCardSystem::getInstance().draws(*cp);

		if (cards.empty())
			throw exception("[channel::requestOpenCardPack][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Card Pack[TYPEID="
					+ std::to_string(_typeid) + ", ID=" + std::to_string(id) + "], mas nao conseguiu sortear os cards. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 101, 0x5400102));

		item_rm.clear();

		item_rm.type = 2;
		item_rm.id = pCi->id;
		item_rm._typeid = pCi->_typeid;
		item_rm.qntd = 1;
		item_rm.STDA_C_ITEM_QNTD = (short)item_rm.qntd * -1;

		if (item_manager::removeItem(item_rm, _session) <= 0)
			throw exception("[channel::requestOpenCardPack][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Card Pack[TYPEID="
					+ std::to_string(_typeid) + ", ID=" + std::to_string(id) + "], mas nao conseguiu deletar o Card Pack[TYPEID=" + std::to_string(pCi->_typeid) + ", ID="
					+ std::to_string(pCi->id) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 104, 0x5400105));

		v_item.push_back(&item_rm);

		BuyItem bi{ 0 };

		v_item_add.reserve(3);

		auto it = v_item_add.end();

		for (auto& el : cards) {

			bi.clear();
			item.clear();

			bi.id = -1;
			bi._typeid = el._typeid;
			bi.qntd = 1;

			item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1 );

			if (item._typeid == 0)
				throw exception("[channel::requestOpenCardPack][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Card Pack[TYPEID="
						+ std::to_string(_typeid) + ", ID=" + std::to_string(id) + "], mas nao conseguiu inicializar Card[TYPEID=" + std::to_string(bi._typeid) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 105, 0x5400106));

			it = VECTOR_FIND_ITEM(v_item_add, _typeid, == , item._typeid);

			if (it != v_item_add.end()) {
				it->qntd += 1;
				it->STDA_C_ITEM_QNTD = (short)it->qntd;
			}else
				it = v_item_add.insert(v_item_add.end(), item);

#if defined(_WIN32)
			v_item.push_back(it._Ptr);
#elif defined(__linux__)
			v_item.push_back(&(*it));
#endif

#define CARD_ABBOT_ELEMENTAL_SHARD 0x7C800026ul

			switch (sIff::getInstance().getItemSubGroupIdentify22(el._typeid)) {
			case IFF::Card::CARD_SUB_TYPE::T_CHARACTER:
				sys_achieve.incrementCounter(0x6C400079u );
				break;
			case IFF::Card::CARD_SUB_TYPE::T_CADDIE:
				sys_achieve.incrementCounter(0x6C40007Au );
				break;
			case IFF::Card::CARD_SUB_TYPE::T_SPECIAL:
				sys_achieve.incrementCounter(0x6C40007Bu );

				if (el._typeid == CARD_ABBOT_ELEMENTAL_SHARD)
					sys_achieve.incrementCounter(0x6C400080u );
				break;
			case IFF::Card::CARD_SUB_TYPE::T_NPC:
				sys_achieve.incrementCounter(0x6C4000A8u);
				break;
			}

			sys_achieve.incrementCounter(0x6C40007Cu + el.tipo);

		}

		auto rai = item_manager::addItem(v_item_add, _session, 0, 0);

		if (rai.fails.size() > 0 && rai.type != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH) {
			std::string ids = "";

			for (auto i = 0u; i < v_item_add.size(); ++i)
				ids += (i == 0 ? "TYPEID=" : ", TYPEID=") + std::to_string(v_item_add[i]._typeid);

			throw exception("[channel::requestOpenCardPack][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou abrir Card Pack[TYPEID="
					+ std::to_string(_typeid) + ", ID=" + std::to_string(id) + "], mas nao conseguiu adicionar o Cards[TYPEID=" + ids + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 106, 0x5400107));
		}

		sys_achieve.incrementCounter(0x6C400078u );

		p.init_plain((unsigned short)0x154);

		p.addUint32(0);

		for (auto& el : v_item) {
			p.addInt32(el->id);
			p.addUint32(el->_typeid);
			p.addZeroByte(12);

			auto subGroup = sIff::getInstance().getItemSubGroupIdentify22(el->_typeid);

			p.addUint32((subGroup == 3  || subGroup == 4 ) ? 1 : el->stat.qntd_dep);

			p.addZeroByte(32);
			p.addUint8(1);
			p.addUint8(0);

			if (subGroup == 3  || subGroup == 4 )
				p.addUint8((const unsigned char)v_item.size() - 1);
			else
				p.addUint32(1);
		}

		packet_func::session_send(p, &_session, 1);

		sys_achieve.finish_and_update(_session);

		auto ids = std::string("");

		for (auto i = 0u; i < cards.size(); ++i)
			ids += ((i == 0) ? "TYPEID=" : ", TYPEID=") + std::to_string(cards[i]._typeid);

		_smp::message_pool::getInstance().push(new message("[CardSystem::requestOpenCardPack][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] abriu Card Pack[TYPEID="
				+ std::to_string(_typeid) + ", ID=" + std::to_string(id) + "] e ganhou Card(s)[" + ids + "]", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestOpenCardPack][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x154);

		p.addUint32(1);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestLoloCardCompose(player& _session, packet *_packet) {
	REQUEST_BEGIN("LoloCardCompose");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("LoloCardCompose");

		if (_session.m_pi.block_flag.m_flag.stBit.lolo_copound_card)
			throw exception("[channel::requestLoloCardCompose][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou fundir card no Lolo Card Compose, mas ele nao pode. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 7, 0x790001));

		LoloCardComposeEx lcc{ 0 };
		std::vector< stItem > v_item;
		stItem item{ 0 };

		SysAchievement sys_achieve;

		uint64_t pang = 0ull;

		_packet->readBuffer(&lcc, sizeof(LoloCardCompose));

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		for (auto i = 0u; i < (sizeof(lcc._typeid) / sizeof(lcc._typeid[0])); ++i) {

			item.clear();

			item.type = 2;
			item._typeid = lcc._typeid[i];

			auto card_iff = sIff::getInstance().findCard(item._typeid);

			if (card_iff == nullptr)
				throw exception("[channel::requestLoloCardCompose][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fundir card(s)[TYPEID="
						+ std::to_string(lcc._typeid[0]) + ", TYPEID=" + std::to_string(lcc._typeid[1]) + ", TYPEID="
						+ std::to_string(lcc._typeid[2]) + "] no Lolo Card Compose, mas o card[TYPEID=" + std::to_string(item._typeid) + "] nao existe no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 150, 0x5400151));

			if (card_iff->tipo == CARD_TYPE::T_SECRET)
				throw exception("[channel::requestLoloCardCompose][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fundir card(s)[TYPEID="
						+ std::to_string(lcc._typeid[0]) + ", TYPEID=" + std::to_string(lcc._typeid[1]) + ", TYPEID="
						+ std::to_string(lcc._typeid[2]) + "] no Lolo Card Compose, mas nao pode fundir card secret. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 151, 0x5400152));

			auto pCi = _session.m_pi.findCardByTypeid(card_iff->_typeid);

			if (pCi == nullptr)
				throw exception("[channel::requestLoloCardCompose][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fundir card(s)[TYPEID="
						+ std::to_string(lcc._typeid[0]) + ", TYPEID=" + std::to_string(lcc._typeid[1]) + ", TYPEID="
						+ std::to_string(lcc._typeid[2]) + "] no Lolo Card Compose, mas ele nao tem esse card[TYPEID=" + std::to_string(item._typeid) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 152, 0x5400153));

			if (pCi->qntd < 1)
				throw exception("[channel::requestLoloCardCompose][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fundir card(s)[TYPEID="
						+ std::to_string(lcc._typeid[0]) + ", TYPEID=" + std::to_string(lcc._typeid[1]) + ", TYPEID="
						+ std::to_string(lcc._typeid[2]) + "] no Lolo Card Compose, mas ele nao tem quantidade suficiente do card[TYPEID="
						+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + ", QNTD=" + std::to_string(pCi->qntd) + ", request=1]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 153, 0x5400154));

			if (r != nullptr && r->checkPersonalShopItem(_session, pCi->id))
				throw exception("[channel::requestLoloCardCompose][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fundir card(s)[TYPEID="
						+ std::to_string(lcc._typeid[0]) + ", TYPEID=" + std::to_string(lcc._typeid[1]) + ", TYPEID="
						+ std::to_string(lcc._typeid[2]) + "], mas o card[TYPEID=" + std::to_string(pCi->_typeid) + ", ID="
						+ std::to_string(pCi->id) + "] esta sendo vendido no Personal shop dele. Hacker ou Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1010, 0x5201010));

			auto it = VECTOR_FIND_ITEM(v_item, id, == , pCi->id);

			if (it != v_item.end() && it->_typeid == pCi->_typeid) {
				it->qntd += 1;
				it->STDA_C_ITEM_QNTD = (short)it->qntd * -1;
			}else {
				item.id = pCi->id;
				item.qntd = 1;
				item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

				v_item.push_back(item);
			}

			lcc.tipo = card_iff->tipo;

			pang += (lcc.tipo == CARD_TYPE::T_NORMAL ? 1000 : (lcc.tipo == CARD_TYPE::T_RARE ? 2000 : (lcc.tipo == CARD_TYPE::T_SUPER_RARE ? 5000 : 1000)));
		}

		END_FIND_ROOM;

		if (pang != lcc.pang)
			throw exception("[channel::requestLoloCardCompose][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fundir card(s)[TYPEID="
					+ std::to_string(lcc._typeid[0]) + ", TYPEID=" + std::to_string(lcc._typeid[1]) + ", TYPEID="
					+ std::to_string(lcc._typeid[2]) + "] no Lolo Card Compose, mas os pang[value=" + std::to_string(pang) + ", request=" + std::to_string(lcc.pang) + "] eh diferente. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 154, 0x5400155));

		auto card = sCardSystem::getInstance().drawsLoloCardCompose(lcc);

		if (card._typeid == 0)
			throw exception("[channel::requestLoloCardCompose][ErrorSystem] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fundir card(s)[TYPEID="
					+ std::to_string(lcc._typeid[0]) + ", TYPEID=" + std::to_string(lcc._typeid[1]) + ", TYPEID="
					+ std::to_string(lcc._typeid[2]) + "] no Lolo Card Compose, mas nao conseguiu sortear um card. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 155, 0x5400156));

		if (item_manager::removeItem(v_item, _session) <= 0)
			throw exception("[channel::requestLoloCardCompose][ErrorSystem] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fundir card(s)[TYPEID="
					+ std::to_string(lcc._typeid[0]) + ", TYPEID=" + std::to_string(lcc._typeid[1]) + ", TYPEID="
					+ std::to_string(lcc._typeid[2]) + "] no Lolo Card Compose, nao conseguiu remover os cards[TYPEID="
					+ std::to_string(lcc._typeid[0]) + ", TYPEID=" + std::to_string(lcc._typeid[1]) + ", TYPEID="
					+ std::to_string(lcc._typeid[2]) + "]. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 156, 0x5400157));

		BuyItem bi{ 0 };

		bi.id = -1;
		bi._typeid = card._typeid;
		bi.qntd = 1;

		item.clear();

		item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1 );

		if (item._typeid == 0)
			throw exception("[channel::requestLoloCardCompose][ErrorSystem] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fundir card(s)[TYPEID="
					+ std::to_string(lcc._typeid[0]) + ", TYPEID=" + std::to_string(lcc._typeid[1]) + ", TYPEID="
					+ std::to_string(lcc._typeid[2]) + "] no Lolo Card Compose, mas nao conseguiu inicializar o card[TYPEID=" + std::to_string(bi._typeid) + "]. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 157, 0x5400158));

		auto rt = item_manager::RetAddItem::T_INIT_VALUE;

		if ((rt = item_manager::addItem(item, _session, 0, 0)) < 0)
			throw exception("[channel::requestLoloCardCompose][ErrorSystem] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou fundir card(s)[TYPEID="
					+ std::to_string(lcc._typeid[0]) + ", TYPEID=" + std::to_string(lcc._typeid[1]) + ", TYPEID="
					+ std::to_string(lcc._typeid[2]) + "] no Lolo Card Compose, mas nao conseguiu adicionar o card[TYPEID=" + std::to_string(item._typeid) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 158, 0x5400159));

		if (rt != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH)
			v_item.push_back(item);

		_session.m_pi.consomePang(pang);

		sys_achieve.incrementCounter(0x6C40008Au + card.tipo);

		sys_achieve.incrementCounter(0x6C400089u );

		_smp::message_pool::getInstance().push(new message("[CardSystem::LoloCardCompose][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] fundiu os cards[TYPEID="
				+ std::to_string(lcc._typeid[0]) + ", TYPEID=" + std::to_string(lcc._typeid[1]) + ", TYPEID=" + std::to_string(lcc._typeid[2]) + "] e ganhou o card[TYPEID="
				+ std::to_string(item._typeid) + ", ID=" + std::to_string(item.id) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0xC8);

		p.addUint64(_session.m_pi.ui.pang);
		p.addUint64(pang);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addBuffer(&el.stat, sizeof(el.stat));
			p.addUint32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addZeroByte(25);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x229);

		p.addUint32(card.tipo);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x22A);

		p.addUint32(0);

		p.addUint32(card._typeid);

		packet_func::session_send(p, &_session, 1);

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestLoloCardCompose][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x22A);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5400150);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestUseCardSpecial(player& _session, packet *_packet) {
	REQUEST_BEGIN("UseCardSpecial");

	packet p;

	try {

		uint32_t card_typeid = _packet->readUint32();

		CHECK_SESSION_IS_AUTHORIZED("UseCardSpecial");

		SysAchievement sys_achieve;

		stItem item{ 0 };
		CardEquipInfoEx cei{ 0 };

		if (card_typeid == 0)
			throw exception("[channel::requestUseCardSpecial][Error] player[UID=" + std::to_string(_session.m_pi.uid) +"] tentou usar card special[TYPEID="
					+ std::to_string(card_typeid) + "], mas o typeid eh invalid.(zero). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 350, 0x5500351));

		auto pCi = _session.m_pi.findCardByTypeid(card_typeid);

		if (pCi == nullptr)
			throw exception("[channel::requestUseCardSpecial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar card special[TYPEID="
					+ std::to_string(card_typeid) + "], mas ele nao tem o card. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 351, 0x5500352));

		if (pCi->qntd < 1)
			throw exception("[channel::requestUseCardSpecial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar card special[TYPEID="
					+ std::to_string(card_typeid) + "], nao tem quantidade suficiante[value=" + std::to_string(pCi->qntd) + ", request=1] de card. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 357, 0x5500358));

		auto card = sIff::getInstance().findCard(pCi->_typeid);

		if (card == nullptr)
			throw exception("[channel::requestUseCardSpecial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar card special[TYPEID="
					+ std::to_string(card_typeid) + "], mas o card nao existe no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 352, 0x5500353));

		if (sIff::getInstance().getItemSubGroupIdentify22(card->_typeid) != IFF::Card::T_SPECIAL)
			throw exception("[channel::requestUseCardSpecial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar card special[TYPEID="
					+ std::to_string(card_typeid) + "], tentou usar um card que nao eh espacial. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 353, 0x5500354));

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r != nullptr && r->checkPersonalShopItem(_session, pCi->id))
			throw exception("[channel::requestUseCardSpecial][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar card special[TYPEID=" + std::to_string(pCi->_typeid) + ", ID="
					+ std::to_string(pCi->id) + "], mas o card esta sendo vendido no Personal shop dele. Hacker ou Bug.",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1010, 0x5201010));

		END_FIND_ROOM;

		item.clear();

		item.type = 2;
		item.id = pCi->id;
		item._typeid = pCi->_typeid;
		item.qntd = 1;
		item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

		cei.index = -1;
		cei.id = pCi->id;
		cei._typeid = pCi->_typeid;
		cei.efeito = card->efeito.type;
		cei.efeito_qntd = card->efeito.qntd;
		cei.parts_typeid = 0;
		cei.parts_id = 0;
		cei.use_yn = 1;
		cei.tipo = sIff::getInstance().getItemSubGroupIdentify22(pCi->_typeid);
		cei.slot = 0;

		switch (card->efeito.type) {

		case 1:
		{
			if ((int)card->efeito.qntd <= 0)
				throw exception("[channel::requestUseCardSpecial][ErrorSystem] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar card special[TYPEID="
						+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "], mas a quantidade do efeito[TYPE=" + std::to_string(card->efeito.type) + ", QNTD="
						+ std::to_string(card->efeito.qntd) + "] eh invalida. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 356, 0x5500357));

			if (item_manager::removeItem(item, _session) <= 0)
				throw exception("[channel::requestUseCardSpecial][ErrorSystem] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar card special[TYPEID="
						+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "], mas nao conseguiu deletar o card. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 355, 0x5500356));

			_session.addExp(card->efeito.qntd );

			_smp::message_pool::getInstance().push(new message("[UseCardSpecial][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] usou card special[TYPEID="
					+ std::to_string(cei._typeid) + ", ID=" + std::to_string(cei.id) + "] com efeito[TYPE=" + std::to_string(card->efeito.type) + ", QNTD="
					+ std::to_string(card->efeito.qntd) + ", TEMPO=" + std::to_string(card->tempo) + "min]", CL_FILE_LOG_AND_CONSOLE));
			break;
		}
		case 4:
		{
			if ((int)card->efeito.qntd <= 0)
				throw exception("[channel::requestUseCardSpecial][ErrorSystem] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar card special[TYPEID="
						+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "], mas a quantidade do efeito[TYPE=" + std::to_string(card->efeito.type) + ", QNTD="
						+ std::to_string(card->efeito.qntd) + "] eh invalida. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 356, 0x5500357));

			if (item_manager::removeItem(item, _session) <= 0)
				throw exception("[channel::requestUseCardSpecial][ErrorSystem] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar card special[TYPEID="
						+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "], mas nao conseguiu deletar o card. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 355, 0x5500356));

			_session.addPang(card->efeito.qntd);

			_smp::message_pool::getInstance().push(new message("[UseCardSpecial][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] usou card special[TYPEID="
					+ std::to_string(cei._typeid) + ", ID=" + std::to_string(cei.id) + "] com efeito[TYPE=" + std::to_string(card->efeito.type) + ", QNTD="
					+ std::to_string(card->efeito.qntd) + ", TEMPO=" + std::to_string(card->tempo) + "min]", CL_FILE_LOG_AND_CONSOLE));
			break;
		}
		case 17:
		{
			if ((int)card->efeito.qntd <= 0)
				throw exception("[channel::requestUseCardSpecial][ErrorSystem] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar card special[TYPEID="
						+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "], mas a quantidade do efeito[TYPE=" + std::to_string(card->efeito.type) + ", QNTD="
						+ std::to_string(card->efeito.qntd) + "] eh invalida. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 356, 0x5500357));

			if (item_manager::removeItem(item, _session) <= 0)
				throw exception("[channel::requestUseCardSpecial][ErrorSystem] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar card special[TYPEID="
						+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "], mas nao conseguiu deletar o card. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 355, 0x5500356));

			uint64_t pang = 100  + (sRandomGen::getInstance().rIbeMt19937_64_chrono() % (card->efeito.qntd - 100 + 1));

			_session.addPang(pang);

			_smp::message_pool::getInstance().push(new message("[UseCardSpecial][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] usou card special[TYPEID="
					+ std::to_string(cei._typeid) + ", ID=" + std::to_string(cei.id) + "] com efeito[TYPE=" + std::to_string(card->efeito.type) + ", QNTD="
					+ std::to_string(card->efeito.qntd) + ", TEMPO=" + std::to_string(card->tempo) + "min]", CL_FILE_LOG_AND_CONSOLE));
			break;
		}

		case 2:
		case 3:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		{

			if (item_manager::removeItem(item, _session) <= 0)
				throw exception("[channel::requestUseCardSpecial][ErrorSystem] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar card special[TYPEID="
						+ std::to_string(pCi->_typeid) + ", ID=" + std::to_string(pCi->id) + "], mas nao conseguiu deletar o card. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 355, 0x5500356));

			auto pCei = _session.m_pi.findCardEquipedByTypeid(cei._typeid, 0, 0, sIff::getInstance().getItemSubGroupIdentify22(cei._typeid), card->efeito.type);

			if (pCei != nullptr) {

				if (pCei->_typeid != cei._typeid) {

					pCei->id = cei.id;
					pCei->_typeid = cei._typeid;
					pCei->efeito = card->efeito.type;
					pCei->efeito_qntd = card->efeito.qntd;
					pCei->tipo = sIff::getInstance().getItemSubGroupIdentify22(cei._typeid);

					GetLocalTime(&pCei->use_date);

					pCei->end_date = UnixToSystemTime(SystemTimeToUnix(pCei->use_date) + (card->tempo * 60 ));
				}else {
					time_t new_end_date = (GetLocalTimeAsUnix() > SystemTimeToUnix(pCei->end_date)) ? GetLocalTimeAsUnix() : SystemTimeToUnix(pCei->end_date);

					pCei->end_date = UnixToSystemTime((uint32_t)(new_end_date + (card->tempo * 60 )));
				}

				snmdb::NormalManagerDB::getInstance().add(17, new CmdUpdateCardSpecialTime(_session.m_pi.uid, *pCei), channel::SQLDBResponse, this);

				cei = *pCei;

			}else {

				GetLocalTime(&cei.use_date);

				cei.end_date = UnixToSystemTime(SystemTimeToUnix(cei.use_date) + (card->tempo * 60 ));

				CmdEquipCard cmd_ec(_session.m_pi.uid, cei, card->tempo, true );

				snmdb::NormalManagerDB::getInstance().add(10, &cmd_ec, nullptr, nullptr);

				cmd_ec.waitEvent();

				if (cmd_ec.getException().getCodeError() != 0)
					throw cmd_ec.getException();

				cei = cmd_ec.getInfo();

				auto it = _session.m_pi.v_cei.insert(_session.m_pi.v_cei.end(), cei);

#if defined(_WIN32)
				pCei = it._Ptr;
#elif defined(__linux__)
				pCei = &(*it);
#endif
			}

			_smp::message_pool::getInstance().push(new message("[UseCardSpecial][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] usou card special[TYPEID="
					+ std::to_string(cei._typeid) + ", ID=" + std::to_string(cei.id) + "] com efeito[TYPE=" + std::to_string(card->efeito.type) + ", QNTD="
					+ std::to_string(card->efeito.qntd) + ", TEMPO=" + std::to_string(card->tempo) + "min]", CL_FILE_LOG_AND_CONSOLE));
			break;
		}
		default:
			throw exception("[channel::requestUseCardSpecial][ErrorSystem] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar card special[TYPEID="
					+ std::to_string(cei._typeid) + ", ID=" + std::to_string(cei.id) + "], mas card efeito[TYPE=" + std::to_string(card->efeito.type)
					+ ", QNTD=" + std::to_string(card->efeito.qntd) + ", TEMPO=" + std::to_string(card->tempo) + "min] no IFF_STRUCT do Server eh desconhecido. Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 354, 0x5500355));
		}

		sys_achieve.incrementCounter(0x6C40009E );

		p.init_plain((unsigned short)0x160);

		p.addUint32(0);

		p.addInt32(cei.id);
		p.addUint32(cei._typeid);
		p.addUint32(cei.parts_typeid);
		p.addInt32(cei.parts_id);
		p.addUint32(cei.slot);
		p.addUint32(1);
		p.addBuffer(&cei.use_date, sizeof(cei.use_date));
		p.addBuffer(&cei.end_date, sizeof(cei.end_date));
		p.addUint16(0);

		packet_func::session_send(p, &_session, 1);

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestUseCardSpecial][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x160);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5500350);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestUseItemBuff(player& _session, packet *_packet) {
	REQUEST_BEGIN("UseItemBuff");

	packet p;

	try {

		uint32_t item_typeid = _packet->readUint32();

		CHECK_SESSION_IS_AUTHORIZED("UseItemBuff");

		stItem item{ 0 };
		ItemBuffEx ib{ 0 };

		if (item_typeid == 0)
			throw exception("[channel::requestUseItemBuff][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar o item[TYPEID="
					+ std::to_string(item_typeid) + "], mas typeid eh invalido(zero). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 400, 0x5500401));

		auto pWi = _session.m_pi.findWarehouseItemByTypeid(item_typeid);

		if (pWi == nullptr)
			throw exception("[channel::requestUseItemBuff][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar o item[TYPEID="
					+ std::to_string(item_typeid) + "], mas ele nao tem esse item. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 401, 0x5500402));

		if (pWi->STDA_C_ITEM_QNTD < 1)
			throw exception("[channel::requestUseItemBuff][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar o item[TYPEID="
					+ std::to_string(item_typeid) + "], mas nao tem quantidade suficiente[value=" + std::to_string(pWi->STDA_C_ITEM_QNTD) + ", request=1] do item. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 404, 0x5500405));

		auto item_iff = sIff::getInstance().findItem(item_typeid);

		if (item_iff == nullptr)
			throw exception("[channel::requestUseItemBuff][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar o item[TYPEID="
					+ std::to_string(item_typeid) + "], mas nao tem esse item no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 402, 0x5500403));

		auto tli = sIff::getInstance().findTimeLimitItem(item_typeid);

		if (tli == nullptr)
			throw exception("[channel::requestUseItemBuff][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar o item[TYPEID="
					+ std::to_string(item_typeid) + "], mas nao tem esse item na tabela de item buff no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 403, 0x5500404));

		item.clear();

		item.type = 2;
		item.id = pWi->id;
		item._typeid = pWi->_typeid;
		item.qntd = 1;
		item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

		ib.index = -1;
		ib._typeid = pWi->_typeid;
		ib.tipo = tli->type;
		ib.percent = tli->percent;
		ib.use_yn = 1;

		if (item_manager::removeItem(item, _session) <= 0)
			throw exception("[channel::requestUseItemBuff][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar o item[TYPEID="
					+ std::to_string(item_typeid) + "], mas nao conseguiu deletar  o item. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 405, 0x5500406));

		auto pIb = _session.m_pi.findItemBuff(ib._typeid);

		if (pIb != nullptr) {

			if (pIb->_typeid != ib._typeid) {

				pIb->tipo = tli->type;
				pIb->_typeid = ib._typeid;
				pIb->percent = tli->percent;

				GetLocalTime(&pIb->use_date);

				pIb->end_date = UnixToSystemTime(SystemTimeToUnix(pIb->use_date) + (tli->time * 60 ));

				pIb->tempo.setTime((uint32_t)(SystemTimeToUnix(pIb->end_date) - SystemTimeToUnix(pIb->use_date)));

			}else {

				time_t new_end_date = (GetLocalTimeAsUnix() > SystemTimeToUnix(pIb->end_date)) ? GetLocalTimeAsUnix() : SystemTimeToUnix(pIb->end_date);

				pIb->end_date = UnixToSystemTime((uint32_t)(new_end_date + (tli->time * 60 )));

				pIb->tempo.setTime((uint32_t)(SystemTimeToUnix(pIb->end_date) - SystemTimeToUnix(pIb->use_date)));
			}

			snmdb::NormalManagerDB::getInstance().add(16, new CmdUpdateItemBuff(_session.m_pi.uid, *pIb), channel::SQLDBResponse, this);

			ib = *pIb;

			_smp::message_pool::getInstance().push(new message("[UseItemBuff][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] Atualizou o tempo do Item Buff[TYPEID=" + std::to_string(ib._typeid) + ", TIPO="
					+ std::to_string(ib.tipo) + ", PERCENT=" + std::to_string(ib.percent) + "] por " + std::to_string(ib.tempo.getTime() / 60 ) + "min", CL_FILE_LOG_AND_CONSOLE));

		}else {

			GetLocalTime(&ib.use_date);

			ib.end_date = UnixToSystemTime(SystemTimeToUnix(ib.use_date) + (tli->time * 60 ));

			ib.tempo.setTime((uint32_t)(SystemTimeToUnix(ib.end_date) - SystemTimeToUnix(ib.use_date)));

			CmdUseItemBuff cmd_uib(_session.m_pi.uid, ib, tli->time, true );

			snmdb::NormalManagerDB::getInstance().add(15, &cmd_uib, nullptr, nullptr);

			cmd_uib.waitEvent();

			if (cmd_uib.getException().getCodeError() != 0)
				throw cmd_uib.getException();

			ib = cmd_uib.getInfo();

			auto it = _session.m_pi.v_ib.insert(_session.m_pi.v_ib.end(), ib);

#if defined(_WIN32)
			pIb = it._Ptr;
#elif defined(__linux__)
			pIb = &(*it);
#endif

			_smp::message_pool::getInstance().push(new message("[UseItemBuff][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] usou o Item Buff[TYPEID=" + std::to_string(ib._typeid) + ", TIPO="
					+ std::to_string(ib.tipo) + ", PERCENT=" + std::to_string(ib.percent) + "] por " + std::to_string(tli->time) + "min", CL_FILE_LOG_AND_CONSOLE));
		}

		p.init_plain((unsigned short)0x181);

		p.addUint32(2);

		p.addUint32(1);
		p.addUint32(ib._typeid);
		p.addBuffer(&ib, sizeof(ItemBuff));

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestUseItemBuff][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x181);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5500400);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestCometRefill(player& _session, packet *_packet) {
	REQUEST_BEGIN("CometRefill");

	packet p;

	try {

		uint32_t item_typeid = _packet->readUint32();
		uint32_t ball_typeid = _packet->readUint32();

		CHECK_SESSION_IS_AUTHORIZED("CometRefill");

		if (!sCometRefillSystem::getInstance().isLoad())
			sCometRefillSystem::getInstance().load();

		auto pBall = _session.m_pi.findWarehouseItemByTypeid(ball_typeid);
		auto pItem = _session.m_pi.findWarehouseItemByTypeid(item_typeid);

		if (pBall == nullptr)
			throw exception("[channel::requestCometRefill][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou repreencher a Ball[TYPEID="
					+ std::to_string(ball_typeid) + "] com o Item[TYPEID=" + std::to_string(item_typeid) + "], mas ele nao tem a ball. Hacker ou Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5600101));

		if (pItem == nullptr)
			throw exception("[channel::requestCometRefill][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou repreencher a Ball[TYPEID="
					+ std::to_string(ball_typeid) + ", ID=" + std::to_string(pBall->id) + "] com o Item[TYPEID=" + std::to_string(item_typeid) + "], mas ele nao tem o item. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0x5600102));

		if (pItem->STDA_C_ITEM_QNTD < 1)
			throw exception("[channel::requestCometRefill][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou repreencher a Ball[TYPEID="
					+ std::to_string(ball_typeid) + ", ID=" + std::to_string(pBall->id) + "]] com o Item[TYPEID=" + std::to_string(item_typeid) + ", ID=" + std::to_string(pItem->id) + "], mas ele nao tem quantidade suficiente[value="
					+ std::to_string(pItem->STDA_C_ITEM_QNTD) + ", request=1] do item. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 5, 0x5600105));

		auto item_iff = sIff::getInstance().findItem(item_typeid);

		if (item_iff == nullptr)
			throw exception("[channel::requestCometRefill][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou repreencher a Ball[TYPEID="
					+ std::to_string(ball_typeid) + ", ID=" + std::to_string(pBall->id) + "] com o Item[TYPEID=" + std::to_string(item_typeid) + ", ID=" + std::to_string(pItem->id) + "], mas o item nao existe no IFF_STRUCT do Server. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0x5600103));

		auto ball_iff = sIff::getInstance().findBall(ball_typeid);

		if (ball_iff == nullptr)
			throw exception("[channel::requestCometRefill][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou repreencher a Ball[TYPEID="
				+ std::to_string(ball_typeid) + ", ID=" + std::to_string(pBall->id) + "] com o Item[TYPEID=" + std::to_string(item_typeid) + ", ID=" + std::to_string(pItem->id) + "], mas o ball nao existe no IFF_STRUCT do Server. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4, 0x5600104));

		auto ctx_cr = sCometRefillSystem::getInstance().findCometRefill(pItem->_typeid);

		if (ctx_cr == nullptr)
			throw exception("[channel::requestCometRefill][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou repreencher a Ball[TYPEID="
					+ std::to_string(ball_typeid) + ", ID=" + std::to_string(pBall->id) + "] com o Item[TYPEID=" + std::to_string(item_typeid) + ", ID="
					+ std::to_string(pItem->id) + "], mas nao tem o Comet Refill no sistema. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 8, 0x5600100));

		auto qntd = sCometRefillSystem::getInstance().drawsCometRefill(*ctx_cr);

		stItem item{ 0 };

		item.type = 2;
		item.id = pItem->id;
		item._typeid = pItem->_typeid;
		item.qntd = 1;
		item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

		if (item_manager::removeItem(item, _session) <= 0)
			throw exception("[channel::requestCometRefill][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou repreencher a Ball[TYPEID="
					+ std::to_string(ball_typeid) + ", ID=" + std::to_string(pBall->id) + "] com o Item[TYPEID=" + std::to_string(item_typeid) + ", ID=" + std::to_string(pItem->id) + "], mas ele nao conseguiu deletar o item. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 6, 0x5600106));

		item.clear();

		item.type = 2;
		item.id = pBall->id;
		item._typeid = pBall->_typeid;
		item.qntd = qntd;
		item.STDA_C_ITEM_QNTD = (short)item.qntd;

		auto rt = item_manager::RetAddItem::T_INIT_VALUE;

		if ((rt = item_manager::addItem(item, _session, 0, 0)) < 0)
			throw exception("[channel::requestCometRefill][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou repreencher a Ball[TYPEID="
					+ std::to_string(ball_typeid) + ", ID=" + std::to_string(pBall->id) + "] com o Item[TYPEID=" + std::to_string(item_typeid) + ", ID=" + std::to_string(pItem->id) + "], mas nao conseguiu atualizar quantidade da ball. Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 7, 0x5600107));

		_smp::message_pool::getInstance().push(new message("[CometRefill][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] Repreencheu a Ball[TYPEID="
				+ std::to_string(pBall->_typeid) + ", ID=" + std::to_string(pBall->id) + ", QNTD={ant: " + std::to_string(item.stat.qntd_ant) + ", dep: "
				+ std::to_string(item.stat.qntd_dep) + ", qntd: " + std::to_string(qntd) + "}] com o Item[TYPEID=" + std::to_string(pItem->_typeid) + ", ID=" + std::to_string(pItem->id) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x197);

		p.addUint8(1);

		p.addUint32(pItem->_typeid);
		p.addUint32(pBall->_typeid);
		p.addUint16(pBall->STDA_C_ITEM_QNTD);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestCometRefill][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x197);

		p.addInt8(0);

		p.addZeroByte(10);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestOpenMailBox(player& _session, packet *_packet) {
	REQUEST_BEGIN("OpenMailBox");

	packet p;

	try {

		if (_session.m_pi.block_flag.m_flag.stBit.mail_box)
			throw exception("[channel::requestOpenMailBox][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou abrir Mail Box, mas ele nao pode. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 5, 0x790001));

		int32_t pagina = _packet->readInt32();

		if (pagina <= 0)
			throw exception("[channel::requestOpenMailBox][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou abrir Mail Box[Pagina=" + std::to_string(pagina)
					+ "], mas a pagina eh invalida.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 6, 0x790002));

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[channel::requestOpenMailBox][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "]\tRequest Pagina: " + std::to_string(pagina) + " MailBox", CL_FILE_LOG_AND_CONSOLE));
#endif

		CHECK_SESSION_IS_AUTHORIZED("OpenMailBox");

		auto mails = _session.m_pi.m_mail_box.getPage(pagina);

		if (!mails.empty()) {

#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("[channel::requestOpenMailBox][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] abriu o MailBox[Pagina=" + std::to_string(pagina) + "] com sucesso.", CL_FILE_LOG_AND_CONSOLE));
#else
			_smp::message_pool::getInstance().push(new message("[channel::requestOpenMailBox][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] abriu o MailBox[Pagina=" + std::to_string(pagina) + "] com sucesso.", CL_ONLY_FILE_LOG));
#endif

			packet_func::pacote211(p, &_session, mails, pagina, _session.m_pi.m_mail_box.getTotalPages() );
			packet_func::session_send(p, &_session, 1);

		}else {
			packet_func::pacote211(p, &_session, std::vector< MailBox >(), pagina, 1);
			packet_func::session_send(p, &_session, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestOpenMailBox][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x211);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5500200);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestInfoMail(player& _session, packet *_packet) {
	REQUEST_BEGIN("InfoMail");

	packet p;

	try {

		int32_t email_id = _packet->readInt32();

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[channel::requestInfoMail][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "]\tRequest Email Info: " + std::to_string(email_id), CL_FILE_LOG_AND_CONSOLE));
#endif

		CHECK_SESSION_IS_AUTHORIZED("InfoMail");

		auto email = _session.m_pi.m_mail_box.getEmailInfo(email_id);

		if (email.id == 0)
			throw exception("[channel::requestInfoMail][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "] pediu para ver o info do Mail[ID=" + std::to_string(email_id)
					+ "], mais ele nao existe no banco de dados. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 0x5500251, 1));

		try {
			item_manager::checkSetItemOnEmail(_session, email);
		}catch (exception& e) {

			if (!STDA_ERROR_CHECK_SOURCE_AND_ERROR(e.getCodeError(), STDA_ERROR_TYPE::_ITEM_MANAGER, 20))
				throw;
		}

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[channel::requestInfoMail][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "] pediu info do Mail[ID="
				+ std::to_string(email.id) + "] com sucesso.", CL_FILE_LOG_AND_CONSOLE));
#else
		_smp::message_pool::getInstance().push(new message("[channel::requestInfoMail][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "] pediu info do Mail[ID="
				+ std::to_string(email.id) + "] com sucesso.", CL_ONLY_FILE_LOG));
#endif

		packet_func::pacote212(p, &_session, email);
		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestInfoMail][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x212);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5500250);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestSendMail(player& _session, packet *_packet) {
	REQUEST_BEGIN("SendMail");

	packet p;

	try {

		uint32_t from_uid = _packet->readUint32();
		uint32_t to_uid = _packet->readUint32();
		std::string to_nick = _packet->readString();
		unsigned short unknown_opt = _packet->readUint16();
		std::string to_msg = _packet->readString();
		uint64_t pang_price = _packet->readUint64();
		unsigned char count_item = _packet->readUint8();

		CHECK_SESSION_IS_AUTHORIZED("SendMail");

		if (to_msg.empty())
			throw exception("[channel::requestSendMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar email para o player[UID="
					+ std::to_string(to_uid) + "] sem nenhum recardo, a msg nao pode ser vazia", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PACKET_FUNC_SV, 160, 5100091));

		if (count_item > 0) {

			if (count_item > 4)
				throw exception("[channel::requestSendMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar um numero[value="
						+ std::to_string(count_item) + "] de itens eh maior que o permitido. Bug ou Hacker", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PACKET_FUNC_SV, 150, 5100081));

			if (pang_price != (count_item * 500))
				throw exception("[channel::requestSendMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar pang price[value_client="
						+ std::to_string(count_item) + ", value_srv=" + std::to_string(count_item * 500) + "] send message is wrong. Bug ou Hacker", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PACKET_FUNC_SV, 153, 5100084));

			EmailInfo::item aItem[4];
			std::vector< stItem > v_item;
			stItem item{ 0 };

			_packet->readBuffer(aItem, sizeof(EmailInfo::item) * count_item);

			IFF::Base *pBase = nullptr;

			BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

			for (auto i = 0u; i < count_item; ++i) {

				auto group = sIff::getInstance().getItemGroupIdentify(aItem[i]._typeid);

				if (group != iff::BALL && group != iff::CLUBSET && group != iff::ITEM && group != iff::PART)
					throw exception("[channel::requestSendMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar um item[TYPEID="
							+ std::to_string(aItem[i]._typeid) + ", ID=" + std::to_string(aItem[i].id) + "] para o player[UID=" + std::to_string(to_uid) + "], mas esse item nao pode ser enviado. Hacker ou Bug",
							STDA_MAKE_ERROR(STDA_ERROR_TYPE::PACKET_FUNC_SV, 154, 5100085));

				pBase = sIff::getInstance().findCommomItem(aItem[i]._typeid);

				if (pBase == nullptr)
					throw exception("[channel::requestSendMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar um item[TYPEID="
							+ std::to_string(aItem[i]._typeid) + ", ID=" + std::to_string(aItem[i].id) + "] para o player[UID=" + std::to_string(to_uid) + "], mas esse item nao tem no STRUCT IFF do server. Bug ou Hacker",
							STDA_MAKE_ERROR(STDA_ERROR_TYPE::PACKET_FUNC_SV, 151, 5100082));

				if (!pBase->shop.flag_shop.uFlagShop.stFlagShop.can_send_mail_and_personal_shop)
					throw exception("[channel::requestSendMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar um item[TYPEID="
							+ std::to_string(aItem[i]._typeid) + ", ID=" + std::to_string(aItem[i].id) + "] para o player[UID=" + std::to_string(to_uid) + "], mas esse item nao eh permitido ser enviado por mail. Bug ou Hacker",
							STDA_MAKE_ERROR(STDA_ERROR_TYPE::PACKET_FUNC_SV, 152, 5100083));

				if (!sIff::getInstance().IsCanOverlapped(pBase->_typeid) && item_manager::ownerItem(to_uid, pBase->_typeid))
					throw exception("[channel::requestSendMail][Error][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar um item[TYPEID="
							+ std::to_string(pBase->_typeid) + ", ID=" + std::to_string(aItem[i].id) + "] que o outro player[UID=" + std::to_string(to_uid) + "] ja tem esse item.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PACKET_FUNC_SV, 156, 5100087));

				item.clear();

				auto pWi = _session.m_pi.findWarehouseItemByTypeid(aItem[i]._typeid);

				if (pWi == nullptr)
					throw exception("[channel::requestSendMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar um item[TYPEID="
							+ std::to_string(aItem[i]._typeid) + ", ID=" + std::to_string(aItem[i].id) + "] para o player[UID=" + std::to_string(to_uid) + "], mas ele nao tem esse item. Bug ou Hacker",
							STDA_MAKE_ERROR(STDA_ERROR_TYPE::PACKET_FUNC_SV, 157, 5100088));

				if (r != nullptr && r->checkPersonalShopItem(_session, aItem[i].id))
					throw exception("[channel::requestSendMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar o item[TYPEID=" + std::to_string(aItem[i]._typeid) + ", ID="
							+ std::to_string(aItem[i].id) + "] para o player[UID=" + std::to_string(to_uid) + "], mas o item esta sendo vendido no Personal shop dele. Hacker ou Bug.",
							STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1010, 0x5201010));

				if (group == iff::ITEM) {

					if (aItem[i].qntd > 99)
						throw exception("[channel::requestSendMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar um item[TYPEID="
								+ std::to_string(aItem[i]._typeid) + ", ID=" + std::to_string(aItem[i].id) + "] para o player[UID=" + std::to_string(to_uid) + "], mas a quantidade[value="
								+ std::to_string(aItem[i].qntd) + "] maior que 99. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PACKET_FUNC_SV, 155, 5100086));

					if (pWi->STDA_C_ITEM_QNTD < aItem[i].qntd)
						throw exception("[channel::requestSendMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar um item[TYPEID="
								+ std::to_string(aItem[i]._typeid) + ", ID=" + std::to_string(aItem[i].id) + "] para o player[UID=" + std::to_string(to_uid) + "], mas ele nao tem quantidade[value="
								+ std::to_string(pWi->STDA_C_ITEM_QNTD) + ", req=" + std::to_string(aItem[i].qntd) + "] suficiente. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PACKET_FUNC_SV, 158, 5100089));
				}

				item.id = aItem[i].id;
				item._typeid = aItem[i]._typeid;
				item.flag_time = aItem[i].flag_time;
				item.STDA_C_ITEM_QNTD = (short)(item.qntd = aItem[i].qntd);
				item.STDA_C_ITEM_TIME = (short)aItem[i].tempo_qntd;
#if defined(_WIN32)
				memcpy_s(item.ucc.IDX, sizeof(item.ucc.IDX), aItem[i].ucc_img_mark, sizeof(item.ucc.IDX));
#elif defined(__linux__)
				memcpy(item.ucc.IDX, aItem[i].ucc_img_mark, sizeof(item.ucc.IDX));
#endif

				item.type = 2;

				v_item.push_back(item);
			}

			END_FIND_ROOM;

			if (item_manager::giveItem(v_item, _session, 1) <= 0)
				throw exception("[channel::requestSendMail][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] nao conseguiu presentear o player[UID=" + std::to_string(to_uid) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PACKET_FUNC_SV, 159, 5100090));

			packet_func::pacote216(p, &_session, v_item);
			packet_func::session_send(p, &_session, 1);

			auto msg_id = MailBoxManager::sendMessageWithItem(from_uid, to_uid, to_msg, aItem, count_item);

			_session.m_pi.consomePang(pang_price);

			std::string log_itens = "";

			for (auto el : v_item) {

				if (!log_itens.empty())
					log_itens += ", ";

				log_itens += "[TYPEID=" + std::to_string(el._typeid) + ", ID=" + std::to_string(el.id) + ", FLAG_TIME="
					+ std::to_string((unsigned short)el.flag_time) + ", QNTD="
					+ std::to_string((el.STDA_C_ITEM_TIME > 0 ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD))
					+ ", QNTD_DEPOIS=" + std::to_string(el.stat.qntd_dep) + "]";
			}

#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("[channel::requestSendMail][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] enviou presente para o Player[UID=" + std::to_string(to_uid) + "] MailBox[Email_ID=" + std::to_string(msg_id) + ", Message=" + to_msg
					+ "] item(ns)[QNTD=" + std::to_string(v_item.size()) + "] Item(ns){" + log_itens + "}", CL_FILE_LOG_AND_CONSOLE));
#else
			_smp::message_pool::getInstance().push(new message("[channel::requestSendMail][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] enviou presente para o Player[UID=" + std::to_string(to_uid) + "] MailBox[Email_ID=" + std::to_string(msg_id) + ", Message=" + to_msg
					+ "] item(ns)[QNTD=" + std::to_string(v_item.size()) + "] Item(ns){" + log_itens + "}", CL_ONLY_FILE_LOG));
#endif

			p.init_plain((unsigned short)0xC8);

			p.addUint64(_session.m_pi.ui.pang);
			p.addUint64(pang_price);

			packet_func::session_send(p, &_session, 1);

			p.init_plain((unsigned short)0x213);

			p.addUint32(0);

			packet_func::session_send(p, &_session, 1);

		}else {

			if (pang_price != 100)
				throw exception("[channel::requestSendMail][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar pang price[value_client="
						+ std::to_string(count_item) + ", value_srv=" + std::to_string(100) + "] send message is wrong. Bug ou Hacker", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PACKET_FUNC_SV, 153, 5100084));

			auto msg_id = MailBoxManager::sendMessage(from_uid, to_uid, to_msg);

			_session.m_pi.consomePang(pang_price);

#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("[channel::requestSendMail][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] enviou presente para o Player[UID=" + std::to_string(to_uid) + "] MailBox[Email_ID=" + std::to_string(msg_id) + ", Message=" + to_msg + "]", CL_FILE_LOG_AND_CONSOLE));
#else
			_smp::message_pool::getInstance().push(new message("[channel::requestSendMail][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] enviou presente para o Player[UID=" + std::to_string(to_uid) + "] MailBox[Email_ID=" + std::to_string(msg_id) + ", Message=" + to_msg + "]", CL_ONLY_FILE_LOG));
#endif

			p.init_plain((unsigned short)0xC8);

			p.addUint64(_session.m_pi.ui.pang);
			p.addUint64(pang_price);

			packet_func::session_send(p, &_session, 1);

			p.init_plain((unsigned short)0x213);

			p.addUint32(0);

			packet_func::session_send(p, &_session, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestSendMail][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x213);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5500300);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestTakeItemFomMail(player& _session, packet *_packet) {
	REQUEST_BEGIN("TakeItemFromMail");

	packet p;

	try {

		int32_t email_id = _packet->readInt32();

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[channel::requestTakeItemFrom][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
				+ "]\tMove Item from Email to armario: " + std::to_string(email_id), CL_FILE_LOG_AND_CONSOLE));
#endif

		CHECK_SESSION_IS_AUTHORIZED("TakeItemFromMail");

		unsigned short tmp_level = _session.m_pi.level;

		auto ei = _session.m_pi.m_mail_box.getEmailInfo(email_id, false);

		std::vector< stItem > v_item;

		if (!ei.itens.empty() ) {

			stItem item = { 0 };

			for (auto i = 0u; i < ei.itens.size(); ++i) {

				item.clear();

				item_manager::initItemFromEmailItem(_session.m_pi, item, ei.itens[i]);

				if (item._typeid == 0) {

					_smp::message_pool::getInstance().push(new message("[channel::requestTakeItemFrom][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
							+"] tentou inicializar o item que pegou do mailbox[MAIL_ID=" + std::to_string(email_id) + "].", CL_FILE_LOG_AND_CONSOLE));

					packet_func::pacote214(p, &_session, 3);
					packet_func::session_send(p, &_session, 1);

					return;
				}

				if ((sIff::getInstance().IsCanOverlapped(ei.itens[i]._typeid) && sIff::getInstance().getItemGroupIdentify(ei.itens[i]._typeid) != iff::CAD_ITEM) || !_session.m_pi.ownerItem(ei.itens[i]._typeid, 1 )) {

					if (item_manager::isSetItem(item._typeid)) {

						auto v_stItem = item_manager::getItemOfSetItem(_session, ei.itens[i]._typeid, false, 1 );

						if (!v_stItem.empty()) {

							for (auto& el : v_stItem)
								if ((sIff::getInstance().IsCanOverlapped(el._typeid) && sIff::getInstance().getItemGroupIdentify(el._typeid) != iff::CAD_ITEM) || !_session.m_pi.ownerItem(el._typeid, 1 ))
									v_item.push_back(el);
						}else
							_smp::message_pool::getInstance().push(new message("[channel::requestTakeItemFrom][Log] player[UID=" + std::to_string(_session.m_pi.uid)
									+ "] tentou add set item sem item dentro, do MailBox[MAIL_ID=" + std::to_string(email_id) + "]", CL_FILE_LOG_AND_CONSOLE));
					}else
						v_item.push_back(item);

				}else if (sIff::getInstance().getItemGroupIdentify(ei.itens[i]._typeid) == iff::CAD_ITEM) {
					throw exception("[channel::requestTakeItemFrom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou pegar um CaddieItem[TYPEID="
							+ std::to_string(ei.itens[i]._typeid) + "] do Mail[ID=" + std::to_string(email_id) + "] de um caddie que ele nao possui", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PACKET_FUNC_SV, 201, 5100072));
				}else
					throw exception("[channel::requestTakeItemFrom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou pegar um item[TYPEID="
							+ std::to_string(ei.itens[i]._typeid) + "] do Mail[ID=" + std::to_string(email_id) + "] que ele ja possui, nao pode ter duplicatas", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PACKET_FUNC_SV, 201, 5100071));
			}

			_session.m_pi.m_mail_box.leftItensFromEmail(email_id);

			auto rai = item_manager::addItem(v_item, _session, 1 , 0);

			if (rai.fails.size() > 0 && rai.type != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH) {

				for (auto& fail : rai.fails)
					_smp::message_pool::getInstance().push(new message("[channel::requestTakeItemFrom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou mover o item[TYPEID="
							+ std::to_string(fail._typeid) + ", ID=" + std::to_string(fail.id) + "] do MailBox[EMAIL_ID=" + std::to_string(email_id) + "] para o MyRoom, mas nao conseguiu. Bug", CL_FILE_LOG_AND_CONSOLE));

				packet_func::pacote214(p, &_session, 2);
				packet_func::session_send(p, &_session, 1);

				return;
			}

			std::string log_itens = "";

			for (auto el : v_item) {

				if (!log_itens.empty())
					log_itens += ", ";

				log_itens += "[TYPEID=" + std::to_string(el._typeid) + ", ID=" + std::to_string(el.id) + ", FLAG_TIME="
					+ std::to_string((unsigned short)el.flag_time) + ", QNTD="
					+ std::to_string((el.STDA_C_ITEM_TIME > 0 ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD))
					+ ", QNTD_DEPOIS=" + std::to_string(el.stat.qntd_dep) + "]";
			}

#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("[channel::requestTakeItemFromMail][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] pegou item(ns)[QNTD=" + std::to_string(v_item.size()) + "] do MailBox[Email_ID=" + std::to_string(email_id) + "] Item(ns){" + log_itens + "}", CL_FILE_LOG_AND_CONSOLE));
#else
			_smp::message_pool::getInstance().push(new message("[channel::requestTakeItemFromMail][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] pegou item(ns)[QNTD=" + std::to_string(v_item.size()) + "] do MailBox[Email_ID=" + std::to_string(email_id) + "] Item(ns){" + log_itens + "}", CL_ONLY_FILE_LOG));
#endif

			packet_func::pacote216(p, &_session, v_item);
			packet_func::session_send(p, &_session, 1);

			packet_func::pacote214(p, &_session);
			packet_func::session_send(p, &_session, 1);

			if (tmp_level != _session.m_pi.level) {

				updatePlayerInfo(_session);

				if (_session.m_pi.lobby != (unsigned char)~0u) {

					auto pi = getPlayerInfo(&_session);

					if (pi != nullptr && packet_func::pacote046(p, &_session, std::vector< PlayerCanalInfo > { *pi }, 3))
						packet_func::channel_broadcast(*this, p, 1);
				}
			}

		}else {

			packet_func::pacote214(p, &_session, 1);
			packet_func::session_send(p, &_session, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestTakeItemFromMail][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		packet_func::pacote214(p, &_session, (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5500100);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestDeleteMail(player& _session, packet *_packet) {
	REQUEST_BEGIN("DeleteMail");

	packet p;
	int32_t *a_email_id = nullptr;

	try {

		CHECK_SESSION_IS_AUTHORIZED("DeleteMail");

		size_t num_email = _packet->readUint32();
		size_t pagina = 1;

		a_email_id = new int32_t[num_email];

		_packet->readBuffer(a_email_id, sizeof(int32_t) * num_email);

		pagina = _packet->readUint32();

		if ((int)pagina <= 0)
			throw exception("[channel::requestDeleteMail][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] pedeiu para deletar email(s)[COUNT=" + std::to_string(num_email) + "] da pagina("
					+ std::to_string((int)pagina) + "), mas a pagina eh invalida.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 6, 0x791002));

		std::string ids = "";

		for (size_t i = 0u; i < num_email; ++i) {
			if (i == 0)
				ids += std::to_string(a_email_id[i]);
			else
				ids += ", " + std::to_string(a_email_id[i]);
		}

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[channel::requestDeleteMail][Log] Player: " + std::to_string(_session.m_pi.uid)
				+ "\tRequest delete email(s) Pagina: " + std::to_string(pagina)
				+ " Email Count: " + std::to_string(num_email) + " Email(s): " + ids, CL_FILE_LOG_AND_CONSOLE));
#else
		_smp::message_pool::getInstance().push(new message("[channel::requestDeleteMail][Log] Player: " + std::to_string(_session.m_pi.uid)
				+ "\tRequest delete email(s) Pagina: " + std::to_string(pagina)
				+ " Email Count: " + std::to_string(num_email) + " Email(s): " + ids, CL_ONLY_FILE_LOG));
#endif

		_session.m_pi.m_mail_box.deleteEmail(a_email_id, (uint32_t)num_email);

		auto mails = _session.m_pi.m_mail_box.getPage((uint32_t)pagina);

		if (!mails.empty()) {

			packet_func::pacote215(p, &_session, mails, (uint32_t)pagina, _session.m_pi.m_mail_box.getTotalPages() );
			packet_func::session_send(p, &_session, 1);

		}else {
			packet_func::pacote215(p, &_session, std::vector< MailBox >(), (uint32_t)pagina, 1);
			packet_func::session_send(p, &_session, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestDeleteMail][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x215);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5500150);

		packet_func::session_send(p, &_session, 1);
	}

	if (a_email_id != nullptr)
		delete[] a_email_id;
};

void channel::requestMakePassDolfiniLocker(player& _session, packet *_packet) {
	REQUEST_BEGIN("MakePassDolfiniLocker");

	packet p;

	try {

		std::string pass = _packet->readString();

		CHECK_SESSION_IS_AUTHORIZED("MakePassDolfiniLocker");

		if (pass.empty())
			throw exception("[channel::requestMakePassDolfiniLocker][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentrou trocar a senha do dolfini locker com senha vazia. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 200, 5100101));

		if (pass.size() > 4)
			throw exception("[channel::requestMakePassDolfiniLocker][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar a senha do dolfini locker com um senha maior do que o permitido. Hacker ou Bug",
				STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 201, 5100102));

#if defined(_WIN32)
		memcpy_s(_session.m_pi.df.pass, sizeof(_session.m_pi.df.pass), pass.c_str(), sizeof(_session.m_pi.df.pass));
#elif defined(__linux__)
		memcpy(_session.m_pi.df.pass, pass.c_str(), sizeof(_session.m_pi.df.pass));
#endif

		_smp::message_pool::getInstance().push(new message("[Dolfini Locker::MakePass][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "] Criou a senha[value=" + pass + "] do Dolfini Locker com sucesso.", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x176);

		p.addUint32(0);

		packet_func::session_send(p, &_session, 1);

		snmdb::NormalManagerDB::getInstance().add(1, new CmdUpdateDolfiniLockerPass(_session.m_pi.uid, pass), channel::SQLDBResponse, this);

	}catch (exception& e) {
		_smp::message_pool::getInstance().push(new message("[channel::requestMakePassDolfiniLocker][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x176);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 5100100);

		packet_func::session_send(p, &_session, 1);
	}

};

void channel::requestCheckDolfiniLockerPass(player& _session, packet *_packet) {
	REQUEST_BEGIN("CheckDolfiniLockerPass");

	packet p;

	try {

		std::string pass = _packet->readString();

		CHECK_SESSION_IS_AUTHORIZED("CheckDolfiniLockerPass");

		if (pass.empty())
			throw exception("[channel::requestCheckDolfiniLockerPass][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou entrar no Dolfini Locker com uma senha vazia. Hacker ou Bug.",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 250, 5100151));

		if (pass.size() > 4)
			throw exception("[channel::requestCheckDolfiniLockerPass][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou entrar no Dolfini Locker com uma senha maior que a suportada. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 251, 5100152));

		p.init_plain((unsigned short)0x16C);

		if (pass.compare(_session.m_pi.df.pass) != 0) {
			_smp::message_pool::getInstance().push(new message("[channel::requesCheckDolfiniLockerPass][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou entrar no Dolfini Locker com senha[value=" + pass + "] errada", CL_FILE_LOG_AND_CONSOLE));

			p.addUint32(0x75);
		}else {
			_smp::message_pool::getInstance().push(new message("[channel::requesCheckDolfiniLockerPass][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "] logou com sucesso no Dolfini Locker", CL_FILE_LOG_AND_CONSOLE));

			_session.m_pi.df.pass_check = 1;

			p.addUint32(0);
		}

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestDolfiniLockerPass][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x16C);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 5100150);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestChangeDolfiniLockerPass(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChangeDolfiniLockerPass");

	packet p;

	try {

		std::string old_pass = _packet->readString();
		std::string new_pass = _packet->readString();

		CHECK_SESSION_IS_AUTHORIZED("ChangeDolfiniLockerPass");

		if (old_pass.empty() || new_pass.empty())
			throw exception("[channel::requestChangeDolfiniLocker][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar a senha, mas old_pass[value="
					+ old_pass + "] or new_pass[value=" + new_pass + "] is empty. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 300, 5100201));

		if (old_pass.size() > 4 || new_pass.size() > 4)
			throw exception("[channel::requestChangeDolfiniLocker][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar a senha, mas old_pass[value="
					+ old_pass + "] or new_pass[value=" + new_pass + "] length is hight of permited. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 301, 5100202));

		p.init_plain((unsigned short)0x174);

		if (old_pass.compare(_session.m_pi.df.pass) != 0) {
			_smp::message_pool::getInstance().push(new message("[Dolfini Locker::Change Pass][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar a senha mas a senha[value=" + old_pass + "] antiga esta incorreta", CL_FILE_LOG_AND_CONSOLE));

			p.addUint32(1);
		}else {

#if defined(_WIN32)
			memcpy_s(_session.m_pi.df.pass, sizeof(_session.m_pi.df.pass), new_pass.c_str(), sizeof(_session.m_pi.df.pass));
#elif defined(__linux__)
			memcpy(_session.m_pi.df.pass, new_pass.c_str(), sizeof(_session.m_pi.df.pass));
#endif

			p.addUint32(0);

			_smp::message_pool::getInstance().push(new message("[Dolfini Locker::Change Pass][Log] player[UID=" + std::to_string(_session.m_pi.uid) +"] trocou a senha[old=" + old_pass + ", new=" + new_pass + "] com sucesso", CL_FILE_LOG_AND_CONSOLE));

			snmdb::NormalManagerDB::getInstance().add(1, new CmdUpdateDolfiniLockerPass(_session.m_pi.uid, new_pass), channel::SQLDBResponse, this);
		}

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::DolfiniLockerPass][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x174);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 5100200);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestChangeDolfiniLockerModeEnter(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChangeDolfiniLockerModeEnter");

	packet p;

	try {

		unsigned char locker = _packet->readUint8();
		std::string pass = _packet->readString();

		CHECK_SESSION_IS_AUTHORIZED("ChangeDolfiniLockerModeEnter");

		if (pass.empty())
			throw exception("[channel::requestChangeDolfiniLockerModeEnter][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar o modo de entrar no dolfini locker, mas a senha fornecida esta vazia. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 350, 5100251));

		if (pass.size() > 4)
			throw exception("[channel::requestChangeDolfiniLockerModeEnter][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar o modo de entrar no dolfini locker, mas o tamanho da senha eh maior que o permitido. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 351, 5100252));

		p.init_plain((unsigned short)0x173);

		if (pass.compare(_session.m_pi.df.pass) != 0) {
			_smp::message_pool::getInstance().push(new message("[Dolfini Locker::Change Mode Enter][Log] senha[value=" + pass + "] fornecida incorreta, nao combina com a do player[UID=" + std::to_string(_session.m_pi.uid) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.addUint32(1);
		}else {

			_session.m_pi.df.locker = locker;

			p.addUint32(0);

			_smp::message_pool::getInstance().push(new message("[Dolfini Locker::Change Mode Enter][Log] ", CL_FILE_LOG_AND_CONSOLE));

			snmdb::NormalManagerDB::getInstance().add(2, new CmdUpdateDolfiniLockerMode(_session.m_pi.uid, locker), channel::SQLDBResponse, this);
		}

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::ChangeDolfiniLockerModeEnter][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x173);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 5100250);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestDolfiniLockerItem(player& _session, packet *_packet) {
	REQUEST_BEGIN("DolfiniLockerItem");

	packet p;

	try {

#define DL_LIMIT_ITEM_PER_PAGE	20

		uint32_t opt = _packet->readUint32();
		unsigned short pagina = _packet->readUint16();

		CHECK_SESSION_IS_AUTHORIZED("DolfiniLockerItem");

		unsigned short paginas = 0u;
		uint32_t index = 0u;
		unsigned char count = 0u;

		auto num_item = _session.m_pi.df.v_item.size();

		paginas = (num_item % DL_LIMIT_ITEM_PER_PAGE == 0) ? (unsigned short)num_item / DL_LIMIT_ITEM_PER_PAGE : (unsigned short)num_item / DL_LIMIT_ITEM_PER_PAGE + 1;

		if (num_item > 0 && pagina > paginas)
			throw exception("[channel::requestDolfiniLockerItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou acessa a pagina[value=" + std::to_string(pagina) + "] que nao existe. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 400, 5100300));

		index = (pagina - 1) * DL_LIMIT_ITEM_PER_PAGE;

		count = ((index + DL_LIMIT_ITEM_PER_PAGE) > _session.m_pi.df.v_item.size()) ? (unsigned char)(_session.m_pi.df.v_item.size() - index) : DL_LIMIT_ITEM_PER_PAGE;

		p.init_plain((unsigned short)0x16D);

		p.addUint16(paginas);
		p.addUint16((num_item > 0) ? pagina : 0);
		p.addUint8(count);

		for (auto i = index; i < (index + count); ++i)
			p.addBuffer(&_session.m_pi.df.v_item[i], sizeof(DolfiniLockerItem));

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestDolfiniLockerItem][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x16D);

		p.addZeroByte(5);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestDolfiniLockerPang(player& _session, packet *_packet) {
	REQUEST_BEGIN("DolfiniLockerPang");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("DolfiniLockerPang");

		p.init_plain((unsigned short)0x172);

		p.addUint64(_session.m_pi.df.pang);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestDolfiniLockerPang][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x172);

		p.addUint64(0);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestUpdateDolfiniLockerPang(player& _session, packet *_packet) {
	REQUEST_BEGIN("UpdateDolfiniLockerPang");

	packet p;

	try {

		unsigned char opt = _packet->readUint8();
		uint64_t pang = _packet->readUint64();

		CHECK_SESSION_IS_AUTHORIZED("UpdateDolfiniLockerPang");

		if (opt == 1) {

			if (pang > _session.m_pi.ui.pang)
				throw exception("[channel::requestUpdateDolfiniLockerPang][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou colocar pang(s)[value="
					+ std::to_string(pang) + "] que ele nao tem no Dolfini Locker. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 451, 5100352));

			_session.m_pi.df.pang += pang;

			_session.m_pi.consomePang(pang);

		}else if (opt == 0) {

			if (pang > _session.m_pi.df.pang)
				throw exception("[channel::requestUpdateDolfiniLockerPang][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou tirar pang(s)[value="
						+ std::to_string(pang) + "] que ele nao tem no Dolfini Locker. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 452, 5100353));

			_session.m_pi.df.pang -= pang;

			_session.m_pi.addPang(pang);

		}else
			throw exception("[channel::requestUpdateDolfiniLockerPang][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou colocar ou tirar pangs do Dolfini Locker com um opt[value="
					+ std::to_string((unsigned short)opt) + "] desconhecide. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 450, 5100351));

		_smp::message_pool::getInstance().push(new message("[Dolfini Locker::Update pang][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] Atualizou Pang[value="
				+ std::to_string(pang) + ", OPTION=" + std::to_string((unsigned short)opt) + "] com sucesso.", CL_FILE_LOG_AND_CONSOLE));

		snmdb::NormalManagerDB::getInstance().add(3, new CmdUpdateDolfiniLockerPang(_session.m_pi.uid, _session.m_pi.df.pang), channel::SQLDBResponse, this);

		p.init_plain((unsigned short)0x171);

		p.addUint32(0);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0xC8);

		p.addUint64(_session.m_pi.ui.pang);
		p.addUint64(pang);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x172);

		p.addUint64(_session.m_pi.df.pang);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestUpdateDolfiniLockerPang][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x171);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 5100350);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestAddDolfiniLockerItem(player& _session, packet *_packet) {
	REQUEST_BEGIN("AddDolfiniLockerItem");

	packet p;

	DolfiniLockerItem *aTI = nullptr;

	try {

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("Packet[ID=0xCE] Hex.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getBuffer(), _packet->getSize()), CL_FILE_LOG_AND_CONSOLE));
#endif

		unsigned char count = _packet->readUint8();
		aTI = new DolfiniLockerItem[count];

		_packet->readBuffer(aTI, sizeof(DolfiniLockerItem) * count);

		CHECK_SESSION_IS_AUTHORIZED("AddDolfiniLockerItem");

		uint32_t char_typeid = 0u;
		uint32_t i = 0u;

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		for (i = 0u; i < count; ++i) {

			if (r != nullptr && r->checkPersonalShopItem(_session, aTI[i].item.id))
				throw exception("[channel::requestAddDolfiniLockerItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou colocar o item[TYPEID=" + std::to_string(aTI[i].item._typeid) + ", ID="
						+ std::to_string(aTI[i].item.id) + "] no Dolfini Locker, mas o item esta sendo vendido no Personal shop dele. Hacker ou Bug.",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1010, 0x5201010));

			if (sIff::getInstance().getItemGroupIdentify(aTI[i].item._typeid) != iff::PART)
				throw exception("[channel::requestAddDolfiniLockerItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou colocar um item[TYPEID="
						+ std::to_string(aTI[i].item._typeid) + "] no Dolfini Locker que nao eh um IFF::PART.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 500, 109  ));

			auto part = sIff::getInstance().findPart(aTI[i].item._typeid);

			if (part == nullptr)
				throw exception("[channel::requestAddDolfiniLockerItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou colocar um item[TYPEID="
						+ std::to_string(aTI[i].item._typeid) + ", ID=" + std::to_string(aTI[i].item.id) +"] no Dolfini Locker que nao tem no IFF_STRUCT do server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 504, 5100405));

			if (part->type_item == 8  || part->type_item == 9 )
				throw exception("[channel::requestAddDolfiniLockerItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou colocar um Self Design Original/Copy item[TYPEID="
						+ std::to_string(aTI[i].item._typeid) + ", ID=" + std::to_string(aTI[i].item.id) + "] no Dolfini Locker, mas nao eh permitido", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 505, 5100406));

			char_typeid = (iff::CHARACTER << 26) | sIff::getInstance().getItemCharIdentify(aTI[i].item._typeid);

			auto character = _session.m_pi.findCharacterByTypeid(char_typeid);

			if (character != nullptr) {
				auto part_num = sIff::getInstance().getItemCharPartNumber(aTI[i].item._typeid);

				if (character->parts_id[part_num] == aTI[i].item.id && character->parts_typeid[part_num] == aTI[i].item._typeid)
					throw exception("[channel::requestAddDolfiniLockerItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou colocar um item[TYPEID="
							+ std::to_string(aTI[i].item._typeid) + ", ID=" + std::to_string(aTI[i].item.id) + "] equipado Part[num=" + std::to_string(part_num) + "] no Dolfini Locker. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 501, 5100402));
			}

			auto it = _session.m_pi.findWarehouseItemItById(aTI[i].item.id);

			if (it == _session.m_pi.mp_wi.end())
				throw exception("[channel::reuqestAddDolfiniLockerItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou colocar um item[TYPEID="
						+ std::to_string(aTI[i].item._typeid) + ", ID=" + std::to_string(aTI[i].item.id) + "] no Dolfini Locker que ele nao tem. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 502, 5100403));

			CmdAddDolfiniLockerItem cmd_adli(_session.m_pi.uid, aTI[i], true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_adli, nullptr, nullptr);

			cmd_adli.waitEvent();

			if (cmd_adli.getException().getCodeError() != 0) {
				_smp::message_pool::getInstance().push(new message("[channel::requestAddDolfiniLockerItem][Error] " + cmd_adli.getException().getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

				if (i < (count - 1u))
					aTI[i] = aTI[i + 1];

				i--;
				count--;

				continue;
			}

			aTI[i] = cmd_adli.getInfo();

			if (aTI[i].index == ~0ull) {
				_smp::message_pool::getInstance().push(new message("[channel::requestAddDolfiniLockerItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao conseguiu add o item[TYPEID="
						+ std::to_string(aTI[i].item._typeid) + ", ID=" + std::to_string(aTI[i].item.id) + "] no Dolfini Locker no DB", CL_FILE_LOG_AND_CONSOLE));

				if (i < (count - 1u))
					aTI[i] = aTI[i + 1];

				i--;
				count--;

				continue;
			}

			_session.m_pi.mp_wi.erase(it);

			_session.m_pi.df.v_item.insert(_session.m_pi.df.v_item.begin(), aTI[i]);

			_smp::message_pool::getInstance().push(new message("[Dolfini Locker::AddItem][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] Adicionou Item[TYPEID="
					+ std::to_string(aTI[i].item._typeid) + ", ID=" + std::to_string(aTI[i].item.id) + "] no Dolfini Locker com sucesso", CL_FILE_LOG_AND_CONSOLE));
		}

		END_FIND_ROOM;

		if (count == 0)
			throw exception("[channel::requestAddDolfiniLockerItem][Error] nenhum item passou nas verificacoes, player[UID=" + std::to_string(_session.m_pi.uid) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 503, 5100404));

		p.init_plain((unsigned short)0x139);

		p.addUint16(0);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0xEC);

		p.addUint32(count);

		p.addUint8(1);

		p.addUint64(0);

		p.addUint32(0);

		for (i = 0u; i < count; ++i)
			p.addBuffer(&aTI[i].item, sizeof(TradeItem));

		packet_func::session_send(p, &_session, 1);

		for (i = 0u; i < count; ++i) {
			p.init_plain((unsigned short)0x16E);

			p.addUint32(0);

			p.addUint64(0 );

			p.addBuffer(&aTI[i].item, sizeof(TradeItem));

			packet_func::session_send(p, &_session, 1);
		}

		if (aTI != nullptr)
			delete[] aTI;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestAddDolfiniLockerItem][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x16E);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 5100400);

		packet_func::session_send(p, &_session, 1);

		if (aTI != nullptr)
			delete[] aTI;
	}
};

void channel::requestRemoveDolfiniLockerItem(player& _session, packet *_packet) {
	REQUEST_BEGIN("RemoveDolfiniLockerItem");

	packet p;

	DolfiniLockerItem *aTI = nullptr;
	WarehouseItemEx *aWi = nullptr;

	try {

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("Packet[ID=0xCF] Hex.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getBuffer(), _packet->getSize()), CL_FILE_LOG_AND_CONSOLE));
#endif

		unsigned char count = _packet->readUint8();
		aTI = new DolfiniLockerItem[count];
		aWi = new WarehouseItemEx[count];

		_packet->readBuffer(aTI, sizeof(DolfiniLockerItem) * count);

		CHECK_SESSION_IS_AUTHORIZED("RemoveDolfiniLockerItem");

		uint32_t char_typeid = 0u;
		uint32_t i = 0u;

		for (i = 0u; i < count; ++i) {

			auto ii = VECTOR_FIND_ITEM(_session.m_pi.df.v_item , item.id, == , aTI[i].item.id);

			if (ii == _session.m_pi.df.v_item.end())
				throw exception("[channel::reuqestRemoveDolfiniLockerItem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou tirar um item[TYPEID="
					+ std::to_string(aTI[i].item._typeid) + ", ID=" + std::to_string(aTI[i].item.id) + "] que ele nao tem. Do Dolfini Locker ", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 550, 5100451));

			snmdb::NormalManagerDB::getInstance().add(4, new CmdDeleteDolfiniLockerItem(_session.m_pi.uid, aTI[i].index), channel::SQLDBResponse, this);

			_session.m_pi.df.v_item.erase(ii);

			aWi[i].clear();

			aWi[i].id = aTI[i].item.id;
			aWi[i]._typeid = aTI[i].item._typeid;
			aWi[i].ano = -1;
			aWi[i].STDA_C_ITEM_QNTD = 1;
			aWi[i].purchase = 1;
			aWi[i].type = 2;
			aWi[i].clubset_workshop.level = - 1;

#if defined(_WIN32)
			memcpy_s(aWi[i].ucc.name, sizeof(aWi[i].ucc.name), aTI[i].item.sd_name, sizeof(aWi[i].ucc.name));
			memcpy_s(aWi[i].ucc.idx, sizeof(aWi[i].ucc.idx), aTI[i].item.sd_idx, sizeof(aWi[i].ucc.idx));
			memcpy_s(aWi[i].ucc.copier_nick, sizeof(aWi[i].ucc.copier_nick), aTI[i].item.sd_copier_nick, sizeof(aWi[i].ucc.copier_nick));
#elif defined(__linux__)
			memcpy(aWi[i].ucc.name, aTI[i].item.sd_name, sizeof(aWi[i].ucc.name));
			memcpy(aWi[i].ucc.idx, aTI[i].item.sd_idx, sizeof(aWi[i].ucc.idx));
			memcpy(aWi[i].ucc.copier_nick, aTI[i].item.sd_copier_nick, sizeof(aWi[i].ucc.copier_nick));
#endif
			aWi[i].ucc.seq = aTI[i].item.sd_seq;
			aWi[i].ucc.status = aTI[i].item.sd_status;

			_session.m_pi.mp_wi.insert(std::make_pair(aWi[i].id, aWi[i]));

			_smp::message_pool::getInstance().push(new message("[Dolfini Locker::RemoveItem][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] removeu o Item[TYPEID="
					+ std::to_string(aWi[i]._typeid) + ", ID=" + std::to_string(aWi[i].id) + "] do Dolfini Locker com sucesso", CL_FILE_LOG_AND_CONSOLE));
		}

		if (count == 0)
			throw exception("[channel::requestRemoveDolfiniLockerItem][Error] nenhum item passou nas verificacoes, player[UID=" + std::to_string(_session.m_pi.uid) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 503, 5100404));

		p.init_plain((unsigned short)0xEC);

		p.addUint32(count);

		p.addUint8(0);

		p.addUint64(_session.m_pi.ui.pang);

		p.addUint32(0);

		for (i = 0u; i < count; ++i) {

			p.addBuffer(&aTI[i].item, sizeof(TradeItem));

			p.addUint8(3);

			p.addBuffer(&aWi[i], sizeof(WarehouseItem));
		}

		packet_func::session_send(p, &_session, 1);

		for (i = 0u; i < count; ++i) {
			p.init_plain((unsigned short)0x16F);

			p.addUint32(0);

			p.addBuffer(&aTI[i], sizeof(DolfiniLockerItem));

			packet_func::session_send(p, &_session, 1);
		}

		if (aTI != nullptr)
			delete[] aTI;

		if (aWi != nullptr)
			delete[] aWi;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestRemoveDolfiniLockerItem][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x16F);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 5100450);

		packet_func::session_send(p, &_session, 1);

		if (aTI != nullptr)
			delete[] aTI;

		if (aWi != nullptr)
			delete[] aWi;
	}
};

void channel::requestOpenLegacyTikiShop(player& _session, packet* _packet) {
	REQUEST_BEGIN("OpenLegacyTikiShop");

	packet p;

	try {

#ifdef _DEBUG

		_smp::message_pool::getInstance().push(new message("[channel::requestOpenLegacyTikiShop][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] request open Point Shop(Tiki Shop antigo).", CL_FILE_LOG_AND_CONSOLE));

		_smp::message_pool::getInstance().push(new message("[channel::requestOpenLegacyTikiShop][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
				+ "]. Packet raw: " + hex_util::BufferToHexString(_packet->getBuffer(), _packet->getSize()), CL_FILE_LOG_AND_CONSOLE));
#endif

		CHECK_SESSION_IS_AUTHORIZED("OpenLegacyTikiShop");

		if (_session.m_pi.block_flag.m_flag.stBit.legacy_tiki_shop)
			throw exception("[channel::requestOpenLegacyTikiShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] esta bloqueado no Legacy Tiki Shop.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4000, 1));

		p.init_plain((unsigned short)0x1E7);

		p.addUint32(0u);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestOpenLegacyTikiShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x1E7);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 1u);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestPointLegacyTikiShop(player& _session, packet* _packet) {
	REQUEST_BEGIN("PointLegacyTikiShop");

	packet p;

	try {

#ifdef _DEBUG

		_smp::message_pool::getInstance().push(new message("[channel::requestPointLegacyTikiShop][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] request TP from Point Shop(Tiki Shop antigo).", CL_FILE_LOG_AND_CONSOLE));

		_smp::message_pool::getInstance().push(new message("[channel::requestPointLegacyTikiShop][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
				+ "]. Packet raw: " + hex_util::BufferToHexString(_packet->getBuffer(), _packet->getSize()), CL_FILE_LOG_AND_CONSOLE));
#endif

		CHECK_SESSION_IS_AUTHORIZED("PointLegacyTikiShop");

		if (_session.m_pi.block_flag.m_flag.stBit.legacy_tiki_shop)
			throw exception("[channel::requestOpenLerequestPointLegacyTikiShopgacyTikiShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] esta bloqueado no Legacy Tiki Shop.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4000, 1));

		p.init_plain((unsigned short)0x1E8);

		p.addUint32(0u);

		p.addUint32((uint32_t)_session.m_pi.m_legacy_tiki_pts);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestPointLegacyTikiShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x1E8);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 1u);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestExchangeTPByItemLegacyTikiShop(player& _session, packet* _packet) {
	REQUEST_BEGIN("ExchangeTPByItemLegacyTikiShop");

#if defined(__linux__)
#pragma pack(1)
#endif

	struct stLegacyTikiShopExchangeItem {
		void clear() { memset(this, 0, sizeof(stLegacyTikiShopExchangeItem)); };
		uint32_t _typeid;
		int32_t id;
		int32_t qntd;
		uint32_t value_tp;
	};

#if defined(__linux__)
#pragma pack()
#endif

	packet p;

	try {

#ifdef _DEBUG

		_smp::message_pool::getInstance().push(new message("[channel::requestExchangeTPByItemLegacyTikiShop][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] request exchange TP by Item in Point Shop(Tiki Shop antigo).", CL_FILE_LOG_AND_CONSOLE));

		_smp::message_pool::getInstance().push(new message("[channel::requestExchangeTPByItemLegacyTikiShop][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
				+ "]. Packet raw: " + hex_util::BufferToHexString(_packet->getBuffer(), _packet->getSize()), CL_FILE_LOG_AND_CONSOLE));
#endif

		CHECK_SESSION_IS_AUTHORIZED("ExchangeTPByItemLegacyTikiShop");

		if (_session.m_pi.block_flag.m_flag.stBit.legacy_tiki_shop)
			throw exception("[channel::requestExchangeTPByItemLegacyTikiShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] esta bloqueado no Legacy Tiki Shop.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4000, 1));

		constexpr auto getNumberItensPerTikiShopPts = [](IFF::TikiShopDados& _tiki) {

			std::pair< uint32_t , uint32_t  > ret{ 0u, 0u };

			ret.first = (_tiki.qnt_per_tikis_pts == 0u) ? 1u : _tiki.qnt_per_tikis_pts;

			ret.second = (_tiki.tiki_pts == 0u) ? 1u : _tiki.tiki_pts;

			return ret;
		};

		uint32_t tiki_pts = 0u;

		std::string s_item = "";
		std::string s_ids = "";

		stLegacyTikiShopExchangeItem tsei{ 0 };

		std::vector< stItem > v_item;
		stItem item{ 0 };

		SysAchievement sys_achieve;

		uint32_t count = _packet->readUint8();

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		for (auto i = 0u; i < count; ++i) {
			tsei.clear();

			_packet->readBuffer(&tsei, sizeof(stLegacyTikiShopExchangeItem));

			auto base = sIff::getInstance().findCommomItem(tsei._typeid);

			if (base == nullptr)
				throw exception("[channel::ExchangeTPByItemLegacyTikiShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar item[TYPEID="
						+ std::to_string(tsei._typeid) + ", ID=" + std::to_string(tsei.id) + "] no Tiki's Shop, mas o item nao existe no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 901, 0x5200902));

			if (!base->tiki.isActived())
				throw exception("[channel::ExchangeTPByItemLegacyTikiShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar item[TYPEID="
						+ std::to_string(tsei._typeid) + ", ID=" + std::to_string(tsei.id) + "] no Tiki's Shop, mas o item nao eh valido para ser trocado. Hacker ou Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 904, 0x5200905));

			auto dados_tiki = getNumberItensPerTikiShopPts(base->tiki);

			auto item = item_manager::exchangeTikiShop(_session, tsei._typeid, tsei.id, dados_tiki.first * tsei.qntd);

			if (item.empty())
				throw exception("[channel::ExchangeTPByItemLegacyTikiShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar item[TYPEID="
						+ std::to_string(tsei._typeid) + ", ID=" + std::to_string(tsei.id) + ", QNTD=" + std::to_string(tsei.qntd) + "] no Tiki's Shop, mas nao conseguiu inicializar o item. Hacker ou Bug.",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 900, 0x52000901));

			if (r != nullptr && r->checkPersonalShopItem(_session, tsei.id))
				throw exception("[channel::ExchangeTPByItemLegacyTikiShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar item[TYPEID=" + std::to_string(tsei._typeid) + ", ID="
						+ std::to_string(tsei.id) + ", QNTD=" + std::to_string(tsei.qntd) + "] no Tiki's Shop, mas o item esta sendo vendido no Personal shop dele. Hacker ou Bug.",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1010, 0x5201010));

			tiki_pts += dados_tiki.second * tsei.qntd;

			v_item.insert(v_item.end(), item.begin(), item.end());

			s_ids = "";

			for (auto ii = 0u; ii < item.size(); ++ii)
				s_ids += std::string((ii == 0) ? "" : ", ") + std::to_string(item[ii].id);

			s_item += std::string((i == 0) ? "" : ", ") + "[TYPEID=" + std::to_string(tsei._typeid) + ", ID(s)={" + s_ids + "}, QNTD=" + std::to_string(tsei.qntd)
					+ ", QNTD_REAL=" + std::to_string(dados_tiki.first * tsei.qntd) + ", TIPO(Normal, CP, Rare)=" + std::to_string(base->tiki.tipo_tiki_shop) +"]";
		}

		END_FIND_ROOM;

		if (tiki_pts == 0u)
			throw exception("[channel::ExchangeTPByItemLegacyTikiShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou trocar item(ns)(" + s_item + "), mas ocorreu um erro na inicializacao do Tiki Points from IFF_STRUCT is invalid("
					+ std::to_string(tiki_pts) + ").", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 905, 0x5200905));

		if (item_manager::removeItem(v_item, _session) <= 0)
			throw exception("[channel::ExchangeTPByItemLegacyTikiShop][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou trocar item(ns)(" + s_item + "), mas nao conseguiu deletar ele(s).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 902, 0x5200903));

		_session.m_pi.m_legacy_tiki_pts += tiki_pts;

		snmdb::NormalManagerDB::getInstance().add(28, new CmdUpdateLegacyTikiShopPoint(_session.m_pi.uid, _session.m_pi.m_legacy_tiki_pts), channel::SQLDBResponse, this);

		sys_achieve.incrementCounter(0x6C400086u , 1);

		_smp::message_pool::getInstance().push(new message("[ExchangeTPByItemLegacyTikiShop][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] player trocou item(ns)("
				+ s_item + ") por Tiki Point[value=" + std::to_string(tiki_pts) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addUint32(el.stat.qntd_ant);
			p.addUint32(el.stat.qntd_dep);
			p.addUint32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addZeroByte(25);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x1E9);

		p.addUint32(0u);
		p.addUint32((uint32_t)_session.m_pi.m_legacy_tiki_pts);

		packet_func::session_send(p, &_session, 1);

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestExchangeTPByItemLegacyTikiShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x1E9);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 1u);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestExchangeItemByTPLegacyTikiShop(player& _session, packet* _packet) {
	REQUEST_BEGIN("ExchangeItemByTPLegacyTikiShop");

#if defined(__linux__)
#pragma pack(1)
#endif

	struct stLegacyTikiShopExchangeTP {
		void clear() { memset(this, 0, sizeof(stLegacyTikiShopExchangeTP)); };
		uint32_t _typeid;
		int32_t qntd;
		uint32_t tp;
	};

#if defined(__linux__)
#pragma pack()
#endif

	packet p;

	try {

#ifdef _DEBUG

		_smp::message_pool::getInstance().push(new message("[channel::requestExchangeItemByTPLegacyTikiShop][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] request exchange Item By TP in Point Shop(Tiki Shop antigo).", CL_FILE_LOG_AND_CONSOLE));

		_smp::message_pool::getInstance().push(new message("[channel::requestExchangeItemByTPLegacyTikiShop][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
				+ "]. Packet raw: " + hex_util::BufferToHexString(_packet->getBuffer(), _packet->getSize()), CL_FILE_LOG_AND_CONSOLE));
#endif

		CHECK_SESSION_IS_AUTHORIZED("ExchangeItemByTPLegacyTikiShop");

		if (_session.m_pi.block_flag.m_flag.stBit.legacy_tiki_shop)
			throw exception("[channel::requestExchangeItemByTPLegacyTikiShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] esta bloqueado no Legacy Tiki Shop.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4000, 1));

		uint32_t tiki_pts = 0u;

		std::string s_item = "";

		stLegacyTikiShopExchangeTP tsetp{ 0 };

		std::vector< stItem > v_item;
		stItem item{ 0 };
		BuyItem bi{ 0u };

		SysAchievement sys_achieve;

		uint32_t count = _packet->readUint8();

		for (auto i = 0u; i < count; ++i) {
			tsetp.clear();
			bi.clear();

			_packet->readBuffer(&tsetp, sizeof(stLegacyTikiShopExchangeTP));

			auto base = sIff::getInstance().findCommomItem(tsetp._typeid);

			if (base == nullptr)
				throw exception("[channel::requestExchangeItemByTPLegacyTikiShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar item[TYPEID="
						+ std::to_string(tsetp._typeid) + "] no Tiki's Shop, mas o item nao existe no IFF_STRUCT do Server. Hacker ou Bug",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 901, 0x5200902));

			auto point_shop = sIff::getInstance().findPointShop(tsetp._typeid);

			if (point_shop == nullptr)
				throw exception("[channel::requestExchangeItemByTPLegacyTikiShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou trocar item[TYPEID="
						+ std::to_string(tsetp._typeid) + "] no Tiki's Shop, mas o item nao existe no IFF_STRUCT(PointShop) do Server. Hacker ou Bug",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 901, 0x5200902));

			tiki_pts += point_shop->point * tsetp.qntd;

			bi.id = -1;
			bi._typeid = tsetp._typeid;
			bi.qntd = point_shop->qntd * tsetp.qntd;

			item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1 );

			if (item._typeid == 0)
				throw exception("[channel::requestExchangeItemByTPLegacyTikiShop][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou trocar Item[TYPEID=" + std::to_string(bi._typeid) + ", QNTD=" + std::to_string(bi.qntd)
						+ "], mas nao conseguiu inicializar o item. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 901, 0x5200902));

			v_item.push_back(item);

			s_item += std::string((i == 0) ? "" : ", ") + "[TYPEID=" + std::to_string(tsetp._typeid) + ", QNTD=" + std::to_string(tsetp.qntd)
					+ ", QNTD_REAL=" + std::to_string(point_shop->qntd * tsetp.qntd) + "]";
		}

		if (tiki_pts == 0u)
			throw exception("[channel::requestExchangeItemByTPLegacyTikiShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou trocar item(ns)(" + s_item + "), mas ocorreu um erro na inicializacao do Tiki Points from IFF_STRUCT is invalid("
					+ std::to_string(tiki_pts) + ").", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 905, 0x5200905));

		if (tiki_pts > _session.m_pi.m_legacy_tiki_pts)
			throw exception("[channel::requestExchangeItemByTPLegacyTikiShop][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou trocar item(ns)(" + s_item + "), mas o player nao tem tiki_pts suficiente para a troca[HAVE=" + std::to_string(_session.m_pi.m_legacy_tiki_pts)
					+ ", REQUEST=" + std::to_string(tiki_pts) + "].", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 906, 0x5200906));

		_session.m_pi.m_legacy_tiki_pts -= tiki_pts;

		snmdb::NormalManagerDB::getInstance().add(28, new CmdUpdateLegacyTikiShopPoint(_session.m_pi.uid, _session.m_pi.m_legacy_tiki_pts), channel::SQLDBResponse, this);

		auto rai = item_manager::addItem(v_item, _session, 0, 0);

		if (rai.fails.size() > 0 && rai.type != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH) {

			std::string str = "";

			for (auto i = 0u; i < rai.fails.size(); ++i) {

				if (i == 0)
					str += "[TYPEID=" + std::to_string(rai.fails[i]._typeid) + ", ID=" + std::to_string(rai.fails[i].id) + ", QNTD=" + std::to_string((rai.fails[i].qntd > 0xFFu) ? rai.fails[i].qntd : rai.fails[i].STDA_C_ITEM_QNTD)
						+ (rai.fails[i].STDA_C_ITEM_TIME > 0 ? ", TEMPO=" + std::to_string(rai.fails[i].STDA_C_ITEM_TIME) : std::string("")) + "]";
				else
					str += ", [TYPEID=""" + std::to_string(rai.fails[i]._typeid) + ", ID=" + std::to_string(rai.fails[i].id) + ", QNTD=" + std::to_string((rai.fails[i].qntd > 0xFFu) ? rai.fails[i].qntd : rai.fails[i].STDA_C_ITEM_QNTD)
						+ (rai.fails[i].STDA_C_ITEM_TIME > 0 ? ", TEMPO=" + std::to_string(rai.fails[i].STDA_C_ITEM_TIME) : std::string("")) + "]";
			}

			throw exception("[channel::requestExchangeItemByTPLegacyTikiShop][Error] Itens que falhou ao add os itens que o Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] trocou item(ns){" + str + "}. Hacker ou bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 907, 0x5200907));
		}

		sys_achieve.incrementCounter(0x6C400086u , 1);

		_smp::message_pool::getInstance().push(new message("[channel::requestExchangeItemByTPLegacyTikiShop][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] trocou Tiki Points[TP=" + std::to_string(tiki_pts) + "] por Item(ns)(" + s_item + ").", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addUint32(el.stat.qntd_ant);
			p.addUint32(el.stat.qntd_dep);
			p.addUint32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addZeroByte(25);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x1EA);

		p.addUint32(0u);
		p.addUint32((uint32_t)_session.m_pi.m_legacy_tiki_pts);

		packet_func::session_send(p, &_session, 1);

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestExchangeItemByTPLegacyTikiShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x1EA);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 1u);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestOpenEditSaleShop(player& _session, packet* _packet) {
	REQUEST_BEGIN("OpenEditSaleShop");

	try {

		CHECK_SESSION_IS_AUTHORIZED("OpenEditSaleShop");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r != nullptr) {

			r->requestOpenEditSaleShop(_session, _packet);

		}else {

			_smp::message_pool::getInstance().push(new message("[channel::requestOpenEditSaleShop][Error][WARNIG] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID="
					+ std::to_string((unsigned short)m_ci.id) + "] tentou abrir ou editar um/o personal shop para ele, mas nao esta em nenhum sala[numero="
					+ std::to_string(_session.m_pi.mi.sala_numero) + "]. Hacker ou Bug [Tem que enviar a resposta para o cliente, por que ainda nao esta enviando]", CL_FILE_LOG_AND_CONSOLE));
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestOpenEditSaleShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::ROOM)
			throw;
	}
};

void channel::requestCloseSaleShop(player& _session, packet* _packet) {
	REQUEST_BEGIN("CloseSaleShop");

	try {

		CHECK_SESSION_IS_AUTHORIZED("CloseSaleShop");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r != nullptr)
			r->requestCloseSaleShop(_session, _packet);
		else {

			_smp::message_pool::getInstance().push(new message("[channel::requestCloseSaleShop][Error][WARNIG] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID="
					+ std::to_string((unsigned short)m_ci.id) + "] tentou deletar um personal shop dele, mas nao esta em nenhum sala[numero="
					+ std::to_string(_session.m_pi.mi.sala_numero) + "]. Hacker ou Bug [Tem que enviar a resposta para o cliente, por que ainda nao esta enviando]", CL_FILE_LOG_AND_CONSOLE));
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestCloseSaleShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::ROOM)
			throw;
	}
};

void channel::requestChangeNameSaleShop(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChangeNameSaleShop");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ChangeNameSaleShop");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r != nullptr)
			r->requestChangeNameSaleShop(_session, _packet);
		else {

			_smp::message_pool::getInstance().push(new message("[channel::requestChangeNameSaleShop][Error][WARNIG] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID="
					+ std::to_string((unsigned short)m_ci.id) + "] tentou trocar o nome do personal shop dele. mas nao esta em nenhum sala[numero="
					+ std::to_string(_session.m_pi.mi.sala_numero) + "]. Hacker ou Bug [Tem que enviar a resposta para o cliente, por que ainda nao esta enviando]", CL_FILE_LOG_AND_CONSOLE));
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestChangeNameSaleShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::ROOM)
			throw;
	}
};

void channel::requestOpenSaleShop(player& _session, packet *_packet) {
	REQUEST_BEGIN("OpenSaleShop");

	try {

		CHECK_SESSION_IS_AUTHORIZED("OpenSaleShop");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r != nullptr)
			r->requestOpenSaleShop(_session, _packet);
		else {

			_smp::message_pool::getInstance().push(new message("[channel::requestOpenSaleShop][Error][WARNIG] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID="
					+ std::to_string((unsigned short)m_ci.id) + "] tentou abrir o personal shop dele. mas nao esta em nenhum sala[numero="
					+ std::to_string(_session.m_pi.mi.sala_numero) + "]. Hacker ou Bug [Tem que enviar a resposta para o cliente, por que ainda nao esta enviando]", CL_FILE_LOG_AND_CONSOLE));
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestOpenSaleShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::ROOM)
			throw;
	}
};

void channel::requestVisitCountSaleShop(player& _session, packet *_packet) {
	REQUEST_BEGIN("VisitCountSaleShop");

	try {

		CHECK_SESSION_IS_AUTHORIZED("VisitCountSaleShop");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r != nullptr)
			r->requestVisitCountSaleShop(_session, _packet);
		else {

			_smp::message_pool::getInstance().push(new message("[channel::requestVisitCountSaleShop][Error][WARNIG] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID="
					+ std::to_string((unsigned short)m_ci.id) + "] tentou pedir Visit Count do personal shop dele. mas nao esta em nenhum sala[numero="
					+ std::to_string(_session.m_pi.mi.sala_numero) + "]. Hacker ou Bug [Tem que enviar a resposta para o cliente, por que ainda nao esta enviando]", CL_FILE_LOG_AND_CONSOLE));
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestVisitCountSaleShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::ROOM)
			throw;
	}
};

void channel::requestPangSaleShop(player& _session, packet* _packet) {
	REQUEST_BEGIN("PangSaleShop");

	try {

		CHECK_SESSION_IS_AUTHORIZED("PangSaleShop");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r != nullptr)
			r->requestPangSaleShop(_session, _packet);
		else {

			_smp::message_pool::getInstance().push(new message("[channel::requestPangSaleShop][Error][WARNIG] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID="
					+ std::to_string((unsigned short)m_ci.id) + "] tentou pedir Pang Sale do personal shop dele. mas nao esta em nenhum sala[numero="
					+ std::to_string(_session.m_pi.mi.sala_numero) + "]. Hacker ou Bug [Tem que enviar a resposta para o cliente, por que ainda nao esta enviando]", CL_FILE_LOG_AND_CONSOLE));
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestPangSaleShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::ROOM)
			throw;
	}
};

void channel::requestCancelEditSaleShop(player& _session, packet *_packet) {
	REQUEST_BEGIN("CancelEditSaleShop");

	try {

		CHECK_SESSION_IS_AUTHORIZED("CancelEditSaleShop");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r != nullptr)
			r->requestCancelEditSaleShop(_session, _packet);
		else {

			_smp::message_pool::getInstance().push(new message("[channel::requestCancelEditSaleShop][Error][WARNIG] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID="
					+ std::to_string((unsigned short)m_ci.id) + "] tentou cancelar edit o personal shop dele. mas nao esta em nenhum sala[numero="
					+ std::to_string(_session.m_pi.mi.sala_numero) + "]. Hacker ou Bug [Tem que enviar a resposta para o cliente, por que ainda nao esta enviando]", CL_FILE_LOG_AND_CONSOLE));
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestCancelEditSaleShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::ROOM)
			throw;
	}
};

void channel::requestViewSaleShop(player& _session, packet* _packet) {
	REQUEST_BEGIN("ViewSaleShop");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ViewSaleShop");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r != nullptr)
			r->requestViewSaleShop(_session, _packet);
		else {

			_smp::message_pool::getInstance().push(new message("[channel::requestViewSaleShop][Error][WARNIG] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID="
					+ std::to_string((unsigned short)m_ci.id) + "] tentou ver o personal shop de outro player. mas nao esta em nenhum sala[numero="
					+ std::to_string(_session.m_pi.mi.sala_numero) + "]. Hacker ou Bug [Tem que enviar a resposta para o cliente, por que ainda nao esta enviando]", CL_FILE_LOG_AND_CONSOLE));
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestViewSaleShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::ROOM)
			throw;
	}
};

void channel::requestCloseViewSaleShop(player& _session, packet *_packet) {
	REQUEST_BEGIN("CloseViewSaleShop");

	try {

		CHECK_SESSION_IS_AUTHORIZED("CloseViewSaleShop");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r != nullptr)
			r->requestCloseViewSaleShop(_session, _packet);
		else {

			_smp::message_pool::getInstance().push(new message("[channel::requestCloseViewSaleShop][Error][WARNIG] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID="
					+ std::to_string((unsigned short)m_ci.id) + "] tentou fechar o personal shop de outro player. mas nao esta em nenhum sala[numero="
					+ std::to_string(_session.m_pi.mi.sala_numero) + "]. Hacker ou Bug [Tem que enviar a resposta para o cliente, por que ainda nao esta enviando]", CL_FILE_LOG_AND_CONSOLE));
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestCloseViewSaleShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::ROOM)
			throw;
	}
};

void channel::requestBuyItemSaleShop(player& _session, packet* _packet) {
	REQUEST_BEGIN("BuyItemSaleShop");

	try {

		CHECK_SESSION_IS_AUTHORIZED("BuyItemSaleShop");

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r != nullptr)
			r->requestBuyItemSaleShop(_session, _packet);
		else {

			_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemSaleShop][Error][WARNIG] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID="
					+ std::to_string((unsigned short)m_ci.id) + "] tentou comprar no personal shop de outro player. mas nao esta em nenhum sala[numero="
					+ std::to_string(_session.m_pi.mi.sala_numero) + "]. Hacker ou Bug [Tem que enviar a resposta para o cliente, por que ainda nao esta enviando]", CL_FILE_LOG_AND_CONSOLE));
		}

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestBuyItemSaleShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::ROOM)
			throw;
	}
};

void channel::requestOpenPapelShop(player& _session, packet *_packet) {
	REQUEST_BEGIN("OpenPapelShop");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("OpenPapelShop");

		p.init_plain((unsigned short)0x10B);

		p.addUint32(0);

		p.addInt64(_session.m_pi.mi.papel_shop.limit_count);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestOpenPapelShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x10B);

		p.addInt64(-1);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5800100);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestPlayPapelShop(player& _session, packet *_packet) {
	REQUEST_BEGIN("PlayPapelShop");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("PlayPapelShop");

		if (_session.m_pi.block_flag.m_flag.stBit.papel_shop)
			throw exception("[channel::requestPlayPapelShop][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou jogar no Papel Shop, mas ele nao pode. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0x790001));

		if (_session.m_pi.level < 1)
			throw exception("[channel::requestPlayPapelShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Papel Shop Normal, mas nao tem o level necessario[level="
					+ std::to_string(_session.m_pi.level) + ", request=1]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 8, 0x5900108));

		if (!sPapelShopSystem::getInstance().isLoad())
			sPapelShopSystem::getInstance().load();

		if (sPapelShopSystem::getInstance().isLimittedPerDay() && _session.m_pi.mi.papel_shop.remain_count <= 0)
			throw exception("[channel::requestPlayPapelShop][Warning] player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] tentou jogar o Papel Shop Normal, mas o limite por dia esta ativado, e ele nao tem mais vezes no dia ele ja chegou ao seu limite.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5900101));

		auto coupon = sPapelShopSystem::getInstance().hasCoupon(_session);

		if ((coupon == nullptr || coupon->STDA_C_ITEM_QNTD < 1) && _session.m_pi.ui.pang < sPapelShopSystem::getInstance().getPriceNormal())
			throw exception("[channel::requestPlayPapelShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Papel Shop Normal, ele nao tem Coupon e nem Pangs suficiente[value="
					+ std::to_string(_session.m_pi.ui.pang) + ", request=" + std::to_string(sPapelShopSystem::getInstance().getPriceNormal()) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0x5900102));

		auto balls = sPapelShopSystem::getInstance().dropBalls(_session);

		if (balls.empty())
			throw exception("[channel::requestPlayPapelShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Papel Shop Normal, mas nao conseguiu sortear as bolas. Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0x5900103));

		std::vector< stItem > v_item;
		stItem item{ 0 };
		BuyItem bi{ 0 };

		SysAchievement sys_achieve;

		v_item.reserve(balls.size() + 1 );

		for (auto& el : balls) {

			bi.clear();
			item.clear();

			bi.id = -1;
			bi._typeid = el.ctx_psi._typeid;
			bi.qntd = el.qntd;

			item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1);

			if (item._typeid == 0)
				throw exception("[channel::requestPlayPapelShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Papel Shop Normal, mas nao conseguiu inicializar o Item[TYPEID="
						+ std::to_string(bi._typeid) + "]. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4, 0x5900104));

			auto it = std::find_if(v_item.begin(), v_item.end(), [&](auto& el2) {
				return el2._typeid == item._typeid;
			});

			if (it != v_item.end()) {
				it->qntd += item.qntd;
				it->STDA_C_ITEM_QNTD = (short)it->qntd;
			}else
				it = v_item.insert(v_item.end(), item);

#if defined(_WIN32)
			el.item = it._Ptr;
#elif defined(__linux__)
			el.item = &(*it);
#endif
		}

		std::string ids = "";

		for (auto i = 0u; i < v_item.size(); ++i)
			ids += ((i == 0) ? std::string("") : std::string(", ")) + "TYPEID=" + std::to_string(v_item[i]._typeid) + ", ID=" + std::to_string(v_item[i].id) + ", QNTD=" + std::to_string(v_item[i].STDA_C_ITEM_QNTD);

		auto rai = item_manager::addItem(v_item, _session, 0, 0);

		if (rai.fails.size() > 0 && rai.type != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH)
			throw exception("[channel::requestPlayPapelShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Papel Shop Normal, mas nao conseguiu adicionar o(s) Item(ns){"
					+ ids + "}", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 6, 0x5900106));

		if (coupon != nullptr) {
			item.clear();

			item.type = 2;
			item.id = coupon->id;
			item._typeid = coupon->_typeid;
			item.qntd = 1;
			item.STDA_C_ITEM_QNTD = (short)item.qntd * -1;

			if (item_manager::removeItem(item, _session) <= 0)
				throw exception("[channel::requestPlayPapelShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Papel Shop Normal, mas nao conseguiu deletar o Coupon[TYPEID="
					+ std::to_string(coupon->_typeid) + ", ID=" + std::to_string(coupon->id) + "]. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 5, 0x5900105));

			v_item.push_back(item);

		}else
			_session.m_pi.consomePang(sPapelShopSystem::getInstance().getPriceNormal());

		sPapelShopSystem::getInstance().updatePlayerCount(_session);

		auto rare = std::for_each(balls.begin(), balls.end(), [&](auto& el) {
			if (el.ctx_psi.tipo == PST_RARE) {
				_smp::message_pool::getInstance().push(new message("[PapelShopSystem::PlayNormal][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] ganhou Item Raro[TYPEID="
						+ std::to_string(el.ctx_psi._typeid) + ", QNTD=" + std::to_string(el.qntd) + ", BALL_COLOR=" + std::to_string(el.color) + ", PROBABILIDADE=" + std::to_string(el.ctx_psi.probabilidade) + "]", CL_FILE_LOG_AND_CONSOLE));

				sys_achieve.incrementCounter(0x6C400081u );

				snmdb::NormalManagerDB::getInstance().add(19, new CmdInsertPapelShopRareWinLog(_session.m_pi.uid, el), channel::SQLDBResponse, this);
			}
		});

		sys_achieve.incrementCounter(0x6C40004Au );

		_smp::message_pool::getInstance().push(new message("[PapelShopSystem::PlayNormal][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] jogou Papel Shop Normal e ganhou Item(ns){" + ids + "}", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addBuffer(&el.stat, sizeof(el.stat));
			p.addUint32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addZeroByte(25);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0xFB);

		if (sPapelShopSystem::getInstance().isLimittedPerDay()) {
			p.addInt32(_session.m_pi.mi.papel_shop.remain_count);
			p.addInt32(-2);
		}else {
			p.addInt32(-1);
			p.addInt32(-3);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x21B);

		p.addUint32(0);

		p.addInt32((coupon != nullptr) ? coupon->id : 0);

		p.addUint32((uint32_t)balls.size());

		for (auto& el : balls) {
			p.addUint32(el.color);
			p.addUint32(el.ctx_psi._typeid);
			p.addInt32((el.item != 0) ? ((stItem*)el.item)->id : 0);
			p.addUint32(el.qntd);
			p.addUint32(el.ctx_psi.tipo);
		}

		p.addUint64(_session.m_pi.ui.pang);
		p.addUint64(_session.m_pi.cookie);

		packet_func::session_send(p, &_session, 1);

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestPlayPapelShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x21B);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5900100);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestPlayBigPapelShop(player& _session, packet *_packet) {
	REQUEST_BEGIN("PlayBigPapelShop");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("PlayBigPapelShop");

		if (_session.m_pi.block_flag.m_flag.stBit.papel_shop)
			throw exception("[channel::requestPlayBigPapelShop][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou jogar no Papel Shop, mas ele nao pode. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0x790001));

		if (_session.m_pi.level < 1)
			throw exception("[channel::requestPlayBigPapelShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Papel Shop Big, mas nao tem o level necessario[level="
					+ std::to_string(_session.m_pi.level) + ", request=1]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 8, 0x5900108));

		if (!sPapelShopSystem::getInstance().isLoad())
			sPapelShopSystem::getInstance().load();

		if (sPapelShopSystem::getInstance().isLimittedPerDay() && _session.m_pi.mi.papel_shop.remain_count <= 0)
			throw exception("[channel::requestPlayBigPapelShop][Warning] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou jogar o Papel Shop Big, mas o limite por dia esta ativado, e ele nao tem mais vezes no dia ele ja chegou ao seu limite.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0x5900101));

		if (_session.m_pi.ui.pang < sPapelShopSystem::getInstance().getPriceBig())
			throw exception("[channel::requestPlayBigPapelShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Papel Shop Big, ele nao tem Pangs suficiente[value="
					+ std::to_string(_session.m_pi.ui.pang) + ", request=" + std::to_string(sPapelShopSystem::getInstance().getPriceBig()) + "]. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0x5900102));

		auto balls = sPapelShopSystem::getInstance().dropBigBall(_session);

		if (balls.empty())
			throw exception("[channel::requestPlayBigPapelShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Papel Shop Big, mas nao conseguiu sortear as bolas. Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0x5900103));

		std::vector< stItem > v_item;
		stItem item{ 0 };
		BuyItem bi{ 0 };

		SysAchievement sys_achieve;

		v_item.reserve(balls.size());

		for (auto& el : balls) {

			bi.clear();
			item.clear();

			bi.id = -1;
			bi._typeid = el.ctx_psi._typeid;
			bi.qntd = el.qntd;

			item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1);

			if (item._typeid == 0)
				throw exception("[channel::requestPlayBigPapelShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Papel Shop Big, mas nao conseguiu inicializar o Item[TYPEID="
						+ std::to_string(bi._typeid) + "]. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4, 0x5900104));

			auto it = std::find_if(v_item.begin(), v_item.end(), [&](auto& el2) {
				return el2._typeid == item._typeid;
			});

			if (it != v_item.end()) {
				it->qntd += item.qntd;
				it->STDA_C_ITEM_QNTD = (short)it->qntd;
			}else
				it = v_item.insert(v_item.end(), item);

			el.item = &(*it);
		}

		std::string ids = "";

		for (auto i = 0u; i < v_item.size(); ++i)
			ids += ((i == 0) ? std::string("") : std::string(", ")) + "TYPEID=" + std::to_string(v_item[i]._typeid) + ", ID=" + std::to_string(v_item[i].id) + ", QNTD=" + std::to_string(v_item[i].STDA_C_ITEM_QNTD);

		auto rai = item_manager::addItem(v_item, _session, 0, 0);

		if (rai.fails.size() > 0 && rai.type != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH)
			throw exception("[channel::requestPlayBigPapelShop][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou jogar o Papel Shop Big, mas nao conseguiu adicionar o(s) Item(ns){"
					+ ids + "}", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 6, 0x5900106));

		_session.m_pi.consomePang(sPapelShopSystem::getInstance().getPriceBig());

		sPapelShopSystem::getInstance().updatePlayerCount(_session);

		auto rare = std::for_each(balls.begin(), balls.end(), [&](auto& el) {
			if (el.ctx_psi.tipo == PST_RARE) {
				_smp::message_pool::getInstance().push(new message("[PapelShopSystem::PlayBig][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] ganhou Item Raro[TYPEID="
						+ std::to_string(el.ctx_psi._typeid) + ", QNTD=" + std::to_string(el.qntd) + ", BALL_COLOR=" + std::to_string(el.color) + ", PROBABILIDADE=" + std::to_string(el.ctx_psi.probabilidade) + "]", CL_FILE_LOG_AND_CONSOLE));

				sys_achieve.incrementCounter(0x6C400081u );

				snmdb::NormalManagerDB::getInstance().add(19, new CmdInsertPapelShopRareWinLog(_session.m_pi.uid, el), channel::SQLDBResponse, this);
			}
		});

		sys_achieve.incrementCounter(0x6C40004Au );

		_smp::message_pool::getInstance().push(new message("[PapelShopSystem::PlayBig][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] jogou Papel Shop Big e ganhou Item(ns){" + ids + "}", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0xC8);

		p.addUint64(_session.m_pi.ui.pang);
		p.addUint64(0);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x216);

		p.addUint32((const uint32_t)GetSystemTimeAsUnix());
		p.addUint32((uint32_t)v_item.size());

		for (auto& el : v_item) {
			p.addUint8(el.type);
			p.addUint32(el._typeid);
			p.addInt32(el.id);
			p.addUint32(el.flag_time);
			p.addBuffer(&el.stat, sizeof(el.stat));
			p.addUint32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
			p.addZeroByte(25);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0xFB);

		if (sPapelShopSystem::getInstance().isLimittedPerDay()) {
			p.addInt32(_session.m_pi.mi.papel_shop.remain_count);
			p.addInt32(_session.m_pi.mi.papel_shop.current_count);
		}else {
			p.addInt32(-1);
			p.addInt32(-3);
		}

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x26C);

		p.addUint32(0);

		p.addInt32(0);

		p.addUint32((uint32_t)balls.size());

		for (auto& el : balls) {
			p.addUint32(el.color);
			p.addUint32(el.ctx_psi._typeid);
			p.addInt32((el.item != 0) ? ((stItem*)el.item)->id : 0);
			p.addUint32(el.qntd);
			p.addUint32(el.ctx_psi.tipo);
		}

		p.addUint64(_session.m_pi.ui.pang);
		p.addUint64(_session.m_pi.cookie);

		packet_func::session_send(p, &_session, 1);

		sys_achieve.finish_and_update(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestPlayBigPapelShop][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x26C);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5900100);

		packet_func::session_send(p, &_session, 1);
	}
};

void channel::requestSendMsgChatRoom(player& _session, std::string _msg) {

	if (!_session.getState())
		throw exception("[channel::requestSendMsgChatRoom][Error] player nao esta connectado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1, 0));

	packet p;

	try {

		BEGIN_FIND_ROOM(_session.m_pi.mi.sala_numero);

		if (r == nullptr)
			throw exception("[channel::requestSendMsgChatRoom][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] nao esta em uma sala[NUMERO=" + std::to_string(_session.m_pi.mi.sala_numero) + "]. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 18, 0));

		packet_func::pacote040(p, &_session, &_session.m_pi, _msg, (_session.m_pi.m_cap.stBit.game_master ) ? 0x80 : 0);
		packet_func::room_broadcast(*r, p, 0);

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestSendMsgChatRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		throw;
	}
};

void channel::sendUpdateRoomInfo(RoomInfoEx& _ri, int _option) {

	if (_ri.tipo != RoomInfo::TIPO::PRACTICE && _ri.tipo != RoomInfo::TIPO::GRAND_ZODIAC_PRACTICE) {

		packet p;

		if (packet_func::pacote047(p, std::vector< RoomInfo > { _ri }, _option))
			packet_func::channel_broadcast(*this, p, 0);
	}
};

void channel::sendUpdatePlayerInfo(player& _session, int _option) {

	packet p;
	PlayerCanalInfo *pci = getPlayerInfo(&_session);

	if (packet_func::pacote046(p, &_session, std::vector< PlayerCanalInfo > { (pci == nullptr) ? PlayerCanalInfo() : *pci }, _option))
		packet_func::channel_broadcast(*this, p, 0);
};

void channel::destroyRoom(short _number) {

	try {

		BEGIN_FIND_ROOM(_number);

		if (r == nullptr)
			throw exception("[channel::destroyRoom][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou destruir a sala[NUMERO=" + std::to_string(_number)
					+ "], mas a sala nao existe.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 16, 0x5700100));

		auto v_sessions = r->getSessions();

		if (v_sessions.empty()) {

			RoomInfoEx ri = *r->getInfo();

			m_rm.destroyRoom(r);

			sendUpdateRoomInfo(ri, 2);

		}else {

			for (auto& el : v_sessions)
				kickPlayerRoom(*el, 0 );
		}

		_smp::message_pool::getInstance().push(new message("[channel::destroyRoom][Log] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
				+ "] destruiu a sala[NUMERO=" + std::to_string(_number) + "] no canal[NOME=" + std::string(m_ci.name) + "].", CL_FILE_LOG_AND_CONSOLE));

		END_FIND_ROOM;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::destroyRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

int channel::_enter_left_time_is_over(void* _arg1, void* _arg2) {

	channel *c = (channel*)_arg1;
	short numero = (short)(size_t)_arg2;

	try {

		if (c == nullptr)
			throw exception("[channel::_enter_left_time_is_over][Error] Channel[ID=-1] Sala[NUMERO=" + std::to_string(numero)
					+ "] channel ponteiro fornecido pelo argumento is invalid.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1201, 0));

		if (numero < 0)
			throw exception("[channel::_enter_left_time_is_over][Error] Channel[ID=" + std::to_string((unsigned short)c->getId())
					+ "] Sala[NUMERO=" + std::to_string(numero) + "] numero da sala fornecido pelo argumento is invalid", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1200, 0));

		BEGIN_FIND_ROOM_C(numero);

		if (r == nullptr)
			throw exception("[channel::_enter_left_time_is_over][Error] Channel[ID=" + std::to_string((unsigned short)c->getId())
					+ "] Sala[NUMERO=" + std::to_string(numero) + "] nao encontrou a sala no canal", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 1202, 0));

		r->setState(0);
		r->setFlag(0);

		r->requestEndAfterEnter();

		packet p;

		if (packet_func::pacote047(p, std::vector< RoomInfo > { *(RoomInfo*)r->getInfo() }, 3))
			packet_func::channel_broadcast(*c, p, 1);

		_smp::message_pool::getInstance().push(new message("[channel::_enter_left_time_is_over][Log] Channel[ID=" + std::to_string((unsigned short)c->getId())
				+ "] Tempo para entrar na sala[NUMERO=" + std::to_string(numero) + "] depois de ter comecado Acabou.", CL_FILE_LOG_AND_CONSOLE));

		END_FIND_ROOM_C;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::_enter_left_time_is_over][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	return 0;
};

void channel::addInviteTimeRequest(InviteChannelInfo& _ici) {

	if (_ici.room_number < 0)
		throw exception("[channel::addInviteTimeRequest][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
				+ "] tentou adicionar Invite Time Request[INVITE=" + std::to_string(_ici.invite_uid) + ", INVITED=" + std::to_string(_ici.invited_uid)
				+ "] para sala[NUMERO=" + std::to_string(_ici.room_number) + "], mas o numero da sala eh invalido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3010, 0));

	if (_ici.invite_uid == 0u)
		throw exception("[channel::addInviteTimeRequest][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
				+ "] tentou adicionar Invite Time Request[INVITE=" + std::to_string(_ici.invite_uid) + ", INVITED=" + std::to_string(_ici.invited_uid)
				+ "] para sala[NUMERO=" + std::to_string(_ici.room_number) + "], mas quem convidou o uid is invalid(zero)", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3010, 1));

	if (_ici.invited_uid == 0u)
		throw exception("[channel::addInviteTimeRequest][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
				+ "] tentou adicionar Invite Time Request[INVITE=" + std::to_string(_ici.invite_uid) + ", INVITED=" + std::to_string(_ici.invited_uid)
				+ "] para sala[NUMERO=" + std::to_string(_ici.room_number) + "], mas o convidado uid is invalid(zero)", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3010, 2));

#if defined(_WIN32)
	EnterCriticalSection(&m_cs_invite);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs_invite);
#endif

	v_invite.push_back(_ici);

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs_invite);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs_invite);
#endif
}

void channel::deleteInviteTimeRequest(InviteChannelInfo& _ici) {

	if (_ici.room_number < 0)
		throw exception("[channel::deleteInviteTimeRequest][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
				+ "] tentou deletar Invite Time Request[INVITE=" + std::to_string(_ici.invite_uid) + ", INVITED=" + std::to_string(_ici.invited_uid)
				+ "] para sala[NUMERO=" + std::to_string(_ici.room_number) + "], mas o numero da sala eh invalido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3011, 0));

	if (_ici.invite_uid == 0u)
		throw exception("[channel::deleteInviteTimeRequest][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
				+ "] tentou deletar Invite Time Request[INVITE=" + std::to_string(_ici.invite_uid) + ", INVITED=" + std::to_string(_ici.invited_uid)
				+ "] para sala[NUMERO=" + std::to_string(_ici.room_number) + "], mas quem convidou o uid is invalid(zero)", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3011, 1));

	if (_ici.invited_uid == 0u)
		throw exception("[channel::deleteInviteTimeRequest][Error] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
				+ "] tentou deletar Invite Time Request[INVITE=" + std::to_string(_ici.invite_uid) + ", INVITED=" + std::to_string(_ici.invited_uid)
				+ "] para sala[NUMERO=" + std::to_string(_ici.room_number) + "], mas o convidado uid is invalid(zero)", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3011, 2));

#if defined(_WIN32)
	EnterCriticalSection(&m_cs_invite);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs_invite);
#endif

	auto it = std::find_if(v_invite.begin(), v_invite.end(), [&](auto& _el) {
		return (_el.room_number == _ici.room_number && _el.invite_uid == _ici.invite_uid && _el.invited_uid == _ici.invited_uid);
	});

	if (it != v_invite.end())
		v_invite.erase(it);
	else
		_smp::message_pool::getInstance().push(new message("[channel::deleteInviteTimeRequest][Log] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
				+ "] tentou deletar Invite Time Request[INVITE=" + std::to_string(_ici.invite_uid) + ", INVITED=" + std::to_string(_ici.invited_uid)
				+ "] para sala[NUMERO=" + std::to_string(_ici.room_number) + "], mas ele nao existe mais no vector do canal.", CL_FILE_LOG_AND_CONSOLE));

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs_invite);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs_invite);
#endif
}

void channel::deleteInviteTimeResquestByInvited(player& _session) {

	try {

#if defined(_WIN32)
		EnterCriticalSection(&m_cs_invite);
#elif defined(__linux__)
		pthread_mutex_lock(&m_cs_invite);
#endif

		for (auto i = 0u; i < v_invite.size(); ++i) {

			if (v_invite[i].invited_uid == _session.m_pi.uid) {

				BEGIN_FIND_ROOM(v_invite[i].room_number);

				if (r != nullptr && r->isInvited(_session)) {

					auto ici = r->deleteInvited(_session);

					v_invite.erase(v_invite.begin() + i--);

					sendUpdateRoomInfo(*const_cast< RoomInfoEx* >(r->getInfo()), 3);
				}

				END_FIND_ROOM;
			}
		}

#if defined(_WIN32)
		LeaveCriticalSection(&m_cs_invite);
#elif defined(__linux__)
		pthread_mutex_unlock(&m_cs_invite);
#endif

	}catch (exception& e) {

#if defined(_WIN32)
		LeaveCriticalSection(&m_cs_invite);
#elif defined(__linux__)
		pthread_mutex_unlock(&m_cs_invite);
#endif

		_smp::message_pool::getInstance().push(new message("[channel::deleteInviteTimeRequestByInvited][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

bool channel::send_time_out_invite(InviteChannelInfo& _ici) {

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs_invite);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs_invite);
#endif

	BEGIN_FIND_ROOM(_ici.room_number);

#if defined(_WIN32)
	EnterCriticalSection(&m_cs_invite);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs_invite);
#endif

	auto it = std::find_if(v_invite.begin(), v_invite.end(), [&](auto& _el) {
		return (InviteChannelInfo*)&_el == (InviteChannelInfo*)&_ici;
	});

	if (it == v_invite.end())
		return false;

	try {

		if (r == nullptr) {
			_smp::message_pool::getInstance().push(new message("[channel::send_time_out_invite][Log] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou deletar o convite[CONVIDOU=" + std::to_string(_ici.invite_uid) + ", CONVIDADO=" + std::to_string(_ici.invited_uid)
					+ "] da Sala[NUMERO=" + std::to_string(_ici.room_number) + "], mas a sala nao existe mais no canal.", CL_FILE_LOG_AND_CONSOLE));

			return true;
		}

		auto s = findSessionByUID(_ici.invited_uid);

		if (s == nullptr) {

			_smp::message_pool::getInstance().push(new message("[channel::send_time_out_invite][Log] Channel[ID=" + std::to_string((unsigned short)m_ci.id)
					+ "] tentou deletar o convite[CONVIDOU=" + std::to_string(_ici.invite_uid) + ", CONVIDADO=" + std::to_string(_ici.invited_uid)
					+ "] da Sala[NUMERO=" + std::to_string(_ici.room_number) + "], mas o convidado nao esta mais no canal, tenta excluir o convite com uid.", CL_FILE_LOG_AND_CONSOLE));

			r->deleteInvited(_ici.invited_uid);

		}else
			r->deleteInvited(*s);

		sendUpdateRoomInfo(*const_cast< RoomInfoEx* >(r->getInfo()), 3);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::send_time_out_invite][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	END_FIND_ROOM;

	return true;
}

void channel::clear_invite_time() {

	if (!v_invite.empty()) {

		for (auto& el : v_invite)
			send_time_out_invite(el);

		v_invite.clear();
		v_invite.shrink_to_fit();
	}
}

void channel::removeSession(player *_session) {

	if (_session == nullptr)
		throw exception("[channel::removeSession][Error] _session is nullptr.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0));

	size_t index = INVALID_INDEX;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	if ((index = findIndexSession(_session)) == INVALID_INDEX) {
#if defined(_WIN32)
		LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
		pthread_mutex_unlock(&m_cs);
#endif
		throw exception("[channel::removeSession][Error] _session not exists on vector sessions.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 4, 0));
	}

	v_sessions.erase(v_sessions.begin() + index);

	v_sessions.shrink_to_fit();

	m_ci.curr_user--;

	_session->m_pi.channel = INVALID_CHANNEL;
	_session->m_pi.place = 0u;

	deletePlayerInfo(*_session);

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif
};

void channel::addSession(player *_session) {

	if (_session == nullptr || !_session->getState())
		throw exception("[channel::addSession][Error] _session is nullptr or invalid.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 1));

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	v_sessions.push_back(_session);

	m_ci.curr_user++;

	_session->m_pi.channel = m_ci.id;
	_session->m_pi.place = 0u;

	if (_session->m_pi.level >= 6 && _session->m_pi.ui.jogado >= 50) {
		float rate = _session->m_pi.ui.getQuitRate();

		if (rate < GOOD_PLAYER_ICON)
			_session->m_pi.mi.state_flag.stFlagBit.azinha = 1;
		else if (rate >= QUITER_ICON_1 && rate < QUITER_ICON_2)
			_session->m_pi.mi.state_flag.stFlagBit.quiter_1 = 1;
		else if (rate >= QUITER_ICON_2)
			_session->m_pi.mi.state_flag.stFlagBit.quiter_2 = 1;
	}

	if (_session->m_pi.ei.char_info != nullptr && _session->m_pi.ui.getQuitRate() < GOOD_PLAYER_ICON)
		_session->m_pi.mi.state_flag.stFlagBit.icon_angel = _session->m_pi.ei.char_info->AngelEquiped();
	else
		_session->m_pi.mi.state_flag.stFlagBit.icon_angel = 0u;

	_session->m_pi.mi.state_flag.stFlagBit.sexo = _session->m_pi.mi.sexo;

	makePlayerInfo(*_session);

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif
};

player* channel::findSessionByOID(uint32_t _oid) {

	auto it = std::find_if(v_sessions.begin(), v_sessions.end(), [&](auto& el) {
		return el->m_oid == _oid;
	});

	return (it != v_sessions.end() ? *it : nullptr);
}

player* channel::findSessionByUID(uint32_t _uid) {

	auto it = std::find_if(v_sessions.begin(), v_sessions.end(), [&](auto& el) {
		return el->m_pi.uid == _uid;
	});

	return (it != v_sessions.end() ? *it : nullptr);
}

player* channel::findSessionByNickname(std::string _nickname) {

	auto it = std::find_if(v_sessions.begin(), v_sessions.end(), [&](auto& el) {
		return (_nickname.compare(el->m_pi.nickname) == 0);
	});

	return (it != v_sessions.end() ? *it : nullptr);
}

size_t channel::findIndexSession(player *_session) {

	if (_session == nullptr)
		throw exception("[channel::findIndexSession][Error] _session is nullptr.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 3, 0));

	for (auto i = 0u; i < v_sessions.size(); ++i)
		if (v_sessions[i] == _session)
			return i;

	return INVALID_INDEX;
};

void channel::makePlayerInfo(player& _session) {
	PlayerCanalInfo pci{};

	pci.uid = _session.m_pi.uid;
	pci.oid = _session.m_oid;
	pci.sala_numero = _session.m_pi.mi.sala_numero;
#if defined(_WIN32)
	memcpy_s(pci.nickname, sizeof(pci.nickname), _session.m_pi.nickname, sizeof(pci.nickname));
#elif defined(__linux__)
	memcpy(pci.nickname, _session.m_pi.nickname, sizeof(pci.nickname));
#endif
	pci.level = (unsigned char)_session.m_pi.level;
	pci.capability = _session.m_pi.m_cap;
	pci.title = _session.m_pi.ue.m_title;
	pci.team_point = 1000;

	if (_session.m_pi.level >= 6 && _session.m_pi.ui.jogado >= 50) {
		float rate = _session.m_pi.ui.getQuitRate();

		if (rate < GOOD_PLAYER_ICON)
			pci.state_flag.sBit.azinha = 1;
		else if (rate >= QUITER_ICON_1 && rate < QUITER_ICON_2)
			pci.state_flag.sBit.quiter_1 = 1;
		else if (rate >= QUITER_ICON_2)
			pci.state_flag.sBit.quiter_2 = 1;
	}

	if (_session.m_pi.ei.char_info != nullptr && _session.m_pi.ui.getQuitRate() < GOOD_PLAYER_ICON)
		pci.state_flag.sBit.icon_angel = _session.m_pi.ei.char_info->AngelEquiped();
	else
		pci.state_flag.sBit.icon_angel = 0u;

	pci.state_flag.sBit.sexo = _session.m_pi.mi.sexo;

	pci.guid_uid = _session.m_pi.gi.uid;
	pci.guild_index_mark = _session.m_pi.gi.index_mark_emblem;
#if defined(_WIN32)
	memcpy_s(pci.guild_mark_img, sizeof(pci.guild_mark_img), _session.m_pi.gi.mark_emblem, sizeof(pci.guild_mark_img));
#elif defined(__linux__)
	memcpy(pci.guild_mark_img, _session.m_pi.gi.mark_emblem, sizeof(pci.guild_mark_img));
#endif
	pci.flag_visible_gm = _session.m_pi.mi.state_flag.stFlagBit.visible;
	pci.l_unknown = 0;

	auto it = m_player_info.insert(std::make_pair(&_session, pci));

	if (!it.second) {

		if (it.first != m_player_info.end() && it.first->first != nullptr && it.first->first == (&_session)) {

			if (it.first->second.uid != _session.m_pi.uid) {

				try {

					auto pci_ant = m_player_info.at(&_session);

					m_player_info.at(&_session) = pci;

					_smp::message_pool::getInstance().push(new message("[channel::makePlayerInfo][WARNING][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] esta trocando o PlayerChannelInfo[UID=" + std::to_string(pci_ant.uid) + "] do player anterior que estava conectado com essa session, pelo o PlayerChannelInfo[UID="
							+ std::to_string(pci.uid) + "] do player atual da session.", CL_FILE_LOG_AND_CONSOLE));

				}catch (std::out_of_range& e) {
					UNREFERENCED_PARAMETER(e);

					_smp::message_pool::getInstance().push(new message("[channel::makePlayerInfo][Error][WARNING] Player[UID=" + std::to_string(_session.m_pi.uid)
							+ "], nao conseguiu atualizar o PlayerChannelInfo da session para o novo PlayerChannelInfo do player atual da session. Bug", CL_FILE_LOG_AND_CONSOLE));
				}

			}else
				_smp::message_pool::getInstance().push(new message("[channel::makePlayerInfo][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] nao conseguiu adicionar o PlayerChannelInfo da session, por que ja tem o mesmo PlayerChannelInfo no map.", CL_FILE_LOG_AND_CONSOLE));

		}else
			_smp::message_pool::getInstance().push(new message("[channel::makePlayerInfo][Error] nao conseguiu inserir o pair de PlayerInfo do player[UID="
					+ std::to_string(_session.m_pi.uid) + "] no map de player info do channel. Bug", CL_FILE_LOG_AND_CONSOLE));
	}

	_session.m_pi.updateLocationDB();
};

void channel::updatePlayerInfo(player& _session) {
	PlayerCanalInfo pci{}, *_pci = nullptr;

	if ((_pci = getPlayerInfo(&_session)) == nullptr)
		throw exception("[channel::updatePlayerInfo][Error] nao tem o player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] info dessa session no canal.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 16, 0));

	pci = *_pci;

	pci.uid = _session.m_pi.uid;
	pci.oid = _session.m_oid;
	pci.sala_numero = _session.m_pi.mi.sala_numero;
#if defined(_WIN32)
	memcpy_s(pci.nickname, sizeof(pci.nickname), _session.m_pi.nickname, sizeof(pci.nickname));
#elif defined(__linux__)
	memcpy(pci.nickname, _session.m_pi.nickname, sizeof(pci.nickname));
#endif
	pci.level = (unsigned char)_session.m_pi.level;
	pci.capability = _session.m_pi.m_cap;
	pci.title = _session.m_pi.ue.m_title;
	pci.team_point = 1000;

	if (_session.m_pi.level >= 6 && _session.m_pi.ui.jogado >= 50) {
		float rate = _session.m_pi.ui.getQuitRate();

		if (rate < GOOD_PLAYER_ICON)
			pci.state_flag.sBit.azinha = 1;
		else if (rate >= QUITER_ICON_1 && rate < QUITER_ICON_2)
			pci.state_flag.sBit.quiter_1 = 1;
		else if (rate >= QUITER_ICON_2)
			pci.state_flag.sBit.quiter_2 = 1;
	}

	if (_session.m_pi.ei.char_info != nullptr && _session.m_pi.ui.getQuitRate() < GOOD_PLAYER_ICON)
		pci.state_flag.sBit.icon_angel = _session.m_pi.ei.char_info->AngelEquiped();
	else
		pci.state_flag.sBit.icon_angel = 0u;

	pci.state_flag.sBit.sexo = _session.m_pi.mi.sexo;

	pci.guid_uid = _session.m_pi.gi.uid;
	pci.guild_index_mark = _session.m_pi.gi.index_mark_emblem;
#if defined(_WIN32)
	memcpy_s(pci.guild_mark_img, sizeof(pci.guild_mark_img), _session.m_pi.gi.mark_emblem, sizeof(pci.guild_mark_img));
#elif defined(__linux__)
	memcpy(pci.guild_mark_img, _session.m_pi.gi.mark_emblem, sizeof(pci.guild_mark_img));
#endif
	pci.flag_visible_gm = _session.m_pi.mi.state_flag.stFlagBit.visible;
	pci.l_unknown = 0;

	*_pci = pci;

	_session.m_pi.updateLocationDB();
};

void channel::deletePlayerInfo(player& _session) {

	_session.m_pi.updateLocationDB();

	m_player_info.erase(&_session);
};

void channel::SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg) {

	if (_arg == nullptr) {
		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][WARNING] _arg is nullptr com msg_id = " + std::to_string(_msg_id), CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	if (_pangya_db.getException().getCodeError() != 0) {
		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Error] " + _pangya_db.getException().getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	auto *_channel = reinterpret_cast< channel* >(_arg);

	switch (_msg_id) {
	case 1:
	{
		auto cmd_udlp = reinterpret_cast< CmdUpdateDolfiniLockerPass* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] Atualizou a senha[value=" + cmd_udlp->getPass() + "] do Dolfini Locker do player[UID=" + std::to_string(cmd_udlp->getUID()) + "]", CL_FILE_LOG_AND_CONSOLE));

		break;
	}
	case 2:
	{
		auto cmd_udlm = reinterpret_cast< CmdUpdateDolfiniLockerMode* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] Atualizou o Modo[locker=" + std::to_string((unsigned short)cmd_udlm->getLocker()) + "] do Dolfini Locker do player[UID=" + std::to_string(cmd_udlm->getUID()) + "]", CL_FILE_LOG_AND_CONSOLE));

		break;
	}
	case 3:
	{
		auto cmd_udlp = reinterpret_cast< CmdUpdateDolfiniLockerPang* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] Atualizou o Pang[value=" + std::to_string(cmd_udlp->getPang()) + "] do Dolfini Locker do player[UID=" + std::to_string(cmd_udlp->getUID()) + "]", CL_FILE_LOG_AND_CONSOLE));

		break;
	}
	case 4:
	{
		auto cmd_ddli = reinterpret_cast< CmdDeleteDolfiniLockerItem* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] Deletou o Dolfini Locker Item[index=" + std::to_string(cmd_ddli->getIndex()) + "] do player[UID=" + std::to_string(cmd_ddli->getUID()) + "]", CL_FILE_LOG_AND_CONSOLE));

		break;
	}
	case 5:
	{
		auto cmd_er = reinterpret_cast< CmdExtendRental* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] Extendeu Part Rental[ID=" + std::to_string(cmd_er->getItemID()) + "] ate o a date[value="
					+ cmd_er->getDate() + "] para o player[UID=" + std::to_string(cmd_er->getUID()) + "]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 6:
	{
		auto cmd_dr = reinterpret_cast< CmdDeleteRental* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] Deletou Part Rental[ID=" + std::to_string(cmd_dr->getItemID()) + "] do player[UID=" + std::to_string(cmd_dr->getUID()) + "]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 7:
	{
		auto cmd_ucp = reinterpret_cast< CmdUpdateCharacterPCL* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] Atualizou Character[TYPEID=" + std::to_string(cmd_ucp->getInfo()._typeid) + ", ID="
				+ std::to_string(cmd_ucp->getInfo().id) + "] PCL[C0=" + std::to_string((unsigned short)cmd_ucp->getInfo().pcl[CharacterInfo::S_POWER]) + ", C1="
				+ std::to_string((unsigned short)cmd_ucp->getInfo().pcl[CharacterInfo::S_CONTROL]) + ", C2=" + std::to_string((unsigned short)cmd_ucp->getInfo().pcl[CharacterInfo::S_ACCURACY]) + ", C3="
				+ std::to_string((unsigned short)cmd_ucp->getInfo().pcl[CharacterInfo::S_SPIN]) + ", C4=" + std::to_string((unsigned short)cmd_ucp->getInfo().pcl[CharacterInfo::S_CURVE]) + "] do Player[UID="
				+ std::to_string(cmd_ucp->getUID()) + "]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 8:
	{
		auto cmd_ucss = reinterpret_cast<CmdUpdateClubSetStats*>(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] Atualizou ClubSet[TYPEID=" + std::to_string(cmd_ucss->getInfo()._typeid) + ", ID="
				+ std::to_string(cmd_ucss->getInfo().id) + "] Stats[C0=" + std::to_string((unsigned short)cmd_ucss->getInfo().c[CharacterInfo::S_POWER]) + ", C1="
				+ std::to_string((unsigned short)cmd_ucss->getInfo().c[CharacterInfo::S_CONTROL]) + ", C2=" + std::to_string((unsigned short)cmd_ucss->getInfo().c[CharacterInfo::S_ACCURACY]) + ", C3="
				+ std::to_string((unsigned short)cmd_ucss->getInfo().c[CharacterInfo::S_SPIN]) + ", C4=" + std::to_string((unsigned short)cmd_ucss->getInfo().c[CharacterInfo::S_CURVE]) + "] do Player[UID="
				+ std::to_string(cmd_ucss->getUID()) + "]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 9:
	{
		auto cmd_ucm = reinterpret_cast< CmdUpdateCharacterMastery* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] Atualizou Character[TYPEID=" + std::to_string(cmd_ucm->getInfo()._typeid) + ", ID="
				+ std::to_string(cmd_ucm->getInfo().id) + "] Mastery[value=" + std::to_string(cmd_ucm->getInfo().mastery) + "] do player[UID=" + std::to_string(cmd_ucm->getUID()) + "]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 10:
	{
		auto cmd_ec = reinterpret_cast< CmdEquipCard* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] Equipou Card[TYPEID=" + std::to_string(cmd_ec->getInfo()._typeid) + "] no Character[TYPEID="
				+ std::to_string(cmd_ec->getInfo().parts_typeid) + ", ID=" + std::to_string(cmd_ec->getInfo().parts_id) + "] do Player[UID=" + std::to_string(cmd_ec->getUID()) + "]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 11:
	{
		auto cmd_rec = reinterpret_cast< CmdRemoveEquipedCard* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] Desequipou Card[TYPEID=" + std::to_string(cmd_rec->getInfo()._typeid) +"] do Character[TYPEID="
				+ std::to_string(cmd_rec->getInfo().parts_typeid) + ", ID=" + std::to_string(cmd_rec->getInfo().parts_id) + "] do player[UID=" + std::to_string(cmd_rec->getUID()) + "]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 12:
	{
		auto cmd_ucw = reinterpret_cast< CmdUpdateClubSetWorkshop* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_ucw->getUID()) + "] Atualizou ClubSet[TYPEID=" + std::to_string(cmd_ucw->getInfo()._typeid) + ", ID="
				+ std::to_string(cmd_ucw->getInfo().id) + "] Workshop[C0=" + std::to_string(cmd_ucw->getInfo().clubset_workshop.c[0]) + ", C1=" + std::to_string(cmd_ucw->getInfo().clubset_workshop.c[1]) + ", C2="
				+ std::to_string(cmd_ucw->getInfo().clubset_workshop.c[2]) + ", C3=" + std::to_string(cmd_ucw->getInfo().clubset_workshop.c[3]) + ", C4=" + std::to_string(cmd_ucw->getInfo().clubset_workshop.c[4])
				+ ", Level=" + std::to_string(cmd_ucw->getInfo().clubset_workshop.level) + ", Mastery=" + std::to_string(cmd_ucw->getInfo().clubset_workshop.mastery) + ", Rank="
				+ std::to_string(cmd_ucw->getInfo().clubset_workshop.rank) + ", Recovery=" + std::to_string(cmd_ucw->getInfo().clubset_workshop.recovery_pts) + "] Flag=" + std::to_string(cmd_ucw->getFlag()) + "", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 13:
	{
		auto cmd_ut = reinterpret_cast< CmdUpdateTutorial* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_ut->getUID()) + "] Atualizou Tutorial[Rookie=" + std::to_string(cmd_ut->getInfo().rookie) + ", Beginner="
				+ std::to_string(cmd_ut->getInfo().beginner) + ", Advancer=" + std::to_string(cmd_ut->getInfo().advancer) + "]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 14:
	{
		auto cmd_tec = reinterpret_cast< CmdTutoEventClear* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_tec->getUID()) + "] Concluiu Tutorial Event[Type=" + std::to_string(cmd_tec->getType()) + "]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 15:
	{
		auto cmd_uib = reinterpret_cast< CmdUseItemBuff* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_uib->getUID()) + "] Usou o Item Buff[TYPEID="
				+ std::to_string(cmd_uib->getInfo()._typeid) + ", TEMPO=" + std::to_string(cmd_uib->getTime()) + "]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 16:
	{
		auto cmd_uib = reinterpret_cast< CmdUpdateItemBuff* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_uib->getUID()) + "] Atualizou o tempo do Item Buff[INDEX=" + std::to_string(cmd_uib->getInfo().index) + ", TYPEID="
				+ std::to_string(cmd_uib->getInfo()._typeid) + ", TIPO=" + std::to_string(cmd_uib->getInfo().tipo) + ", DATE{REG_DT: "
				+ _formatDate(cmd_uib->getInfo().use_date) + ", END_DT: " + _formatDate(cmd_uib->getInfo().end_date) + "}]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 17:
	{
		auto cmd_ucst = reinterpret_cast< CmdUpdateCardSpecialTime* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_ucst->getUID()) + "] Atualizou o tempo do Card Special[index=" + std::to_string(cmd_ucst->getInfo().index) + ", TYPEID="
				+ std::to_string(cmd_ucst->getInfo()._typeid) + ", EFEITO{TYPE: " + std::to_string(cmd_ucst->getInfo().efeito) + ", QNTD: " + std::to_string(cmd_ucst->getInfo().efeito_qntd) + "}, TIPO="
				+ std::to_string(cmd_ucst->getInfo().tipo) + ", DATE{REG_DT: " + _formatDate(cmd_ucst->getInfo().use_date) + ", END_DT: " + _formatDate(cmd_ucst->getInfo().end_date) + "}]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 18:
	{
		auto cmd_upsl = reinterpret_cast< CmdUpdatePapelShopInfo* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_upsl->getUID()) + "] Atualizou o Papel Shop Limit[current_cnt="
				+ std::to_string(cmd_upsl->getInfo().current_count) + ", remain_cnt=" + std::to_string(cmd_upsl->getInfo().remain_count) + ", limit_cnt=" + std::to_string(cmd_upsl->getInfo().limit_count) + "]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 19:
	{
		auto cmd_ipsrwl = reinterpret_cast< CmdInsertPapelShopRareWinLog* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_ipsrwl->getUID()) + "] Adicionou Papel Shop Rare Win Log[TYPEID="
				+ std::to_string(cmd_ipsrwl->getInfo().ctx_psi._typeid) + ", QNTD=" + std::to_string(cmd_ipsrwl->getInfo().qntd) + ", COLOR="
				+ std::to_string(cmd_ipsrwl->getInfo().color) + ", PROBABILIDADE=" + std::to_string(cmd_ipsrwl->getInfo().ctx_psi.probabilidade) + "]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 20:
	{
		auto cmd_pchd = reinterpret_cast< CmdPayCaddieHolyDay* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_pchd->getUID()) + "] Pagou as ferias do Caddie[ID="
				+ std::to_string(cmd_pchd->getId()) + "] ate " + cmd_pchd->getEndDate(), CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 21:
	{
		auto cmd_snchd = reinterpret_cast< CmdSetNoticeCaddieHolyDay* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_snchd->getUID()) + "] setou Aviso[check="
				+ (cmd_snchd->getCheck() ? std::string("ON") : std::string("OFF") ) + "] de ferias do Caddie[ID=" + std::to_string(cmd_snchd->getId()) + "]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 22:
	{
		auto cmd_ibrwl = reinterpret_cast< CmdInsertBoxRareWinLog* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_ibrwl->getUID()) + "] Inseriu Box[TYPEID="
				+ std::to_string(cmd_ibrwl->getBoxTypeid()) + "] Rare[TYPEID=" + std::to_string(cmd_ibrwl->getInfo()._typeid) + ", QNTD=" + std::to_string(cmd_ibrwl->getInfo().qntd) + ", RARIDADE="
				+ std::to_string((unsigned short)cmd_ibrwl->getInfo().raridade) + "] Win Log", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 23:
	{
		auto cmd_ispcsrwb = reinterpret_cast< CmdInsertSpinningCubeSuperRareWinBroadcast* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] Inseriu Spinning Cube Super Rare Win Broadcast[MSG=" + cmd_ispcsrwb->getMessage() + ", OPT=" + std::to_string((unsigned short)cmd_ispcsrwb->getOpt()) + "]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 24:
	{
		auto cmd_imrwl = reinterpret_cast< CmdInsertMemorialRareWinLog* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_imrwl->getUID()) + "] Inseriu Memorial Shop[COIN="
				+ std::to_string(cmd_imrwl->getCoinTypeid()) + "] Rare[TYPEID=" + std::to_string(cmd_imrwl->getInfo()._typeid) + ", QNTD="
				+ std::to_string(cmd_imrwl->getInfo().qntd) + ", RARIDADE=" + std::to_string(cmd_imrwl->getInfo().tipo) + "] Win Log", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 26:
	{

		auto cmd_umi = reinterpret_cast< CmdUpdateMascotInfo* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] Player[UID=" + std::to_string(cmd_umi->getUID()) + "] Atualizar Mascot Info[TYPEID="
				+ std::to_string(cmd_umi->getInfo()._typeid) + ", ID=" + std::to_string(cmd_umi->getInfo().id) + ", LEVEL=" + std::to_string((unsigned short)cmd_umi->getInfo().level)
				+ ", EXP=" + std::to_string(cmd_umi->getInfo().exp) + ", FLAG=" + std::to_string((unsigned short)cmd_umi->getInfo().flag) + ", TIPO="
				+ std::to_string(cmd_umi->getInfo().tipo) + ", IS_CASH=" + std::to_string((unsigned short)cmd_umi->getInfo().is_cash) + ", PRICE="
				+ std::to_string(cmd_umi->getInfo().price) + ", MESSAGE=" + std::string(cmd_umi->getInfo().message) + ", END_DT=" + _formatDate(cmd_umi->getInfo().data) + "]", CL_FILE_LOG_AND_CONSOLE));

		break;
	}
	case 27:
	{
		auto cmd_uguai = reinterpret_cast< CmdUpdateGuildUpdateActiviy* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] Atualizou Guild Update Activity[INDEX="
				+ std::to_string(cmd_uguai->getIndex()) + "] com sucesso.", CL_FILE_LOG_AND_CONSOLE));

		break;
	}
	case 28:
	{
		auto cmd_ultp = reinterpret_cast< CmdUpdateLegacyTikiShopPoint* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[channel::SQLDBResponse][Log] Player[UID=" + std::to_string(cmd_ultp->getUID())
				+ "] atualizou Legacy Tiki Shop Point(" + std::to_string(cmd_ultp->getTikiShopPoint()) + ")", CL_FILE_LOG_AND_CONSOLE));

		break;
	}
	case 0:
	default:
		break;
	}

};
