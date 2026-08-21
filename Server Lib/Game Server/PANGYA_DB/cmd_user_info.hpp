
#pragma once
#ifndef _STDA_CMD_USER_INFO_HPP
#define _STDA_CMD_USER_INFO_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../TYPE/pangya_game_st.h"

namespace stdA {
    class CmdUserInfo : public pangya_db {
        public:
            explicit CmdUserInfo(bool _waiter = false);
            CmdUserInfo(uint32_t _uid, bool _waiter = false);
            virtual ~CmdUserInfo();

            UserInfoEx& getInfo();
            void setInfo(UserInfoEx& _ui);

            uint32_t getUID();
            void setUID(uint32_t _uid);

        protected:
            void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
            response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdUserInfo"; };
			virtual std::wstring _wgetName() override { return L"CmdUserInfo"; };

        private:
            uint32_t m_uid;
            UserInfoEx m_ui;

            const char* m_szConsulta = "pangya.GetInfo_User";
    };
}

#endif
