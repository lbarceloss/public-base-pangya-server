
#pragma once
#ifndef _STDA_CMD_UPDATE_RANK_REGISTRY_HPP
#define _STDA_CMD_UPDATE_RANK_REGISTRY_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"

#include <string>

namespace stdA {
	class CmdUpdateRankRegistry : public pangya_db {
		public:
			explicit CmdUpdateRankRegistry(bool _waiter = false);
			virtual ~CmdUpdateRankRegistry();

			uint32_t& getRetState();
			std::string& getDate();

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			std::string _getName() override { return "CmdUpdateRankRegistry"; };
			std::wstring _wgetName() override { return L"CmdUpateRankRegistry"; };

		private:
			uint32_t m_ret_state;
			std::string m_date;

			const char* m_szConsulta = "pangya.GeraRankAll";
	};
}

#endif
