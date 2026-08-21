
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "player.hpp"

#include "../../Projeto IOCP/UTIL/exception.h"
#include "../../Projeto IOCP/TYPE/stda_error.h"
#include "../../Projeto IOCP/UTIL/message_pool.h"

#include "../GAME/mail_box_manager.hpp"
#include "../GAME/item_manager.h"

#include "../PANGYA_DB/cmd_player_info.hpp"
#include "../PANGYA_DB/cmd_update_caddie_info.hpp"
#include "../PANGYA_DB/cmd_update_mascot_info.hpp"

#include "../../Projeto IOCP/PANGYA_DB/cmd_update_character_equiped.hpp"

#include "../PANGYA_DB/cmd_update_item_slot.hpp"
#include "../PANGYA_DB/cmd_update_caddie_equiped.hpp"
#include "../PANGYA_DB/cmd_update_ball_equiped.hpp"
#include "../PANGYA_DB/cmd_update_clubset_equiped.hpp"
#include "../PANGYA_DB/cmd_update_mascot_equiped.hpp"
#include "../PANGYA_DB/cmd_update_character_cutin_equiped.hpp"
#include "../PANGYA_DB/cmd_update_skin_equiped.hpp"
#include "../PANGYA_DB/cmd_update_poster_equiped.hpp"
#include "../PANGYA_DB/cmd_update_character_all_part_equiped.hpp"

#include "../PANGYA_DB/cmd_insert_cp_log.hpp"
#include "../PANGYA_DB/cmd_insert_cp_log_item.hpp"

#include "../../Projeto IOCP/DATABASE/normal_manager_db.hpp"

#include "../GAME/premium_system.hpp"

#include "../PACKET/packet_func_sv.h"

using namespace stdA;

player::player(threadpool_base& _threadpool)
	: session(_threadpool), m_pi(), m_gi()
#if STDA_BLOCK_PACKET_ONE_TIME_DISABLE != 0x1
	, m_sbeopt(nullptr)
#endif
{

};

player::~player() {

#if STDA_BLOCK_PACKET_ONE_TIME_DISABLE != 0x1 && STDA_BLOCK_PACKET_ONE_TIME_VER == 0x2
	if (m_sbeopt != nullptr)
		delete m_sbeopt;

	m_sbeopt = nullptr;
#endif
};

bool player::clear() {

	bool ret = true;

	if ((ret = session::clear())) {

#if STDA_BLOCK_PACKET_ONE_TIME_DISABLE != 0x1 && STDA_BLOCK_PACKET_ONE_TIME_VER == 0x2
		if (m_sbeopt != nullptr)
			delete m_sbeopt;

		m_sbeopt = nullptr;
#elif STDA_BLOCK_PACKET_ONE_TIME_DISABLE != 0x1
		sSyncBlockExecOnePerTime::getInstance().removePlayer(m_pi.uid);
#endif

		m_pi.clear();

		m_gi.clear();

		m_gg.m_auth_reply = false;
		m_gg.m_auth_time = 0;
		m_gg.m_csa.Close();
	}

	return ret;
};

unsigned char player::getStateLogged() {
	return m_pi.m_state_logged;
};

uint32_t player::getUID() {
	return m_pi.uid;
};

uint32_t player::getCapability() {
	return m_pi.m_cap.ulCapability;
};

char* player::getNickname() {
	return m_pi.nickname;
};

char* player::getID() {
	return m_pi.id;
};

void player::addExp(uint32_t _exp, bool _upt_on_game) {

	if (_exp == 0)
		throw exception("[player::addExp][Error] _exp is invalid(zero)", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 1, 0));

	try {

		packet p;

		int ret = -1;

		if ((ret = m_pi.addExp(_exp)) >= 0) {

			if (ret > 0) {

				std::vector< stItem > v_item;
				stItem item{ 0 };
				BuyItem bi{ 0 };

				auto level_prize = sIff::getInstance().getLevelUpPrizeItem();

				for (auto i = (m_pi.mi.level - ret + 1); i <= m_pi.mi.level; ++i) {

					v_item.clear();

					auto it = level_prize.end();

					if ((it = level_prize.find(i)) == level_prize.end())
						throw exception("[player::addExp][ErrorSystem] player[UID=" + std::to_string(m_pi.uid) + "] addExp, mas nao encontrou o level up prize[level="
								+ std::to_string(i) + "] no IFF_STRUCT do server", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 2, 0));

					for (auto ii = 0u; ii < (sizeof(it->second.reward._typeid) / sizeof(it->second.reward._typeid[0])); ++ii) {

						if (it->second.reward._typeid[ii] != 0) {
							bi.clear();
							item.clear();

							bi.id = -1;
							bi._typeid = it->second.reward._typeid[ii];
							bi.qntd = it->second.reward.qntd[ii];
							bi.time = (unsigned short)it->second.reward.time[ii];

							item_manager::initItemFromBuyItem(m_pi, item, bi, false, 0, 0, 1 );

							if (item._typeid == 0)
								throw exception("[player::addExp][ErrorSystem] player[UID=" + std::to_string(m_pi.uid) + "] addExp, mas nao conseguiu inicializar o item[TYPEID="
									+ std::to_string(bi._typeid) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 3, 0));

							v_item.push_back(item);
						}
					}

					auto msg = std::string("Level UP! Prize.");

					MailBoxManager::sendMessageWithItem(0, m_pi.uid, msg, v_item);
				}

				p.init_plain((unsigned short)0x10F);

				p.addUint32(0);

				p.addUint8((unsigned char)ret);
				p.addUint8((unsigned char)m_pi.mi.level);

				packet_func::session_send(p, this, 1);
			}
		}

		if (_upt_on_game) {
			p.init_plain((unsigned short)0x1D9);

			p.addUint32(m_pi.mi.level);
			p.addUint32(m_pi.ui.exp);

			packet_func::session_send(p, this, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[player::addExp][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::PLAYER_INFO)
			throw;
	}
};

void player::addCaddieExp(uint32_t _exp) {

	if (_exp == 0)
		throw exception("[player::addCaddieExp][Error] player[UID=" + std::to_string(m_pi.uid) + "] tentou adicionar mais exp[VALUE="
				+ std::to_string(_exp) + "] ao caddie equipado, mas exp is invalid(zero).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 300, 0));

	if (m_pi.ei.cad_info == nullptr)
		throw exception("[player::addCaddieExp][Error] player[UID=" + std::to_string(m_pi.uid) + "] tentou adicionar mais exp[VALUE="
				+ std::to_string(_exp) + "] ao caddie equipado, mas ele nao esta com nenhum caddie equipado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 301, 0));

#define CALC_CADDIE_EXP_LEVEL(_level) 520 + (160 * (_level))
#define LIMIT_LEVEL_CADDIE 3

	if (m_pi.ei.cad_info->level < LIMIT_LEVEL_CADDIE) {

		m_pi.ei.cad_info->exp += _exp;

		uint32_t exp_level = 0u;

		bool upou = false;

		while (m_pi.ei.cad_info->level < LIMIT_LEVEL_CADDIE && m_pi.ei.cad_info->exp >= (exp_level = CALC_CADDIE_EXP_LEVEL(m_pi.ei.cad_info->level))) {

			m_pi.ei.cad_info->level++;

			m_pi.ei.cad_info->exp -= exp_level;

			upou = true;
		}

		snmdb::NormalManagerDB::getInstance().add(1, new CmdUpdateCaddieInfo(m_pi.uid, *m_pi.ei.cad_info), player::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[player::addCaddieExp][Log] player[UID=" + std::to_string(m_pi.uid) + "] add Exp para o Caddie[TYPEID="
				+ std::to_string(m_pi.ei.cad_info->_typeid) + ", ID=" + std::to_string(m_pi.ei.cad_info->id) + ", LEVEL="
				+ std::to_string((unsigned short)m_pi.ei.cad_info->level + 1 ) + ", EXP=" + std::to_string(m_pi.ei.cad_info->exp) + "]"
				+ (upou ? " Upou de Level!" : ""), CL_FILE_LOG_AND_CONSOLE));
	}
};

void player::addMascotExp(uint32_t _exp) {

	if (_exp == 0)
		throw exception("[player::addMascotExp][Error] player[UID=" + std::to_string(m_pi.uid) + "] tentou adicionar mais exp[VALUE="
			+ std::to_string(_exp) + "] ao mascot equipado, mas exp is invalid(zero).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 400, 0));

	if (m_pi.ei.mascot_info == nullptr)
		throw exception("[player::addMascotExp][Error] player[UID=" + std::to_string(m_pi.uid) + "] tentou adicionar mais exp[VALUE="
			+ std::to_string(_exp) + "] ao mascot equipado, mas ele nao esta com nenhum mascot equipado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 401, 0));

#define CALC_MASCOT_EXP_LEVEL(_level) 50  + ((20 + (20 + ((_level) - 1) * 10)) * (_level) / 2)
#define LIMIT_LEVEL_MASCOT 9

	if (m_pi.ei.mascot_info->level < LIMIT_LEVEL_MASCOT) {

		m_pi.ei.mascot_info->exp += _exp;

		uint32_t exp_level = 0u;

		bool upou = false;

		while (m_pi.ei.mascot_info->level < LIMIT_LEVEL_MASCOT && m_pi.ei.mascot_info->exp >= (exp_level = CALC_MASCOT_EXP_LEVEL(m_pi.ei.mascot_info->level))) {

			m_pi.ei.mascot_info->level++;

			m_pi.ei.mascot_info->exp -= exp_level;

			upou = true;
		}

		snmdb::NormalManagerDB::getInstance().add(2, new CmdUpdateMascotInfo(m_pi.uid, *m_pi.ei.mascot_info), player::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[player::addMascotExp][Log] player[UID=" + std::to_string(m_pi.uid) + "] add Exp para o Mascot[TYPEID="
				+ std::to_string(m_pi.ei.mascot_info->_typeid) + ", ID=" + std::to_string(m_pi.ei.mascot_info->id) + ", LEVEL="
				+ std::to_string((unsigned short)m_pi.ei.mascot_info->level + 1 ) + ", EXP=" + std::to_string(m_pi.ei.mascot_info->exp) + "]"
				+ (upou ? " Upou de Level!" : ""), CL_FILE_LOG_AND_CONSOLE));
	}
};

void player::addExp(uint32_t _uid, uint32_t _exp) {

	if (_exp == 0)
		throw exception("[player::addExp][Error] _exp is invalid(zero)", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 1, 0));

	PlayerInfo *pi = nullptr;
	packet *p = nullptr;

	try {

		int ret = -1;

		CmdPlayerInfo cmd_pi(_uid, true);

		snmdb::NormalManagerDB::getInstance().add(0, &cmd_pi, nullptr, nullptr);

		cmd_pi.waitEvent();

		if (cmd_pi.getException().getCodeError() != 0)
			throw cmd_pi.getException();

		pi = new PlayerInfo();

		*(player_info*)pi = cmd_pi.getInfo();

		if ((ret = pi->addExp(_exp)) >= 0) {

			p = new packet();

			if (ret > 0) {

				std::vector< stItem > v_item;
				stItem item{ 0 };
				BuyItem bi{ 0 };

				auto level_prize = sIff::getInstance().getLevelUpPrizeItem();

				for (auto i = (pi->mi.level - ret + 1); i <= pi->mi.level; ++i) {

					v_item.clear();

					auto it = level_prize.end();

					if ((it = level_prize.find(i)) == level_prize.end())
						throw exception("[player::addExp][ErrorSystem] player[UID=" + std::to_string(pi->uid) + "] addExp, mas nao encontrou o level up prize[level="
								+ std::to_string(i) + "] no IFF_STRUCT do server", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 2, 0));

					for (auto ii = 0u; ii < (sizeof(it->second.reward._typeid) / sizeof(it->second.reward._typeid[0])); ++ii) {

						if (it->second.reward._typeid[ii] != 0) {
							bi.clear();
							item.clear();

							bi.id = -1;
							bi._typeid = it->second.reward._typeid[ii];
							bi.qntd = it->second.reward.qntd[ii];
							bi.time = (unsigned short)it->second.reward.time[ii];

							item_manager::initItemFromBuyItem(*pi, item, bi, false, 0, 0, 1 );

							if (item._typeid == 0)
								throw exception("[player::addExp][ErrorSystem] player[UID=" + std::to_string(pi->uid) + "] addExp, mas nao conseguiu inicializar o item[TYPEID="
									+ std::to_string(bi._typeid) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 3, 0));

							v_item.push_back(item);
						}
					}

					auto msg = std::string("Level UP! Prize.");

					MailBoxManager::sendMessageWithItem(0, pi->uid, msg, v_item);
				}
			}
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[player::addExp][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (pi != nullptr) {

			delete pi;

			pi = nullptr;
		}

		if (p != nullptr) {

			delete p;

			p = nullptr;
		}

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::PLAYER_INFO)
			throw;
	}

	if (pi != nullptr) {

		delete pi;

		pi = nullptr;
	}

	if (p != nullptr) {

		delete p;

		p = nullptr;
	}
};

void player::addPang(uint64_t _pang) {

	m_pi.addPang(_pang);

	packet p((unsigned short)0xC8);

	p.addUint64(m_pi.ui.pang);
	p.addUint64(_pang);

	packet_func::session_send(p, this, 1);
};

void player::consomePang(uint64_t _pang) {

	m_pi.consomePang(_pang);

	packet p((unsigned short)0xC8);

	p.addUint64(m_pi.ui.pang);
	p.addUint64(_pang);

	packet_func::session_send(p, this, 1);
};

void player::addCookie(uint64_t _cookie) {

	m_pi.addCookie(_cookie);

	packet p((unsigned short)0x96);

	p.addQWord(&m_pi.cookie);

	packet_func::session_send(p, this, 1);
};

void player::consomeCookie(uint64_t _cookie) {

	m_pi.consomeCookie(_cookie);

	packet p((unsigned short)0x96);

	p.addQWord(&m_pi.cookie);

	packet_func::session_send(p, this, 1);
};

void player::addMoeda(uint64_t _pang, uint64_t _cookie) {

	addPang(_pang);
	addCookie(_cookie);
};

void player::consomeMoeda(uint64_t _pang, uint64_t _cookie) {

	consomePang(_pang);
	consomeCookie(_cookie);
};

void player::saveCPLog(CPLog& _cp_log) {

	auto cp = _cp_log.getCookie();

	try {

		if (cp > 0ull) {

			int64_t log_id = -1ll;

			CmdInsertCPLog cmd_icpl(m_pi.uid, _cp_log, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_icpl, nullptr, nullptr);

			cmd_icpl.waitEvent();

			if (cmd_icpl.getException().getCodeError() != 0)
				throw cmd_icpl.getException();

			if ((log_id = cmd_icpl.getId()) <= 0)
				throw exception("[player::saveCPLog][Error] Player[UID=" + std::to_string(m_pi.uid)
						+ "] nao conseguiu salvar o CPLog[" + _cp_log.toString() + "] do player. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 1300, 0));

			if ((_cp_log.getType() == CPLog::TYPE::BUY_SHOP || _cp_log.getType() == CPLog::TYPE::GIFT_SHOP)
					&& _cp_log.getItemCount() > 0) {

				for (auto& el : _cp_log.getItens())
					snmdb::NormalManagerDB::getInstance().add(3, new CmdInsertCPLogItem(m_pi.uid , log_id, el), player::SQLDBResponse, this);
			}
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[player::saveCPLog][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void player::saveCPLog(uint32_t _uid, CPLog& _cp_log) {

	if (_uid == 0u)
		throw exception("[player::saveCPLog(static)][Error] _uid is invalid(" + std::to_string(_uid) + ")", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 1301, 0));

	auto cp = _cp_log.getCookie();

	try {

		if (cp > 0ull) {

			int64_t log_id = -1ll;

			CmdInsertCPLog cmd_icpl(_uid, _cp_log, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_icpl, nullptr, nullptr);

			cmd_icpl.waitEvent();

			if (cmd_icpl.getException().getCodeError() != 0)
				throw cmd_icpl.getException();

			if ((log_id = cmd_icpl.getId()) <= 0)
				throw exception("[player::saveCPLog(static)][Error] Player[UID=" + std::to_string(_uid)
						+ "] nao conseguiu salvar o CPLog[" + _cp_log.toString() + "] do player. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 1300, 0));

			if ((_cp_log.getType() == CPLog::TYPE::BUY_SHOP || _cp_log.getType() == CPLog::TYPE::GIFT_SHOP)
					&& _cp_log.getItemCount() > 0) {

				for (auto& el : _cp_log.getItens())
					snmdb::NormalManagerDB::getInstance().add(3, new CmdInsertCPLogItem(_uid , log_id, el), player::SQLDBResponse, nullptr);
			}
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[player::saveCPLog(static)][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

bool player::checkCharacterEquipedPart(CharacterInfo& ci) {

	uint32_t def_part = 0u;

	auto angel_wings_typeid = std::find_if(angel_wings, LAST_ELEMENT_IN_ARRAY(angel_wings), [&](auto& el) {
		return (sIff::getInstance().getItemCharIdentify(el) == (ci._typeid & 0x000000FF));
	});

	int32_t angel_wings_part_num = (angel_wings_typeid == LAST_ELEMENT_IN_ARRAY(angel_wings) ? -1 : sIff::getInstance().getItemCharPartNumber(*angel_wings_typeid));

	bool upt_on_db = false;

	for (auto i = 0u; i < (sizeof(ci.parts_typeid) / sizeof(ci.parts_typeid[0])); ++i) {

		if (ci.parts_typeid[i] != 0) {

			if (sIff::getInstance().getItemGroupIdentify(ci.parts_typeid[i]) == iff::PART && (sIff::getInstance().getItemCharPartNumber(ci.parts_typeid[i]) == i || (ci.parts_typeid[i] & 0x08000400 ) == 0x8000400)) {

				auto part = sIff::getInstance().findPart(ci.parts_typeid[i]);

				if (part != nullptr && part->active) {

					if (ci.parts_id[i] == 0) {

						def_part = ((sIff::getInstance().getItemCharPartNumber(ci.parts_typeid[i]) | (uint32_t)(ci._typeid << 5)) << 13) | 0x8000400;

						if ((ci.parts_typeid[i] & def_part) == def_part) {
#ifdef _DEBUG
							_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
									+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i) + "]"
									+ (ci.parts_typeid[i] & 0x00000200  ? std::string(" Sub") : std::string("")) + " Def Part Equiped.", CL_FILE_LOG_AND_CONSOLE));
#endif
						}else {

							ci.unequipPart(part);

							upt_on_db = true;

#ifdef _DEBUG
							_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
									+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i)
									+ "] Not Def Part, Unequip part, Pode ser Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));
#else
							_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
									+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i)
									+ "] Not Def Part, Unequip part, Pode ser Hacker ou bug.", CL_ONLY_FILE_LOG));
#endif
						}

					}else {

						auto parts = m_pi.findWarehouseItemById(ci.parts_id[i]);

						if (parts != nullptr ) {

							char slot = part->position_mask.getSlot(i);

							if (slot == -1) {

								ci.unequipPart(part);

								upt_on_db = true;

#ifdef _DEBUG
								_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
										+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i) + "]"
										+ " desconhecido, Unequip Part. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));
#else
								_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
										+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i) + "]"
										+ " desconhecido, Unequip Part. Hacker ou bug.", CL_ONLY_FILE_LOG));
#endif
							}else if (slot) {

								if (part->level.goodLevel((unsigned char)m_pi.level)) {

									if (angel_wings_part_num == -1l || parts->_typeid != *angel_wings_typeid || m_pi.ui.getQuitRate() < 3.f) {
#ifdef _DEBUG
										_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
												+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i)
												+ "] Equiped.", CL_FILE_LOG_AND_CONSOLE));

#endif
									}else {

										ci.unequipPart(part);

										upt_on_db = true;

#ifdef _DEBUG
										_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
												+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i) + "]"
												+ " Player nao pode equipar a asa [Angel Wings] por que ele nao tem menos que 3% do quit rate[" + std::to_string(m_pi.ui.getQuitRate()) + "]. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));
#else
										_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
												+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i) + "]"
												+ " Player nao pode equipar a asa [Angel Wings] por que ele nao tem menos que 3% do quit rate[" + std::to_string(m_pi.ui.getQuitRate()) + "]. Hacker ou bug.", CL_ONLY_FILE_LOG));
#endif
									}

								}else {

									ci.unequipPart(part);

									upt_on_db = true;

#ifdef _DEBUG
									_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
											+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i) + "]"
											+ " Player[Lv=" + std::to_string(m_pi.level) + "] nao tem o level[is_max=" + std::to_string((unsigned short)part->level.is_max)
											+ ", Lv=" + std::to_string((unsigned short)part->level.level) + "] para equipar esse item. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));
#else
									_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
											+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i) + "]"
											+ " Player[Lv=" + std::to_string(m_pi.level) + "] nao tem o level[is_max=" + std::to_string((unsigned short)part->level.is_max)
											+ ", Lv=" + std::to_string((unsigned short)part->level.level) + "] para equipar esse item. Hacker ou bug.", CL_ONLY_FILE_LOG));
#endif
								}
							}

						}else {

							ci.unequipPart(part);

							upt_on_db = true;

#ifdef _DEBUG
							_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
									+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i) + "]"
									+ " Player nao tem o item, Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));
#else
							_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
									+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i) + "]"
									+ " Player nao tem o item, Hacker ou bug.", CL_ONLY_FILE_LOG));
#endif
						}
					}

				}else {

					if (part != nullptr)
						ci.unequipPart(part);
					else {

						part = sIff::getInstance().findPart((def_part = ((i | (uint32_t)(ci._typeid << 5)) << 13) | 0x8000400));
						ci.parts_typeid[i] = (part != nullptr) ? def_part : 0;
						ci.parts_id[i] = 0;
					}

					upt_on_db = true;

#ifdef _DEBUG
					_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
							+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i) + "]"
							+ " Not found in IFF_STRUCTURE or Not Actived, Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));
#else
					_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
							+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i) + "]"
							+ " Not found in IFF_STRUCTURE or Not Actived, Hacker ou bug.", CL_ONLY_FILE_LOG));
#endif
				}

			}else {

				auto part = sIff::getInstance().findPart(ci.parts_typeid[i]);

				if (part != nullptr)
					ci.unequipPart(part);
				else {

					part = sIff::getInstance().findPart((def_part = ((i | (uint32_t)(ci._typeid << 5)) << 13) | 0x8000400));
					ci.parts_typeid[i] = (part != nullptr) ? def_part : 0;
					ci.parts_id[i] = 0;
				}

				upt_on_db = true;

#ifdef _DEBUG
				_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
						+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i) + "]"
						+ " Slot wrong or Group id not match, Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));
#else
				_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
						+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i) + "]"
						+ " Slot wrong or Group id not match, Hacker ou bug.", CL_ONLY_FILE_LOG));
#endif
			}

		}else {
#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedPart][Log] Player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
					+ ", ID=" + std::to_string(ci.id) + "] Part[TYPEID=" + std::to_string(ci.parts_typeid[i]) + ", Slot=" + std::to_string(i)
					+ "] Not Equiped.", CL_FILE_LOG_AND_CONSOLE));

#endif
		}
	}

	return upt_on_db;
}

bool player::checkCharacterEquipedAuxPart(CharacterInfo& ci) {

	bool upt_on_db = false;

	for (auto i = 0u; i < (sizeof(ci.auxparts) / sizeof(ci.auxparts[0])); ++i) {

		if (ci.auxparts[i] != 0) {

			if (sIff::getInstance().getItemGroupIdentify(ci.auxparts[i]) == iff::AUX_PART) {

				auto aux = sIff::getInstance().findAuxPart(ci.auxparts[i]);
				auto pAux = m_pi.findWarehouseItemByTypeid(ci.auxparts[i]);

				if (aux != nullptr && aux->active && pAux != nullptr) {

					if (aux->level.goodLevel((unsigned char)m_pi.level)) {

						if (aux->cc[0]  == 0 || pAux->c[0] > 0) {
#ifdef _DEBUG
							_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedAuxPart][Log] player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
									+ ", ID=" + std::to_string(ci.id) + "] AuxPart[TYPEID=" + std::to_string(ci.auxparts[i]) + ", SLOT="
									+ std::to_string(i) + "] Equiped.", CL_FILE_LOG_AND_CONSOLE));

#endif
						}else {

							ci.auxparts[i] = 0;

							upt_on_db = true;
#ifdef _DEBUG
							_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedAuxPart][Log] player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
									+ ", ID=" + std::to_string(ci.id) + "] AuxPart[TYPEID=" + std::to_string(ci.auxparts[i]) + ", SLOT="
									+ std::to_string(i) + "] Not have enough count item for equip, Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));
#else
							_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedAuxPart][Log] player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
									+ ", ID=" + std::to_string(ci.id) + "] AuxPart[TYPEID=" + std::to_string(ci.auxparts[i]) + ", SLOT="
									+ std::to_string(i) + "] Not have enough count item for equip, Hacker ou bug.", CL_ONLY_FILE_LOG));
#endif
						}

					}else {

						ci.auxparts[i] = 0;

						upt_on_db = true;

#ifdef _DEBUG
						_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedAuxPart][Log] player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
								+ ", ID=" + std::to_string(ci.id) + "] AuxPart[TYPEID=" + std::to_string(ci.auxparts[i]) + ", SLOT=" + std::to_string(i)
								+ "]  Player[Lv=" + std::to_string(m_pi.level) + "] nao tem o level[is_max=" + std::to_string((unsigned short)aux->level.is_max)
								+ ", Lv=" + std::to_string((unsigned short)aux->level.level) + "] para equipar esse item. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));
#else
						_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedAuxPart][Log] player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
								+ ", ID=" + std::to_string(ci.id) + "] AuxPart[TYPEID=" + std::to_string(ci.auxparts[i]) + ", SLOT=" + std::to_string(i)
								+ "]  Player[Lv=" + std::to_string(m_pi.level) + "] nao tem o level[is_max=" + std::to_string((unsigned short)aux->level.is_max)
								+ ", Lv=" + std::to_string((unsigned short)aux->level.level) + "] para equipar esse item. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));
#endif
					}

				}else {

					ci.auxparts[i] = 0;

					upt_on_db = true;

#ifdef _DEBUG
					_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedAuxPart][Log] player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
							+ ", ID=" + std::to_string(ci.id) + "] AuxPart[TYPEID=" + std::to_string(ci.auxparts[i]) + ", SLOT=" + std::to_string(i)
							+ "] Not found in IFF_STRUCTURE, Not Actived or player nao tem esse item, Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));
#else
					_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedAuxPart][Log] player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
							+ ", ID=" + std::to_string(ci.id) + "] AuxPart[TYPEID=" + std::to_string(ci.auxparts[i]) + ", SLOT=" + std::to_string(i)
							+ "] Not found in IFF_STRUCTURE, Not Actived or player nao tem esse item, Hacker ou bug.", CL_ONLY_FILE_LOG));
#endif
				}

			}else {

				ci.auxparts[i] = 0;

				upt_on_db = true;

#ifdef _DEBUG
				_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedAuxPart][Log] player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
						+ ", ID=" + std::to_string(ci.id) + "] AuxPart[TYPEID=" + std::to_string(ci.auxparts[i]) + ", SLOT="
						+ std::to_string(i) + "] Group id not match, Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));
#else
				_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedAuxPart][Log] player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
						+ ", ID=" + std::to_string(ci.id) + "] AuxPart[TYPEID=" + std::to_string(ci.auxparts[i]) + ", SLOT="
						+ std::to_string(i) + "] Group id not match, Hacker ou bug.", CL_ONLY_FILE_LOG));
#endif
			}

		}else {
#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedAuxPart][Log] player[UID=" + std::to_string(m_pi.uid) + "] Character[TYPEID=" + std::to_string(ci._typeid)
					+ ", ID=" + std::to_string(ci.id) + "] AuxPart[TYPEID=" + std::to_string(ci.auxparts[i]) + ", SLOT="
					+ std::to_string(i) + "] Not Equiped.", CL_FILE_LOG_AND_CONSOLE));

#endif
		}
	}

	return upt_on_db;
}

bool player::checkCharacterEquipedCutin(CharacterInfo& ci) {

	bool upt_on_db = false;

	for (auto i = 0u; i < (sizeof(ci.cut_in) / sizeof(ci.cut_in[0])); ++i) {

		if (ci.cut_in[i] != 0) {

			auto pCutin = m_pi.findWarehouseItemById(ci.cut_in[i]);

			if (pCutin == nullptr) {

				ci.cut_in[i] = 0;

				upt_on_db = true;

				_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedCutin][Error] player[UID=" + std::to_string(m_pi.uid)
						+ "] Character[TYPEID=" + std::to_string(ci._typeid)
						+ ", ID=" + std::to_string(ci.id) + "] Not Have Cutin[ID=" + std::to_string(ci.cut_in[i]) + ", SLOT=" + std::to_string(i)
						+ "], but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

			}else {

				auto cutin = sIff::getInstance().findSkin(pCutin->_typeid);

				if (cutin != nullptr && !cutin->level.goodLevel((unsigned char)m_pi.level)) {

					ci.cut_in[i] = 0;

					upt_on_db = true;

					_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedCutin][Error] player[UID=" + std::to_string(m_pi.uid)
							+ "] Character[TYPEID=" + std::to_string(ci._typeid)
							+ ", ID=" + std::to_string(ci.id) + "] Cutin[TYPEID=" + std::to_string(pCutin->_typeid) + " ID=" + std::to_string(pCutin->id) + ", SLOT=" + std::to_string(i)
							+ "]  Player[Lv=" + std::to_string(m_pi.level) + "] nao tem o level[is_max=" + std::to_string((unsigned short)cutin->level.is_max)
							+ ", Lv=" + std::to_string((unsigned short)cutin->level.level) + "] para equipar esse item. Hacker ou bug..", CL_FILE_LOG_AND_CONSOLE));

				}else if (cutin == nullptr) {

					ci.cut_in[i] = 0;

					upt_on_db = true;

					_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquipedCutin][Error] player[UID=" + std::to_string(m_pi.uid)
							+ "] Character[TYPEID=" + std::to_string(ci._typeid)
							+ ", ID=" + std::to_string(ci.id) + "] Not Have Cutin[TYPEID=" + std::to_string(pCutin->_typeid) + " ID=" + std::to_string(pCutin->id) + ", SLOT=" + std::to_string(i)
							+ "] in IFF_STRUCT of server, but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
				}
			}
		}
	}

	return upt_on_db;
}

void player::checkCharacterAllItemEquiped(CharacterInfo& ci) {

	auto ret = checkCharacterEquipedPart(ci);

	ret |= checkCharacterEquipedAuxPart(ci);

	ret |= checkCharacterEquipedCutin(ci);

	if (ret)
		snmdb::NormalManagerDB::getInstance().add(5, new CmdUpdateCharacterAllPartEquiped(m_pi.uid, ci), player::SQLDBResponse, this);
}

bool player::checkSkinEquiped(UserEquip& _ue) {

	bool upt_on_db = false;
	unsigned tmp_typeid = 0, tmp_id = 0;

	for (auto i = 0u; i < (sizeof(_ue.skin_typeid) / sizeof(_ue.skin_typeid[0])); ++i) {

		if (_ue.skin_typeid[i] != 0) {

			auto pSkin = m_pi.findWarehouseItemByTypeid(_ue.skin_typeid[i]);

			if (pSkin == nullptr) {

				tmp_typeid = _ue.skin_typeid[i];
				tmp_id = _ue.skin_id[i];

				_ue.skin_id[i] = 0;
				_ue.skin_typeid[i] = 0;

				upt_on_db = true;

				_smp::message_pool::getInstance().push(new message("[player::checkSkinEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
						+ "] Not Have Skin[TYPEID=" + std::to_string(tmp_typeid) + ", ID=" + std::to_string(tmp_id)
						+ ", SLOT=" + std::to_string(i) + "], but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

			}else {

				auto skin = sIff::getInstance().findSkin(pSkin->_typeid);

				if (skin != nullptr && !skin->level.goodLevel((unsigned char)m_pi.level)) {

					_ue.skin_id[i] = 0;
					_ue.skin_typeid[i] = 0;

					upt_on_db = true;

					_smp::message_pool::getInstance().push(new message("[player::checkSkinEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
							+ "] Skin[TYPEID=" + std::to_string(pSkin->_typeid) + " ID=" + std::to_string(pSkin->id) + ", SLOT=" + std::to_string(i)
							+ "]  Player[Lv=" + std::to_string(m_pi.level) + "] nao tem o level[is_max=" + std::to_string((unsigned short)skin->level.is_max)
							+ ", Lv=" + std::to_string((unsigned short)skin->level.level) + "] para equipar esse item. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

				}else if (skin != nullptr && sIff::getInstance().IsTitle(pSkin->_typeid)) {

					uint32_t title_num = sIff::getInstance().getItemTitleNum(pSkin->_typeid);

					auto check_title = m_pi.getTitleCallBack(title_num);

					if (check_title != nullptr  && check_title->exec() == 0 ) {

						_ue.skin_id[i] = 0;
						_ue.skin_typeid[i] = 0;

						upt_on_db = true;

						_smp::message_pool::getInstance().push(new message("[player::checkSkinEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
								+ "] Skin[TYPEID=" + std::to_string(pSkin->_typeid) + " ID=" + std::to_string(pSkin->id) + ", SLOT=" + std::to_string(i)
								+ "] nao passou na condition TITLE[NUM=" + std::to_string(title_num) + "], para equipar esse item. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));
					}

				}else if (skin == nullptr) {

					_ue.skin_id[i] = 0;
					_ue.skin_typeid[i] = 0;

					upt_on_db = true;

					_smp::message_pool::getInstance().push(new message("[player::checkSkinEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
							+ "] Not Have Skin[TYPEID=" + std::to_string(pSkin->_typeid) + ", ID=" + std::to_string(pSkin->id)
							+ ", SLOT=" + std::to_string(i) + "] in IFF_STRUCT of server, but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
				}
			}
		}
	}

	return upt_on_db;
}

bool player::checkPosterEquiped(UserEquip& _ue) {

	bool upt_on_db = false;
	int32_t tmp_typeid = 0;

	for (auto i = 0u; i < (sizeof(_ue.poster) / sizeof(_ue.poster[0])); ++i) {

		if (_ue.poster[i] != 0) {

			auto pPoster = m_pi.findMyRoomItemByTypeid(_ue.poster[i]);

			if (pPoster == nullptr) {

				tmp_typeid = _ue.poster[i];

				_ue.poster[i] = 0;

				upt_on_db = true;

				_smp::message_pool::getInstance().push(new message("[player::checkPosterEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
						+ "] Not Have Poster[TYPEID=" + std::to_string(tmp_typeid) + ", SLOT=" + std::to_string(i)
						+ "], but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

			}else {

				auto poster = sIff::getInstance().findFurniture(pPoster->_typeid);

				if (poster != nullptr && !poster->level.goodLevel((unsigned char)m_pi.level)) {

					_ue.poster[i] = 0;

					upt_on_db = true;

					_smp::message_pool::getInstance().push(new message("[player::checkPosterEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
							+ "] Poster[TYPEID=" + std::to_string(pPoster->_typeid) + " ID=" + std::to_string(pPoster->id) + ", SLOT=" + std::to_string(i)
							+ "]  Player[Lv=" + std::to_string(m_pi.level) + "] nao tem o level[is_max=" + std::to_string((unsigned short)poster->level.is_max)
							+ ", Lv=" + std::to_string((unsigned short)poster->level.level) + "] para equipar esse item. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

				}else if (poster == nullptr) {

					_ue.poster[i] = 0;

					upt_on_db = true;

					_smp::message_pool::getInstance().push(new message("[player::checkPosterEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
							+ "] Not Have Poster[TYPEID=" + std::to_string(pPoster->_typeid) + ", ID=" + std::to_string(pPoster->id)
							+ ", SLOT=" + std::to_string(i) + "] in IFF_STRUCT of server, but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
				}
			}
		}
	}

	return upt_on_db;
}

bool player::checkCharacterEquiped(UserEquip& _ue) {

	bool upt_on_db = false;
	int32_t tmp_id = 0;

	if (_ue.character_id != 0) {

		if (m_pi.findCharacterById(_ue.character_id) == nullptr) {

			tmp_id = _ue.character_id;

			try {

				equipDefaultCharacter(_ue);

			}catch (exception& e) {

				_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquiped][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
			}

			upt_on_db = true;

			_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
					+ "] Not Have Character[ID=" + std::to_string(tmp_id) + "], but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

		}

	}else {

		try {

			equipDefaultCharacter(_ue);

		}catch (exception& e) {

			_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquiped][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		}

		upt_on_db = true;

		_smp::message_pool::getInstance().push(new message("[player::checkCharacterEquiped][Error][WARNING] player[UID=" + std::to_string(m_pi.uid)
				+ "] nao tem um character equipado. Bug", CL_FILE_LOG_AND_CONSOLE));
	}

	return upt_on_db;
}

bool player::checkCaddieEquiped(UserEquip& _ue) {

	bool upt_on_db = false;
	int32_t tmp_id = 0;

	if (_ue.caddie_id != 0) {

		auto pCaddie = m_pi.findCaddieById(_ue.caddie_id);

		if (pCaddie == nullptr) {

			tmp_id = _ue.caddie_id;

			_ue.caddie_id = 0;
			m_pi.ei.cad_info = nullptr;

			upt_on_db = true;

			_smp::message_pool::getInstance().push(new message("[player::checkCaddieEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
					+ "] Not Have Caddie[ID=" + std::to_string(tmp_id) + "], but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

		}else {

			auto caddie = sIff::getInstance().findCaddie(pCaddie->_typeid);

			if (caddie != nullptr && !caddie->level.goodLevel((unsigned char)m_pi.level)) {

				_ue.caddie_id = 0;
				m_pi.ei.cad_info = nullptr;

				upt_on_db = true;

				_smp::message_pool::getInstance().push(new message("[player::checkCaddieEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
						+ "] Caddie[TYPEID=" + std::to_string(pCaddie->_typeid) + " ID=" + std::to_string(pCaddie->id)
						+ " Player[Lv=" + std::to_string(m_pi.level) + "] nao tem o level[is_max=" + std::to_string((unsigned short)caddie->level.is_max)
						+ ", Lv=" + std::to_string((unsigned short)caddie->level.level) + "] para equipar esse item. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

			}else if (caddie != nullptr && pCaddie->rent_flag == 2 && getLocalTimeDiffDESC(pCaddie->end_date) <= 0ll) {

				_ue.caddie_id = 0;
				m_pi.ei.cad_info = nullptr;

				upt_on_db = true;

				_smp::message_pool::getInstance().push(new message("[player::checkCaddieEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
						+ "] Caddie[TYPEID=" + std::to_string(pCaddie->_typeid) + " ID=" + std::to_string(pCaddie->id)
						+ ", END_DATE=" + _formatDate(pCaddie->end_date) + "] esta de ferias, nao pode equipar esse caddie. Hacker ou bug.",
						CL_FILE_LOG_AND_CONSOLE));

			}else if (caddie == nullptr) {

				_ue.caddie_id = 0;
				m_pi.ei.cad_info = nullptr;

				upt_on_db = true;

				_smp::message_pool::getInstance().push(new message("[player::checkCaddieEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
						+ "] Not Have Caddie[TYPEID=" + std::to_string(pCaddie->_typeid) + ", ID=" + std::to_string(pCaddie->id)
						+ "] in IFF_STRUCT of server, but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
			}
		}
	}

	return upt_on_db;
}

bool player::checkMascotEquiped(UserEquip& _ue) {

	bool upt_on_db = false;
	int32_t tmp_id = 0;

	if (_ue.mascot_id != 0) {

		auto pMascot = m_pi.findMascotById(_ue.mascot_id);

		if (pMascot == nullptr) {

			tmp_id = _ue.mascot_id;

			_ue.mascot_id = 0;
			m_pi.ei.mascot_info = nullptr;

			upt_on_db = true;

			_smp::message_pool::getInstance().push(new message("[player::checkMascotEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
					+ "] Not Have Mascot[ID=" + std::to_string(tmp_id) + "], but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

		}else {

			auto mascot = sIff::getInstance().findMascot(pMascot->_typeid);

			if (mascot != nullptr && !mascot->level.goodLevel((unsigned char)m_pi.level)) {

				_ue.mascot_id = 0;
				m_pi.ei.mascot_info = nullptr;

				upt_on_db = true;

				_smp::message_pool::getInstance().push(new message("[player::checkMascotEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
						+ "] Mascot[TYPEID=" + std::to_string(pMascot->_typeid) + " ID=" + std::to_string(pMascot->id)
						+ " Player[Lv=" + std::to_string(m_pi.level) + "] nao tem o level[is_max=" + std::to_string((unsigned short)mascot->level.is_max)
						+ ", Lv=" + std::to_string((unsigned short)mascot->level.level) + "] para equipar esse item. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

			}else if (mascot != nullptr && pMascot->tipo == 1 && getLocalTimeDiffDESC(pMascot->data) <= 0ll) {

				_ue.mascot_id = 0;
				m_pi.ei.mascot_info = nullptr;

				upt_on_db = true;

				_smp::message_pool::getInstance().push(new message("[player::checkMascotEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
						+ "] Caddie[TYPEID=" + std::to_string(pMascot->_typeid) + " ID=" + std::to_string(pMascot->id)
						+ ", END_DATE=" + _formatDate(pMascot->data) + "] acabou o tempo do mascot, nao pode equipar esse mascot. Hacker ou bug.",
						CL_FILE_LOG_AND_CONSOLE));

			}else if (mascot == nullptr) {

				_ue.mascot_id = 0;
				m_pi.ei.mascot_info = nullptr;

				upt_on_db = true;

				_smp::message_pool::getInstance().push(new message("[player::checkMascotEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
						+ "] Not Have Mascot[TYPEID=" + std::to_string(pMascot->_typeid) + ", ID=" + std::to_string(pMascot->id)
						+ "] in IFF_STRCT of server, but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
			}
		}
	}

	return upt_on_db;
}

bool player::checkClubSetEquiped(UserEquip& _ue){

	bool upt_on_db = false;
	int32_t tmp_id = 0;

	if (_ue.clubset_id != 0) {

		auto pClubSet = m_pi.findWarehouseItemById(_ue.clubset_id);

		if (pClubSet == nullptr) {

			auto tmp_id = _ue.clubset_id;

			try {

				equipDefaultClubSet(_ue);

			}catch (exception& e) {

				_smp::message_pool::getInstance().push(new message("[player::checkClubSetEquiped][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
			}

			upt_on_db = true;

			_smp::message_pool::getInstance().push(new message("[player::checkClubSetEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
					+ "] Not Have ClubSet[ID=" + std::to_string(tmp_id) + "], but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

		}else {

			auto clubset = sIff::getInstance().findClubSet(pClubSet->_typeid);

			if (clubset != nullptr && !clubset->level.goodLevel((unsigned char)m_pi.level)) {

				try {

					equipDefaultClubSet(_ue);

				}catch (exception& e) {

					_smp::message_pool::getInstance().push(new message("[player::checkClubSetEquiped][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
				}

				upt_on_db = true;

				_smp::message_pool::getInstance().push(new message("[player::checkClubSetEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
						+ "] ClubSet[TYPEID=" + std::to_string(pClubSet->_typeid) + " ID=" + std::to_string(pClubSet->id)
						+ " Player[Lv=" + std::to_string(m_pi.level) + "] nao tem o level[is_max=" + std::to_string((unsigned short)clubset->level.is_max)
						+ ", Lv=" + std::to_string((unsigned short)clubset->level.level) + "] para equipar esse item. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

			}else if (clubset == nullptr) {

				try {

					equipDefaultClubSet(_ue);

				}catch (exception& e) {

					_smp::message_pool::getInstance().push(new message("[player::checkClubSetEquiped][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
				}

				upt_on_db = true;

				_smp::message_pool::getInstance().push(new message("[player::checkClubSetEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
						+ "] Not Have ClubSet[TYPEID=" + std::to_string(pClubSet->_typeid) + ", ID=" + std::to_string(pClubSet->id)
						+ "] in IFF_STRUCT of server, but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
			}
		}

	}else {

		try {

			equipDefaultClubSet(_ue);

		}catch (exception& e) {

			_smp::message_pool::getInstance().push(new message("[player::checkClubSetEquiped][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		}

		upt_on_db = true;

		_smp::message_pool::getInstance().push(new message("[player::checkClubSetEquiped][Error][WARNING] player[UID=" + std::to_string(m_pi.uid)
				+ "] nao esta com um ClubSet equipado. Bug", CL_FILE_LOG_AND_CONSOLE));
	}

	return upt_on_db;
}

bool player::checkBallEquiped(UserEquip& _ue) {

	bool upt_on_db = false;
	uint32_t tmp_typeid = 0u;

	if (_ue.ball_typeid != 0) {

		auto pBall = m_pi.findWarehouseItemByTypeid(_ue.ball_typeid);

		if (pBall == nullptr) {

			auto tmp_typeid = _ue.ball_typeid;

			try {

				equipDefaultBall(_ue);

			}catch (exception& e) {

				_smp::message_pool::getInstance().push(new message("[player::checkBallEquiped][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
			}

			upt_on_db = true;

			_smp::message_pool::getInstance().push(new message("[player::checkBallEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
					+ "] Not Have Ball[TYPEID=" + std::to_string(tmp_typeid) + "], but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

		}else {

			auto ball = sIff::getInstance().findBall(pBall->_typeid);

			if (ball != nullptr && !ball->level.goodLevel((unsigned char)m_pi.level)) {

				try {

					equipDefaultBall(_ue);

				}catch (exception& e) {

					_smp::message_pool::getInstance().push(new message("[player::checkBallEquiped][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
				}

				upt_on_db = true;

				_smp::message_pool::getInstance().push(new message("[player::checkBallEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
						+ "] Ball[TYPEID=" + std::to_string(pBall->_typeid) + " ID=" + std::to_string(pBall->id)
						+ " Player[Lv=" + std::to_string(m_pi.level) + "] nao tem o level[is_max=" + std::to_string((unsigned short)ball->level.is_max)
						+ ", Lv=" + std::to_string((unsigned short)ball->level.level) + "] para equipar esse item. Hacker ou bug.", CL_FILE_LOG_AND_CONSOLE));

			}else if (ball == nullptr) {

				try {

					equipDefaultBall(_ue);

				}catch (exception& e) {

					_smp::message_pool::getInstance().push(new message("[player::checkBallEquiped][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
				}

				upt_on_db = true;

				_smp::message_pool::getInstance().push(new message("[player::checkBallEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
						+ "] Not Have Ball[TYPEID=" + std::to_string(pBall->_typeid) + ", ID=" + std::to_string(pBall->id)
						+ "] in IFF_STRUCT of server, but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
			}
		}

	}else {

		try {

			equipDefaultBall(_ue);

		}catch (exception& e) {

			_smp::message_pool::getInstance().push(new message("[player::checkBallEquiped][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		}

		upt_on_db = true;

		_smp::message_pool::getInstance().push(new message("[player::checkBallEquiped][Error][WARNING] player[UID=" + std::to_string(m_pi.uid)
				+ "] nao esta com uma Ball equipada. Bug", CL_FILE_LOG_AND_CONSOLE));
	}

	return upt_on_db;
}

bool player::checkItemEquiped(UserEquip& _ue) {

	bool upt_on_db = false;
	uint32_t tmp_typeid = 0u;

	WarehouseItemEx *pWi = nullptr;

	std::map< uint32_t , uint32_t  > mp_count_same_item;
	std::map< uint32_t , uint32_t  >::iterator it;

	for (auto i = 0u; i < (sizeof(_ue.item_slot) / sizeof(_ue.item_slot[0])); ++i) {

		if (_ue.item_slot[i] != 0) {

			if (!sIff::getInstance().ItemEquipavel(_ue.item_slot[i])) {

				tmp_typeid = _ue.item_slot[i];

				_ue.item_slot[i] = 0;

				upt_on_db = true;

				_smp::message_pool::getInstance().push(new message("[player::checkItemEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
						+ "] Not Equipable Item[TYPEID=" + std::to_string(tmp_typeid) + ", SLOT=" + std::to_string(i)
						+ "], but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

			}else if ((pWi = m_pi.findWarehouseItemByTypeid(_ue.item_slot[i])) == nullptr) {

				_ue.item_slot[i] = 0;

				upt_on_db = true;

				_smp::message_pool::getInstance().push(new message("[player::checkItemEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
						+ "] Not Have Item[TYPEID=" + std::to_string(_ue.item_slot[i]) + ", SLOT=" + std::to_string(i)
						+ "], but it is equiped. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
			}else {

				if ((it = mp_count_same_item.find(pWi->_typeid)) != mp_count_same_item.end()) {

					if (std::find(active_item_cant_have_2_inveroty, LAST_ELEMENT_IN_ARRAY(active_item_cant_have_2_inveroty), pWi->_typeid) != LAST_ELEMENT_IN_ARRAY(active_item_cant_have_2_inveroty)) {

						tmp_typeid = _ue.item_slot[i];

						_ue.item_slot[i] = 0;

						upt_on_db = true;

						_smp::message_pool::getInstance().push(new message("[player::checkItemEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
								+ "] Nao pode equipar 2 Ex:[Corta com (Toma ou Safety)] Item[TYPEID=" + std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id)
								+ "] no inventory. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

					}else if (pWi->STDA_C_ITEM_QNTD < (int)(it->second + 1) ) {

						tmp_typeid = _ue.item_slot[i];

						_ue.item_slot[i] = 0;

						upt_on_db = true;

						_smp::message_pool::getInstance().push(new message("[player::checkItemEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
								+ "] Nao tem quantidade do Item[TYPEID=" + std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id)
								+ "] para equipar ele. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

					}else
						it->second++;

				}else {

					if (pWi->STDA_C_ITEM_QNTD < 1 ) {

						tmp_typeid = _ue.item_slot[i];

						_ue.item_slot[i] = 0;

						upt_on_db = true;

						_smp::message_pool::getInstance().push(new message("[player::checkItemEquiped][Error] player[UID=" + std::to_string(m_pi.uid)
								+ "] Nao tem quantidade do Item[TYPEID=" + std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id)
								+ "] para equipar ele. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

					}else

						mp_count_same_item.insert(std::make_pair(pWi->_typeid, 1));
				}
			}
		}
	}

	return upt_on_db;
}

void player::checkAllItemEquiped(UserEquip& _ue) {

	if (checkSkinEquiped(_ue))
		snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateSkinEquiped(m_pi.uid, _ue), player::SQLDBResponse, this);

	if (checkPosterEquiped(_ue))
		snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdatePosterEquiped(m_pi.uid, _ue), player::SQLDBResponse, this);

	if (checkCharacterEquiped(_ue))
		snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateCharacterEquiped(m_pi.uid, _ue.character_id), player::SQLDBResponse, this);

	if (checkCaddieEquiped(_ue))
		snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateCaddieEquiped(m_pi.uid, _ue.caddie_id), player::SQLDBResponse, this);

	if (checkMascotEquiped(_ue))
		snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateMascotEquiped(m_pi.uid, _ue.mascot_id), player::SQLDBResponse, this);

	if (checkItemEquiped(_ue))
		snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateItemSlot(m_pi.uid, (uint32_t*)_ue.item_slot), player::SQLDBResponse, this);

	if (checkClubSetEquiped(_ue))
		snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateClubsetEquiped(m_pi.uid, _ue.clubset_id), player::SQLDBResponse, this);

	if (checkBallEquiped(_ue))
		snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateBallEquiped(m_pi.uid, _ue.ball_typeid), player::SQLDBResponse, this);
}

void player::equipDefaultCharacter(UserEquip& _ue) {

	auto tmp_id = _ue.character_id;

	_ue.character_id = 0;
	m_pi.ei.char_info = nullptr;

	if (m_pi.mp_ce.size() > 0) {

		_smp::message_pool::getInstance().push(new message("[player::equipDefaultCharacter][Log][WARNING] player[UID=" + std::to_string(m_pi.uid)
				+ "] tentou verificar o Character[ID=" + std::to_string(tmp_id)
				+ "] para comecar o jogo, colocando o primeiro character do player. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

		m_pi.ei.char_info = &m_pi.mp_ce.begin()->second;
		_ue.character_id = m_pi.ei.char_info->id;

	}else {

		_smp::message_pool::getInstance().push(new message("[player::equipDefaultCharacter][Log][WARNING] player[UID=" + std::to_string(m_pi.uid)
				+ "] tentou verificar o Character[ID=" + std::to_string(tmp_id)
				+ "] para comecar o jogo, ele nao tem nenhum character. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

		BuyItem bi{ 0 };
		stItem item{ 0 };
		int32_t item_id = 0;

		bi.id = -1;
		bi._typeid = iff::CHARACTER << 26;
		bi.qntd = 1;

		item_manager::initItemFromBuyItem(m_pi, item, bi, false, 0, 0, 1 );

		if (item._typeid != 0) {

			if ((item_id = item_manager::addItem(item, *this, 2 , 0)) == item_manager::RetAddItem::T_ERROR)
				throw exception("[player::equipDefaultCharacter][Log][WARNING] player[UID=" + std::to_string(m_pi.uid)
						+ "] nao conseguiu adicionar o Character[TYPEID=" + std::to_string(item._typeid) + "] padrao para ele. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 2500, 2));

		}else
			throw exception("[player::equipDefaultCharacter][Log][WARNING][Error] player[UID=" + std::to_string(m_pi.uid)
					+ "] nao conseguiu inicializar o Character[TYPEID=" + std::to_string(bi._typeid) + "] padrao para ele. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 2500, 1));
	}
}

void player::equipDefaultClubSet(UserEquip& _ue) {

	auto tmp_id = _ue.clubset_id;

	_ue.clubset_id = 0;
	m_pi.ei.clubset = nullptr;
	m_pi.ei.csi.clear();

	_smp::message_pool::getInstance().push(new message("[player::equipDefaultClubSet][Error] player[UID=" + std::to_string(m_pi.uid)
			+ "] tentou verificar o Clubset[ID=" + std::to_string(tmp_id)
			+ "] equipado, mas ClubSet Not exists on IFF structure. Equipa o ClubSet padrao. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

	auto pWi = m_pi.findWarehouseItemByTypeid(AIR_KNIGHT_SET);

	if (pWi != nullptr) {

		_smp::message_pool::getInstance().push(new message("[player::equipDefaultClubSet][Log][WARNING] player[UID=" + std::to_string(m_pi.uid)
				+ "] tentou verificar o ClubSet[ID=" + std::to_string(tmp_id) + "], mas acabou o tempo do ClubSet[ID="
				+ std::to_string(tmp_id) + "], colocando o ClubSet Padrao\"CV1\" do player. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

		m_pi.ei.csi = { pWi->id, pWi->_typeid, pWi->c };

		IFF::ClubSet *cs = sIff::getInstance().findClubSet(pWi->_typeid);

		if (cs != nullptr)
			for (auto j = 0u; j < (sizeof(m_pi.ei.csi.enchant_c) / sizeof(short)); ++j)
				m_pi.ei.csi.enchant_c[j] = cs->slot[j] + pWi->clubset_workshop.c[j];

		m_pi.ei.clubset = pWi;
		_ue.clubset_id = pWi->id;

	}else {

		_smp::message_pool::getInstance().push(new message("[player::equipDefaultClubSet][Log][WARNING] player[UID=" + std::to_string(m_pi.uid)
				+ "] tentou verificar o ClubSet[ID=" + std::to_string(tmp_id) + "], mas acabou o tempo do ClubSet[ID="
				+ std::to_string(tmp_id) + "], ele nao tem o ClubSet Padrao\"CV1\". Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

		BuyItem bi{ 0 };
		stItem item{ 0 };
		int32_t item_id = 0;

		bi.id = -1;
		bi._typeid = AIR_KNIGHT_SET;
		bi.qntd = 1;

		item_manager::initItemFromBuyItem(m_pi, item, bi, false, 0, 0, 1 );

		if (item._typeid != 0) {

			if ((item_id = item_manager::addItem(item, *this, 2 , 0)) != item_manager::RetAddItem::T_ERROR) {

				pWi = m_pi.findWarehouseItemById(item_id);

				if (pWi != nullptr) {

					m_pi.ei.csi = { pWi->id, pWi->_typeid, pWi->c };

					IFF::ClubSet *cs = sIff::getInstance().findClubSet(pWi->_typeid);

					if (cs != nullptr)
						for (auto j = 0u; j < (sizeof(m_pi.ei.csi.enchant_c) / sizeof(short)); ++j)
							m_pi.ei.csi.enchant_c[j] = cs->slot[j] + pWi->clubset_workshop.c[j];

					m_pi.ei.clubset = pWi;
					m_pi.ue.clubset_id = pWi->id;

					snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateClubsetEquiped(m_pi.uid, item_id), player::SQLDBResponse, this);

				}else
					throw exception("[player::equipDefaultClubSet][Log][WARNING] player[UID=" + std::to_string(m_pi.uid)
							+ "] nao conseguiu achar o ClubSet\"CV1\"[ID=" + std::to_string(item.id) + "] padrao que acabou de adicionar para ele. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 2501, 3));

			}else
				throw exception("[player::equipDefaultClubSet][Log][WARNING] player[UID=" + std::to_string(m_pi.uid)
						+ "] nao conseguiu adicionar o ClubSet[TYPEID=" + std::to_string(item._typeid) + "] padrao para ele. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 2501, 2));

		}else
			throw exception("[player::equipDefaultClubSet][Log][WARNING] player[UID=" + std::to_string(m_pi.uid)
					+ "] nao conseguiu inicializar o ClubSet[TYPEID=" + std::to_string(bi._typeid) + "] padrao para ele. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 2501, 1));
	}
}

void player::equipDefaultBall(UserEquip& _ue) {

	try {

		if (m_pi.m_cap.stBit.premium_user) {

			equipDefaultBallPremiumUser(_ue);

			return;
		}

	}catch (exception& e){

			_smp::message_pool::getInstance().push(new message("[player::equipDefaultBall][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	auto tmp_typeid = _ue.ball_typeid;

	_ue.ball_typeid = DEFAULT_COMET_TYPEID;
	m_pi.ei.comet = nullptr;

	auto pWi = m_pi.findWarehouseItemByTypeid(DEFAULT_COMET_TYPEID);

	if (pWi != nullptr) {

		_smp::message_pool::getInstance().push(new message("[player::equipDefaultBall][Log][WARNING] player[UID=" + std::to_string(m_pi.uid)
				+ "] tentou verificar a Ball[TYPEID=" + std::to_string(tmp_typeid)
				+ "] para comecar o jogo, colocando a Ball Padrao do player. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

		m_pi.ei.comet = pWi;
		_ue.ball_typeid = pWi->_typeid;

	}else {

		_smp::message_pool::getInstance().push(new message("[player::equipDefaultBall][Log][WARNING] player[UID=" + std::to_string(m_pi.uid)
				+ "] tentou trocar a Ball[TYPEID=" + std::to_string(tmp_typeid)
				+ "] para comecar o jogo, ele nao tem a Ball Padrao. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

		BuyItem bi{ 0 };
		stItem item{ 0 };
		int32_t item_id = 0;

		bi.id = -1;
		bi._typeid = DEFAULT_COMET_TYPEID;
		bi.qntd = 1;

		item_manager::initItemFromBuyItem(m_pi, item, bi, false, 0);

		if (item._typeid != 0) {

			if ((item_id = item_manager::addItem(item, *this, 2 , 0)) != item_manager::RetAddItem::T_ERROR) {

				pWi = m_pi.findWarehouseItemById(item_id);

				if (pWi != nullptr) {

					m_pi.ei.comet = pWi;
					m_pi.ue.ball_typeid = pWi->_typeid;

					snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateBallEquiped(m_pi.uid, item_id), player::SQLDBResponse, this);

				}else
					throw exception("[player::equipDefaultBall][Log][WARNING] player[UID=" + std::to_string(m_pi.uid)
							+ "] nao conseguiu achar a Ball[ID=" + std::to_string(item.id) + "] padrao que acabou de adicionar para ele. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 2502, 3));

			}else
				throw exception("[player::equipDefaultBall][Log][WARNING] player[UID=" + std::to_string(m_pi.uid)
						+ "] nao conseguiu adicionar a Ball[TYPEID=" + std::to_string(item._typeid) + "] padrao para ele. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 2502, 2));

		}else
			throw exception("[player::equipDefaultBall][Log][WARNING] player[UID=" + std::to_string(m_pi.uid)
					+ "] nao conseguiu inicializar a Ball[TYPEID=" + std::to_string(bi._typeid) + "] padrao para ele. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER, 2502, 1));

	}
}

void player::equipDefaultBallPremiumUser(UserEquip& _ue) {

	auto tmp_typeid = _ue.ball_typeid;

	_ue.ball_typeid = sPremiumSystem::getInstance().getPremiumBallByTicket(m_pi.pt._typeid);
	m_pi.ei.comet = nullptr;

	auto pWi = m_pi.findWarehouseItemByTypeid(_ue.ball_typeid);

	if (pWi != nullptr) {

		_smp::message_pool::getInstance().push(new message("[player::equipDefaultBallPremiumUser][Log][WARNING] player[UID=" + std::to_string(m_pi.uid)
				+ "] tentou verificar a Ball[TYPEID=" + std::to_string(tmp_typeid)
				+ "] para comecar o jogo, colocando a Ball Premium User Padrao do player. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

		m_pi.ei.comet = pWi;
		_ue.ball_typeid = pWi->_typeid;

	}else {

		_smp::message_pool::getInstance().push(new message("[player::equipDefaultBallPremiumUser][Log][WARNING] player[UID=" + std::to_string(m_pi.uid)
				+ "] tentou trocar a Ball[TYPEID=" + std::to_string(tmp_typeid)
				+ "] para comecar o jogo, ele nao tem a Ball Premium User Padrao. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

		stItem item  = sPremiumSystem::getInstance().addPremiumBall(*this);

		if (item._typeid != 0u)

			snmdb::NormalManagerDB::getInstance().add(0, new CmdUpdateBallEquiped(m_pi.uid, item._typeid), player::SQLDBResponse, this);
		else
			_smp::message_pool::getInstance().push(new message("[player::equipDefaultBallPremiumUser][ERROR][WARNING] player[UID=" + std::to_string(m_pi.uid)
					+ "] tentou trocar a Ball[TYPEID=" + std::to_string(tmp_typeid)
					+ "] para comecar o jogo, mas nao conseguiu adicionar a ball premium. Hacker ou Bug.", CL_FILE_LOG_AND_CONSOLE));
	}
}

std::vector< CharacterInfo* > player::isAuxPartEquiped(uint32_t _typeid) {

	std::vector< CharacterInfo* > v_ci;

	std::for_each(m_pi.mp_ce.begin(), m_pi.mp_ce.end(), [&](auto& _el) {

		if (_el.second.isAuxPartEquiped(_typeid))
			v_ci.push_back(&_el.second);
	});

	return v_ci;
}

CharacterInfo* player::isPartEquiped(uint32_t _typeid) {

	auto it = std::find_if(m_pi.mp_ce.begin(), m_pi.mp_ce.end(), [&](auto& _el) {
		return _el.second.isPartEquiped(_typeid);
	});

	return (it != m_pi.mp_ce.end() ? &it->second : nullptr);
}

void player::SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg) {

	if (_arg == nullptr) {
		_smp::message_pool::getInstance().push(new message("[player::SQLDBResponse][WARNING] _arg is nullptr na msg_id = " + std::to_string(_msg_id), CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	if (_pangya_db.getException().getCodeError() != 0) {
		_smp::message_pool::getInstance().push(new message("[player:SQLDBResponse][Error] " + _pangya_db.getException().getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	auto _session = reinterpret_cast< player* >(_arg);

	switch (_msg_id) {
	case 1:
	{
		break;
	}
	case 2:
	{
		break;
	}
	case 3:
	{
		auto cmd_icpli = reinterpret_cast< CmdInsertCPLogItem* >(&_pangya_db);

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[player::SQLDBResponse][Log] Inseriu CPLogItem[LOD_ID=" + std::to_string(cmd_icpli->getLogId())
				+ ", ITEM_TYPEID=" + std::to_string(cmd_icpli->getItem()._typeid) + ", ITEM_QNTD=" + std::to_string(cmd_icpli->getItem().qntd)
				+ ", ITEM_PRICE=" + std::to_string(cmd_icpli->getItem().price) + "] do Player[UID=" + std::to_string(cmd_icpli->getUID()) + "] com sucesso.", CL_FILE_LOG_AND_CONSOLE));
#else
		_smp::message_pool::getInstance().push(new message("[player::SQLDBResponse][Log] Inseriu CPLogItem[LOD_ID=" + std::to_string(cmd_icpli->getLogId())
				+ ", ITEM_TYPEID=" + std::to_string(cmd_icpli->getItem()._typeid) + ", ITEM_QNTD=" + std::to_string(cmd_icpli->getItem().qntd)
				+ ", ITEM_PRICE=" + std::to_string(cmd_icpli->getItem().price) + "] do Player[UID=" + std::to_string(cmd_icpli->getUID()) + "] com sucesso.", CL_ONLY_FILE_LOG));
#endif

		break;
	}
	case 0:
	default:
		break;
	}
};
