
#if defined(_WIN32)
#pragma pack(1)
#endif

#include "cmd_update_guild_ranking.hpp"

using namespace stdA;

CmdUpdateGuildRanking::CmdUpdateGuildRanking(bool _waiter) : pangya_db(_waiter) {
}

CmdUpdateGuildRanking::~CmdUpdateGuildRanking() {
}

void CmdUpdateGuildRanking::lineResult(result_set::ctx_res*  , uint32_t  ) {

	return;
}

response* CmdUpdateGuildRanking::prepareConsulta(database& _db) {

	auto r = procedure(_db, m_szConsulta, "");

	checkResponse(r, "Nao conseguiu atualizar Guild Ranking.");

	return r;
}
