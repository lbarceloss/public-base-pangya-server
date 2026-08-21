
#pragma pack(1)
#include "player_info.hpp"

using namespace stdA;

PlayerInfo::PlayerInfo() {
	clear();
}

PlayerInfo::~PlayerInfo() {
	clear();
}

void PlayerInfo::clear() {

	player_info::clear();

	m_state = 0u;
}
