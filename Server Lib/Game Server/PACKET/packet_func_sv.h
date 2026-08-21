
#pragma once
#ifndef _STDA_PACKET_FUNC_SV_H
#define _STDA_PACKET_FUNC_SV_H

#if defined(_WIN32)
#include <Windows.h>
#include <WinSock2.h>
#elif defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#endif

#include "../../Projeto IOCP/TYPE/stdAType.h"
#include "../../Projeto IOCP/PACKET/packet_func.h"
#include "../TYPE/pangya_game_st.h"
#include "../../Projeto IOCP/PACKET/packet.h"
#include "../GAME/channel.h"
#include <string>
#include <vector>
#include <map>

#include "../TYPE/player_info.h"
#include "../SESSION/player.hpp"

#include "../../Projeto IOCP/TAP/pkt_tap.h"

#define MAKE_BEGIN_SERVER(_arg1) game_server *gs = reinterpret_cast< game_server* >((_arg1));

#define _PKT_TAP_IN pkttap::record(pkttap::SRC_GAME, pkttap::DIR_IN, pd._session.m_pi.uid, \
									pd._session.m_oid, pd._packet->getTipo(), \
									pd._packet->getPlainBuf().buf, \
									(uint32_t)pd._packet->getPlainBuf().len);

#define _MAKE_BEGIN_PACKET_SERVER(_arg1, _arg2) MAKE_BEGIN_SERVER(_arg1) _MAKE_BEGIN_PACKET(_arg2) _PKT_TAP_IN

#define MAKE_BEGIN_PACKET_AUTH_SERVER(_arg1, _arg2) MAKE_BEGIN_SERVER(_arg1) _MAKE_BEGIN_PACKET_AUTH_SERVER(_arg2)

#ifdef _DEBUG
#define MAKE_BEGIN_PACKET_SERVER(_arg1, _arg2) _MAKE_BEGIN_PACKET_SERVER(_arg1, _arg2) _MSG_BEGIN_PACKET
#else
#define MAKE_BEGIN_PACKET_SERVER(_arg1, _arg2) _MAKE_BEGIN_PACKET_SERVER(_arg1, _arg2)
#endif

#define _MAKE_BEGIN_PACKET_SERVER_RET(_arg1, _arg2) MAKE_BEGIN_SERVER(_arg1) _MAKE_BEGIN_PACKET(_arg2)

#ifdef _DEBUG
#define MAKE_BEGIN_PACKET_SERVER_RET(_arg1, _arg2) _MAKE_BEGIN_PACKET_SERVER_RET(_arg1, _arg2) _MSG_BEGIN_PACKET
#else
#define MAKE_BEGIN_PACKET_SERVER_RET(_arg1, _arg2) _MAKE_BEGIN_PACKET_SERVER_RET(_arg1, _arg2)
#endif

namespace stdA {
    class packet_func : public packet_func_base {
        public:

            static int packet002(void* _arg1, void* _arg2);
			static int packet003(void* _arg1, void* _arg2);
			static int packet004(void* _arg1, void* _arg2);
			static int packet006(void* _arg1, void* _arg2);
			static int packet007(void* _arg1, void* _arg2);
			static int packet038(void* _arg1, void* _arg2);
			static int packet008(void* _arg1, void* _arg2);
			static int packet009(void* _arg1, void* _arg2);
			static int packet00A(void* _arg1, void* _arg2);
			static int packet00B(void* _arg1, void* _arg2);
			static int packet00C(void* _arg1, void* _arg2);
			static int packet00D(void* _arg1, void* _arg2);
			static int packet00E(void* _arg1, void* _arg2);
			static int packet00F(void* _arg1, void* _arg2);
			static int packet010(void* _arg1, void* _arg2);
			static int packet011(void* _arg1, void* _arg2);
			static int packet012(void* _arg1, void* _arg2);
			static int packet0BB9(void* _arg1, void* _arg2);
			static int packet013(void* _arg1, void* _arg2);
			static int packet014(void* _arg1, void* _arg2);
			static int packet015(void* _arg1, void* _arg2);
			static int packet016(void* _arg1, void* _arg2);
			static int packet017(void* _arg1, void* _arg2);
			static int packet018(void* _arg1, void* _arg2);
			static int packet019(void* _arg1, void* _arg2);
			static int packet01A(void* _arg1, void* _arg2);
			static int packet01B(void* _arg1, void* _arg2);
			static int packet01C(void* _arg1, void* _arg2);
			static int packet01D(void* _arg1, void* _arg2);
			static int packet01F(void* _arg1, void* _arg2);
			static int packet020(void* _arg1, void* _arg2);
			static int packet022(void* _arg1, void* _arg2);
			static int packet026(void* _arg1, void* _arg2);
			static int packet029(void* _arg1, void* _arg2);
			static int packet02A(void* _arg1, void* _arg2);
			static int packet02D(void* _arg1, void* _arg2);
			static int packet02F(void* _arg1, void* _arg2);
			static int packet030(void* _arg1, void* _arg2);
			static int packet031(void* _arg1, void* _arg2);
			static int packet032(void* _arg1, void* _arg2);
			static int packet033(void* _arg1, void* _arg2);
			static int packet034(void* _arg1, void* _arg2);
			static int packet035(void* _arg1, void* _arg2);
			static int packet036(void* _arg1, void* _arg2);
			static int packet037(void* _arg1, void* _arg2);
			static int packet039(void* _arg1, void* _arg2);
			static int packet03A(void* _arg1, void* _arg2);

			static int packet03C(void* _arg1, void* _arg2);
			static int packet03D(void* _arg1, void* _arg2);
			static int packet03E(void* _arg1, void* _arg2);
			static int packet041(void* _arg1, void* _arg2);
			static int packet042(void* _arg1, void* _arg2);
			static int packet043(void* _arg1, void* _arg2);
			static int packet047(void* _arg1, void* _arg2);
			static int packet048(void* _arg1, void* _arg2);
			static int packet04A(void* _arg1, void* _arg2);
			static int packet04B(void* _arg1, void* _arg2);
			static int packet04F(void* _arg1, void* _arg2);
			static int packet054(void* _arg1, void* _arg2);
			static int packet055(void* _arg1, void* _arg2);
			static int packet057(void* _arg1, void* _arg2);
			static int packet05C(void* _arg1, void* _arg2);

			static int packet060(void* _arg1, void* _arg2);

			static int packet061(void* _arg1, void* _arg2);
			static int packet063(void* _arg1, void* _arg2);
			static int packet064(void* _arg1, void* _arg2);
			static int packet065(void* _arg1, void* _arg2);
			static int packet066(void* _arg1, void* _arg2);
			static int packet067(void* _arg1, void* _arg2);
			static int packet069(void* _arg1, void* _arg2);
			static int packet06B(void* _arg1, void* _arg2);
			static int packet070(void* _arg1, void* _arg2);
			static int packet073(void* _arg1, void* _arg2);
			static int packet074(void* _arg1, void* _arg2);
			static int packet075(void* _arg1, void* _arg2);
			static int packet076(void* _arg1, void* _arg2);
			static int packet077(void* _arg1, void* _arg2);
			static int packet078(void* _arg1, void* _arg2);
			static int packet079(void* _arg1, void* _arg2);
			static int packet07A(void* _arg1, void* _arg2);
			static int packet07B(void* _arg1, void* _arg2);
			static int packet07C(void* _arg1, void* _arg2);
			static int packet07D(void* _arg1, void* _arg2);
			static int packet081(void* _arg1, void* _arg2);
			static int packet082(void* _arg1, void* _arg2);
			static int packet083(void* _arg1, void* _arg2);
			static int packet088(void* _arg1, void* _arg2);
			static int packet08B(void* _arg1, void* _arg2);
			static int packet08F(void* _arg1, void* _arg2);
			static int packet098(void* _arg1, void* _arg2);
			static int packet09C(void* _arg1, void* _arg2);
			static int packet09D(void* _arg1, void* _arg2);
			static int packet09E(void* _arg1, void* _arg2);
			static int packet0A1(void* _arg1, void* _arg2);
			static int packet0A2(void* _arg1, void* _arg2);
			static int packet0AA(void* _arg1, void* _arg2);
			static int packet0AB(void* _arg1, void* _arg2);
			static int packet0AE(void* _arg1, void* _arg2);
			static int packet0B2(void* _arg1, void* _arg2);
			static int packet0B4(void* _arg1, void* _arg2);
			static int packet0B5(void* _arg1, void* _arg2);
			static int packet0B7(void* _arg1, void* _arg2);
			static int packet0B9(void* _arg1, void* _arg2);
			static int packet0BA(void* _arg1, void* _arg2);
			static int packet0BD(void* _arg1, void* _arg2);
			static int packet0C1(void* _arg1, void* _arg2);
			static int packet0C9(void* _arg1, void* _arg2);
			static int packet0CA(void* _arg1, void* _arg2);
			static int packet0CB(void* _arg1, void* _arg2);
			static int packet0CC(void* _arg1, void* _arg2);
			static int packet0CD(void* _arg1, void* _arg2);
			static int packet0CE(void* _arg1, void* _arg2);
			static int packet0CF(void* _arg1, void* _arg2);
			static int packet0D0(void* _arg1, void* _arg2);
			static int packet0D1(void* _arg1, void* _arg2);
			static int packet0D2(void* _arg1, void* _arg2);
			static int packet0D3(void* _arg1, void* _arg2);
			static int packet0D4(void* _arg1, void* _arg2);
			static int packet0D5(void* _arg1, void* _arg2);
			static int packet0D8(void* _arg1, void* _arg2);
			static int packet0DE(void* _arg1, void* _arg2);
			static int packet0E5(void* _arg1, void* _arg2);
			static int packet0E6(void* _arg1, void* _arg2);
			static int packet0E7(void* _arg1, void* _arg2);
			static int packet0EB(void* _arg1, void* _arg2);
			static int packet0EC(void* _arg1, void* _arg2);
			static int packet0EF(void* _arg1, void* _arg2);
			static int packet0F4(void* _arg1, void* _arg2);
			static int packet0FB(void* _arg1, void* _arg2);
			static int packet0FE(void* _arg1, void* _arg2);
			static int packet119(void* _arg1, void* _arg2);
			static int packet126(void* _arg1, void* _arg2);
			static int packet127(void* _arg1, void* _arg2);
			static int packet128(void* _arg1, void* _arg2);
			static int packet129(void* _arg1, void* _arg2);
			static int packet12C(void* _arg1, void* _arg2);
			static int packet12D(void* _arg1, void* _arg2);
			static int packet12E(void* _arg1, void* _arg2);
			static int packet12F(void* _arg1, void* _arg2);
			static int packet130(void* _arg1, void* _arg2);
			static int packet131(void* _arg1, void* _arg2);
			static int packet137(void* _arg1, void* _arg2);
			static int packet138(void* _arg1, void* _arg2);
			static int packet140(void* _arg1, void* _arg2);
			static int packet141(void* _arg1, void* _arg2);
			static int packet143(void* _arg1, void* _arg2);
			static int packet144(void* _arg1, void* _arg2);
			static int packet145(void* _arg1, void* _arg2);
			static int packet146(void* _arg1, void* _arg2);
			static int packet147(void* _arg1, void* _arg2);
			static int packet14B(void* _arg1, void* _arg2);
			static int packet151(void* _arg1, void* _arg2);
			static int packet152(void* _arg1, void* _arg2);
			static int packet153(void* _arg1, void* _arg2);
			static int packet154(void* _arg1, void* _arg2);
			static int packet155(void* _arg1, void* _arg2);
			static int packet156(void* _arg1, void* _arg2);
			static int packet157(void* _arg1, void* _arg2);
			static int packet158(void* _arg1, void* _arg2);
			static int packet15C(void* _arg1, void* _arg2);
			static int packet15D(void* _arg1, void* _arg2);
			static int packet164(void* _arg1, void* _arg2);
			static int packet165(void* _arg1, void* _arg2);
			static int packet166(void* _arg1, void* _arg2);
			static int packet167(void* _arg1, void* _arg2);
			static int packet168(void* _arg1, void* _arg2);
			static int packet169(void* _arg1, void* _arg2);
			static int packet16B(void* _arg1, void* _arg2);
			static int packet16C(void* _arg1, void* _arg2);
			static int packet16D(void* _arg1, void* _arg2);
			static int packet16E(void* _arg1, void* _arg2);
			static int packet16F(void* _arg1, void* _arg2);
			static int packet171(void* _arg1, void* _arg2);
			static int packet172(void* _arg1, void* _arg2);
			static int packet176(void* _arg1, void* _arg2);
			static int packet177(void* _arg1, void* _arg2);
			static int packet179(void* _arg1, void* _arg2);
			static int packet17A(void* _arg1, void* _arg2);
			static int packet17F(void* _arg1, void* _arg2);
			static int packet180(void* _arg1, void* _arg2);
			static int packet181(void* _arg1, void* _arg2);
			static int packet184(void* _arg1, void* _arg2);
			static int packet185(void* _arg1, void* _arg2);
			static int packet186(void* _arg1, void* _arg2);
			static int packet187(void* _arg1, void* _arg2);
			static int packet188(void* _arg1, void* _arg2);
			static int packet189(void* _arg1, void* _arg2);
			static int packet18A(void* _arg1, void* _arg2);
			static int packet18B(void* _arg1, void* _arg2);
			static int packet18C(void* _arg1, void* _arg2);
			static int packet18D(void* _arg1, void* _arg2);
			static int packet192(void* _arg1, void* _arg2);
			static int packet196(void* _arg1, void* _arg2);
			static int packet197(void* _arg1, void* _arg2);
			static int packet198(void* _arg1, void* _arg2);
			static int packet199(void* _arg1, void* _arg2);

			static int packet1F4(void* _arg1, void* _arg2);

			static void sendHmacKey(player* _session);
			static void sendChallenge(player* _session);

			static void sendBallRestoreFreeze(player* _session, float x, float y, float z);
			static void sendBallRestoreUnfreeze(player* _session);
			static void sendShotCountRestore(player* _session, const unsigned char* _payload, unsigned short _size);
			static void sendShotResultUnlock(player* _session);
			static void sendApplyShotResult(player* _session);

			static void sendCrazyClubSeed(player* _session, bool _on);
			static double crazySeedPhase(uint32_t _seed, uint32_t _which);
			static float  crazyExpectedFactor(uint32_t _seed, uint32_t _tickMs, float _rawMax, float _ampP);

			static int packet_sv4D(void* _arg1, void* _arg2);
			static int packet_sv055(void* _arg1, void* _arg2);
			static int packet_svRequestInfo(void* _arg1, void* _arg2);
			static int packet_sv22D(void* _arg1, void* _arg2);
			static int packet_svFazNada(void* _arg1, void* _arg2);
			static int packet_svDisconectPlayerBroadcast(void* _arg1, void* _arg2);

			static int packet_as001(void* _arg1, void* _arg2);

			static int						pacote040(packet& p, player *_session, PlayerInfo *pi, std::string msg, unsigned char option = 0);
            static int						pacote044(packet& p, player *_session, ServerInfoEx& _si, int option = 0, PlayerInfo *pi = nullptr, int valor = 0);
			static int						pacote046(packet& p, player *_session, std::vector< PlayerCanalInfo > v_element, int option = 0);
			static int						pacote047(packet& p, std::vector< RoomInfo > v_element, int option = 0);
			static int						pacote048(packet& p, player *_session, std::vector< PlayerRoomInfoEx > v_element, int option = 0);
			static int						pacote049(packet& p, room *_room, int option = 0);
			static int						pacote04A(packet& p, RoomInfoEx& _ri, int option);
			static int						pacote04B(packet& p, player *_session, unsigned char _type, int error = 0, int _valor = 0);
			static int						pacote04C(packet& p, player *_session, int option = 0);
			static int						pacote04D(packet& p, player *_session, std::vector< channel* >& v_element, int option = 0);
			static int						pacote04E(packet& p, player *_session, int option, int _codeErrorInfo = 0);
			static int						pacote06B(packet& p, player *_session, PlayerInfo *pi, unsigned char type, unsigned char err_code = 4);
			static int						pacote070(packet& p, player *_session, std::multimap< int32_t , CharacterInfo >& v_element, int option = 0);
			static int						pacote071(packet& p, player *_session, std::multimap< int32_t , CaddieInfoEx >& v_element, int option = 0);
			static int						pacote072(packet& p, player *_session, UserEquip ue, int option = 0);
			static int						pacote073(packet& p, player *_session, std::multimap< int32_t , WarehouseItemEx >& v_element, int option = 0);
			static int						pacote089(packet& p, player *_session, uint32_t _uid, unsigned char season, uint32_t err_code = 1);
			static int						pacote095(packet& p, player *_session, unsigned short sub_tipo, int option = 0, PlayerInfo *pi = nullptr);
			static int						pacote096(packet& p, player *_session, PlayerInfo *pi);
			static int						pacote09A(packet& p, player *_session, PlayerInfo *pi);
			static int						pacote09F(packet& p, player *_session, std::vector< ServerInfo >& v_server, std::vector< channel* >& v_channel);
			static int						pacote0AA(packet& p, player *_session, std::vector< stItem >& v_item);
			static int						pacote0B2(packet& p, player *_session, std::vector< MsgOffInfo >& v_element, int option = 0);
			static int						pacote0B4(packet& p, player *_session, std::vector< TrofelEspecialInfo >& v_element, int option = 0);
			static int						pacote0D4(packet& p, player *_session, std::multimap< int32_t , CaddieInfoEx >& v_element);
			static int						pacote0E1(packet& p, player *_session, std::multimap< int32_t , MascotInfoEx >& v_element, int option = 0);
			static int						pacote0F1(packet& p, player *_session, int option = 0);
			static int						pacote0F5(packet& p, player *_session);
			static int						pacote0F6(packet& p, player *_session);
			static int						pacote0FC(packet& p, player *_session, std::vector< ServerInfo >& v_si);
			static int						pacote101(packet& p, player *_session, int option = 0);
			static int						pacote102(packet& p, player *_session, PlayerInfo *pi);
			static int						pacote10E(packet& p, player *_session, Last5PlayersGame& l5pg);
			static int						pacote11F(packet& p, player *_session, PlayerInfo *pi, short tipo);
			static int						pacote12E(packet& p, player *_session, WarehouseItemEx *wi, int state, int option = 0);
			static int						pacote131(packet& p, int option = 1);
			static int						pacote135(packet& p, player *_session);
			static int						pacote136(packet& p, player *_session);
			static int						pacote137(packet& p, player *_session, std::vector< CardEquipInfoEx >& v_element);
			static int						pacote138(packet& p, player *_session, std::vector< CardInfo >& v_element, int option = 0);
			static int						pacote13F(packet& p, player *_session, int option = 0);
			static int						pacote144(packet& p, player *_session, int option = 0);
			static int						pacote156(packet& p, player *_session, uint32_t _uid, UserEquip& _ue, unsigned char season);
			static int						pacote157(packet& p, player *_session, MemberInfoEx& _mi, unsigned char season);
			static int						pacote158(packet& p, player *_session, uint32_t _uid, UserInfo& _ui, unsigned char season);
			static int						pacote159(packet& p, player *_session, uint32_t _uid, TrofelInfo& ti, unsigned char season);
			static int						pacote15A(packet& p, player *_session, uint32_t _uid, std::vector< TrofelEspecialInfo >& v_tei, unsigned char season);
			static int						pacote15B(packet& p, player *_session, uint32_t _uid, unsigned char season);
			static int						pacote15C(packet& p, player *_session, uint32_t _uid, std::vector< MapStatistics >& v_ms, std::vector< MapStatistics >& v_msa, unsigned char season);
			static int						pacote15D(packet& p, player *_session, uint32_t _uid, GuildInfo& _gi);
			static int						pacote15E(packet& p, player *_session, uint32_t _uid, CharacterInfo& _ci);
			static int						pacote169(packet& p, player *_session, TrofelInfo& ti, int option = 0);
			static int						pacote181(packet& p, player *_session, std::vector< ItemBuffEx >& v_element, int option = 0);
			static int						pacote1A9(packet& p, player *_session, int32_t ttl_milliseconds , int option = 1);
			static int						pacote1AD(packet& p, player *_session, std::string webKey, int option = 0);
			static int						pacote1B1(packet& p, player *_session);
			static int						pacote1D4(packet& p, player *_session, std::string _AuthKeyLogin, int option = 0);
			static int						pacote210(packet& p, player *_session, std::vector< MailBox >& v_element, int option = 0);
			static int						pacote211(packet& p, player *_session, std::vector< MailBox > v_element, int32_t pagina, int32_t paginas, int error = 0);
			static int						pacote212(packet& p, player *_session, EmailInfo& ei, int error = 0);
			static int						pacote214(packet& p, player *_session, int error = 0);
			static int						pacote215(packet& p, player *_session, std::vector< MailBox > v_element, int32_t pagina, int32_t paginas, int error = 0);
			static int						pacote216(packet& p, player *_session, std::vector< stItem > & v_item, int option = 0);
			static int						pacote21D(packet& p, player *_session, std::vector< CounterItemInfo >& v_element, int option = 0);
			static int						pacote21E(packet& p, player *_session, std::multimap< uint32_t, AchievementInfoEx >& v_element, int option = 0);
			static int						pacote225(packet& p, player *_session, DailyQuestInfoUser& _dq, std::vector< RemoveDailyQuestUser > _delete_quest, int option = 0);
			static int						pacote226(packet& p, player *_session, std::vector< AchievementInfoEx >& v_element, int option = 0);
			static int						pacote227(packet& p, player *_session, std::vector< AchievementInfoEx >& v_element, int option = 0);
			static int						pacote228(packet& p, player *_session, std::vector< AchievementInfoEx >& v_element, int option = 0);
			static int						pacote22C(packet& p, player *_session, int option = 0);
			static int						pacote22D(packet& p, player *_session, std::multimap< uint32_t, AchievementInfoEx >& v_element, int option = 0);
			static int						pacote248(packet& p, player *_session, AttendanceRewardInfo& ari, int option = 0);
			static int						pacote249(packet& p, player *_session, AttendanceRewardInfo& ari, int option = 0);
			static int						pacote257(packet& p, player *_session, uint32_t _uid, std::vector< TrofelEspecialInfo >& v_tegi, unsigned char season);
			static int						pacote25D(packet& p, player *_session, std::vector< TrofelEspecialInfo >& v_element, int option = 0);
			static int						pacote26D(packet& p, player *_session, uint32_t _unix_end_date);

			static int principal(packet& p, PlayerInfo *pi, ServerInfoEx& _si);

			static void channel_broadcast(channel& _channel, packet& p, unsigned char _debug);
			static void channel_broadcast(channel& _channel, std::vector< packet* > v_p, unsigned char _debug);

			static void lobby_broadcast(channel& _channel, packet& p, unsigned char _debug);

			static void room_broadcast(room& _room, packet& p, unsigned char _debug);
			static void room_broadcast(room& _room, std::vector< packet* > v_p, unsigned char _debug);

			static void game_broadcast(Game& _game, packet& p, unsigned char _debug);
			static void game_broadcast(Game& _game, std::vector< packet* > v_p, unsigned char _debug);

			static void vector_send(packet& _p, std::vector< session* > _v_s, unsigned char _debug);
			static void vector_send(packet& _p, std::vector< player* > _v_s, unsigned char _debug);
			static void vector_send(std::vector< packet* > _v_p, std::vector< session* > _v_s, unsigned char _debug);
			static void vector_send(std::vector< packet* > _v_p, std::vector< player* > _v_s, unsigned char _debug);

			static void session_send(packet& p, session *s, unsigned char _debug);
			static void session_send(std::vector< packet* > v_p, session *s, unsigned char _debug);
    };
}

#endif
