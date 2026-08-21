
#pragma once
#ifndef _STDA_CMD_UPDATE_GRAND_PRIX_CLEAR_HPP
#define _STDA_CMD_UPDATE_GRAND_PRIX_CLEAR_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../TYPE/pangya_game_st.h"

namespace stdA {
	class CmdUpdateGrandPrixClear : public pangya_db {
		public:
			explicit CmdUpdateGrandPrixClear(bool _waiter = false);
			CmdUpdateGrandPrixClear(uint32_t _uid, GrandPrixClear& _gpc, bool _waiter = false);
			virtual ~CmdUpdateGrandPrixClear();

			uint32_t getUID();
			void setUID(uint32_t _uid);

			GrandPrixClear& getInfo();
			void setInfo(GrandPrixClear& _gpc);

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			std::string _getName() override { return "CmdUpdateGrandPrixClear"; };
			std::wstring _wgetName() override { return L"CmdUpdateGrandPrixClear"; };

		private:
			uint32_t m_uid;
			GrandPrixClear m_gpc;

			const char* m_szConsulta[3] = { "UPDATE pangya.pangya_grandprix_clear SET flag = ", " WHERE UID = ", " AND typeid = " };
	};
}

#endif
