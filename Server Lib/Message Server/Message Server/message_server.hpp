
#pragma once
#ifndef _STDA_MESSAGE_SERVER_HPP
#define _STDA_MESSAGE_SERVER_HPP

#include "../../Projeto IOCP/Server/server.h"
#include "../SESSION/player_manager.hpp"

#include "../../Projeto IOCP/PACKET/packet.h"
#include "../SESSION/player.hpp"

#define FRIEND_LIST_LIMIT 50
#define FRIEND_PAG_LIMIT 30

namespace stdA {
	class message_server : public server {
		public:
			message_server();
			virtual ~message_server();

			void requestLogin(player& _session, packet *_packet);

			void confirmLoginOnOtherServer(player& _session, uint32_t _req_server_uid, AuthServerPlayerInfo& _aspi);

			void requestFriendAndGuildMemberList(player& _session, packet *_packet);

			void requestUpdateChannelPlayerInfo(player& _session, packet *_packet);

			void requestUpdatePlayerState(player& _session, packet *_packet);

			void requestUpdatePlayerLogout(player& _session, packet *_packet);

			void requestChatFriend(player& _session, packet *_packet);
			void requestChatGuild(player& _session, packet *_packet);

			void requestCheckNickname(player& _session, packet *_packet);

			void requestAssingApelido(player& _session, packet *_packet);

			void requestBlockFriend(player& _session, packet *_packet);

			void requestUnblockFriend(player& _session, packet *_packet);

			void requestAddFriend(player& _session, packet *_packet);

			void requestConfirmFriend(player& _session, packet *_packet);

			void requestDeleteFriend(player& _session, packet *_packet);

			void requestNotityPlayerWasInvitedToRoom(player& _session, packet *_packet);

			void requestInvitPlayerToGuildBattleRoom(player& _session, packet *_packet);

			void requestAcceptGuildMember(packet *_packet);
			void requestMemberExitedFromGuild(packet *_packet);
			void requestKickGuildMember(packet *_packet);

		public:

			virtual void authCmdShutdown(int32_t _time_sec) override;
			virtual void authCmdBroadcastNotice(std::string _notice) override;
			virtual void authCmdBroadcastTicker(std::string _nickname, std::string _msg) override;
			virtual void authCmdBroadcastCubeWinRare(std::string _msg, uint32_t _option) override;
			virtual void authCmdDisconnectPlayer(uint32_t _req_server_uid, uint32_t _player_uid, unsigned char _force) override;
			virtual void authCmdConfirmDisconnectPlayer(uint32_t _player_uid) override;
			virtual void authCmdNewMailArrivedMailBox(uint32_t _player_uid, uint32_t _mail_id) override;
			virtual void authCmdNewRate(uint32_t _tipo, uint32_t _qntd) override;
			virtual void authCmdReloadGlobalSystem(uint32_t _tipo) override;
			virtual void authCmdConfirmSendInfoPlayerOnline(uint32_t _req_server_uid, AuthServerPlayerInfo _aspi) override;

		protected:
			static void SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg);

			virtual void shutdown_time(int32_t _time_sec) override;

		protected:
			virtual bool sendUpdatePlayerLogoutToFriends(player& _session);

		protected:
			player_manager m_player_manager;

			virtual void onAcceptCompleted(session *_session) override;
			virtual void onDisconnected(session *_session) override;

			virtual void onHeartBeat() override;

			virtual void onStart() override;

			virtual bool checkCommand(std::stringstream& _command) override;

			virtual bool checkPacket(session& _session, packet *_packet) override;

			virtual void init_option_accepted_socket(SOCKET _accepted) override;

			virtual void config_init() override;
			virtual void reload_files();

			virtual void reload_systems();
			virtual void reloadGlobalSystem(uint32_t _tipo);

			virtual void updateRateAndEvent(uint32_t _tipo, uint32_t _qntd);
	};

	namespace sms {
		typedef Singleton< message_server > ms;
	}
}

#endif
