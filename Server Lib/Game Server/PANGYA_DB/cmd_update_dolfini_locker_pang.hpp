
#pragma once
#ifndef _STDA_CMD_UPDATE_DOLFINI_LOCKER_PANG_HPP
#define _STDA_CMD_UPDATE_DOLFINI_LOCKER_PANG_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"

namespace stdA {
	class CmdUpdateDolfiniLockerPang : public pangya_db {
		public:
			explicit CmdUpdateDolfiniLockerPang(bool _waiter = false);
			CmdUpdateDolfiniLockerPang(uint32_t _uid, uint64_t _pang, bool _waiter = false);
			virtual ~CmdUpdateDolfiniLockerPang();

			uint32_t getUID();
			void setUID(uint32_t _uid);

			uint64_t getPang();
			void setPang(uint64_t _pang);

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdUpdateDolfiniLockerPang"; };
			virtual std::wstring _wgetName() override { return L"CmdUpdateDolfiniLockerPang"; };

		private:
			uint32_t m_uid;
			uint64_t m_pang;

			const char* m_szConsulta[2] = { "UPDATE pangya.pangya_dolfini_locker SET pang = ", " WHERE UID = " };
	};
}

#endif
