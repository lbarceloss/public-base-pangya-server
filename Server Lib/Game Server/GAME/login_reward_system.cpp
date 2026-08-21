
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "login_reward_system.hpp"

#include "../../Projeto IOCP/UTIL/message_pool.h"
#include "../../Projeto IOCP/UTIL/util_time.h"

#include "../../Projeto IOCP/DATABASE/normal_manager_db.hpp"

#include "../PANGYA_DB/cmd_add_login_reward_player.hpp"
#include "../PANGYA_DB/cmd_login_reward_info.hpp"
#include "../PANGYA_DB/cmd_login_reward_player_info.hpp"
#include "../PANGYA_DB/cmd_update_login_reward.hpp"
#include "../PANGYA_DB/cmd_update_login_reward_player.hpp"

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
								_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::" + std::string(_method) + "][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
#elif defined(__linux__)
#define CATCH_CHECK(_method) }catch (exception& e) { \
								pthread_mutex_unlock(&m_cs); \
								\
								_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::" + std::string(_method) + "][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
#endif

#define END_CHECK			 } \

#define RETURNN_CHECK(_ret_value)	{ \
										LEAVE_CHECK; \
										return (_ret_value); \
									} \

#define RETURN_CHECK()				{ \
										LEAVE_CHECK; \
										return; \
									} \

#define CHECK_SESSION(method) if (!_session.getState()) \
									throw exception("[LoginRewardSystem::" + std::string((method)) +"][Error] player nao esta connectado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::LOGIN_REWARD_SYSTEM, 1, 0)); \

using namespace stdA;

LoginRewardSystem::LoginRewardSystem() : m_events() {

#if defined(_WIN32)
	InitializeCriticalSection(&m_cs);
#elif defined(__linux__)
	INIT_PTHREAD_MUTEXATTR_RECURSIVE;
	INIT_PTHREAD_MUTEX_RECURSIVE(&m_cs);
	DESTROY_PTHREAD_MUTEXATTR_RECURSIVE;
#endif

	initialize();
}

LoginRewardSystem::~LoginRewardSystem() {

	clear();

#if defined(_WIN32)
	DeleteCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_destroy(&m_cs);
#endif
}

void LoginRewardSystem::initialize() {

	TRY_CHECK;

	CmdLoginRewardInfo cmd_lri(true);

	snmdb::NormalManagerDB::getInstance().add(0, &cmd_lri, nullptr, nullptr);

	cmd_lri.waitEvent();

	if (cmd_lri.getException().getCodeError() != 0)
		throw cmd_lri.getException();

	m_events = cmd_lri.getInfo();

	for (auto& el_e : m_events) {

		if (!isEmpty(el_e.end_date) && getLocalTimeDiff(el_e.end_date) > 0ll) {

			el_e.is_end = true;

			snmdb::NormalManagerDB::getInstance().add(1, new CmdUpdateLoginReward(el_e.id, el_e.is_end), LoginRewardSystem::SQLDBResponse, this);
		}
	}

	std::string log = "";

	for (auto& el_e : m_events)
		log += "\nLogin Reward[" + el_e.toString() + "]";

	_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::initialize][Log] Login Reward System Event - Carregou: " + log, CL_FILE_LOG_AND_CONSOLE));

	_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::initialize][Log] Login Reward System carregado com sucesso!", CL_FILE_LOG_AND_CONSOLE));

	m_load = true;

	LEAVE_CHECK;

	CATCH_CHECK("initialize");

	throw;

	END_CHECK;
}

void LoginRewardSystem::clear() {

	TRY_CHECK;

	if (!m_events.empty()) {
		m_events.clear();
		m_events.shrink_to_fit();
	}

	m_load = false;

	LEAVE_CHECK;
	CATCH_CHECK("clear");
	END_CHECK;
}

void LoginRewardSystem::load() {

	if (isLoad())
		clear();

	initialize();
}

bool LoginRewardSystem::isLoad() {

	bool isLoad = false;

	TRY_CHECK;

	isLoad = (m_load);

	LEAVE_CHECK;
	CATCH_CHECK("isLoad");
	END_CHECK;

	return isLoad;
}

void LoginRewardSystem::checkRewardLoginAndSend(player& _session) {
	CHECK_SESSION("checkRewardLoginAndSend");

	TRY_CHECK;

	CmdLoginRewardPlayerInfo cmd_lrpi(_session.m_pi.uid, true);

	for (auto& el_e : m_events) {

		if (el_e.is_end)
			continue;

		if (!isEmpty(el_e.end_date) && getLocalTimeDiff(el_e.end_date) > 0ll) {

			el_e.is_end = true;

			snmdb::NormalManagerDB::getInstance().add(1, new CmdUpdateLoginReward(el_e.id, el_e.is_end), LoginRewardSystem::SQLDBResponse, this);

			continue;
		}

		cmd_lrpi.setId(el_e.id);

		snmdb::NormalManagerDB::getInstance().add(0, &cmd_lrpi, nullptr, nullptr);

		cmd_lrpi.waitEvent();

		if (cmd_lrpi.getException().getCodeError() != 0) {

			_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::checkRewardLoginAndSend][Error][WARNING] " + cmd_lrpi.getException().getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

			continue;
		}

		auto p = cmd_lrpi.getInfo();

		if (p.id == 0ull && p.uid == 0u) {

			p = stPlayerState{ 0ull, _session.m_pi.uid, 1u, 0u, { 0u } };

			if (isEmpty(p.update_date))
				GetLocalTime(&p.update_date);

			CmdAddLoginRewardPlayer cmd_alrp(el_e.id, p, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_alrp, nullptr, nullptr);

			cmd_alrp.waitEvent();

			if (cmd_alrp.getException().getCodeError() != 0) {

				_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::checkRewardLoginAndSend][Error][WARNING] " + cmd_alrp.getException().getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

				continue;
			}

			if (!cmd_alrp.isGood()) {

				_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::checkRewardLoginAndSend][Error][WARINIG] nao conseguiu adicionar o Player[UID="
						+ std::to_string(_session.m_pi.uid) + "] no Login Reward[ID=" + std::to_string(el_e.id) + "] no banco de dados, nao retornou o id do player criado.", CL_FILE_LOG_AND_CONSOLE));

				continue;
			}

			p = cmd_alrp.getPlayerState();

			_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::checkRewardLoginAndSend][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] Primeira participacao do player[" + p.toString() + "] no Login Reward Event[" + el_e.toString() + "]", CL_FILE_LOG_AND_CONSOLE));

		}else {

			if (p.is_clear)
				continue;

			if (getLocalDateDiff(p.update_date) <= 0ll)
				continue;

			p.count_days++;

			GetLocalTime(&p.update_date);

			snmdb::NormalManagerDB::getInstance().add(2, new CmdUpdateLoginRewardPlayer(p), LoginRewardSystem::SQLDBResponse, this);
		}

		if (p.count_days < el_e.days_to_gift)
			continue;

		p.count_seq++;

		if (el_e.type == stLoginReward::eTYPE::N_TIME) {

			if (p.count_seq < el_e.n_times_gift)
				p.count_days = 0u;
			else
				p.is_clear = true;

		}else if (el_e.type == stLoginReward::eTYPE::FOREVER)
			p.count_days = 0u;

		snmdb::NormalManagerDB::getInstance().add(2, new CmdUpdateLoginRewardPlayer(p), LoginRewardSystem::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::checkRewardLoginAndSend][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] ganhou item[" + el_e.item_reward.toString() + "] no Login Reward[" + el_e.toString() + "] com [DAYS=" + std::to_string(p.count_days)
				+ ", SEQ=" + std::to_string(p.count_seq) + ", IS_CLEAR=" + std::string(p.is_clear ? "TRUE" : "FALSE") + "]", CL_FILE_LOG_AND_CONSOLE));

		sendGiftToPlayer(_session, el_e);
	}

	LEAVE_CHECK;
	CATCH_CHECK("checkRewardLoginAndSend");
	END_CHECK;
}

void LoginRewardSystem::updateLoginReward() {

	TRY_CHECK;

	for (auto& el_e : m_events) {

		if (el_e.is_end)
			continue;

		if (!isEmpty(el_e.end_date) && getLocalTimeDiff(el_e.end_date) > 0ll) {

			el_e.is_end = true;

			snmdb::NormalManagerDB::getInstance().add(1, new CmdUpdateLoginReward(el_e.id, el_e.is_end), LoginRewardSystem::SQLDBResponse, this);
		}
	}

	LEAVE_CHECK;
	CATCH_CHECK("updateLoginReward");
	END_CHECK;
}

void LoginRewardSystem::sendGiftToPlayer(player& _session, stLoginReward& _lr) {
	CHECK_SESSION("sendGiftToPlayer");

	auto getItemName = [](uint32_t _typeid) -> std::string {

		std::string ret = "";

		auto base = sIff::getInstance().findCommomItem(_typeid);

		if (base != nullptr)
			ret = std::string(base->name);

		return ret;
	};

	try {

		stItem item{ 0 };
		BuyItem bi{ 0 };

		bi.clear();
		item.clear();

		bi.id = -1;
		bi._typeid = _lr.item_reward._typeid;
		bi.qntd = _lr.item_reward.qntd;
		bi.time = (unsigned short)_lr.item_reward.qntd_time;

		item_manager::initItemFromBuyItem(_session.m_pi, item, bi, false, 0, 0, 1 );

		if (item._typeid == 0)
			_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::sendGiftToPlayer][Error][WARNING] tentou enviar o reward para o player[UID="
					+ std::to_string(_session.m_pi.uid) + "] o Item[" + _lr.item_reward.toString() + "], mas nao conseguiu inicializar o item. Bug", CL_FILE_LOG_AND_CONSOLE));

		auto msg = std::string("Login Reward System - \"" + _lr.getName() + "\": item[ " + getItemName(_lr.item_reward._typeid) + " ]");

		if (MailBoxManager::sendMessageWithItem(0, _session.m_pi.uid, msg, item) <= 0)
			_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::sendGiftToPlayer][Error][WARNING] tentou enviar reward para o player[UID="
					+ std::to_string(_session.m_pi.uid) + "] o Item[" + _lr.item_reward.toString() + "], mas nao conseguiu colocar o item no mail box dele. Bug", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::sendGiftToPlayer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void LoginRewardSystem::SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg) {

	if (_arg == nullptr) {
#ifdef _DEBUG

		_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::SQLDBResponse][WARNING] _arg is nullptr na msg_id = " + std::to_string(_msg_id), CL_FILE_LOG_AND_CONSOLE));
#endif
		return;
	}

	if (_pangya_db.getException().getCodeError() != 0) {
		_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::SQLDBResponse][Error] " + _pangya_db.getException().getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	auto gts = reinterpret_cast<LoginRewardSystem*>(_arg);

	switch (_msg_id) {
	case 1:
	{

		auto cmd_ulr = reinterpret_cast< CmdUpdateLoginReward* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::SQLDBResponse][Log] Atualizaou Login Reward[ID="
				+ std::to_string(cmd_ulr->getId()) + ", IS_END=" + std::string(cmd_ulr->getIsEnd() ? "TRUE" : "FALSE") + "]", CL_FILE_LOG_AND_CONSOLE));

		break;
	}
	case 2:
	{

		auto cmd_ulrp = reinterpret_cast< CmdUpdateLoginRewardPlayer* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[LoginRewardSystem::SQLDBResponse][Log] Atualizou o Player["
				+ cmd_ulrp->getPlayerState().toString() + "] do Login Reward.", CL_FILE_LOG_AND_CONSOLE));

		break;
	}
	case 0:
	default:
		break;
	}
};
