
#pragma once
#ifndef _STDA_CMD_SCRATCHY_ITEM_HPP
#define _STDA_CMD_SCRATCHY_ITEM_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../TYPE/scratch_card_type.hpp"

#include <vector>

namespace stdA {
	class CmdScratchyItem : public pangya_db {
		public:
			explicit CmdScratchyItem(bool _waiter = false);
			virtual ~CmdScratchyItem();

			std::vector< ctx_scratch_card_item >& getInfo();

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			std::string _getName() override { return "CmdScratchyItem"; };
			std::wstring _wgetName() override { return L"CmdScratchyItem"; };

		private:
			std::vector< ctx_scratch_card_item > v_item;

			const char* m_szConsulta = "SELECT TypeID, Numero, Quantidade, Probabilidade, Tipo, Active FROM pangya.scratchy_item WHERE Active = 1";
	};
}

#endif
