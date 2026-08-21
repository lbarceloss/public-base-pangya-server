
#pragma once
#ifndef _STDA_CMD_UPDATE_UCC_HPP
#define STDA_CMD_UPDATE_UCC_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../TYPE/pangya_game_st.h"

namespace stdA {
	class CmdUpdateUCC : public pangya_db {
		public:
			enum T_UPDATE : unsigned char {
				TEMPORARY,
				FOREVER,
				COPY,
			};

		public:
			explicit CmdUpdateUCC(bool _waiter = false);
			CmdUpdateUCC(uint32_t _uid, WarehouseItemEx& _wi, SYSTEMTIME& _si, T_UPDATE _type, bool _waiter = false);
			virtual ~CmdUpdateUCC();

			uint32_t getUID();
			void setUID(uint32_t _uid);

			SYSTEMTIME& getDrawDate();
			void setDrawDate(SYSTEMTIME& _si);

			T_UPDATE getType();
			void setType(T_UPDATE _type);

			WarehouseItemEx& getInfo();
			void setInfo(WarehouseItemEx& _wi);

		protected:
			void lineResult(result_set::ctx_res *_result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdUpdateUCC"; };
			virtual std::wstring _wgetName() override { return L"CmdUpdateUCC"; };

		private:
			uint32_t m_uid;
			WarehouseItemEx m_wi;
			SYSTEMTIME m_dt_draw;
			T_UPDATE m_type;

			const char* m_szConsulta = "pangya.ProcUpdateUCC";
	};
}

#endif
