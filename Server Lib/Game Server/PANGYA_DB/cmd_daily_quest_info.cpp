
#if defined(_WIN32)
#pragma pack(1)
#endif

#include "cmd_daily_quest_info.hpp"
#include "../../Projeto IOCP/UTIL/util_time.h"

using namespace stdA;

CmdDailyQuestInfo::CmdDailyQuestInfo(bool _waiter) : pangya_db(_waiter), m_dqi{0} {
}

CmdDailyQuestInfo::~CmdDailyQuestInfo() {
}

void CmdDailyQuestInfo::lineResult(result_set::ctx_res* _result, uint32_t  ) {

	checkColumnNumber(4, (uint32_t)_result->cols);

	for (auto i = 0u; i < 3; ++i)
		m_dqi._typeid[i] = IFNULL(atoi, _result->data[0 + i]);

	if (_result->data[3] != nullptr)
		_translateDate(_result->data[3], &m_dqi.date);
}

response* CmdDailyQuestInfo::prepareConsulta(database& _db) {

	m_dqi.clear();

	auto r = consulta(_db, m_szConsulta);

	checkResponse(r, "nao conseguiu pegar o Daily Quest Info");

	return r;
}

DailyQuestInfo& CmdDailyQuestInfo::getInfo() {
	return m_dqi;
}

void CmdDailyQuestInfo::setInfo(DailyQuestInfo& _dqi) {
	m_dqi = _dqi;
}
