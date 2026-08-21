
#pragma once
#ifndef _STDA_PLAYER_HPP
#define _STDA_PLAYER_HPP

#include <cstdint>
#include <cstring>

#include "../../Projeto IOCP/SOCKET/session.h"
#include "../TYPE/pangya_game_st.h"
#include "../TYPE/player_info.h"
#include "../TYPE/gm_info.hpp"

#include "../TYPE/game_guard_type.hpp"

#include "../UTIL/block_exec_one_per_time.hpp"

namespace stdA {
    class player : public session {
        public:
            player(threadpool_base& _threapool);
            virtual ~player();

			virtual bool clear() override;

			virtual unsigned char getStateLogged() override;

			virtual uint32_t getUID() override;
			virtual uint32_t getCapability() override;
			virtual char* getNickname() override;
			virtual char* getID() override;

			virtual void addExp(uint32_t _exp, bool _upt_on_game = false);

			virtual void addCaddieExp(uint32_t _exp);

			virtual void addMascotExp(uint32_t _exp);

			static void addExp(uint32_t _uid, uint32_t _exp);

			virtual void addPang(uint64_t _pang);
			virtual void consomePang(uint64_t _pang);

			virtual void addCookie(uint64_t _cookie);
			virtual void consomeCookie(uint64_t _cookie);

			virtual void addMoeda(uint64_t _pang, uint64_t _cookie);
			virtual void consomeMoeda(uint64_t _pang, uint64_t _cookie);

			virtual void saveCPLog(CPLog& _cp_log);

			static void saveCPLog(uint32_t _uid, CPLog& _cp_log);

			bool checkCharacterEquipedPart(CharacterInfo& ci);
			bool checkCharacterEquipedAuxPart(CharacterInfo& ci);
			bool checkCharacterEquipedCutin(CharacterInfo& ci);
			void checkCharacterAllItemEquiped(CharacterInfo& ci);

			bool checkSkinEquiped(UserEquip& _ue);
			bool checkPosterEquiped(UserEquip& _ue);
			bool checkCharacterEquiped(UserEquip& _ue);
			bool checkCaddieEquiped(UserEquip& _ue);
			bool checkMascotEquiped(UserEquip& _ue);
			bool checkClubSetEquiped(UserEquip& _ue);
			bool checkBallEquiped(UserEquip& _ue);
			bool checkItemEquiped(UserEquip& _ue);
			void checkAllItemEquiped(UserEquip& _ue);

			void equipDefaultCharacter(UserEquip& _ue);
			void equipDefaultClubSet(UserEquip& _ue);
			void equipDefaultBall(UserEquip& _ue);
			void equipDefaultBallPremiumUser(UserEquip& _ue);

			std::vector< CharacterInfo* > isAuxPartEquiped(uint32_t _typeid);

			CharacterInfo* isPartEquiped(uint32_t _typeid);

		protected:
			static void SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg);

        public:
            PlayerInfo m_pi;
			GMInfo m_gi;

			PlayerGameGuard m_gg;

			struct AntiCheatData {
				uint8_t  hmac_key[32];
				uint8_t  pending_nonce[16];
				uint64_t nonce_sent_at;
				uint64_t last_challenge_at;
				uint64_t key_sent_at;
				bool     hmac_key_sent;
				bool     pending_nonce_valid;
				bool     first_attest_received;

				uint32_t crazy_seed;
				float    crazy_amp;
				uint64_t crazy_hole_started_at;
				bool     crazy_active;

				AntiCheatData() : nonce_sent_at(0), last_challenge_at(0), key_sent_at(0),
				                  hmac_key_sent(false), pending_nonce_valid(false),
				                  first_attest_received(false),
				                  crazy_seed(0), crazy_amp(5.0f), crazy_hole_started_at(0), crazy_active(false) {
					memset(hmac_key, 0, sizeof(hmac_key));
					memset(pending_nonce, 0, sizeof(pending_nonce));
				}
			} m_ac;

#if STDA_BLOCK_PACKET_ONE_TIME_DISABLE != 0x1

			SyncBlockExecOnePerTime* m_sbeopt;
#endif
    };
}

#endif
