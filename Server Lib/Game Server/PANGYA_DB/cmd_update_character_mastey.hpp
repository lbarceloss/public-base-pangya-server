
#pragma once
#ifndef _STDA_CMD_UPDATE_CHARACTER_MASTERY_HPP
#define _STDA_CMD_UPDATE_CHARACTER_MASTERY_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../TYPE/pangya_game_st.h"

namespace stdA {
	class CmdUpdateCharacterMastery : public pangya_db {
		public:
			explicit CmdUpdateCharacterMastery(bool _waiter = false);
			CmdUpdateCharacterMastery(uint32_t _uid, CharacterInfo& _ci, bool _waiter = false);
			virtual ~CmdUpdateCharacterMastery();

			uint32_t getUID();
			void setUID(uint32_t _uid);

			CharacterInfo& getInfo();
			void setInfo(CharacterInfo& _ci);

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdUpdateCharacterMastery"; };
			virtual std::wstring _wgetName() override { return L"CmdUpdateCharacterMastery"; };

		private:
			uint32_t m_uid;
			CharacterInfo m_ci;

			const char* m_szConsulta[3] = { "UPDATE pangya.pangya_character_information SET mastery = ", " WHERE UID = ", " AND item_id = " };
	};
}

#endif
