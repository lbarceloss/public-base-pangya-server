
#pragma once
#ifndef _STDA_CMD_SET_NOTICE_CADDIE_HOLY_DAY_HPP
#define _STDA_CMD_SET_NOTICE_CADDIE_HOLY_DAY_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"

namespace stdA {
	class CmdSetNoticeCaddieHolyDay : public pangya_db {
		public:
			explicit CmdSetNoticeCaddieHolyDay(bool _waiter = false);
			CmdSetNoticeCaddieHolyDay(uint32_t _uid, int32_t _id, unsigned short _check, bool _waiter = false);
			virtual ~CmdSetNoticeCaddieHolyDay();

			uint32_t getUID();
			void setUID(uint32_t _uid);

			int32_t getId();
			void setId(int32_t _id);

			unsigned short getCheck();
			void setCheck(unsigned short _check);

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdSetNoticeCaddieHolyDay"; };
			virtual std::wstring _wgetName() override { return L"CmdSetNoticeCaddieHolyDay"; };

		private:
			uint32_t m_uid;
			int32_t m_id;
			unsigned short m_check;

			const char* m_szConsulta[3] = { "UPDATE pangya.pangya_caddie_information SET CheckEnd = ", " WHERE UID = ", " AND item_id = " };
	};
}

#endif
