
#if defined(_WIN32)
#pragma pack(1)
#endif

#include "cmd_delete_daily_quest.hpp"

using namespace stdA;

CmdDeleteDailyQuest::CmdDeleteDailyQuest(bool _waiter)
	: pangya_db(_waiter), m_uid(0), v_rdqu() {
}

CmdDeleteDailyQuest::CmdDeleteDailyQuest(uint32_t _uid, std::vector< RemoveDailyQuestUser >& _rdqu, bool _waiter)
	: pangya_db(_waiter), m_uid(_uid), v_rdqu(_rdqu) {
}

CmdDeleteDailyQuest::~CmdDeleteDailyQuest() {
}

void CmdDeleteDailyQuest::lineResult(result_set::ctx_res*  , uint32_t  ) {

	return;
}

response* CmdDeleteDailyQuest::prepareConsulta(database& _db) {

	std::string ids = "";

	for (auto i = 0u; i < v_rdqu.size(); ++i) {
		if (i == 0)
			ids = std::to_string(v_rdqu[i].id);
		else
			ids += ", " + std::to_string(v_rdqu[i].id);
	}

	auto r = _delete(_db, m_szConsulta[0] + std::to_string(m_uid) + m_szConsulta[1] + ids + m_szConsulta[2] + std::to_string(m_uid) + m_szConsulta[3] + ids + m_szConsulta[4]);

	checkResponse(r, "nao conseguiu deletar a daily[ID(s)=" + ids + "] quest do player: " + std::to_string(m_uid));

	return r;
}

uint32_t CmdDeleteDailyQuest::getUID() {
	return m_uid;
}

void CmdDeleteDailyQuest::setUID(uint32_t _uid) {
	m_uid = _uid;
}

std::vector< RemoveDailyQuestUser >& CmdDeleteDailyQuest::getDeleteDailyQuest() {
	return v_rdqu;
}

void CmdDeleteDailyQuest::setDeleteDailyQuest(std::vector< RemoveDailyQuestUser >& _rdqu) {
	v_rdqu = _rdqu;
}
