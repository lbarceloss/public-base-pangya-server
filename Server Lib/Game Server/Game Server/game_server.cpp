
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "game_server.h"
#include "../../Projeto IOCP/UTIL/exception.h"
#include "../../Projeto IOCP/UTIL/message_pool.h"
#include "../../Projeto IOCP/UTIL/iff.h"
#include "../PACKET/packet_func_sv.h"
#include "../../Projeto IOCP/SOCKET/socketserver.h"
#include "../../Projeto IOCP/SOCKET/socket.h"

#include <sstream>

#include "../../Projeto IOCP/UTIL/hex_util.h"
#include "../../Projeto IOCP/TYPE/stdAType.h"

#include "../GAME/item_manager.h"
#include "../PANGYA_DB/cmd_bet_lobby.hpp"

#include "../PANGYA_DB/cmd_daily_quest_info.hpp"
#include "../UTIL/mgr_daily_quest.hpp"

#include "../GAME/card_system.hpp"
#include "../GAME/comet_refill_system.hpp"
#include "../GAME/papel_shop_system.hpp"
#include "../GAME/box_system.hpp"
#include "../GAME/memorial_system.hpp"
#include "../GAME/mail_box_manager.hpp"
#include "../GAME/cube_coin_system.hpp"
#include "../GAME/coin_cube_location_update_system.hpp"
#include "../GAME/treasure_hunter_system.hpp"
#include "../GAME/drop_system.hpp"
#include "../GAME/attendance_reward_system.hpp"
#include "../GAME/approach_mission_system.hpp"
#include "../GAME/grand_zodiac_event.hpp"
#include "../GAME/golden_time_system.hpp"
#include "../GAME/login_reward_system.hpp"
#include "../GAME/bot_gm_event.hpp"
#include "../GAME/premium_system.hpp"

#include "../../Projeto IOCP/Smart Calculator/Smart Calculator.hpp"

#include "ghost_calc_bridge.hpp"

#include "../UTIL/map.hpp"

#include "../../Projeto IOCP/PANGYA_DB/cmd_auth_key_game.hpp"
#include "../../Projeto IOCP/PANGYA_DB/cmd_update_auth_key_login.hpp"
#include "../../Projeto IOCP/PANGYA_DB/cmd_insert_block_ip.hpp"
#include "../../Projeto IOCP/PANGYA_DB/cmd_insert_block_mac.hpp"

#include "../PANGYA_DB/cmd_gera_ucc_web_key.hpp"
#include "../PANGYA_DB/cmd_update_ucc.hpp"
#include "../PANGYA_DB/cmd_find_ucc.hpp"
#include "../PANGYA_DB/cmd_update_chat_macro_user.hpp"

#include "../PANGYA_DB/cmd_player_info.hpp"
#include "../PANGYA_DB/cmd_user_equip.hpp"
#include "../PANGYA_DB/cmd_map_statistics.hpp"
#include "../PANGYA_DB/cmd_member_info.hpp"
#include "../PANGYA_DB/cmd_user_info.hpp"
#include "../PANGYA_DB/cmd_trofel_info.hpp"
#include "../PANGYA_DB/cmd_character_info.hpp"
#include "../PANGYA_DB/cmd_trophy_special.hpp"
#include "../PANGYA_DB/cmd_guild_info.hpp"
#include "../PANGYA_DB/cmd_insert_msg_off.hpp"
#include "../PANGYA_DB/cmd_insert_ticker.hpp"
#include "../PANGYA_DB/cmd_mail_box_info.hpp"
#include "../PANGYA_DB/cmd_register_logon_server.hpp"

#include "../../Projeto IOCP/PANGYA_DB/cmd_rate_config_info.hpp"
#include "../../Projeto IOCP/PANGYA_DB/cmd_update_rate_config_info.hpp"

#include "../../Projeto IOCP/PANGYA_DB/cmd_register_logon.hpp"
#include "../../Projeto IOCP/PANGYA_DB/cmd_auth_key_login_info.hpp"

#include "../../Projeto IOCP/UTIL/string_util.hpp"

#include "../GAME/room_grand_prix.hpp"
#include "../GAME/room_grand_zodiac_event.hpp"
#include "../GAME/room_bot_gm_event.hpp"

#include "../../Projeto IOCP/UTIL/string_util.hpp"
#include "../../Projeto IOCP/UTIL/md5.hpp"

#if defined(_WIN32)
#include <DbgHelp.h>
#endif

#include "../UTIL/block_exec_one_per_time.hpp"

#define CHECK_SESSION_BEGIN(method) if (!_session.getState()) \
											throw exception("[game_server::" + std::string((method)) +"][Error] player nao esta connectado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0)); \

#define REQUEST_BEGIN(method) CHECK_SESSION_BEGIN(std::string("request") + (method)) \
								if (_packet == nullptr) \
									throw exception("[game_server::request" + std::string((method)) +"][Error] _packet is nullptr", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 6, 0)); \
								M_SMART_BLOCK_PACKET_ONE_TIME \

#define CHECK_SESSION_IS_AUTHORIZED(method) if (!_session.m_is_authorized) \
												throw exception("[game_server::request" + std::string((method)) + "][Error] Player[UID=" + std::to_string(_session.m_pi.uid) \
														+ "] Nao esta autorizado a fazer esse request por que ele ainda nao fez o login com o Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x7000501)); \

#define PACKET_VERSION_SERVER 2017082900

#define packet_ver_key "{782AE110-2EEF-4c61-B030-A53F17634F7D}"

#define de_encrypt_packet_ver(packet_ver) unsigned char *_tmp_p_ver = (unsigned char*)&(packet_ver); \
					size_t i, index; \
					for (i = 0, index = 0; i < sizeof(packet_ver_key); i++) { \
						_tmp_p_ver[index] ^= packet_ver_key[i]; \
						index = (index == 3) ? 0 : ++index; \
					} \

using namespace stdA;

game_server::game_server() : server(m_player_manager, 2, 26, 8), m_player_manager(*this, m_si.max_user), m_dqi{0}, m_login_manager(), m_timer_manager(),
		m_notice(60 ), m_ticker(30 ), m_GameGuardAuth(false), m_game_guard(nullptr) {

	if (m_state == FAILURE) {
		_smp::message_pool::getInstance().push(new message("[game_server::game_server][Error] na inicializacao do game server", CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	try {

		m_threads.push_back(new thread(TT_MONITOR, game_server::_check_player, (LPVOID)this, THREAD_PRIORITY_BELOW_NORMAL));

		config_init();

		init_systems();

		if (m_GameGuardAuth) {
#if INTPTR_MAX == INT64_MAX && MY_GG_SRV_LIB == 0
			m_game_guard = new unit_gg_auth_server_connect(*this, m_si);
#elif INTPTR_MAX == INT32_MAX || MY_GG_SRV_LIB == 1
			m_game_guard = new GGAuth(m_si.max_user);
#else
#error Unknown pointer size or missing size macros!
#endif
		}

		CmdDailyQuestInfo cmd_dqi(true);

		snmdb::NormalManagerDB::getInstance().add(1, &cmd_dqi, game_server::SQLDBResponse, this);

		cmd_dqi.waitEvent();

		if (cmd_dqi.getException().getCodeError() != 0)
			throw exception("[game_server::game_server][Error] nao conseguiu pegar o Daily Quest Info[Exption: "
				+ cmd_dqi.getException().getFullMessageError() + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 277, 0));

		m_dqi = cmd_dqi.getInfo();

#ifdef _DEBUG

#endif

		ChannelInfo ci{ 0 };
		int num_channel = m_reader_ini.readInt("CHANNELINFO", "NUM_CHANNEL");

		for (auto i = 0; i < num_channel; ++i) {
			ci.clear();

			ci.id = i;

#if defined(_WIN32)
			memcpy_s(ci.name, sizeof(ci.name), m_reader_ini.readString("CHANNEL" + std::to_string(i + 1), "NAME").c_str(), sizeof(ci.name));
#elif defined(__linux__)
			memcpy(ci.name, m_reader_ini.readString("CHANNEL" + std::to_string(i + 1), "NAME").c_str(), sizeof(ci.name));
#endif

			ci.max_user = m_reader_ini.readInt("CHANNEL" + std::to_string(i + 1), "MAXUSER");
			ci.max_level_allow = m_reader_ini.readInt("CHANNEL" + std::to_string(i + 1), "MAXLEVEL");

			try {
				ci.min_level_allow = m_reader_ini.readInt("CHANNEL" + std::to_string(i + 1), "LOWLEVEL");
			}catch (exception& e) {

				if (!STDA_ERROR_CHECK_SOURCE_AND_ERROR(e.getCodeError(), STDA_ERROR_TYPE::READER_INI, 6))
					_smp::message_pool::getInstance().push(new message("[game_server::game_server][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
			}

			try {
				ci.flag.ulFlag = m_reader_ini.readInt("CHANNEL" + std::to_string(i + 1), "FLAG");
			}catch (exception& e) {

				if (!STDA_ERROR_CHECK_SOURCE_AND_ERROR(e.getCodeError(), STDA_ERROR_TYPE::READER_INI, 6))
					_smp::message_pool::getInstance().push(new message("[game_server::game_server][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
			}

			v_channel.push_back(new channel(ci, m_si.propriedade));
		}

		packet_func::funcs.addPacketCall(0x02, packet_func::packet002, this);
		packet_func::funcs.addPacketCall(0x03, packet_func::packet003, this);
		packet_func::funcs.addPacketCall(0x04, packet_func::packet004, this);
		packet_func::funcs.addPacketCall(0x06, packet_func::packet006, this);
		packet_func::funcs.addPacketCall(0x07, packet_func::packet007, this);
		packet_func::funcs.addPacketCall(0x38, packet_func::packet038, this);
		packet_func::funcs.addPacketCall(0x08, packet_func::packet008, this);
		packet_func::funcs.addPacketCall(0x09, packet_func::packet009, this);
		packet_func::funcs.addPacketCall(0x0A, packet_func::packet00A, this);
		packet_func::funcs.addPacketCall(0x0B, packet_func::packet00B, this);
		packet_func::funcs.addPacketCall(0x0C, packet_func::packet00C, this);
		packet_func::funcs.addPacketCall(0x0D, packet_func::packet00D, this);
		packet_func::funcs.addPacketCall(0x0E, packet_func::packet00E, this);
		packet_func::funcs.addPacketCall(0x0F, packet_func::packet00F, this);
		packet_func::funcs.addPacketCall(0x10, packet_func::packet010, this);
		packet_func::funcs.addPacketCall(0x11, packet_func::packet011, this);
		packet_func::funcs.addPacketCall(0x12, packet_func::packet012, this);
		packet_func::funcs.addPacketCall(0x0BB9, packet_func::packet0BB9, this);
		packet_func::funcs.addPacketCall(0x13, packet_func::packet013, this);
		packet_func::funcs.addPacketCall(0x14, packet_func::packet014, this);
		packet_func::funcs.addPacketCall(0x15, packet_func::packet015, this);
		packet_func::funcs.addPacketCall(0x16, packet_func::packet016, this);
		packet_func::funcs.addPacketCall(0x17, packet_func::packet017, this);
		packet_func::funcs.addPacketCall(0x18, packet_func::packet018, this);
		packet_func::funcs.addPacketCall(0x19, packet_func::packet019, this);
		packet_func::funcs.addPacketCall(0x1A, packet_func::packet01A, this);
		packet_func::funcs.addPacketCall(0x1B, packet_func::packet01B, this);
		packet_func::funcs.addPacketCall(0x1C, packet_func::packet01C, this);
		packet_func::funcs.addPacketCall(0x1D, packet_func::packet01D, this);
		packet_func::funcs.addPacketCall(0x1F, packet_func::packet01F, this);
		packet_func::funcs.addPacketCall(0x20, packet_func::packet020, this);
		packet_func::funcs.addPacketCall(0x22, packet_func::packet022, this);
		packet_func::funcs.addPacketCall(0x26, packet_func::packet026, this);
		packet_func::funcs.addPacketCall(0x29, packet_func::packet029, this);
		packet_func::funcs.addPacketCall(0x2A, packet_func::packet02A, this);
		packet_func::funcs.addPacketCall(0x2D, packet_func::packet02D, this);
		packet_func::funcs.addPacketCall(0x2F, packet_func::packet02F, this);
		packet_func::funcs.addPacketCall(0x30, packet_func::packet030, this);
		packet_func::funcs.addPacketCall(0x31, packet_func::packet031, this);
		packet_func::funcs.addPacketCall(0x32, packet_func::packet032, this);
		packet_func::funcs.addPacketCall(0x33, packet_func::packet033, this);
		packet_func::funcs.addPacketCall(0x34, packet_func::packet034, this);
		packet_func::funcs.addPacketCall(0x35, packet_func::packet035, this);
		packet_func::funcs.addPacketCall(0x36, packet_func::packet036, this);
		packet_func::funcs.addPacketCall(0x37, packet_func::packet037, this);
		packet_func::funcs.addPacketCall(0x39, packet_func::packet039, this);
		packet_func::funcs.addPacketCall(0x3A, packet_func::packet03A, this);
		packet_func::funcs.addPacketCall(0x3C, packet_func::packet03C, this);
		packet_func::funcs.addPacketCall(0x3D, packet_func::packet03D, this);
		packet_func::funcs.addPacketCall(0x3E, packet_func::packet03E, this);
		packet_func::funcs.addPacketCall(0x41, packet_func::packet041, this);
		packet_func::funcs.addPacketCall(0x42, packet_func::packet042, this);
		packet_func::funcs.addPacketCall(0x43, packet_func::packet043, this);
		packet_func::funcs.addPacketCall(0x47, packet_func::packet047, this);
		packet_func::funcs.addPacketCall(0x48, packet_func::packet048, this);
		packet_func::funcs.addPacketCall(0x4A, packet_func::packet04A, this);
		packet_func::funcs.addPacketCall(0x4B, packet_func::packet04B, this);
		packet_func::funcs.addPacketCall(0x4F, packet_func::packet04F, this);
		packet_func::funcs.addPacketCall(0x54, packet_func::packet054, this);
		packet_func::funcs.addPacketCall(0x55, packet_func::packet055, this);
		packet_func::funcs.addPacketCall(0x57, packet_func::packet057, this);
		packet_func::funcs.addPacketCall(0x5C, packet_func::packet05C, this);
		packet_func::funcs.addPacketCall(0x60, packet_func::packet060, this);
		packet_func::funcs.addPacketCall(0x61, packet_func::packet061, this);
		packet_func::funcs.addPacketCall(0x63, packet_func::packet063, this);
		packet_func::funcs.addPacketCall(0x64, packet_func::packet064, this);
		packet_func::funcs.addPacketCall(0x65, packet_func::packet065, this);
		packet_func::funcs.addPacketCall(0x66, packet_func::packet066, this);
		packet_func::funcs.addPacketCall(0x67, packet_func::packet067, this);
		packet_func::funcs.addPacketCall(0x69, packet_func::packet069, this);
		packet_func::funcs.addPacketCall(0x6B, packet_func::packet06B, this);
		packet_func::funcs.addPacketCall(0x70, packet_func::packet070, this);
		packet_func::funcs.addPacketCall(0x73, packet_func::packet073, this);
		packet_func::funcs.addPacketCall(0x74, packet_func::packet074, this);
		packet_func::funcs.addPacketCall(0x75, packet_func::packet075, this);
		packet_func::funcs.addPacketCall(0x76, packet_func::packet076, this);
		packet_func::funcs.addPacketCall(0x77, packet_func::packet077, this);
		packet_func::funcs.addPacketCall(0x78, packet_func::packet078, this);
		packet_func::funcs.addPacketCall(0x79, packet_func::packet079, this);
		packet_func::funcs.addPacketCall(0x7A, packet_func::packet07A, this);
		packet_func::funcs.addPacketCall(0x7B, packet_func::packet07B, this);
		packet_func::funcs.addPacketCall(0x7C, packet_func::packet07C, this);
		packet_func::funcs.addPacketCall(0x7D, packet_func::packet07D, this);
		packet_func::funcs.addPacketCall(0x81, packet_func::packet081, this);
		packet_func::funcs.addPacketCall(0x82, packet_func::packet082, this);
		packet_func::funcs.addPacketCall(0x83, packet_func::packet083, this);
		packet_func::funcs.addPacketCall(0x88, packet_func::packet088, this);
		packet_func::funcs.addPacketCall(0x8B, packet_func::packet08B, this);
		packet_func::funcs.addPacketCall(0x8F, packet_func::packet08F, this);
		packet_func::funcs.addPacketCall(0x98, packet_func::packet098, this);
		packet_func::funcs.addPacketCall(0x9C, packet_func::packet09C, this);
		packet_func::funcs.addPacketCall(0x9D, packet_func::packet09D, this);
		packet_func::funcs.addPacketCall(0x9E, packet_func::packet09E, this);
		packet_func::funcs.addPacketCall(0xA1, packet_func::packet0A1, this);
		packet_func::funcs.addPacketCall(0xA2, packet_func::packet0A2, this);
		packet_func::funcs.addPacketCall(0xAA, packet_func::packet0AA, this);
		packet_func::funcs.addPacketCall(0xAB, packet_func::packet0AB, this);
		packet_func::funcs.addPacketCall(0xAE, packet_func::packet0AE, this);
		packet_func::funcs.addPacketCall(0xB2, packet_func::packet0B2, this);

		packet_func::funcs.addPacketCall(0xB4, packet_func::packet0B4, this);
		packet_func::funcs.addPacketCall(0xB5, packet_func::packet0B5, this);
		packet_func::funcs.addPacketCall(0xB7, packet_func::packet0B7, this);
		packet_func::funcs.addPacketCall(0xB9, packet_func::packet0B9, this);
		packet_func::funcs.addPacketCall(0xBA, packet_func::packet0BA, this);
		packet_func::funcs.addPacketCall(0xBD, packet_func::packet0BD, this);
		packet_func::funcs.addPacketCall(0xC1, packet_func::packet0C1, this);
		packet_func::funcs.addPacketCall(0xC9, packet_func::packet0C9, this);
		packet_func::funcs.addPacketCall(0xCA, packet_func::packet0CA, this);
		packet_func::funcs.addPacketCall(0xCB, packet_func::packet0CB, this);
		packet_func::funcs.addPacketCall(0xCC, packet_func::packet0CC, this);
		packet_func::funcs.addPacketCall(0xCD, packet_func::packet0CD, this);
		packet_func::funcs.addPacketCall(0xCE, packet_func::packet0CE, this);
		packet_func::funcs.addPacketCall(0xCF, packet_func::packet0CF, this);
		packet_func::funcs.addPacketCall(0xD0, packet_func::packet0D0, this);
		packet_func::funcs.addPacketCall(0xD1, packet_func::packet0D1, this);
		packet_func::funcs.addPacketCall(0xD2, packet_func::packet0D2, this);
		packet_func::funcs.addPacketCall(0xD3, packet_func::packet0D3, this);
		packet_func::funcs.addPacketCall(0xD4, packet_func::packet0D4, this);
		packet_func::funcs.addPacketCall(0xD5, packet_func::packet0D5, this);
		packet_func::funcs.addPacketCall(0xD8, packet_func::packet0D8, this);
		packet_func::funcs.addPacketCall(0xDE, packet_func::packet0DE, this);
		packet_func::funcs.addPacketCall(0xE5, packet_func::packet0E5, this);
		packet_func::funcs.addPacketCall(0xE6, packet_func::packet0E6, this);
		packet_func::funcs.addPacketCall(0xE7, packet_func::packet0E7, this);
		packet_func::funcs.addPacketCall(0xEB, packet_func::packet0EB, this);
		packet_func::funcs.addPacketCall(0xEC, packet_func::packet0EC, this);
		packet_func::funcs.addPacketCall(0xEF, packet_func::packet0EF, this);
		packet_func::funcs.addPacketCall(0xF4, packet_func::packet0F4, this);
		packet_func::funcs.addPacketCall(0xFB, packet_func::packet0FB, this);
		packet_func::funcs.addPacketCall(0xFE, packet_func::packet0FE, this);
		packet_func::funcs.addPacketCall(0x119, packet_func::packet119, this);
		packet_func::funcs.addPacketCall(0x126, packet_func::packet126, this);
		packet_func::funcs.addPacketCall(0x127, packet_func::packet127, this);
		packet_func::funcs.addPacketCall(0x128, packet_func::packet128, this);
		packet_func::funcs.addPacketCall(0x129, packet_func::packet129, this);
		packet_func::funcs.addPacketCall(0x12C, packet_func::packet12C, this);
		packet_func::funcs.addPacketCall(0x12D, packet_func::packet12D, this);
		packet_func::funcs.addPacketCall(0x12E, packet_func::packet12E, this);
		packet_func::funcs.addPacketCall(0x12F, packet_func::packet12F, this);
		packet_func::funcs.addPacketCall(0x130, packet_func::packet130, this);
		packet_func::funcs.addPacketCall(0x131, packet_func::packet131, this);
		packet_func::funcs.addPacketCall(0x137, packet_func::packet137, this);
		packet_func::funcs.addPacketCall(0x138, packet_func::packet138, this);
		packet_func::funcs.addPacketCall(0x140, packet_func::packet140, this);
		packet_func::funcs.addPacketCall(0x141, packet_func::packet141, this);
		packet_func::funcs.addPacketCall(0x143, packet_func::packet143, this);
		packet_func::funcs.addPacketCall(0x144, packet_func::packet144, this);
		packet_func::funcs.addPacketCall(0x145, packet_func::packet145, this);
		packet_func::funcs.addPacketCall(0x146, packet_func::packet146, this);
		packet_func::funcs.addPacketCall(0x147, packet_func::packet147, this);
		packet_func::funcs.addPacketCall(0x14B, packet_func::packet14B, this);
		packet_func::funcs.addPacketCall(0x151, packet_func::packet151, this);
		packet_func::funcs.addPacketCall(0x152, packet_func::packet152, this);
		packet_func::funcs.addPacketCall(0x153, packet_func::packet153, this);
		packet_func::funcs.addPacketCall(0x154, packet_func::packet154, this);
		packet_func::funcs.addPacketCall(0x155, packet_func::packet155, this);
		packet_func::funcs.addPacketCall(0x156, packet_func::packet156, this);
		packet_func::funcs.addPacketCall(0x157, packet_func::packet157, this);
		packet_func::funcs.addPacketCall(0x158, packet_func::packet158, this);
		packet_func::funcs.addPacketCall(0x15C, packet_func::packet15C, this);
		packet_func::funcs.addPacketCall(0x15D, packet_func::packet15D, this);
		packet_func::funcs.addPacketCall(0x164, packet_func::packet164, this);
		packet_func::funcs.addPacketCall(0x165, packet_func::packet165, this);
		packet_func::funcs.addPacketCall(0x166, packet_func::packet166, this);
		packet_func::funcs.addPacketCall(0x167, packet_func::packet167, this);
		packet_func::funcs.addPacketCall(0x168, packet_func::packet168, this);
		packet_func::funcs.addPacketCall(0x169, packet_func::packet169, this);
		packet_func::funcs.addPacketCall(0x16B, packet_func::packet16B, this);
		packet_func::funcs.addPacketCall(0x16C, packet_func::packet16C, this);
		packet_func::funcs.addPacketCall(0x16D, packet_func::packet16D, this);
		packet_func::funcs.addPacketCall(0x16E, packet_func::packet16E, this);
		packet_func::funcs.addPacketCall(0x16F, packet_func::packet16F, this);
		packet_func::funcs.addPacketCall(0x171, packet_func::packet171, this);
		packet_func::funcs.addPacketCall(0x172, packet_func::packet172, this);
		packet_func::funcs.addPacketCall(0x176, packet_func::packet176, this);
		packet_func::funcs.addPacketCall(0x177, packet_func::packet177, this);
		packet_func::funcs.addPacketCall(0x179, packet_func::packet179, this);
		packet_func::funcs.addPacketCall(0x17A, packet_func::packet17A, this);
		packet_func::funcs.addPacketCall(0x17F, packet_func::packet17F, this);
		packet_func::funcs.addPacketCall(0x180, packet_func::packet180, this);
		packet_func::funcs.addPacketCall(0x181, packet_func::packet181, this);
		packet_func::funcs.addPacketCall(0x184, packet_func::packet184, this);
		packet_func::funcs.addPacketCall(0x185, packet_func::packet185, this);
		packet_func::funcs.addPacketCall(0x186, packet_func::packet186, this);
		packet_func::funcs.addPacketCall(0x187, packet_func::packet187, this);
		packet_func::funcs.addPacketCall(0x188, packet_func::packet188, this);
		packet_func::funcs.addPacketCall(0x189, packet_func::packet189, this);
		packet_func::funcs.addPacketCall(0x18A, packet_func::packet18A, this);
		packet_func::funcs.addPacketCall(0x18B, packet_func::packet18B, this);
		packet_func::funcs.addPacketCall(0x18C, packet_func::packet18C, this);
		packet_func::funcs.addPacketCall(0x18D, packet_func::packet18D, this);
		packet_func::funcs.addPacketCall(0x192, packet_func::packet192, this);
		packet_func::funcs.addPacketCall(0x196, packet_func::packet196, this);
		packet_func::funcs.addPacketCall(0x197, packet_func::packet197, this);
		packet_func::funcs.addPacketCall(0x198, packet_func::packet198, this);
		packet_func::funcs.addPacketCall(0x199, packet_func::packet199, this);
		packet_func::funcs.addPacketCall(0x1F4, packet_func::packet1F4, this);

		packet_func::funcs_sv.addPacketCall(0x0BB8, packet_func::packet_svFazNada, this);

		{
			job _job_bet(game_server::betLobbyTick, this, (void*)0);

			m_timer_manager.createTimer(3000, new (timer::timer_param){ _job_bet, m_job_pool },
					std::vector< DWORD > { 3000 }, timer::PERIODIC_INFINITE);

			_smp::message_pool::getInstance().push(new message("[CRAZY][game_server][Log] poller do bet_lobby ligado (a cada 3s, infinito).", CL_FILE_LOG_AND_CONSOLE));
		}
		packet_func::funcs_sv.addPacketCall(0x3F, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x40, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x42, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x44, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x45, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x46, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x47, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x48, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x49, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x4A, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x4B, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x4C, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x4D, packet_func::packet_sv4D, this);
		packet_func::funcs_sv.addPacketCall(0x4E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x50, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x52, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x53, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x55, packet_func::packet_sv055, this);
		packet_func::funcs_sv.addPacketCall(0x56, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x58, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x59, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x5A, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x5B, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x5C, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x5D, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x60, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x61, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x63, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x64, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x65, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x66, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x67, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x68, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x6A, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x6B, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x6C, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x6D, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x6E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x70, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x71, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x72, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x73, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x76, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x77, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x78, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x79, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x7C, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x7D, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x7E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x83, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x84, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x86, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x89, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x8A, packet_func::packet_sv055, this);
		packet_func::funcs_sv.addPacketCall(0x8B, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x8C, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x8D, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x8E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x90, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x91, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x92, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x93, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x94, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x95, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x96, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x97, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x9A, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x9E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x9F, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xA1, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xA2, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xA3, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xA4, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xA5, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xA7, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xAA, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xAC, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xB0, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xB2, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xB4, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xB9, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xBA, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xBF, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xC2, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xC4, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xC5, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xC7, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xC8, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xC9, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xCA, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xCC, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xCE, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xD4, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xD7, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xE1, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xE2, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xE3, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xE4, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xE5, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xE6, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xE7, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xE8, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xE9, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xEA, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xEB, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xEC, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xED, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xF1, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xF5, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xF6, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xF8, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xF9, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xFA, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xFB, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0xFC, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x101, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x102, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x10B, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x10E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x10F, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x113, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x115, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x11A, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x11B, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x11C, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x11F, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x129, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x12A, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x12B, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x12D, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x12E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x12F, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x130, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x131, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x132, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x133, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x134, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x135, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x136, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x137, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x138, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x139, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x13F, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x144, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x14E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x14F, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x150, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x151, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x153, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x154, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x156, packet_func::packet_svRequestInfo, this);
		packet_func::funcs_sv.addPacketCall(0x157, packet_func::packet_svRequestInfo, this);
		packet_func::funcs_sv.addPacketCall(0x158, packet_func::packet_svRequestInfo, this);
		packet_func::funcs_sv.addPacketCall(0x159, packet_func::packet_svRequestInfo, this);
		packet_func::funcs_sv.addPacketCall(0x15A, packet_func::packet_svRequestInfo, this);
		packet_func::funcs_sv.addPacketCall(0x15B, packet_func::packet_svRequestInfo, this);
		packet_func::funcs_sv.addPacketCall(0x15C, packet_func::packet_svRequestInfo, this);
		packet_func::funcs_sv.addPacketCall(0x15D, packet_func::packet_svRequestInfo, this);
		packet_func::funcs_sv.addPacketCall(0x15E, packet_func::packet_svRequestInfo, this);
		packet_func::funcs_sv.addPacketCall(0x160, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x168, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x169, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x16A, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x16C, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x16D, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x16E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x16F, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x170, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x171, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x172, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x24D, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x24E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x173, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x174, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x176, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x181, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x18D, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x18F, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x190, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x196, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x197, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x198, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x199, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x19D, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1A9, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1AD, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1B1, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1D3, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1D4, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1D9, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1E7, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1E8, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1E9, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1EA, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1EC, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1EE, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1EF, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1F0, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1F1, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1F2, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1F3, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1F4, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1F5, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1F7, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1F8, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1F9, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x1FA, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x200, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x201, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x203, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x20E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x210, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x211, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x212, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x213, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x214, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x215, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x216, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x21B, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x21D, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x21E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x220, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x225, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x226, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x227, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x228, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x229, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x22A, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x22B, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x22C, packet_func::packet_svFazNada , this);
		packet_func::funcs_sv.addPacketCall(0x22D, packet_func::packet_svFazNada , this);
		packet_func::funcs_sv.addPacketCall(0x22E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x22F, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x230, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x231, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x236, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x237, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x23D, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x23E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x23F, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x240, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x241, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x242, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x243, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x244, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x245, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x246, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x247, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x248, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x249, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x24C, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x24F, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x250, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x251, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x253, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x254, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x255, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x256, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x257, packet_func::packet_svRequestInfo, this);
		packet_func::funcs_sv.addPacketCall(0x258, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x259, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x25A, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x25C, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x25D, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x264, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x265, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x266, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x26A, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x26B, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x26C, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x26D, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x26E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x26F, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x270, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x271, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x272, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x273, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x274, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x27E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x27F, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x280, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x281, packet_func::packet_svFazNada, this);

		packet_func::funcs_as.addPacketCall(0x1, packet_func::packet_as001, this);

		m_state = INITIALIZED;

	}catch (exception& e) {
		_smp::message_pool::getInstance().push(new message(e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		m_state = FAILURE;
	}
}

game_server::~game_server() {

	clear();

	if (m_game_guard != nullptr)
		delete m_game_guard;

	m_game_guard = nullptr;

};

void game_server::clear() {

	for (auto i = 0u; i < v_channel.size(); ++i)
		if (v_channel[i] != nullptr)
			delete v_channel[i];

	v_channel.clear();
	v_channel.shrink_to_fit();

	m_chat_discord = false;
};

channel* game_server::enterChannel(player& _session, unsigned char _channel) {
	CHECK_SESSION_BEGIN("enterChannel");

	channel *enter = nullptr, *last = nullptr;

	try {

		if ((enter = findChannel(_channel)) == nullptr)
			throw exception("[game_server::enterChannel][Error] id channel nao exite.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 2, 0));

		if (enter->getId() == _session.m_pi.channel) {

			packet p;
			packet_func::pacote04E(p, &_session, 1);
			packet_func::session_send(p, &_session, 0);

			return enter;
		}

		if (enter->isFull()) {

			enter = nullptr;

			packet p;
			packet_func::pacote04E(p, &_session, 2 );
			packet_func::session_send(p, &_session, 0);

		}else {

			enter->checkEnterChannel(_session);

			if (_session.m_pi.channel != INVALID_CHANNEL && (last = findChannel(_session.m_pi.channel)) != nullptr)
				last->leaveChannel(_session);

			enter->enterChannel(_session);

		}

	}catch (exception& e) {

		enter = nullptr;

		packet p;
		packet_func::pacote04E(p, &_session, 6, (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5700052);

		packet_func::session_send(p, &_session, 0);

		_smp::message_pool::getInstance().push(new message("[game_server::enterChannel][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::CHANNEL)
			throw;
	}

	return enter;
};

void game_server::sendChannelListToSession(player& _session) {
	CHECK_SESSION_BEGIN("sendChannelListToSession");

	packet p;

	try {

		packet_func::pacote04D(p, &_session, v_channel);
		packet_func::session_send(p, &_session, 0);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::sendChannelListToSession][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::sendServerListAndChannelListToSession(player& _session) {
	CHECK_SESSION_BEGIN("sendServerListAndChannelListToSession");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("sendServerListAndChannelListToSession");

		packet_func::pacote09F(p, &_session, m_server_list, v_channel);
		packet_func::session_send(p, &_session, 0);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::sendServerListAndChannelListToSession][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::sendDateTimeToSession(player& _session) {
	CHECK_SESSION_BEGIN("sendDateTimeToSession");

	SYSTEMTIME localtime{ 0 };

	try {

		CHECK_SESSION_IS_AUTHORIZED("sendDateTimeToSession");

		GetLocalTime(&localtime);

		packet p((unsigned short)0xBA);

		p.addBuffer(&localtime, sizeof(SYSTEMTIME));

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::sendDateTimeToSession][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::sendRankServer(player& _session) {
	CHECK_SESSION_BEGIN("sendRankServer");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("sendRankServer");

		if (_session.m_pi.block_flag.m_flag.stBit.rank_server)
			throw exception("[game_server::sendRankServer][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] esta bloqueado o Rank Server, ele nao pode acessar o rank server.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 7010, 0));

		CmdServerList cmd_sl(CmdServerList::TYPE_SERVER::RANK, true);

		snmdb::NormalManagerDB::getInstance().add(0, &cmd_sl, nullptr, nullptr);

		cmd_sl.waitEvent();

		if (cmd_sl.getException().getCodeError() != 0)
			throw cmd_sl;

		auto sl = cmd_sl.getServerList();

		if (sl.empty())
			throw exception("[game_server::sendRankServer][WARNING] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] requisitou o Rank Server, mas nao tem nenhum Rank Server online no DB.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 7011, 0));

		p.init_plain((unsigned short)0xA2);

		p.addString(sl[0].ip);
		p.addUint32(sl[0].port);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::sendRankServer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0xA2);

		p.addUint16(0u);
		p.addUint32(0u);

		packet_func::session_send(p, &_session, 1);
	}
};

channel *game_server::findChannel(uint32_t _channel) {

	if (_channel == INVALID_CHANNEL)
		return nullptr;

	for (auto i = 0u; i < v_channel.size(); ++i)
		if (v_channel[i]->getId() == _channel)
			return v_channel[i];

	return nullptr;
};

player* game_server::findPlayer(uint32_t _uid, bool _oid) {
	return m_player_manager.findPlayer(_uid, _oid);
};

std::vector< player* > game_server::findAllGM() {
	return m_player_manager.findAllGM();
}

void game_server::blockOID(uint32_t _oid) {
	m_player_manager.blockOID(_oid);
};

void game_server::unblockOID(uint32_t _oid) {
	m_player_manager.unblockOID(_oid);
};

uint32_t game_server::findReconnectOID(uint32_t _uid) {

	uint32_t oid = 0u;

	if (_uid == 0u)
		return oid;

	for (auto i = 0u; i < v_channel.size() && oid == 0u; ++i)
		if (v_channel[i] != nullptr)
			oid = v_channel[i]->findReconnectOID(_uid);

	return oid;
};

bool game_server::reclaimOID(uint32_t _oid_cadeira, uint32_t _oid_novo) {
	return m_player_manager.reclaimOID(_oid_cadeira, _oid_novo);
};

LoginManager& game_server::getLoginManager() {
	return m_login_manager;
};

void game_server::sendSmartCalculatorReplyToPlayer(const uint32_t _uid, std::string _from, std::string _msg) {

	try {

		auto player = findPlayer(_uid);

		if (player == nullptr) {

			_smp::message_pool::getInstance().push(new message("[game_server::sendSmartCalculatorReplyToPlayer][WARNING] Player[UID="
					+ std::to_string(_uid) + "] nao esta mais online.", CL_FILE_LOG_AND_CONSOLE));

			return;
		}

		auto reply = clearBreakLineAndTab(_msg);

		auto v_msg = split(reply, "\n");

		v_msg = limit_chat_display(v_msg, (80 - (uint32_t)strlen(player->m_pi.nickname)));

		packet p;

		for (auto& el : v_msg) {

			p.init_plain((unsigned short)0x84);

			p.addUint8(1);

			p.addString(_from);
			p.addString(el);

#ifdef _DEBUG

#endif

			packet_func::session_send(p, player, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::sendSmartCalculatorReplyToPlayer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void game_server::sendNoticeGMFromDiscordCmd(std::string& _notice) {

	try {

		if (_notice.empty())
			return;

		_smp::message_pool::getInstance().push(new message("[game_server::sendNoticeGMFromDiscordCmd][Log] sending notice[" + _notice + "] from discord command.", CL_FILE_LOG_AND_CONSOLE));

		packet p((unsigned short)0x40);

		p.addUint8(7);

		p.addString("Discord_CMD");
		p.addString(_notice);

		for (auto& el : v_channel)
			packet_func::channel_broadcast(*el, p, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::sendNoticeGMFromDiscordCmd][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::requestLogin(player& _session, packet *_packet) {
	REQUEST_BEGIN("Login");

	packet p;

	try {

		size_t ii = 0, packet_version = 0;

		KeysOfLogin kol = { 0 };

		AuthKeyLoginInfo akli{ 0 };
		AuthKeyGameInfo akgi{};

		std::string client_version;

		player_info _pi;

		PlayerInfo *pi = &(_session.m_pi);
		pi->clear();

		auto copyFixed = [](char* _dst, size_t _dstsz, const std::string& _src) {
			size_t _n = _src.size() < _dstsz ? _src.size() : _dstsz - 1;
			memset(_dst, 0, _dstsz);
			memcpy(_dst, _src.data(), _n);
		};

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[game_server::requestLogin][Log] Hex:\n\r" + hex_util::BufferToHexString(_packet->getBuffer(), _packet->getSize()), CL_FILE_LOG_AND_CONSOLE));

		copyFixed(_pi.id, sizeof(_pi.id), _packet->readString());

		_smp::message_pool::getInstance().push(new message("id: " + std::string(_pi.id), CL_FILE_LOG_AND_CONSOLE));

		_pi.uid = _packet->readInt32();

		_smp::message_pool::getInstance().push(new message("uid: " + std::to_string(_pi.uid), CL_FILE_LOG_AND_CONSOLE));
		_smp::message_pool::getInstance().push(new message("ntKey: " + std::to_string(_packet->readInt32()), CL_FILE_LOG_AND_CONSOLE));
		_smp::message_pool::getInstance().push(new message("command: " + hex_util::ltoaToHex(_packet->readUint16()), CL_FILE_LOG_AND_CONSOLE));

		copyFixed(kol.keys[0], sizeof(kol.keys[0]), _packet->readString());

		_smp::message_pool::getInstance().push(new message("authKey: " + std::string(kol.keys[0]) , CL_FILE_LOG_AND_CONSOLE));

		client_version = _packet->readString();

		_smp::message_pool::getInstance().push(new message("Version: " + client_version , CL_FILE_LOG_AND_CONSOLE));

		packet_version = _packet->readInt32();

		_smp::message_pool::getInstance().push(new message("Packet Version: " + std::to_string(packet_version), CL_FILE_LOG_AND_CONSOLE));

		std::string mac_address = _packet->readString();

		_smp::message_pool::getInstance().push(new message("Mac Address: " + mac_address, CL_FILE_LOG_AND_CONSOLE));

		copyFixed(kol.keys[1], sizeof(kol.keys[1]), _packet->readString());

		_smp::message_pool::getInstance().push(new message("AuthKey2: " + std::string(kol.keys[1]) , CL_FILE_LOG_AND_CONSOLE));
#else

		copyFixed(_pi.id, sizeof(_pi.id), _packet->readString());

		_smp::message_pool::getInstance().push(new message("id: " + std::string(_pi.id), CL_ONLY_FILE_LOG));

		_pi.uid = _packet->readInt32();

		_smp::message_pool::getInstance().push(new message("uid: " + std::to_string(_pi.uid), CL_ONLY_FILE_LOG));
		_smp::message_pool::getInstance().push(new message("ntKey: " + std::to_string(_packet->readInt32()), CL_ONLY_FILE_LOG));
		_smp::message_pool::getInstance().push(new message("command: " + hex_util::ltoaToHex(_packet->readUint16()), CL_ONLY_FILE_LOG));

		copyFixed(kol.keys[0], sizeof(kol.keys[0]), _packet->readString());

		_smp::message_pool::getInstance().push(new message("authKey: " + std::string(kol.keys[0]) , CL_ONLY_FILE_LOG));

		client_version = _packet->readString();

		_smp::message_pool::getInstance().push(new message("Version: " + client_version , CL_ONLY_FILE_LOG));

		packet_version = _packet->readInt32();

		_smp::message_pool::getInstance().push(new message("Packet Version: " + std::to_string(packet_version), CL_ONLY_FILE_LOG));

		std::string mac_address = _packet->readString();

		_smp::message_pool::getInstance().push(new message("Mac Address: " + mac_address, CL_ONLY_FILE_LOG));

		copyFixed(kol.keys[1], sizeof(kol.keys[1]), _packet->readString());

		_smp::message_pool::getInstance().push(new message("AuthKey2: " + std::string(kol.keys[1]) , CL_ONLY_FILE_LOG));

#endif

		if (haveBanList(_session.m_ip, mac_address))
			throw exception("[game_server::requestLogin][Error] Player[UID=" + std::to_string(_pi.uid) + ", IP="
					+ _session.m_ip +", MAC=" + mac_address + "] esta bloqueado por regiao IP/MAC Addrress.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1058, 0));

		if (_pi.id[0] == '\0')
			throw exception("[game_server::requestLogin][Error] Player[UID=" + std::to_string(_pi.uid)
					+ ", IP=" + _session.m_ip + "] id que o player enviou eh invalido. id: " + std::string(_pi.id), STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1050, 0));

		CmdPlayerInfo cmd_pi(_pi.uid, true);

		snmdb::NormalManagerDB::getInstance().add(0, &cmd_pi, nullptr, nullptr);

		cmd_pi.waitEvent();

		if (cmd_pi.getException().getCodeError() != 0)
			throw cmd_pi.getException();

		*(player_info*)pi = cmd_pi.getInfo();

		if (pi->uid == ~0u)
			throw exception("[game_server::requestLogin][Error] player[UID=" + std::to_string(_pi.uid) + "] nao existe no banco de dados", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1051, 0));

		if (strcmp(pi->id, _pi.id) != 0)
			throw exception("[game_server::requestLogin][Error] Player[UID=" + std::to_string(pi->uid) + ", REQ_UID="
					+ std::to_string(_pi.uid) + "] Player ID nao bate : client send ID : " + std::string(_pi.id) + "\t player DB ID : "
					+ std::string(pi->id), STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1052, 0));

		if (pi->block_flag.m_id_state.id_state.ull_IDState != 0) {

			if (pi->block_flag.m_id_state.id_state.st_IDState.L_BLOCK_TEMPORARY && (pi->block_flag.m_id_state.block_time == -1 || pi->block_flag.m_id_state.block_time > 0)) {

				throw exception("[game_server::requestLogin][Log] Bloqueado por tempo[Time="
						+ (pi->block_flag.m_id_state.block_time == -1 ? std::string("indeterminado") : (std::to_string(pi->block_flag.m_id_state.block_time / 60)
						+ "min " + std::to_string(pi->block_flag.m_id_state.block_time % 60) + "sec"))
						+ "]. player [UID=" + std::to_string(pi->uid) + ", ID=" + std::string(pi->id) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1029, 0));

			}else if (pi->block_flag.m_id_state.id_state.st_IDState.L_BLOCK_FOREVER) {

				throw exception("[game_server::requestLogin][Log] Bloqueado permanente. player [UID=" + std::to_string(pi->uid)
						+ ", ID=" + std::string(pi->id) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1030, 0));

			}else if (pi->block_flag.m_id_state.id_state.st_IDState.L_BLOCK_ALL_IP) {

				snmdb::NormalManagerDB::getInstance().add(9, new CmdInsertBlockIP(_session.m_ip, "255.255.255.255"), game_server::SQLDBResponse, this);

				throw exception("[game_server::requestLogin][Log] Player[UID=" + std::to_string(pi->uid) + ", IP=" + std::string(_session.m_ip)
						+ "] Block ALL IP que o player fizer login.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1031, 0));

			}else if (pi->block_flag.m_id_state.id_state.st_IDState.L_BLOCK_MAC_ADDRESS) {

				snmdb::NormalManagerDB::getInstance().add(10, new CmdInsertBlockMAC(mac_address), game_server::SQLDBResponse, this);

				throw exception("[game_server::requestLogin][Log] Player[UID=" + std::to_string(pi->uid)
						+ ", IP=" + std::string(_session.m_ip) + ", MAC=" + mac_address + "] Block MAC Address que o player fizer login.",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1032, 0));

			}
		}

		de_encrypt_packet_ver(packet_version);

		if (!m_login_manager.canSameIDLogin() && packet_version != PACKET_VERSION_SERVER)
			throw exception("[game_server::requestLogin][Error] Player UID: " + std::to_string(_pi.uid) + ".\tPacket Version of client not match. Server: "
					+ std::to_string(PACKET_VERSION_SERVER) + " == Client: " + std::to_string(packet_version), STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1055, 0));

		CmdAuthKeyLoginInfo cmd_akli(pi->uid, true);

		snmdb::NormalManagerDB::getInstance().add(0, &cmd_akli, nullptr, nullptr);

		cmd_akli.waitEvent();

		if (cmd_akli.getException().getCodeError() != 0)
			throw cmd_akli.getException();

		if (!m_login_manager.canSameIDLogin() && !cmd_akli.getInfo().valid)
			throw exception("[game_server::requestLogin][Error] Player[UID=" + std::to_string(pi->uid) + "].\tAuthKey ja foi utilizada antes.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1056, 0));

		if (!m_login_manager.canSameIDLogin() &&
#if defined(_WIN32)
			_stricmp(kol.keys[0], cmd_akli.getInfo().key) != 0
#elif defined(__linux__)
			strcasecmp(kol.keys[0], cmd_akli.getInfo().key) != 0
#endif
		)
			throw exception("[game_server::requestLogin][Error] Player[UID=" + std::to_string(pi->uid) + "].\tAuthKey no bate(no match).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1057, 0));

		ClientVersion cv_side_sv = ClientVersion::make_version(const_cast<std::string&>(m_login_manager.getClientVersionSideServer()));
		auto cv_side_c = ClientVersion::make_version(client_version);

		if (cv_side_c.flag == ClientVersion::COMPLETE_VERSION && strcmp(cv_side_c.region, cv_side_sv.region) == 0
				&& strcmp(cv_side_c.season, cv_side_sv.season) == 0) {

			if (cv_side_c.high != cv_side_sv.high || cv_side_c.low < cv_side_sv.low) {
				_smp::message_pool::getInstance().push(new message("[game_server::requestLogin][WARNING] Player[UID=" + std::to_string(pi->uid) + "].\tClient Version not match. Server: "
						+ std::string(m_login_manager.getClientVersionSideServer()) + " == Client: " + cv_side_c.toString(), CL_ONLY_FILE_LOG));

				pi->block_flag.m_flag.stBit.all_game = 1u;
			}

		}else if (cv_side_c.high != cv_side_sv.high || cv_side_c.low < cv_side_sv.low) {

			_smp::message_pool::getInstance().push(new message("[game_server::requestLogin][WARNING] Player[UID=" + std::to_string(pi->uid) + "].\tClient Version not match. Server: "
					+ std::string(m_login_manager.getClientVersionSideServer()) + " == Client: " + cv_side_c.toString(), CL_ONLY_FILE_LOG));

			pi->block_flag.m_flag.stBit.all_game = 1u;
		}

		CmdMemberInfo cmd_mi(pi->uid, true);

		snmdb::NormalManagerDB::getInstance().add(0, &cmd_mi, nullptr, nullptr);

		cmd_mi.waitEvent();

		if (cmd_mi.getException().getCodeError() != 0)
			throw cmd_mi.getException();

		_session.m_pi.mi = cmd_mi.getInfo();

		{
			auto oid_cadeira = findReconnectOID(pi->uid);

			if (oid_cadeira != 0u && oid_cadeira != _session.m_oid) {

				auto oid_novo = _session.m_oid;

				if (reclaimOID(oid_cadeira, oid_novo)) {

					_session.m_oid = oid_cadeira;

					_smp::message_pool::getInstance().push(new message("[game_server::requestLogin][Log] player[UID=" + std::to_string(pi->uid)
							+ "] esta RECONECTANDO: logou com o OID=" + std::to_string(oid_novo) + " mas tem uma cadeira guardada com o OID="
							+ std::to_string(oid_cadeira) + ". Devolvendo o OID da cadeira para ele ja no login, para o cliente e o server"
							" concordarem desde o inicio.", CL_FILE_LOG_AND_CONSOLE));

				}else
					_smp::message_pool::getInstance().push(new message("[game_server::requestLogin][WARNING] player[UID=" + std::to_string(pi->uid)
							+ "] tem uma cadeira guardada com o OID=" + std::to_string(oid_cadeira) + " mas nao consegui reservar esse OID"
							" (ocupado por outra session?). Ele vai continuar com o OID=" + std::to_string(oid_novo)
							+ " e a reconexao vai crashar o cliente dele.", CL_FILE_LOG_AND_CONSOLE));
			}
		}

		pi->mi.oid = _session.m_oid;
		pi->mi.state_flag.stFlagBit.visible = 1u;
		pi->mi.state_flag.stFlagBit.whisper = pi->whisper;
		pi->mi.state_flag.stFlagBit.channel = !pi->whisper;

		if (pi->m_cap.stBit.game_master ) {

			_session.m_gi.setGMUID(pi->uid);

			pi->mi.state_flag.stFlagBit.visible = _session.m_gi.visible;
			pi->mi.state_flag.stFlagBit.whisper = _session.m_gi.whisper;
			pi->mi.state_flag.stFlagBit.channel = _session.m_gi.channel;

			_smp::message_pool::getInstance().push(new message("[game_server::requestLogin][Log] [GM][ID=" + std::string(pi->id) + "][UID="
					+ std::to_string(pi->uid) + "][OID=" + std::to_string(_session.m_oid) + "] Logou.", CL_FILE_LOG_AND_CONSOLE));
		}

		if (m_si.propriedade.stBit.only_rookie && pi->level >= 6 )
			throw exception("[game_server::requestLogin][Error] Player[UID=" + std::to_string(pi->uid) + ", LEVEL="
					+ std::to_string((unsigned short)pi->level) + "] nao pode entrar no server por que o server eh so para rookie.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1052, 0));

		if (m_si.propriedade.stBit.mantle && !(pi->m_cap.stBit.mantle || pi->m_cap.stBit.game_master))
			throw exception("[game_server::requestLogin][Error] Player[UID=" + std::to_string(pi->uid) + ", CAP=" + std::to_string(pi->m_cap.ulCapability)
					+ "] nao tem a capacidade para entrar no server mantle.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1053, 0));

		auto player_logado = HasLoggedWithOuterSocket(_session);

		if (!m_login_manager.canSameIDLogin() && player_logado != nullptr) {

			_smp::message_pool::getInstance().push(new message("[game_server::requestLogin][Log] Player[UID=" + std::to_string(_pi.uid) + ", OID="
					+ std::to_string(_session.m_oid) + ", IP=" + _session.getIP() + "] que esta logando agora, ja tem uma outra session com o mesmo UID logado, desloga o outro Player[UID="
					+ std::to_string(player_logado->getUID()) + ", OID=" + std::to_string(player_logado->m_oid) + ", IP=" + player_logado->getIP() + "]", CL_FILE_LOG_AND_CONSOLE));

			if (!DisconnectSession(player_logado))
				throw exception("[game_server::requestLogin][Error] Nao conseguiu disconnectar o player[UID=" + std::to_string(player_logado->getUID())
						+ ", OID=" + std::to_string(player_logado->m_oid) + ", IP=" + player_logado->getIP() + "], ele pode esta com o bug do oid bloqueado, ou Session::UsaCtx bloqueado.",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1054, 0));
		}

		pi->block_flag.m_flag.ullFlag |= m_si.flag.ullFlag;

		_session.m_is_authorized = 1u;

		snmdb::NormalManagerDB::getInstance().add(5, new CmdRegisterLogon(pi->uid, 0 ), game_server::SQLDBResponse, this);

		snmdb::NormalManagerDB::getInstance().add(7, new CmdRegisterLogonServer(pi->uid, std::to_string(m_si.uid)), game_server::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[game_server::requestLogin][Log] Player[OID=" + std::to_string(_session.m_oid) + ", UID=" + std::to_string(pi->uid) + ", NICKNAME="
				+ std::string(pi->nickname) + "] Autenticou com sucesso.", CL_FILE_LOG_AND_CONSOLE));

		sPapelShopSystem::getInstance().init_player_papel_shop_info(_session);

#if _DEBUG
		_smp::message_pool::getInstance().push(new message("[game_server::requestLogin][Log] Enviando os pacotes principais do jogo para o player: " + std::to_string(pi->uid), CL_FILE_LOG_AND_CONSOLE));
#endif

		m_login_manager.createTask(_session, kol, _pi, cv_side_c , this);

		_session.m_tick_bot = std::clock();

		packet p;

		packet_func::pacote044(p, &_session, m_si, 0xD3);

		packet_func::session_send(p, &_session, 0);

		packet_func::sendHmacKey(&_session);
		packet_func::sendChallenge(&_session);

	}catch (exception& e) {

		_session.m_is_authorized = 0u;

		packet p((unsigned short)0x44);

		p.addUint8(0xE2);
		p.addInt32(500020);

		packet_func::session_send(p, &_session, 1);

#if defined(_WIN32)
		::shutdown(_session.m_sock, SD_RECEIVE);
#elif defined(__linux__)
		::shutdown(_session.m_sock.fd, SD_RECEIVE);
#endif

		_smp::message_pool::getInstance().push(new message("[game_server::requestLogin][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::requestEnterChannel(player& _session, packet *_packet) {
	REQUEST_BEGIN("EnterChannel");

	try {

		unsigned char channel = _packet->readUint8();

		CHECK_SESSION_IS_AUTHORIZED("EnterChannel");

		enterChannel(_session, channel);

	}catch (exception& e) {

		packet p;
		packet_func::pacote04E(p, &_session, 6, (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::CHANNEL) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5700052);

		packet_func::session_send(p, &_session, 0);

		_smp::message_pool::getInstance().push(new message("[game_server::requestEnterChannel][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::requestEnterOtherChannelAndLobby(player& _session, packet *_packet) {
	REQUEST_BEGIN("EnterOtherChannelAndLobby");

	try {

		CHECK_SESSION_IS_AUTHORIZED("EnterOtherChannelAndLobby");

		unsigned char lobby = _session.m_pi.lobby;

		auto c = enterChannel(_session, _packet->readUint8());

		if (c != nullptr)
			c->enterLobby(_session, lobby);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestEnterOtherChannelAndLobby][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) != STDA_ERROR_TYPE::CHANNEL)
			throw;
	}
};

void game_server::requestChangeServer(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChangeServer");

	packet p;

	try {

		int32_t server_uid = _packet->readUint32();

		CHECK_SESSION_IS_AUTHORIZED("ChangeServer");

		auto it = std::find_if(m_server_list.begin(), m_server_list.end(), [&](auto& _el) {
			return _el.uid == server_uid;
		});

		if (it == m_server_list.end())
			throw exception("[game_server::requestChangeServer][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou trocar de server para o Server[UID=" + std::to_string(server_uid)
					+ "], mas ele nao esta no server list mais.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 0x7500001, 1));

		if (_session.m_pi.lobby != (unsigned char)~0 && _session.m_pi.lobby == 176u
			&& !it->propriedade.stBit.grand_prix )
			throw exception("[game_server::requestChangeServer][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou trocar de server para o Server[UID=" + std::to_string(server_uid)
					+ "], mas o player esta na lobby grand prix e o server que ele quer entrar nao e' grand prix.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 0x7500002, 2));

		CmdAuthKeyGame cmd_akg(_session.m_pi.uid, server_uid, true);

		snmdb::NormalManagerDB::getInstance().add(0, &cmd_akg, nullptr, nullptr);

		cmd_akg.waitEvent();

		if (cmd_akg.getException().getCodeError() != 0)
			throw cmd_akg.getException();

		auto auth_key_game = cmd_akg.getAuthKey();

		CmdUpdateAuthKeyLogin cmd_uakl(_session.m_pi.uid, 1, true);

		snmdb::NormalManagerDB::getInstance().add(0, &cmd_uakl, nullptr, nullptr);

		cmd_uakl.waitEvent();

		if (cmd_uakl.getException().getCodeError() != 0)
			throw cmd_uakl.getException();

		packet_func::pacote1D4(p, &_session, auth_key_game);
		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[requestChangeServer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		sendServerListAndChannelListToSession(_session);

	}
};

void game_server::requestUCCWebKey(player& _session, packet *_packet) {
	REQUEST_BEGIN("UCCWebKey");

	packet p;

#if defined(__linux__)
#pragma pack(1)
#endif

	struct ctx_UCCWebKey {
		void clear() { memset(this, 0, sizeof(ctx_UCCWebKey)); };
		unsigned char opt;
		uint32_t uid;
		unsigned char seq;
		int32_t item_id;
	};

#if defined(__linux__)
#pragma pack()
#endif

	try {

		ctx_UCCWebKey ctx_uwk{ 0 };

		_packet->readBuffer(&ctx_uwk, sizeof(ctx_UCCWebKey));

		CHECK_SESSION_IS_AUTHORIZED("UCCWebKey");

		if (ctx_uwk.uid == 0)
			throw exception("[game_server::requestUCCWebKey][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou gerar chave web de UCC[ITEM_ID="
					+ std::to_string(ctx_uwk.item_id) + "] do player[UID=" + std::to_string(ctx_uwk.uid) + "], mas o uid do player eh invalid. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5100101));

		if (ctx_uwk.item_id <= 0)
			throw exception("[game_server::requestUCCWebKey][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou gerar chave web de UCC[ITEM_ID="
					+ std::to_string(ctx_uwk.item_id) + "] do player[UID=" + std::to_string(ctx_uwk.uid) + "], mas o item_id is invalid. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 2, 0x5100102));

		player *s = (player*)m_session_manager.findSessionByUID(ctx_uwk.uid);

		if (s == nullptr)
			throw exception("[game_server::requestUCCWebKey][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou gerar chave web de UCC[ITEM_ID="
					+ std::to_string(ctx_uwk.item_id) + "] do player[UID=" + std::to_string(ctx_uwk.uid) + "], mas o player nao esta nesse server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 3, 0x5100103));

		auto pWi = s->m_pi.findWarehouseItemById(ctx_uwk.item_id);

		if (pWi == nullptr)
			throw exception("[game_server::requestUCCWebKey][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou gerar chave web de UCC[ITEM_ID="
					+ std::to_string(ctx_uwk.item_id) + "] do player[UID=" + std::to_string(ctx_uwk.uid) + "], mas o ele nao tem a UCC. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 4, 0x5100104));

		CmdGeraUCCWebKey cmd_guwk(_session.m_pi.uid, pWi->id, true);

		snmdb::NormalManagerDB::getInstance().add(0, &cmd_guwk, nullptr, nullptr);

		cmd_guwk.waitEvent();

		if (cmd_guwk.getException().getCodeError() != 0)
			throw cmd_guwk.getException();

		std::string key = cmd_guwk.getKey();

		_smp::message_pool::getInstance().push(new message("[UCC::SelfDesignSystem::GeraWebKey][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] gerou Web Key[KEY=" + key + "] da UCC[TYPEID="
				+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "] do player[UID=" + std::to_string(ctx_uwk.uid) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x153);

		p.addUint8(0);
		p.addUint8(1);

		p.addInt32(pWi->id);
		p.addString(key);
		p.addUint8(ctx_uwk.seq);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestUCCWebKey][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x153);

		p.addUint8(1);
		p.addUint8(1);
		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::GAME_SERVER) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5100100);

		packet_func::session_send(p, &_session, 1);
	}
};

void game_server::requestUCCSystem(player& _session, packet *_packet) {
	REQUEST_BEGIN("UCCSystem");

	packet p;

	try {

		unsigned char opt = _packet->readUint8();

		CHECK_SESSION_IS_AUTHORIZED("UCCSystem");

		switch (opt) {
		case 0:
		{
			uint32_t ucc_typeid = _packet->readUint32();
			std::string ucc_idx = _packet->readString();
			std::string ucc_name = _packet->readString();

			if (sIff::getInstance().getItemGroupIdentify(ucc_typeid) != iff::PART)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou salvar definitivo a UCC[TYPEID="
					+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "], mas o UCC nao eh um part valido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 13, 0x5200113));

			auto part = sIff::getInstance().findPart(ucc_typeid);

			if (part == nullptr)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou salvar definitivo a UCC[TYPEID="
					+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "], mas nao tem a UCC no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 11, 0x5200111));

			if (part->type_item != IFF::Part::UCC_BLANK && part->type_item != IFF::Part::UCC_COPY)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou salvar definitivo a UCC[TYPEID="
					+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "], mas nao eh uma UCC valida. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 12, 0x5200112));

			if (ucc_typeid == 0)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou salvar definitivo a UCC[TYPEID="
					+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "], mas o typeid is invalid. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 4, 0x5200104));

			if (ucc_idx.empty())
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou salvar definitivo a UCC[TYPEID="
					+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "], mas o idx eh invalido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 6, 0x5200106));

			if (ucc_name.empty())
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou salvar definitivo a UCC[TYPEID="
						+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "], mas o name eh invalido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 7, 0x5200107));

			auto it = std::find_if(_session.m_pi.mp_wi.begin(), _session.m_pi.mp_wi.end(), [&](auto& el) {
				return (el.second._typeid == ucc_typeid && (el.second.ucc.name[0] == '\0' || strcmp(el.second.ucc.name, "0") == 0) && ucc_idx.compare(el.second.ucc.idx) == 0);
			});

			if (it == _session.m_pi.mp_wi.end())
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou salvar definitivo a UCC[TYPEID="
					+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "], mas ele nao tem essa UCC. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 5, 0x5200105));

			it->second.ucc.status = 1;
#if defined(_WIN32)
			strcpy_s(it->second.ucc.name, ucc_name.c_str());
			strcpy_s(it->second.ucc.copier_nick, _session.m_pi.nickname);
#elif defined(__linux__)
			strcpy(it->second.ucc.name, ucc_name.c_str());
			strcpy(it->second.ucc.copier_nick, _session.m_pi.nickname);
#endif
			it->second.ucc.copier = _session.m_pi.uid;

			SYSTEMTIME si{ 0 };

			GetLocalTime(&si);

			CmdUpdateUCC cmd_uu(_session.m_pi.uid, it->second, si, CmdUpdateUCC::FOREVER, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_uu, nullptr, nullptr);

			cmd_uu.waitEvent();

			if (cmd_uu.getException().getCodeError() != 0)
				throw cmd_uu.getException();

			_smp::message_pool::getInstance().push(new message("[UCC::Self Design System][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] salvo definitivo a UCC[TYPEID="
				+ std::to_string(it->second._typeid) + ", ID=" + std::to_string(it->second.id) + ", IDX=" + std::string(it->second.ucc.idx) + ", NAME=" + std::string(it->second.ucc.name) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x12E);

			p.addUint8(opt);

			p.addUint8(1);

			p.addInt32(it->second.id);
			p.addUint32(it->second._typeid);
			p.addString(it->second.ucc.idx);
			p.addString(it->second.ucc.name);

			packet_func::session_send(p, &_session, 1);

			break;
		}
		case 1:
		{
			int32_t ucc_id = _packet->readInt32();
			unsigned char owner = _packet->readUint8();

			WarehouseItemEx wi{ 0 };

			if (ucc_id <= 0)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou ver info da UCC[ID="
						+ std::to_string(ucc_id) + "], mas o id da ucc eh invalido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 2, 0x5200102));

			auto pWi = _session.m_pi.findWarehouseItemById(ucc_id);

			if (pWi == nullptr) {

				CmdFindUCC cmd_fu(ucc_id, true);

				snmdb::NormalManagerDB::getInstance().add(0, &cmd_fu, nullptr, nullptr);

				cmd_fu.waitEvent();

				if (cmd_fu.getException().getCodeError() != 0)
					throw cmd_fu.getException();

				if (cmd_fu.getInfo().id == -1)
					throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou ver info da UCC[ID="
							+ std::to_string(ucc_id) + "], mas nao encontrou essa UCC. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 3, 0x5200103));

				wi = cmd_fu.getInfo();

				pWi = &wi;
			}

#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("[UCC::Self Design System][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] pediu info da ucc[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "]", CL_FILE_LOG_AND_CONSOLE));
#else
			_smp::message_pool::getInstance().push(new message("[UCC::Self Design System][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] pediu info da ucc[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "]", CL_ONLY_FILE_LOG));
#endif

			p.init_plain((unsigned short)0x12E);

			p.addInt8(opt);

			p.addInt32(pWi->_typeid);
			p.addString(pWi->ucc.idx);
			p.addInt8(owner);

			p.addBuffer(pWi, sizeof(WarehouseItem));

			packet_func::session_send(p, &_session, 0);

			break;
		}
		case 2:
		{
			uint32_t ucc_typeid = _packet->readUint32();
			std::string ucc_idx = _packet->readString();
			unsigned short seq = _packet->readUint16();
			int32_t cpy_id = _packet->readInt32();

			IFF::Part *part = nullptr;

			if (sIff::getInstance().getItemGroupIdentify(ucc_typeid) != iff::PART)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou copiar a UCC[TYPEID="
					+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "] para UCC_CPY[ID=" + std::to_string(cpy_id) + "], mas o UCC nao eh um part valido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 13, 0x5200113));

			part = sIff::getInstance().findPart(ucc_typeid);

			if (part == nullptr)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou copiar a UCC[TYPEID="
					+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "] para UCC_CPY[ID=" + std::to_string(cpy_id) + "], mas nao tem a UCC no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 11, 0x5200111));

			if (part->type_item != IFF::Part::UCC_BLANK && part->type_item != IFF::Part::UCC_COPY)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou copiar a UCC[TYPEID="
					+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "] para UCC_CPY[ID=" + std::to_string(cpy_id) + "], mas nao eh uma UCC valida. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 12, 0x5200112));

			if (ucc_typeid == 0)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou copiar a UCC[TYPEID="
					+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "] para UCC_CPY[ID=" + std::to_string(cpy_id) + "], mas o typeid is invalid. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 4, 0x5200104));

			if (ucc_idx.empty())
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou copiar a UCC[TYPEID="
					+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "] para UCC_CPY[ID=" + std::to_string(cpy_id) + "], mas o idx eh invalido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 6, 0x5200106));

			if (seq == 0)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou copiar a UCC[TYPEID="
						+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "] para UCC_CPY[ID=" + std::to_string(cpy_id) + "], mas seq[value=" + std::to_string(seq) + "] is invalid. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 8, 0x5200108));

			if (cpy_id <= 0)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou copiar a UCC[TYPEID="
						+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "] para UCC_CPY[ID=" + std::to_string(cpy_id) + "], mas o copy_id is invalid. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 9, 0x5200109));

			auto pWi = _session.m_pi.findWarehouseItemById(cpy_id);

			if (pWi == nullptr)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou copiar a UCC[TYPEID="
						+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "] para UCC_CPY[ID=" + std::to_string(cpy_id) + "], mas o ele nao tem a UCC_CPY, Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 10, 0x5200110));

			if (sIff::getInstance().getItemGroupIdentify(pWi->_typeid) != iff::PART)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou copiar a UCC[TYPEID="
						+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "] para UCC_CPY[ID=" + std::to_string(cpy_id) + "], mas o UCC nao eh um part valido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 13, 0x5200113));

			part = sIff::getInstance().findPart(pWi->_typeid);

			if (part == nullptr)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou copiar a UCC[TYPEID="
						+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "] para UCC_CPY[ID=" + std::to_string(cpy_id) + "], mas nao tem a UCC no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 11, 0x5200111));

			if (part->type_item != IFF::Part::UCC_BLANK && part->type_item != IFF::Part::UCC_COPY)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou copiar a UCC[TYPEID="
						+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "] para UCC_CPY[ID=" + std::to_string(cpy_id) + "], mas nao eh uma UCC valida. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 12, 0x5200112));

			auto it = std::find_if(_session.m_pi.mp_wi.begin(), _session.m_pi.mp_wi.end(), [&](auto& el) {
				return (el.second._typeid == ucc_typeid && (el.second.ucc.name[0] != '\0' && strcmp(el.second.ucc.name, "0") != 0) && ucc_idx.compare(el.second.ucc.idx) == 0);
			});

			if (it == _session.m_pi.mp_wi.end())
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou copiar a UCC[TYPEID="
						+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "], mas ele nao tem essa UCC. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 5, 0x5200105));

			pWi->ucc.status = 1;
#if defined(_WIN32)
			strcpy_s(pWi->ucc.idx, sizeof(pWi->ucc.idx), it->second.ucc.idx);
			strcpy_s(pWi->ucc.name, sizeof(pWi->ucc.name), it->second.ucc.name);
			strcpy_s(pWi->ucc.copier_nick, sizeof(pWi->ucc.copier_nick), _session.m_pi.nickname);
#elif defined(__linux__)
			strcpy(pWi->ucc.idx, it->second.ucc.idx);
			strcpy(pWi->ucc.name, it->second.ucc.name);
			strcpy(pWi->ucc.copier_nick, _session.m_pi.nickname);
#endif
			pWi->ucc.copier = _session.m_pi.uid;

			SYSTEMTIME draw_dt{ 0 };

			GetLocalTime(&draw_dt);

			CmdUpdateUCC cmd_uu(_session.m_pi.uid, *pWi, draw_dt, CmdUpdateUCC::COPY, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_uu, nullptr, nullptr);

			cmd_uu.waitEvent();

			if (cmd_uu.getException().getCodeError() != 0)
				throw cmd_uu.getException();

			(*pWi) = cmd_uu.getInfo();

			_smp::message_pool::getInstance().push(new message("[UCC::Self Design System][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] fez um copia da UCC[TYPEID="
					+ std::to_string(it->second._typeid) + ", ID=" + std::to_string(it->second.id) + ", IDX=" + std::string(it->second.ucc.idx) + "] na UCC_CPY[TYPEID="
					+ std::to_string(pWi->_typeid) + ", ID=" + std::to_string(pWi->id) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x12E);

			p.addUint8(opt);

			p.addUint32(it->second._typeid);
			p.addString(it->second.ucc.idx);
			p.addUint16(it->second.ucc.seq);

			p.addInt32(pWi->id);
			p.addInt32(pWi->id);
			p.addUint32(pWi->_typeid);
			p.addString(pWi->ucc.idx);
			p.addUint16(pWi->ucc.seq);

			p.addUint8(1);

			packet_func::session_send(p, &_session, 1);

			break;
		}
		case 3:
		{
			uint32_t ucc_typeid = _packet->readUint32();
			std::string ucc_idx = _packet->readString();

			if (sIff::getInstance().getItemGroupIdentify(ucc_typeid) != iff::PART)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou salvar temporario a UCC[TYPEID="
					+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "], mas o UCC nao eh um part valido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 13, 0x5200113));

			auto part = sIff::getInstance().findPart(ucc_typeid);

			if (part == nullptr)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou salvar temporario a UCC[TYPEID="
					+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "], mas nao tem a UCC no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 11, 0x5200111));

			if (part->type_item != IFF::Part::UCC_BLANK && part->type_item != IFF::Part::UCC_COPY)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou salvar temporario a UCC[TYPEID="
					+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "], mas nao eh uma UCC valida. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 12, 0x5200112));

			if (ucc_typeid == 0)
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou salvar temporario a UCC[TYPEID="
						+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "], mas o typeid is invalid. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 4, 0x5200104));

			if (ucc_idx.empty())
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou salvar temporario a UCC[TYPEID="
						+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "], mas o idx eh invalido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 6, 0x5200106));

			auto it = std::find_if(_session.m_pi.mp_wi.begin(), _session.m_pi.mp_wi.end(), [&](auto& el) {
				return (el.second._typeid == ucc_typeid && (el.second.ucc.name[0] == '\0' || strcmp(el.second.ucc.name, "0") == 0) && ucc_idx.compare(el.second.ucc.idx) == 0);
			});

			if (it == _session.m_pi.mp_wi.end())
				throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou salvar temporario a UCC[TYPEID="
						+ std::to_string(ucc_typeid) + ", IDX=" + ucc_idx + "], mas ele nao tem essa UCC. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 5, 0x5200105));

			it->second.ucc.status = 2;
#if defined(_WIN32)
			strcpy_s(it->second.ucc.name, sizeof(it->second.ucc.name), "0");
#elif defined(__linux__)
			strcpy(it->second.ucc.name, "0");
#endif
			SYSTEMTIME si{ 0 };

			CmdUpdateUCC cmd_uu(_session.m_pi.uid, it->second, si, CmdUpdateUCC::TEMPORARY, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_uu, nullptr, nullptr);

			cmd_uu.waitEvent();

			if (cmd_uu.getException().getCodeError() != 0)
				throw cmd_uu.getException();

			_smp::message_pool::getInstance().push(new message("[UCC::Self Design System][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] salvo temporario a UCC[TYPEID="
					+ std::to_string(it->second._typeid) + ", ID=" + std::to_string(it->second.id) + ", IDX=" + std::string(it->second.ucc.idx) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x12E);

			p.addUint8(opt);

			p.addUint32(it->second._typeid);
			p.addString(it->second.ucc.idx);
			p.addUint8(1);

			packet_func::session_send(p, &_session, 1);

			break;
		}
		default:
			throw exception("[game_server::requestUCCSystem][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou usar UCC System, mas forneceu uma option desconhecida. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5200101));
		}
	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestUCCSystem][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x12E);

		p.addInt8(-1);

		packet_func::session_send(p, &_session, 1);
	}
};

void game_server::requestChat(player& _session, packet *_packet) {
	REQUEST_BEGIN("Chat");

	packet p;

	try {

		std::string nickname, msg;

		nickname = _packet->readString();
		msg = _packet->readString();

	#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("Player UID: " + std::to_string(_session.m_pi.uid) + ".\tNickname: " + nickname, CL_ONLY_FILE_LOG));
		_smp::message_pool::getInstance().push(new message("Player UID: " + std::to_string(_session.m_pi.uid) + ".\tmessage: " + msg, CL_ONLY_FILE_LOG));
	#endif

		CHECK_SESSION_IS_AUTHORIZED("Chat");

		channel *c = findChannel(_session.m_pi.channel);

		if (c != nullptr) {

			auto gm = m_player_manager.findAllGM();

			if (!gm.empty()) {

				std::string msg_gm = "\\5" + std::string(_session.m_pi.nickname) + ": '" + msg + "'";
				std::string from = "\\1[Channel=" + std::string(c->getInfo()->name) + ", \\1ROOM=" + std::to_string(_session.m_pi.mi.sala_numero) + "]";

				auto index = from.find(' ');

				if (index != std::string::npos)
					from.replace(index, 1, " \\1");

				for (auto& el : gm) {
					if (((el->m_gi.channel &&  el->m_pi.channel == c->getInfo()->id) || el->m_gi.whisper || el->m_gi.isOpenPlayerWhisper(_session.m_pi.uid))
						&&  (el->m_pi.channel != _session.m_pi.channel || el->m_pi.mi.sala_numero != _session.m_pi.mi.sala_numero)) {

						p.init_plain((unsigned short)0x40);

						p.addUint8(0);

						p.addString(from);

						p.addString(msg_gm);

						packet_func::session_send(p, el, 1);
					}
				}
			}

			if (_session.m_pi.mi.sala_numero != -1)
				c->requestSendMsgChatRoom(_session, msg);
			else {
				packet p;
				packet_func::pacote040(p, &_session, &_session.m_pi, msg, (_session.m_pi.m_cap.stBit.game_master ) ? 0x80 : 0);
				packet_func::lobby_broadcast(*c, p, 0);
			}

			if (m_si.rate.smart_calculator && m_chat_discord)
				sendMessageToDiscordChatHistory(
					"[Channel=" + std::string(c->getInfo()->name) + ", ROOM=" + std::to_string(_session.m_pi.mi.sala_numero) + "]",
					std::string(_session.m_pi.nickname) + ": '" + msg + "'"
				);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestChat][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x40);

		p.addUint8((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::GAME_SERVER) ? (unsigned char)STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 1u);

		p.addUint16(0);

		packet_func::session_send(p, &_session, 1);
	}
};

void game_server::requestChangeChatMacroUser(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChangeChatMacroUser");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ChangeChatMacroUser");

		chat_macro_user cmu{ 0 };

		_packet->readBuffer(&cmu, sizeof(chat_macro_user));

		for (auto i = 0u; i < (sizeof(cmu.macro) / sizeof(cmu.macro[0])); ++i)
			if (std::empty(cmu.macro[i]) || cmu.macro[i][0] == '\0')
#if defined(_WIN32)
				memcpy_s(cmu.macro[i], sizeof(cmu.macro[i]), "PangYa SuperSS! Por favor configure seu chat macro", 51);
#elif defined(__linux__)
				memcpy(cmu.macro[i], "PangYa SuperSS! Por favor configure seu chat macro", 51);
#endif

		_session.m_pi.cmu = cmu;

		snmdb::NormalManagerDB::getInstance().add(3, new CmdUpdateChatMacroUser(_session.m_pi.uid, _session.m_pi.cmu), game_server::SQLDBResponse, this);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestChangeChatMacroUser][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::requestPlayerInfo(player& _session, packet *_packet) {
	REQUEST_BEGIN("PlayerInfo");

	packet p;

	try {

		uint32_t uid = _packet->readInt32();
		unsigned char season = _packet->readInt8();

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("Player UID: " + std::to_string(_session.m_pi.uid) + ".\tPlayer Info request uid: " + std::to_string(uid) + "\tseason: " + std::to_string((int)season), CL_ONLY_FILE_LOG));
#endif

		CHECK_SESSION_IS_AUTHORIZED("PlayerInfo");

		player *s = nullptr;
		PlayerInfo *pi = nullptr;
		CharacterInfo ci;

		if (uid == _session.m_pi.uid) {

			pi = &_session.m_pi;

		}else if ((s = findPlayer(uid)) != nullptr) {

			pi = &s->m_pi;

		}else {

			CmdMemberInfo cmd_mi(uid, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_mi, nullptr, nullptr);

			cmd_mi.waitEvent();

			if (cmd_mi.getException().getCodeError() != 0)
				throw cmd_mi.getException();

			MemberInfoEx mi = cmd_mi.getInfo();

			if (uid != _session.m_pi.uid && !_session.m_pi.m_cap.stBit.game_master && mi.capability.stBit.game_master ) {

				packet_func::pacote089(p, &_session, uid, season, 3);
				packet_func::session_send(p, &_session, 1);

			}else {

				std::vector< MapStatistics > v_ms_n, v_msa_n, v_ms_na, v_msa_na, v_ms_g, v_msa_g;

				CmdCharacterInfo cmd_ci(uid, CmdCharacterInfo::ONE, -1, true);

				snmdb::NormalManagerDB::getInstance().add(0, &cmd_ci, nullptr, nullptr);

				cmd_ci.waitEvent();

				if (cmd_ci.getException().getCodeError() != 0)
					throw cmd_ci.getException();

				ci = cmd_ci.getInfo();

				CmdUserEquip cmd_ue(uid, true);

				snmdb::NormalManagerDB::getInstance().add(0, &cmd_ue, nullptr, nullptr);

				cmd_ue.waitEvent();

				if (cmd_ue.getException().getCodeError() != 0)
					throw cmd_ue.getException();

				UserEquip ue = cmd_ue.getEquip();

				CmdUserInfo cmd_ui(uid, true);

				snmdb::NormalManagerDB::getInstance().add(0, &cmd_ui, nullptr, nullptr);

				cmd_ui.waitEvent();

				if (cmd_ui.getException().getCodeError() != 0)
					throw cmd_ui.getException();

				UserInfoEx ui = cmd_ui.getInfo();

				CmdGuildInfo cmd_gi(uid, 0, true);

				snmdb::NormalManagerDB::getInstance().add(0, &cmd_gi, nullptr, nullptr);

				cmd_gi.waitEvent();

				if (cmd_gi.getException().getCodeError() != 0)
					throw cmd_gi.getException();

				auto gi = cmd_gi.getInfo();

				CmdMapStatistics cmd_ms(uid, CmdMapStatistics::TYPE_SEASON(season), CmdMapStatistics::NORMAL, CmdMapStatistics::M_NORMAL, true);

				snmdb::NormalManagerDB::getInstance().add(0, &cmd_ms, nullptr, nullptr);

				cmd_ms.waitEvent();

				if (cmd_ms.getException().getCodeError() != 0)
					throw cmd_ms.getException();

				v_ms_n = cmd_ms.getMapStatistics();

				cmd_ms.setType(CmdMapStatistics::ASSIST);

				snmdb::NormalManagerDB::getInstance().add(0, &cmd_ms, nullptr, nullptr);

				cmd_ms.waitEvent();

				if (cmd_ms.getException().getCodeError() != 0)
					throw cmd_ms.getException();

				v_msa_n = cmd_ms.getMapStatistics();

				cmd_ms.setType(CmdMapStatistics::NORMAL);
				cmd_ms.setModo(CmdMapStatistics::M_NATURAL);

				snmdb::NormalManagerDB::getInstance().add(0, &cmd_ms, nullptr, nullptr);

				cmd_ms.waitEvent();

				if (cmd_ms.getException().getCodeError() != 0)
					throw cmd_ms.getException();

				v_ms_na = cmd_ms.getMapStatistics();

				cmd_ms.setType(CmdMapStatistics::ASSIST);

				snmdb::NormalManagerDB::getInstance().add(0, &cmd_ms, nullptr, nullptr);

				cmd_ms.waitEvent();

				if (cmd_ms.getException().getCodeError() != 0)
					throw cmd_ms.getException();

				v_msa_na = cmd_ms.getMapStatistics();

				cmd_ms.setType(CmdMapStatistics::NORMAL);
				cmd_ms.setModo(CmdMapStatistics::M_GRAND_PRIX);

				snmdb::NormalManagerDB::getInstance().add(0, &cmd_ms, nullptr, nullptr);

				cmd_ms.waitEvent();

				if (cmd_ms.getException().getCodeError() != 0)
					throw cmd_ms.getException();

				v_ms_g = cmd_ms.getMapStatistics();

				cmd_ms.setType(CmdMapStatistics::ASSIST);

				snmdb::NormalManagerDB::getInstance().add(0, &cmd_ms, nullptr, nullptr);

				cmd_ms.waitEvent();

				if (cmd_ms.getException().getCodeError() != 0)
					throw cmd_ms.getException();

				v_msa_g = cmd_ms.getMapStatistics();

				CmdTrophySpecial cmd_tei(uid, CmdTrophySpecial::TYPE_SEASON(season), CmdTrophySpecial::NORMAL, true);

				snmdb::NormalManagerDB::getInstance().add(0, &cmd_tei, nullptr, nullptr);

				cmd_tei.waitEvent();

				if (cmd_tei.getException().getCodeError() != 0)
					throw cmd_tei.getException();

				std::vector< TrofelEspecialInfo > v_tei = cmd_tei.getInfo();

				CmdTrofelInfo cmd_ti(uid, CmdTrofelInfo::TYPE_SEASON(season), true);

				snmdb::NormalManagerDB::getInstance().add(0, &cmd_ti, nullptr, nullptr);

				cmd_ti.waitEvent();

				if (cmd_ti.getException().getCodeError() != 0)
					throw cmd_ti.getException();

				TrofelInfo ti = cmd_ti.getInfo();

				cmd_tei.setType(CmdTrophySpecial::GRAND_PRIX);

				snmdb::NormalManagerDB::getInstance().add(0, &cmd_tei, nullptr, nullptr);

				cmd_tei.waitEvent();

				if (cmd_tei.getException().getCodeError() != 0)
					throw cmd_tei.getException();

				std::vector< TrofelEspecialInfo > v_tegi = cmd_tei.getInfo();

				packet_func::pacote157(p, &_session, mi, season);
				packet_func::session_send(p, &_session, 1);
				packet_func::pacote15E(p, &_session, uid, ci);
				packet_func::session_send(p, &_session, 1);
				packet_func::pacote156(p, &_session, uid, ue, season);
				packet_func::session_send(p, &_session, 1);
				packet_func::pacote158(p, &_session, uid, ui, season);
				packet_func::session_send(p, &_session, 1);
				packet_func::pacote15D(p, &_session, uid, gi);
				packet_func::session_send(p, &_session, 1);
				packet_func::pacote15C(p, &_session, uid, v_ms_na, v_msa_na, (season != 0) ? 0x33 : 0x0A);
				packet_func::session_send(p, &_session, 1);
				packet_func::pacote15C(p, &_session, uid, v_ms_g, v_msa_g, (season != 0) ? 0x34 : 0x0B);
				packet_func::session_send(p, &_session, 1);
				packet_func::pacote15B(p, &_session, uid, season);
				packet_func::session_send(p, &_session, 1);
				packet_func::pacote15A(p, &_session, uid, v_tei, season);
				packet_func::session_send(p, &_session, 1);
				packet_func::pacote159(p, &_session, uid, ti, season);
				packet_func::session_send(p, &_session, 1);
				packet_func::pacote15C(p, &_session, uid, v_ms_n, v_msa_n, season);
				packet_func::session_send(p, &_session, 1);
				packet_func::pacote257(p, &_session, uid, v_tegi, season);
				packet_func::session_send(p, &_session, 1);

				packet_func::pacote089(p, &_session, uid, season);
				packet_func::session_send(p, &_session, 1);
			}

			return;
		}

		if (uid != _session.m_pi.uid && !_session.m_pi.m_cap.stBit.game_master && pi->m_cap.stBit.game_master ) {

			packet_func::pacote089(p, &_session, uid, season, 3);
			packet_func::session_send(p, &_session, 1);

		}else {

			auto pCi = pi->findCharacterById(pi->ue.character_id);

			if (pCi != nullptr)
				ci = *pCi;

			std::vector< MapStatistics > v_ms_n, v_msa_n, v_ms_na, v_msa_na, v_ms_g, v_msa_g;

			for (size_t i = 0; i < MS_NUM_MAPS; ++i)
				if (pi->a_ms_normal[i].best_score != 127)
					v_ms_n.push_back(pi->a_ms_normal[i]);

			for (size_t i = 0; i < MS_NUM_MAPS; ++i)
				if (pi->a_msa_normal[i].best_score != 127)
					v_msa_n.push_back(pi->a_msa_normal[i]);

			for (size_t i = 0; i < MS_NUM_MAPS; ++i)
				if (pi->a_ms_natural[i].best_score != 127)
					v_ms_na.push_back(pi->a_ms_natural[i]);

			for (size_t i = 0; i < MS_NUM_MAPS; ++i)
				if (pi->a_msa_natural[i].best_score != 127)
					v_msa_na.push_back(pi->a_msa_natural[i]);

			for (size_t i = 0; i < MS_NUM_MAPS; ++i)
				if (pi->a_ms_grand_prix[i].best_score != 127)
					v_ms_g.push_back(pi->a_ms_grand_prix[i]);

			for (size_t i = 0; i < MS_NUM_MAPS; ++i)
				if (pi->a_msa_grand_prix[i].best_score != 127)
					v_msa_g.push_back(pi->a_msa_grand_prix[i]);

			packet_func::pacote157(p, &_session, pi->mi, season);
			packet_func::session_send(p, &_session, 1);
			packet_func::pacote15E(p, &_session, pi->uid, ci);
			packet_func::session_send(p, &_session, 1);
			packet_func::pacote156(p, &_session, pi->uid, pi->ue, season);
			packet_func::session_send(p, &_session, 1);
			packet_func::pacote158(p, &_session, pi->uid, pi->ui, season);
			packet_func::session_send(p, &_session, 1);
			packet_func::pacote15D(p, &_session, pi->uid, pi->gi);
			packet_func::session_send(p, &_session, 1);
			packet_func::pacote15C(p, &_session, pi->uid, v_ms_na, v_msa_na, (season != 0) ? 0x33 : 0x0A);
			packet_func::session_send(p, &_session, 1);
			packet_func::pacote15C(p, &_session, pi->uid, v_ms_g, v_msa_g, (season != 0) ? 0x34 : 0x0B);
			packet_func::session_send(p, &_session, 1);
			packet_func::pacote15B(p, &_session, uid, season);
			packet_func::session_send(p, &_session, 1);
			packet_func::pacote15A(p, &_session, pi->uid, (season != 0) ? pi->v_tsi_current_season : pi->v_tsi_rest_season, season);
			packet_func::session_send(p, &_session, 1);
			packet_func::pacote159(p, &_session, pi->uid, (season != 0) ? pi->ti_current_season : pi->ti_rest_season, season);
			packet_func::session_send(p, &_session, 1);
			packet_func::pacote15C(p, &_session, pi->uid, v_ms_n, v_msa_n, season);
			packet_func::session_send(p, &_session, 1);
			packet_func::pacote257(p, &_session, pi->uid, (season != 0) ? pi->v_tgp_current_season : pi->v_tgp_rest_season, season);
			packet_func::session_send(p, &_session, 1);

			packet_func::pacote089(p, &_session, uid, season);
			packet_func::session_send(p, &_session, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestPlayerInfo][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x89);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::GAME_SERVER) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0 );

		packet_func::session_send(p, &_session, 1);
	}

};

void game_server::requestPrivateMessage(player& _session, packet *_packet) {
	REQUEST_BEGIN("PrivateMessage");

	packet p;
	player *s = nullptr;
	std::string nickname;

	try {

		CHECK_SESSION_IS_AUTHORIZED("PrivateMessage");

		nickname = _packet->readString();
		std::string msg = _packet->readString();

		if (nickname.empty())
			throw exception("[game_server::requestPrivateMessage][Error] player[UID=" + std::to_string(_session.m_pi.uid) +"] tentou enviar message privada[msg=" + msg + "] para o player[NICKNAME="
					+ nickname + "], mas o nick esta vazio. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 5));

		if (msg.empty())
			throw exception("[game_server::requestPrivateMessage][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar message privada[msg=" + msg + "] para o player[NICKNAME="
				+ nickname + "], mas message esta vazia. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 4, 5));

		if (nickname.compare("#GC") == 0) {

			if (checkGhostCalcCmd(_session, msg))
				return;
		}

		if (nickname.compare("#BET") == 0) {

			if (checkBetCmd(_session, msg))
				return;
		}

		s = reinterpret_cast< player* >(m_session_manager.findSessionByNickname(nickname));

		if (s == nullptr || !s->getState() || !s->isConnected())
			throw exception("[game_server::requestPrivateMessage][WARNING] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar message privada[msg=" + msg + "] para o player[NICKNAME="
					+ nickname + "], mas o player nao esta online nesse server.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 2, 5));

		if (!s->m_pi.whisper)
			throw exception("[game_server::requestPrivateMessage][WARNING] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar message privada[msg=" + msg + "] para o player[NICKNAME="
					+ nickname + "], mas o whisper do player esta bloqueado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 5, 4));

		if ((s->m_pi.lobby == (unsigned char)~0  && s->m_pi.mi.sala_numero == -1 ) || s->m_pi.place == 2)
			throw exception("[game_server::requestPrivateMessage][WARNING] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar message privada[msg=" + msg + "] para o player[NICKNAME="
					+ nickname + "], mas o player nao pode receber message agora, por que nao pode ver o chat. pode estar no Papel Shop e Etc.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 3, 4));

		auto gm = m_player_manager.findAllGM();

		if (!gm.empty()) {

			std::string msg_gm = "\\5" + std::string(_session.m_pi.nickname) + ">" + std::string(s->m_pi.nickname) + ": '" + msg + "'";

			for (auto& el : gm) {
				if ((el->m_gi.whisper || el->m_gi.isOpenPlayerWhisper(_session.m_pi.uid) || el->m_gi.isOpenPlayerWhisper(s->m_pi.uid))
					&&  (el->m_pi.uid != _session.m_pi.uid && el->m_pi.uid != s->m_pi.uid)) {

					p.init_plain((unsigned short)0x40);

					p.addUint8(0);

					p.addString("\\1[PM]");

					p.addString(msg_gm);

					packet_func::session_send(p, el, 1);
				}
			}
		}

		_smp::message_pool::getInstance().push(new message("[PrivateMessage][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] enviou a Message[" + msg + "] para o player[UID=" + std::to_string(s->m_pi.uid) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x84);

		p.addUint8(0);

		p.addString(s->m_pi.nickname);
		p.addString(msg);

		packet_func::session_send(p, &_session, 1);

		p.init_plain((unsigned short)0x84);

		p.addUint8(1);

		p.addString(_session.m_pi.nickname);
		p.addString(msg);

		packet_func::session_send(p, s, 1);

		if (m_si.rate.smart_calculator && m_chat_discord)
			sendMessageToDiscordChatHistory(
				"[PM]",
				std::string(_session.m_pi.nickname) + ">" + std::string(s->m_pi.nickname) + ": '" + msg + "'"
			);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestPrivateMessage][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x40);

		p.addUint8((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::GAME_SERVER) ? (unsigned char)STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 5u);

		if (s != nullptr &&
#if defined(_WIN32)
			s->m_sock != INVALID_SOCKET
#elif defined(__linux__)
			s->m_sock.fd != INVALID_SOCKET
#endif
		)
			p.addString(s->m_pi.nickname);
		else
			p.addString(nickname);

		packet_func::session_send(p, &_session, 1);
	}
};

void game_server::requestChangeWhisperState(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChangeWisperState");

	try {

		auto whisper = _packet->readUint8();

		CHECK_SESSION_IS_AUTHORIZED("ChangeWisperState");

		if (whisper > 1)
			throw exception("[game_server::requestChangeWhisperState][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou alterar o estado do Whisper[state="
					+ std::to_string((unsigned short)whisper) + "], mas ele mandou um valor invalido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5300101));

		_session.m_pi.mi.state_flag.stFlagBit.whisper = _session.m_pi.whisper = whisper;

		_smp::message_pool::getInstance().push(new message("[Whisper::ChangeState][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] trocou o Whisper State para : " + (whisper ? std::string("ON") : std::string("OFF")), CL_FILE_LOG_AND_CONSOLE));

	}catch(exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestChangeWhisperState][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::requestNotifyNotDisplayPrivateMessageNow(player& _session, packet *_packet) {
	REQUEST_BEGIN("NotifyNotDisplayPrivateMessageNow");

	try {

		CHECK_SESSION_IS_AUTHORIZED("requestNotifyNotDisplayPrivateMessageNow");

		std::string nickname = _packet->readString();

		if (nickname.empty())
			throw exception("[game_server::requestNotifyNotDisplayPrivateMessageNow][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] nao pode ver mensagem agora, mas o nickname de quem enviou a mensagem para ele eh invalido(empty). Hacker ou Bug.",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 0x750050, 0));

		auto s = reinterpret_cast< player* >(m_player_manager.findSessionByNickname(nickname));

		if (s != nullptr &&
#if defined(_WIN32)
			s->m_sock != INVALID_SOCKET
#elif defined(__linux__)
			s->m_sock.fd != INVALID_SOCKET
#endif
		) {

			_smp::message_pool::getInstance().push(new message("[game_server::requestNotifyNotDisplayPrivateMessageNow][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] recebeu mensagem do Player[UID=" + std::to_string(s->m_pi.uid) + ", NICKNAME=" + nickname + "], mas ele nao pode ver a mensagem agora.", CL_FILE_LOG_AND_CONSOLE));

			packet p((unsigned short)0x40);

			p.addUint8(4u);

			p.addString(_session.m_pi.nickname);

			packet_func::session_send(p, s, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestNotifyNotDisplayPrivateMessageNow][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::requestCommonCmdGM(player& _session, packet *_packet) {
	REQUEST_BEGIN("CommonCmdGM");

	enum COMMON_CMD_GM {
		CCG_VISIBLE = 3,
		CCG_WHISPER,
		CCG_CHANNEL,
		CCG_OPEN_WHISPER_PLAYER_LIST = 8,
		CCG_CLOSE_WHISPER_PLAYER_LIST,
		CCG_KICK,
		CCG_DISCONNECT,
		CCG_DESTROY = 13,
		CCG_CHANGE_WIND_VERSUS,
		CCG_CHANGE_WEATHER,
		CCG_IDENTITY,
		CCG_GIVE_ITEM = 18,
		CCG_GOLDENBELL,
	};

	packet p;

	try {

		unsigned short cmd = _packet->readUint16();

		CHECK_SESSION_IS_AUTHORIZED("CommonCmdGM");

		switch (cmd) {
		case CCG_VISIBLE:
		{

			if (!(_session.m_pi.m_cap.stBit.game_master ))
				throw exception("[game_server::requestCommonCmdGM][VISIBLE][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao eh GM mas tentou executar comando GM. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5700100));

			auto c = findChannel(_session.m_pi.channel);

			if (c == nullptr)
				throw exception("[game_server::requestCommonCmdGM][VISIBLE][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou executar o comando /visible mas ele nao esta em nenhum canal. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 8, 0x5700100));

			c->requestExecCCGVisible(_session, _packet);

			break;
		}
		case CCG_WHISPER:
		{
			unsigned short whisper = _packet->readUint16();

			if (!(_session.m_pi.m_cap.stBit.game_master ))
				throw exception("[game_server::requestCommonCmdGM][WHISPER][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao eh GM mas tentou executar comando GM. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5700100));

			_session.m_gi.whisper = _session.m_pi.mi.state_flag.stFlagBit.whisper = (whisper & 2) >> 1;

			_session.m_gi.channel = !_session.m_gi.whisper;

			_smp::message_pool::getInstance().push(new message("[game_server::requestCommonCmdGM][WHISPER][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] trocou WHISPER STATUS[STATE="
					+ (whisper & 2 ? std::string("ON") : std::string("OFF")) + "]", CL_FILE_LOG_AND_CONSOLE));

			break;
		}
		case CCG_CHANNEL:
		{
			unsigned short channel = _packet->readUint16();

			if (!(_session.m_pi.m_cap.stBit.game_master ))
				throw exception("[game_server::requestCommonCmdGM][CHANNEL][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao eh GM mas tentou executar comando GM. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5700100));

			_session.m_gi.channel = _session.m_pi.mi.state_flag.stFlagBit.channel = (channel & 4) >> 2;

			_session.m_gi.whisper = !_session.m_gi.channel;

			_smp::message_pool::getInstance().push(new message("[game_server::requestCommonCmdGM][CHANNEL][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] trocou CHANNEL STATUS[STATE="
					+ (channel & 4 ? std::string("ON") : std::string("OFF")) + "]", CL_FILE_LOG_AND_CONSOLE));

			break;
		}
		case CCG_OPEN_WHISPER_PLAYER_LIST:
		{
			std::string nickname = _packet->readString();

			if (nickname.empty())
				throw exception("[game_server::requestCommonCmdGM][OPEN_WHISPER_PLAYER_LIST][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou add um player a lista de Whisper, mas o nickname esta vazio. Hacker ou Bug",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 8, 0x5700108));

			if (!(_session.m_pi.m_cap.stBit.game_master ))
				throw exception("[game_server::requestCommonCmdGM][OPEN_WHISPER_PLAYER_LIST][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao eh GM mas tentou executar comando GM. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5700100));

			auto s = (player*)m_player_manager.findSessionByNickname(nickname);

			if (s == nullptr)
				throw exception("[game_server::requestCommonCmdGM][OPEN_WHISPER_PLAYER_LIST][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou add um player a lista de Whisper, mas nao encontrou o nickname no Server.",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 9, 0x5700109));

			_session.m_gi.openPlayerWhisper(s->m_pi.uid);

			_smp::message_pool::getInstance().push(new message("[game_server::request::CommonCmdGM][OPEN_WHISPER_PLAYER_LIST][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] add player[NICKNAME=" + nickname + "] ao Whisper list.", CL_FILE_LOG_AND_CONSOLE));

			break;
		}
		case CCG_CLOSE_WHISPER_PLAYER_LIST:
		{
			std::string nickname = _packet->readString();

			if (nickname.empty())
				throw exception("[game_server::requestCommonCmdGM][CLOSE_WHISPER_PLAYER_LIST][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar um player da lista de Whisper, mas o nickname esta vazio. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 8, 0x5700108));

			if (!(_session.m_pi.m_cap.stBit.game_master ))
				throw exception("[game_server::requestCommonCmdGM][CLOSE_WHISPER_PLAYER_LIST][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao eh GM mas tentou executar comando GM. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5700100));

			auto s = (player*)m_player_manager.findSessionByNickname(nickname);

			if (s == nullptr)
				throw exception("[game_server::requestCommonCmdGM][CLOSE_WHISPER_PLAYER_LIST][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar um player da lista de Whisper, mas nao encontrou o nickname no Server.",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 9, 0x5700109));

			_session.m_gi.closePlayerWhisper(s->m_pi.uid);

			_smp::message_pool::getInstance().push(new message("[game_server::requestCommonCmdGM][CLOSE_WHISPER_PLAYER_LIST][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] deletou player[NICKNAME=" + nickname + "] do Whisper list.", CL_FILE_LOG_AND_CONSOLE));

			break;
		}
		case CCG_KICK:
		{
			if (!(_session.m_pi.m_cap.stBit.game_master ))
				throw exception("[game_server::requestCommonCmdGM][KICK][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao eh GM mas tentou executar comando GM. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5700100));

			auto c = findChannel(_session.m_pi.channel);

			if (c == nullptr)
				throw exception("[game_server::requestCommonCmdGM][KICK][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou executar o comando /kick mas ele nao esta em nenhum canal. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 8, 0x5700100));

			c->requestExecCCGKick(_session, _packet);

			break;
		}
		case CCG_DISCONNECT:
		{
			if (!(_session.m_pi.m_cap.stBit.game_master ))
				throw exception("[game_server::requestCommonCmdGM][DISCONNECT][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao eh GM mas tentou executar comando GM. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5700100));

			int32_t oid = _packet->readInt32();

			auto s = (player*)m_player_manager.findSessionByOID(oid);

			if (s == nullptr)
				throw exception("[game_server::requestCommonCmdGM][DISCONNECT][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou executar o comando /disconnect or /discon_uid mas nao encontrou o player[OID="
						+ std::to_string(oid) + "] do oid fornecido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 8, 0));

			_smp::message_pool::getInstance().push(new message("[game_server::requestCommonCmdGM][DISCONNECT][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] desconecta o player[UID="
					+ std::to_string(s->m_pi.uid) + ", NICKNAME=" + std::string(s->m_pi.nickname) + "]", CL_FILE_LOG_AND_CONSOLE));

			DisconnectSession(s);

			break;
		}
		case CCG_DESTROY:
		{

			_smp::message_pool::getInstance().push(new message("[game_server::requestCommonCmdGM][DESTROY][Log] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] destroy a sala, mas aqui o cliente nao passa mais o numero dela, mas passa no packet060, e eu destruo a sala por la.", CL_FILE_LOG_AND_CONSOLE));

			_smp::message_pool::getInstance().push(new message("[game_server::requestCommonCmdGM][DESTROY][Log] [Command=" + std::to_string(cmd) + "]. Hex: " + hex_util::BufferToHexString(_packet->getBuffer(), _packet->getSize()), CL_ONLY_FILE_LOG));

			break;
		}
		case CCG_CHANGE_WIND_VERSUS:
		{

			if (!(_session.m_pi.m_cap.stBit.game_master ))
				throw exception("[game_server::requestCommonCmdGM][CHANGE_WIND_VERSUS][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao eh GM mas tentou executar comando GM. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5700100));

			auto c = findChannel(_session.m_pi.channel);

			if (c == nullptr)
				throw exception("[game_server::requestCommonCmdGM][CHANGE_WIND_VERSUS][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou executar o comando /wind mas ele nao esta em nenhum canal. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 8, 0x5700100));

			c->requestExecCCGChangeWindVersus(_session, _packet);

			break;
		}
		case CCG_CHANGE_WEATHER:
		{

			if (!(_session.m_pi.m_cap.stBit.game_master ))
				throw exception("[game_server::requestCommonCmdGM][CHANGE_WEATHER][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao eh GM mas tentou executar comando GM. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5700100));

			auto c = findChannel(_session.m_pi.channel);

			if (c == nullptr)
				throw exception("[game_server::requestCommonCmdGM][CHANGE_WEATHER][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou executar o comando /weather mas ele nao esta em nenhum canal. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 8, 0x5700100));

			c->requestExecCCGChangeWeather(_session, _packet);

			break;
		}
		case CCG_GIVE_ITEM:
		{

			if (!(_session.m_pi.m_cap.stBit.game_master ))
				throw exception("[game_server::requestCommonCmdGM][GIVE_ITEM][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao eh GM mas tentou executar comando GM. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5700100));

			if (_session.m_pi.m_cap.stBit.block_give_item_gm)
				throw exception("[game_server::requestCommonCmdGM][GIVE_ITEM][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] esta bloqueado para enviar itens pelo comando giveitem.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 9, 0x5700100));

			uint32_t oid_send = _packet->readUint32();
			uint32_t item_typeid = _packet->readUint32();
			uint32_t item_qntd = _packet->readUint32();

			auto s = (player*)m_player_manager.findSessionByOID(oid_send);

			if (s == nullptr || !s->getState() || !s->isConnected())
				throw exception("[game_server::requestCommonCmdGM][GIVE_ITEM][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar presente para o player[OID="
						+ std::to_string(oid_send) + "] mas ele nao esta nesse server.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CHANNEL, 2, 0x5700100));

			if (item_typeid == 0)
				throw exception("[game_server::requestCommonCmdGM][GIVE_ITEM][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar presente para o player[UID="
						+ std::to_string(s->m_pi.uid) + "] mas o Item[TYPEID=" + std::to_string(item_typeid) + "QNTD = " + std::to_string(item_qntd) + "] eh invalid. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 3, 0x5700100));

			if (item_qntd > 20000u)
				throw exception("[game_server::requestCommonCmdGM][GIVE_ITEM][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar presente para o player[UID="
						+ std::to_string(s->m_pi.uid) + "] o Item[TYPEID=" + std::to_string(item_typeid) + ", QNTD=" + std::to_string(item_qntd) + "], mas a quantidade passa de 20mil. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 4, 0x5700100));

			auto base = sIff::getInstance().findCommomItem(item_typeid);

			if (base == nullptr)
				throw exception("[game_server::requestCommonCmdGM][GIVE_ITEM][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar presente para o player[UID="
						+ std::to_string(s->m_pi.uid) + "] o Item[TYPEID=" + std::to_string(item_typeid) + ", QNTD=" + std::to_string(item_qntd) + "], mas o item nao existe no IFF_STRUCT do Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 6, 0));

			stItem item{ 0 };
			BuyItem bi{ 0 };

			bi.id = -1;
			bi._typeid = item_typeid;
			bi.qntd = item_qntd;

			item_manager::initItemFromBuyItem(s->m_pi, item, bi, false, 0, 0, 1 );

			if (item._typeid == 0)
				throw exception("[game_server::requestCommonCmdGM][GIVE_ITEM][ErrorSystem] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar presente para o player[UID="
						+ std::to_string(s->m_pi.uid) + "] o Item[TYPEID=" + std::to_string(item_typeid) + ", QNTD=" + std::to_string(item_qntd) + "], mas nao conseguiu inicializar o item. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 5, 0));

			auto msg = std::string("GM enviou um item para voce: item[ " + std::string(base->name) + " ]");

			if (MailBoxManager::sendMessageWithItem(0, s->m_pi.uid, msg, item) <= 0)
				throw exception("[game_server::requestCommonCmdGM][GIVE_ITEM][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar presente para o player[UID="
						+ std::to_string(s->m_pi.uid) + "] o Item[TYPEID=" + std::to_string(item_typeid) + ", QNTD=" + std::to_string(item_qntd) + "], mas nao conseguiu colocar o item no mail box dele. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 7, 0));

			_smp::message_pool::getInstance().push(new message("[game_server::requestCommonCmdGM][GIVE_ITEM][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] enviou um Item[TYPEID="
					+ std::to_string(item_typeid) + ", QNTD=" + std::to_string(item_qntd) + "] para o player[UID=" + std::to_string(s->m_pi.uid) + "]", CL_FILE_LOG_AND_CONSOLE));

			break;
		}
		case CCG_GOLDENBELL:
		{

			if (!(_session.m_pi.m_cap.stBit.game_master ))
				throw exception("[game_server::requestCommonCmdGM][GOLDENBELL][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao eh GM mas tentou executar comando GM. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5700100));

			if (_session.m_pi.m_cap.stBit.block_give_item_gm)
				throw exception("[game_server::requestCommonCmdGM][GOLDENBELL][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] esta bloqueado para enviar itens pelo comando goldenbell.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 9, 0x5700100));

			auto c = findChannel(_session.m_pi.channel);

			if (c == nullptr)
				throw exception("[game_server::requestCommonCmdGM][GOLDENBELL][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou executar o comando /goldenbell mas ele nao esta em nenhum canal. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 8, 0x5700100));

			c->requestExecCCGGoldenBell(_session, _packet);

			break;
		}
		case CCG_IDENTITY:
		{

			if (!(_session.m_pi.m_cap.stBit.game_master ))
				throw exception("[game_server::requestCommonCmdGM][IDENTITY][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] nao eh GM mas tentou executar comando GM. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5700100));

			auto c = findChannel(_session.m_pi.channel);

			if (c == nullptr)
				throw exception("[game_server::requestCommonCmdGM][IDENTITY][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou executar o comando /identity mas ele nao esta em nenhum canal. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 8, 0x5700100));

			c->requestExecCCGIdentity(_session, _packet);

			break;
		}
		default:
#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("Packet 0x8F.\n\rHex Dump.\n\r" + hex_util::BufferToHexString((unsigned char*)_packet->getPlainBuf().buf, _packet->getPlainBuf().len), CL_FILE_LOG_AND_CONSOLE));
#endif
			throw exception("[game_server::requestCommonCmdGM][Error] Comando do GM Comum[value=" + std::to_string(cmd) + "] nao implementado ou nao existe. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 0, 0x5700100));
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestCommonCmdGM][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x40);

		p.addUint8(7);

		p.addString(_session.m_pi.nickname);

		if (STDA_ERROR_DECODE(e.getCodeError()) == 9 )
			p.addString("Nao pode executar esse comando, voce foi bloqueado pelo ADM.");
		else
			p.addString("Nao conseguiu executar o comando.");

		packet_func::session_send(p, &_session, 1);
	}
};

void game_server::requestCommandNoticeGM(player& _session, packet *_packet) {
	REQUEST_BEGIN("CommandNoticeGM");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("CommandNoticeGM");

		if (!(_session.m_pi.m_cap.stBit.game_master ))
			throw exception("[game_server::requestCommandNoticeGM][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] nao eh GM mas tentou executar comando GM. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5700100));

		std::string notice = _packet->readString();

		if (notice.empty())
			throw exception("[game_server::requestCommandNoticeGM][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou executar o comando de notice, mas a notice is empty. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 8, 0x5700100));

		_smp::message_pool::getInstance().push(new message("[game_server::requestCommandNoticeGM][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] enviou notice[NOTICE="
				+ notice + "] para todos do game server.", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x40);

		p.addUint8(7);

		p.addString(_session.m_pi.nickname);
		p.addString(notice);

		for (auto& el : v_channel)
			packet_func::channel_broadcast(*el, p, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestCommandNoticeGM][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x40);

		p.addUint8(7);

		p.addString(_session.m_pi.nickname);
		p.addString("Nao conseguiu executar o comando.");

		packet_func::session_send(p, &_session, 1);
	}
};

void game_server::requestTranslateSubPacket(player& _session, packet *_packet) {
	REQUEST_BEGIN("TranslateSubPacket");

	packet p;

	unsigned short sub_packet_id = _packet->readUint16();

	try {

		CHECK_SESSION_IS_AUTHORIZED("TranslateSubPacket");

		switch (sub_packet_id) {
		case 0x111:
		{

			uint32_t uid = _packet->readUint32();
			std::string msg = _packet->readString();
			unsigned char opt = _packet->readUint8();

			if (uid == 0)
				throw exception("[game_server::requestTranslateSubPacket][ID=" + std::to_string(sub_packet_id) + "][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou mandar Message Off["
						+ msg + "] para o player[UID=" + std::to_string(uid) + "], mas m_uid is invalid(zero). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 0x5700101));

			if (msg.empty())
				throw exception("[game_server::requestTranslateSubPacket][ID=" + std::to_string(sub_packet_id) + "][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou mandar Message Off["
						+ msg + "] para o player[UID=" + std::to_string(uid) + "], mas msg is empty. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 2, 0x5700102));

			if (msg.size() > 256)
				throw exception("[game_server::requestTranslateSubPacket][ID=" + std::to_string(sub_packet_id) + "][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou mandar Message Off["
						+ msg + "] para o player[UID=" + std::to_string(uid) + "], mas o tamanho[SIZE=" + std::to_string(msg.size()) + "] da message eh maior que o limite suportado. Hacker ou Bug",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 3, 0x5700103));

			if (opt != 0)
				throw exception("[game_server::requestTranslateSubPacket][ID=" + std::to_string(sub_packet_id) + "][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou mandar Message Off["
						+ msg + "] para o player[UID=" + std::to_string(uid) + "], mas opt[VALUE=" + std::to_string((unsigned short)opt) + " eh diferente de 0. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 4, 0x5700104));

			if (_session.m_pi.ui.pang < 10)
				throw exception("[game_server::requestTranslateSubPacket][ID=" + std::to_string(sub_packet_id) + "][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou mandar Message Off["
						+ msg + "] para o player[UID=" + std::to_string(uid) + "], mas ele nao tem pang(s) suficiente[have=" + std::to_string(_session.m_pi.ui.pang) + ", request=10]. Hacker ou Bug",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 5, 0x5700105));

			_session.m_pi.consomePang(10);

			msg = verifyAndEncode(msg);

#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("[game_server::requestTranslateSubPacket::MessageOff][Log] Encoded Message: " + msg, CL_FILE_LOG_AND_CONSOLE));
#endif

			snmdb::NormalManagerDB::getInstance().add(4, new CmdInsertMsgOff(_session.m_pi.uid, uid, msg), game_server::SQLDBResponse, this);

			_smp::message_pool::getInstance().push(new message("[game_server::requestTranslateSubPacket::MessageOff][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] mandou Message Off["
					+ msg + "] para o player[UID=" + std::to_string(uid) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x95);

			p.addUint16(sub_packet_id);

			p.addUint32(0);

			p.addUint64(_session.m_pi.ui.pang);

			packet_func::session_send(p, &_session, 1);

			break;
		}
		case 0x11F:
		{
			throw exception("[game_server::requestTranslateSubPacket][ID=" + std::to_string(sub_packet_id) + "][Error] player[UID="
					+ std::to_string(_session.m_pi.uid) + "] pediu a lista de amigos, mas ainda nao foi implementado essa funcionalidade.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 5, 0x5700105));
			break;
		}
		}

	}catch(exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestTranslateSubPacket][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x95);

		p.addUint16(sub_packet_id);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::GAME_SERVER) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5700100);

		packet_func::session_send(p, &_session, 1);
	}

};

void game_server::requestSendTicker(player& _session, packet *_packet) {
	REQUEST_BEGIN("SendTicker");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("SendTicker");

		if (_session.m_pi.block_flag.m_flag.stBit.ticker)
			throw exception("[game_server::requestSendTicker][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou abrir a fila do Ticker, mas o ticker esta bloqueado. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 10, 1 ));

		std::string msg = _packet->readString();

		if (msg.empty())
			throw exception("[game_server::requestSendTicker][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar ticker[MESSAGE="
					+ msg + "], mas msg is empty(vazia). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 1 ));

		try {

			CPLog cp_log;

			cp_log.setType(CPLog::TYPE::TICKER);

			cp_log.setCookie(1ull);

			_session.m_pi.consomeCookie(1);

			m_ticker.push_back((int)std::time(nullptr) + 3 , _session.m_pi.nickname, msg, BroadcastList::TYPE::TICKER);

			msg = verifyAndEncode(msg);

#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("[game_server::requestSendTicker][Log] Messgae Ticker. Hex: " + hex_util::StringToHexString(msg), CL_FILE_LOG_AND_CONSOLE));
#endif

			snmdb::NormalManagerDB::getInstance().add(6, new CmdInsertTicker(_session.m_pi.uid, m_si.uid, msg), game_server::SQLDBResponse, this);

			_session.saveCPLog(cp_log);

			_smp::message_pool::getInstance().push(new message("[game_server::requestSendTicker][Log] player[UID=" + std::to_string(_session.m_pi.uid) + ", NICKNAME="
					+ std::string(_session.m_pi.nickname) + "] enviou Ticker[MESSAGE=" + msg + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x96);

			p.addUint64(_session.m_pi.cookie);

			packet_func::session_send(p, &_session, 1);

		}catch (exception& e) {

			if (STDA_ERROR_CHECK_SOURCE_AND_ERROR(e.getCodeError(), STDA_ERROR_TYPE::PLAYER_INFO, 20)) {

				throw exception("[game_server::requestSendTicker][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar ticker[MESSAGE="
						+ msg + "], mas ele nao tem cookies suficiente[HAVE=" + std::to_string(_session.m_pi.cookie) + ", REQ=1]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 4 ));

			}else if (STDA_ERROR_CHECK_SOURCE_AND_ERROR(e.getCodeError(), STDA_ERROR_TYPE::PLAYER_INFO, 200 )) {

				throw exception("[game_server::requestSendTicker][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar ticker[MESSAGE="
						+ msg + ", mas tem alteracoes no Cookie dele no Banco de dados.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1, 4 ));

			}else {

				_session.m_pi.addCookie(1);

				throw;
			}
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestSendTicker][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x50);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::GAME_SERVER) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 1 );

		packet_func::session_send(p, &_session, 1);
	}
};

void game_server::requestQueueTicker(player& _session, packet *_packet) {
	REQUEST_BEGIN("QueueTicker");

	packet p;

	try {

		CHECK_SESSION_IS_AUTHORIZED("QueueTicker");

		if (_session.m_pi.block_flag.m_flag.stBit.ticker)
			throw exception("[game_server::requestQueueTicker][Error] player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou abrir a fila do Ticker, mas o ticker esta bloqueado. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 10, 1 ));

		auto count = m_ticker.getSize();

		uint32_t time_left_milisecond = count * 30000;

		p.init_plain((unsigned short)0xCA);

		p.addUint16((unsigned short)count);
		p.addUint32(time_left_milisecond);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestQueueTicker][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		packet p((unsigned short)0x50);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::GAME_SERVER) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 1 );

		packet_func::session_send(p, &_session, 1);
	}
};

void game_server::requestExceptionClientMessage(player& _session, packet *_packet) {
	REQUEST_BEGIN("ExceptionClientMessage");

	try {

		CHECK_SESSION_IS_AUTHORIZED("ExceptionClientMessage");

		unsigned char tipo = _packet->readUint8();

		std::string exception_msg = _packet->readString();

		_smp::message_pool::getInstance().push(new message("[game_server::requestExceptionClientMessage][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "], EXCEPTION[TIPO="
				+ std::to_string((unsigned short)tipo) + ", MESSAGE=" + exception_msg + "]", CL_ONLY_FILE_LOG));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestExceptionClientMessage][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::requestCheckGameGuardAuthAnswer(player& _session, packet *_packet) {
	REQUEST_BEGIN("CheckGameGuardAuthAnswer");

	try {

		CHECK_SESSION_IS_AUTHORIZED("CheckGameGuardAuthAnswer");

		if (m_GameGuardAuth) {

#ifdef _DEBUG
			_smp::message_pool::getInstance().push(new message("[game_server::requestCheckGameGuardAuthAnswer][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] GG Auth Hash: " + hex_util::BufferToHexString(_packet->getBuffer(), _packet->getSize()), CL_FILE_LOG_AND_CONSOLE));
#endif

			_packet->readBuffer(&_session.m_gg.m_csa.m_AuthAnswer, sizeof(GG_AUTH_DATA));

#if INTPTR_MAX == INT64_MAX && MY_GG_SRV_LIB == 0
			DWORD err_success = _session.m_gg.m_csa.CheckAuthAnswer();

			if (err_success != ERROR_SUCCESS) {

				char sd[1024];

#if defined(_WIN32)
				sprintf_s(sd, 1024, "Err: %d, Index: %08X, Value1: %08X, Value2: %08X, Value3: %08X", err_success, _session.m_gg.m_csa.m_AuthAnswer.dwIndex,
					_session.m_gg.m_csa.m_AuthAnswer.dwValue1, _session.m_gg.m_csa.m_AuthAnswer.dwValue2, _session.m_gg.m_csa.m_AuthAnswer.dwValue3);
#elif defined(__linux__)
				sprintf(sd, "Err: %d, Index: %08X, Value1: %08X, Value2: %08X, Value3: %08X", err_success, _session.m_gg.m_csa.m_AuthAnswer.dwIndex,
					_session.m_gg.m_csa.m_AuthAnswer.dwValue1, _session.m_gg.m_csa.m_AuthAnswer.dwValue2, _session.m_gg.m_csa.m_AuthAnswer.dwValue3);
#endif

				_smp::message_pool::getInstance().push(new message("[game_server::requestCheckGameGuardAuthAnswer][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "] CSAuth Fail. " + sd, CL_FILE_LOG_AND_CONSOLE));

				DisconnectSession(&_session);

				_smp::message_pool::getInstance().push(new message("[game_server::requestCheckGameGuardAuthAnswer][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] GG Auth Check answer: false", CL_FILE_LOG_AND_CONSOLE));
			}
#elif INTPTR_MAX == INT32_MAX || MY_GG_SRV_LIB == 1
			DWORD err_success = _session.m_gg.m_csa.CheckAuthAnswer();

			if (err_success == ERROR_SUCCESS)
				_session.m_gg.m_auth_reply = true;
			else {

				char sd[1024];

#if defined(_WIN32)
				sprintf_s(sd, 1024, "Err: %d, Index: %08X, Value1: %08X, Value2: %08X, Value3: %08X", err_success, _session.m_gg.m_csa.m_AuthAnswer.dwIndex,
						_session.m_gg.m_csa.m_AuthAnswer.dwValue1, _session.m_gg.m_csa.m_AuthAnswer.dwValue2, _session.m_gg.m_csa.m_AuthAnswer.dwValue3);
#elif defined(__linux__)
				sprintf(sd, "Err: %d, Index: %08X, Value1: %08X, Value2: %08X, Value3: %08X", err_success, _session.m_gg.m_csa.m_AuthAnswer.dwIndex,
						_session.m_gg.m_csa.m_AuthAnswer.dwValue1, _session.m_gg.m_csa.m_AuthAnswer.dwValue2, _session.m_gg.m_csa.m_AuthAnswer.dwValue3);
#endif

				_smp::message_pool::getInstance().push(new message("[game_server::requestCheckGameGuardAuthAnswer][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "] CSAuth Fail. " + sd, CL_FILE_LOG_AND_CONSOLE));

				DisconnectSession(&_session);
			}

			_smp::message_pool::getInstance().push(new message("[game_server::requestCheckGameGuardAuthAnswer][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] GG Auth Check answer: " + (err_success == ERROR_SUCCESS ? "true" : "false"), CL_FILE_LOG_AND_CONSOLE));
#else
#error Unknown pointer size or missing size macros!
#endif

		}else {

			_smp::message_pool::getInstance().push(new message("[game_server::requestCheckGameGuardAuthAnswer][Error][WARNING] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] enviou GameGuardAuth Answer, mas o GameGuard Auth no server esta desabilitado.", CL_FILE_LOG_AND_CONSOLE));

			DisconnectSession(&_session);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::requestCheckGameGuardAuthAnswer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

timer* game_server::makeTime(DWORD _milleseconds, job& _job) {

	if (!_job.isValid())
		throw exception("[game_server::makeTime][Error] job is invalid", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 50, 0));

	timer *t = nullptr;

	if ((t = m_timer_manager.createTimer(_milleseconds, new (timer::timer_param){ _job, m_job_pool })) == nullptr)
		throw exception("[game_server::makeTime][Error] nao conseguiu criar o timer", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 51, 0));

	return t;
};

timer* game_server::makeTime(DWORD _milleseconds, job& _job, std::vector< DWORD > _table_interval) {

	if (!_job.isValid())
		throw exception("[game_server::makeTime][Error] job is invalid", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 50, 0));

	timer *t = nullptr;

	if ((t = m_timer_manager.createTimer(_milleseconds, new (timer::timer_param){ _job, m_job_pool }, _table_interval)) == nullptr)
		throw exception("[game_server::makeTime][Error] nao conseguiu criar o timer", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 51, 0));

	return t;
};

void game_server::unMakeTime(timer* _timer) {

	if (_timer == nullptr)
		throw exception("[game_server::unMakeTime][Error] tentou deletar o timer, mas o argumento eh nullptr", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 52, 0));

	m_timer_manager.deleteTimer(_timer);
};

void game_server::setRatePang(uint32_t _pang) {

	m_si.event_flag.stBit.pang_x_plus = (_pang >= 200) ? 1u : 0u;

	m_si.rate.pang = (unsigned short)_pang;
};

void game_server::setRateExp(uint32_t _exp) {

	m_si.event_flag.stBit.exp_x2 = m_si.event_flag.stBit.exp_x_plus = 0u;

	if (_exp > 200)
		m_si.event_flag.stBit.exp_x_plus = 1u;
	else if (_exp == 200)
		m_si.event_flag.stBit.exp_x2 = 1u;
	else
		m_si.event_flag.stBit.exp_x2 = m_si.event_flag.stBit.exp_x_plus = 0u;

	m_si.rate.exp = (unsigned short)_exp;
};

void game_server::setRateClubMastery(uint32_t _club_mastery) {

	m_si.event_flag.stBit.club_mastery_x_plus = (_club_mastery >= 200) ? 1u : 0u;

	m_si.rate.club_mastery = (unsigned short)_club_mastery;
};

void game_server::setAngelEvent(uint32_t _angel_event) {

	m_si.event_flag.stBit.angel_wing = _angel_event;

	m_si.rate.angel_event = (unsigned short)_angel_event;
};

void game_server::updateDailyQuest(DailyQuestInfo& _dqi) {
	m_dqi = _dqi;
};

void game_server::sendUpdateRoomInfo(room *_r, int _option) {

	try {

		if (_r != nullptr) {

			auto c = findChannel(_r->getChannelOwenerId());

			if (c != nullptr)
				c->sendUpdateRoomInfo(*const_cast< RoomInfoEx* >(_r->getInfo()), _option);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::sendUpdateRoomInfo][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::authCmdShutdown(int32_t _time_sec) {

	try {

		if (m_shutdown == nullptr) {

			_smp::message_pool::getInstance().push(new message("[game_server::authCmdShutdown][Log] Auth Server requisitou desligar o Server em "
					+ std::to_string(_time_sec) + " segundos.", CL_FILE_LOG_AND_CONSOLE));

			shutdown_time(_time_sec);

		}else
			_smp::message_pool::getInstance().push(new message("[game_server::authCmdShutdown][WARNING] Auth Server requisitou para o server ser delisgado em "
					+ std::to_string(_time_sec) + " segundos, mas o server ja esta com o timer de shutdown", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::authCmdShutdown][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::authCmdBroadcastNotice(std::string _notice) {

	try {

		_smp::message_pool::getInstance().push(new message("[game_server::authCmdBroadcastNotice][Log] Auth Server Comando Notice[MESSAGE=" + _notice + "].", CL_FILE_LOG_AND_CONSOLE));

		auto notice = verifyAndTranslate(_notice, 2 );

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[game_server::authCmdBroadcasNotice][Log] Message. Hex: " + hex_util::StringToHexString(notice), CL_FILE_LOG_AND_CONSOLE));
#endif

		m_notice.push_back(0, notice, BroadcastList::TYPE::GM_NOTICE);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::authCmdBroadcastNotice][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::authCmdBroadcastTicker(std::string _nickname, std::string _msg) {

	try {

		_smp::message_pool::getInstance().push(new message("[game_server::authCmdBroadcastTicker][Log] Auth Server Comando Ticker[NICKNAME="
				+ _nickname + ", MESSAGE=" + _msg + "].", CL_FILE_LOG_AND_CONSOLE));

		auto msg = verifyAndTranslate(_msg, 2 );

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[game_server::authCmdBroadcasTicker][Log] Message. Hex: " + hex_util::StringToHexString(msg), CL_FILE_LOG_AND_CONSOLE));
#endif

		m_ticker.push_back(0, _nickname, msg, BroadcastList::TYPE::TICKER);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::authCmdBroadcastTicker][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::authCmdBroadcastCubeWinRare(std::string _msg, uint32_t _option) {

	try {

		_smp::message_pool::getInstance().push(new message("[game_server::authCmdBroadcastCubeWinRare][Log] Auth Server Comando Cube Win Rare Notice[MESSAGE="
				+ _msg + ", OPTION=" + std::to_string(_option) + "].", CL_FILE_LOG_AND_CONSOLE));

		auto msg = verifyAndTranslate(_msg, 2 );

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[game_server::authCmdBroadcastCubeWinRare][Log] Message. Hex: " + hex_util::StringToHexString(msg), CL_FILE_LOG_AND_CONSOLE));
#endif

		m_notice.push_back(0, msg, _option, BroadcastList::TYPE::CUBE_WIN_RARE);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::authCmdBroadcastCubeWinRare][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::authCmdDisconnectPlayer(uint32_t _req_server_uid, uint32_t _player_uid, unsigned char _force) {

	try {

		auto s = m_player_manager.findPlayer(_player_uid);

		if (s != nullptr) {

			_smp::message_pool::getInstance().push(new message("[game_server::authCmdDisconnectPlayer][log] Comando do Auth Server, Server[UID=" + std::to_string(_req_server_uid)
					+ "] pediu para desconectar o Player[UID=" + std::to_string(s->m_pi.uid) + "]", CL_FILE_LOG_AND_CONSOLE));

			if (_force)
				DisconnectSession(s);
			else {

#if defined(_WIN32)
				ReaderIni ini("\\server.ini");
#elif defined(__linux__)
				ReaderIni ini("/server.ini");
#endif

				ini.init();

				int32_t same_id_login = 0u;

				try {
					same_id_login = ini.readInt("OPTION", "SAME_ID_LOGIN");
				}catch (exception& e) {

					UNREFERENCED_PARAMETER(e);
				}

				if (!same_id_login)
					DisconnectSession(s);
			}

		}else {

			snmdb::NormalManagerDB::getInstance().add(5, new CmdRegisterLogon(_player_uid, 1 ), game_server::SQLDBResponse, this);

			_smp::message_pool::getInstance().push(new message("[game_server::authCmdDisconnectPlayer][WARNING] Comando do Auth Server, Server[UID=" + std::to_string(_req_server_uid)
					+ "] pediu para desconectar o Player[UID=" + std::to_string(_player_uid) + "], mas nao encontrou ele no server, entao desconecta ele no banco de dados.", CL_FILE_LOG_AND_CONSOLE));
		}

		m_unit_connect->sendConfirmDisconnectPlayer(_req_server_uid, _player_uid);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::authCmdDisconnectPlayer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::authCmdConfirmDisconnectPlayer(uint32_t  ) {

	return;
};

void game_server::authCmdNewMailArrivedMailBox(uint32_t _player_uid, uint32_t _mail_id) {

	try {

		auto s = m_player_manager.findPlayer(_player_uid);

		if (s == nullptr)
			throw exception("[game_server::authCmdNewMailArrivedMailBox][Warning] Auth Server Comando New Mail[ID=" + std::to_string(_mail_id)
					+ "] Arrived no Mailbox do player[UID=" + std::to_string(_player_uid) + "], mas o player nao esta conectado no server.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 130, 0));

		s->m_pi.m_mail_box.addNewEmailArrived(_mail_id);

		auto v_mi = s->m_pi.m_mail_box.getAllUnreadEmail();

		if (v_mi.empty())
			throw exception("[game_server::authCmdNewMailArrivedMailBox][Error] Auth Server Comando New Mail[ID=" + std::to_string(_mail_id)
					+ "] Arrived no Mailbox do player[UID=" + std::to_string(_player_uid) + "], mas nao tem nenhum email nao lido no Mailbox dele.",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 131, 0));

		_smp::message_pool::getInstance().push(new message("[game_server::authCmdNewMailArrivedMailBox][Log] Auth Server Comando New Mail[ID=" + std::to_string(_mail_id)
				+ "] Arrived Mailbox do player[UID=" + std::to_string(_player_uid) + "].", CL_FILE_LOG_AND_CONSOLE));

		packet p((unsigned short)0x210);

		p.addUint32(0);

		p.addUint32((uint32_t)v_mi.size());

		for (auto& el : v_mi)
			p.addBuffer(&el, sizeof(MailBox));

		packet_func::session_send(p, s, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::authCmdNewMailArrivedMailBox][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::authCmdNewRate(uint32_t _tipo, uint32_t _qntd) {

	try {

		updateRateAndEvent(_tipo, _qntd);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::authCmdNewRate][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::authCmdReloadGlobalSystem(uint32_t _tipo) {

	try {

		reloadGlobalSystem(_tipo);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::authCmdReloadGlobalSystem][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::authCmdConfirmSendInfoPlayerOnline(uint32_t _req_server_uid, AuthServerPlayerInfo _aspi) {

	UNREFERENCED_PARAMETER(_req_server_uid);

	try {

		auto s = m_player_manager.findPlayer(_aspi.uid);

		if (s != nullptr) {

		}else
			_smp::message_pool::getInstance().push(new message("[game_server::authCmdConfirmSendInfoPlayerOnline][WARNING] Player[UID=" + std::to_string(_aspi.uid)
					+ "] retorno do confirma login com Auth Server do Server[UID=" + std::to_string(_req_server_uid) + "], mas o palyer nao esta mais conectado.", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::authCmdConfirmSendInfoPlayerOnline][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

#if INTPTR_MAX == INT64_MAX && MY_GG_SRV_LIB == 0
void game_server::ggAuthCmdGetQuery(uint32_t _error, uint32_t _socket_id, GG_AUTH_DATA& _data) {

	try {

		if (!m_GameGuardAuth || m_game_guard == nullptr) {

			_smp::message_pool::getInstance().push(new message("[game_server::ggAuthCmdGetQuery][Error] Game Guard " + std::string(m_GameGuardAuth ? "esta desativado." : "nao foi inicializado."), CL_FILE_LOG_AND_CONSOLE));

			return;
		}

		auto session = m_player_manager.findPlayerBySocketId(_socket_id);

		if (session == nullptr) {

			_smp::message_pool::getInstance().push(new message("[game_server::ggAuthCmdGetQuery][Error] Session[SOCKET=" + std::to_string(_socket_id) + "] nao foi encontrado.", CL_FILE_LOG_AND_CONSOLE));

			return;
		}

		if (_error != ERROR_SUCCESS) {

			_smp::message_pool::getInstance().push(new message("[game_server::ggAuthCmdGetQuery][Error] Player[UID=" + std::to_string(session->m_pi.uid) + "] CSAuth Fail Err: " + std::to_string(_error)
					+ ", Index: " + std::to_string(session->m_gg.m_csa.m_AuthQuery.dwIndex) + ", Value1: " + std::to_string(session->m_gg.m_csa.m_AuthQuery.dwValue1)
					+ ", Value2: " + std::to_string(session->m_gg.m_csa.m_AuthQuery.dwValue2) + ", Value3: " + std::to_string(session->m_gg.m_csa.m_AuthQuery.dwValue3), CL_FILE_LOG_AND_CONSOLE));

			DisconnectSession(session);

			return;
		}

#if defined(_WIN32)
		memcpy_s(&session->m_gg.m_csa.m_AuthQuery, sizeof(session->m_gg.m_csa.m_AuthQuery), &_data, sizeof(_data));
#elif defined(__linux__)
		memcpy(&session->m_gg.m_csa.m_AuthQuery, &_data, sizeof(_data));
#endif

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[game_server::ggAuthCmdGetQuery][Log] Player[UID=" + std::to_string(session->m_pi.uid) + "] send GameGuard Auth Query. Index: "
				+ std::to_string(session->m_gg.m_csa.m_AuthQuery.dwIndex) + ", Value1: " + std::to_string(session->m_gg.m_csa.m_AuthQuery.dwValue1)
				+ ", Value2: " + std::to_string(session->m_gg.m_csa.m_AuthQuery.dwValue2) + ", Value3: " + std::to_string(session->m_gg.m_csa.m_AuthQuery.dwValue3), CL_FILE_LOG_AND_CONSOLE));
#endif

		auto now = std::time(nullptr);

		session->m_gg.m_auth_time = now + (sRandomGen::getInstance().rIbeMt19937_64_chronoRange(0, AUTH_PERIOD_VARIATION_SEC) + AUTH_PERIOD_SEC);

		packet p((unsigned short)0xD7);

		p.addBuffer(&session->m_gg.m_csa.m_AuthQuery, sizeof(_GG_AUTH_DATA));

		packet_func::session_send(p, session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::ggAuthCmdGetQuery][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::ggAuthCmdCheckAnswer(uint32_t _error, uint32_t _socket_id) {

	try {

		if (!m_GameGuardAuth || m_game_guard == nullptr) {

			_smp::message_pool::getInstance().push(new message("[game_server::ggAuthCmdCheckAnswer][Error] Game Guard " + std::string(m_GameGuardAuth ? "esta desativado." : "nao foi inicializado."), CL_FILE_LOG_AND_CONSOLE));

			return;
		}

		auto session = m_player_manager.findPlayerBySocketId(_socket_id);

		if (session == nullptr) {

			_smp::message_pool::getInstance().push(new message("[game_server::ggAuthCmdCheckAnswer][Error] Session[SOCKET=" + std::to_string(_socket_id) + "] nao foi encontrado.", CL_FILE_LOG_AND_CONSOLE));

			return;
		}

		if (_error == ERROR_SUCCESS)
			session->m_gg.m_auth_reply = true;
		else {

			char sd[1024];

#if defined(_WIN32)
			sprintf_s(sd, 1024, "Err: %d, Index: %08X, Value1: %08X, Value2: %08X, Value3: %08X", _error, session->m_gg.m_csa.m_AuthAnswer.dwIndex,
				session->m_gg.m_csa.m_AuthAnswer.dwValue1, session->m_gg.m_csa.m_AuthAnswer.dwValue2, session->m_gg.m_csa.m_AuthAnswer.dwValue3);
#elif defined(__linux__)
			sprintf(sd, "Err: %d, Index: %08X, Value1: %08X, Value2: %08X, Value3: %08X", _error, session->m_gg.m_csa.m_AuthAnswer.dwIndex,
				session->m_gg.m_csa.m_AuthAnswer.dwValue1, session->m_gg.m_csa.m_AuthAnswer.dwValue2, session->m_gg.m_csa.m_AuthAnswer.dwValue3);
#endif

			_smp::message_pool::getInstance().push(new message("[game_server::ggAuthCmdCheckAnswer][Error] Player[UID=" + std::to_string(session->m_pi.uid) + "] CSAuth Fail. " + sd, CL_FILE_LOG_AND_CONSOLE));

			DisconnectSession(session);
		}

		_smp::message_pool::getInstance().push(new message("[game_server::ggAuthCmdCheckAnswer][Log] Player[UID=" + std::to_string(session->m_pi.uid)
				+ "] GG Auth Check answer: " + (_error == ERROR_SUCCESS ? "true" : "false"), CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::ggAuthCmdCheckAnswer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

uint32_t game_server::ggAuthCreateUser(uint32_t _socket_id) {

	uint32_t error = ERROR_SUCCESS;

	try {

		if (!m_GameGuardAuth)
			throw exception("[game_server::ggAuthCreateUser][Error] Player[SOCKET=" + std::to_string(_socket_id)
					+ "] tentou criar User Game Guard Auth, porem o Game Guard esta desativado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 15500, 101));

		if (m_game_guard == nullptr)
			throw exception("[game_server::ggAuthCreateUser][Error] Player[SOCKET=" + std::to_string(_socket_id)
				+ "] tentou criar User Game Guard Auth, porem o Game Guard nao esta inicializado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 15501, 102));

		auto p = m_player_manager.findPlayerBySocketId(_socket_id);

		if (p == nullptr ||
#if defined(_WIN32)
			p->m_sock == INVALID_SOCKET
#elif defined(__linux__)
			p->m_sock.fd == INVALID_SOCKET
#endif
		)
			throw exception("[game_server::ggAuthCreateUser][Error] Player[SOCKET=" + std::to_string(_socket_id)
				+ "] tentou criar User Game Guard Auth, porem o player nao esta online no server.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 15502, 103));

		_smp::message_pool::getInstance().push(new message("[game_server::ggAuthCreateUser][Log] Player[OID=" + std::to_string(p->m_oid)
				+ ", SOCKET=" + std::to_string(
#if defined(_WIN32)
					p->m_sock
#elif defined(__linux__)
					p->m_sock.fd
#endif
				) + "] envia o pedido para criar o User Game Guard Auth para o Game Guard Auth Server.", CL_FILE_LOG_AND_CONSOLE));

		m_game_guard->sendCreateUser((uint32_t)
#if defined(_WIN32)
			p->m_sock
#elif defined(__linux__)
			p->m_sock.fd
#endif
		);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::ggAuthCreateUser][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		error = (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::GAME_SERVER ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 1234);
	}

	return error;
};

uint32_t game_server::ggAuthDeleteUser(uint32_t _socket_id) {

	uint32_t error = ERROR_SUCCESS;

	try {

		if (!m_GameGuardAuth)
			throw exception("[game_server::ggAuthDeleteUser][Error] Player[SOCKET=" + std::to_string(_socket_id)
					+ "] tentou deletar User Game Guard Auth, porem o Game Guard esta desativado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 15500, 101));

		if (m_game_guard == nullptr)
			throw exception("[game_server::ggAuthDeleteUser][Error] Player[SOCKET=" + std::to_string(_socket_id)
					+ "] tentou deletar User Game Guard Auth, porem o Game Guard nao esta inicializado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 15501, 102));

		_smp::message_pool::getInstance().push(new message("[game_server::ggAuthDeleteUser][Log] Player[SOCKET="
				+ std::to_string(_socket_id) + "] envia o pedido para deletar o User Game Guard Auth para o Game Guard Auth Server.", CL_FILE_LOG_AND_CONSOLE));

		m_game_guard->sendDeleteUser(_socket_id);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::ggAuthDeleteUser][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		error = (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::GAME_SERVER ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 1234);
	}

	return error;
};

uint32_t game_server::ggAuthGetQuery(uint32_t _socket_id) {

	uint32_t error = ERROR_SUCCESS;

	try {

		if (!m_GameGuardAuth)
			throw exception("[game_server::ggAuthGetQuery][Error] Player[SOCKET=" + std::to_string(_socket_id)
					+ "] tentou pegar o Auth Query, porem o Game Guard esta desativado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 15500, 101));

		if (m_game_guard == nullptr)
			throw exception("[game_server::ggAuthGetQuery][Error] Player[SOCKET=" + std::to_string(_socket_id)
				+ "] tentou pegar o Auth Query, porem o Game Guard nao esta inicializado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 15501, 102));

		auto p = m_player_manager.findPlayerBySocketId(_socket_id);

		if (p == nullptr ||
#if defined(_WIN32)
			p->m_sock == INVALID_SOCKET
#elif defined(__linux__)
			p->m_sock.fd == INVALID_SOCKET
#endif
		)
			throw exception("[game_server::ggAuthGetQuery][Error] Player[SOCKET=" + std::to_string(_socket_id)
				+ "] tentou pegar o Auth Query, porem o player nao esta online no server.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 15502, 103));

		_smp::message_pool::getInstance().push(new message("[game_server::ggAuthGetQuery][Log] Player[UID=" + std::to_string(p->m_pi.uid)
				+ ", SOCKET=" + std::to_string(
#if defined(_WIN32)
					p->m_sock
#elif defined(__linux__)
					p->m_sock.fd
#endif
				) + "] envia o pedido de Auth Query para o Game Guard Auth Server.", CL_FILE_LOG_AND_CONSOLE));

		m_game_guard->sendGetQuery((uint32_t)
#if defined(_WIN32)
			p->m_sock
#elif defined(__linux__)
			p->m_sock.fd
#endif
		);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::ggAuthGetQuery][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		error = (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::GAME_SERVER ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 1234);
	}

	return error;
};

uint32_t game_server::ggAuthCheckAnswer(uint32_t _socket_id, GG_AUTH_DATA& _data) {

	uint32_t error = ERROR_SUCCESS;

	try {

		if (!m_GameGuardAuth)
			throw exception("[game_server::ggAuthCheckAnswer][Error] Player[SOCKET=" + std::to_string(_socket_id)
					+ "] tentou enviar para o Game Guard Auth Server o Auth Answer para verificar, porem o Game Guard esta desativado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 15500, 101));

		if (m_game_guard == nullptr)
			throw exception("[game_server::ggAuthCheckAnswer][Error] Player[SOCKET=" + std::to_string(_socket_id)
				+ "] tentou enviar para o Game Guard Auth Server o Auth Answer para verificar, porem o Game Guard nao esta inicializado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 15501, 102));

		auto p = m_player_manager.findPlayerBySocketId(_socket_id);

		if (p == nullptr ||
#if defined(_WIN32)
			p->m_sock == INVALID_SOCKET
#elif defined(__linux__)
			p->m_sock.fd == INVALID_SOCKET
#endif
		)
			throw exception("[game_server::ggAuthCheckAnswer][Error] Player[SOCKET=" + std::to_string(_socket_id)
				+ "] tentou enviar para o Game Guard Auth Server o Auth Answer para verificar, porem o player nao esta online no server.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 15502, 103));

		char sd[1024];

#if defined(_WIN32)
		sprintf_s(sd, 1024, "Index: %08X, Value1: %08X, Value2: %08X, Value3: %08X", _data.dwIndex, _data.dwValue1, _data.dwValue2, _data.dwValue3);
#elif defined(__linux__)
		sprintf(sd, "Index: %08X, Value1: %08X, Value2: %08X, Value3: %08X", _data.dwIndex, _data.dwValue1, _data.dwValue2, _data.dwValue3);
#endif

		_smp::message_pool::getInstance().push(new message("[game_server::ggAuthCheckAnswer][Log] Player[UID=" + std::to_string(p->m_pi.uid)
				+ ", SOCKET=" + std::to_string(
#if defined(_WIN32)
					p->m_sock
#elif defined(__linux__)
					p->m_sock.fd
#endif
				) + "] DATA{" + sd
				+ "} envia o pedido de verificacao do Auth Answer para o Game Guard Auth Server.", CL_FILE_LOG_AND_CONSOLE));

		m_game_guard->sendCheckAnswer((uint32_t)
#if defined(_WIN32)
			p->m_sock
#elif defined(__linux__)
			p->m_sock.fd
#endif
		, _data);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::ggAuthCheckAnswer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		error = (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::GAME_SERVER ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 1234);
	}

	return error;
};

void game_server::authCmdKeyLoginGGAuthServer(packet *_packet) {

	if (_packet == nullptr)
		throw exception("[game_server::authCmdKeyLoginGGAuthServer][Error] _packet is nullptr", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 6, 0));

	try {

		uint32_t key_auth = _packet->readUint32();

		if (m_GameGuardAuth && m_game_guard != nullptr) {

			std::string key = std::to_string((uint32_t)(m_si.uid ^ key_auth));

			try {

				key = md5::hash((const unsigned char*)key.data(), (uint32_t)key.length());

			}catch (exception& e) {

				_smp::message_pool::getInstance().push(new message("[game_server::authCmdKeyLoginGGAuthServer][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

				throw;
			}

			m_game_guard->sendHaskKeyLogin(key);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::authCmdKeyLoginGGAuthServer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};
#endif

void game_server::onAcceptCompleted(session *_session) {

	if (_session == nullptr)
		throw exception("[game_server::onAcceptCompleted][Error] session is nullptr.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::SERVER, 60, 0));

	if (!_session->getState())
		throw exception("[game_server::onAcceptCompleted][Error] session is invalid.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::SERVER, 61, 0));

	if (!_session->isConnected())
		throw exception("[game_server::onAcceptCompleted][Error] session is not connected.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::SERVER, 62, 0));

	player *p = (player*)_session;

	if (m_GameGuardAuth) {

		p->m_gg.m_auth_reply = true;
		p->m_gg.m_auth_time = 0;
#if INTPTR_MAX == INT64_MAX && MY_GG_SRV_LIB == 0
		p->m_gg.m_csa.Init((uint32_t)
#if defined(_WIN32)
			p->m_sock
#elif defined(__linux__)
			p->m_sock.fd
#endif
		);
#elif INTPTR_MAX == INT32_MAX || MY_GG_SRV_LIB == 1
		p->m_gg.m_csa.Init();
#else
#error Unknown pointer size or missing size macros!
#endif
	}

#if STDA_BLOCK_PACKET_ONE_TIME_DISABLE != 0x1 && STDA_BLOCK_PACKET_ONE_TIME_VER == 0x2
	if (p->m_sbeopt != nullptr)
		delete p->m_sbeopt;

	p->m_sbeopt = nullptr;

	p->m_sbeopt = new SyncBlockExecOnePerTime(_session->m_oid + 1u);
#elif STDA_BLOCK_PACKET_ONE_TIME_DISABLE != 0x1
	sSyncBlockExecOnePerTime::getInstance().getPlayer(_session->m_oid + 1u);
#endif

	packet _packet((unsigned short)0x3F);

	_packet.addInt8(1);
	_packet.addInt8(1);
	_packet.addInt8(_session->m_key);

	_packet.makeRaw();
	WSABUF mb = _packet.getMakedBuf();

	try {
		_session->requestRecvBuffer();
		_session->requestSendBuffer(mb.buf, mb.len);
	}catch (exception& e) {
		if (STDA_ERROR_CHECK_SOURCE_AND_ERROR(e.getCodeError(), STDA_ERROR_TYPE::SESSION, 1))
			throw;
	}
};

void game_server::onDisconnected(session *_session) {

	if (_session == nullptr)
		throw exception("[game_server::onDisconnected][Error] _session is nullptr", STDA_MAKE_ERROR(STDA_ERROR_TYPE::SERVER, 60, 0));

	player *_player = reinterpret_cast< player* >(_session);

	_smp::message_pool::getInstance().push(new message("[game_server::onDisconnected][Log] Player Desconectou. ID: " + std::string(_player->m_pi.id) + "  UID: " + std::to_string(_player->m_pi.uid), CL_FILE_LOG_AND_CONSOLE));

	channel *_channel = findChannel(_player->m_pi.channel);

	try {

		if (_channel != nullptr)

			_channel->leaveChannel(*_player, 0x1000);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::onDisconnect][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	snmdb::NormalManagerDB::getInstance().add(5, new CmdRegisterLogon(_player->m_pi.uid, 1 ), game_server::SQLDBResponse, this);

	if (m_si.rate.smart_calculator)
		sSmartCalculator::getInstance().removeAllPlayerCtx(_player->m_pi.uid);

};

void game_server::onHeartBeat() {

	try {

		if (m_state != INITIALIZED)
			return;

		for (auto& el : v_channel)
			el->checkInviteTime();

		if (!sIff::getInstance().isLoad())
			sIff::getInstance().load();

		if (!sCardSystem::getInstance().isLoad())
			sCardSystem::getInstance().load();

		if (!sCometRefillSystem::getInstance().isLoad())
			sCometRefillSystem::getInstance().load();

		if (!sPapelShopSystem::getInstance().isLoad())
			sPapelShopSystem::getInstance().load();

		if (!sBoxSystem::getInstance().isLoad())
			sBoxSystem::getInstance().load();

		if (!sMemorialSystem::getInstance().isLoad())
			sMemorialSystem::getInstance().load();

		if (!sCubeCoinSystem::getInstance().isLoad())
			sCubeCoinSystem::getInstance().load();

		if (!sTreasureHunterSystem::getInstance().isLoad())
			sTreasureHunterSystem::getInstance().load();

		if (!sDropSystem::getInstance().isLoad())
			sDropSystem::getInstance().load();

		if (!sAttendanceRewardSystem::getInstance().isLoad())
			sAttendanceRewardSystem::getInstance().load();

		if (!sMap::getInstance().isLoad())
			sMap::getInstance().load();

		if (!sApproachMissionSystem::getInstance().isLoad())
			sApproachMissionSystem::getInstance().load();

		if (!sGrandZodiacEvent::getInstance().isLoad())
			sGrandZodiacEvent::getInstance().load();

		if (!sCoinCubeLocationUpdateSystem::getInstance().isLoad())
			sCoinCubeLocationUpdateSystem::getInstance().load();

		if (!sGoldenTimeSystem::getInstance().isLoad())
			sGoldenTimeSystem::getInstance().load();

		if (!sLoginRewardSystem::getInstance().isLoad())
			sLoginRewardSystem::getInstance().load();

		if (m_si.rate.smart_calculator && !sSmartCalculator::getInstance().hasStopped() && !sSmartCalculator::getInstance().isLoad())
			sSmartCalculator::getInstance().load();

		if (m_si.rate.grand_zodiac_event_time && sGrandZodiacEvent::getInstance().checkTimeToMakeRoom())
			makeGrandZodiacEventRoom();

		if (m_si.rate.bot_gm_event && sBotGMEvent::getInstance().checkTimeToMakeRoom())
			makeBotGMEventRoom();

		if (m_si.rate.golden_time_event && sGoldenTimeSystem::getInstance().checkRound())
			makeListOfPlayersToGoldenTime();

		if (m_si.rate.login_reward_event)
			sLoginRewardSystem::getInstance().updateLoginReward();

		if (MgrDailyQuest::checkCurrentQuest(m_dqi))
			MgrDailyQuest::updateDailyQuest(m_dqi);

		sPapelShopSystem::getInstance().updateDia();

		if (sTreasureHunterSystem::getInstance().checkUpdateTimePointCourse()) {

			packet p;

			packet_func::pacote131(p);

			for (auto& el : v_channel)
				packet_func::channel_broadcast(*el, p, 1);
		}

		BroadcastList::RetNoticeCtx rt;

		rt = m_notice.peek();

		if (rt.ret == BroadcastList::RET_TYPE::OK) {

			packet p;

			if (rt.nc.type == BroadcastList::TYPE::GM_NOTICE) {

				p.init_plain((unsigned short)0x42);

				p.addString(rt.nc.notice);

			}else if (rt.nc.type == BroadcastList::TYPE::CUBE_WIN_RARE) {

				p.init_plain((unsigned short)0x1D3);

				p.addUint32(1);

					p.addUint32(rt.nc.option);
					p.addString(rt.nc.notice);

			}

			for (auto& el : v_channel)
				packet_func::channel_broadcast(*el, p, 1);
		}

		rt = m_ticker.peek();

		if (rt.ret == BroadcastList::RET_TYPE::OK && rt.nc.type == BroadcastList::TYPE::TICKER) {

			packet p((unsigned short)0xC9);

			p.addString(rt.nc.nickname);
			p.addString(rt.nc.notice);

			for (auto& el : v_channel)
				packet_func::channel_broadcast(*el, p, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::onHeartBeat][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	return;
};

void game_server::onStart() {

	try {

#if INTPTR_MAX == INT64_MAX && MY_GG_SRV_LIB == 0
		if (m_game_guard != nullptr)
			m_game_guard->start();
#endif

		sGhostCalcBridge.start(13360);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::onStart][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

bool game_server::checkCommand(std::stringstream& _command) {

	std::string s = "";

	_command >> s;

	if (!s.empty() && s.compare("exit") == 0)
		return true;
	else if (!s.empty() && s.compare("reload_files") == 0)
		reload_files();
	else if (!s.empty() && s.compare("reload_socket_config") == 0) {

		if (m_accept_sock != nullptr)
			m_accept_sock->reload_config_file();
		else
			_smp::message_pool::getInstance().push(new message("[game_server::checkCommand][WARNING] m_accept_sock(socket que gerencia os socket que pode aceitar etc) is invalid.", CL_FILE_LOG_AND_CONSOLE));

	}else if (!s.empty() && s.compare("rate") == 0) {

		std::string sTipo = "";

		_command >> sTipo;

		int32_t tipo = -1;

		if (!sTipo.empty()) {

			if (sTipo.compare("pang") == 0) {
				tipo = 0l;
			}else if (sTipo.compare("exp") == 0) {
				tipo = 1l;
			}else if (sTipo.compare("club") == 0) {
				tipo = 2l;
			}else if (sTipo.compare("chuva") == 0) {
				tipo = 3l;
			}else  if (sTipo.compare("treasure") == 0) {
				tipo = 4l;
			}else if (sTipo.compare("scratchy") == 0) {
				tipo = 5l;
			}else if (sTipo.compare("pprareitem") == 0) {
				tipo = 6l;
			}else if (sTipo.compare("ppcookieitem") == 0) {
				tipo = 7l;
			}else if (sTipo.compare("memorial") == 0) {
				tipo = 8l;
			}

		}else
			_smp::message_pool::getInstance().push(new message("[game_server::checkCommand][Error] Unknown Command: \"rate " + sTipo + "\"", CL_ONLY_CONSOLE));

		if (tipo != -1l && (tipo >= 0l && tipo <= 8l)) {

			uint32_t qntd = 0u;

			_command >> qntd;

			if (qntd > 0)
				updateRateAndEvent((uint32_t)tipo, qntd);
			else
				_smp::message_pool::getInstance().push(new message("[game_server::checkCommand][Error] Unknown value, Command: \"rate " + sTipo + "\"" + std::to_string(qntd), CL_ONLY_CONSOLE));

		}else
			_smp::message_pool::getInstance().push(new message("[game_server::checkCommand][Error] Unknown Command: \"rate " + sTipo + "\"", CL_ONLY_CONSOLE));

	}else if (!s.empty() && s.compare("event") == 0) {

		s = "";

		_command >> s;

		if (!s.empty() && s.compare("grand_zodiac_event") == 0) {

			uint32_t qntd = 0u;

			_command >> qntd;

			updateRateAndEvent(9u , qntd);

		}else if (!s.empty() && s.compare("angel_event") == 0) {

			uint32_t qntd = 0u;

			_command >> qntd;

			updateRateAndEvent(10u , qntd);

		}else if (!s.empty() && s.compare("grand_prix") == 0) {

			uint32_t qntd = 0u;

			_command >> qntd;

			updateRateAndEvent(11u , qntd);

		}else if (!s.empty() && s.compare("golden_time") == 0) {

			uint32_t qntd = 0u;

			_command >> qntd;

			updateRateAndEvent(12u , qntd);

		}else if (!s.empty() && s.compare("login_reward") == 0) {

			uint32_t qntd = 0u;

			_command >> qntd;

			updateRateAndEvent(13u , qntd);

		}else if (!s.empty() && s.compare("bot_gm_event") == 0) {

			uint32_t qntd = 0u;

			_command >> qntd;

			updateRateAndEvent(14u , qntd);

		}else if (!s.empty() && s.compare("smart_calc") == 0) {

			uint32_t qntd = 0u;

			_command >> qntd;

			updateRateAndEvent(15u , qntd);

		}else
			_smp::message_pool::getInstance().push(new message("[game_server::checkCommand][Error] Unknown Comamnd: \"Event " + s + "\"", CL_ONLY_CONSOLE));

	}else if (!s.empty() && s.compare("reload_system") == 0) {

		std::string sTipo = "";

		_command >> sTipo;

		int32_t tipo = -1;

		if (!sTipo.empty()) {

			if (sTipo.compare("all") == 0) {
				tipo = 0l;
			}else if (sTipo.compare("iff") == 0) {
				tipo = 1l;
			}else if (sTipo.compare("card") == 0) {
				tipo = 2l;
			}else if (sTipo.compare("comet_refill") == 0) {
				tipo = 3l;
			}else if (sTipo.compare("papel_shop") == 0) {
				tipo = 4l;
			}else  if (sTipo.compare("box") == 0) {
				tipo = 5l;
			}else if (sTipo.compare("memorial_shop") == 0) {
				tipo = 6l;
			}else if (sTipo.compare("cube_coin") == 0) {
				tipo = 7l;
			}else if (sTipo.compare("treasure_hunter") == 0) {
				tipo = 8l;
			}else if (sTipo.compare("drop") == 0) {
				tipo = 9l;
			}else if (sTipo.compare("attendance_reward") == 0) {
				tipo = 10l;
			}else if (sTipo.compare("map_course") == 0) {
				tipo = 11l;
			}else if (sTipo.compare("approach_mission") == 0) {
				tipo = 12l;
			}else if (sTipo.compare("grand_zodiac_event") == 0) {
				tipo = 13l;
			}else if (sTipo.compare("coin_cube_location") == 0) {
				tipo = 14l;
			}else if (sTipo.compare("golden_time") == 0) {
				tipo = 15l;
			}else if (sTipo.compare("login_reward") == 0) {
				tipo = 16l;
			}else if (sTipo.compare("bot_gm_event") == 0) {
				tipo = 17l;
			}else if (sTipo.compare("smart_calc") == 0) {
				tipo = 18l;
			}

		}else
			_smp::message_pool::getInstance().push(new message("[game_server::checkCommand][Error] Unknown Command: \"reload_system " + sTipo + "\"", CL_ONLY_CONSOLE));

		if (tipo != -1l && (tipo >= 0l && tipo <= 18l))
			reloadGlobalSystem((uint32_t)tipo);
		else
			_smp::message_pool::getInstance().push(new message("[game_server::checkCommand][Error] Unknown Command: \"reload_system " + sTipo + "\"", CL_ONLY_CONSOLE));

	}else if (!s.empty() && s.compare("smart_calc") == 0) {

		std::string sTipo = "";

		_command >> sTipo;

		if (!sTipo.empty()) {

			if (m_si.rate.smart_calculator) {

				if (sTipo.compare("reload") == 0)
					sSmartCalculator::getInstance().load();
				else if (sTipo.compare("close") == 0) {

					sSmartCalculator::getInstance().setStop(true);

					sSmartCalculator::getInstance().close();

				}else if (sTipo.compare("start") == 0)
					sSmartCalculator::getInstance().load();
				else if (sTipo.compare("chat_discord") == 0) {

					m_chat_discord = !m_chat_discord;

					_smp::message_pool::getInstance().push(new message("[game_server::checkCommand][Log] Chat Discord Flag agora esta " + std::string(m_chat_discord ? "Ativado" : "Desativado"), CL_ONLY_CONSOLE));

				}else
					_smp::message_pool::getInstance().push(new message("[game_server::checkCommand][Error] Unknown Command: \"smart_calc " + sTipo + "\"", CL_ONLY_CONSOLE));

			}else
				_smp::message_pool::getInstance().push(new message("[game_server::checkCommand][Error] Smart Calculator not active, exec Command Event smart_calc to active it.", CL_ONLY_CONSOLE));

		}else
			_smp::message_pool::getInstance().push(new message("[game_server::checkCommand][Error] Unknown Command: \"smart_calc " + sTipo + "\"", CL_ONLY_CONSOLE));

	}else if (!s.empty() && s.compare("snapshot") == 0) {

		try {
			int *bad_ptr_snapshot = nullptr;
			*bad_ptr_snapshot = 2;
		}catch (exception& e) {
			UNREFERENCED_PARAMETER(e);

			_smp::message_pool::getInstance().push(new message("[game_server::checkCommand][Log] Snapshot comando executado.", CL_FILE_LOG_AND_CONSOLE));
		}

	}
#ifdef _DEBUG
	else if (!s.empty() && s.compare("broadcast_tst") == 0) {

		auto getItemName = [](uint32_t _typeid) -> std::string {

			std::string ret = "";

			auto base = sIff::getInstance().findCommomItem(_typeid);

			if (base != nullptr)
				ret = std::string(base->name);

			return ret;
		};

		constexpr char MESSAGE_BOT_GM_EVENT_START_PART1[] = "Bot GM Event comecou, o jogo comeca em ";
		constexpr char MESSAGE_BOT_GM_EVENT_START_PART2[] = " minutos. Os premios sao ";

		auto duration_event_interval = 2;

		auto reward = sBotGMEvent::getInstance().calculeReward();

		std::string reward_str = "{";

		for (auto it_r = reward.begin(); it_r != reward.end(); ++it_r) {

			if (it_r != reward.begin())
				reward_str += ", [";
			else
				reward_str += "[";

			reward_str += it_r->toString() + "]";
		}

		reward_str += "}";

		_smp::message_pool::getInstance().push(new message("[game_server::checkCommand][Log] Bot GM Event Broadcast teste, criando a sala agora, o jogo comeca em "
				+ std::to_string(duration_event_interval) + " minutos. Reward(" + std::to_string(reward.size()) + ")" + reward_str, CL_FILE_LOG_AND_CONSOLE));

		std::string msg = MESSAGE_BOT_GM_EVENT_START_PART1
			+ std::to_string(duration_event_interval)
			+ MESSAGE_BOT_GM_EVENT_START_PART2
			+ [&getItemName](std::vector< stReward >& _el) {

				std::string ret = "";

				for (auto it_r = _el.begin(); it_r != _el.end(); ++it_r) {

					if (it_r != _el.begin())
						ret += ", ";

					ret += getItemName(it_r->_typeid) + "(" + (it_r->qntd_time > 0 ? std::to_string(it_r->qntd_time) + "day" : std::to_string(it_r->qntd)) + ")";
				}

				return ret;
			}(reward);

		packet p((unsigned short)0x1D3);

		p.addUint32(1);

		p.addUint32(eBROADCAST_TYPES::BT_MESSAGE_PLAIN);
		p.addString(msg);

		for (auto& el : v_channel)
			packet_func::channel_broadcast(*el, p, 1);
	}
	else if (!s.empty() && s.compare("scratch") == 0) {

		packet p((unsigned short)0x1EB);

		p.addUint32(0u);
		p.addUint8(0u);

		for (auto& c : v_channel)
			packet_func::channel_broadcast(*c, p, 1);
	}
	else if (!s.empty() && s.compare("upt_coin_cube_location") == 0) {

		sCoinCubeLocationUpdateSystem::getInstance().forceUpdate();
	}
	else if (!s.empty() && s.compare("msg_gz") == 0) {

		std::string sTipo = "";

		_command >> sTipo;

		int32_t tipo = -1;

		uint32_t opt_broadcast = 0u;

		if (!sTipo.empty())
			opt_broadcast = atoi(sTipo.c_str());

		packet p;

		uint32_t count = 1u;

			p.init_plain((unsigned short)0x1D3);

			p.addUint32(count);

			for (auto i = 0u; i < count; ++i) {

				p.addUint32(opt_broadcast);
				p.addString("<PARAMS><RunningTime>10</RunningTime><TYPEID>469762063</TYPEID><BOX_TYPEID>436207963</BOX_TYPEID><QTY>7</QTY><NICKNAME>Saitama(rock)</NICKNAME><NICKNAME>Genos(idea)</NICKNAME><EVENTSTARTTIME>2017-07-29 17:50:31.447</EVENTSTARTTIME><EVENTNEXTTIME>2017-07-29 19:50:31.447</EVENTNEXTTIME><WinnerEventNotice>Parabens</WinnerEventNotice><BroadCastReservedNoticesIdx>531</BroadCastReservedNoticesIdx></PARAMS>");
			}

			for (auto& el : v_channel)
				packet_func::channel_broadcast(*el, p, 1);

			_smp::message_pool::getInstance().push(new message("[game_server::checkCommand][Log] Broadcast Msg ID=" + std::to_string(opt_broadcast), CL_FILE_LOG_AND_CONSOLE));

	}
#endif
	else
		_smp::message_pool::getInstance().push(new message("[game_server::checkCommand][Error] Unknown Command: " + s, CL_ONLY_CONSOLE));

	return false;
};

bool game_server::checkPacket(session& _session, packet* _packet) {

	if (

		_session.m_check_packet.checkPacketId(_packet->getTipo())) {

		uint32_t limit_count = CHK_PCKT_COUNT_LIMIT;

		switch (_packet->getTipo()) {
		case 0xB9:
			limit_count += 100;
			break;
		case 0x03:
			limit_count += 10;
			break;
		case 0x0A:
			limit_count += 20;
			break;
		case 0x1C:
			limit_count += 10;
			break;
		case 0x2D:
			limit_count += 10;
			break;
		case 0x2F:
			limit_count += 10;
			break;
		case 0x20:
			limit_count += 50;
			break;
		case 0x18:
			limit_count += 50;
			break;
		case 0x14:
			limit_count += 150;
			break;
		case 0x16:
			limit_count += 30;
			break;
		case 0x48:
			limit_count += 30;
			break;
		case 0xBA:
			limit_count += 20;
			break;
		case 0xEB:
			limit_count += 100;
			break;
		case 0x63:
			limit_count += 150;
			break;
		case 0x143:
		case 0x146:
		case 0x147:
			limit_count += 20;
			break;
		case 0x12D:
			limit_count += 5;
			break;
		case 0x12E:
			limit_count += 20;
			break;
		default:
			limit_count += 2;
		}

		if (_session.m_check_packet.incrementCount() >= limit_count ) {

			_session.m_check_packet.clearLast();

			_smp::message_pool::getInstance().push(new message("[WARNING] Player[UID=" + std::to_string(((player*)&_session)->m_pi.uid) + "] Tentativa de DDoS ataque com pacote ID: (0x"
					+ hex_util::lltoaToHex(_packet->getTipo()) + ") " + std::to_string(_packet->getTipo()) + ". IP=" + std::string(_session.m_ip), CL_FILE_LOG_AND_CONSOLE));

			DisconnectSession(&_session);

			return false;
		}
	}

	return true;
};

void game_server::config_init() {

	m_si.tipo = 1;

	m_si.img_no = m_reader_ini.readInt("SERVERINFO", "ICONINDEX");

	try {

		m_si.flag.ullFlag = m_reader_ini.readBigInt("SERVERINFO", "FLAG");

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::config_init][ErrorSystem] SERVERINFO->FLAG" + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	try {

		m_GameGuardAuth = (m_reader_ini.readInt("SERVERINFO", "GAMEGUARDAUTH") >= 1 ? true : false);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::config_init][ErrorSystem] SERVERINFO->GAMEGUARDAUTH. " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	CmdRateConfigInfo cmd_rci(m_si.uid, true);

	snmdb::NormalManagerDB::getInstance().add(0, &cmd_rci, nullptr, nullptr);

	cmd_rci.waitEvent();

	if (cmd_rci.getException().getCodeError() != 0 || cmd_rci.isError() ) {

		if (cmd_rci.getException().getCodeError() != 0)
			_smp::message_pool::getInstance().push(new message("[game_server::config_init][ErrorSystem] " + cmd_rci.getException().getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		_smp::message_pool::getInstance().push(new message("[game_server::config_init][Error] nao conseguiu recuperar os valores de rate do server[UID="
				+ std::to_string(m_si.uid) + "] no banco de dados. Utilizando valores padroes de rates.", CL_FILE_LOG_AND_CONSOLE));

		m_si.rate.scratchy = 100;
		m_si.rate.papel_shop_rare_item = 100;
		m_si.rate.papel_shop_cookie_item = 100;
		m_si.rate.treasure = 100;
		m_si.rate.memorial_shop = 100;
		m_si.rate.chuva = 100;
		m_si.rate.grand_zodiac_event_time = 1u;
		m_si.rate.grand_prix_event = 1u;
		m_si.rate.golden_time_event = 1u;
		m_si.rate.login_reward_event = 1u;
		m_si.rate.bot_gm_event = 1u;
		m_si.rate.smart_calculator = 1u;

		setAngelEvent(0u);
		setRatePang(100u);
		setRateExp(100u);
		setRateClubMastery(100u);

		snmdb::NormalManagerDB::getInstance().add(8, new CmdUpdateRateConfigInfo(m_si.uid, m_si.rate), game_server::SQLDBResponse, this);

	}else {

		m_si.rate.scratchy = cmd_rci.getInfo().scratchy;
		m_si.rate.papel_shop_rare_item = cmd_rci.getInfo().papel_shop_rare_item;
		m_si.rate.papel_shop_cookie_item = cmd_rci.getInfo().papel_shop_cookie_item;
		m_si.rate.treasure = cmd_rci.getInfo().treasure;
		m_si.rate.memorial_shop = cmd_rci.getInfo().memorial_shop;
		m_si.rate.chuva = cmd_rci.getInfo().chuva;
		m_si.rate.grand_zodiac_event_time = cmd_rci.getInfo().grand_zodiac_event_time;
		m_si.rate.grand_prix_event = cmd_rci.getInfo().grand_prix_event;
		m_si.rate.golden_time_event = cmd_rci.getInfo().golden_time_event;
		m_si.rate.login_reward_event = cmd_rci.getInfo().login_reward_event;
		m_si.rate.bot_gm_event = cmd_rci.getInfo().bot_gm_event;
		m_si.rate.smart_calculator = cmd_rci.getInfo().smart_calculator;

		setAngelEvent(cmd_rci.getInfo().angel_event);
		setRatePang(cmd_rci.getInfo().pang);
		setRateExp(cmd_rci.getInfo().exp);
		setRateClubMastery(cmd_rci.getInfo().club_mastery);
	}

	_smp::message_pool::getInstance().push(new message("[game_server::config_init][Log] Rate Config Info [" + m_si.rate.toString() + "]", CL_FILE_LOG_AND_CONSOLE));

	m_si.app_rate = 100;
};

void game_server::reload_files() {

	server::config_init();
	config_init();

	reload_systems();

	packet p((unsigned short)0xF9);

	p.addBuffer(&m_si, sizeof(ServerInfo));

	for (auto& el : v_channel)
		packet_func::channel_broadcast(*el, p, 1);
};

void game_server::init_systems() {

	auto lixo = sRandomGen::getInstance().isGood();

	if (!sIff::getInstance().isLoad())
		sIff::getInstance().load();

	if (!sCardSystem::getInstance().isLoad())
		sCardSystem::getInstance().load();

	if (!sCometRefillSystem::getInstance().isLoad())
		sCometRefillSystem::getInstance().load();

	if (!sPapelShopSystem::getInstance().isLoad())
		sPapelShopSystem::getInstance().load();

	if (!sBoxSystem::getInstance().isLoad())
		sBoxSystem::getInstance().load();

	if (!sMemorialSystem::getInstance().isLoad())
		sMemorialSystem::getInstance().load();

	if (!sCubeCoinSystem::getInstance().isLoad())
		sCubeCoinSystem::getInstance().load();

	if (!sTreasureHunterSystem::getInstance().isLoad())
		sTreasureHunterSystem::getInstance().load();

	if (!sDropSystem::getInstance().isLoad())
		sDropSystem::getInstance().load();

	if (!sAttendanceRewardSystem::getInstance().isLoad())
		sAttendanceRewardSystem::getInstance().load();

	if (!sMap::getInstance().isLoad())
		sMap::getInstance().load();

	if (!sApproachMissionSystem::getInstance().isLoad())
		sApproachMissionSystem::getInstance().load();

	if (!sGrandZodiacEvent::getInstance().isLoad())
		sGrandZodiacEvent::getInstance().load();

	if (!sCoinCubeLocationUpdateSystem::getInstance().isLoad())
		sCoinCubeLocationUpdateSystem::getInstance().load();

	if (!sGoldenTimeSystem::getInstance().isLoad())
		sGoldenTimeSystem::getInstance().load();

	if (!sLoginRewardSystem::getInstance().isLoad())
		sLoginRewardSystem::getInstance().load();

	if (!sBotGMEvent::getInstance().isLoad())
		sBotGMEvent::getInstance().load();

	RoomGrandPrix::initFirstInstance();

	RoomGrandZodiacEvent::initFirstInstance();

	RoomBotGMEvent::initFirstInstance();

};

void game_server::reload_systems() {

	sIff::getInstance().load();

	sCardSystem::getInstance().load();

	sCometRefillSystem::getInstance().load();

	sPapelShopSystem::getInstance().load();

	sBoxSystem::getInstance().load();

	sMemorialSystem::getInstance().load();

	sCubeCoinSystem::getInstance().load();

	sTreasureHunterSystem::getInstance().load();

	sDropSystem::getInstance().load();

	sAttendanceRewardSystem::getInstance().load();

	sMap::getInstance().load();

	sApproachMissionSystem::getInstance().load();

	sGrandZodiacEvent::getInstance().load();

	sCoinCubeLocationUpdateSystem::getInstance().load();

	sGoldenTimeSystem::getInstance().load();

	sLoginRewardSystem::getInstance().load();

	sBotGMEvent::getInstance().load();

	if (m_si.rate.smart_calculator)
		sSmartCalculator::getInstance().load();
};

void game_server::reloadGlobalSystem(uint32_t _tipo) {

	try {

		switch (_tipo) {
		case 0:
			reload_systems();
			break;
		case 1:

			sIff::getInstance().load();
			break;
		case 2:

			sCardSystem::getInstance().load();
			break;
		case 3:

			sCometRefillSystem::getInstance().load();
			break;
		case 4:

			sPapelShopSystem::getInstance().load();
			break;
		case 5:

			sBoxSystem::getInstance().load();
			break;
		case 6:

			sMemorialSystem::getInstance().load();
			break;
		case 7:

			sCubeCoinSystem::getInstance().load();
			break;
		case 8:

			sTreasureHunterSystem::getInstance().load();
			break;
		case 9:

			sDropSystem::getInstance().load();
			break;
		case 10:

			sAttendanceRewardSystem::getInstance().load();
			break;
		case 11:

			sMap::getInstance().load();
			break;
		case 12:

			sApproachMissionSystem::getInstance().load();
			break;
		case 13:

			sGrandZodiacEvent::getInstance().load();
			break;
		case 14:

			sCoinCubeLocationUpdateSystem::getInstance().load();
			break;
		case 15:

			sGoldenTimeSystem::getInstance().load();
			break;
		case 16:

			sLoginRewardSystem::getInstance().load();
			break;
		case 17:

			sBotGMEvent::getInstance().load();
			break;
		case 18:

			sSmartCalculator::getInstance().load();
			break;
		default:
			throw exception("[game_server::reloadGlobalSystem][Error] Tipo[VALUE=" + std::to_string(_tipo) + "] desconhecido.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 400, 0));
		}

		_smp::message_pool::getInstance().push(new message("[game_server::reloadGlobalSystem][Log] Recarregou o Sistema[Tipo=" + std::to_string(_tipo) + "] com sucesso!", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::reloadGlobalSystem][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::updateRateAndEvent(uint32_t _tipo, uint32_t _qntd) {

	try {

		if (_qntd == 0u && _tipo != 9  && _tipo != 10
			&& _tipo != 11  && _tipo != 12  && _tipo != 13
			&& _tipo != 14  && _tipo != 15 )
			throw exception("[game_server::updateRateAndEvent][Error] Rate[TIPO=" + std::to_string(_tipo) + ", QNTD="
					+ std::to_string(_qntd) + "], qntd is invalid(zero).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 120, 0));

		switch (_tipo) {
		case 0:
			setRatePang(_qntd);
			break;
		case 1:
			setRateExp(_qntd);
			break;
		case 2:
			setRateClubMastery(_qntd);
			break;
		case 3:
			m_si.rate.chuva = (short)_qntd;
			break;
		case 4:
			m_si.rate.treasure = (short)_qntd;
			break;
		case 5:
			m_si.rate.scratchy = (short)_qntd;
			break;
		case 6:
			m_si.rate.papel_shop_rare_item = (short)_qntd;
			break;
		case 7:
			m_si.rate.papel_shop_cookie_item = (short)_qntd;
			break;
		case 8:
			m_si.rate.memorial_shop = (short)_qntd;
			break;
		case 9:
		{
			m_si.rate.grand_zodiac_event_time = (short)_qntd;

			if (m_si.rate.grand_zodiac_event_time)
				reloadGlobalSystem(13 );

			break;
		}
		case 10:
			setAngelEvent(_qntd);
			break;
		case 11:
			m_si.rate.grand_prix_event = (short)_qntd;
			break;
		case 12:
		{
			m_si.rate.golden_time_event = (short)_qntd;

			if (m_si.rate.golden_time_event)
				reloadGlobalSystem(15 );

			break;
		}
		case 13:
		{
			m_si.rate.login_reward_event = (short)_qntd;

			if (m_si.rate.login_reward_event)
				reloadGlobalSystem(16 );

			break;
		}
		case 14:
		{
			m_si.rate.bot_gm_event = (short)_qntd;

			if (m_si.rate.bot_gm_event)
				reloadGlobalSystem(17 );

			break;
		}
		case 15:
		{
			m_si.rate.smart_calculator = (short)_qntd;

			if (m_si.rate.smart_calculator)
				reloadGlobalSystem(18 );

			break;
		}
		default:
			throw exception("[game_server::updateRateAndEvent][Error] troca Rate[TIPO=" + std::to_string(_tipo) + ", QNTD="
					+ std::to_string(_qntd) + "], tipo desconhecido.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 120, 0));
		}

		snmdb::NormalManagerDB::getInstance().add(8, new CmdUpdateRateConfigInfo(m_si.uid, m_si.rate), game_server::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[game_server::updateRateAndEvent][Log] New Rate[Tipo=" + std::to_string(_tipo) + ", QNTD="
				+ std::to_string(_qntd) + "] com sucesso!", CL_FILE_LOG_AND_CONSOLE));

		packet p((unsigned short)0xF9);

		p.addBuffer(&m_si, sizeof(ServerInfo));

		for (auto& el : v_channel)
			packet_func::channel_broadcast(*el, p, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::updateRateAndEvent][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::shutdown_time(int32_t _time_sec) {

	if (_time_sec <= 0)
		shutdown();
	else {

		bool fixing_interval = false;

		uint32_t wait = 0u;
		int32_t rest = 0;

		unsigned char type = 1u;

		DWORD interval = 0u;
		float diff = 0.f;

		int32_t elapsed_sec = (m_shutdown != nullptr) ? (int)std::round(m_shutdown->getElapsed() / 1000.f)  : 0;

		_time_sec -= elapsed_sec;

		if ((diff = ((_time_sec - 600 ) / 1800.f )) >= 1.f) {

			if ((_time_sec % 1800) == 0) {

				interval = 60000 * 30;

				wait = interval * (int)diff;

			}else {

				wait = interval = (_time_sec % 1800) * 1000;

				fixing_interval = true;

			}

			type = 0u;

		}else if ((diff = ((_time_sec - 60 ) / 600.f )) >= 1.f) {

			if ((_time_sec % 600) == 0) {

				interval = 60000 * 10;

				wait = interval * (int)diff;

			}else {

				wait = interval = (_time_sec % 600) * 1000;

				fixing_interval = true;

			}

			type = 0u;

		}else if ((diff = ((_time_sec - 10 ) / 60.f )) >= 1.f) {

			if ((_time_sec % 60) == 0) {

				interval = 60 * 1000;

				wait = interval * (int)diff;

			}else {

				wait = interval = (_time_sec % 60) * 1000;

				fixing_interval = true;

			}

			type = 0u;

		}else if ((diff = ((_time_sec - 1 ) / 10.f )) >= 1.f) {

			if ((_time_sec % 10) == 0) {

				interval = 10 * 1000;

				wait = interval * (int)diff;

			}else {

				wait = interval = (_time_sec % 10) * 1000;

				fixing_interval = true;

			}

		}else {

			diff = std::round(_time_sec / 1.f);

			interval = 1000;

			wait = interval * (int)diff;

		}

		if (!fixing_interval) {

			packet p((unsigned short)0xF8);

			p.addUint8(type);

			p.addUint16((unsigned short)(type == 0 ? (unsigned short)std::round(_time_sec / 60.f) : _time_sec));

			for (auto& el : v_channel)
				packet_func::channel_broadcast(*el, p, 1);
		}

		if (m_shutdown == nullptr || m_shutdown->getState() == timer::STOP ||
				m_shutdown->getState() == timer::STOPPING || m_shutdown->getState() == timer::STOPPED) {

			job _job(server::end_time_shutdown, this, (void*)(size_t)_time_sec);

			if (m_shutdown != nullptr)
				unMakeTime(m_shutdown);

			m_shutdown = makeTime(wait, _job, std::vector< DWORD > { interval });
		}
	}

};

#if defined(_WIN32)
DWORD __stdcall game_server::_check_player(LPVOID lpParameter) {
#elif defined(__linux__)
void* game_server::_check_player(LPVOID lpParameter) {
#endif
	BEGIN_THREAD_SETUP(game_server);

	result = pTP->check_player();

	END_THREAD_SETUP("check_player");
};

#if defined(_WIN32)
DWORD game_server::check_player() {
#elif defined(__linux__)
void* game_server::check_player() {
#endif

	try {

		_smp::message_pool::getInstance().push(new message("[game_server::check_player][Log] check_player iniciado com sucesso!"));

		DWORD ret = 0u;

#if defined(_WIN32)
		while (EventShutdownServer != INVALID_HANDLE_VALUE && (ret = WaitForSingleObject(EventShutdownServer, 10000)) == WAIT_TIMEOUT) {
#elif defined(__linux__)
		while (EventShutdownServer != nullptr && (ret = EventShutdownServer->wait(10000)) == WAIT_TIMEOUT) {
#endif

			if (m_state != INITIALIZED)
				continue;

			if (m_GameGuardAuth)
				m_player_manager.checkPlayersGameGuard();

			m_player_manager.checkPlayersItens();

			m_player_manager.tickAntiCheat();
		}

		if (ret != WAIT_OBJECT_0) {

#if defined(_WIN32)
			auto last_error = GetLastError();
#elif defined(__linux__)
			auto last_error = errno;
#endif

			throw exception("[game_server::check_player][Error] Erro ao esperar por um evento com WaitForSingleObject. Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 1000, last_error));
		}

	}catch (exception& e) {
		_smp::message_pool::getInstance().push(new message("[game_server::check_player][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}catch (std::exception& e) {
		_smp::message_pool::getInstance().push(new message(std::string("[game_server::check_player][ErrorSystem] ") + e.what(), CL_FILE_LOG_AND_CONSOLE));
	}catch (...) {
		std::cout << "[game_server::check_player][ErrorSystem] check_player() -> Exception (...) c++ nao tratada ou uma excessao de C(nullptr e etc)\n";
	}

	_smp::message_pool::getInstance().push(new message("Saindo de check_player()..."));

#if defined(_WIN32)
	return 0u;
#elif defined(__linux__)
	return (void*)0u;
#endif
};

void game_server::makeGrandZodiacEventRoom() {

	try {

		auto rt = sGrandZodiacEvent::getInstance().getInterval();

		if (rt != nullptr && !rt->m_sended_message) {

			sGrandZodiacEvent::getInstance().setSendedMessage();

			rt->m_sended_message = true;

			constexpr char MESSAGE_ZODIAC_EVENT_START_PART1[] = "<PARAMS><RunningTime>";
			constexpr char MESSAGE_ZODIAC_EVENT_START_PART2[] = "</RunningTime><BroadCastReservedNoticesIdx>531</BroadCastReservedNoticesIdx></PARAMS>";

			auto duration_event_interval = rt->getDiffInterval() / (1000 * 60);

			auto resto = duration_event_interval % 10;

			if (resto == 9)
				duration_event_interval++;

			_smp::message_pool::getInstance().push(new message("[game_server::makeGrandZodiacEventRom][Log] Grand Zodiac Event comecou, vai ficar ativo por "
					+ std::to_string(duration_event_interval) + "min.", CL_FILE_LOG_AND_CONSOLE));

			std::string msg = MESSAGE_ZODIAC_EVENT_START_PART1 + std::to_string(duration_event_interval) + MESSAGE_ZODIAC_EVENT_START_PART2;

			packet p((unsigned short)0x1D3);

			p.addUint32(2u);

			for (auto i = 0u; i < 2u; ++i) {

				p.addUint32(eBROADCAST_TYPES::BT_GRAND_ZODIAC_EVENT_START_TIME);
				p.addString(msg);
			}

			for (auto& el : v_channel)
				packet_func::channel_broadcast(*el, p, 1);
		}

		if (rt != nullptr) {

			for (auto& el : v_channel)

				el->makeGrandZodiacEventRoom(*rt);

		}else
			_smp::message_pool::getInstance().push(new message("[game_server::makeGrandZodiacEventRoom][WARNING] Entrou no makeGrandZodiacEventRoom, mas nao tem nenhum range_time(interval) que o Grand Zodiac Event esta ativado.", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::makeGrandZodiacEventRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::makeListOfPlayersToGoldenTime() {

	try {

		bool is_first_message = sGoldenTimeSystem::getInstance().checkFirstMessage();

		auto current_round = sGoldenTimeSystem::getInstance().getCurrentRound();

		if (current_round == nullptr) {

			_smp::message_pool::getInstance().push(new message("[game_server::makeListOfPlayersToGoldenTime][Error] current_round(nullptr) is invalid", CL_FILE_LOG_AND_CONSOLE));

			sGoldenTimeSystem::getInstance().checkNextRound();

			return;
		}

		stRound copy_round = *current_round;

		if (is_first_message) {

			packet p((unsigned short)0x1D3);

			p.addUint32(1);

			p.addUint32(eBROADCAST_TYPES::BT_GOLDEN_TIME_START_OF_DAY);
			p.addString("<PARAMS><EVENTSTARTTIME>" + _formatDate(current_round->time) + "</EVENTSTARTTIME><TYPEID>" + std::to_string(current_round->item._typeid)
				+ "</TYPEID><QTY>" + std::to_string(current_round->item.qntd_time > 0 ? current_round->item.qntd_time : current_round->item.qntd)
				+ "</QTY></PARAMS>");

			for (auto& el : v_channel)
				packet_func::channel_broadcast(*el, p, 1);

			return;
		}else {

			packet p((unsigned short)0x1D3);

			p.addUint32(1);

			p.addUint32(eBROADCAST_TYPES::BT_GOLDEN_TIME_START_ROUND);
			p.addString("<PARAMS><EVENTSTARTTIME>" + _formatDate(current_round->time) + "</EVENTSTARTTIME><TYPEID>" + std::to_string(current_round->item._typeid)
				+ "</TYPEID><QTY>" + std::to_string(current_round->item.qntd_time > 0 ? current_round->item.qntd_time : current_round->item.qntd)
				+ "</QTY></PARAMS>");

			for (auto& el : v_channel)
				packet_func::channel_broadcast(*el, p, 1);
		}

		std::vector< stPlayerReward > players;

		for (auto& c : v_channel) {

			auto v_p = c->getAllEligibleToGoldenTime();

			if (v_p.empty())
				continue;

			players.insert(players.end(), v_p.begin(), v_p.end());
		}

		if (players.empty()) {

			_smp::message_pool::getInstance().push(new message("[game_server::makeListOfPlayersToGoldenTime][Log] Nenhum player ganhou no Golden Time Event. Round("
					+ _formatDate(current_round->time) + ")", CL_FILE_LOG_AND_CONSOLE));

			packet p((unsigned short)0x1D3);

			p.addUint32(1);

			p.addUint32(eBROADCAST_TYPES::BT_GOLDEN_TIME_ROUND_NOT_HAVE_WINNERS);
			p.addString("<PARAMS><EVENTSTARTTIME>" + _formatDate(current_round->time) + "</EVENTSTARTTIME></PARAMS>");

			for (auto& el : v_channel)
				packet_func::channel_broadcast(*el, p, 1);

		}else {

			auto printXMLAllPlayerNickname = [this](std::vector< stPlayerReward >& _winners) -> std::string {

				std::string ret = "";

				player* p = nullptr;

				for (auto& el : _winners) {

					if ((p = findPlayer(el.uid)) != nullptr)
						ret += "<NICKNAME>" + std::string(p->getNickname()) + "</NICKNAME>";
				}

				return ret;
			};

			auto printAllPlayerUID = [](std::vector< stPlayerReward >& _winners) -> std::string {

				std::string ret = "";

				bool not_first = false;

				for (auto& el : _winners) {

					ret += (not_first ? ", " : "") + std::to_string(el.uid);

					if (!not_first)
						not_first = true;
				}

				return ret;
			};

			if (players.size() > (uint32_t)m_si.max_user) {

				packet p((unsigned short)0x1D3);

				p.addUint32(1);

				p.addUint32(eBROADCAST_TYPES::BT_GOLDEN_TIME_ROUND_MORE_PEOPLE);
				p.addString("<PARAMS><EVENTSTARTTIME>" + _formatDate(current_round->time) + "</EVENTSTARTTIME></PARAMS>");

				for (auto& el : v_channel)
					packet_func::channel_broadcast(*el, p, 1);
			}

			auto reward = sGoldenTimeSystem::getInstance().calculeRoundReward(players);

			_smp::message_pool::getInstance().push(new message("[game_server::makeListOfPlayersToGoldenTime][Log] Winners Of Round[" + _formatDate(current_round->time)
					+ "] ITEM[TYPEID=" + std::to_string(current_round->item._typeid) + ", QNTD=" + std::to_string(current_round->item.qntd)
					+ ", QNTD_TIME=" + std::to_string(current_round->item.qntd_time) + "] Player(s)[" + printAllPlayerUID(reward.players) + "]", CL_FILE_LOG_AND_CONSOLE));

			packet p((unsigned short)0x1D3);

			p.addUint32(1);

			p.addUint32(eBROADCAST_TYPES::BT_GOLDEN_TIME_ROUND_REWARD_PLAYER);
			p.addString("<PARAMS><EVENTSTARTTIME>" + _formatDate(current_round->time) + "</EVENTSTARTTIME><TYPEID>" + std::to_string(current_round->item._typeid)
					+ "</TYPEID><QTY>" + std::to_string(current_round->item.qntd_time > 0 ? current_round->item.qntd_time : current_round->item.qntd)
					+ "</QTY>" + printXMLAllPlayerNickname(reward.players) + "</PARAMS>");

			for (auto& el : v_channel)
				packet_func::channel_broadcast(*el, p, 1);

			for (auto& el : v_channel)
				el->sendFireWorksWinnerGoldenTime(reward.players);

			sGoldenTimeSystem::getInstance().sendRewardToMailOfPlayers(reward);
		}

		auto next_round = sGoldenTimeSystem::getInstance().checkNextRound();

		SYSTEMTIME current{ 0u };

		GetLocalTime(&current);

		if (next_round == nullptr) {

			packet p((unsigned short)0x1D3);

			p.addUint32(1);

			p.addUint32(eBROADCAST_TYPES::BT_GOLDEN_TIME_FINISH);
			p.addString("<PARAMS><BroadCastReservedNoticesIdx>53</BroadCastReservedNoticesIdx></PARAMS>");

			for (auto& el : v_channel)
				packet_func::channel_broadcast(*el, p, 1);

		}else if (isSameDay(next_round->time, current)) {

			packet p((unsigned short)0x1D3);

			p.addUint32(1);

			p.addUint32(eBROADCAST_TYPES::BT_GOLDEN_TIME_FINISH_ROUND);
			p.addString("<PARAMS><EVENTSTARTTIME>" + _formatDate(copy_round.time) + "</EVENTSTARTTIME><EVENTNEXTTIME>"
					+ _formatDate(next_round->time) + "</EVENTNEXTTIME><TYPEID>" + std::to_string(next_round->item._typeid)
					+ "</TYPEID><QTY>" + std::to_string(next_round->item.qntd_time > 0 ? next_round->item.qntd_time : next_round->item.qntd)
					+ "</QTY></PARAMS>");

			for (auto& el : v_channel)
				packet_func::channel_broadcast(*el, p, 1);

		}else {

			packet p((unsigned short)0x1D3);

			p.addUint32(1);

			p.addUint32(eBROADCAST_TYPES::BT_GOLDEN_TIME_FINISH_OF_DAY);
			p.addString("<PARAMS><EVENTSTARTTIME>" + _formatDate(copy_round.time) + "</EVENTSTARTTIME><EVENTNEXTTIME>"
				+ _formatDate(next_round->time) + "</EVENTNEXTTIME></PARAMS>");

			for (auto& el : v_channel)
				packet_func::channel_broadcast(*el, p, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::makeListOfPlayersToGoldenTime][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::makeBotGMEventRoom() {

	auto getItemName = [](uint32_t _typeid) -> std::string {

		std::string ret = "";

		auto base = sIff::getInstance().findCommomItem(_typeid);

		if (base != nullptr)
			ret = std::string(base->name);

		return ret;
	};

	try {

		auto rt = sBotGMEvent::getInstance().getInterval();

		std::vector< stReward > reward;

		if (rt != nullptr && !rt->m_sended_message) {

			sBotGMEvent::getInstance().setSendedMessage();

			rt->m_sended_message = true;

			constexpr char MESSAGE_BOT_GM_EVENT_START_PART1[] = "Bot GM Event comecou, sala criada no canal \"";
			constexpr char MESSAGE_BOT_GM_EVENT_START_PART2[] = "\", o jogo comeca em ";
			constexpr char MESSAGE_BOT_GM_EVENT_START_PART3[] = " minutos. Os premios sao ";

			auto duration_event_interval = 2;

			reward = sBotGMEvent::getInstance().calculeReward();

			std::string reward_str = "{";

			for (auto it_r = reward.begin(); it_r != reward.end(); ++it_r) {

				if (it_r != reward.begin())
					reward_str += ", [";
				else
					reward_str += "[";

				reward_str += it_r->toString() + "]";
			}

			reward_str += "}";

			_smp::message_pool::getInstance().push(new message("[game_server::makeBotGMEventRoom][Log] Bot GM Event comecou, criando a sala no canal[ID="
					+ std::to_string((unsigned short)rt->m_channel_id) + "], o jogo comeca em " + std::to_string(duration_event_interval)
					+ " minutos. Reward(" + std::to_string(reward.size()) + ")" + reward_str, CL_FILE_LOG_AND_CONSOLE));

			std::string msg = MESSAGE_BOT_GM_EVENT_START_PART1
				+ [this, rt]() -> std::string {

					std::string ret = "Canal (Livre 1)";

					auto c = findChannel(rt->m_channel_id);

					if (c != nullptr)
						ret = c->getInfo()->name;

					return ret;
				}()
				+ MESSAGE_BOT_GM_EVENT_START_PART2
				+ std::to_string(duration_event_interval)
				+ MESSAGE_BOT_GM_EVENT_START_PART3
				+ [&getItemName](std::vector< stReward >& _el) {

					std::string ret = "";

					for (auto it_r = _el.begin(); it_r != _el.end(); ++it_r) {

						if (it_r != _el.begin())
							ret += ", ";

						ret += getItemName(it_r->_typeid) + "(" + (it_r->qntd_time > 0 ? std::to_string(it_r->qntd_time) + "day" : std::to_string(it_r->qntd)) + ")";
					}

					return ret;
				}(reward);

			packet p((unsigned short)0x1D3);

			p.addUint32(2u);

			for (auto i = 0u; i < 2u; ++i) {

				p.addUint32(eBROADCAST_TYPES::BT_MESSAGE_PLAIN);
				p.addString(msg);
			}

			for (auto& el : v_channel)
				packet_func::channel_broadcast(*el, p, 1);
		}

		if (rt != nullptr) {

			auto c = findChannel(rt->m_channel_id);

			if (c != nullptr)

				c->makeBotGMEventRoom(*rt, reward);

		}else
			_smp::message_pool::getInstance().push(new message("[game_server::makeBotGMEventRoom][WARNING] Entrou no makeBotGMEventRoom, mas nao tem nenhum stRangeTime(interval) que o Bot GM Event esta ativado.", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::makeBotGMEventRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

bool game_server::checkSmartCalculatorCmd(player& _session, std::string& _msg, eTYPE_CALCULATOR_CMD _type) {
	CHECK_SESSION_BEGIN("checkSmartCalculatorCmd");

	bool ret = false;

	try {

		if (!m_si.rate.smart_calculator)
			throw exception("[game_server::checkSmartCalculatorCmd][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] Smart Calculator esta desativado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 100002, 0));

		if (!_session.m_pi.m_cap.stBit.game_master && !_session.m_pi.m_cap.stBit.gm_normal
				&& (!_session.m_pi.m_cap.stBit.premium_user || (_type == eTYPE_CALCULATOR_CMD::SMART_CALCULATOR && !sPremiumSystem::getInstance().isPremium2(_session.m_pi.pt._typeid))
					|| (_type == eTYPE_CALCULATOR_CMD::CALCULATOR_STADIUM && !sPremiumSystem::getInstance().isPremiumTicket(_session.m_pi.pt._typeid))))
			throw exception("[game_server::checkSmartCalculatorCmd][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] nao tem permissao para executar esse comando.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 10000, 0));

		auto c = findChannel(_session.m_pi.channel);

		if (c == nullptr)
			throw exception("[game_server::checkSmartCalculatorCmd][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] nao esta em nenhum canal.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 10001, 0));

		ret = c->execSmartCalculatorCmd(_session, _msg, _type);

#ifdef _DEBUG
		if (!ret) {

			auto ctx = sSmartCalculator::getInstance().getPlayerCtx(_session.m_pi.uid, eTYPE_CALCULATOR_CMD::SMART_CALCULATOR);

			if (ctx == nullptr)
				ctx = sSmartCalculator::getInstance().makePlayerCtx(_session.m_pi.uid, eTYPE_CALCULATOR_CMD::SMART_CALCULATOR);

			sSmartCalculator::getInstance().checkCommand(_session.m_pi.uid, _msg, eTYPE_CALCULATOR_CMD::SMART_CALCULATOR);

			ret = true;

			_smp::message_pool::getInstance().push(new message("[game_server::checkSmartCalculatorCmd][Log] Player[UID="
					+ std::to_string(_session.m_pi.uid) + "] mandou o comando(" + _msg + ") para o Smart Calculator.", CL_FILE_LOG_AND_CONSOLE));
		}
#endif

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::checkSmartCalculatorCmd][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		ret = false;
	}

	return ret;
};

int game_server::betLobbyTick(void* _arg1, void*  ) {

	auto gs = reinterpret_cast< game_server* >(_arg1);

	if (gs == nullptr)
		return 0;

	try {

		CmdBetLobbyPendente cmd(true);

		snmdb::NormalManagerDB::getInstance().add(0, &cmd, nullptr, nullptr);

		cmd.waitEvent();

		if (cmd.getException().getCodeError() != 0)
			throw cmd.getException();

		if (!cmd.getInfo().empty())
			_smp::message_pool::getInstance().push(new message("[CRAZY][betLobbyTick][Log] "
					+ std::to_string(cmd.getInfo().size()) + " aposta(s) esperando sala.", CL_FILE_LOG_AND_CONSOLE));

		for (auto& b : cmd.getInfo()) {

			auto s = reinterpret_cast< player* >(gs->m_session_manager.findSessionByUID(b.criador_uid));

			if (s == nullptr || !s->getState() || !s->isConnected()) {

				_smp::message_pool::getInstance().push(new message("[CRAZY][betLobbyTick][Log] aposta[id="
						+ std::to_string(b.id) + "] criador[UID=" + std::to_string(b.criador_uid)
						+ "] nao esta online agora; tento no proximo tick.", CL_FILE_LOG_AND_CONSOLE));

				continue;
			}

			auto c = gs->findChannel(s->m_pi.channel);

			if (c == nullptr) {

				_smp::message_pool::getInstance().push(new message("[CRAZY][betLobbyTick][Log] aposta[id="
						+ std::to_string(b.id) + "] criador[UID=" + std::to_string(b.criador_uid)
						+ "] esta no channel[" + std::to_string((unsigned short)s->m_pi.channel)
						+ "], mas findChannel devolveu nullptr.", CL_FILE_LOG_AND_CONSOLE));

				continue;
			}

			short numero = c->criarSalaAposta(b.criador_uid, b.course, b.holes, b.pin);

			if (numero < 0)
				continue;

			CmdBetLobbySalaCriada upd(b.id, numero, true);

			snmdb::NormalManagerDB::getInstance().add(0, &upd, nullptr, nullptr);

			upd.waitEvent();

			if (upd.getException().getCodeError() != 0)
				throw upd.getException();
		}

		CmdBetLobbyComSala com_sala(true);

		snmdb::NormalManagerDB::getInstance().add(0, &com_sala, nullptr, nullptr);

		com_sala.waitEvent();

		if (com_sala.getException().getCodeError() != 0)
			throw com_sala.getException();

		for (auto& b : com_sala.getInfo()) {

			bool viva = false;

			for (auto c : gs->v_channel) {

				if (c != nullptr && c->salaApostaViva(b.sala_numero)) {
					viva = true;
					break;
				}
			}

			if (viva)
				continue;

			CmdBetLobbyEncerrar enc(b.id, true);

			snmdb::NormalManagerDB::getInstance().add(0, &enc, nullptr, nullptr);

			enc.waitEvent();

			if (enc.getException().getCodeError() != 0)
				throw enc.getException();

			_smp::message_pool::getInstance().push(new message("[CRAZY][betLobbyTick][Log] aposta[id="
					+ std::to_string(b.id) + "] encerrada: a sala[NUMERO="
					+ std::to_string((int)b.sala_numero) + "] dela nao existe mais.", CL_FILE_LOG_AND_CONSOLE));
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::betLobbyTick][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	return 0;
};

bool game_server::checkBetCmd(player& _session, std::string& _msg) {
	CHECK_SESSION_BEGIN("checkBetCmd");

	bool ret = false;

	try {

		if (!_session.m_pi.m_cap.stBit.game_master && !_session.m_pi.m_cap.stBit.gm_normal) {

			sendSmartCalculatorReplyToPlayer(_session.m_pi.uid, "#BET", "Comando restrito.");

			return true;
		}

		auto c = findChannel(_session.m_pi.channel);

		if (c == nullptr)
			throw exception("[game_server::checkBetCmd][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] nao esta em nenhum canal.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 10002, 0));

		ret = c->execBetCmd(_session, _msg);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::checkBetCmd][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		ret = false;
	}

	return ret;
};

bool game_server::checkGhostCalcCmd(player& _session, std::string& _msg) {
	CHECK_SESSION_BEGIN("checkGhostCalcCmd");

	bool ret = false;

	try {

		if (!_session.m_pi.m_cap.stBit.game_master && !_session.m_pi.m_cap.stBit.gm_normal
				&& (!_session.m_pi.m_cap.stBit.premium_user || !sPremiumSystem::getInstance().isPremium2(_session.m_pi.pt._typeid)))
		{

			sendSmartCalculatorReplyToPlayer(_session.m_pi.uid, "#GC",
					"Voce nao e VIP para acessar essa funcao contate o suporte.");
			return true;
		}

		auto c = findChannel(_session.m_pi.channel);

		if (c == nullptr)
			throw exception("[game_server::checkGhostCalcCmd][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] nao esta em nenhum canal.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 10001, 0));

		ret = c->execGhostCalcCmd(_session, _msg);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::checkGhostCalcCmd][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		ret = false;
	}

	return ret;
};

void game_server::destroyRoom(unsigned char _channel_owner, short _number) {

	try {

		auto c = findChannel(_channel_owner);

		if (c != nullptr)
			c->destroyRoom(_number);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::destroyRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void game_server::SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg) {

	if (_arg == nullptr) {
		_smp::message_pool::getInstance().push(new message("[game_server::SQLDBResponse][Error] _arg is nullptr", CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	if (_pangya_db.getException().getCodeError() != 0) {
		_smp::message_pool::getInstance().push(new message("[game_server::SQLDBResponse][Error] " + _pangya_db.getException().getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	auto gs = reinterpret_cast<game_server*>(_arg);

	switch (_msg_id) {
	case 1:
	{
		auto& m_dqi = reinterpret_cast<CmdDailyQuestInfo*>(&_pangya_db)->getInfo();

		if (MgrDailyQuest::checkCurrentQuest(m_dqi)) {
			MgrDailyQuest::updateDailyQuest(m_dqi);

#if defined(_WIN32)
			Sleep(100);
#elif defined(__linux__)
			usleep(100000);
#endif

			snmdb::NormalManagerDB::getInstance().add(1, new CmdDailyQuestInfo, game_server::SQLDBResponse, _arg);

			_smp::message_pool::getInstance().push(new message("[game_server::SQLDBResponse][Log] Daily Quest Atualizada.", CL_FILE_LOG_AND_CONSOLE));
		}
		break;
	}
	case 2:
	{

		gs->getDailyQuestInfo() = reinterpret_cast<CmdDailyQuestInfo*>(&_pangya_db)->getInfo();
		break;
	}
	case 3:
	{

		auto cmd_ucmu = reinterpret_cast< CmdUpdateChatMacroUser* >(&_pangya_db);

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[game_server::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_ucmu->getUID()) + "] Atualizou Chat Macro[M1="
				+ std::string(cmd_ucmu->getInfo().macro[0]) + ", M2=" + std::string(cmd_ucmu->getInfo().macro[1]) + ", M3=" + std::string(cmd_ucmu->getInfo().macro[2])
				+ ", M4=" + std::string(cmd_ucmu->getInfo().macro[3]) + ", M5=" + std::string(cmd_ucmu->getInfo().macro[4]) + ", M6=" + std::string(cmd_ucmu->getInfo().macro[5])
				+ ", M7=" + std::string(cmd_ucmu->getInfo().macro[6]) + ", M8=" + std::string(cmd_ucmu->getInfo().macro[7]) + ", M9=" + std::string(cmd_ucmu->getInfo().macro[8]) + "]", CL_FILE_LOG_AND_CONSOLE));
#else
		_smp::message_pool::getInstance().push(new message("[game_server::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_ucmu->getUID()) + "] Atualizou Chat Macro[M1="
				+ std::string(cmd_ucmu->getInfo().macro[0]) + ", M2=" + std::string(cmd_ucmu->getInfo().macro[1]) + ", M3=" + std::string(cmd_ucmu->getInfo().macro[2])
				+ ", M4=" + std::string(cmd_ucmu->getInfo().macro[3]) + ", M5=" + std::string(cmd_ucmu->getInfo().macro[4]) + ", M6=" + std::string(cmd_ucmu->getInfo().macro[5])
				+ ", M7=" + std::string(cmd_ucmu->getInfo().macro[6]) + ", M8=" + std::string(cmd_ucmu->getInfo().macro[7]) + ", M9=" + std::string(cmd_ucmu->getInfo().macro[8]) + "]", CL_ONLY_FILE_LOG));
#endif
		break;
	}
	case 4:
	{
		auto cmd_imo = reinterpret_cast< CmdInsertMsgOff* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[game_server::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_imo->getUID()) + "] enviou Message Off["
				+ cmd_imo->getMessage()  + "] para o player[UID=" + std::to_string(cmd_imo->getToUID()) + "]", CL_FILE_LOG_AND_CONSOLE));
		break;
	}
	case 5:
	{

		break;
	}
	case 6:
	{
		auto cmd_it = reinterpret_cast< CmdInsertTicker* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[game_server::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_it->getUID()) + "] enviou Ticker[MESSAGE="
				+ cmd_it->getMessage() + "] do Server[UID=" + std::to_string(cmd_it->getServerUID()) + "] para o Banco de dados com sucesso!", CL_FILE_LOG_AND_CONSOLE));

		break;
	}
	case 7:
	{

		break;
	}
	case 8:
	{

		auto cmd_urci = reinterpret_cast< CmdUpdateRateConfigInfo* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[game_server::SQLDBResponse][Log] Atualizou Rate Config Info[SERVER_UID=" + std::to_string(cmd_urci->getServerUID())
				+ ", " + cmd_urci->getInfo().toString() + "]", CL_FILE_LOG_AND_CONSOLE));

		break;
	}
	case 9:
	{
		auto cmd_ibi = reinterpret_cast<CmdInsertBlockIP*>(&_pangya_db);

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[game_server::SQLDBResponse][Log] Inseriu Block IP[IP=" + cmd_ibi->getIP()
				+ ", MASK=" + cmd_ibi->getMask() + "] com sucesso.", CL_FILE_LOG_AND_CONSOLE));
#else
		_smp::message_pool::getInstance().push(new message("[game_server::SQLDBResponse][Log] Inseriu Block IP[IP=" + cmd_ibi->getIP()
				+ ", MASK=" + cmd_ibi->getMask() + "] com sucesso.", CL_ONLY_FILE_LOG));
#endif

		break;
	}
	case 10:
	{
		auto cmd_ibm = reinterpret_cast<CmdInsertBlockMAC*>(&_pangya_db);

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[game_server::SQLDBResponse][Log] Inseriu Block MAC[ADDRESS=" + cmd_ibm->getMACAddress() + "] com sucesso.", CL_FILE_LOG_AND_CONSOLE));
#else
		_smp::message_pool::getInstance().push(new message("[game_server::SQLDBResponse][Log] Inseriu Block MAC[ADDRESS=" + cmd_ibm->getMACAddress() + "] com sucesso.", CL_ONLY_FILE_LOG));
#endif

		break;
	}
	case 0:
	default:
		break;
	}
};
