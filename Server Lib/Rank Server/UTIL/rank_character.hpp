
#pragma once
#ifndef _STDA_RANK_CHARACTER_HPP
#define _STDA_RANK_CHARACTER_HPP

#include "../TYPE/pangya_rank_st.hpp"
#include "../../Projeto IOCP/PACKET/packet.h"

#include <string>

#include <map>

namespace stdA {
	class RankCharacter {
		public:
			RankCharacter();
			RankCharacter(uint32_t _uid, std::string _id, std::string _nickname, unsigned short _level, CharacterInfo _ci, unsigned char _term_s5_type = 0u, unsigned char _class_type = 0u);
			virtual ~RankCharacter();

			void clear();

			void playerInfoToPacket(packet& _packet);
			void playerFullInfoPacket(packet& _packet);
			void playerCharacterInfoToPacket(packet& _packet);

			uint32_t& getUID();
			const char* getId();
			const char* getNickname();
			unsigned short& getLevel();
			unsigned char& getTermS5Type();
			unsigned char& getClassType();

			CharacterInfo& getCharacterInfo();

			void setUID(uint32_t _uid);
			void setId(std::string _id);
			void setNickname(std::string _nickname);
			void setLevel(unsigned short _level);
			void setTermS5Type(unsigned char _term_s5_type);
			void setClassType(unsigned char _class_type);

			void setCharacterInfo(CharacterInfo _ci);

		protected:
			uint32_t m_uid;
			char m_id[22];
			char m_nickname[22];
			unsigned short m_level;

			unsigned char m_term_s5_type;
			unsigned char m_class_type;

			CharacterInfo m_ci;
	};

	typedef std::map< uint32_t  , RankCharacter > RankCharacterEntry;
}

#endif
