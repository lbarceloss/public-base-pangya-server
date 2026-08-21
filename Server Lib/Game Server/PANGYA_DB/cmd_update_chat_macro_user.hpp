
#pragma once
#ifndef _STDA_CMD_UPDATE_CHAT_MACRO_USER_HPP
#define _STDA_CMD_UPDATE_CHAT_MACRO_USER_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../../Projeto IOCP/TYPE/pangya_st.h"

namespace stdA {
	class CmdUpdateChatMacroUser : public pangya_db {
		public:
			explicit CmdUpdateChatMacroUser(bool _waiter = false);
			CmdUpdateChatMacroUser(uint32_t _uid, chat_macro_user& _cmu, bool _waiter = false);
			virtual ~CmdUpdateChatMacroUser();

			uint32_t getUID();
			void setUID(uint32_t _uid);

			chat_macro_user& getInfo();
			void setInfo(chat_macro_user& _cmu);

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdUpdateChatMacroUser"; };
			virtual std::wstring _wgetName() override { return L"CmdUpdateChatMacroUser"; };

		private:
			uint32_t m_uid;
			chat_macro_user m_cmu;

			const char* m_szConsulta = "pangya.ProcUpdateChatMacroUser";
	};
}

#endif
