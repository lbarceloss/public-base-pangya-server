
#pragma once
#ifndef _STDA_CMD_FIND_UCC_HPP
#define _STDA_CMD_FIND_UCC_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../TYPE/pangya_game_st.h"

namespace stdA {
	class CmdFindUCC : public pangya_db {
		public:
			explicit CmdFindUCC(bool _waiter = false);
			CmdFindUCC(int32_t _id, bool _waiter = false);
			virtual ~CmdFindUCC();

			int32_t getId();
			void setId(int32_t _id);

			WarehouseItemEx& getInfo();

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdFindUCC"; };
			virtual std::wstring _wgetName() override { return L"CmdFindUCC"; };

		private:
			int32_t m_id;
			WarehouseItemEx m_wi;

			const char* m_szConsulta = "pangya.ProcFindUCC";
	};
}

#endif
