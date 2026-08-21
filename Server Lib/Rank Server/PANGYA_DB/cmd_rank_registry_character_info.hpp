
#pragma once
#ifndef _STDA_CMD_RANK_REGISTRY_CHARACTER_INFO_HPP
#define _STDA_CMD_RANK_REGISTRY_CHARACTER_INFO_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"

#include "../UTIL/rank_character.hpp"

namespace stdA {
	class CmdRankRegistryCharacterInfo : public pangya_db {
		public:
			explicit CmdRankRegistryCharacterInfo(bool _waiter = false);
			virtual ~CmdRankRegistryCharacterInfo();

			RankCharacterEntry& getInfo();

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			std::string _getName() override { return "CmdRankRegistryCharacterInfo"; };
			std::wstring _wgetName() override { return L"CmdRankRegistryCharacterInfo"; };

		private:
			RankCharacterEntry m_entry;

			const char* m_szConsulta = "pangya.ProcGetRankRegistryCharacterInfo";
	};
}

#endif
