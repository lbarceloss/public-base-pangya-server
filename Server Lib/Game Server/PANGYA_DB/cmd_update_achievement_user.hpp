
#pragma once
#ifndef _STDA_CMD_UPDATE_ACHIEVEMENT_USER_HPP
#define _STDA_CMD_UPDATE_ACHIEVEMENT_USER_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../TYPE/pangya_game_st.h"

namespace stdA {
	class CmdUpdateAchievementUser : public pangya_db {
		public:
			explicit CmdUpdateAchievementUser(bool _waiter = false);
			CmdUpdateAchievementUser(uint32_t _uid, AchievementInfoEx& _ai, bool _waiter = false);
			virtual ~CmdUpdateAchievementUser();

			uint32_t getUID();
			void setUID(uint32_t _uid);

			AchievementInfoEx& getInfo();
			void setInfo(AchievementInfoEx& _ai);

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdUpdateAchievementUser"; };
			virtual std::wstring _wgetName() override { return L"CmdUpdateAchievementUser"; };

		private:
			uint32_t m_uid;
			AchievementInfoEx m_ai;

			const char* m_szConsulta[3] = { "UPDATE pangya.pangya_achievement SET status = ", " WHERE UID = ", " AND ID_ACHIEVEMENT = " };
	};
}

#endif
