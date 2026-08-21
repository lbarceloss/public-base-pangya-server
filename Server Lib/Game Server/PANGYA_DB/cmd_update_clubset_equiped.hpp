
#pragma once
#ifndef _STDA_CMD_UPDATE_CLUBSET_EQUIPED_HPP
#define _STDA_CMD_UPDATE_CLUBSET_EQUIPED_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"

namespace stdA {
	class CmdUpdateClubsetEquiped : public pangya_db {
		public:
			explicit CmdUpdateClubsetEquiped(bool _waiter = false);
			CmdUpdateClubsetEquiped(uint32_t _uid, int32_t _clubset_id, bool _waiter = false);
			virtual ~CmdUpdateClubsetEquiped();

			uint32_t getUID();
			void setUID(uint32_t _uid);

			int32_t getClubsetID();
			void setClubsetID(int32_t _clubset_id);

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdUpdateClubsetEquiped"; };
			virtual std::wstring _wgetName() override { return L"CmdUpdateClubsetEquiped"; };

		private:
			uint32_t m_uid;
			int32_t m_clubset_id;

			const char* m_szConsulta = "pangya.USP_FLUSH_CLUB";
	};
}

#endif
