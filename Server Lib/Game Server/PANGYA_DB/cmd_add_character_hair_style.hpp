
#pragma once
#ifndef _STDA_CMD_ADD_CHARACTER_HAIR_STYLE_HPP
#define _STDA_CMD_ADD_CHARACTER_HAIR_STYLE_HPP

#include "../../Projeto IOCP/PANGYA_DB/cmd_add_item_base.hpp"
#include "../TYPE/pangya_game_st.h"

namespace stdA {
	class CmdAddCharacterHairStyle : public CmdAddItemBase {
		public:
			explicit CmdAddCharacterHairStyle(bool _waiter = false);
			CmdAddCharacterHairStyle(uint32_t _uid, CharacterInfo& _ci, unsigned char _purchase, unsigned char _gift_flag, bool _waiter = false);
			virtual ~CmdAddCharacterHairStyle();

			CharacterInfo& getInfo();
			void setInfo(CharacterInfo& _ci);

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdAddCharacterHairStyle"; };
			virtual std::wstring _wgetName() override { return L"CmdAddCharacterHairStyle"; };

		private:
			CharacterInfo m_ci;

			const char* m_szConsulta[3] = { "UPDATE pangya.pangya_character_information SET default_hair = ", " WHERE UID = ", " AND item_id = " };
	};
}

#endif
