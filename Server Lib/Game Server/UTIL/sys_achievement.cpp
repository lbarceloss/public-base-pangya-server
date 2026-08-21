
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "sys_achievement.hpp"
#include "../../Projeto IOCP/UTIL/exception.h"
#include "../../Projeto IOCP/TYPE/stda_error.h"
#include "../../Projeto IOCP/UTIL/message_pool.h"

#include "../PANGYA_DB/cmd_add_counter_item.hpp"
#include "../PANGYA_DB/cmd_update_counter_item.hpp"
#include "../PANGYA_DB/cmd_update_quest_user.hpp"
#include "../PANGYA_DB/cmd_update_achievement_user.hpp"

#include <algorithm>

#include "../PACKET/packet_func_sv.h"

#include "../../Projeto IOCP/UTIL/iff.h"

#include "../../Projeto IOCP/DATABASE/normal_manager_db.hpp"

#include "../GAME/item_manager.h"

using namespace stdA;

#define CHECK_SESSION(_proc_name, _session) { \
	if (!(_session).isConnected() || !(_session).getState()) \
		throw exception("[SysAchievement::" + std::string((_proc_name)) + "][Error] _session don't connected", STDA_MAKE_ERROR(STDA_ERROR_TYPE::SYS_ACHIEVEMENT, 1, 0)); \
} \

SysAchievement::SysAchievement() : v_c(), v_quest_clear(), map_cii_change(), v_reward() {

	if (!sIff::getInstance().isLoad())
		sIff::getInstance().load();

	Counter c{ 0 };

	for (auto& el : sIff::getInstance().getCounterItem()) {
		c.clear();

		c.value = 0;

		c._typeid = el.second._typeid;

		v_c.push_back(c);
	}
}

SysAchievement::~SysAchievement() {
	clear();
}

void SysAchievement::clear() {

	if (!v_c.empty()) {
		v_c.clear();
		v_c.shrink_to_fit();
	}

	if (!v_quest_clear.empty()) {
		v_quest_clear.clear();
		v_quest_clear.shrink_to_fit();
	}

	if (!map_cii_change.empty())
		map_cii_change.clear();

	if (!v_reward.empty()) {
		v_reward.clear();
		v_reward.shrink_to_fit();
	}
}

void SysAchievement::incrementCounter(uint32_t _typeid) {

	if (_typeid == 0)
		throw exception("[SysAchievement::incrementCounter][Error] _typeid is invalid", STDA_MAKE_ERROR(STDA_ERROR_TYPE::SYS_ACHIEVEMENT, 2, 0));

	std::for_each(v_c.begin(), v_c.end(), [&](auto& el) {
		if (el._typeid == _typeid)
			el.value++;
	});
}

void SysAchievement::incrementCounter(uint32_t _typeid, int32_t _value) {

	if (_typeid == 0)
		throw exception("[SysAchievement::incrementCounter][Error] _typeid is invalid (zero)", STDA_MAKE_ERROR(STDA_ERROR_TYPE::SYS_ACHIEVEMENT, 2, 0));

	if (_value == 0)
		throw exception("[SysAchievement::incrementCounter][Error] CounterItem[typeid=" + std::to_string(_typeid) + "] _value is zero", STDA_MAKE_ERROR(STDA_ERROR_TYPE::SYS_ACHIEVEMENT, 3, 0));

	std::for_each(v_c.begin(), v_c.end(), [&](auto& el) {
		if (el._typeid == _typeid)
			el.value += _value;
	});
}

void SysAchievement::decrementCounter(uint32_t _typeid) {

	if (_typeid == 0)
		throw exception("[SysAchievement::decrementCounter][Error] _typeid is invalid", STDA_MAKE_ERROR(STDA_ERROR_TYPE::SYS_ACHIEVEMENT, 2, 0));

	std::for_each(v_c.begin(), v_c.end(), [&](auto& el) {
		if (el._typeid == _typeid)
			el.value--;
	});
}

void SysAchievement::finish_and_update(player& _session) {

	CHECK_SESSION("finish_and_update", _session);

	packet p;

	try {

		auto v_counter = getCounterChanged();
		auto v_ai = getAchievementChanged(_session, v_counter);

		if (!v_ai.empty()) {

			if (!map_cii_change.empty()) {

				p.init_plain((unsigned short)0x216);

				p.addInt32((const int)GetSystemTimeAsUnix());

				p.addInt32((uint32_t)map_cii_change.size());

				for (auto& el : map_cii_change) {

					snmdb::NormalManagerDB::getInstance().add(3, new CmdUpdateCounterItem(_session.m_pi.uid, el.second), SysAchievement::SQLDBResponse, this);

					p.addUint8(2);
					p.addInt32(el.second._typeid);
					p.addInt32(el.second.id);
					p.addInt32(0);
					p.addInt32(el.second.last_value);
					p.addInt32(el.second.value);
					p.addInt32(el.second.increase_value);
					p.addZeroByte(25);
				}

				packet_func::session_send(p, &_session, 1);
			}

			if (!v_reward.empty()) {

				std::vector< stItem > v_item;
				stItem item{ 0 };

				for (auto& el : v_reward) {

					item.clear();

					item.type = 2;
					item._typeid = el._typeid;
					item.qntd = el.qntd;
					item.STDA_C_ITEM_QNTD = (short)item.qntd;
					item.STDA_C_ITEM_TIME = (short)el.time;

					auto rt = item_manager::RetAddItem::T_INIT_VALUE;

					if ((rt = item_manager::addItem(item, _session, 0, 0)) < 0)
						throw exception("[SysAchievement::finish_and_update][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou adicionar o item[TYPEID="
								+ std::to_string(item._typeid) + "], mas nao conseguiu adicionar o item. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::SYS_ACHIEVEMENT, 50, 0));

					if (rt != item_manager::RetAddItem::T_SUCCESS_PANG_AND_EXP_AND_CP_POUCH)
						v_item.push_back(item);

					_smp::message_pool::getInstance().push(new message("[Log] Achievement->Quest->Reward[TYPEID="
							+ std::to_string(el._typeid) + ", QNTD=" + std::to_string(el.qntd) + ", TIME=" + std::to_string(el.time)
							+ "] para o player: " + std::to_string(_session.m_pi.uid), CL_FILE_LOG_AND_CONSOLE));
				}

				p.init_plain((unsigned short)0x216);

				p.addInt32((const int)GetSystemTimeAsUnix());

				p.addInt32((uint32_t)v_item.size());

				for (auto& el : v_item) {
					p.addUint8(el.type);
					p.addInt32(el._typeid);
					p.addInt32(el.id);
					p.addInt32(el.flag);
					p.addBuffer(&el.stat, sizeof(el.stat));
					p.addInt32((el.STDA_C_ITEM_TIME > 0) ? el.STDA_C_ITEM_TIME : el.STDA_C_ITEM_QNTD);
					p.addZeroByte(25);
				}

				packet_func::session_send(p, &_session, 1);
			}

			p.init_plain((unsigned short)0x22E);

			p.addInt32((uint32_t)v_quest_clear.size());

			for (auto& el : v_quest_clear) {
				p.addInt32(el.achievement_typeid);
				p.addInt32(el.quest_typeid);
			}

			packet_func::session_send(p, &_session, 1);

			CounterItemInfo *cii = nullptr;

			p.init_plain((unsigned short)0x220);

			p.addInt32(0);

			p.addInt32((uint32_t)v_ai.size());

			for (auto& el : v_ai) {
				p.addInt8(el.active);
				p.addInt32(el._typeid);
				p.addInt32(el.id);
				p.addInt32(el.status);
				p.addInt32((uint32_t)el.v_qsi.size());

				for (auto& el2 : el.v_qsi) {
					p.addInt32(el2._typeid);

					if (el2.counter_item_id > 0 && (cii = el.findCounterItemById(el2.counter_item_id)) != nullptr) {
						p.addInt32(cii->_typeid);
						p.addInt32(cii->id);
					}else
						p.addZeroByte(8);

					p.addInt32(el2.clear_date_unix);
				}
			}

			packet_func::session_send(p, &_session, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[SysAchievement::finish_and_update][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

inline std::vector< CounterItemInfo > SysAchievement::getCounterItemInfo(std::vector< AchievementInfoEx >& _v_ai, bool _all_force) {

	std::vector< CounterItemInfo > v_cii;

	std::for_each(_v_ai.begin(), _v_ai.end(), [&](auto& el) {
		std::map< int32_t, CounterItemInfo > map_cii;
		CounterItemInfo *cii = nullptr;

		std::for_each(el.v_qsi.begin(), el.v_qsi.end(), [&](auto& el2) {
			if ((_all_force || el2.clear_date_unix == 0) && (cii = el.findCounterItemById(el2.counter_item_id)) != nullptr)
				map_cii[cii->id] = *cii;
		});

		for (auto& el3 : map_cii)
			v_cii.push_back(el3.second);
	});

	return v_cii;
}

uint32_t SysAchievement::getCharacterCounterTypeId(uint32_t _character_typeid) {

	uint32_t counter_typeid = 0u;

	switch (_character_typeid) {
	case 0x4000000:
		counter_typeid = 0x6C40000Fu;
		break;
	case 0x4000001:
		counter_typeid = 0x6C400010u;
		break;
	case 0x4000002:
		counter_typeid = 0x6C400011u;
		break;
	case 0x4000003:
		counter_typeid = 0x6C400012u;
		break;
	case 0x4000004:
		counter_typeid = 0x6C400013u;
		break;
	case 0x4000005:
		counter_typeid = 0x6C400014u;
		break;
	case 0x4000006:
		counter_typeid = 0x6C400015u;
		break;
	case 0x4000007:
		counter_typeid = 0x6C400016u;
		break;
	case 0x4000008:
		counter_typeid = 0x6C400017u;
		break;
	case 0x4000009:
		counter_typeid = 0x6C400018u;
		break;
	case 0x400000A:
		counter_typeid = 0x6C400040u;
		break;
	case 0x400000B:
		counter_typeid = 0x6C4000B9u;
		break;
	case 0x400000C:
		counter_typeid = 0x6C4000BAu;
		break;
	case 0x400000E:
		counter_typeid = 0x6C4000C4u;
		break;
	}

	return counter_typeid;
}

uint32_t SysAchievement::getCaddieCounterTypeId(uint32_t _caddie_typeid) {

	uint32_t counter_typeid = 0u;

	switch (_caddie_typeid) {
	case 0x1C000000:
		counter_typeid = 0x6C400019u;
		break;
	case 0x1C000001:
		counter_typeid = 0x6C40001Cu;
		break;
	case 0x1C000002:
		counter_typeid = 0x6C40001Bu;
		break;
	case 0x1C000003:
		counter_typeid = 0x6C400042u;
		break;
	case 0x1C000004:
		counter_typeid = 0x6C400043u;
		break;
	case 0x1C000005:
		counter_typeid = 0x6C400041u;
		break;
	case 0x1C000006:
		counter_typeid = 0x6C400044u;
		break;
	case 0x1C000007:
		counter_typeid = 0x6C40001Au;
		break;
	case 0x1C000010:
		counter_typeid = 0x6C40001Cu;
		break;
	case 0x1C000011:
		counter_typeid = 0x6C40001Bu;
		break;
	case 0x1C000012:
		counter_typeid = 0x6C400042u;
		break;
	case 0x1C000013:
		counter_typeid = 0x6C400043u;
		break;
	case 0x1C000014:
		counter_typeid = 0x6C400041u;
		break;
	case 0x1C000015:
		counter_typeid = 0x6C400044u;
		break;
	case 0x1C000016:
		counter_typeid = 0x6C40001Au;
		break;
	case 0x1C00001E:
		counter_typeid = 0x6C400045u;
		break;
	}

	return counter_typeid;
}

uint32_t SysAchievement::getMascotCounterTypeId(uint32_t _mascot_typeid) {

	uint32_t counter_typeid = 0u;

	switch (_mascot_typeid) {
	case 0x40000000:
		counter_typeid = 0x6C400049u;
		break;
	case 0x40000001:
		counter_typeid = 0x6C400048u;
		break;
	case 0x40000002:
		counter_typeid = 0x6C400047u;
		break;
	case 0x40000003:
		counter_typeid = 0x6C400046u;
		break;
	}

	return counter_typeid;
}

uint32_t SysAchievement::getCourseCounterTypeId(uint32_t _course_typeid) {

	uint32_t counter_typeid = 0u;

	switch (_course_typeid) {
	case 0:
		counter_typeid = 0x6C400020u;
		break;
	case 1:
		counter_typeid = 0x6C400021u;
		break;
	case 2:
		counter_typeid = 0x6C400022u;
		break;
	case 3:
		counter_typeid = 0x6C400023u;
		break;
	case 4:
		counter_typeid = 0x6C400024u;
		break;
	case 5:
		counter_typeid = 0x6C400025u;
		break;
	case 6:
		counter_typeid = 0x6C400026u;
		break;
	case 7:
		counter_typeid = 0x6C400027u;
		break;
	case 8:
		counter_typeid = 0x6C400028u;
		break;
	case 9:
		counter_typeid = 0x6C400029u;
		break;
	case 10:
		counter_typeid = 0x6C40002Au;
		break;
	case 11:
		counter_typeid = 0x6C40002Bu;
		break;
	case 13:
		counter_typeid = 0x6C40002Cu;
		break;
	case 14:
		counter_typeid = 0x6C40002Du;
		break;
	case 15:
		counter_typeid = 0x6C40002Eu;
		break;
	case 16:
		counter_typeid = 0x6C40002Fu;
		break;
	case 17:
		counter_typeid = 0x6C40006Du;
		break;
	case 18:
		counter_typeid = 0x6C400031u;
		break;
	case 19:
		counter_typeid = 0x6C400030u;
		break;
	case 20:
		counter_typeid = 0x6C4000A1u;
		break;
	case 21:
		counter_typeid = 0x6C4000C5u;
		break;
	}

	return counter_typeid;
}

uint32_t SysAchievement::getQntdHoleCounterTypeId(uint32_t _qntd_hole) {

	uint32_t counter_typeid = 0u;

	switch (_qntd_hole) {
	case 3:
		counter_typeid = 0x6C400069u;
		break;
	case 6:
		counter_typeid = 0x6C40006Au;
		break;
	case 9:
		counter_typeid = 0x6C40006Bu;
		break;
	case 18:
		counter_typeid = 0x6C40006Cu;
		break;
	}

	return counter_typeid;
}

uint32_t SysAchievement::getScoreCounterTypeId(uint32_t _tacada_num, uint32_t _par_hole) {

	uint32_t counter_typeid = 0u;

	if (_tacada_num == 1)
		counter_typeid = 0x6C400006u;
	else {
		switch ((uint32_t)(_tacada_num - _par_hole)) {
		case (uint32_t)-3:
			counter_typeid = 0x6C400007u;
			break;
		case (uint32_t)-2:
			counter_typeid = 0x6C400008u;
			break;
		case (uint32_t)-1:
			counter_typeid = 0x6C400009u;
			break;
		case 0:
			counter_typeid = 0x6C40000Au;
			break;
		}
	}

	return counter_typeid;
}

uint32_t SysAchievement::getActiveItemCounterTypeId(uint32_t _active_item_typeid) {

	uint32_t counter_typeid = 0u;

	switch (_active_item_typeid) {
	case 0x18000000:
		counter_typeid = 0x6C400072u;
		break;
	case 0x18000001:
		counter_typeid = 0x6C400096u;
		break;
	case 0x18000004:
		counter_typeid = 0x6C400070u;
		break;
	case 0x18000005:
		counter_typeid = 0x6C400074u;
		break;
	case 0x18000006:
		counter_typeid = 0x6C400073u;
		break;
	case 0x18000009:
		counter_typeid = 0x6C4000BDu;
		break;
	case 0x18000010:
		counter_typeid = 0x6C400094u;
		break;
	case 0x18000011:
		counter_typeid = 0x6C400093u;
		break;
	case 0x18000012:
		counter_typeid = 0x6C400092u;
		break;
	case 0x18000025:
		counter_typeid = 0x6C400071u;
		break;
	case 0x18000027:
		counter_typeid = 0x6C400091u;
		break;
	case 0x18000028:
		counter_typeid = 0x6C400095u;
		break;
	case 0x1800002C:
		counter_typeid = 0x6C40008Eu;
		break;
	case 0x1800002D:
		counter_typeid = 0x6C40008Fu;
		break;
	case 0x1800002F:
		counter_typeid = 0x6C400090u;
		break;
	}

	return counter_typeid;
}

uint32_t SysAchievement::getPassiveItemCounterTypeId(uint32_t _passive_item_typeid) {

	uint32_t counter_typeid = 0u;

	switch (_passive_item_typeid) {
	case 0x1A000011:
		counter_typeid = 0x6C400050u;
		break;
	case 0x1A000040:
		counter_typeid = 0x6C400076u;
		break;
	case 0x1A000136:
		counter_typeid = 0x6C400097u;
		break;
	}

	return counter_typeid;
}

int32_t SysAchievement::getScoreNum(uint32_t _tacada_num, uint32_t _par_hole) {

	int32_t score_num = 0;

	if (_tacada_num != 1) {
		switch ((uint32_t)(_tacada_num - _par_hole)) {
		case (uint32_t)-3:
			score_num = 1;
			break;
		case (uint32_t)-2:
			score_num = 2;
			break;
		case (uint32_t)-1:
			score_num = 3;
			break;
		case 0:
			score_num = 4;
			break;
		case 1:
			score_num = 5;
			break;
		case 2:
			score_num = 6;
			break;
		default:
			score_num = -1;
		}
	}

	return score_num;
}

std::vector< SysAchievement::Counter > SysAchievement::getCounterChanged() {

	std::vector< Counter > _v_c;

	std::for_each(v_c.begin(), v_c.end(), [&](auto& el) {
		if (el.value != 0)
			_v_c.push_back(el);
	});

	return _v_c;
}

std::vector< AchievementInfoEx > SysAchievement::getAchievementChanged(player& _session, std::vector< Counter >& _v_c) {

	CHECK_SESSION("getAcheievementChanged", _session);

	std::vector< AchievementInfoEx > v_ai;

	CounterItemInfo *cii = nullptr;

	std::for_each(_v_c.begin(), _v_c.end(), [&](auto& element) {
		for (auto& el : _session.m_pi.mgr_achievement.getAchievementInfo()) {
			if (el.second.status == AchievementInfo::ACTIVED && (cii = el.second.findCounterItemByTypeId(element._typeid)) != nullptr) {

				if (checkAchievement(_session, el.second, element, _v_c))
					v_ai.push_back(el.second);

			}
		}
	});

	Counter count{ 0x6C400039u , 0 };

	for (auto& el : v_quest_clear) {

		if (sIff::getInstance().getItemGroupIdentify(el.achievement_typeid) == iff::QUEST_ITEM)
			count.value++;
	}

	if (count.value > 0) {

		_v_c.push_back(count);

		auto it = std::find_if(_session.m_pi.mgr_achievement.getAchievementInfo().begin(), _session.m_pi.mgr_achievement.getAchievementInfo().end(), [&](auto& el) {
			return (el.second.status == AchievementInfo::ACTIVED && (cii = el.second.findCounterItemByTypeId(count._typeid )) != nullptr);
		});

		if (it != _session.m_pi.mgr_achievement.getAchievementInfo().end()) {

			if (checkAchievement(_session, it->second, count, _v_c))
				v_ai.push_back(it->second);
		}
	}

	return v_ai;
}

#define CHECK_BETTER(_a, _b) (((_a) < 0) ? (_a) >= (_b) : (_a) <= (_b))

#define CHECK_CHANGE_COUNTER_ITEM(_pCii) (map_cii_change.find((_pCii)->id) != map_cii_change.end()) \

unsigned char SysAchievement::checkQuestClear(player& _session, QuestStuffInfo& _qsi, CounterItemInfo& _cii, std::vector< Counter >& _v_c) {

	CHECK_SESSION("checkQuestClear", _session);

	if (_qsi.id <= 0 || _qsi._typeid == 0) {
		_smp::message_pool::getInstance().push(new message("[SysAchievement::checkQuestClear][Error] QuestStuffinfo _qsi is invalid", CL_FILE_LOG_AND_CONSOLE));

		return NOT_MATCH;
	}

	if (_v_c.empty())
		return NOT_MATCH;

	unsigned char ret = NOT_MATCH;

	IFF::QuestStuff *qs = nullptr;

	if ((qs = sIff::getInstance().findQuestStuff(_qsi._typeid)) != nullptr) {

		auto it = _v_c.end();

		for (auto i = 0u; i < (sizeof(qs->counter_item._typeid) / sizeof(qs->counter_item._typeid[0])); ++i) {

			if (qs->counter_item._typeid[i] == 0)
				continue;

			if ((it = VECTOR_FIND_ITEM(_v_c, _typeid, == , qs->counter_item._typeid[i])) == _v_c.end())
				return NOT_MATCH;

			if (_cii._typeid == qs->counter_item._typeid[i]) {

				if (CHECK_BETTER(qs->counter_item.qntd[i], _cii.value + (!CHECK_CHANGE_COUNTER_ITEM(&_cii) ? it->value : 0)))
					ret |= CLEAR;
				else if (qs->counter_item.qntd[i] == 1)
					return NOT_MATCH;
				else
					ret |= INCREMENT_COUNTER;

			}else if (!CHECK_BETTER(qs->counter_item.qntd[i], it->value))
				return NOT_MATCH;
		}

	}else
		_smp::message_pool::getInstance().push(new message("[SysAchievement::checkQuestClear][Error] O quest stuff[TYPEID=" + std::to_string(_qsi._typeid) + "] nao encontrou no IFF dados do server.", CL_FILE_LOG_AND_CONSOLE));

	return ret;
}

#define CHECK_CHANGE_COUNTER_AND_INCREMENT_AND_SAVE { \
	if (map_cii_change.find(pcii->id) == map_cii_change.end()) \
		pcii->value += _c.value; \
		map_cii_change[pcii->id] = { *pcii, pcii->value - _c.value, _c.value }; \
} \

bool SysAchievement::checkAchievement(player& _session, AchievementInfoEx& _ai, Counter& _c, std::vector< Counter >& _v_c) {

	CounterItemInfo *pcii = nullptr, *pcii_new = nullptr, cii{ 0 };
	unsigned char ret = NOT_MATCH;

	bool necessary_update = false;

	for (auto& el : _ai.v_qsi) {
		if (el.clear_date_unix <= 0 && el.counter_item_id != 0 && (pcii = _ai.findCounterItemById(el.counter_item_id)) != nullptr) {

			if ((ret = checkQuestClear(_session, el, *pcii, _v_c)) == CLEAR) {

				necessary_update = true;

				CHECK_CHANGE_COUNTER_AND_INCREMENT_AND_SAVE;

				auto it = _ai.getQuestBase();

				if (it != _ai.v_qsi.end()) {
					if (el._typeid != it->_typeid && el.counter_item_id == it->counter_item_id) {
						cii.clear();

						cii.active = 1;
						cii._typeid = pcii->_typeid;
						cii.value = pcii->value;

						CmdAddCounterItem cmd_aci(_session.m_pi.uid, pcii->_typeid, cii.value, true);

						snmdb::NormalManagerDB::getInstance().add(0, &cmd_aci, nullptr, nullptr);

						cmd_aci.waitEvent();

						if (cmd_aci.getException().getCodeError() != 0 || (cii.id = cmd_aci.getId()) == -1)
							throw exception("[SysAchievement::finish_and_update][Error]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::SYS_ACHIEVEMENT, 12, 0));

						el.counter_item_id = cii.id;

						auto itt = _ai.map_counter_item.insert(std::make_pair(cii.id, cii));

						if (!itt.second)
							throw exception("[SysAchievement::checkAchievement][Error] nao conseguiu adicionar o counter item[TYPEID=" + std::to_string(cii._typeid) + ", ID=" + std::to_string(cii.id)
									+ "] no map de counter item do player: " + std::to_string(_session.m_pi.uid),
									STDA_MAKE_ERROR(STDA_ERROR_TYPE::SYS_ACHIEVEMENT, 13, 0));

						if ((pcii_new = &itt.first->second) == nullptr)
							throw exception("[SysAchievement::checkAchievement][Error] nao conseguiu adicionar o counter item[TYPEID=" + std::to_string(cii._typeid) + ", ID=" + std::to_string(cii.id) +
											"] no map de counter item do player: " + std::to_string(_session.m_pi.uid),
											STDA_MAKE_ERROR(STDA_ERROR_TYPE::SYS_ACHIEVEMENT, 13, 1));

						map_cii_change[pcii_new->id] = { *pcii_new, pcii_new->value - pcii->value, pcii->value };
					}
				}

				auto qs = sIff::getInstance().findQuestStuff(el._typeid);

				if (qs == nullptr)
					throw exception("[SysAchievement::checkAchievement][Error] o quest stuff nao foi encontrado no IFF dados do server. para o player: " + std::to_string(_session.m_pi.uid),
									STDA_MAKE_ERROR(STDA_ERROR_TYPE::SYS_ACHIEVEMENT, 14, 0));

				for (auto i = 0u; i < (sizeof(qs->reward_item._typeid) / sizeof(qs->reward_item._typeid[0])); ++i) {

					if (qs->reward_item._typeid[i] != 0) {

						if (qs->reward_item._typeid[i] == 0x6C000001 )
							_session.m_pi.mgr_achievement.incrementPoint(qs->reward_item.qntd[i]);
						else
							v_reward.push_back({ qs->reward_item._typeid[i], (int)qs->reward_item.qntd[i], (int)qs->reward_item.time[i] });
					}
				}

				el.clear_date_unix = (uint32_t)GetLocalTimeAsUnix();

				snmdb::NormalManagerDB::getInstance().add(1, new CmdUpdateQuestUser(_session.m_pi.uid, el), SysAchievement::SQLDBResponse, this);

				v_quest_clear.push_back({ _ai._typeid, el._typeid });

				_smp::message_pool::getInstance().push(new message("[SysAchievement::checkAchievement][Log] Achievement[ID=" + std::to_string(_ai.id) + "]->Quest Clear[ID=" + std::to_string(el.id) + "] do player: "
						+ std::to_string(_session.m_pi.uid), CL_FILE_LOG_AND_CONSOLE));

				if (_ai.checkAllQuestClear()) {

					_ai.status = AchievementInfo::CONCLUEDED;

					snmdb::NormalManagerDB::getInstance().add(2, new CmdUpdateAchievementUser(_session.m_pi.uid, _ai), SysAchievement::SQLDBResponse, this);

					_smp::message_pool::getInstance().push(new message("[SysAchievement::checkAchievement][Log] Achievement[TYPEID=" + std::to_string(_ai._typeid) + ", ID=" + std::to_string(_ai.id) + "] concluido. do player: "
							+ std::to_string(_session.m_pi.uid), CL_FILE_LOG_AND_CONSOLE));
				}

			}else if (ret & INCREMENT_COUNTER
					&& (sIff::getInstance().getItemGroupIdentify(_ai._typeid) == iff::QUEST_ITEM || _ai.quest_base_typeid != 0)) {

				CHECK_CHANGE_COUNTER_AND_INCREMENT_AND_SAVE;

				necessary_update = true;
			}
		}
	}

	return necessary_update && !map_cii_change.empty();
}

void SysAchievement::SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg) {

	if (_arg == nullptr) {
		_smp::message_pool::getInstance().push(new message("[SysAchievement::SQLDBResponse][WARNING] _arg is nullptr com msg_id = " + std::to_string(_msg_id), CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	if (_pangya_db.getException().getCodeError() != 0) {
		_smp::message_pool::getInstance().push(new message("[SysAchievement::SQLDBResponse][Error] " + _pangya_db.getException().getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	auto *_channel = reinterpret_cast<channel*>(_arg);

	switch (_msg_id) {
	case 1:
	{
		auto cmd_uqu = reinterpret_cast< CmdUpdateQuestUser* >(&_pangya_db);

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[SysAchievement::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_uqu->getUID()) + "] Atualizou o Quest Stuff[TYPEID="
				+ std::to_string(cmd_uqu->getInfo()._typeid) + ", ID=" + std::to_string(cmd_uqu->getInfo().id) + "] com sucesso.", CL_FILE_LOG_AND_CONSOLE));
#else
		_smp::message_pool::getInstance().push(new message("[SysAchievement::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_uqu->getUID()) + "] Atualizou o Quest Stuff[TYPEID="
				+ std::to_string(cmd_uqu->getInfo()._typeid) + ", ID=" + std::to_string(cmd_uqu->getInfo().id) + "] com sucesso.", CL_ONLY_FILE_LOG));
#endif

		break;
	}
	case 2:
	{
		auto cmd_uau = reinterpret_cast< CmdUpdateAchievementUser* >(&_pangya_db);

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[SysAchievement::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_uau->getUID()) + "] Atualizou o Achievement[TYPEID="
				+ std::to_string(cmd_uau->getInfo()._typeid) + ", ID=" + std::to_string(cmd_uau->getInfo().id) + "] com sucesso.", CL_FILE_LOG_AND_CONSOLE));
#else
		_smp::message_pool::getInstance().push(new message("[SysAchievement::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_uau->getUID()) + "] Atualizou o Achievement[TYPEID="
				+ std::to_string(cmd_uau->getInfo()._typeid) + ", ID=" + std::to_string(cmd_uau->getInfo().id) + "] com sucesso.", CL_ONLY_FILE_LOG));
#endif

		break;
	}
	case 3:
	{
		auto cmd_uci = reinterpret_cast< CmdUpdateCounterItem* >(&_pangya_db);

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[SysAchievement::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_uci->getUID()) + "] Atualizou o Counter Item[TYPEID="
				+ std::to_string(cmd_uci->getInfo()._typeid) + ", ID=" + std::to_string(cmd_uci->getInfo().id) + "] com sucesso.", CL_FILE_LOG_AND_CONSOLE));
#else
		_smp::message_pool::getInstance().push(new message("[SysAchievement::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_uci->getUID()) + "] Atualizou o Counter Item[TYPEID="
				+ std::to_string(cmd_uci->getInfo()._typeid) + ", ID=" + std::to_string(cmd_uci->getInfo().id) + "] com sucesso.", CL_ONLY_FILE_LOG));
#endif

		break;
	}
	case 0:
	default:
		break;
	}

}
