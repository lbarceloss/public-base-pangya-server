
#if defined(_WIN32)
#pragma pack(1)
#endif

#include "cmd_scratchy_item.hpp"

using namespace stdA;

CmdScratchyItem::CmdScratchyItem(bool _waiter) : pangya_db(_waiter) {
}

CmdScratchyItem::~CmdScratchyItem() {
}

void CmdScratchyItem::lineResult(result_set::ctx_res* _result, uint32_t  ) {

	checkColumnNumber(6, (uint32_t)_result->cols);

	ctx_scratch_card_item ctx_i{ 0 };

	ctx_i._typeid       = (uint32_t)IFNULL(atoi, _result->data[0]);
	ctx_i.numero        = IFNULL(atoi, _result->data[1]);
	ctx_i.qntd          = (uint32_t)IFNULL(atoi, _result->data[2]);
	ctx_i.probabilidade = (uint32_t)IFNULL(atoi, _result->data[3]);
	ctx_i.tipo          = IFNULL(atoi, _result->data[4]);
	ctx_i.active        = (unsigned char)IFNULL(atoi, _result->data[5]);

	v_item.push_back(ctx_i);
}

response* CmdScratchyItem::prepareConsulta(database& _db) {

	if (!v_item.empty())
		v_item.clear();

	auto r = consulta(_db, m_szConsulta);

	checkResponse(r, "nao conseguiu pegar os Itens da raspadinha (scratchy_item)");

	return r;
}

std::vector< ctx_scratch_card_item >& CmdScratchyItem::getInfo() {
	return v_item;
}
