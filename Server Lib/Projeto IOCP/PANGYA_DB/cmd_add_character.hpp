
#pragma once
#ifndef _STDA_CMD_ADD_CHARACTER_HPP
#define _STDA_CMD_ADD_CHARACTER_HPP

#include "cmd_add_item_base.hpp"
#include "../TYPE/pangya_st.h"

namespace stdA {
	class CmdAddCharacter : public CmdAddItemBase {
		public:
			explicit CmdAddCharacter(bool _waiter = false);
			CmdAddCharacter(uint32_t _uid, CharacterInfo& _ci, unsigned char _purchase, unsigned char _gift_flag, bool _waiter = false);
			virtual ~CmdAddCharacter();

			CharacterInfo& getInfo();
			void setInfo(CharacterInfo& _ci);

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdAddCharacter"; };
			virtual std::wstring _wgetName() override { return L"CmdAddCharacter"; };

		private:
			CharacterInfo m_ci;

			const char* m_szConsulta = "pangya.ProcAddCharacter";
	};
}

#endif
