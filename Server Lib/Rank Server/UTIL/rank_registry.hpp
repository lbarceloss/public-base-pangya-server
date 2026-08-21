
#pragma once
#ifndef _STDA_RANK_REGISTRY_HPP
#define _STDA_RANK_REGISTRY_HPP

#include "../TYPE/pangya_rank_st.hpp"

#include <string>
#include "../../Projeto IOCP/PACKET/packet.h"

#include <map>
#include <vector>

#define CHECK_LIMIT_RANK_POSITION_COMPACT_PACKET(__position) ((__position) > LIMIT_RANK_POSITION_TO_COMPACT_PACKET_SHOW)
#define CHECK_LIMIT_RANK_POSITION_COMPACT_PACKET_AND_SET(__position) ((CHECK_LIMIT_RANK_POSITION_COMPACT_PACKET((__position))) ? 0u : (__position))

namespace stdA {

	constexpr uint32_t LIMIT_RANK_POSITION_TO_COMPACT_PACKET_SHOW = 10000u;

	class RankRegistry {
		public:
			RankRegistry();
			RankRegistry(uint32_t _uid, uint32_t _current_position, uint32_t _last_position, int32_t _value);
			virtual ~RankRegistry();

			void clear();

			void toPacket(packet& _packet);

			void toCompactPacket(packet& _packet);

			uint32_t& getUID();
			uint32_t& getCurrentPosition();
			uint32_t& getLastPosition();
			int32_t& getValue();

			void setUID(uint32_t _uid);
			void setCurrentPosition(uint32_t _current_position);
			void setLastPosition(uint32_t _last_position);
			void setValue(int32_t _value);

		protected:
			uint32_t m_uid;
			uint32_t m_current_position;
			uint32_t m_last_position;
			int32_t m_value;

	};

	typedef std::map< key_position, RankRegistry > RankEntryValue;

	typedef std::map< key_menu, RankEntryValue > RankEntry;

	typedef std::pair< RankEntryValue::iterator  , RankEntryValue::iterator  > RankEntryValueRange;
}

#endif
