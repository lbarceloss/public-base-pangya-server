
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#elif defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#endif

#include "cmd_rank_registry_character_info.hpp"

using namespace stdA;

CmdRankRegistryCharacterInfo::CmdRankRegistryCharacterInfo(bool _waiter) : pangya_db(_waiter) {
}

CmdRankRegistryCharacterInfo::~CmdRankRegistryCharacterInfo() {

	if (!m_entry.empty())
		m_entry.clear();
}

void CmdRankRegistryCharacterInfo::lineResult(result_set::ctx_res* _result, uint32_t  ) {

	checkColumnNumber(85, (uint32_t)_result->cols);

	RankCharacter rc;

	CharacterInfo ce{ 0 };
	auto i = 0;

	rc.setUID(IFNULL(atoi, _result->data[0]));

	if (is_valid_c_string(_result->data[1]))
		rc.setId(_result->data[1]);

	if (is_valid_c_string(_result->data[2]))
		rc.setNickname(_result->data[2]);

	rc.setLevel((unsigned short)IFNULL(atoi, _result->data[3]));

	ce.id = IFNULL(atoi, _result->data[4]);
	ce._typeid = IFNULL(atoi, _result->data[5]);
	for (i = 0; i < 24; i++)
		ce.parts_id[i] = IFNULL(atoi, _result->data[6 + i]);
	for (i = 0; i < 24; i++)
		ce.parts_typeid[i] = IFNULL(atoi, _result->data[30 + i]);
	ce.default_hair = (unsigned char)IFNULL(atoi, _result->data[54]);
	ce.default_shirts = (unsigned char)IFNULL(atoi, _result->data[55]);
	ce.gift_flag = (unsigned char)IFNULL(atoi, _result->data[56]);
	for (i = 0; i < 5; i++)
		ce.pcl[i] = (unsigned char)IFNULL(atoi, _result->data[57 + i]);
	ce.purchase = (unsigned char)IFNULL(atoi, _result->data[62]);
	for (i = 0; i < 5; i++)
		ce.auxparts[i] = IFNULL(atoi, _result->data[63 + i]);
	for (i = 0; i < 4; i++)
		ce.cut_in[i] = IFNULL(atoi, _result->data[68 + i]);
	ce.mastery = IFNULL(atoi, _result->data[72]);
	for (i = 0; i < 4; i++)
		ce.card_character[i] = IFNULL(atoi, _result->data[73 + i]);
	for (i = 0; i < 4; i++)
		ce.card_caddie[i] = IFNULL(atoi, _result->data[77 + i]);
	for (i = 0; i < 4; i++)
		ce.card_NPC[i] = IFNULL(atoi, _result->data[81 + i]);

	rc.setCharacterInfo(ce);

	auto it_map = m_entry.find(rc.getUID());

	if (it_map != m_entry.end()) {

		_smp::message_pool::getInstance().push(new message("[CmdReankRegistryCharacterInfo::lineResult][WARNING] Player[UID=" + std::to_string(rc.getUID())
				+ "] CHARACTER_ANT[TYPEID=" + std::to_string(it_map->second.getCharacterInfo()._typeid)
				+ ", ID=" + std::to_string(it_map->second.getCharacterInfo().id) + "] CHARACTER_REPLACE[TYPEID=" + std::to_string(ce._typeid) + ", ID="
				+ std::to_string(ce.id) + "] tem mais de um character equipado nos registro do rank no banco de dados. Trocando o character antigo pelo novo.", CL_FILE_LOG_AND_CONSOLE));

		it_map->second = rc;

	}else {

		auto ret = m_entry.insert(std::make_pair(rc.getUID(), rc));

		if (!ret.second) {

			if (ret.first != m_entry.end() && ret.first->first == rc.getUID()) {

				if (ret.first->second.getCharacterInfo()._typeid != ce._typeid) {

					try {

						auto rc_ant = m_entry.at(rc.getUID());

						m_entry.at(rc.getUID()) = rc;

						_smp::message_pool::getInstance().push(new message("[CmdRankRegistryCharacterInfo::lineResult][Log] Player[UID=" + std::to_string(rc.getUID())
								+ "] Atualizou o character registro do character rank map. CHARACTER_ANT[TYPEID=" + std::to_string(rc_ant.getCharacterInfo()._typeid)
								+ ", ID=" + std::to_string(rc_ant.getCharacterInfo().id) + "], CHARACTER_NEW[TYPEID=" + std::to_string(ce._typeid)
								+ ", ID=" + std::to_string(ce.id) + "].", CL_FILE_LOG_AND_CONSOLE));

					}catch (std::out_of_range& e) {
						UNREFERENCED_PARAMETER(e);

						_smp::message_pool::getInstance().push(new message("[CmdRankRegistryCharacterInfo::lineResult][Error][WARNING] nao conseguiu atualizar o character registro do player[UID="
								+ std::to_string(rc.getUID()) + "] no character rank map. CHARACTER_ANT[TYPEID=" + std::to_string(ret.first->second.getCharacterInfo()._typeid)
								+ ", ID=" + std::to_string(ret.first->second.getCharacterInfo().id) + "], CHARACTER_NEW[TYPEID="
								+ std::to_string(ce._typeid) + ", ID=" + std::to_string(ce.id) + "].", CL_FILE_LOG_AND_CONSOLE));
					}

				}else
					_smp::message_pool::getInstance().push(new message("[CmdRankRegistryCharacterInfo::lineResult][Error] nao conseguiu adicionar o novo chararacter registro do player[UID="
							+ std::to_string(rc.getUID()) + "] no character rank map, por que ele ja tem o mesmo character[TYPEID="
							+ std::to_string(ret.first->second.getCharacterInfo()._typeid) + ", ID=" + std::to_string(ret.first->second.getCharacterInfo().id) + "] no map.", CL_FILE_LOG_AND_CONSOLE));

			}else
				_smp::message_pool::getInstance().push(new message("[CmdRankRegistryCharacterInfo::lineResult][Error] nao conseguiu adicionar o novo character registro do player[UID="
						+ std::to_string(rc.getUID()) + "] no character rank map.", CL_FILE_LOG_AND_CONSOLE));
		}
	}
}

response* CmdRankRegistryCharacterInfo::prepareConsulta(database& _db) {

	if (!m_entry.empty())
		m_entry.clear();

	auto r = procedure(_db, m_szConsulta, "");

	checkResponse(r, "Nao conseguiu pegar os registro de characters do Rank");

	return r;
}

RankCharacterEntry& CmdRankRegistryCharacterInfo::getInfo() {
	return m_entry;
}
