
#if defined(_WIN32)
#pragma pack(1)
#endif

#include "cmd_trofel_info.hpp"

using namespace stdA;

CmdTrofelInfo::CmdTrofelInfo(bool _waiter) : pangya_db(_waiter), m_uid(0), m_season(CURRENT), m_ti{0} {
}

CmdTrofelInfo::CmdTrofelInfo(uint32_t _uid, TYPE_SEASON _season, bool _waiter) : pangya_db(_waiter), m_uid(_uid), m_season(_season), m_ti{0} {
}

CmdTrofelInfo::~CmdTrofelInfo() {
}

void CmdTrofelInfo::lineResult(result_set::ctx_res* _result, uint32_t  ) {

	checkColumnNumber(39, (uint32_t)_result->cols);

	auto i = 0, j = 0;

	for (i = 0; i < 6; ++i)
		for (j = 0; j < 3; ++j)
			m_ti.ama_6_a_1[i][j] = (short)IFNULL(atoi, _result->data[(i * 3) + j]);

	for (i = 0; i < 7; ++i)
		for (j = 0; j < 3; ++j)
			m_ti.pro_1_a_7[i][j] = (short)IFNULL(atoi, _result->data[18 + (i * 3) + j]);

}

response* CmdTrofelInfo::prepareConsulta(database& _db) {

	m_ti.clear();

	auto r = procedure(_db, m_szConsulta, std::to_string(m_uid) + ", " + std::to_string(m_season));

	checkResponse(r, "nao conseguiu recuperar o trofel info do player: " + std::to_string(m_uid));

	return r;
}

TrofelInfo& CmdTrofelInfo::getInfo() {
	return m_ti;
}

void CmdTrofelInfo::setInfo(TrofelInfo& _ti) {
	m_ti = _ti;
}

uint32_t CmdTrofelInfo::getUID() {
	return m_uid;
}

void CmdTrofelInfo::setUID(uint32_t _uid) {
	m_uid = _uid;
}

CmdTrofelInfo::TYPE_SEASON CmdTrofelInfo::getSeason() {
	return m_season;
}

void CmdTrofelInfo::setSeason(TYPE_SEASON _season) {
	m_season = _season;
}
