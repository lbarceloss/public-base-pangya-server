
#pragma once
#ifndef _STDA_CMD_INSERT_PAPEL_SHOP_RARE_WIN_LOG_HPP
#define _STDA_CMD_INSERT_PAPEL_SHOP_RARE_WIN_LOG_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../TYPE/papel_shop_type.hpp"

namespace stdA {
	class CmdInsertPapelShopRareWinLog : public pangya_db {
		public:
			explicit CmdInsertPapelShopRareWinLog(bool _waiter = false);
			CmdInsertPapelShopRareWinLog(uint32_t _uid, ctx_papel_shop_ball& _ctx_psb, bool _waiter = false);
			virtual ~CmdInsertPapelShopRareWinLog();

			uint32_t getUID();
			void setUID(uint32_t _uid);

			ctx_papel_shop_ball& getInfo();
			void setInfo(ctx_papel_shop_ball& _ctx_psb);

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdInsertPapelShopRareWinLog"; };
			virtual std::wstring _wgetName() override { return L"CmdInsertPapelShopRareWinLog"; };

		private:
			uint32_t m_uid;
			ctx_papel_shop_ball m_ctx_psb;

			const char* m_szConsulta = "pangya.ProcInsertPapelShopRareWinLog";
	};
}

#endif
