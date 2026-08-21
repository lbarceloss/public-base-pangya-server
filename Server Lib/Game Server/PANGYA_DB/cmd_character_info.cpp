
#if defined(_WIN32)
#pragma pack(1)
#endif

#include "cmd_character_info.hpp"
#include "../../Projeto IOCP/UTIL/exception.h"
#include "../../Projeto IOCP/TYPE/stda_error.h"
#include "../../Projeto IOCP/UTIL/message_pool.h"

#include <algorithm>

using namespace stdA;

CmdCharacterInfo::CmdCharacterInfo(bool _waiter)
		: pangya_db(_waiter), m_uid(0), m_type(ALL), m_char_id(INVALID_ID), v_ci() {
}

CmdCharacterInfo::CmdCharacterInfo(uint32_t _uid, TYPE _type, int32_t _char_id, bool _waiter)
		: pangya_db(_waiter), m_uid(_uid), m_type(_type), m_char_id(_char_id), v_ci() {
}

CmdCharacterInfo::~CmdCharacterInfo() {
}

void CmdCharacterInfo::lineResult(result_set::ctx_res* _result, uint32_t  ) {

	checkColumnNumber(81, (uint32_t)_result->cols);

	CharacterInfo ce{ 0 };
	auto i = 0;

	ce.id = IFNULL(atoi, _result->data[0]);
	ce._typeid = IFNULL(atoi, _result->data[1]);
	for (i = 0; i < 24; i++)
		ce.parts_id[i] = IFNULL(atoi, _result->data[2 + i]);
	for (i = 0; i < 24; i++)
		ce.parts_typeid[i] = IFNULL(atoi, _result->data[26 + i]);
	ce.default_hair = (unsigned char)IFNULL(atoi, _result->data[50]);
	ce.default_shirts = (unsigned char)IFNULL(atoi, _result->data[51]);
	ce.gift_flag = (unsigned char)IFNULL(atoi, _result->data[52]);
	for (i = 0; i < 5; i++)
		ce.pcl[i] = (unsigned char)IFNULL(atoi, _result->data[53 + i]);
	ce.purchase = (unsigned char)IFNULL(atoi, _result->data[58]);
	for (i = 0; i < 5; i++)
		ce.auxparts[i] = IFNULL(atoi, _result->data[59 + i]);
	for (i = 0; i < 4; i++)
		ce.cut_in[i] = IFNULL(atoi, _result->data[64 + i]);
	ce.mastery = IFNULL(atoi, _result->data[68]);
	for (i = 0; i < 4; i++)
		ce.card_character[i] = IFNULL(atoi, _result->data[69 + i]);
	for (i = 0; i < 4; i++)
		ce.card_caddie[i] = IFNULL(atoi, _result->data[73 + i]);
	for (i = 0; i < 4; i++)
		ce.card_NPC[i] = IFNULL(atoi, _result->data[77 + i]);

	auto it = v_ci.find(ce.id);

	if (it == v_ci.end() || (v_ci.count(ce.id) == 1 && it->second._typeid != ce._typeid))
		v_ci.insert(std::make_pair(ce.id, ce));
	else if (v_ci.count(ce.id) > 1) {

		auto er = v_ci.equal_range(ce.id);

		it = std::find_if(er.first, er.second , [&](auto& _el) {
			return _el.second._typeid == ce._typeid;
		});

		if (it == er.second ) {

			v_ci.insert(std::make_pair(ce.id, ce));

			_smp::message_pool::getInstance().push(new message("[CmdCharacterInfo::lineResult][WARNING] player[UID=" + std::to_string(m_uid) + "] adicionou Character[TYPEID="
					+ std::to_string(ce._typeid) + ", ID=" + std::to_string(ce.id) + "], com mesmo id e typeid diferente de outro Character que tem no multimap", CL_FILE_LOG_AND_CONSOLE));
		}else

			_smp::message_pool::getInstance().push(new message("[CmdCharacterInfo::lineResult][WARNING] player[UID=" + std::to_string(m_uid) + "] tentou adicionar no multimap um Character[TYPEID="
					+ std::to_string(it->second._typeid) + ", ID=" + std::to_string(it->second.id) + "] com o mesmo ID e TYPEID, DUPLICATA", CL_FILE_LOG_AND_CONSOLE));

	}else
		_smp::message_pool::getInstance().push(new message("[CmdCharacterInfo::lineResult][WARNING] player[UID=" + std::to_string(m_uid) + "] tentou adicionar no multimap um Character[TYPEID="
				+ std::to_string(it->second._typeid) + ", ID=" + std::to_string(it->second.id) + "] com o mesmo ID e TYPEID, DUPLICATA", CL_FILE_LOG_AND_CONSOLE));
}

response* CmdCharacterInfo::prepareConsulta(database& _db) {

	v_ci.clear();

	auto r = procedure(_db, (m_type == ALL) ? m_szConsulta[0] : m_szConsulta[1], std::to_string(m_uid) + ((m_type == ONE) ? ", " + std::to_string(m_char_id) : std::string()));

	checkResponse(r, "nao conseguiu pegar o(s) character(s) do player: " + std::to_string(m_uid) + "\t charID: " + std::to_string(m_char_id));

	return r;
}

std::multimap< int32_t , CharacterInfo >& stdA::CmdCharacterInfo::getAllInfo() {
	return v_ci;
}

CharacterInfo& CmdCharacterInfo::getInfo() {
	return v_ci.begin()->second;
}

uint32_t CmdCharacterInfo::getUID() {
	return m_uid;
}

void CmdCharacterInfo::setUID(uint32_t _uid) {
	m_uid = _uid;
}

int32_t CmdCharacterInfo::getCharID() {
	return m_char_id;
}

void CmdCharacterInfo::setCharID(int32_t _char_id) {
	m_char_id = _char_id;
}

CmdCharacterInfo::TYPE CmdCharacterInfo::getType() {
	return m_type;
}

void CmdCharacterInfo::setType(TYPE _type) {
	m_type = _type;
}
