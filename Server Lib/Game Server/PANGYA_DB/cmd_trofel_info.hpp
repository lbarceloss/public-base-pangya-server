
#pragma once
#ifndef _STDA_CMD_TROFEL_INFO_HPP
#define _STDA_CMD_TROFEL_INFO_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../TYPE/pangya_game_st.h"

namespace stdA {
    class CmdTrofelInfo : public pangya_db {
		public:
			enum TYPE_SEASON : unsigned char {
				ALL,
				ONE,
				TWO,
				THREE,
				FOUR,
				CURRENT
			};

        public:
            explicit CmdTrofelInfo(bool _waiter = false);
            CmdTrofelInfo(uint32_t _uid, TYPE_SEASON _season, bool _waiter = false);
            virtual ~CmdTrofelInfo();

            TrofelInfo& getInfo();
            void setInfo(TrofelInfo& _ti);

            uint32_t getUID();
            void setUID(uint32_t _uid);

			TYPE_SEASON getSeason();
			void setSeason(TYPE_SEASON _season);

        protected:
            void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
            response* prepareConsulta(database& _db) override;

			virtual std::string _getName() override { return "CmdTrofelInfo"; };
			virtual std::wstring _wgetName() override { return L"CmdTrofelInfo"; };

        private:
			TYPE_SEASON m_season;
            uint32_t m_uid;
            TrofelInfo m_ti;

			const char* m_szConsulta = "pangya.ProcGetTrofel";
    };
}

#endif
