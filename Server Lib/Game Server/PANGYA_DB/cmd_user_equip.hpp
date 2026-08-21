
#pragma once
#ifndef _STDA_CMD_USER_EQUIP_HPP
#define _STDA_CMD_USER_EQUIP_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../TYPE/pangya_game_st.h"

namespace stdA {
	class CmdUserEquip : public pangya_db {
		public:
			explicit CmdUserEquip(bool _waiter = false);
			CmdUserEquip(uint32_t _uid, bool _waiter = false);
			virtual ~CmdUserEquip();

			UserEquip& getEquip();
			void setEquip(UserEquip& _ue);

			uint32_t getUID();
			void setUID(uint32_t _uid);

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdUserEquip"; };
			virtual std::wstring _wgetName() override { return L"CmdUserEquip"; };

		private:
			uint32_t m_uid;
			UserEquip m_ue;

			const char* m_szConsulta = "pangya.USP_CHAR_USER_EQUIP";
	};
}

#endif
