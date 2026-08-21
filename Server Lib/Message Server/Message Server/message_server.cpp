
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#include <mstcpip.h>
#elif defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#endif

#include "message_server.hpp"
#include "../../Projeto IOCP/UTIL/exception.h"
#include "../../Projeto IOCP/TYPE/stda_error.h"
#include "../../Projeto IOCP/UTIL/message_pool.h"

#include "../../Projeto IOCP/TYPE/stdAType.h"

#include "../PACKET/packet_func_ms.hpp"

#include "../../Projeto IOCP/DATABASE/normal_manager_db.hpp"

#include "../../Projeto IOCP/PANGYA_DB/cmd_insert_block_ip.hpp"
#include "../../Projeto IOCP/PANGYA_DB/cmd_update_rate_config_info.hpp"
#include "../../Projeto IOCP/PANGYA_DB/cmd_rate_config_info.hpp"

#include "../PANGYA_DB/cmd_player_info.hpp"
#include "../PANGYA_DB/cmd_friend_info.hpp"

#include "../../Projeto IOCP/Smart Calculator/Smart Calculator.hpp"

#include "../../Projeto IOCP/PANGYA_DB/cmd_verify_nick.hpp"

#define CHECK_SESSION_BEGIN(method) if (!_session.getState()) \
										throw exception("[message_server::" + std::string((method)) +"][Error] player nao esta connectado.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1, 0)); \

#define REQUEST_BEGIN(method) CHECK_SESSION_BEGIN(std::string("request") + (method)) \
							  if (_packet == nullptr) \
									throw exception("[message_server::request" + std::string((method)) +"][Error] _packet is nullptr", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 6, 0)); \

#define CHECK_SESSION_IS_AUTHORIZED(method) if (!_session.m_is_authorized) \
												throw exception("[message_server::request" + std::string((method)) + "][Error] Player[UID=" + std::to_string(_session.m_pi.uid) \
														+ "] Nao esta autorizado a fazer esse request por que ele ainda nao fez o login com o Server. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1, 0x5000501)); \

using namespace stdA;

message_server::message_server() : server(m_player_manager, 1, 12, 4), m_player_manager(*this, m_si.max_user) {

	if (m_state == FAILURE) {
		_smp::message_pool::getInstance().push(new message("[message_server::message_server][Error] falha ao incializar o message server.", CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	try {

		config_init();

		if (!sIff::getInstance().isLoad())
			sIff::getInstance().load();

		packet_func::funcs.addPacketCall(0x12, packet_func::packet012, this);
		packet_func::funcs.addPacketCall(0x13, packet_func::packet013, this);
		packet_func::funcs.addPacketCall(0x14, packet_func::packet014, this);
		packet_func::funcs.addPacketCall(0x16, packet_func::packet016, this);
		packet_func::funcs.addPacketCall(0x17, packet_func::packet017, this);
		packet_func::funcs.addPacketCall(0x18, packet_func::packet018, this);
		packet_func::funcs.addPacketCall(0x19, packet_func::packet019, this);
		packet_func::funcs.addPacketCall(0x1A, packet_func::packet01A, this);
		packet_func::funcs.addPacketCall(0x1B, packet_func::packet01B, this);
		packet_func::funcs.addPacketCall(0x1C, packet_func::packet01C, this);
		packet_func::funcs.addPacketCall(0x1D, packet_func::packet01D, this);
		packet_func::funcs.addPacketCall(0x1E, packet_func::packet01E, this);
		packet_func::funcs.addPacketCall(0x1F, packet_func::packet01F, this);
		packet_func::funcs.addPacketCall(0x23, packet_func::packet023, this);
		packet_func::funcs.addPacketCall(0x24, packet_func::packet024, this);
		packet_func::funcs.addPacketCall(0x25, packet_func::packet025, this);
		packet_func::funcs.addPacketCall(0x28, packet_func::packet028, this);
		packet_func::funcs.addPacketCall(0x29, packet_func::packet029, this);
		packet_func::funcs.addPacketCall(0x2A, packet_func::packet02A, this);
		packet_func::funcs.addPacketCall(0x2B, packet_func::packet02B, this);
		packet_func::funcs.addPacketCall(0x2C, packet_func::packet02C, this);
		packet_func::funcs.addPacketCall(0x2D, packet_func::packet02D, this);

		packet_func::funcs_sv.addPacketCall(0x2E, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x2F, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x30, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x3B, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x3C, packet_func::packet_svFazNada, this);
		packet_func::funcs_sv.addPacketCall(0x40, packet_func::packet_svFazNada, this);

		packet_func::funcs_as.addPacketCall(0x01, packet_func::packet_as001, this);
		packet_func::funcs_as.addPacketCall(0x02, packet_func::packet_as002, this);
		packet_func::funcs_as.addPacketCall(0x03, packet_func::packet_as003, this);

		m_state = INITIALIZED;

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::message_server][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		m_state = FAILURE;
	}
}

message_server::~message_server() {
}

void message_server::requestLogin(player& _session, packet *_packet) {
	REQUEST_BEGIN("Login");

	packet p;

	try {

		uint32_t uid = _packet->readUint32();
		std::string nickname = _packet->readString();

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("UID: " + std::to_string(uid), CL_FILE_LOG_AND_CONSOLE));
		_smp::message_pool::getInstance().push(new message("NICKNAME: " + nickname, CL_FILE_LOG_AND_CONSOLE));
#else
		_smp::message_pool::getInstance().push(new message("UID: " + std::to_string(uid), CL_ONLY_FILE_LOG));
		_smp::message_pool::getInstance().push(new message("NICKNAME: " + nickname, CL_ONLY_FILE_LOG));
#endif

		if (uid == 0)
			throw exception("[message_server::requestLogin][Error] player[UID=" + std::to_string(uid) + ", NICKNAME="
					+ nickname + "] tentou logar com Server, mas o uid eh invalido. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1, 0x5200101));

		if (nickname.empty())
			throw exception("[message_server::requestLogin][Error] player[UID=" + std::to_string(uid) + ", NICKNAME="
					+ nickname + "] tentou logar com Server, mas o nickname esta vazio. Hacker ou Bug",  STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 2, 0x5200102));

		if (haveBanList(_session.getIP(), "", false ))
			throw exception("[message_server::requestLogin][Error] Player[UID=" + std::to_string(uid) + ", NICKNAME=" + nickname + ", IP=" + _session.getIP()
					+ "] tentou logar com o Server, mas ele esta com IP banido.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5, 0x5200105));

		CmdPlayerInfo cmd_pi(uid, true);

		snmdb::NormalManagerDB::getInstance().add(0, &cmd_pi, nullptr, nullptr);

		cmd_pi.waitEvent();

		if (cmd_pi.getException().getCodeError() != 0)
			throw cmd_pi.getException();

		*(player_info*)&_session.m_pi = cmd_pi.getInfo();

		if (nickname.compare(_session.m_pi.nickname) != 0)
			throw exception("[message_server::requestLogin][Error] player[UID=" + std::to_string(uid) + ", NICKNAME="
					+ nickname + "] tentou logar com Server, mas o nickname do databse[NICKNAME_DB=" + std::string(_session.m_pi.nickname) + "] eh diferente do fornecido pelo cliente. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 4, 0x5200104));

		if (_session.m_pi.block_flag.m_id_state.id_state.ull_IDState != 0) {

			if (_session.m_pi.block_flag.m_id_state.id_state.st_IDState.L_BLOCK_TEMPORARY && (_session.m_pi.block_flag.m_id_state.block_time == -1 || _session.m_pi.block_flag.m_id_state.block_time > 0)) {

				throw exception("[message_server::requestLogin][Log] Bloqueado por tempo[Time="
						+ (_session.m_pi.block_flag.m_id_state.block_time == -1 ? std::string("indeterminado") : (std::to_string(_session.m_pi.block_flag.m_id_state.block_time / 60)
						+ "min " + std::to_string(_session.m_pi.block_flag.m_id_state.block_time % 60) + "sec"))
						+ "]. player [UID=" + std::to_string(_session.m_pi.uid) + ", ID=" + std::string(_session.m_pi.id) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1029, 0));

			}else if (_session.m_pi.block_flag.m_id_state.id_state.st_IDState.L_BLOCK_FOREVER) {

				throw exception("[message_server::requestLogin][Log] Bloqueado permanente. player [UID=" + std::to_string(_session.m_pi.uid)
						+ ", ID=" + std::string(_session.m_pi.id) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1030, 0));

			}else if (_session.m_pi.block_flag.m_id_state.id_state.st_IDState.L_BLOCK_ALL_IP) {

				snmdb::NormalManagerDB::getInstance().add(1, new CmdInsertBlockIP(_session.m_ip, "255.255.255.255"), message_server::SQLDBResponse, this);

				throw exception("[message_server::requestLogin][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + ", IP=" + std::string(_session.m_ip)
						+ "] Block ALL IP que o player fizer login.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1031, 0));

			}else if (_session.m_pi.block_flag.m_id_state.id_state.st_IDState.L_BLOCK_MAC_ADDRESS) {

				throw exception("[message_server::requestLogin][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
						+ ", IP=" + std::string(_session.m_ip) + ", MAC=UNKNON] (MSG nao recebe o MAC Address do cliente) Block MAC Address que o player fizer login.",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1032, 0));

			}
		}

		auto s = HasLoggedWithOuterSocket(_session);

		if (s != nullptr) {

			_smp::message_pool::getInstance().push(new message("[message_server::requestLogin][Log] Player[UID=" + std::to_string(uid) + ", OID="
					+ std::to_string(_session.m_oid) + ", IP=" + _session.getIP() + "] que esta logando agora, ja tem uma outra session com o mesmo UID logado, desloga o outro Player[UID="
					+ std::to_string(s->getUID()) + ", OID=" + std::to_string(s->m_oid) + ", IP=" + s->getIP() + "]", CL_FILE_LOG_AND_CONSOLE));

			if (!DisconnectSession(s))
				throw exception("[message_server::requestLogin][Error] Nao conseguiu disconnectar o player[UID=" + std::to_string(s->getUID())
						+ "OID=" + std::to_string(s->m_oid) + ", IP=" + s->getIP() + "], ele pode esta com o bug do oid bloqueado, ou Session::UsaCtx bloqueado.",
						STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 3, 0x5200103));
		}

		if (m_unit_connect->isLive()) {

			m_unit_connect->getInfoPlayerOnline(_session.m_pi.server_uid, _session.m_pi.uid);

		}else
			throw exception("[message_server::requestLogin][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] tentou logar, mas nao conseguiu verificar com o Auth Server se ele estava online no Server[UID=" + std::to_string(_session.m_pi.server_uid) + "]. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 6, 0x5200106));

	}catch (exception& e) {

		p.init_plain((unsigned short)0x2F);

		p.addUint8(1);

		packet_func::session_send(p, &_session, 1);

#if defined(_WIN32)
		::shutdown(_session.m_sock, SD_BOTH);
#elif defined(__linux__)
		::shutdown(_session.m_sock.fd, SD_BOTH);
#endif

		_smp::message_pool::getInstance().push(new message("[message_server::requestLogin][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void message_server::confirmLoginOnOtherServer(player& _session, uint32_t _req_server_uid, AuthServerPlayerInfo& _aspi) {
	CHECK_SESSION_BEGIN("confirmLoginOnOtherServer");

	packet p;

	try {

		if (_aspi.uid != _session.m_pi.uid)
			throw exception("[message_server::confirmLoginOnOtherServer][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + ", REQ_UID=" + std::to_string(_aspi.uid)
					+ ", REQ_SERVER=" + std::to_string(_req_server_uid) + "] request Info player, mas nao eh o mesmo UID que foi retornado do request com o Auth Server. Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1, 0x5200201));

		if (_aspi.option != 1)
			throw exception("[message_server::confirmLoginOnOtherServer][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + ", REQ_UID=" + std::to_string(_aspi.uid)
					+ ", REQ_SERVER=" + std::to_string(_req_server_uid) + "] request Info player, mas nao esta online no outro server.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 2, 0x5200202));

		if (_aspi.id.compare(_session.m_pi.id) != 0)
			throw exception("[message_server::confirmLoginOnOtherServer][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + ", REQ_UID=" + std::to_string(_aspi.uid)
					+ ", REQ_SERVER=" + std::to_string(_req_server_uid) + "] request Info player, mas nao eh o mesmo ID[ID=" + _session.m_pi.id + ", REQ_ID=" + _aspi.id
					+ "] que foi retornado do request com o Auth Server.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 3, 0x5200203));

		if (_aspi.ip.compare(_session.getIP()) != 0)
			throw exception("[message_server::confirmLoginOnOtherServer][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + ", REQ_UID=" + std::to_string(_aspi.uid)
					+ ", REQ_SERVER=" + std::to_string(_req_server_uid) + "] request Info player, mas nao eh o mesmo IP[IP=" + _session.getIP() + ", REQ_IP=" + _aspi.ip
					+ "] que foi retornado do request com o Auth Server.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 4, 0x5200204));

		_session.m_pi.m_friend_manager.init(_session.m_pi);

		_session.m_pi.m_state = 4;

		_session.m_is_authorized = 1u;

		_smp::message_pool::getInstance().push(new message("[message_server::confirmLoginOnOtherServer][Log] player[UID=" + std::to_string(_session.m_pi.uid)
				+ ", NICKNAME=" + std::string(_session.m_pi.nickname) + "] logou com sucesso!", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x2F);

		p.addUint8(0);

		p.addUint32(_session.m_pi.uid);

		packet_func::session_send(p, &_session, 1);

	} catch (exception& e) {

		p.init_plain((unsigned short)0x2F);

		p.addUint8(1);

		packet_func::session_send(p, &_session, 1);

#if defined(_WIN32)
		::shutdown(_session.m_sock, SD_BOTH);
#elif defined(__linux__)
		::shutdown(_session.m_sock.fd, SD_BOTH);
#endif

		_smp::message_pool::getInstance().push(new message("[message_server::confirmLoginOnOtherServer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void message_server::requestFriendAndGuildMemberList(player& _session, packet *_packet) {
	REQUEST_BEGIN("FriendAndGuildMemberList");

	packet p;

	try {

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[FriendList][Log] envia lista de amigos para o player[UID=" + std::to_string(_session.m_pi.uid) + "].", CL_FILE_LOG_AND_CONSOLE));
#endif

		CHECK_SESSION_IS_AUTHORIZED("FriendAndGuildMemberList");

		auto friend_list = _session.m_pi.m_friend_manager.getAllFriendAndGuildMember();

		ManyPacket mp((const unsigned short)friend_list.size(), FRIEND_PAG_LIMIT);

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x115);

		p.addUint32(_session.m_pi.uid);
		p.addUint32(_session.m_pi.m_state);

		p.addUint8(1);

		p.addBuffer(&_session.m_pi.m_cpi, sizeof(_session.m_pi.m_cpi));

		packet_func::session_send(p, &_session, 1);

		FriendInfoEx *pFi = nullptr;

		if (mp.paginas > 0) {

			for (auto i = 0u; i < mp.paginas; i++, ++mp) {
				p.init_plain((unsigned short)0x30);

				p.addUint16(0x102);

				p.addBuffer(&mp.pag, sizeof(mp.pag));

				auto begin = friend_list.begin() + mp.index.start;
				auto end = friend_list.begin() + mp.index.end;

				for (; begin != end; ++begin) {
					p.addBuffer((*begin), sizeof(FriendInfo));

					auto s = (player*)m_player_manager.findSessionByUID((*begin)->uid);

					if (s != nullptr && (pFi = s->m_pi.m_friend_manager.findFriendInAllFriend(_session.m_pi.uid)) != nullptr && !pFi->state.stState.block) {

						p.addBuffer(&s->m_pi.m_cpi, sizeof(ChannelPlayerInfo));

						p.addUint8(s->m_pi.m_state);

						switch (s->m_pi.m_state) {
						case 0:
							(*begin)->state.stState.play = 1;
							break;
						case 1:
							(*begin)->state.stState.AFK = 1;
							break;
						case 3:
							(*begin)->state.stState.busy = 1;
							break;
						case 4:
						default:
							(*begin)->state.stState.online = 1;
						}

						(*begin)->state.stState.online = 1;

					}else {
						p.addInt16(-1);
						p.addInt32(-1);
						p.addInt32(-1);
						p.addInt8(-1);
						p.addZeroByte(64);

						p.addUint8(5);

						(*begin)->state.stState.online = 0;
					}

					p.addInt8((*begin)->cUnknown_flag);

					p.addUint8((*begin)->flag.ucFlag == 2  ? ((*begin)->uid == _session.m_pi.uid ? 1  : 0) : (*begin)->level);

					p.addUint8((*begin)->state.ucState);
					p.addUint8((*begin)->flag.ucFlag);
				}

				packet_func::session_send(p, &_session, 1);
			}

		}else {

			p.init_plain((unsigned short)0x30);

			p.addUint16(0x102);

			p.addBuffer(&mp.pag, sizeof(mp.pag));

			packet_func::session_send(p, &_session, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[channel::requestFriendAndGuildMemberList][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x102);

		p.addUint8(1);

		p.addUint32(0);

		packet_func::session_send(p, &_session, 1);
	}

}

void message_server::requestUpdateChannelPlayerInfo(player& _session, packet *_packet) {
	REQUEST_BEGIN("UpdateChannelPlayerInfo");

	packet p;

	try {

		ChannelPlayerInfo cpi{ 0 };

		_packet->readBuffer(&cpi, sizeof(cpi));

		_session.m_pi.m_cpi = cpi;

		CHECK_SESSION_IS_AUTHORIZED("UpdateChannelPlayerInfo");

#ifdef _DEBUG

		_smp::message_pool::getInstance().push(new message("[UpdateChannelPlayerInfo][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] Atualizou Channel Info[NAME="
				+ std::string(_session.m_pi.m_cpi.name) + ", ID=" + std::to_string(_session.m_pi.m_cpi.id) + ", ROOM=" + std::to_string(_session.m_pi.m_cpi.room.number)
				+ ", ROOM_TYPE=" + std::to_string(_session.m_pi.m_cpi.room.type) + ", SERVER_UID=" + std::to_string(_session.m_pi.m_cpi.server_uid) + "]", CL_FILE_LOG_AND_CONSOLE));
#endif

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x115);

		p.addUint32(_session.m_pi.uid);
		p.addUint32(_session.m_pi.m_state);

		p.addUint8(1);

		p.addBuffer(&_session.m_pi.m_cpi, sizeof(_session.m_pi.m_cpi));

		packet_func::session_send(p, &_session, 1);

		packet_func::friend_broadcast(m_player_manager.findAllFriend(_session.m_pi.m_friend_manager.getAllFriendAndGuildMember(true )), p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestUpdateChannelPlayerInfo][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x115);

		p.addUint32(_session.m_pi.uid);
		p.addUint32(_session.m_pi.m_state);

		p.addUint8(0);

		packet_func::session_send(p, &_session, 1);
	}
}

void message_server::requestUpdatePlayerState(player& _session, packet *_packet) {
	REQUEST_BEGIN("UpdatePlayerState");

	packet p;

	try {

		unsigned char state = _packet->readUint8();

		CHECK_SESSION_IS_AUTHORIZED("UpdatePlayerState");

		if (_session.m_pi.m_state != state)
			_session.m_pi.m_state = state;

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[UpdateState][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] atualizou seu status[value=" + std::to_string((unsigned short)state) + "]", CL_FILE_LOG_AND_CONSOLE));
#endif

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x115);

		p.addUint32(_session.m_pi.uid);
		p.addUint32(_session.m_pi.m_state);

		p.addUint8(1);

		p.addBuffer(&_session.m_pi.m_cpi, sizeof(_session.m_pi.m_cpi));

		packet_func::friend_broadcast(m_player_manager.findAllFriend(_session.m_pi.m_friend_manager.getAllFriendAndGuildMember(true )), p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestUpdatePlayerState][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void message_server::requestUpdatePlayerLogout(player& _session, packet *_packet) {
	REQUEST_BEGIN("UpdatePlayerLogout");

	try {

		CHECK_SESSION_IS_AUTHORIZED("UpdatePlayerLogout");

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[PlayerLogout][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "] deslogou-se", CL_FILE_LOG_AND_CONSOLE));
#endif

		sendUpdatePlayerLogoutToFriends(_session);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestUpdatePlayerLogout][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

	}
}

void message_server::requestChatFriend(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChatFriend");

	packet p;

	try {

		uint32_t uid = _packet->readUint32();
		std::string msg = _packet->readString();

		CHECK_SESSION_IS_AUTHORIZED("ChatFriend");

		if (uid == 0)
			throw exception("[message_server::requestChatFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar Message[MSG="
					+ msg + "] para o Amigo[UID=" + std::to_string(uid) + "], mas o uid is invalid(zero). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1, 0x5200301));

		if (msg.empty())
			throw exception("[message_server::requestChatFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar Message[MSG="
					+ msg + "] para o Amigo[UID=" + std::to_string(uid) + "], mas msg is empty. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 2, 0x5200302));

		auto pFi = _session.m_pi.m_friend_manager.findFriendInAllFriend(uid);

		if (pFi == nullptr)
			throw exception("[message_server::requestChatFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar Message[MSG="
					+ msg + "] para o Amigo[UID=" + std::to_string(uid) + "], mas player nao eh amigo dele. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 3, 0x5200303));

		if (pFi->state.stState.block)
			throw exception("[message_server::requestChatFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar Message[MSG="
					+ msg + "] para o Amigo[UID=" + std::to_string(uid) + "], mas o amigo esta bloqueado. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 4, 0x5200304));

		auto s = (player*)m_player_manager.findSessionByUID(uid);

		if (s == nullptr)
			throw exception("[message_server::requestChatFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar Message[MSG="
					+ msg + "] para o Amigo[UID=" + std::to_string(uid) + "], mas o Amigo nao esta online.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5, 0x5200305));

		pFi = s->m_pi.m_friend_manager.findFriendInAllFriend(_session.m_pi.uid);

		if (pFi == nullptr)
			throw exception("[message_server::requestChatFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar Message[MSG="
					+ msg + "] para o Amigo[UID=" + std::to_string(uid) + "], mas o amigo nao tem ele na lista de amigos. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 6, 0x5200306));

		if (pFi->state.stState.block)
			throw exception("[message_server::requestChatFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar Message[MSG="
					+ msg + "] para o Amigo[UID=" + std::to_string(uid) + "], mas amigo bloqueou ele. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE, 7, 0x5200307));

		auto gm = m_player_manager.findAllGM();

		if (!gm.empty()) {

			std::string msg_gm = "\\5" + std::string(_session.m_pi.nickname) + ">" + std::string(s->m_pi.nickname) + ": '" + msg + "'";

			for (auto& el : gm) {

				if (el->m_pi.uid != _session.m_pi.uid && el->m_pi.uid != s->m_pi.uid) {

					p.init_plain((unsigned short)0x40);

					p.addUint8(0);

					p.addString("\\1[MSN->PM]");

					p.addString(msg_gm);

					packet_func::session_send(p, el, 1);
				}
			}
		}

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[ChatFriend][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] enviou Message[MSG="
				+ msg + "] para seu Amigo[UID=" + std::to_string(s->m_pi.uid) + "]", CL_FILE_LOG_AND_CONSOLE));
#endif

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x113);

		p.addUint32(_session.m_pi.uid);
		p.addString(_session.m_pi.nickname);
		p.addString(msg);

		p.addUint8(0);

		packet_func::session_send(p, s, 1);

		if (m_si.rate.smart_calculator && m_chat_discord)
			sendMessageToDiscordChatHistory(
				"[MSN->PM]",
				std::string(_session.m_pi.nickname) + ">" + std::string(s->m_pi.nickname) + ": '" + msg + "'"
			);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestChatFriend][ErrorSystem] " + e.getFullMessageError() , CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x113);

		p.addInt32(-1);

		packet_func::session_send(p, &_session, 1);
	}
}

void message_server::requestChatGuild(player& _session, packet *_packet) {
	REQUEST_BEGIN("ChatGuild");

	packet p;

	try {

		std::string msg = _packet->readString();

		CHECK_SESSION_IS_AUTHORIZED("ChatGuild");

		if (_session.m_pi.guild_uid == 0)
			throw exception("[message_server::requestChatGuild][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar Message[MSG="
					+ msg + "] para o Chat da Guild[UID=" + std::to_string(_session.m_pi.guild_uid) + "], mas o player nao esta em uma guild. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1, 0x5200401));

		if (msg.empty())
			throw exception("[message_server::requestChatGuild][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou enviar Message[MSG="
					+ msg + "] para o Chat da Guild[UID=" + std::to_string(_session.m_pi.guild_uid) + "], mas a msg is empty. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 2, 0x5200402));

		auto gm = m_player_manager.findAllGM();

		if (!gm.empty()) {

			auto guild_name = std::string(_session.m_pi.guild_name);

			size_t index = std::string::npos;

			while ((index = guild_name.find(' ', (index != std::string::npos ? index + 1 : 0u))) != std::string::npos)
				guild_name.replace(index, 1, " \\2");

			std::string msg_gm = "[\\2" + guild_name  + "\\0]\\5>" + std::string(_session.m_pi.nickname) + ": '" + msg + "'";

			for (auto& el : gm) {

				if (el->m_pi.uid != _session.m_pi.uid && el->m_pi.guild_uid != _session.m_pi.guild_uid) {

					p.init_plain((unsigned short)0x40);

					p.addUint8(0);

					p.addString("\\1[CC]");

					p.addString(msg_gm);

					packet_func::session_send(p, el, 1);
				}
			}
		}

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[ChatGuild][Log] player[UID=" + std::to_string(_session.m_pi.uid) +"] enviu Message[MSG=" + msg + "] no Chat da Guild[UID="
				+ std::to_string(_session.m_pi.guild_uid) + "]", CL_FILE_LOG_AND_CONSOLE));
#endif

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x113);

		p.addUint32(_session.m_pi.uid);
		p.addString(_session.m_pi.nickname);
		p.addString(msg);

		p.addUint8(1);

		packet_func::session_send(p, &_session, 1);

		packet_func::friend_broadcast(m_player_manager.findAllGuildMember(_session.m_pi.guild_uid), p, &_session, 1);

		if (m_si.rate.smart_calculator && m_chat_discord)
			sendMessageToDiscordChatHistory(
				"[CC]",
				"[" + std::string(_session.m_pi.guild_name) + "]>" + std::string(_session.m_pi.nickname) + ": '" + msg + "'"
			);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestChatGuild][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x113);

		p.addUint32(-1);

		packet_func::session_send(p, &_session, 1);
	}
}

void message_server::requestCheckNickname(player& _session, packet *_packet) {
	REQUEST_BEGIN("CheckNickname");

	packet p;
	std::string nickname = "";

	try {

		nickname = _packet->readString();

		CHECK_SESSION_IS_AUTHORIZED("CheckNickname");

		if (nickname.empty())
			throw exception("[message_server::requestCheckNickname][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou verificar o Nickname[value="
					+ nickname + "], mas o nickname is empty. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1, 0x5200501));

		CmdVerifNick cmd_vn(nickname, true);

		snmdb::NormalManagerDB::getInstance().add(0, &cmd_vn, nullptr, nullptr);

		cmd_vn.waitEvent();

		if (cmd_vn.getException().getCodeError() != 0)
			throw cmd_vn.getException();

		if (!cmd_vn.getLastCheck())
			throw exception("[message_server::requestCheckNickname][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou verificar o Nickname[value="
				+ nickname + "], mas o nickname nao existe.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 2, 1));

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[CheckNickname][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] pediu para verificar o Nickname[value=" + nickname + "]", CL_FILE_LOG_AND_CONSOLE));
#endif

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x117);

		p.addUint32(0);

		p.addString(nickname);
		p.addUint32(cmd_vn.getUID());

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestCheckNickname][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x117);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::MESSAGE_SERVER) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5200500);

		p.addString(nickname);

		packet_func::session_send(p, &_session, 1);
	}
}

void message_server::requestAssingApelido(player& _session, packet *_packet) {
	REQUEST_BEGIN("AssingApelido");

	packet p;

	try {

		uint32_t uid = _packet->readUint32();
		std::string apelido = _packet->readString();

		CHECK_SESSION_IS_AUTHORIZED("AssingApelido");

		if (uid == 0)
			throw exception("[message_server::requestAssingApelido][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou da um apelido para o Amigo[UID="
					+ std::to_string(uid) + ", APELIDO=" + apelido + "], mas o uid is invalid(zero). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1, 0x5200901));

		if (apelido.empty())
			throw exception("[message_server::requestAssingApelido][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou da um apelido para o Amigo[UID="
					+ std::to_string(uid) + ", APELIDO=" + apelido + "], mas o apelido is empty. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 2, 0x5200902));

		if (apelido.size() >= 11)
			throw exception("[message_server::requestAssingApelido][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou da um apelido para o Amigo[UID="
					+ std::to_string(uid) + ", APELIDO=" + apelido + "], mas o comprimento do apelido[max=11, request=" + std::to_string(apelido.size()) + "] eh invalido.",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 3, 0x5200903));

		auto pFi = _session.m_pi.m_friend_manager.findFriend(uid);

		if (pFi == nullptr)
			throw exception("[message_server::requestAssingApelido][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou da um apelido para o Amigo[UID="
					+ std::to_string(uid) + ", APELIDO=" + apelido + "], mas ele nao tem esse player como amigo. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 4, 0x5200903));

		{
			size_t _n = apelido.size() < sizeof(pFi->apelido) ? apelido.size() : sizeof(pFi->apelido) - 1;
			memset(pFi->apelido, 0, sizeof(pFi->apelido));
			memcpy(pFi->apelido, apelido.data(), _n);
		}

		_session.m_pi.m_friend_manager.requestUpdateFriendInfo(*pFi);

		_smp::message_pool::getInstance().push(new message("[AssingApelido][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] colocou apelido[VALUE="
				+ apelido + "] no Amigo[UID="  + std::to_string(pFi->uid) + ", NICKNAME=" + std::string(pFi->nickname) + "]", CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x119);

		p.addUint32(0);

		p.addUint32(pFi->uid);
		p.addString(pFi->apelido);

		packet_func::session_send(p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestAssingApelido][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x119);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::MESSAGE_SERVER) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5200900);

		packet_func::session_send(p, &_session, 1);
	}
}

void message_server::requestBlockFriend(player& _session, packet *_packet) {
	REQUEST_BEGIN("BlockFriend");

	packet p;

	try {

		uint32_t uid = _packet->readUint32();

		CHECK_SESSION_IS_AUTHORIZED("BlockFriend");

		if (uid == 0)
			throw exception("[message_server::requestBlockFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou bloqueiar Amigo[UID="
					+ std::to_string(uid) + "], mas o uid is invalid(zero). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1, 0x5300101));

		auto pFi = _session.m_pi.m_friend_manager.findFriend(uid);

		if (pFi == nullptr)
			throw exception("[message_server::requestBlockFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou bloqueiar Amigo[UID="
				+ std::to_string(uid) + "], mas o player nao eh amigo dele. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 2, 0x5300102));

		if (pFi->state.stState.block)
			throw exception("[message_server::requestBlockFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou bloqueiar Amigo[UID="
					+ std::to_string(uid) + "], mas o amigo ja esta bloqueado. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 3, 0x5300103));

		auto s = (player*)m_player_manager.findSessionByUID(uid);

		FriendInfoEx *pFi2 = nullptr;

		if (s != nullptr) {

			if ((pFi2 = s->m_pi.m_friend_manager.findFriend(_session.m_pi.uid)) == nullptr)
				throw exception("[message_server::requestBlockFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou bloqueiar Amigo[UID="
						+ std::to_string(uid) + "], mas o amigo nao tem ele na lista de amigos. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 4, 0x5300104));

			pFi->state.stState.block = 1;

			_session.m_pi.m_friend_manager.requestUpdateFriendInfo(*pFi);

			_smp::message_pool::getInstance().push(new message("[BlockFriend][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] bloqueou o Amigo[UID="
					+ std::to_string(s->m_pi.uid) + ", NICKNAME=" + std::string(s->m_pi.nickname) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x30);

			p.addUint16(0x10C);

			p.addUint32(0);

			p.addUint32(s->m_pi.uid);

			packet_func::session_send(p, &_session, 1);

			p.init_plain((unsigned short)0x30);

			p.addUint16(0x10F);

			p.addUint32(_session.m_pi.uid);

			packet_func::session_send(p, s, 1);

		}else {

			CmdPlayerInfo cmd_pi(uid, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_pi, nullptr, nullptr);

			cmd_pi.waitEvent();

			if (cmd_pi.getException().getCodeError() != 0)
				throw cmd_pi.getException();

			auto pi = cmd_pi.getInfo();

			if (pi.uid == 0)
				throw exception("[message_server::requestBlockFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou bloqueiar Amigo[UID="
						+ std::to_string(uid) + "], mas player nao existe. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5, 0x5300105));

			FriendManager fm(pi);

			fm.init(pi);

			if (!fm.isInitialized())
				throw exception("[message_server::requestBlockFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou bloqueiar Amigo[UID="
						+ std::to_string(uid) + "], nao conseguiu inicializar Friend Manager do amigo. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 6, 0x5300106));

			if ((pFi2 = fm.findFriend(_session.m_pi.uid)) == nullptr)
				throw exception("[message_server::requestBlockFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou bloqueiar Amigo[UID="
						+ std::to_string(uid) + "], mas o amigo nao tem ele na lista de amigos. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 4, 0x5300104));

			pFi->state.stState.block = 1;

			_session.m_pi.m_friend_manager.requestUpdateFriendInfo(*pFi);

			_smp::message_pool::getInstance().push(new message("[BlockFriend][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] bloqueou o Amigo[UID="
					+ std::to_string(pi.uid) + ", NICKNAME=" + std::string(pi.nickname) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x30);

			p.addUint16(0x10C);

			p.addUint32(0);

			p.addUint32(pi.uid);

			packet_func::session_send(p, &_session, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestBlockFriend][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x10C);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::MESSAGE_SERVER) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5300100);

		packet_func::session_send(p, &_session, 1);
	}
}

void message_server::requestUnblockFriend(player& _session, packet *_packet) {
	REQUEST_BEGIN("UnblockFriend");

	packet p;

	try {

		uint32_t uid = _packet->readUint32();

		CHECK_SESSION_IS_AUTHORIZED("UnblockFriend");

		if (uid == 0)
			throw exception("[message_server::requestUnBlockFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou desbloquear Amigo[UID="
					+ std::to_string(uid) + "], mas uid is invalid(zero). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1, 0x5300201));

		auto pFi = _session.m_pi.m_friend_manager.findFriend(uid);

		if (pFi == nullptr)
			throw exception("[message_server::requestUnBlockFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou desbloquear Amigo[UID="
					+ std::to_string(uid) + "], mas o player nao eh amigo dele. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 2, 0x5300202));

		if (!pFi->state.stState.block)
			throw exception("[message_server::requestUnBlockFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou desbloquear Amigo[UID="
					+ std::to_string(uid) + "], mas o amigo ja esta desbloqueado. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 3, 0x5300203));

		auto s = (player*)m_player_manager.findSessionByUID(uid);

		FriendInfoEx *pFi2 = nullptr;

		if (s != nullptr) {

			if ((pFi2 = s->m_pi.m_friend_manager.findFriend(_session.m_pi.uid)) == nullptr)
				throw exception("[message_server::requestUnBlockFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou desbloquear Amigo[UID="
						+ std::to_string(uid) + "], mas o amigo nao tem ele na lista de amigos. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 4, 0x5200204));

			pFi->state.stState.block = 0;

			_session.m_pi.m_friend_manager.requestUpdateFriendInfo(*pFi);

			_smp::message_pool::getInstance().push(new message("[UnBlockFriend][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] desbloqueou o Amigo[UID="
					+ std::to_string(s->m_pi.uid) + ", NICKNAME=" + std::string(s->m_pi.nickname) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x30);

			p.addUint16(0x10D);

			p.addUint32(0);

			p.addUint32(s->m_pi.uid);

			packet_func::session_send(p, &_session, 1);

			p.init_plain((unsigned short)0x30);

			p.addUint16(0x115);

			p.addUint32(_session.m_pi.uid);
			p.addUint32(_session.m_pi.m_state);

			p.addUint8(1);

			p.addBuffer(&_session.m_pi.m_cpi, sizeof(_session.m_pi.m_cpi));

			packet_func::session_send(p, s, 1);

		}else {

			CmdPlayerInfo cmd_pi(uid, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_pi, nullptr, nullptr);

			cmd_pi.waitEvent();

			if (cmd_pi.getException().getCodeError() != 0)
				throw cmd_pi.getException();

			auto pi = cmd_pi.getInfo();

			if (pi.uid == 0)
				throw exception("[message_server::requestUnBlockFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou desbloquear Amigo[UID="
						+ std::to_string(uid) + "], mas o player nao existe. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5, 0x5300205));

			FriendManager fm(pi);

			fm.init(pi);

			if (!fm.isInitialized())
				throw exception("[message_server::requestUnBlockFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou desbloquear Amigo[UID="
						+ std::to_string(uid) + "], mas nao conseguiu inicializar Friend Manager do amigo. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 6, 0x5300206));

			if ((pFi2 = fm.findFriend(_session.m_pi.uid)) == nullptr)
				throw exception("[message_server::requestUnBlockFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou desbloquear Amigo[UID="
						+ std::to_string(uid) + "], mas o amigo nao tem ele na lista de amigos. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 4, 0x5300204));

			pFi->state.stState.block = 0;

			_session.m_pi.m_friend_manager.requestUpdateFriendInfo(*pFi);

			_smp::message_pool::getInstance().push(new message("[UnBlockFriend][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] desbloqueou o Amigo[UID="
					+ std::to_string(pi.uid) + ", NICKNAME=" + std::string(pi.nickname) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x30);

			p.addUint16(0x10D);

			p.addUint32(0);

			p.addUint32(pi.uid);

			packet_func::session_send(p, &_session, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestUnblockFriend][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x10D);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::MESSAGE_SERVER) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5300200);

		packet_func::session_send(p, &_session, 1);
	}
}

void message_server::requestAddFriend(player& _session, packet *_packet) {
	REQUEST_BEGIN("AddFriend");

	packet p;

	try {

		uint32_t uid = _packet->readUint32();
		std::string nickname = _packet->readString();

		CHECK_SESSION_IS_AUTHORIZED("AddFriend");

		if (uid == 0)
			throw exception("[message_server::requestAddFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou add Friend[UID="
					+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas o uid is invalid(zero). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1, 0x5200601));

		if (nickname.empty())
			throw exception("[message_server::requestAddFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou add Friend[UID="
					+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas o nickname is empty. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 2, 0x5200602));

		auto pFi = _session.m_pi.m_friend_manager.findFriendInAllFriend(uid);

		if (pFi != nullptr && pFi->flag.stFlag._friend)
			throw exception("[message_server::requestAddFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou add Friend[UID="
				+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas o player ja eh amigo dele.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 3, 2));

		if (_session.m_pi.m_friend_manager.countFriend() >= FRIEND_LIST_LIMIT)
			throw exception("[message_server::requestAddFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou add Friend[UID="
					+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas ele esta com a lista de amigos cheia[LIMIT=" + std::to_string(FRIEND_LIST_LIMIT) + "].", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 4, 0x5200603));

		auto s = (player*)m_player_manager.findSessionByUID(uid);

		FriendInfoEx fi{ 0 }, fi2{ 0 };

		if (s != nullptr) {

#if defined(_WIN32)
			if (_stricmp(nickname.c_str(), s->m_pi.nickname) != 0)
#elif defined(__linux__)
			if (strcasecmp(nickname.c_str(), s->m_pi.nickname) != 0)
#endif
				throw exception("[message_server::requestAddFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou add Friend[UID="
						+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas o nickname nao bate. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE, 7, 0x5200607));

			if (s->m_pi.m_friend_manager.countFriend() >= FRIEND_LIST_LIMIT)
				throw exception("[message_server::requestAddFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou add Friend[UID="
						+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas o amigo esta com a lista full[LIMIT=" + std::to_string(FRIEND_LIST_LIMIT) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5, 3));

			fi.uid = s->m_pi.uid;
			fi.flag.ucFlag = (pFi == nullptr) ? 1 : pFi->flag.ucFlag | 1;

#if defined(_WIN32)
			strncpy_s(fi.apelido, sizeof(fi.apelido), "Friend", sizeof(fi.apelido));
			strncpy_s(fi.nickname, sizeof(fi.nickname), s->m_pi.nickname, sizeof(fi.nickname));
#elif defined(__linux__)
			strncpy(fi.apelido, "Friend", sizeof(fi.apelido));
			strncpy(fi.nickname, s->m_pi.nickname, sizeof(fi.nickname));
#endif

			fi.state.stState.online = 1;
			fi.state.stState.request_friend = 1;
			fi.state.stState.sex = s->m_pi.sex;

			fi.level = (unsigned char)s->m_pi.level;

			fi2.uid = _session.m_pi.uid;
			fi2.flag.ucFlag = (pFi == nullptr) ? 1 : pFi->flag.ucFlag | 1;

#if defined(_WIN32)
			strncpy_s(fi2.apelido, sizeof(fi2.apelido), "Friend", sizeof(fi2.apelido));
			strncpy_s(fi2.nickname, sizeof(fi2.nickname), _session.m_pi.nickname, sizeof(fi2.nickname));
#elif defined(__linux__)
			strncpy(fi2.apelido, "Friend", sizeof(fi2.apelido));
			strncpy(fi2.nickname, _session.m_pi.nickname, sizeof(fi2.nickname));
#endif

			fi2.state.stState.online = 1;
			fi2.state.stState.sex = _session.m_pi.sex;

			fi2.level = (unsigned char)_session.m_pi.level;

			_session.m_pi.m_friend_manager.requestAddFriend(fi);
			s->m_pi.m_friend_manager.requestAddFriend(fi2);

			_smp::message_pool::getInstance().push(new message("[AddFriend][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] add Amigo[UID=" + std::to_string(s->m_pi.uid) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x30);

			p.addUint16(0x104);

			p.addUint32(0);

			p.addBuffer(&fi, sizeof(FriendInfo));

			p.addBuffer(&s->m_pi.m_cpi, sizeof(ChannelPlayerInfo));

			p.addUint8(s->m_pi.m_state);

			p.addInt8(fi.cUnknown_flag);
			p.addUint8(fi.level);
			p.addUint8(fi.state.ucState);
			p.addUint8(fi.flag.ucFlag);

			packet_func::session_send(p, &_session, 1);

			p.init_plain((unsigned short)0x30);

			p.addUint16(0x106);

			p.addBuffer(&fi2, sizeof(FriendInfo));

			p.addBuffer(&_session.m_pi.m_cpi, sizeof(ChannelPlayerInfo));

			p.addUint8(_session.m_pi.m_state);

			p.addInt8(fi2.cUnknown_flag);
			p.addUint8(fi2.level);
			p.addUint8(fi2.state.ucState);
			p.addUint8(fi2.flag.ucFlag);

			packet_func::session_send(p, s, 1);

		}else {

			CmdPlayerInfo cmd_pi(uid, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_pi, nullptr, nullptr);

			cmd_pi.waitEvent();

			if (cmd_pi.getException().getCodeError() != 0)
				throw cmd_pi.getException();

			auto pi = cmd_pi.getInfo();

			if (pi.uid == 0)
				throw exception("[message_server::requestAddFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou add Friend[UID="
						+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas o player nao existe.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 6, 0x5200606));

#if defined(_WIN32)
			if (_stricmp(nickname.c_str(), pi.nickname) != 0)
#elif defined(__linux__)
			if (strcasecmp(nickname.c_str(), pi.nickname) != 0)
#endif
				throw exception("[message_server::requestAddFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou add Friend[UID="
					+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas o nickname nao bate. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE, 7, 0x5200607));

			FriendManager fm(pi);

			fm.init(pi);

			if (!fm.isInitialized())
				throw exception("[message_server::requestAddFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou add Friend[UID="
						+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas nao conseguiu inicializar o FriendManager do Amigo.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 8, 0x5200607));

			if (fm.countFriend() >= FRIEND_LIST_LIMIT)
				throw exception("[message_server::requestAddFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou add Friend[UID="
					+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas o amigo esta com a lista full[LIMIT=" + std::to_string(FRIEND_LIST_LIMIT) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5, 3));

			fi.uid = pi.uid;
			fi.flag.ucFlag = (pFi == nullptr) ? 1 : pFi->flag.ucFlag | 1;

#if defined(_WIN32)
			memcpy_s(fi.apelido, sizeof(fi.apelido), "Friend", 7);
			memcpy_s(fi.nickname, sizeof(fi.nickname), pi.nickname, sizeof(fi.nickname));
#elif defined(__linux__)
			memcpy(fi.apelido, "Friend", 7);
			memcpy(fi.nickname, pi.nickname, sizeof(fi.nickname));
#endif

			fi.state.stState.online = 1;
			fi.state.stState.request_friend = 1;
			fi.state.stState.sex = pi.sex;

			fi.level = (unsigned char)pi.level;

			fi2.uid = _session.m_pi.uid;
			fi2.flag.ucFlag = (pFi == nullptr) ? 1 : pFi->flag.ucFlag | 1;

#if defined(_WIN32)
			memcpy_s(fi2.apelido, sizeof(fi2.apelido), "Friend", 7);
			memcpy_s(fi2.nickname, sizeof(fi2.nickname), _session.m_pi.nickname, sizeof(fi2.nickname));
#elif defined(__linux__)
			memcpy(fi2.apelido, "Friend", 7);
			memcpy(fi2.nickname, _session.m_pi.nickname, sizeof(fi2.nickname));
#endif

			fi2.state.stState.online = 1;
			fi2.state.stState.sex = _session.m_pi.sex;

			fi2.level = (unsigned char)_session.m_pi.level;

			_session.m_pi.m_friend_manager.requestAddFriend(fi);
			fm.requestAddFriend(fi2);

			_smp::message_pool::getInstance().push(new message("[AddFriend][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] add Amigo[UID=" + std::to_string(pi.uid) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x30);

			p.addUint16(0x104);

			p.addUint32(0);

			p.addBuffer(&fi, sizeof(FriendInfo));

			p.addInt16(-1);
			p.addInt32(-1);
			p.addInt32(-1);
			p.addInt8(-1);
			p.addZeroByte(64);

			p.addUint8(5);

			fi.state.stState.online = 0;

			p.addInt8(fi.cUnknown_flag);
			p.addUint8(fi.level);
			p.addUint8(fi.state.ucState);
			p.addUint8(fi.flag.ucFlag);

			packet_func::session_send(p, &_session, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestAddFriend][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x104);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::MESSAGE_SERVER) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5200600);

		packet_func::session_send(p, &_session, 1);
	}
}

void message_server::requestConfirmFriend(player& _session, packet *_packet) {
	REQUEST_BEGIN("ConfirmFriend");

	packet p;

	try {

		uint32_t uid = _packet->readUint32();

		CHECK_SESSION_IS_AUTHORIZED("ConfirmFriend");

		if (uid == 0)
			throw exception("[message_server::requestConfirmFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou aceitar Amigo[UID="
					+ std::to_string(uid) + "], mas o uid is invalid(zero). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1, 0x5200801));

		auto pFi = _session.m_pi.m_friend_manager.findFriend(uid);

		if (pFi == nullptr)
			throw exception("[message_server::requestConfirmFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou aceitar Amigo[UID="
					+ std::to_string(uid) + "], mas o player nao eh amigo dele. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 2, 0x5200802));

		if (pFi->state.stState.request_friend)
			throw exception("[message_server::requestConfirmFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou aceitar Amigo[UID="
					+ std::to_string(uid) + "], mas ele nao pode aceitar um amigo, que ele mesmo enviou pedido de amizade. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 3, 0x5200803));

		if (pFi->state.stState._friend)
			throw exception("[message_server::requestConfirmFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou aceitar Amigo[UID="
					+ std::to_string(uid) + "], mas o player ja eh seu amigo. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 4, 0x5200804));

		auto s = (player*)m_player_manager.findSessionByUID(uid);

		FriendInfoEx *pFi2 = nullptr;

		if (s != nullptr) {

			if ((pFi2 = s->m_pi.m_friend_manager.findFriend(_session.m_pi.uid)) == nullptr)
				throw exception("[message_server::requestConfirmFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou aceitar Amigo[UID="
						+ std::to_string(uid) + "], mas o player nao esta na lista do amigo que ele vai aceitar. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5, 0x5200804));

			pFi->state.stState._friend = 1;

			pFi2->state.stState.request_friend = 0;
			pFi2->state.stState._friend = 1;

			_session.m_pi.m_friend_manager.requestUpdateFriendInfo(*pFi);
			s->m_pi.m_friend_manager.requestUpdateFriendInfo(*pFi2);

			_smp::message_pool::getInstance().push(new message("[ConfirmFriend][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] aceitou Amigo[UID="
					+ std::to_string(s->m_pi.uid) + ", NICKNAME=" + std::string(s->m_pi.nickname) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x30);

			p.addUint16((unsigned short)0x109);

			p.addUint32(0);

			p.addUint32(s->m_pi.uid);

			packet_func::session_send(p, &_session, 1);

			p.init_plain((unsigned short)0x30);

			p.addUint16(0x10A);

			p.addUint32(0);

			p.addUint32(_session.m_pi.uid);

			packet_func::session_send(p, s, 1);

		}else {

			CmdPlayerInfo cmd_pi(uid, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_pi, nullptr, nullptr);

			cmd_pi.waitEvent();

			if (cmd_pi.getException().getCodeError() != 0)
				throw cmd_pi.getException();

			auto pi = cmd_pi.getInfo();

			if (pi.uid == 0)
				throw exception("[message_server::requestConfirmFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou aceitar Amigo[UID="
						+ std::to_string(uid) + "], mas o player nao existe. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 6, 0x5200806));

			FriendManager fm(pi);

			fm.init(pi);

			if (!fm.isInitialized())
				throw exception("[message_server::requestConfirmFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou aceitar Amigo[UID="
						+ std::to_string(uid) + "], mas nao conseguiu incializar o Friend Manager do amigo. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 7, 0x5200807));

			if ((pFi2 = fm.findFriend(_session.m_pi.uid)) == nullptr)
				throw exception("[message_server::requestConfirmFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou aceitar Amigo[UID="
						+ std::to_string(uid) + "], mas o player nao esta na lista do amigo que ele vai aceitar. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5, 0x5200805));

			pFi->state.stState._friend = 1;

			pFi2->state.stState.request_friend = 0;
			pFi2->state.stState._friend = 1;

			_session.m_pi.m_friend_manager.requestUpdateFriendInfo(*pFi);
			fm.requestUpdateFriendInfo(*pFi2);

			_smp::message_pool::getInstance().push(new message("[ConfirmFriend][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] aceitou Amigo[UID="
					+ std::to_string(pi.uid) + ", NICKNAME=" + std::string(pi.nickname) + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x30);

			p.addUint16((unsigned short)0x109);

			p.addUint32(0);

			p.addUint32(pi.uid);

			packet_func::session_send(p, &_session, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestConfirmFriend][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x109);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::MESSAGE_SERVER) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5200800);

		packet_func::session_send(p, &_session, 1);
	}
}

void message_server::requestDeleteFriend(player& _session, packet *_packet) {
	REQUEST_BEGIN("DeleteFriend");

	packet p;

	try {

		uint32_t uid = _packet->readUint32();
		std::string nickname = _packet->readString();

		CHECK_SESSION_IS_AUTHORIZED("DeleteFriend");

		if (uid == 0)
			throw exception("[message_server::requestDeleteFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar Amigo[UID="
					+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas o uid is invalid(zero). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 1, 0x5200701));

		if (nickname.empty())
			throw exception("[message_server::requestDeleteFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar Amigo[UID="
					+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas nickname is empty. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 2, 0x5200702));

		auto pFi = _session.m_pi.m_friend_manager.findFriend(uid);

		if (pFi == nullptr)
			throw exception("[message_server::requestDeleteFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar Amigo[UID="
					+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas o player nao eh amigo dele. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 3, 0x5200703));

		auto s = (player*)m_player_manager.findSessionByUID(uid);

		FriendInfoEx *pFi2 = nullptr;

		if (s != nullptr) {

#if defined(_WIN32)
			if (_stricmp(nickname.c_str(), s->m_pi.nickname) != 0)
#elif defined(__linux__)
			if (strcasecmp(nickname.c_str(), s->m_pi.nickname) != 0)
#endif
				throw exception("[message_server::requestDeleteFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar Amigo[UID="
						+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas o nickname nao bate. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE, 6, 0x5200705));

			if ((pFi2 = s->m_pi.m_friend_manager.findFriend(_session.m_pi.uid)) == nullptr)
				throw exception("[message_server::requestDeleteFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar Amigo[UID="
						+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas o amigo nao tem ele na lista de amigos. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE, 4, 0x5200704));

			_session.m_pi.m_friend_manager.requestDeleteFriend(*pFi);
			s->m_pi.m_friend_manager.requestDeleteFriend(*pFi2);

			_smp::message_pool::getInstance().push(new message("[DeleteFriend][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] deletou Amigo[UID="
					+ std::to_string(s->m_pi.uid) + ", NICKNAME=" + nickname + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x30);

			p.addUint16(0x10B);

			p.addUint32(0);

			p.addUint32(s->m_pi.uid);

			packet_func::session_send(p, &_session, 1);

			p.init_plain((unsigned short)0x30);

			p.addUint16(0x10B);

			p.addUint32(0);

			p.addUint32(_session.m_pi.uid);

			packet_func::session_send(p, s, 1);

		}else {

			CmdPlayerInfo cmd_pi(uid, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_pi, nullptr, nullptr);

			cmd_pi.waitEvent();

			if (cmd_pi.getException().getCodeError() != 0)
				throw cmd_pi.getException();

			auto pi = cmd_pi.getInfo();

			if (pi.uid == 0)
				throw exception("[message_server::requestDeleteFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar Amigo[UID="
						+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas o player nao existe. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5, 0x5200705));

#if defined(_WIN32)
			if (_stricmp(nickname.c_str(), pi.nickname) != 0)
#elif defined(__linux__)
			if (strcasecmp(nickname.c_str(), pi.nickname) != 0)
#endif
				throw exception("[message_server::requestDeleteFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar Amigo[UID="
						+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas o nickname nao bate. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 6, 0x5200706));

			FriendManager fm(pi);

			fm.init(pi);

			if (!fm.isInitialized())
				throw exception("[message_server::requestDeleteFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar Amigo[UID="
						+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas nao conseguiu incializar o Friend Manager do Amigo. Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 7, 0x5200707));

			if ((pFi2 = fm.findFriend(_session.m_pi.uid)) == nullptr)
				throw exception("[message_server::requestDeleteFriend][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou deletar Amigo[UID="
						+ std::to_string(uid) + ", NICKNAME=" + nickname + "], mas o amigo nao tem ele na lista de amigos. Hacker ou Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE, 8, 0x5200708));

			_session.m_pi.m_friend_manager.requestDeleteFriend(*pFi);
			fm.requestDeleteFriend(*pFi2);

			_smp::message_pool::getInstance().push(new message("[DeleteFriend][Log] player[UID=" + std::to_string(_session.m_pi.uid) + "] deletou Amigo[UID="
					+ std::to_string(pi.uid) + ", NICKNAME=" + nickname + "]", CL_FILE_LOG_AND_CONSOLE));

			p.init_plain((unsigned short)0x30);

			p.addUint16(0x10B);

			p.addUint32(0);

			p.addUint32(pi.uid);

			packet_func::session_send(p, &_session, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestDeleteFriend][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x10B);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::MESSAGE_SERVER) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5200700);

		packet_func::session_send(p, &_session, 1);
	}
}

void message_server::requestNotityPlayerWasInvitedToRoom(player& _session, packet *_packet) {
	REQUEST_BEGIN("NotifyPlayerWasInvitedToRoom");

	try {

		CHECK_SESSION_IS_AUTHORIZED("NotifyPlayerWasInvitedToRoom");

		uint32_t player_invited_uid = _packet->readUint32();

		if (player_invited_uid != _session.m_pi.uid)
			throw exception("[message_server::requestNotityPlayerWasInvitedToRoom][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] que foi convidado passou um Player[UID=" + std::to_string(player_invited_uid)
					+ "] com uid que nao eh o dele. Hacker ou Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 3749, 0));

		_smp::message_pool::getInstance().push(new message("[message_server::requestNotityPlayerWasInvitedToRoom][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
				+ "] foi convidado para um sala no jogo.", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestNotifyPlayerWasInvitedToRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void message_server::requestInvitPlayerToGuildBattleRoom(player& _session, packet *_packet) {
	REQUEST_BEGIN("InvitPlayerToGuildBattleRoom");

	try {

		CHECK_SESSION_IS_AUTHORIZED("InvitPlayerToGuildBattleRoom");

		uint32_t server_uid = _packet->readUint32();
		unsigned char channel_id = _packet->readUint8();
		unsigned short room_numero = _packet->readUint16();

		uint32_t player_invite_uid = _packet->readUint32();
		std::string player_invite_nickname = _packet->readString();

		uint32_t player_invited_uid = _packet->readUint32();

		if (player_invite_uid != _session.m_pi.uid)
			throw exception("[message_server::requestInvitPlayerToGuildBattleRoom][Error] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] nao bate com o Player[UID=" + std::to_string(player_invite_uid) + "] que fez o request para convidar o player[UID="
					+ std::to_string(player_invited_uid) + "] para a sala de Guild Battle. Hacker ou Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 3750, 0));

		_smp::message_pool::getInstance().push(new message("[message_server::requestInvitPlayerToGuildBattleRoom][Log] Player[UID="
				+ std::to_string(_session.m_pi.uid) + ", NICKNAME=" + player_invite_nickname + "] convidou o Player[UID="
				+ std::to_string(player_invited_uid) + "] no Server[UID=" + std::to_string(server_uid) + ", CHANNEL_ID="
				+ std::to_string((unsigned short)channel_id) + ", ROOM=" + std::to_string(room_numero) + "] para Guild Battle.", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestInvitPlayerToGuildBattleRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

#define REQUEST_AUTH_COMMAND_BEGIN(_method) if (_packet == nullptr) \
	throw exception("[message_server::request" + std::string((_method)) + "][Error] _packet is invalid.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5400, 0));

void message_server::requestAcceptGuildMember(packet *_packet) {
	REQUEST_AUTH_COMMAND_BEGIN("AcceptGuildMember");

	packet p;

	try {

		uint32_t club_id = _packet->readUint32();
		uint32_t member_uid = _packet->readUint32();

		if (club_id == 0u)
			throw exception("[message_server::requestAcceptGuildMember][Error] club_id is invalid(zero).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5401, 0));

		if (member_uid == 0u)
			throw exception("[message_server::requestAcceptGuildMember][Error] member_uid is invalid(zero).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5401, 0));

		auto v_cm = m_player_manager.findAllGuildMember(club_id);

		if (v_cm.empty())
			_smp::message_pool::getInstance().push(new message("[message_server::requestAcceptGuildMember][WARNING] Club[ID=" + std::to_string(club_id)
					+ "] nao tem nenhum membro online para atualizar.", CL_FILE_LOG_AND_CONSOLE));

		for (auto& el : v_cm)
			if (el.second != nullptr)
				el.second->m_pi.m_friend_manager.init(el.second->m_pi);

		auto s = m_player_manager.findPlayer(member_uid);

		if (s == nullptr ||
#if defined(_WIN32)
			s->m_sock == INVALID_SOCKET
#elif defined(__linux__)
			s->m_sock.fd == INVALID_SOCKET
#endif
		) {

			CmdPlayerInfo cmd_pi(member_uid, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_pi, nullptr, nullptr);

			cmd_pi.waitEvent();

			if (cmd_pi.getException().getCodeError() != 0)
				throw cmd_pi.getException();

			auto pi = cmd_pi.getInfo();

			{
				for (auto& el : v_cm) {

					if (el.second != nullptr) {

						auto friend_list = el.second->m_pi.m_friend_manager.getAllFriendAndGuildMember();

						ManyPacket mp((const unsigned short)friend_list.size(), FRIEND_PAG_LIMIT);

						FriendInfoEx *pFi = nullptr;

						if (mp.paginas > 0) {

							for (auto i = 0u; i < mp.paginas; i++, ++mp) {
								p.init_plain((unsigned short)0x30);

								p.addUint16(0x102);

								p.addBuffer(&mp.pag, sizeof(mp.pag));

								auto begin = friend_list.begin() + mp.index.start;
								auto end = friend_list.begin() + mp.index.end;

								for (; begin != end; ++begin) {
									p.addBuffer((*begin), sizeof(FriendInfo));

									auto s = (player*)m_player_manager.findSessionByUID((*begin)->uid);

									if (s != nullptr && (pFi = s->m_pi.m_friend_manager.findFriendInAllFriend(el.second->m_pi.uid)) != nullptr && !pFi->state.stState.block) {

										p.addBuffer(&s->m_pi.m_cpi, sizeof(ChannelPlayerInfo));

										p.addUint8(s->m_pi.m_state);

										switch (s->m_pi.m_state) {
										case 0:
											(*begin)->state.stState.play = 1;
											break;
										case 1:
											(*begin)->state.stState.AFK = 1;
											break;
										case 3:
											(*begin)->state.stState.busy = 1;
											break;
										case 4:
										default:
											(*begin)->state.stState.online = 1;
										}

										(*begin)->state.stState.online = 1;

									}else {
										p.addInt16(-1);
										p.addInt32(-1);
										p.addInt32(-1);
										p.addInt8(-1);
										p.addZeroByte(64);

										p.addUint8(5);

										(*begin)->state.stState.online = 0;
									}

									p.addInt8((*begin)->cUnknown_flag);

									p.addUint8((*begin)->flag.ucFlag == 2  ? ((*begin)->uid == el.second->m_pi.uid ? 1  : 0) : (*begin)->level);

									p.addUint8((*begin)->state.ucState);
									p.addUint8((*begin)->flag.ucFlag);
								}

								packet_func::session_send(p, el.second, 1);
							}

						}else {

							p.init_plain((unsigned short)0x30);

							p.addUint16(0x102);

							p.addBuffer(&mp.pag, sizeof(mp.pag));

							packet_func::session_send(p, el.second, 1);
						}
					}
				}
			}

			p.init_plain((unsigned short)0x3B);

			p.addUint32(pi.uid);
			p.addUint32(club_id);
			p.addUint8(pi.sex);
			p.addString(pi.id);
			p.addString(pi.nickname);
			p.addUint16(0x1F);

			packet_func::friend_broadcast(v_cm, p, (session*)1 , 1);

		}else {

			s->m_pi.guild_uid = club_id;

			s->m_pi.m_friend_manager.init(s->m_pi);

			{
				for (auto& el : v_cm) {

					if (el.second != nullptr) {

						auto friend_list = el.second->m_pi.m_friend_manager.getAllFriendAndGuildMember();

						ManyPacket mp((const unsigned short)friend_list.size(), FRIEND_PAG_LIMIT);

						FriendInfoEx *pFi = nullptr;

						if (mp.paginas > 0) {

							for (auto i = 0u; i < mp.paginas; i++, ++mp) {
								p.init_plain((unsigned short)0x30);

								p.addUint16(0x102);

								p.addBuffer(&mp.pag, sizeof(mp.pag));

								auto begin = friend_list.begin() + mp.index.start;
								auto end = friend_list.begin() + mp.index.end;

								for (; begin != end; ++begin) {
									p.addBuffer((*begin), sizeof(FriendInfo));

									auto s = (player*)m_player_manager.findSessionByUID((*begin)->uid);

									if (s != nullptr && (pFi = s->m_pi.m_friend_manager.findFriendInAllFriend(el.second->m_pi.uid)) != nullptr && !pFi->state.stState.block) {

										p.addBuffer(&s->m_pi.m_cpi, sizeof(ChannelPlayerInfo));

										p.addUint8(s->m_pi.m_state);

										switch (s->m_pi.m_state) {
										case 0:
											(*begin)->state.stState.play = 1;
											break;
										case 1:
											(*begin)->state.stState.AFK = 1;
											break;
										case 3:
											(*begin)->state.stState.busy = 1;
											break;
										case 4:
										default:
											(*begin)->state.stState.online = 1;
										}

										(*begin)->state.stState.online = 1;

									}else {
										p.addInt16(-1);
										p.addInt32(-1);
										p.addInt32(-1);
										p.addInt8(-1);
										p.addZeroByte(64);

										p.addUint8(5);

										(*begin)->state.stState.online = 0;
									}

									p.addInt8((*begin)->cUnknown_flag);

									p.addUint8((*begin)->flag.ucFlag == 2  ? ((*begin)->uid == el.second->m_pi.uid ? 1  : 0) : (*begin)->level);

									p.addUint8((*begin)->state.ucState);
									p.addUint8((*begin)->flag.ucFlag);
								}

								packet_func::session_send(p, el.second, 1);
							}

						}else {

							p.init_plain((unsigned short)0x30);

							p.addUint16(0x102);

							p.addBuffer(&mp.pag, sizeof(mp.pag));

							packet_func::session_send(p, el.second, 1);
						}
					}
				}
			}

			p.init_plain((unsigned short)0x3B);

			p.addUint32(s->m_pi.uid);
			p.addUint32(club_id);
			p.addUint8(s->m_pi.sex);
			p.addString(s->m_pi.id);
			p.addString(s->m_pi.nickname);
			p.addUint16(0x1F);

			packet_func::friend_broadcast(v_cm, p, s, 1);
		}

		_smp::message_pool::getInstance().push(new message("[message_server::requestAcceptGuildMember][Log] Player[UID=" + std::to_string(member_uid)
				+ "] foi aceito no Club[UID=" + std::to_string(club_id) + "] com sucesso.", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestAcceptGuildMember][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void message_server::requestMemberExitedFromGuild(packet *_packet) {
	REQUEST_AUTH_COMMAND_BEGIN("MemberExitedFromGuild");

	packet p;

	try {

		uint32_t club_id = _packet->readUint32();
		uint32_t member_uid = _packet->readUint32();

		if (club_id == 0u)
			throw exception("[message_server::requestMemberExitedFromGuild][Error] club_id is invalid(zero).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5401, 0));

		if (member_uid == 0u)
			throw exception("[message_server::requestMemberExitedFromGuild][Error] member_uid is invalid(zero).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5401, 0));

		auto v_cm = m_player_manager.findAllGuildMember(club_id);

		if (v_cm.empty())
			_smp::message_pool::getInstance().push(new message("[message_server::requestMemberExitedFromGuild][WARNING] Club[ID=" + std::to_string(club_id)
					+ "] nao tem nenhum membro online para atualizar.", CL_FILE_LOG_AND_CONSOLE));

		for (auto& el : v_cm)
			if (el.second != nullptr)
				el.second->m_pi.m_friend_manager.init(el.second->m_pi);

		auto s = m_player_manager.findPlayer(member_uid);

		if (s == nullptr ||
#if defined(_WIN32)
			s->m_sock == INVALID_SOCKET
#elif defined(__linux__)
			s->m_sock.fd == INVALID_SOCKET
#endif
		) {

			CmdPlayerInfo cmd_pi(member_uid, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_pi, nullptr, nullptr);

			cmd_pi.waitEvent();

			if (cmd_pi.getException().getCodeError() != 0)
				throw cmd_pi.getException();

			auto pi = cmd_pi.getInfo();

			{
				for (auto& el : v_cm) {

					if (el.second != nullptr) {

						auto friend_list = el.second->m_pi.m_friend_manager.getAllFriendAndGuildMember();

						ManyPacket mp((const unsigned short)friend_list.size(), FRIEND_PAG_LIMIT);

						FriendInfoEx *pFi = nullptr;

						if (mp.paginas > 0) {

							for (auto i = 0u; i < mp.paginas; i++, ++mp) {
								p.init_plain((unsigned short)0x30);

								p.addUint16(0x102);

								p.addBuffer(&mp.pag, sizeof(mp.pag));

								auto begin = friend_list.begin() + mp.index.start;
								auto end = friend_list.begin() + mp.index.end;

								for (; begin != end; ++begin) {
									p.addBuffer((*begin), sizeof(FriendInfo));

									auto s = (player*)m_player_manager.findSessionByUID((*begin)->uid);

									if (s != nullptr && (pFi = s->m_pi.m_friend_manager.findFriendInAllFriend(el.second->m_pi.uid)) != nullptr && !pFi->state.stState.block) {

										p.addBuffer(&s->m_pi.m_cpi, sizeof(ChannelPlayerInfo));

										p.addUint8(s->m_pi.m_state);

										switch (s->m_pi.m_state) {
										case 0:
											(*begin)->state.stState.play = 1;
											break;
										case 1:
											(*begin)->state.stState.AFK = 1;
											break;
										case 3:
											(*begin)->state.stState.busy = 1;
											break;
										case 4:
										default:
											(*begin)->state.stState.online = 1;
										}

										(*begin)->state.stState.online = 1;

									}else {
										p.addInt16(-1);
										p.addInt32(-1);
										p.addInt32(-1);
										p.addInt8(-1);
										p.addZeroByte(64);

										p.addUint8(5);

										(*begin)->state.stState.online = 0;
									}

									p.addInt8((*begin)->cUnknown_flag);

									p.addUint8((*begin)->flag.ucFlag == 2  ? ((*begin)->uid == el.second->m_pi.uid ? 1  : 0) : (*begin)->level);

									p.addUint8((*begin)->state.ucState);
									p.addUint8((*begin)->flag.ucFlag);
								}

								packet_func::session_send(p, el.second, 1);
							}

						}else {

							p.init_plain((unsigned short)0x30);

							p.addUint16(0x102);

							p.addBuffer(&mp.pag, sizeof(mp.pag));

							packet_func::session_send(p, el.second, 1);
						}
					}
				}
			}

			p.init_plain((unsigned short)0x3C);

			p.addUint32(pi.uid);

			packet_func::friend_broadcast(v_cm, p, (session*)1 , 1);

		}else {

			s->m_pi.guild_uid = 0;

			s->m_pi.m_friend_manager.init(s->m_pi);

			{
				for (auto& el : v_cm) {

					if (el.second != nullptr) {

						auto friend_list = el.second->m_pi.m_friend_manager.getAllFriendAndGuildMember();

						ManyPacket mp((const unsigned short)friend_list.size(), FRIEND_PAG_LIMIT);

						FriendInfoEx *pFi = nullptr;

						if (mp.paginas > 0) {

							for (auto i = 0u; i < mp.paginas; i++, ++mp) {
								p.init_plain((unsigned short)0x30);

								p.addUint16(0x102);

								p.addBuffer(&mp.pag, sizeof(mp.pag));

								auto begin = friend_list.begin() + mp.index.start;
								auto end = friend_list.begin() + mp.index.end;

								for (; begin != end; ++begin) {
									p.addBuffer((*begin), sizeof(FriendInfo));

									auto s = (player*)m_player_manager.findSessionByUID((*begin)->uid);

									if (s != nullptr && (pFi = s->m_pi.m_friend_manager.findFriendInAllFriend(el.second->m_pi.uid)) != nullptr && !pFi->state.stState.block) {

										p.addBuffer(&s->m_pi.m_cpi, sizeof(ChannelPlayerInfo));

										p.addUint8(s->m_pi.m_state);

										switch (s->m_pi.m_state) {
										case 0:
											(*begin)->state.stState.play = 1;
											break;
										case 1:
											(*begin)->state.stState.AFK = 1;
											break;
										case 3:
											(*begin)->state.stState.busy = 1;
											break;
										case 4:
										default:
											(*begin)->state.stState.online = 1;
										}

										(*begin)->state.stState.online = 1;

									}else {
										p.addInt16(-1);
										p.addInt32(-1);
										p.addInt32(-1);
										p.addInt8(-1);
										p.addZeroByte(64);

										p.addUint8(5);

										(*begin)->state.stState.online = 0;
									}

									p.addInt8((*begin)->cUnknown_flag);

									p.addUint8((*begin)->flag.ucFlag == 2  ? ((*begin)->uid == el.second->m_pi.uid ? 1  : 0) : (*begin)->level);

									p.addUint8((*begin)->state.ucState);
									p.addUint8((*begin)->flag.ucFlag);
								}

								packet_func::session_send(p, el.second, 1);
							}

						}else {

							p.init_plain((unsigned short)0x30);

							p.addUint16(0x102);

							p.addBuffer(&mp.pag, sizeof(mp.pag));

							packet_func::session_send(p, el.second, 1);
						}
					}
				}
			}

			p.init_plain((unsigned short)0x3C);

			p.addUint32(s->m_pi.uid);

			packet_func::friend_broadcast(v_cm, p, s, 1);
		}

		_smp::message_pool::getInstance().push(new message("[message_server::requestMemberExitedFromGuild][Log] Player[UID=" + std::to_string(member_uid)
				+ "] saiu do Club[UID=" + std::to_string(club_id) + "] com sucesso.", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestMemberExitedFromGuild][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void message_server::requestKickGuildMember(packet *_packet) {
	REQUEST_AUTH_COMMAND_BEGIN("KickGuildMember");

	packet p;

	try {

		uint32_t club_id = _packet->readUint32();
		uint32_t member_uid = _packet->readUint32();

		if (club_id == 0u)
			throw exception("[message_server::requestKickGuildMember][Error] club_id is invalid(zero).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5401, 0));

		if (member_uid == 0u)
			throw exception("[message_server::requestKickGuildMember][Error] member_uid is invalid(zero).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 5401, 0));

		auto v_cm = m_player_manager.findAllGuildMember(club_id);

		if (v_cm.empty())
			_smp::message_pool::getInstance().push(new message("[message_server::requestKickGuildMember][WARNING] Club[ID=" + std::to_string(club_id)
					+ "] nao tem nenhum membro online para atualizar.", CL_FILE_LOG_AND_CONSOLE));

		for (auto& el : v_cm)
			if (el.second != nullptr)
				el.second->m_pi.m_friend_manager.init(el.second->m_pi);

		auto s = m_player_manager.findPlayer(member_uid);

		if (s == nullptr ||
#if defined(_WIN32)
			s->m_sock == INVALID_SOCKET
#elif defined(__linux__)
			s->m_sock.fd == INVALID_SOCKET
#endif
		) {

			CmdPlayerInfo cmd_pi(member_uid, true);

			snmdb::NormalManagerDB::getInstance().add(0, &cmd_pi, nullptr, nullptr);

			cmd_pi.waitEvent();

			if (cmd_pi.getException().getCodeError() != 0)
				throw cmd_pi.getException();

			auto pi = cmd_pi.getInfo();

			{
				for (auto& el : v_cm) {

					if (el.second != nullptr) {

						auto friend_list = el.second->m_pi.m_friend_manager.getAllFriendAndGuildMember();

						ManyPacket mp((const unsigned short)friend_list.size(), FRIEND_PAG_LIMIT);

						FriendInfoEx *pFi = nullptr;

						if (mp.paginas > 0) {

							for (auto i = 0u; i < mp.paginas; i++, ++mp) {
								p.init_plain((unsigned short)0x30);

								p.addUint16(0x102);

								p.addBuffer(&mp.pag, sizeof(mp.pag));

								auto begin = friend_list.begin() + mp.index.start;
								auto end = friend_list.begin() + mp.index.end;

								for (; begin != end; ++begin) {
									p.addBuffer((*begin), sizeof(FriendInfo));

									auto s = (player*)m_player_manager.findSessionByUID((*begin)->uid);

									if (s != nullptr && (pFi = s->m_pi.m_friend_manager.findFriendInAllFriend(el.second->m_pi.uid)) != nullptr && !pFi->state.stState.block) {

										p.addBuffer(&s->m_pi.m_cpi, sizeof(ChannelPlayerInfo));

										p.addUint8(s->m_pi.m_state);

										switch (s->m_pi.m_state) {
										case 0:
											(*begin)->state.stState.play = 1;
											break;
										case 1:
											(*begin)->state.stState.AFK = 1;
											break;
										case 3:
											(*begin)->state.stState.busy = 1;
											break;
										case 4:
										default:
											(*begin)->state.stState.online = 1;
										}

										(*begin)->state.stState.online = 1;

									}else {
										p.addInt16(-1);
										p.addInt32(-1);
										p.addInt32(-1);
										p.addInt8(-1);
										p.addZeroByte(64);

										p.addUint8(5);

										(*begin)->state.stState.online = 0;
									}

									p.addInt8((*begin)->cUnknown_flag);

									p.addUint8((*begin)->flag.ucFlag == 2  ? ((*begin)->uid == el.second->m_pi.uid ? 1  : 0) : (*begin)->level);

									p.addUint8((*begin)->state.ucState);
									p.addUint8((*begin)->flag.ucFlag);
								}

								packet_func::session_send(p, el.second, 1);
							}

						}else {

							p.init_plain((unsigned short)0x30);

							p.addUint16(0x102);

							p.addBuffer(&mp.pag, sizeof(mp.pag));

							packet_func::session_send(p, el.second, 1);
						}
					}
				}
			}

			p.init_plain((unsigned short)0x3C);

			p.addUint32(pi.uid);

			packet_func::friend_broadcast(v_cm, p, (session*)1 , 1);

		}else {

			s->m_pi.guild_uid = 0;

			s->m_pi.m_friend_manager.init(s->m_pi);

			{
				for (auto& el : v_cm) {

					if (el.second != nullptr) {

						auto friend_list = el.second->m_pi.m_friend_manager.getAllFriendAndGuildMember();

						ManyPacket mp((const unsigned short)friend_list.size(), FRIEND_PAG_LIMIT);

						FriendInfoEx *pFi = nullptr;

						if (mp.paginas > 0) {

							for (auto i = 0u; i < mp.paginas; i++, ++mp) {
								p.init_plain((unsigned short)0x30);

								p.addUint16(0x102);

								p.addBuffer(&mp.pag, sizeof(mp.pag));

								auto begin = friend_list.begin() + mp.index.start;
								auto end = friend_list.begin() + mp.index.end;

								for (; begin != end; ++begin) {
									p.addBuffer((*begin), sizeof(FriendInfo));

									auto s = (player*)m_player_manager.findSessionByUID((*begin)->uid);

									if (s != nullptr && (pFi = s->m_pi.m_friend_manager.findFriendInAllFriend(el.second->m_pi.uid)) != nullptr && !pFi->state.stState.block) {

										p.addBuffer(&s->m_pi.m_cpi, sizeof(ChannelPlayerInfo));

										p.addUint8(s->m_pi.m_state);

										switch (s->m_pi.m_state) {
										case 0:
											(*begin)->state.stState.play = 1;
											break;
										case 1:
											(*begin)->state.stState.AFK = 1;
											break;
										case 3:
											(*begin)->state.stState.busy = 1;
											break;
										case 4:
										default:
											(*begin)->state.stState.online = 1;
										}

										(*begin)->state.stState.online = 1;

									}else {
										p.addInt16(-1);
										p.addInt32(-1);
										p.addInt32(-1);
										p.addInt8(-1);
										p.addZeroByte(64);

										p.addUint8(5);

										(*begin)->state.stState.online = 0;
									}

									p.addInt8((*begin)->cUnknown_flag);

									p.addUint8((*begin)->flag.ucFlag == 2  ? ((*begin)->uid == el.second->m_pi.uid ? 1  : 0) : (*begin)->level);

									p.addUint8((*begin)->state.ucState);
									p.addUint8((*begin)->flag.ucFlag);
								}

								packet_func::session_send(p, el.second, 1);
							}

						}else {

							p.init_plain((unsigned short)0x30);

							p.addUint16(0x102);

							p.addBuffer(&mp.pag, sizeof(mp.pag));

							packet_func::session_send(p, el.second, 1);
						}
					}
				}
			}

			p.init_plain((unsigned short)0x3C);

			p.addUint32(s->m_pi.uid);

			packet_func::friend_broadcast(v_cm, p, s, 1);
		}

		_smp::message_pool::getInstance().push(new message("[message_server::requestKickGuildMember][Log] Player[UID=" + std::to_string(member_uid)
				+ "] foi chutado do Club[UID=" + std::to_string(club_id) + "] com sucesso.", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::requestKickGuildMember][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void message_server::authCmdShutdown(int32_t _time_sec) {

	try {

		if (m_shutdown == nullptr) {

			_smp::message_pool::getInstance().push(new message("[message_server::authCmdShutdown][Log] Auth Server requisitou para o server ser desligado em "
					+ std::to_string(_time_sec) + " segundos", CL_FILE_LOG_AND_CONSOLE));

			shutdown_time(_time_sec);

		}else
			_smp::message_pool::getInstance().push(new message("[message_server::authCmdShutdown][WARNING] Auth Server requisitou para o server ser delisgado em "
					+ std::to_string(_time_sec) + " segundos, mas o server ja esta com o timer de shutdown", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::authCmdShutdown][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void message_server::authCmdBroadcastNotice(std::string  ) {

	return;
}

void message_server::authCmdBroadcastTicker(std::string  , std::string  ) {

	return;
}

void message_server::authCmdBroadcastCubeWinRare(std::string  , uint32_t  ) {

	return;
}

void message_server::authCmdDisconnectPlayer(uint32_t _req_server_uid, uint32_t _player_uid, unsigned char _force) {

	UNREFERENCED_PARAMETER(_force);

	try {

		auto s = m_player_manager.findPlayer(_player_uid);

		if (s != nullptr) {

			_smp::message_pool::getInstance().push(new message("[message_server::authCmdDisconnectPlayer][log] Comando do Auth Server, Server[UID=" + std::to_string(_req_server_uid)
					+ "] pediu para desconectar o Player[UID=" + std::to_string(s->m_pi.uid) + "]", CL_FILE_LOG_AND_CONSOLE));

			DisconnectSession(s);

			m_unit_connect->sendConfirmDisconnectPlayer(_req_server_uid, _player_uid);

		}else
			_smp::message_pool::getInstance().push(new message("[message_server::authCmdDisconnectPlayer][WARNING] Comando do Auth Server, Server[UID=" + std::to_string(_req_server_uid)
					+ "] pediu para desconectar o Player[UID=" + std::to_string(_player_uid) + "], mas nao encontrou ele no server.", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::authCmdDisconnectPlayer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void message_server::authCmdConfirmDisconnectPlayer(uint32_t _player_uid) {

	return;
}

void message_server::authCmdNewMailArrivedMailBox(uint32_t  , uint32_t  ) {

	return;
}

void message_server::authCmdNewRate(uint32_t _tipo, uint32_t _qntd) {

	try {

		updateRateAndEvent(_tipo, _qntd);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::authCmdNewRate][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void message_server::authCmdReloadGlobalSystem(uint32_t _tipo) {

	try {

		reloadGlobalSystem(_tipo);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::authCmdReloadGlobalSystem][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void message_server::authCmdConfirmSendInfoPlayerOnline(uint32_t _req_server_uid, AuthServerPlayerInfo _aspi) {

	try {

		auto s = m_player_manager.findPlayer(_aspi.uid);

		if (s != nullptr) {

			confirmLoginOnOtherServer(*s, _req_server_uid, _aspi);

		}else
			_smp::message_pool::getInstance().push(new message("[message_server::authCmdConfirmSendInfoPlayerOnline][WARNING] Player[UID=" + std::to_string(_aspi.uid)
					+ "] retorno do confirma login com Auth Server do Server[UID=" + std::to_string(_req_server_uid) + "], mas o palyer nao esta mais conectado.", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::authCmdConfirmSendInfoPlayerOnline][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void message_server::SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg) {

	if (_arg == nullptr) {
		_smp::message_pool::getInstance().push(new message("[message_server::SQLDBResponse][WARNING] _arg is nullptr, na msg_id = " + std::to_string(_msg_id), CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	if (_pangya_db.getException().getCodeError() != 0) {
		_smp::message_pool::getInstance().push(new message("[message_server::SQLDBResponse][Error] " + _pangya_db.getException().getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	auto *_message_server = reinterpret_cast< message_server*>(_arg);

	switch (_msg_id) {
	case 1:
	{
		auto cmd_ibi = reinterpret_cast<CmdInsertBlockIP*>(&_pangya_db);

#ifdef _DEBUG
		_smp::message_pool::getInstance().push(new message("[message_server::SQLDBResponse][Log] Inseriu Block IP[IP=" + cmd_ibi->getIP()
				+ ", MASK=" + cmd_ibi->getMask() + "] com sucesso.", CL_FILE_LOG_AND_CONSOLE));
#else
		_smp::message_pool::getInstance().push(new message("[message_server::SQLDBResponse][Log] Inseriu Block IP[IP=" + cmd_ibi->getIP()
				+ ", MASK=" + cmd_ibi->getMask() + "] com sucesso.", CL_ONLY_FILE_LOG));
#endif

		break;
	}
	case 2:
	{

		auto cmd_urci = reinterpret_cast<CmdUpdateRateConfigInfo*>(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[message_server::SQLDBResponse][Log] Atualizou Rate Config Info[SERVER_UID=" + std::to_string(cmd_urci->getServerUID())
				+ ", " + cmd_urci->getInfo().toString() + "]", CL_FILE_LOG_AND_CONSOLE));

		break;
	}
	case 0:
	default:
		break;
	}
};

void message_server::shutdown_time(int32_t _time_sec) {

	if (_time_sec <= 0)
		shutdown();
	else {

		job _job(server::end_time_shutdown, this, (void*)0);

		if (m_shutdown != nullptr) {

			if (m_shutdown->getState() != timer::STOPPED)
				m_shutdown->stop();

			m_timer_mgr.deleteTimer(m_shutdown);
		}

		if ((m_shutdown = m_timer_mgr.createTimer(_time_sec * 1000, new (timer::timer_param){ _job, m_job_pool })) == nullptr)
			throw exception("[message_server::shutdown_time][Error] nao conseguiu criar o timer", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 51, 0));
	}
}

bool message_server::sendUpdatePlayerLogoutToFriends(player& _session) {
	CHECK_SESSION_BEGIN("sendUpdatePlayerLogoutToFriends");

	bool ret = true;

	packet p;

	try {

#if defined(_WIN32)
		if (InterlockedCompareExchange(&_session.m_pi.m_logout, 1, 0) == 1)
			return false;
#elif defined(__linux__)
		uint32_t check_m = 0;
		if (!__atomic_compare_exchange_n(&_session.m_pi.m_logout, &check_m, 1, true, __ATOMIC_RELAXED, __ATOMIC_RELAXED) && check_m == 1u)
			return false;
#endif

		p.init_plain((unsigned short)0x30);

		p.addUint16(0x10F);

		p.addUint32(_session.m_pi.uid);

		packet_func::friend_broadcast(m_player_manager.findAllFriend(_session.m_pi.m_friend_manager.getAllFriendAndGuildMember(true )), p, &_session, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::sendUpdatePlayerLogoutToFriends][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		ret = false;
	}

	return ret;
}

void message_server::onAcceptCompleted(session *_session) {

	if (_session == nullptr)
		throw exception("[message_server::onAcceptCompleted][Error] _session is nullptr.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 60, 0));

	if (!_session->getState())
		throw exception("[message_server::onAcceptCompleted][Error] _session is invalid.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 61, 0));

	if (!_session->isConnected())
		throw exception("[message_server::onAcceptCompleted][Error] _session is not connected.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 62, 0));

	packet p((unsigned short)0x2E);

	p.addUint8(1);
	p.addUint8(1);
	p.addUint32(_session->m_key);

	p.makeRaw();
	WSABUF mb = p.getMakedBuf();

	try {
		_session->requestRecvBuffer();
		_session->requestSendBuffer(mb.buf, mb.len);
	}catch (exception& e) {

		if (STDA_ERROR_CHECK_SOURCE_AND_ERROR(e.getCodeError(), STDA_ERROR_TYPE::SESSION, 1))
			throw;
	}
}

void message_server::onDisconnected(session *_session) {

	if (_session == nullptr)
		throw exception("[message_server::onDisconnect][Error] _session is nullptr.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 60, 0));

	player *p = reinterpret_cast< player* >(_session);

	bool ret = true;

	try {

#if defined(_WIN32)
		if (p->isConnected() && p->getState() && InterlockedCompareExchange(&p->m_pi.m_logout, p->m_pi.m_logout, 0) == 0) {
#elif defined(__linux__)
		uint32_t check_m = 0;

		if (p->isConnected() && p->getState() && __atomic_compare_exchange_n(&p->m_pi.m_logout, &check_m, 0, true, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
#endif

			if (p->m_is_authorized)
				ret = sendUpdatePlayerLogoutToFriends(*p);

		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::onDisconnected][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	if (ret)
		_smp::message_pool::getInstance().push(new message("[message_server::onDisconnected][Log] Player Desconectou ID: " + std::string(p->m_pi.id) + " UID: " + std::to_string(p->m_pi.uid), CL_FILE_LOG_AND_CONSOLE));

}

void message_server::onHeartBeat() {

	try {

		if (m_state != INITIALIZED)
			return;

		if (m_si.rate.smart_calculator && !sSmartCalculator::getInstance().hasStopped() && !sSmartCalculator::getInstance().isLoad())
			sSmartCalculator::getInstance().load();

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::onHeartBeat][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	return;
}

void message_server::onStart() {

	try {

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::onStart][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

bool message_server::checkCommand(std::stringstream& _command) {

	std::string s = "";

	_command >> s;

	if (!s.empty() && s.compare("exit") == 0)
		return true;
	else if (!s.empty() && s.compare("reload_files") == 0) {

		reload_files();

		_smp::message_pool::getInstance().push(new message("Message Server files has been reloaded.", CL_FILE_LOG_AND_CONSOLE));

	}else if (!s.empty() && s.compare("reload_socket_config") == 0) {

		if (m_accept_sock != nullptr)
			m_accept_sock->reload_config_file();
		else
			_smp::message_pool::getInstance().push(new message("[message_server::checkCommand][WARNING] m_accept_sock(socket que gerencia os socket que pode aceitar etc) is invalid.", CL_FILE_LOG_AND_CONSOLE));

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

					_smp::message_pool::getInstance().push(new message("[message_server::checkCommand][Log] Chat Discord Flag agora esta " + std::string(m_chat_discord ? "Ativado" : "Desativado"), CL_ONLY_CONSOLE));

				}else
					_smp::message_pool::getInstance().push(new message("[message_server::checkCommand][Error] Unknown Command: \"smart_calc " + sTipo + "\"", CL_ONLY_CONSOLE));

			}else
				_smp::message_pool::getInstance().push(new message("[message_server::checkCommand][Error] Smart Calculator not active, exec Command Event smart_calc to active it.", CL_ONLY_CONSOLE));

		}else
			_smp::message_pool::getInstance().push(new message("[message_server::checkCommand][Error] Unknown Command: \"smart_calc " + sTipo + "\"", CL_ONLY_CONSOLE));

	}else if (!s.empty() && s.compare("snapshot") == 0) {

		try {
			int *bad_ptr_snapshot = nullptr;
			*bad_ptr_snapshot = 2;
		}catch (exception& e) {
			UNREFERENCED_PARAMETER(e);

			_smp::message_pool::getInstance().push(new message("[message_server::checkCommand][Log] Snapshot comando executado.", CL_FILE_LOG_AND_CONSOLE));
		}

	}else
		_smp::message_pool::getInstance().push(new message("Unknown Command: " + s, CL_ONLY_CONSOLE));

	return false;
}

bool message_server::checkPacket(session& _session, packet *_packet) {

	if (

		_session.m_check_packet.checkPacketId(_packet->getTipo())) {

		uint32_t limit_count = CHK_PCKT_COUNT_LIMIT;

		switch (_packet->getTipo()) {
		case 0x1D:
			limit_count += 7;
			break;
		case 0:
		default:
			limit_count += 2;
		}

		if (_session.m_check_packet.incrementCount() >= limit_count ) {

			_smp::message_pool::getInstance().push(new message("[message_server::checkPacket][WARNING] Tentativa de DDoS ataque com pacote ID: (0x"
					+ hex_util::lltoaToHex(_packet->getTipo()) + ") " + std::to_string(_packet->getTipo()) + ". IP=" + std::string(_session.m_ip), CL_FILE_LOG_AND_CONSOLE));

			DisconnectSession(&_session);

			return false;
		}
	}

	return true;
}

void message_server::init_option_accepted_socket(SOCKET _accepted) {

	BOOL tcp_nodelay = 1u;

#if defined(_WIN32)
	tcp_keepalive keep;
	DWORD retk = 0;

	keep.onoff = 1;

	keep.keepalivetime = 60000;
	keep.keepaliveinterval = 10000;

	if (WSAIoctl(_accepted, SIO_KEEPALIVE_VALS, &keep, sizeof(keep), nullptr, 0, &retk, nullptr, nullptr) == SOCKET_ERROR)
		throw exception("[message_server::init_option_accepted_socket][Error] nao conseguiu setar o socket option KEEPALIVE[ONOFF=" + std::to_string(keep.onoff)
				+ ", TIME=" + std::to_string(keep.keepalivetime) + ", INTERVAL=" + std::to_string(keep.keepaliveinterval)
				+ "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 53, WSAGetLastError()));

	_smp::message_pool::getInstance().push(new message("[message_server::init_option_accepted_socket][Log] socket[ID=" + std::to_string(_accepted)
			+ "] KEEPALIVE[ONOFF=" + std::to_string(keep.onoff) + ", TIME=" + std::to_string(keep.keepalivetime)
			+ ", INTERVAL=" + std::to_string(keep.keepaliveinterval) + "] foi ativado para esse ", CL_FILE_LOG_AND_CONSOLE));

	if (setsockopt(_accepted, IPPROTO_TCP, TCP_NODELAY, (char*)&tcp_nodelay, sizeof(tcp_nodelay)) == SOCKET_ERROR)
		throw exception("[message_server::init_option_accepted_socket][Error] nao conseguiu desabilitar tcp delay(nagle algorithm).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 53, WSAGetLastError()));
#elif defined(__linux__)
	int flag = O_NONBLOCK;
	if (fcntl(_accepted.fd, F_SETFL, flag) != 0)
		throw exception("[message_server::init_option_accepted_socket][Error] nao conseguiu habilitar o NONBLOCK(fcntl).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 53, errno));

	int keepalive = 1;
	if (setsockopt(_accepted.fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) == -1)
		throw exception("[message_server::init_option_accepted_socket][error] nao conseguiu habilitaro keepalive(setsockopt).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 53, errno));

	keepalive = 60;
	if (setsockopt(_accepted.fd, SOL_TCP, TCP_KEEPIDLE, &keepalive, sizeof(keepalive)) == -1)
		throw exception("[message_server::init_option_accepted_socket][Error] nao conseguiu setar o keepalive idl time(setsockopt).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 53, errno));

	keepalive = 10;
	if (setsockopt(_accepted.fd, SOL_TCP, TCP_KEEPINTVL, &keepalive, sizeof(keepalive)) == -1)
		throw exception("[message_server::init_option_accepted_socket][Error] nao conseguiu setar o keepalive interval pobs(setsockopt).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 53, errno));

	keepalive = 10;
	if (setsockopt(_accepted.fd, SOL_TCP, TCP_KEEPCNT, &keepalive, sizeof(keepalive)) == -1)
		throw exception("[message_server::init_option_accepted_socket][Error] nao conseguiu setar o keepalive probs count(setsockopt).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 53, errno));

	_smp::message_pool::getInstance().push(new message("[message_server::init_option_accepted_socket][Log] socket[ID=" + std::to_string(_accepted.fd)
			+ "] KEEPALIVE[ONOFF=1, TIME=60000, INTERVAL=10000, COUNT=10] foi ativado para esse ", CL_FILE_LOG_AND_CONSOLE));

	if (setsockopt(_accepted.fd, IPPROTO_TCP, TCP_NODELAY, (char*)&tcp_nodelay, sizeof(tcp_nodelay)) == SOCKET_ERROR)
		throw exception("[message_server::init_option_accepted_socket][Error] nao conseguiu desabilitar tcp delay(nagle algorithm).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 53, errno));
#endif
}

void message_server::config_init() {

	m_si.tipo = 3;

	CmdRateConfigInfo cmd_rci(m_si.uid, true);

	snmdb::NormalManagerDB::getInstance().add(0, &cmd_rci, nullptr, nullptr);

	cmd_rci.waitEvent();

	if (cmd_rci.getException().getCodeError() != 0 || cmd_rci.isError() ) {

		if (cmd_rci.getException().getCodeError() != 0)
			_smp::message_pool::getInstance().push(new message("[message_server::config_init][ErrorSystem] " + cmd_rci.getException().getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		_smp::message_pool::getInstance().push(new message("[message_server::config_init][Error] nao conseguiu recuperar os valores de rate do server[UID="
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

		m_si.rate.angel_event = 0u;
		m_si.rate.pang = 0u;
		m_si.rate.exp = 0u;
		m_si.rate.club_mastery = 0u;

		snmdb::NormalManagerDB::getInstance().add(2, new CmdUpdateRateConfigInfo(m_si.uid, m_si.rate), message_server::SQLDBResponse, this);

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

		m_si.rate.angel_event = cmd_rci.getInfo().angel_event;
		m_si.rate.pang = cmd_rci.getInfo().pang;
		m_si.rate.exp = cmd_rci.getInfo().exp;
		m_si.rate.club_mastery = cmd_rci.getInfo().club_mastery;
	}
}

void message_server::reload_files() {

	server::config_init();
	config_init();

	sIff::getInstance().reload();
}

void message_server::reload_systems() {

	sIff::getInstance().load();

	if (m_si.rate.smart_calculator)
		sSmartCalculator::getInstance().load();
}

void message_server::reloadGlobalSystem(uint32_t _tipo) {

	try {

		switch (_tipo) {
		case 0:
			reload_systems();
			break;
		case 1:

			sIff::getInstance().load();
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:

			break;
		case 18:

			sSmartCalculator::getInstance().load();
			break;
		default:
			throw exception("[message_server::reloadGlobalSystem][Error] Tipo[VALUE=" + std::to_string(_tipo) + "] desconhecido.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 400, 0));
		}

		_smp::message_pool::getInstance().push(new message("[message_server::reloadGlobalSystem][Log] Recarregou o Sistema[Tipo=" + std::to_string(_tipo) + "] com sucesso!", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[message_server::reloadGlobalSystem][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void message_server::updateRateAndEvent(uint32_t _tipo, uint32_t _qntd) {

	try {

		if (_qntd == 0u && _tipo != 9  && _tipo != 10
				&& _tipo != 11  && _tipo != 12  && _tipo != 13
				&& _tipo != 14  && _tipo != 15 )
			throw exception("[message_server::updateRateAndEvent][Error] Rate[TIPO=" + std::to_string(_tipo) + ", QNTD="
					+ std::to_string(_qntd) + "], qntd is invalid(zero).", STDA_MAKE_ERROR(STDA_ERROR_TYPE::MESSAGE_SERVER, 120, 0));

		switch (_tipo) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		{
			m_si.rate.smart_calculator = (short)_qntd;

			if (m_si.rate.smart_calculator)
				reloadGlobalSystem(18 );

			break;
		}
		default:
			throw exception("[message_server::updateRateAndEvent][Error] troca Rate[TIPO=" + std::to_string(_tipo) + ", QNTD="
					+ std::to_string(_qntd) + "], tipo desconhecido.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::GAME_SERVER, 120, 0));
		}

		snmdb::NormalManagerDB::getInstance().add(2, new CmdUpdateRateConfigInfo(m_si.uid, m_si.rate), message_server::SQLDBResponse, this);

		_smp::message_pool::getInstance().push(new message("[message_server::updateRateAndEvent][Log] New Rate[Tipo=" + std::to_string(_tipo) + ", QNTD="
				+ std::to_string(_qntd) + "] com sucesso!", CL_FILE_LOG_AND_CONSOLE));

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[game_server::updateRateAndEvent][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}
