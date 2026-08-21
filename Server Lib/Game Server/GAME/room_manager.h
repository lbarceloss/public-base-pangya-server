
#pragma once
#ifndef _STDA_ROOM_MANAGER_H
#define _STDA_ROOM_MANAGER_H

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

#include <vector>
#include <climits>
#include "room.h"
#include "room_grand_prix.hpp"
#include "room_grand_zodiac_event.hpp"
#include "room_bet.hpp"
#include "room_bot_gm_event.hpp"
#include "../TYPE/bot_gm_event_type.hpp"

namespace stdA {
    class RoomManager {
        public:
            RoomManager(unsigned char _channel_id);
            ~RoomManager();

			void destroy();

			room* makeRoom(unsigned char _channel_owner, RoomInfoEx _ri, player* _session, int _option = 0);
			void destroyRoom(room* _room);

			RoomGrandPrix* makeRoomGrandPrix(unsigned char _channel_owner, RoomInfoEx _ri, player* _session, IFF::GrandPrixData& _gp, int _option = 0);

			RoomGrandZodiacEvent*  makeRoomGrandZodiacEvent(unsigned char _channel_owner, RoomInfoEx _ri);

			RoomBet* makeRoomBet(unsigned char _channel_owner, RoomInfoEx _ri);

			RoomBotGMEvent* makeRoomBotGMEvent(unsigned char _channel_owner, RoomInfoEx _ri, std::vector< stReward > _rewards);

			room* findRoom(short _numero);

			uint32_t findReconnectOID(uint32_t _uid);

			RoomGrandPrix* findRoomGrandPrix(uint32_t _typeid);

			std::vector< RoomInfo > getRoomsInfo(bool _without_practice_room = true);

			std::vector< RoomGrandZodiacEvent* > getAllRoomsGrandZodiacEvent();

			std::vector< RoomBotGMEvent* > getAllRoomsBotGMEvent();

			void unlockRoom(room* _r);

		protected:
			size_t findIndexRoom(room* _room);

		private:
			unsigned short getNewIndex();
			void clearIndex(unsigned short _index);

			unsigned char m_map_index[USHRT_MAX];

			unsigned char m_channel_id;

        protected:
            std::vector< room* > v_rooms;

		protected:
#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
			CONDITION_VARIABLE m_cv;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
			pthread_cond_t m_cv;
#endif
    };
}

#endif
