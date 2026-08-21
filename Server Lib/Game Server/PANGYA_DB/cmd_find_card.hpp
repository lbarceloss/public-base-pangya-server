
#pragma once
#ifndef _STDA_CMD_FIND_CARD_HPP
#define _STDA_CMD_FIND_CARD_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../TYPE/pangya_game_st.h"

namespace stdA {
	class CmdFindCard : public pangya_db {
		public:
			explicit CmdFindCard(bool _waiter = false);
			CmdFindCard(uint32_t _uid, uint32_t _typeid, bool _waiter = false);
			virtual ~CmdFindCard();

			uint32_t getUID();
			void setUID(uint32_t _uid);

			uint32_t getTypeid();
			void setTypeid(uint32_t _typeid);

			bool hasFound();

			CardInfo& getInfo();

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdFindCard"; };
			virtual std::wstring _wgetName() override { return L"CmdFindCard"; };

		private:
			uint32_t m_uid;
			uint32_t m_typeid;
			CardInfo m_ci;

			const char* m_szConsulta = "pangya.ProcFindCard";
	};
}

#endif
