
#pragma once
#ifndef _STDA_CMD_LIST_IP_BAN_HPP
#define _STDA_CMD_LIST_IP_BAN_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../TYPE/pangya_st.h"
#include <vector>

namespace stdA {
	class CmdListIPBan : public pangya_db {
		public:
			CmdListIPBan(bool _waiter = false);
			~CmdListIPBan();

			std::vector< IPBan >& getListIPBan();

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdListIPBan"; };
			virtual std::wstring _wgetName() override { return L"CmdListIPBan"; };

		private:
			std::vector< IPBan > v_ip_ban;

			const char* m_szConsulta = "SELECT ip, mask FROM pangya.pangya_ip_table";
	};
}

#endif
