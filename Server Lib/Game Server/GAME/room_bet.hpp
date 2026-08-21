
#pragma once
#ifndef _STDA_ROOM_BET_HPP
#define _STDA_ROOM_BET_HPP

#include "room.h"

namespace stdA {

	class RoomBet : public room {
		public:
			RoomBet(unsigned char _channel_owner, RoomInfoEx _ri) : room(_channel_owner, _ri) {};
			virtual ~RoomBet() {};

			virtual bool isDropRoom() override { return false; };
	};
}

#endif
