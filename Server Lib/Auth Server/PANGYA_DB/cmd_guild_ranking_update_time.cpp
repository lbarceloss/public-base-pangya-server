
#if defined(_WIN32)
#pragma pack(1)
#endif

#include "cmd_guild_ranking_update_time.hpp"

#include "../../Projeto IOCP/UTIL/util_time.h"

using namespace stdA;

CmdGuildRankingUpdateTime::CmdGuildRankingUpdateTime(bool _waiter) : pangya_db(_waiter), m_si{0} {
}

CmdGuildRankingUpdateTime::~CmdGuildRankingUpdateTime() {
}

void CmdGuildRankingUpdateTime::lineResult(result_set::ctx_res* _result, uint32_t  ) {

	checkColumnNumber(1, (uint32_t)_result->cols);

	if (_result->data[0] != nullptr)
		_translateDate(_result->data[0], &m_si);
}

response* CmdGuildRankingUpdateTime::prepareConsulta(database& _db) {

	auto r = procedure(_db, m_szConsulta, "");

	checkResponse(r, "Nao conseguiu pegar a date em que o Guild Ranking foi atualizado.");

	return r;
}

SYSTEMTIME& CmdGuildRankingUpdateTime::getTime() {
	return m_si;
}
