
#pragma once
#ifndef _STDA_CMD_CARD_PACK_HPP
#define _STDA_CMD_CARD_PACK_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../TYPE/card_type.hpp"

#include <map>

namespace stdA {
	class CmdCardPack : public pangya_db {
		public:
			explicit CmdCardPack(bool _waiter = false);
			virtual ~CmdCardPack();

			std::map< uint32_t, CardPack >& getCardPack();

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdCardPack"; };
			virtual std::wstring _wgetName() override { return L"CmdCardPack"; };

	private:
		std::map< uint32_t, CardPack > m_card_pack;

		const char* m_szConsulta = "SELECT B.typeid as CardPack, B.quantidade as qntd, B.tipo as Vol, B.rate_N, B.rate_R, B.rate_SR, B.rate_SC, \
				A.typeid, A.probabilidade as prob, A.tipo FROM pangya.pangya_new_cards A INNER JOIN pangya.pangya_new_card_pack B ON A.pack = B.tipo";
	};
}

#endif
