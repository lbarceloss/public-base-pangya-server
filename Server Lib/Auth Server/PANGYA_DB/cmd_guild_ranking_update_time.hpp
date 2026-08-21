
#pragma once
#ifndef _STDA_CMD_GUILD_RANKING_UPDATE_TIME_HPP
#define _STDA_CMD_GUILD_RANKING_UPDATE_TIME_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"

namespace stdA {
	class CmdGuildRankingUpdateTime : public pangya_db {
		public:
			CmdGuildRankingUpdateTime(bool _waiter = false);
			virtual ~CmdGuildRankingUpdateTime();

			SYSTEMTIME& getTime();

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			std::string _getName() override { return "CmdGuildRankingUpdateTime"; };
			std::wstring _wgetName() override { return L"CmdGuildRankingUpdateTime"; };

		private:
			SYSTEMTIME m_si;

			const char* m_szConsulta = "pangya.ProcGetGuildRankingUpdateTime";
	};
}

#endif
