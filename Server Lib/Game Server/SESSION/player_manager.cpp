
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "player_manager.hpp"
#include "../../Projeto IOCP/UTIL/exception.h"
#include "../../Projeto IOCP/TYPE/stda_error.h"
#include "../../Projeto IOCP/UTIL/message_pool.h"

#include "../../Projeto IOCP/UTIL/util_time.h"
#include "../../Projeto IOCP/UTIL/reader_ini.hpp"

#include "../PACKET/packet_func_sv.h"
#include "../Game Server/game_server.h"

#include "../PANGYA_DB/cmd_update_caddie_info.hpp"
#include "../PANGYA_DB/cmd_update_character_all_part_equiped.hpp"

#include "../../Projeto IOCP/DATABASE/normal_manager_db.hpp"

#include "../GAME/premium_system.hpp"

#include "../GAME/item_manager.h"

#define CHECK_PLAYER(_p)	if (!(_p).getState()) \
								return; \
							if (!(_p).isConnected()) \
									return; \

using namespace stdA;

player_manager::player_manager(threadpool& _threadpool, uint32_t _max_session) : session_manager(_threadpool, _max_session) {

	if (m_max_session != ~0u) {

		m_sessions.reserve(m_max_session);

		for (auto i = 0u; i < m_max_session; ++i)
			m_sessions.push_back(new player(*(threadpool_base*)&m_threadpool));
	}
};

player_manager::~player_manager() {
	clear();
};

void player_manager::clear() {

	session_manager::clear();

	if (!m_indexes.empty())
		m_indexes.clear();
};

player* player_manager::findPlayer(uint32_t _uid, bool _oid) {

	player *_player = nullptr;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	for (auto& el : m_sessions) {
		if (
#if defined(_WIN32)
			el->m_sock != INVALID_SOCKET
#elif defined(__linux__)
			el->m_sock.fd != INVALID_SOCKET
#endif
		&& ((!_oid) ? el->getUID() : el->m_oid) == _uid) {
			_player = (player*)el;
			break;
		}
	}

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif

	return _player;
};

player* player_manager::findPlayerBySocketId(uint32_t _socket_id) {

	player *_player = nullptr;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	for (auto& el : m_sessions) {
		if (
#if defined(_WIN32)
			el->m_sock != INVALID_SOCKET && el->m_sock == _socket_id
#elif defined(__linux__)
			el->m_sock.fd != INVALID_SOCKET && el->m_sock.fd == _socket_id
#endif
		) {
			_player = (player*)el;
			break;
		}
	}

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif

	return _player;
};

std::vector< player* > player_manager::findAllGM() {

	std::vector< player* > v_gm;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	for (auto& el : m_sessions)
		if (
#if defined(_WIN32)
			el->m_sock != INVALID_SOCKET
#elif defined(__linux__)
			el->m_sock.fd != INVALID_SOCKET
#endif
		&& ((el->getCapability() & 4) || el->getCapability() & 128 ))
			v_gm.push_back((player*)el);

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif

	return v_gm;
};

bool player_manager::deleteSession(session *_session) {

	if (_session == nullptr)
		throw exception("[player_manager::deleteSession][ERR_SESSION] _session is nullptr.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER_MANAGER, 1, 0));

	if (!_session->getState() &&
#if defined(_WIN32)
		_session->m_sock == INVALID_SOCKET
#elif defined(__linux__)
		_session->m_sock.fd == INVALID_SOCKET
#endif
	)
		throw exception(std::string("[player_manager::deleteSession][ERR_SESSION] SESSION[IP=") + _session->getIP() + ", UID="
				+ std::to_string(((player*)_session)->m_pi.uid) + ", OID=" + std::to_string(_session->m_oid)
				+ "] _seession not connected.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER_MANAGER, 2, 0));

	bool ret = true;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	if (!_session->getState() &&
#if defined(_WIN32)
		_session->m_sock == INVALID_SOCKET
#elif defined(__linux__)
		_session->m_sock.fd == INVALID_SOCKET
#endif
	) {

#if defined(_WIN32)
		LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
		pthread_mutex_unlock(&m_cs);
#endif

		throw exception(std::string("[player_manager::deleteSession][ERR_SESSION] SESSION[IP=") + _session->getIP() + ", UID="
				+ std::to_string(((player*)_session)->m_pi.uid) + ", OID=" + std::to_string(_session->m_oid)
				+ "] _seession not connected.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PLAYER_MANAGER, 2, 0));
	}

	_session->lock();

	uint32_t tmp_oid = _session->m_oid;

	if ((ret = _session->clear())) {

		m_count--;

		freeOID(tmp_oid );
	}

	_session->unlock();

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif

	return ret;
};

void player_manager::checkPlayersItens() {

	try {

		for (auto& s : m_sessions) {

			if (s->isCreated()) {

				checkItemBuff(*(player*)s);

				checkCardSpecial(*(player*)s);

				checkCaddie(*(player*)s);

				checkMascot(*(player*)s);

				checkWarehouse(*(player*)s);
			}
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[player_manager::checkPlayersItens][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void player_manager::tickAntiCheat() {

	static int s_ac_enabled = -1;
	if (s_ac_enabled < 0) {
		s_ac_enabled = 1;
		try {
#if defined(_WIN32)
			stdA::ReaderIni ini("\\server.ini");
#elif defined(__linux__)
			stdA::ReaderIni ini("/server.ini");
#endif
			ini.init();
			s_ac_enabled = ini.readInt("OPTION", "ANTICHEAT_ENABLE");
		} catch (exception& e) { (void)e; s_ac_enabled = 1; }
		if (s_ac_enabled == 0)
			_smp::message_pool::getInstance().push(new message("[AntiCheat] DESLIGADO via server.ini (ANTICHEAT_ENABLE=0)", CL_FILE_LOG_AND_CONSOLE));
	}
	if (s_ac_enabled == 0) return;

	const uint64_t CHALLENGE_INTERVAL_MS     = 30000;
	const uint64_t RESPONSE_TIMEOUT_MS       = 15000;
	const uint64_t FIRST_ATTEST_DEADLINE_MS  = 60000;

	try {

		uint64_t now = (uint64_t)GetTickCount64();

		for (auto& s : m_sessions) {

			if (s == nullptr) continue;
			if (!s->isCreated() || !s->isConnected()) continue;

			player* p = (player*)s;
			if (!p->m_is_authorized) continue;
			if (!p->m_ac.hmac_key_sent) continue;

			std::string nick(p->m_pi.nickname);
			std::string uid_s = std::to_string(p->m_pi.uid);

			if (p->m_ac.pending_nonce_valid && (now - p->m_ac.nonce_sent_at) > RESPONSE_TIMEOUT_MS) {
				p->m_ac.pending_nonce_valid = false;
				_smp::message_pool::getInstance().push(new message("[AntiCheat] CR TIMEOUT — " + nick + " (UID=" + uid_s + ") — DISCONNECT", CL_FILE_LOG_AND_CONSOLE));
				sgs::gs::getInstance().DisconnectSession(p);
				continue;
			}

			if (!p->m_ac.first_attest_received
				&& p->m_ac.key_sent_at > 0
				&& (now - p->m_ac.key_sent_at) > FIRST_ATTEST_DEADLINE_MS) {
				_smp::message_pool::getInstance().push(new message("[AntiCheat] FIRST ATTEST NEVER — " + nick + " (UID=" + uid_s + ") — DISCONNECT", CL_FILE_LOG_AND_CONSOLE));
				sgs::gs::getInstance().DisconnectSession(p);
				continue;
			}

			if ((now - p->m_ac.last_challenge_at) >= CHALLENGE_INTERVAL_MS) {
				packet_func::sendChallenge(p);
			}
		}

	}catch (exception& e) {
		_smp::message_pool::getInstance().push(new message("[player_manager::tickAntiCheat][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void player_manager::checkPlayersGameGuard() {

	try {

		for (auto& s : m_sessions) {

			if (s->isCreated() && s->isConnected() && checkGameGuard(*(player*)s))
#if defined(_WIN32)
				shutdown(s->m_sock, SD_RECEIVE);
#elif defined(__linux__)
				shutdown(s->m_sock.fd, SD_RECEIVE);
#endif
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[player_manager::checkPlayersGameGuard][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
};

void player_manager::blockOID(uint32_t _oid) {

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	auto it = m_indexes.find(_oid);

	if (it != m_indexes.end())
		it->second.flag.block = 1;

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif
};

bool player_manager::reclaimOID(uint32_t _oid_cadeira, uint32_t _oid_novo) {

	bool ret = false;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	auto it = m_indexes.find(_oid_cadeira);

	if (it != m_indexes.end() && it->second.flag.busy == 0) {

		it->second.flag.busy = 1;

		ret = true;
	}

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif

	if (ret)
		_smp::message_pool::getInstance().push(new message("[player_manager::reclaimOID][Log] OID da cadeira=" + std::to_string(_oid_cadeira)
				+ " reservado para a session que esta reconectando. O slot fisico dela continua sendo o OID=" + std::to_string(_oid_novo)
				+ ", que fica ocupado ate o server reiniciar (vaza 1 slot por reconexao, aceitavel).", CL_FILE_LOG_AND_CONSOLE));

	return ret;
};

void player_manager::unblockOID(uint32_t _oid) {

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	auto it = m_indexes.find(_oid);

	if (it != m_indexes.end())
		it->second.flag.block = 0;

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif
};

inline void player_manager::checkItemBuff(player& _session) {
	CHECK_PLAYER(_session);

	for (auto it = _session.m_pi.v_ib.begin(); it != _session.m_pi.v_ib.end();  ) {

		if (getLocalTimeDiffDESC(it->end_date) <= 0ll) {

			_smp::message_pool::getInstance().push(new message("[player_manager::checkItemBuff][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] Acabou o Tempo do Item Buff[TYPEID=" + std::to_string(it->_typeid) + ", ID="  + std::to_string(it->id)
					+ ", EFEITO=" + std::to_string(it->efeito) + ", EFEITO_QNTD=" + std::to_string(it->efeito_qntd) + ", END_DATE="
					+ _formatDate(it->end_date) + "] excluindo ele do vector.", CL_FILE_LOG_AND_CONSOLE));

			it = _session.m_pi.v_ib.erase(it);

		}else
			it++;
	}
};

inline void player_manager::checkCardSpecial(player& _session) {
	CHECK_PLAYER(_session);

	for (auto it = _session.m_pi.v_cei.begin(); it != _session.m_pi.v_cei.end();  ) {

		if (it->tipo == IFF::Card::CARD_SUB_TYPE::T_SPECIAL && getLocalTimeDiffDESC(it->end_date) <= 0ll) {

			_smp::message_pool::getInstance().push(new message("[player_manager::checkCardSpecial][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
					+ "] Acabou o Tempo do Card Equiped Special[TYPEID=" + std::to_string(it->_typeid) + ", ID="  + std::to_string(it->id)
					+ ", EFEITO=" + std::to_string(it->efeito) + ", EFEITO_QNTD=" + std::to_string(it->efeito_qntd) + ", END_DATE="
					+ _formatDate(it->end_date) + "] excluindo ele do vector.", CL_FILE_LOG_AND_CONSOLE));

			it = _session.m_pi.v_cei.erase(it);

		}else
			it++;
	}
};

inline void player_manager::checkCaddie(player& _session) {
	CHECK_PLAYER(_session);

	for (auto& el : _session.m_pi.mp_ci) {

		if (el.second.rent_flag == 2 && getLocalTimeDiffDESC(el.second.end_date) <= 0ll) {

			if (_session.m_pi.findUpdateItemByIdAndType(el.second.id, UpdateItem::CADDIE) == _session.m_pi.mp_ui.end()) {

				_session.m_pi.mp_ui.insert(std::make_pair(PlayerInfo::stIdentifyKey(el.second._typeid, el.second.id), UpdateItem{ UpdateItem::CADDIE, el.second._typeid, el.second.id }));

				_smp::message_pool::getInstance().push(new message("[player_manager::checkCaddie][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] Caddie[TYPEID=" + std::to_string(el.second._typeid) + ", ID=" + std::to_string(el.second.id)
						+ ", END_DATE=" + _formatDate(el.second.end_date) + "] Entrou de ferias, coloca no vector de update itens.", CL_FILE_LOG_AND_CONSOLE));

				if ((_session.m_pi.ei.cad_info != nullptr && _session.m_pi.ei.cad_info->id == el.second.id) || _session.m_pi.ue.caddie_id == el.second.id) {

					_session.m_pi.ei.cad_info = nullptr;
					_session.m_pi.ue.caddie_id = 0;

					_smp::message_pool::getInstance().push(new message("[player_manager::checkCaddie][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] Desequipando Caddie[TYPEID=" + std::to_string(el.second._typeid) + ", ID=" + std::to_string(el.second.id)
							+ "].", CL_FILE_LOG_AND_CONSOLE));
				}
			}
		}

		if (el.second.parts_typeid != 0u && !isEmpty(el.second.end_parts_date) && getLocalTimeDiffDESC(el.second.end_parts_date) <= 0ll) {

			if (_session.m_pi.findUpdateItemByIdAndType(el.second.id, UpdateItem::CADDIE_PARTS) == _session.m_pi.mp_ui.end()) {

				_session.m_pi.mp_ui.insert(std::make_pair(PlayerInfo::stIdentifyKey(el.second._typeid, el.second.id), UpdateItem{ UpdateItem::CADDIE_PARTS, el.second._typeid, el.second.id }));

				_smp::message_pool::getInstance().push(new message("[player_manager::checkCaddie][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] Caddie Parts[TYPEID=" + std::to_string(el.second._typeid) + ", END_DATE=" + std::to_string(el.second.id)
						+ "] acabou o tempo, coloca no vector de update itens.", CL_FILE_LOG_AND_CONSOLE));

				el.second.parts_typeid = 0u;
				el.second.parts_end_date_unix = 0u;
				el.second.end_parts_date = { 0 };

#ifdef _DEBUG

				_smp::message_pool::getInstance().push(new message("[player_manager::checkCaddie][Log] hex: " + hex_util::BufferToHexString((unsigned char*)&el.second, sizeof(CaddieInfoEx)), CL_FILE_LOG_AND_CONSOLE));
#endif

				snmdb::NormalManagerDB::getInstance().add(1, new CmdUpdateCaddieInfo(_session.m_pi.uid, el.second), player_manager::SQLDBResponse, nullptr);
			}
		}
	}
};

inline void player_manager::checkMascot(player& _session) {
	CHECK_PLAYER(_session);

	for (auto& el : _session.m_pi.mp_mi) {

		if (el.second.tipo == 1 && getLocalTimeDiffDESC(el.second.data) <= 0ll) {

			if (_session.m_pi.findUpdateItemByIdAndType(el.second.id, UpdateItem::MASCOT) == _session.m_pi.mp_ui.end()) {

				_session.m_pi.mp_ui.insert(std::make_pair(PlayerInfo::stIdentifyKey(el.second._typeid, el.second.id), UpdateItem{ UpdateItem::MASCOT, el.second._typeid, el.second.id }));

				_smp::message_pool::getInstance().push(new message("[player_manager::checkMascot][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] Mascout[TYPEID=" + std::to_string(el.second._typeid) + ", ID=" + std::to_string(el.second.id)
						+ ", END_DATE=" + _formatDate(el.second.data) + "] acabou o tempo dele, coloca no vector de update itens.", CL_FILE_LOG_AND_CONSOLE));

				if ((_session.m_pi.ei.mascot_info != nullptr && _session.m_pi.ei.mascot_info->id == el.second.id) || _session.m_pi.ue.mascot_id == el.second.id) {

					_session.m_pi.ei.mascot_info = nullptr;
					_session.m_pi.ue.mascot_id = 0;

					_smp::message_pool::getInstance().push(new message("[player_manager::checkMascot][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] Desequipando Mascot[TYPEID=" + std::to_string(el.second._typeid) + ", ID=" + std::to_string(el.second.id)
							+ "].", CL_FILE_LOG_AND_CONSOLE));
				}
			}
		}
	}
};

inline void player_manager::checkWarehouse(player& _session) {

	for (auto& el : _session.m_pi.mp_wi) {

		if (((el.second.flag & 0x20) || (el.second.flag & 0x40) || (el.second.flag & 0x60))
				&& el.second.end_date_unix_local > 0) {

			auto st = UnixToSystemTime(el.second.end_date_unix_local);

			if (getLocalTimeDiffDESC(st) <= 0ll) {

				if (_session.m_pi.findUpdateItemByIdAndType(el.second.id, UpdateItem::WAREHOUSE) == _session.m_pi.mp_ui.end()) {

					_session.m_pi.mp_ui.insert(std::make_pair(PlayerInfo::stIdentifyKey(el.second._typeid, el.second.id), UpdateItem{ UpdateItem::WAREHOUSE, (uint32_t)el.second._typeid, el.second.id }));

					_smp::message_pool::getInstance().push(new message("[player_manager::checkWarehouse][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
							+ "] Warehouse Item[TYPEID=" + std::to_string(el.second._typeid) + ", ID=" + std::to_string(el.second.id)
							+ ", END_DATE=" + formatDateLocal(el.second.end_date_unix_local) + "] acabou o tempo dele, coloca no vector de update itens.", CL_FILE_LOG_AND_CONSOLE));

					if (sIff::getInstance().getItemGroupIdentify(el.second._typeid) == iff::PART && _session.m_pi.isPartEquiped(el.second._typeid, el.second.id)) {

						auto ci = _session.m_pi.findCharacterByTypeid((iff::CHARACTER << 26) | sIff::getInstance().getItemCharIdentify(el.second._typeid));

						if (ci != nullptr) {

							auto part = sIff::getInstance().findPart(el.second._typeid);

							if (part != nullptr) {

								ci->unequipPart(part);

								_smp::message_pool::getInstance().push(new message("[player_manager::checkWarehouse][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
										+ "] Desequipando Part[TYPEID=" + std::to_string(el.second._typeid) + ", ID=" + std::to_string(el.second.id)
										+ "] do Character[TYPEID=" + std::to_string(ci->_typeid) + "], coloca parts default no lugar do part que estava equipado.", CL_FILE_LOG_AND_CONSOLE));

							}else {

								for (auto i = 0u; i < (sizeof(ci->parts_typeid) / sizeof(ci->parts_typeid[0])); ++i) {

									if (ci->parts_id[i] == el.second.id && ci->parts_typeid[i] == el.second._typeid) {
										ci->parts_typeid[i] = 0;
										ci->parts_id[i] = 0;
									}
								}

								_smp::message_pool::getInstance().push(new message("[player_manager::checkWarehouse][Error] player[UID=" + std::to_string(_session.m_pi.uid)
										+ "] nao tem o Part[TYPEID=" + std::to_string(el.second._typeid) + "] do Character[TYPEID=" + std::to_string(ci->_typeid) + "], no IFF_STRUCT desequipa ele. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
							}

							snmdb::NormalManagerDB::getInstance().add(2, new CmdUpdateCharacterAllPartEquiped(_session.m_pi.uid, *ci), player_manager::SQLDBResponse, nullptr);

						}else
							_smp::message_pool::getInstance().push(new message("[player_manager::checkWarehouse][Error][WARNING] player[UID=" + std::to_string(_session.m_pi.uid)
									+ "] nao tem o Character[TYPEID=" + std::to_string((iff::CHARACTER << 26) | sIff::getInstance().getItemCharIdentify(el.second._typeid)) + "]. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));

					}

					if (sIff::getInstance().getItemGroupIdentify(el.second._typeid) == iff::CLUBSET && _session.m_pi.ei.clubset != nullptr
							&& _session.m_pi.ei.clubset->id == el.second.id || _session.m_pi.ue.clubset_id == el.second.id) {

						auto it = _session.m_pi.findWarehouseItemItByTypeid(AIR_KNIGHT_SET);

						if (it != _session.m_pi.mp_wi.end()) {

							_session.m_pi.ei.clubset = &it->second;
							_session.m_pi.ue.clubset_id = it->second.id;

							_session.m_pi.ei.csi = { it->second.id, it->second._typeid, it->second.c };

							IFF::ClubSet *cs = sIff::getInstance().findClubSet(it->second._typeid);

							if (cs != nullptr)
								for (auto i = 0u; i < (sizeof(_session.m_pi.ei.csi.enchant_c) / sizeof(_session.m_pi.ei.csi.enchant_c[0])); ++i)
									_session.m_pi.ei.csi.enchant_c[i] = cs->slot[i] + it->second.clubset_workshop.c[i];
						}

						_smp::message_pool::getInstance().push(new message("[player_manager::checkWarehouse][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
								+ "] Desequipando ClubSet[TYPEID=" + std::to_string(el.second._typeid) + ", ID=" + std::to_string(el.second.id)
								+ "]" + (it != _session.m_pi.mp_wi.end() ? ", e colocando o Air Knight Set[TYPEID=" + std::to_string(it->second._typeid) + ", ID="
								+ std::to_string(it->second.id) + "] no lugar." : "."), CL_FILE_LOG_AND_CONSOLE));
					}

					if (sIff::getInstance().getItemGroupIdentify(el.second._typeid) == iff::BALL && _session.m_pi.ei.comet != nullptr
							&& _session.m_pi.ei.comet->id == el.second.id || _session.m_pi.ue.ball_typeid == el.second._typeid) {

						auto it = _session.m_pi.findWarehouseItemItByTypeid(DEFAULT_COMET_TYPEID);

						if (it != _session.m_pi.mp_wi.end()) {

							_session.m_pi.ei.comet = &it->second;
							_session.m_pi.ue.ball_typeid = DEFAULT_COMET_TYPEID;
						}

						_smp::message_pool::getInstance().push(new message("[player_manager::checkWarehouse][Log] Player[UID=" + std::to_string(_session.m_pi.uid)
								+ "] Desequipando Ball[TYPEID=" + std::to_string(el.second._typeid) + ", ID=" + std::to_string(el.second.id)
								+ "]" + (it != _session.m_pi.mp_wi.end() ? ", e colocando a Ball[TYPEID=" + std::to_string(it->second._typeid) + ", ID="
								+ std::to_string(it->second.id) + "] padrao no lugar." : "."), CL_FILE_LOG_AND_CONSOLE));
					}

					if (sIff::getInstance().getItemGroupIdentify(el.second._typeid) == iff::SKIN) {

						for (auto i = 0u; i < (sizeof(_session.m_pi.ue.skin_typeid) / sizeof(_session.m_pi.ue.skin_typeid[0])); ++i) {

							if (_session.m_pi.ue.skin_typeid[i] == el.second._typeid && _session.m_pi.ue.skin_id[i] == el.second.id) {

								_session.m_pi.ue.skin_id[i] = 0;
								_session.m_pi.ue.skin_typeid[i] = 0;

								_smp::message_pool::getInstance().push(new message("[player_manager::checkWarehouse][Log] player[UID=" + std::to_string(_session.m_pi.uid)
										+ "] Desequipando SKIN[TYPEID=" + std::to_string(el.second._typeid) + ", ID=" + std::to_string(el.second.id)
										+ ", SLOT=" + std::to_string(i) + "]", CL_FILE_LOG_AND_CONSOLE));

								break;
							}
						}
					}

					if (sIff::getInstance().getItemGroupIdentify(el.second._typeid) == iff::ITEM && sPremiumSystem::getInstance().isPremiumTicket(el.second._typeid)) {

						_smp::message_pool::getInstance().push(new message("[player_manager::checkWarehouse][Log] player[UID=" + std::to_string(_session.m_pi.uid)
								+ "] Tirando o Modo Premium User do Player, acabou o tempo do ticket, tirando a capacidade e a Comet(Ball)", CL_FILE_LOG_AND_CONSOLE));

						sPremiumSystem::getInstance().removePremiumUser(_session);
					}
				}
			}
		}
	}
};

bool player_manager::checkGameGuard(player& _session) {

	if (_session.getState()) {

#if INTPTR_MAX == INT64_MAX && MY_GG_SRV_LIB == 0
		auto now = std::time(nullptr);

		if (!_session.m_gg.m_auth_reply)
			return true;
		else if (_session.m_gg.m_auth_time < now) {

			DWORD err = _session.m_gg.m_csa.GetAuthQuery();

			if (err != ERROR_SUCCESS) {

				_smp::message_pool::getInstance().push(new message("[player_manager::checkGameGuard][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "] CSAuth Fail Err: " + std::to_string(err)
						+ ", Index: " + std::to_string(_session.m_gg.m_csa.m_AuthQuery.dwIndex) + ", Value1: " + std::to_string(_session.m_gg.m_csa.m_AuthQuery.dwValue1)
						+ ", Value2: " + std::to_string(_session.m_gg.m_csa.m_AuthQuery.dwValue2) + ", Value3: " + std::to_string(_session.m_gg.m_csa.m_AuthQuery.dwValue3), CL_FILE_LOG_AND_CONSOLE));

				return true;
			}
		}
#elif INTPTR_MAX == INT32_MAX || MY_GG_SRV_LIB == 1
		auto now = time(nullptr);

		if (!_session.m_gg.m_auth_reply)
			return true;
		else if (_session.m_gg.m_auth_time < now) {

			DWORD err = _session.m_gg.m_csa.GetAuthQuery();

			if (err == ERROR_SUCCESS) {

#ifdef _DEBUG
				_smp::message_pool::getInstance().push(new message("[player_manager::checkGameGuard][Log] Player[UID=" + std::to_string(_session.m_pi.uid) + "] send GameGuard Auth Query. Index: "
						+ std::to_string(_session.m_gg.m_csa.m_AuthQuery.dwIndex) + ", Value1: " + std::to_string(_session.m_gg.m_csa.m_AuthQuery.dwValue1)
						+ ", Value2: " + std::to_string(_session.m_gg.m_csa.m_AuthQuery.dwValue2) + ", Value3: " + std::to_string(_session.m_gg.m_csa.m_AuthQuery.dwValue3), CL_FILE_LOG_AND_CONSOLE));
#endif

				_session.m_gg.m_auth_time = now + (sRandomGen::getInstance().rIbeMt19937_64_chronoRange(0, AUTH_PERIOD_VARIATION_SEC) + AUTH_PERIOD_SEC);

				packet p((unsigned short)0xD7);

				p.addBuffer(&_session.m_gg.m_csa.m_AuthQuery, sizeof(_GG_AUTH_DATA));

				packet_func::session_send(p, &_session, 1);

			}else {

				_smp::message_pool::getInstance().push(new message("[player_manager::checkGameGuard][Error] Player[UID=" + std::to_string(_session.m_pi.uid) + "] CSAuth Fail Err: " + std::to_string(err)
						+ ", Index: " + std::to_string(_session.m_gg.m_csa.m_AuthQuery.dwIndex) + ", Value1: " + std::to_string(_session.m_gg.m_csa.m_AuthQuery.dwValue1)
						+ ", Value2: " + std::to_string(_session.m_gg.m_csa.m_AuthQuery.dwValue2) + ", Value3: " + std::to_string(_session.m_gg.m_csa.m_AuthQuery.dwValue3), CL_FILE_LOG_AND_CONSOLE));

				return true;
			}
		}
#else
#error Unknown pointer size or missing size macros!
#endif
	}

	return false;
};

void player_manager::SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg) {

	if (_arg == nullptr) {
#ifdef _DEBUG

		_smp::message_pool::getInstance().push(new message("[player_manager::SQLDBResponse]WARNING] _arg is nullptr", CL_FILE_LOG_AND_CONSOLE));
#endif
		return;
	}

	if (_pangya_db.getException().getCodeError() != 0) {
		_smp::message_pool::getInstance().push(new message("[player_manager::SQLDBResponse][Error] " + _pangya_db.getException().getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
		return;
	}

	switch (_msg_id) {
	case 1:
	{
		auto cmd_uci = reinterpret_cast< CmdUpdateCaddieInfo* >(&_pangya_db);

		_smp::message_pool::getInstance().push(new message("[player_manager::SQLDBResponse][Log] player[UID=" + std::to_string(cmd_uci->getUID()) + "] Atualizou Caddie Info[TYPEID="
				+ std::to_string(cmd_uci->getInfo()._typeid) + ", ID=" + std::to_string(cmd_uci->getInfo().id) + ", PARTS_TYPEID=" + std::to_string(cmd_uci->getInfo().parts_typeid)
				+ ", END_DATE=" + _formatDate(cmd_uci->getInfo().end_date) + ", PARTS_END_DATE=" + _formatDate(cmd_uci->getInfo().end_parts_date) + "] com sucesso!", CL_FILE_LOG_AND_CONSOLE));

		break;
	}
	case 2:
	{
		break;
	}
	case 0:
	default:
		break;
	}
};

int32_t player_manager::findSessionFree() {

	for (auto i = 0u; i < m_sessions.size(); ++i)
		if (
#if defined(_WIN32)
			m_sessions[i]->m_sock == INVALID_SOCKET
#elif defined(__linux__)
			m_sessions[i]->m_sock.fd == INVALID_SOCKET
#endif
		)
			return (int)getNewOID();

	return -1;
};

uint32_t player_manager::getNewOID() {

	uint32_t oid = 0u;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	auto it = std::find_if(m_indexes.begin(), m_indexes.end(), [](auto& el) {
		return el.second.ucFlag == 0;
	});

	if (it != m_indexes.end()) {

		it->second.flag.busy = 1;

		oid = it->first;

	}else {

		oid = (uint32_t)m_indexes.size();

		m_indexes[oid] = { 1  };
	}

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif

	return oid;
};

void player_manager::freeOID(uint32_t _oid) {

	auto it = m_indexes.find(_oid);

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	if (it != m_indexes.end()) {

		it->second.flag.busy = 0;

		if (it->second.flag.block)
			_smp::message_pool::getInstance().push(new message("[player_manager::freeOID][WARNING] index[OID=" + std::to_string(it->first) + "] esta bloqueado, nao pode liberar ele agora", CL_FILE_LOG_AND_CONSOLE));
	}else
		_smp::message_pool::getInstance().push(new message("[player_manager::freeOID][WARNING] index[OID=" + std::to_string(_oid) + "] nao esta no mapa.", CL_FILE_LOG_AND_CONSOLE));

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif
};
