
#pragma once
#ifndef _STDA_UPDATE_ITEM_BUFF_HPP
#define _STDA_UPDATE_ITEM_BUFF_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../TYPE/pangya_game_st.h"

namespace stdA {
	class CmdUpdateItemBuff : public pangya_db {
		public:
			explicit CmdUpdateItemBuff(bool _waiter = false);
			CmdUpdateItemBuff(uint32_t _uid, ItemBuffEx& _ib, bool _waiter = false);
			virtual ~CmdUpdateItemBuff();

			uint32_t getUID();
			void setUID(uint32_t _uid);

			ItemBuffEx& getInfo();
			void setInfo(ItemBuffEx& _ib);

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdUpdateItemBuff"; };
			virtual std::wstring _wgetName() override { return L"CmdUpdateItemBuff"; };

		private:
			uint32_t m_uid;
			ItemBuffEx m_ib;

			const char* m_szConsulta = "pangya.ProcUpdateItemBuffTime";
	};
}

#endif
