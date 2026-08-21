
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "approach_mission_system.hpp"

#include "../../Projeto IOCP/UTIL/exception.h"
#include "../../Projeto IOCP/TYPE/stda_error.h"
#include "../../Projeto IOCP/UTIL/message_pool.h"

#include "../../Projeto IOCP/DATABASE/normal_manager_db.hpp"

#include "../PANGYA_DB/cmd_approach_missions.hpp"

#include "../../Projeto IOCP/UTIL/iff.h"

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
								_smp::message_pool::getInstance().push(new message("[ApproachMissionSystem::" + std::string(_method) + "][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
#elif defined(__linux__)
#define CATCH_CHECK(_method) }catch (exception& e) { \
								pthread_mutex_unlock(&m_cs); \
								\
								_smp::message_pool::getInstance().push(new message("[ApproachMissionSystem::" + std::string(_method) + "][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
#endif

#define END_CHECK			} \

using namespace stdA;

ApproachMissionSystem::ApproachMissionSystem() : m_mad() {

#if defined(_WIN32)
	InitializeCriticalSection(&m_cs);
#elif defined(__linux__)
	INIT_PTHREAD_MUTEXATTR_RECURSIVE;
	INIT_PTHREAD_MUTEX_RECURSIVE(&m_cs);
	DESTROY_PTHREAD_MUTEXATTR_RECURSIVE;
#endif

	initialize();
}

ApproachMissionSystem::~ApproachMissionSystem() {

	clear();

#if defined(_WIN32)
	DeleteCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_destroy(&m_cs);
#endif
}

void ApproachMissionSystem::initialize() {

	TRY_CHECK;

	CmdApproachMissions cmd_am(true);

	snmdb::NormalManagerDB::getInstance().add(0, &cmd_am, nullptr, nullptr);

	cmd_am.waitEvent();

	if (cmd_am.getException().getCodeError() != 0)
		throw cmd_am.getException();

	if (cmd_am.getInfo().empty())
		throw exception("[ApproachMissionSystem::initialize][Error] nao conseguiu pegar as missions do approach na data base.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::APPROACH_MISSION_SYSTEM, 1, 0));

	m_mad = cmd_am.getInfo();

	_smp::message_pool::getInstance().push(new message("[ApproachMissionSystem::initialize][Log] Approach Mission System carregado com sucesso!", CL_FILE_LOG_AND_CONSOLE));

	m_load = true;

	LEAVE_CHECK;

	CATCH_CHECK("initialize");

	throw;

	END_CHECK;
}

void ApproachMissionSystem::clear() {

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	if (!m_mad.empty())
		m_mad.clear();

	m_load = false;

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif
}

void ApproachMissionSystem::load() {

	if (isLoad())
		clear();

	initialize();
}

bool ApproachMissionSystem::isLoad() {

	bool isLoad = false;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	isLoad = (m_load && !m_mad.empty());

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif

	return isLoad;
}

mission_approach_ex ApproachMissionSystem::drawMission(uint32_t _num_players) {

	mission_approach_ex ma{ 0 };

	if (!isLoad())
		return ma;

	TRY_CHECK;

	std::srand((uint32_t)std::clock() * 7777 * (uint32_t)(uint64_t)&ma);

	if (((std::rand() % 1000) / 10) <= 50) {

		auto index = std::rand() % m_mad.size();

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[ApproachMissionSystem::drawMission][Log] Mission[Number=" + std::to_string(m_mad[index].numero) + "]", CL_FILE_LOG_AND_CONSOLE));
#endif

		if (m_mad[index].flag.bits.players < _num_players) {

			ma.numero = (unsigned char)m_mad[index].numero;
			ma.box_qntd = (unsigned char)m_mad[index].box;
			ma.tipo = m_mad[index].tipo;

			switch (m_mad[index].numero) {
			case 1:
			case 11:
			case 23:
			case 25:
				ma.condition[0] = std::rand() % m_mad[index].flag.bits.condition1;
				break;
			case 6:
			case 14:
			{
				ma.condition[0] = std::rand() % m_mad[index].flag.bits.condition1 + 1;

				if (ma.condition[0] <= 1)
					ma.box_qntd = 3;
				else if (ma.condition[0] <= 3)
					ma.box_qntd = 2;

				break;
			}
			case 7:
				ma.condition[0] = std::rand() % _num_players + 1;
				break;
			case 10:
			{
				auto characters = sIff::getInstance().getCharacter();
				auto it = characters.begin();

				auto choice = std::rand() % characters.size();

				std::advance(it, choice);

				ma.condition[0] = it->first ;
				ma.condition[1] = (uint32_t)choice;
				break;
			}
			case 15:
				ma.condition[0] = std::rand() % _num_players + 1;
				ma.condition[1] = std::rand() % m_mad[index].flag.bits.condition1;
				break;
			case 17:
				ma.condition[1] = (_num_players < 15) ? 150 : 250;
				break;
			case 18:
				ma.condition[0] = std::rand() % m_mad[index].flag.bits.condition1 + 1;
				ma.condition[1] = std::rand() % 9;
				break;
			case 29:
				ma.is_player_uid = true;
				ma.condition[1] = std::rand() % _num_players;
				break;
			}
		}
	}

	LEAVE_CHECK;

	CATCH_CHECK("drawMission");
	END_CHECK

	return ma;
}

void ApproachMissionSystem::SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg) {

	if (_arg == nullptr) {
#ifdef _DEBUG

		_smp::message_pool::getInstance().push(new message("[ApproachMissionSystem::SQLDBResponse][WARNING] _arg is nullptr com msg_id = " + std::to_string(_msg_id), CL_FILE_LOG_AND_CONSOLE));
#endif
		return;
	}

	if (_pangya_db.getException().getCodeError() != 0) {
		_smp::message_pool::getInstance().push(new message("[ApproachMissionSystem::SQLDBResponse][Error] " + _pangya_db.getException().getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	auto *_channel = reinterpret_cast< ApproachMissionSystem* >(_arg);

	switch (_msg_id) {
	case 0:
	default:
		break;
	}
}
