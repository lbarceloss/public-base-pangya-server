
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "attendance_reward_system.hpp"

#include "../../Projeto IOCP/UTIL/exception.h"
#include "../../Projeto IOCP/TYPE/stda_error.h"
#include "../../Projeto IOCP/UTIL/message_pool.h"

#include "../../Projeto IOCP/DATABASE/normal_manager_db.hpp"

#include "../PANGYA_DB/cmd_attendance_reward_item_info.hpp"
#include "../PANGYA_DB/cmd_update_attendance_reward.hpp"

#include "../PACKET/packet_func_sv.h"

#include "../../Projeto IOCP/UTIL/util_time.h"

#include "../UTIL/sys_achievement.hpp"
#include "../UTIL/lottery.hpp"

#include "item_manager.h"
#include "mail_box_manager.hpp"

#if defined(_WIN32)
#define TRY_CHECK			 try { \
								EnterCriticalSection(&m_cs);
#elif defined(__linux__)
#define TRY_CHECK			 try { \
								pthread_mutex_lock(&m_cs);
#endif

#if defined(_WIN32)
#define LEAVE_CHECK				LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
#define LEAVE_CHECK				pthread_mutex_unlock(&m_cs);
#endif

#if defined(_WIN32)
#define CATCH_CHECK(_method) }catch (exception& e) { \
								LeaveCriticalSection(&m_cs); \
								\
								_smp::message_pool::getInstance().push(new message("[AttendanceRewardSystem::" + std::string(_method) + "][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
#elif defined(__linux__)
#define CATCH_CHECK(_method) }catch (exception& e) { \
								pthread_mutex_unlock(&m_cs); \
								\
								_smp::message_pool::getInstance().push(new message("[AttendanceRewardSystem::" + std::string(_method) + "][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
#endif

#define END_CHECK			 } \

using namespace stdA;

#define CHECK_SESSION(method) if (!_session.getState()) \
									throw exception("[AttendanceRewardSystem::" + std::string((method)) +"][Error] player nao esta connectado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ATTENDANCE_REWARD_SYSTEM, 1, 0)); \

#define REQUEST_BEGIN(method) CHECK_SESSION(std::string("request") + (method)); \
							  if (_packet == nullptr) \
									throw exception("[AttendanceRewardSystem::request" + std::string((method)) +"][Error] _packet is nullptr", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ATTENDANCE_REWARD_SYSTEM, 6, 0)); \

AttendanceRewardSystem::AttendanceRewardSystem() : m_load(false), v_item{} {

#if defined(_WIN32)
	InitializeCriticalSection(&m_cs);
#elif defined(__linux__)
	INIT_PTHREAD_MUTEXATTR_RECURSIVE;
	INIT_PTHREAD_MUTEX_RECURSIVE(&m_cs);
	DESTROY_PTHREAD_MUTEXATTR_RECURSIVE;
#endif

	initialize();
}

AttendanceRewardSystem::~AttendanceRewardSystem() {

	clear();

#if defined(_WIN32)
	DeleteCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_destroy(&m_cs);
#endif
}

void AttendanceRewardSystem::initialize() {

	TRY_CHECK;

	CmdAttendanceRewardItemInfo cmd_aric(true);

	snmdb::NormalManagerDB::getInstance().add(0, &cmd_aric, nullptr, nullptr);

	cmd_aric.waitEvent();

	if (cmd_aric.getException().getCodeError() != 0)
		throw cmd_aric.getException();

	v_item = cmd_aric.getInfo();

	_smp::message_pool::getInstance().push(new message("[AttendanceRewardSystem::initialize][Log] Attendance Reward System Carregado com sucesso.", CL_FILE_LOG_AND_CONSOLE));

	m_load = true;

	LEAVE_CHECK;

	CATCH_CHECK("initialize");

	throw;

	END_CHECK;
}

void AttendanceRewardSystem::clear() {

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	if (!v_item.empty()) {
		v_item.clear();
		v_item.shrink_to_fit();
	}

	m_load = false;

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif
}

void AttendanceRewardSystem::sendGrandPrixTicket(player& _session) {
	CHECK_SESSION("sendGrandPrixTicket");

	TRY_CHECK;

	auto pWi = _session.m_pi.findWarehouseItemByTypeid(GRAND_PRIX_TICKET);

	if (pWi == nullptr || pWi->STDA_C_ITEM_QNTD < LIMIT_GRAND_PRIX_TICKET) {

		stItem item{ 0 };

		item.type = 2;
		item.id = -1;
		item._typeid = GRAND_PRIX_TICKET;
		item.qntd = (pWi == nullptr) ? 3 : ((LIMIT_GRAND_PRIX_TICKET - pWi->STDA_C_ITEM_QNTD) >= 3 ? 3 : LIMIT_GRAND_PRIX_TICKET - pWi->STDA_C_ITEM_QNTD);
		item.STDA_C_ITEM_QNTD = (short)item.qntd;

		auto rt = item_manager::RetAddItem::T_ERROR;

		if ((rt = item_manager::addItem(item, _session, 0, 0)) < 0 )
			throw exception("[AttendanceRewardSystem::sendGrandPrixTicket][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou adicionar o Grand Prix Ticket do login, mas nao conseguiu. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ATTENDANCE_REWARD_SYSTEM, 9, 0));

		_smp::message_pool::getInstance().push(new message("[AttendanceRewardSystem::sendGrandPrixTicket][Log] player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] enviou os Grand Prix Ticket para ele com sucesso.", CL_FILE_LOG_AND_CONSOLE));

		if (rt != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH) {

			packet p((unsigned short)0x216);

			p.addUint32((const uint32_t)GetSystemTimeAsUnix());

			p.addUint32(1u);

			p.addUint8(item.type);
			p.addUint32(item._typeid);
			p.addInt32(item.id);
			p.addUint32(item.flag_time);
			p.addBuffer(&item.stat, sizeof(item.stat));
			p.addUint32((item.STDA_C_ITEM_TIME > 0) ? item.STDA_C_ITEM_TIME : item.STDA_C_ITEM_QNTD);
			p.addZeroByte(25);

			packet_func::session_send(p, &_session, 1);
		}

	}else
#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[AttendanceRewardSystem::sendGrandPrixTicket][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] ja tem o limite[LIMIT="
				+ std::to_string(LIMIT_GRAND_PRIX_TICKET) + "] de Grand Prix Ticket.", CL_FILE_LOG_AND_CONSOLE));
#else
		_smp::message_pool::getInstance().push(new message("[AttendanceRewardSystem::sendGrandPrixTicket][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] ja tem o limite[LIMIT="
				+ std::to_string(LIMIT_GRAND_PRIX_TICKET) + "] de Grand Prix Ticket.", CL_ONLY_FILE_LOG));
#endif

	LEAVE_CHECK;

	CATCH_CHECK("sendGrandPrixTicket");
	END_CHECK;
}

AttendanceRewardItemCtx* AttendanceRewardSystem::drawReward(unsigned char _tipo) {

	if (!isLoad())
		throw exception("[AttendanceRewardSystem::drawReward][Error] Attendance Reward not load, please call load method first.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ATTENDANCE_REWARD_SYSTEM, 4, 0));

	AttendanceRewardItemCtx *aric = nullptr;

	TRY_CHECK;

	Lottery lottery((uint64_t)
#if defined(_WIN32)
		GetCurrentThreadId()
#elif defined(__linux__)
		gettid()
#endif
	);

	for (auto& el : v_item)
		if (el.tipo == _tipo)
			lottery.push(400, (size_t)&el);

	auto lc = lottery.spinRoleta();

	if (lc == nullptr)
		throw exception("[AttendanceRewardSystem::drawReward][Error] nao conseguiu rodar a roleta. falhou ao sortear o item. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ATTENDANCE_REWARD_SYSTEM, 5, 0));

	aric = (AttendanceRewardItemCtx*)lc->value;

	LEAVE_CHECK;

	CATCH_CHECK("drawReward");
	END_CHECK;

	return aric;
}

bool AttendanceRewardSystem::passedOneDay(player& _session) {

	SYSTEMTIME st{ 0 };

	GetLocalTime(&st);

	st.wMilliseconds = st.wSecond = st.wMinute = st.wHour = 0u;

	auto diff = getTimeDiff(st, _session.m_pi.ari.last_login);

	return (diff / STDA_10_MICRO_PER_DAY) >= 1;
}

void AttendanceRewardSystem::load() {

	if (isLoad())
		clear();

	initialize();
}

bool AttendanceRewardSystem::isLoad() {

	bool isLoad = false;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	isLoad = (m_load && !v_item.empty());

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif

	return  isLoad;
}

void AttendanceRewardSystem::requestCheckAttendance(player& _session, packet *_packet) {
	REQUEST_BEGIN("CheckAttendance");

	packet p;

	TRY_CHECK;

	if (passedOneDay(_session)) {

		_session.m_pi.ari.login = 0u;

		_session.m_pi.ari.now = _session.m_pi.ari.after;

		_session.m_pi.ari.after.clear();

		snmdb::NormalManagerDB::getInstance().add(1, new CmdUpdateAttendanceReward(_session.m_pi.uid, _session.m_pi.ari), AttendanceRewardSystem::SQLDBResponse, nullptr);

	}else
		_session.m_pi.ari.login = 1u;

	packet_func::pacote248(p, &_session, _session.m_pi.ari);
	packet_func::session_send(p, &_session, 0);

	LEAVE_CHECK;

	CATCH_CHECK("requestCheckAttendace");

	_smp::message_pool::getInstance().push(new message("[AttendanceRewardSystem::checkAttendance][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

	p.init_plain((unsigned short)0x248);

	p.addUint32(STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::ATTENDANCE_REWARD_SYSTEM ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : ~0u);

	packet_func::session_send(p, &_session, 1);

	END_CHECK;
}

void AttendanceRewardSystem::requestUpdateCountLogin(player& _session, packet *_packet) {
	REQUEST_BEGIN("UpdateCountLogin");

	packet p;

	TRY_CHECK;

	if (_session.m_pi.ari.login == 1u)
		throw exception("[AttendanceRewardSystem::requestUpdateCountLogin][Error] player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] tentou pegar o premio do dia logado, mas ele ja pegou. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ATTENDANCE_REWARD_SYSTEM, 8, 0));

	if (!passedOneDay(_session))
		throw exception("[AttendanceRewardSystem::requestUpdateCountLogin][Error] player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] tentou pegar o premio do dia logado, mas ele ja pegou. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ATTENDANCE_REWARD_SYSTEM, 8, 1));

	if (_session.m_pi.ari.counter++ == 0 || _session.m_pi.ari.now._typeid == 0u) {

		auto reward_item = drawReward(1);

		if (reward_item == nullptr)
			throw exception("[AttendanceRewardSystem::requestUpdateCountLogin][Error] nao conseguiu sortear um item para o player. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ATTENDANCE_REWARD_SYSTEM, 7, 0));

		_session.m_pi.ari.now._typeid = reward_item->_typeid;
		_session.m_pi.ari.now.qntd = reward_item->qntd;
	}

	GetLocalTime(&_session.m_pi.ari.last_login);

	_session.m_pi.ari.last_login.wMilliseconds = _session.m_pi.ari.last_login.wSecond = _session.m_pi.ari.last_login.wMinute = _session.m_pi.ari.last_login.wHour = 0u;

	_session.m_pi.ari.login = 1u;

	stItem item{ 0 };

	item.type = 2;
	item.id = -1;
	item._typeid = _session.m_pi.ari.now._typeid;
	item.qntd = _session.m_pi.ari.now.qntd;
	item.STDA_C_ITEM_QNTD = (short)item.qntd;

	std::string msg = "Your Attendance rewards have arrived";

	MailBoxManager::sendMessageWithItem(0, _session.m_pi.uid, msg, item);

	auto reward_item = drawReward(((_session.m_pi.ari.counter + 1) % 10 == 0) ? 2  : 1 );

	if (reward_item == nullptr)
		throw exception("[AttendanceRewardSystem::requestUpdateCountLogin][Error] nao conseguiu sortear um item para o player. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ATTENDANCE_REWARD_SYSTEM, 7, 0));

	_session.m_pi.ari.after._typeid = reward_item->_typeid;
	_session.m_pi.ari.after.qntd = reward_item->qntd;

	snmdb::NormalManagerDB::getInstance().add(1, new CmdUpdateAttendanceReward(_session.m_pi.uid, _session.m_pi.ari), AttendanceRewardSystem::SQLDBResponse, nullptr);

	sendGrandPrixTicket(_session);

	_smp::message_pool::getInstance().push(new message("[AttendanceRewardSystem::requestUpdateCountLogin][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] Atualizou seu Count de Login, e ganhou o Item[TYPEID="
			+ std::to_string(reward_item->_typeid) + ", QNTD=" + std::to_string(reward_item->qntd) + "]", CL_FILE_LOG_AND_CONSOLE));

	SysAchievement sys_achieve;

	sys_achieve.incrementCounter(0x6C4000A0u );

	packet_func::pacote249(p, &_session, _session.m_pi.ari);
	packet_func::session_send(p, &_session, 0);

	sys_achieve.finish_and_update(_session);

	LEAVE_CHECK;

	CATCH_CHECK("requestUpdateCountLogin");

	_smp::message_pool::getInstance().push(new message("[AttendanceRewardSystem::requestUpdateCountLogin][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

	p.init_plain((unsigned short)0x249);

	p.addUint32(STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::ATTENDANCE_REWARD_SYSTEM ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : ~0u);

	packet_func::session_send(p, &_session, 1);

	END_CHECK;
}

void AttendanceRewardSystem::SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg) {

	if (_arg == nullptr) {
#ifdef _DEBUG

		_smp::message_pool::getInstance().push(new message("[AttendanceRewardSystem::SQLDBResponse][WARNING] _arg is nullptr com msg_id = " + std::to_string(_msg_id), CL_FILE_LOG_AND_CONSOLE));
#endif
		return;
	}

	if (_pangya_db.getException().getCodeError() != 0) {
		_smp::message_pool::getInstance().push(new message("[AttendanceRewardSystem::SQLDBResponse][Error] " + _pangya_db.getException().getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	switch (_msg_id) {
	case 1:
	{
		auto cmd_uar = reinterpret_cast< CmdUpdateAttendanceReward* >(&_pangya_db);

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[AttendanceRewardSystem::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_uar->getUID()) + "] Atualizou Attendance Reward com sucesso.", CL_FILE_LOG_AND_CONSOLE));
#else
		_smp::message_pool::getInstance().push(new message("[AttendanceRewardSystem::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_uar->getUID()) + "] Atualizou Attendance Reward com sucesso.", CL_ONLY_FILE_LOG));
#endif

		break;
	}
	case 0:
	default:
		break;
	}

};
