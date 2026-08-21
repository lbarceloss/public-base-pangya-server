
#pragma once
#ifndef _STDA_CMD_INSERT_SPINNING_CUBE_SUPER_RARE_WIN_BROADCAST_HPP
#define _STDA_CMD_INSERT_SPINNING_CUBE_SUPER_RARE_WIN_BROADCAST_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include <string>

namespace stdA {
	class CmdInsertSpinningCubeSuperRareWinBroadcast : public pangya_db {
		public:
			explicit CmdInsertSpinningCubeSuperRareWinBroadcast(bool _waiter = false);
			CmdInsertSpinningCubeSuperRareWinBroadcast(std::string& _message, unsigned char _opt, bool _waiter = false);
			virtual ~CmdInsertSpinningCubeSuperRareWinBroadcast();

			std::string& getMessage();
			void setMessage(std::string& _message);

			unsigned char getOpt();
			void setOpt(unsigned char _opt);

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdInsertSpinningCubeSuperRareWinBroadcast"; };
			virtual std::wstring _wgetName() override { return L"CmdInsertSpinningCubeSuperRareWinBroadcast"; };

		private:
			std::string m_message;
			unsigned char m_opt;

			const char *m_szConsulta = "pangya.ProcInsertSpinningCubeSuperRareWinBroadCast";
	};
}

#endif
