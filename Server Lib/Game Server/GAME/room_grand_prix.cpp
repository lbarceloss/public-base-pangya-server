
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "room_grand_prix.hpp"

#include "../PACKET/packet_func_sv.h"

#include "../Game Server/game_server.h"

#include "grand_prix.hpp"

#include "../UTIL/lottery.hpp"

#include "item_manager.h"

using namespace stdA;

#define CHECK_SESSION_BEGIN(method) if (!_session.getState()) \
										throw exception("[RoomGrandPrix::" + std::string((method)) + "][Error] player nao esta connectado", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_PRIX, 12, 0)); \

#define REQUEST_BEGIN(method) CHECK_SESSION_BEGIN(std::string("request") + (method)) \
							  if (_packet == nullptr) \
									throw exception("[RoomGrandPrix::request" + std::string((method)) + "][Error] _packet is nullptr", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_PRIX, 12, 0)); \

RoomGrandPrix::RoomGrandPrix(unsigned char _channel_owner, RoomInfoEx _ri, IFF::GrandPrixData& _gp)
	: room(_channel_owner, _ri), m_gp(_gp), m_count_down(nullptr) {

	push_instancia(this);

	if (!(sIff::getInstance().getGrandPrixAba(m_gp._typeid) == IFF::GrandPrixData::GP_ABA::ROOKIE && sIff::getInstance().isGrandPrixNormal(m_gp._typeid))) {

		SYSTEMTIME local{ 0 };

		GetLocalTime(&local);

		local.wDay = local.wDayOfWeek = local.wMonth = local.wYear = 0u;

		if (m_gp.open.wHour >= 23 && m_gp.start.wHour <= 1)
			m_gp.start.wDay = 1u;

		auto diff = (!isEmpty(m_gp.start) ? getHourDiff(m_gp.start, local) : 0ll);

		if (diff < 0ll)
			diff = 0ll;
		else
			diff = (int64_t)std::round(diff / 1000.f);

		count_down_to_start(diff);

	}

}

RoomGrandPrix::~RoomGrandPrix() {

	if (m_count_down != nullptr) {

		sgs::gs::getInstance().unMakeTime(m_count_down);

		m_count_down = nullptr;
	}

	pop_instancia(this);
}

bool RoomGrandPrix::isAllReady() {

	return !_haveInvited();
}

void RoomGrandPrix::requestChangePlayerItemRoom(player& _session, ChangePlayerItemRoom& _cpir) {
	CHECK_SESSION_BEGIN("ChangePlayerItemRoom");

	packet p;

	try {

		auto gp_condition = sIff::getInstance().findGrandPrixConditionEquip(m_gp.typeid_link);

		if (gp_condition != nullptr) {

			auto grup_type = sIff::getInstance().getItemGroupIdentify(gp_condition->item_typeid);

			switch (_cpir.type) {
				case ChangePlayerItemRoom::TYPE_CHANGE::TC_CADDIE:
					if (grup_type == iff::CADDIE) {
						CaddieInfoEx *pCi = nullptr;

						if (_cpir.caddie != 0 && (pCi = _session.m_pi.findCaddieById(_cpir.caddie)) != nullptr
								&& sIff::getInstance().getItemGroupIdentify(pCi->_typeid) == iff::CADDIE) {

							if (gp_condition->item_typeid != pCi->_typeid) {

								if ((pCi = _session.m_pi.findCaddieByTypeid(gp_condition->item_typeid)) != nullptr && sIff::getInstance().getItemGroupIdentify(pCi->_typeid) == iff::CADDIE) {

									_cpir.caddie = pCi->id;

									_session.m_pi.ei.cad_info = pCi;
									_session.m_pi.ue.caddie_id = pCi->id;

									packet_func::pacote04B(p, &_session, ChangePlayerItemRoom::TYPE_CHANGE::TC_CADDIE, 0);
									packet_func::room_broadcast(*this, p, 1);

								}else {

									_smp::message_pool::getInstance().push(new message("[RoomGrandPrix::requestChangePlayerItemRoom][WARNING] Player[UID=" + std::to_string(_session.m_pi.uid)
											+ "] tentou trocar item[ID=" + std::to_string(_cpir.caddie) + "] equipado na sala[NUMERO=" + std::to_string(m_ri.numero)
											+ "] Grand Prix, mas a sala tem uma condicao que nao pode trocar o caddie equipada. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
								}
							}
						}
					}
					break;
				case ChangePlayerItemRoom::TYPE_CHANGE::TC_BALL:
					if (grup_type == iff::BALL) {
						WarehouseItemEx *pWi = nullptr;

						if (_cpir.ball != 0 && (pWi = _session.m_pi.findWarehouseItemByTypeid(_cpir.ball)) != nullptr
								&& sIff::getInstance().getItemGroupIdentify(pWi->_typeid) == iff::BALL) {

							if (gp_condition->item_typeid != pWi->_typeid) {

								if ((pWi = _session.m_pi.findWarehouseItemByTypeid(gp_condition->item_typeid)) != nullptr && sIff::getInstance().getItemGroupIdentify(pWi->_typeid) == iff::BALL) {

									_cpir.ball = pWi->_typeid;

									_session.m_pi.ei.comet = pWi;
									_session.m_pi.ue.ball_typeid = pWi->_typeid;

									packet_func::pacote04B(p, &_session, ChangePlayerItemRoom::TYPE_CHANGE::TC_BALL, 0);
									packet_func::room_broadcast(*this, p, 1);

								}else {

									_smp::message_pool::getInstance().push(new message("[RoomGrandPrix::requestChangePlayerItemRoom][WARNING] Player[UID=" + std::to_string(_session.m_pi.uid)
											+ "] tentou trocar item[TYPEID=" + std::to_string(_cpir.ball) + "] equipado na sala[NUMERO=" + std::to_string(m_ri.numero)
											+ "] Grand Prix, mas a sala tem uma condicao que nao pode trocar a bola equipada. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
								}
							}
						}
					}
					break;
				case ChangePlayerItemRoom::TYPE_CHANGE::TC_CLUBSET:
					if (grup_type == iff::CLUBSET) {
						WarehouseItemEx *pWi = nullptr;

						if (_cpir.clubset != 0 && (pWi = _session.m_pi.findWarehouseItemById(_cpir.clubset)) != nullptr
								&& sIff::getInstance().getItemGroupIdentify(pWi->_typeid) == iff::CLUBSET) {

							if (gp_condition->item_typeid != pWi->_typeid) {

								if ((pWi = _session.m_pi.findWarehouseItemByTypeid(gp_condition->item_typeid)) != nullptr && sIff::getInstance().getItemGroupIdentify(pWi->_typeid) == iff::CLUBSET) {

									_cpir.clubset = pWi->id;

									_session.m_pi.ei.clubset = pWi;

									_session.m_pi.ei.csi = { pWi->id, pWi->_typeid, pWi->c };

									IFF::ClubSet *cs = sIff::getInstance().findClubSet(pWi->_typeid);

									if (cs != nullptr) {

										for (auto j = 0u; j < (sizeof(_session.m_pi.ei.csi.enchant_c) / sizeof(short)); ++j)
											_session.m_pi.ei.csi.enchant_c[j] = cs->slot[j] + pWi->clubset_workshop.c[j];

										_session.m_pi.ue.clubset_id = pWi->id;
									}

									packet_func::pacote04B(p, &_session, ChangePlayerItemRoom::TYPE_CHANGE::TC_CLUBSET, 0);
									packet_func::room_broadcast(*this, p, 1);

								}else {

									_smp::message_pool::getInstance().push(new message("[RoomGrandPrix::requestChangePlayerItemRoom][WARNING] Player[UID=" + std::to_string(_session.m_pi.uid)
											+ "] tentou trocar item[ID=" + std::to_string(_cpir.clubset) + "] equipado na sala[NUMERO=" + std::to_string(m_ri.numero)
											+ "] Grand Prix, mas a sala tem uma condicao que nao pode trocar o ClubSet equipada. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
								}
							}
						}
					}
					break;
				case ChangePlayerItemRoom::TYPE_CHANGE::TC_CHARACTER:
					if (grup_type == iff::CHARACTER) {
						CharacterInfo *pCe = nullptr;

						if (_cpir.character != 0 && (pCe = _session.m_pi.findCharacterById(_cpir.character)) != nullptr
								&& sIff::getInstance().getItemGroupIdentify(pCe->_typeid) == iff::CHARACTER) {

							if (gp_condition->item_typeid != pCe->_typeid) {

								if ((pCe = _session.m_pi.findCharacterByTypeid(gp_condition->item_typeid)) != nullptr && sIff::getInstance().getItemGroupIdentify(pCe->_typeid) == iff::CHARACTER) {

									_cpir.character = pCe->id;

									_session.m_pi.ei.char_info = pCe;
									_session.m_pi.ue.character_id = pCe->id;

									packet_func::pacote06B(p, &_session, &_session.m_pi, 5 , 4 );
									packet_func::session_send(p, &_session, 1);

								}else {

									_smp::message_pool::getInstance().push(new message("[RoomGrandPrix::requestChangePlayerItemRoom][WARNING] Player[UID=" + std::to_string(_session.m_pi.uid)
											+ "] tentou trocar item[ID=" + std::to_string(_cpir.character) + "] equipado na sala[NUMERO=" + std::to_string(m_ri.numero)
											+ "] Grand Prix, mas a sala tem uma condicao que nao pode trocar o character equipada. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
								}
							}
						}
					}
					break;
				case ChangePlayerItemRoom::TYPE_CHANGE::TC_MASCOT:
					if (grup_type == iff::MASCOT) {
						MascotInfoEx *pMi = nullptr;

						if (_cpir.mascot != 0 && (pMi = _session.m_pi.findMascotById(_cpir.mascot)) != nullptr
								&& sIff::getInstance().getItemGroupIdentify(pMi->_typeid) == iff::MASCOT) {

							if (gp_condition->item_typeid != pMi->_typeid) {

								if ((pMi = _session.m_pi.findMascotByTypeid(gp_condition->item_typeid)) != nullptr && sIff::getInstance().getItemGroupIdentify(pMi->_typeid) == iff::MASCOT) {

									_cpir.mascot = pMi->id;

									_session.m_pi.ei.mascot_info = pMi;
									_session.m_pi.ue.mascot_id = pMi->id;

									packet_func::pacote04B(p, &_session, ChangePlayerItemRoom::TYPE_CHANGE::TC_MASCOT, 0);
									packet_func::room_broadcast(*this, p, 1);

								}else {

									_smp::message_pool::getInstance().push(new message("[RoomGrandPrix::requestChangePlayerItemRoom][WARNING] Player[UID=" + std::to_string(_session.m_pi.uid)
											+ "] tentou trocar item[ID=" + std::to_string(_cpir.mascot) + "] equipado na sala[NUMERO=" + std::to_string(m_ri.numero)
											+ "] Grand Prix, mas a sala tem uma condicao que nao pode trocar o mascot equipada. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
								}
							}
						}
					}
					break;
				case ChangePlayerItemRoom::TYPE_CHANGE::TC_ITEM_EFFECT_LOUNGE:
					if (grup_type == iff::PART) {

					}
					break;
				case ChangePlayerItemRoom::TYPE_CHANGE::TC_ALL:
				{
					if (grup_type == iff::CHARACTER) {
						CharacterInfo *pCe = nullptr;

						if (_cpir.character != 0 && (pCe = _session.m_pi.findCharacterById(_cpir.character)) != nullptr
								&& sIff::getInstance().getItemGroupIdentify(pCe->_typeid) == iff::CHARACTER) {

							if (gp_condition->item_typeid != pCe->_typeid) {

								if ((pCe = _session.m_pi.findCharacterByTypeid(gp_condition->item_typeid)) != nullptr && sIff::getInstance().getItemGroupIdentify(pCe->_typeid) == iff::CHARACTER) {

									_cpir.character = pCe->id;

									_session.m_pi.ei.char_info = pCe;
									_session.m_pi.ue.character_id = pCe->id;

									packet_func::pacote06B(p, &_session, &_session.m_pi, 5 , 4 );
									packet_func::session_send(p, &_session, 1);

								}else {

									_smp::message_pool::getInstance().push(new message("[RoomGrandPrix::requestChangePlayerItemRoom][WARNING] Player[UID=" + std::to_string(_session.m_pi.uid)
											+ "] tentou trocar item[ID=" + std::to_string(_cpir.character) + "] equipado na sala[NUMERO=" + std::to_string(m_ri.numero)
											+ "] Grand Prix, mas a sala tem uma condicao que nao pode trocar o character equipada. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
								}
							}
						}

					}else if (grup_type == iff::CADDIE) {
						CaddieInfoEx *pCi = nullptr;

						if (_cpir.caddie != 0 && (pCi = _session.m_pi.findCaddieById(_cpir.caddie)) != nullptr
								&& sIff::getInstance().getItemGroupIdentify(pCi->_typeid) == iff::CADDIE) {

							if (gp_condition->item_typeid != pCi->_typeid) {

								if ((pCi = _session.m_pi.findCaddieByTypeid(gp_condition->item_typeid)) != nullptr && sIff::getInstance().getItemGroupIdentify(pCi->_typeid) == iff::CADDIE) {

									_cpir.caddie = pCi->id;

									_session.m_pi.ei.cad_info = pCi;
									_session.m_pi.ue.caddie_id = pCi->id;

									packet_func::pacote04B(p, &_session, ChangePlayerItemRoom::TYPE_CHANGE::TC_CADDIE, 0);
									packet_func::room_broadcast(*this, p, 1);

								}else {

									_smp::message_pool::getInstance().push(new message("[RoomGrandPrix::requestChangePlayerItemRoom][WARNING] Player[UID=" + std::to_string(_session.m_pi.uid)
											+ "] tentou trocar item[ID=" + std::to_string(_cpir.caddie) + "] equipado na sala[NUMERO=" + std::to_string(m_ri.numero)
											+ "] Grand Prix, mas a sala tem uma condicao que nao pode trocar o caddie equipada. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
								}
							}
						}

					}else if (grup_type == iff::CLUBSET) {
						WarehouseItemEx *pWi = nullptr;

						if (_cpir.clubset != 0 && (pWi = _session.m_pi.findWarehouseItemById(_cpir.clubset)) != nullptr
								&& sIff::getInstance().getItemGroupIdentify(pWi->_typeid) == iff::CLUBSET) {

							if (gp_condition->item_typeid != pWi->_typeid) {

								if ((pWi = _session.m_pi.findWarehouseItemByTypeid(gp_condition->item_typeid)) != nullptr && sIff::getInstance().getItemGroupIdentify(pWi->_typeid) == iff::CLUBSET) {

									_cpir.clubset = pWi->id;

									_session.m_pi.ei.clubset = pWi;

									_session.m_pi.ei.csi = { pWi->id, pWi->_typeid, pWi->c };

									IFF::ClubSet *cs = sIff::getInstance().findClubSet(pWi->_typeid);

									if (cs != nullptr) {

										for (auto j = 0u; j < (sizeof(_session.m_pi.ei.csi.enchant_c) / sizeof(short)); ++j)
											_session.m_pi.ei.csi.enchant_c[j] = cs->slot[j] + pWi->clubset_workshop.c[j];

										_session.m_pi.ue.clubset_id = pWi->id;
									}

									packet_func::pacote04B(p, &_session, ChangePlayerItemRoom::TYPE_CHANGE::TC_CLUBSET, 0);
									packet_func::room_broadcast(*this, p, 1);

								}else {

									_smp::message_pool::getInstance().push(new message("[RoomGrandPrix::requestChangePlayerItemRoom][WARNING] Player[UID=" + std::to_string(_session.m_pi.uid)
											+ "] tentou trocar item[ID=" + std::to_string(_cpir.clubset) + "] equipado na sala[NUMERO=" + std::to_string(m_ri.numero)
											+ "] Grand Prix, mas a sala tem uma condicao que nao pode trocar o ClubSet equipada. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
								}
							}
						}

					}else if (grup_type == iff::BALL) {
						WarehouseItemEx *pWi = nullptr;

						if (_cpir.ball != 0 && (pWi = _session.m_pi.findWarehouseItemByTypeid(_cpir.ball)) != nullptr
								&& sIff::getInstance().getItemGroupIdentify(pWi->_typeid) == iff::BALL) {

							if (gp_condition->item_typeid != pWi->_typeid) {

								if ((pWi = _session.m_pi.findWarehouseItemByTypeid(gp_condition->item_typeid)) != nullptr && sIff::getInstance().getItemGroupIdentify(pWi->_typeid) == iff::BALL) {

									_cpir.ball = pWi->_typeid;

									_session.m_pi.ei.comet = pWi;
									_session.m_pi.ue.ball_typeid = pWi->_typeid;

									packet_func::pacote04B(p, &_session, ChangePlayerItemRoom::TYPE_CHANGE::TC_BALL, 0);
									packet_func::room_broadcast(*this, p, 1);

								}else {

									_smp::message_pool::getInstance().push(new message("[RoomGrandPrix::requestChangePlayerItemRoom][WARNING] Player[UID=" + std::to_string(_session.m_pi.uid)
											+ "] tentou trocar item[TYPEID=" + std::to_string(_cpir.ball) + "] equipado na sala[NUMERO=" + std::to_string(m_ri.numero)
											+ "] Grand Prix, mas a sala tem uma condicao que nao pode trocar a bola equipada. Hacker ou Bug", CL_FILE_LOG_AND_CONSOLE));
								}
							}
						}

					}
					break;
				}
			}

		}

		room::requestChangePlayerItemRoom(_session, _cpir);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[RoomGrandPrix::requestChangePlayerItemRoom][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		packet_func::pacote04B(p, &_session, _cpir.type, (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::ROOM ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 1 ));
		packet_func::session_send(p, &_session, 0);
	}
}

bool RoomGrandPrix::requestStartGame(player& _session, packet *_packet) {
	REQUEST_BEGIN("StartGame");

	packet p;

	bool ret = true;

	try {

		if (sIff::getInstance().getGrandPrixAba(m_gp._typeid) == IFF::GrandPrixData::GP_ABA::ROOKIE && !sIff::getInstance().isGrandPrixNormal(m_gp._typeid))
			throw exception("[RoomGrandPrix::requestStartGame][Error] player[UID=" + std::to_string(_session.m_pi.uid) + "] tentou comecar o jogo na sala[NUMERO="
					+ std::to_string(m_ri.numero) + "], mas a sala nao eh uma Grand Prix Rookie(Tuto). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_PRIX, 1, 0x5900201));

		if (sIff::getInstance().getGrandPrixAba(m_gp._typeid) != IFF::GrandPrixData::GP_ABA::ROOKIE)
			ret = room::requestStartGame(_session, _packet);
		else {

			if (m_pGame != nullptr)
				throw exception("[RoomGrandPrix::requestStartGame][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou comecar o jogo na sala[NUMERO=" + std::to_string(m_ri.numero) + "], mas ja tem um jogo inicializado. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_PRIX, 7, 0x5900202));

			if (!isAllReady())
				throw exception("[RoomGrandPrix::requestStartGame][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou comecar o jogo na sala[NUMERO=" + std::to_string(m_ri.numero) + ", MASTER=" + std::to_string(m_ri.master)
						+ "], mas nem todos jogadores estao prontos. Hacker ou Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_PRIX, 8, 0x5900202));

			if (m_ri.course >= 0x7Fu) {

				if (m_ri.tipo == RoomInfo::TIPO::SPECIAL_SHUFFLE_COURSE && m_ri.modo == Hole::eMODO::M_SHUFFLE_COURSE) {

					m_ri.course = RoomInfo::eCOURSE(0x80 | RoomInfo::eCOURSE::CHRONICLE_1_CHAOS );

				}else {

					Lottery lottery((uint64_t)this);

					for (auto& el : sIff::getInstance().getCourse()) {

						auto course_id = sIff::getInstance().getItemIdentify(el.second._typeid);

						if (course_id != 17  && course_id != 0x40 )
							lottery.push(100, course_id);
					}

					auto lc = lottery.spinRoleta();

					if (lc != nullptr)
						m_ri.course = RoomInfo::eCOURSE(0x80u | (unsigned char)lc->value);
				}
			}

			RateValue rv{ 0 };

			rv.exp = m_ri.rate_exp = sgs::gs::getInstance().getInfo().rate.exp;
			rv.pang = m_ri.rate_pang = sgs::gs::getInstance().getInfo().rate.pang;

			m_ri.angel_event = sgs::gs::getInstance().getInfo().rate.angel_event;

			rv.clubset = sgs::gs::getInstance().getInfo().rate.club_mastery;
			rv.rain = sgs::gs::getInstance().getInfo().rate.chuva;
			rv.treasure = sgs::gs::getInstance().getInfo().rate.treasure;

			rv.persist_rain = 0u;

			switch (m_ri.tipo) {
			case RoomInfo::TIPO::GRAND_PRIX:
				m_pGame = new GrandPrix(v_sessions, m_ri, rv, m_ri.channel_rookie, m_gp);
				break;
			default:
				throw exception("[RoomGrandPrix::requestStartGame][Error] player[UID=" + std::to_string(_session.m_pi.uid)
						+ "] tentou comecar o jogo na sala[NUMERO=" + std::to_string(m_ri.numero) + ", MASTER=" + std::to_string(m_ri.master)
						+ "], mas o tipo da sala nao eh Grand Prix. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_PRIX, 9, 0x5900202));
			}

			m_ri.state = 0;

			p.init_plain((unsigned short)0x253);

			p.addUint32(0u);

			packet_func::room_broadcast(*this, p, 1);

			p.init_plain((unsigned short)0x230);

			packet_func::room_broadcast(*this, p, 1);

			p.init_plain((unsigned short)0x231);

			packet_func::room_broadcast(*this, p, 1);

			uint32_t rate_pang = sgs::gs::getInstance().getInfo().rate.pang;

			p.init_plain((unsigned short)0x77);

			p.addUint32(rate_pang);

			packet_func::room_broadcast(*this, p, 1);
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[RoomGrandPrix::requestStartGame][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		p.init_plain((unsigned short)0x253);

		p.addUint32((STDA_SOURCE_ERROR_DECODE(e.getCodeError() == STDA_ERROR_TYPE::ROOM_GRAND_PRIX)) ? STDA_SYSTEM_ERROR_DECODE(e.getCodeError()) : 0x5900200);

		packet_func::session_send(p, &_session, 1);

		ret = false;
	}

	return ret;
}

bool RoomGrandPrix::startGame() {

	packet p;

	bool ret = true;

	try {

		if (sIff::getInstance().getGrandPrixAba(m_gp._typeid) == IFF::GrandPrixData::GP_ABA::ROOKIE && !sIff::getInstance().isGrandPrixNormal(m_gp._typeid))
			throw exception("[RoomGrandPrix::startGame][Error] Server tentou comecar o jogo na sala[NUMERO="
					+ std::to_string(m_ri.numero) + "], mas a sala nao eh uma Grand Prix Rookie(Tuto). Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_PRIX, 1, 0x5900201));

		if (m_pGame != nullptr)
			throw exception("[RoomGrandPrix::startGame][Error] Server tentou comecar o jogo na sala[NUMERO="
					+ std::to_string(m_ri.numero) + "], mas ja tem um jogo inicializado. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_PRIX, 7, 0x5900202));

		if (!isAllReady())
			throw exception("[RoomGrandPrix::startGame][Error] Server tentou comecar o jogo na sala[NUMERO="
					+ std::to_string(m_ri.numero) + ", MASTER=" + std::to_string(m_ri.master)
					+ "], mas nem todos jogadores estao prontos. Hacker ou Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_PRIX, 8, 0x5900202));

		if (m_ri.course >= 0x7Fu) {

			if (m_ri.tipo == RoomInfo::TIPO::SPECIAL_SHUFFLE_COURSE && m_ri.modo == Hole::eMODO::M_SHUFFLE_COURSE) {

				m_ri.course = RoomInfo::eCOURSE(0x80 | RoomInfo::eCOURSE::CHRONICLE_1_CHAOS );

			}else {

				Lottery lottery((uint64_t)this);

				for (auto& el : sIff::getInstance().getCourse()) {

					auto course_id = sIff::getInstance().getItemIdentify(el.second._typeid);

					if (course_id != 17  && course_id != 0x40 )
						lottery.push(100, course_id);
				}

				auto lc = lottery.spinRoleta();

				if (lc != nullptr)
					m_ri.course = RoomInfo::eCOURSE(0x80u | (unsigned char)lc->value);
			}
		}

		RateValue rv{ 0 };

		rv.exp = m_ri.rate_exp = sgs::gs::getInstance().getInfo().rate.exp;
		rv.pang = m_ri.rate_pang = sgs::gs::getInstance().getInfo().rate.pang;

		m_ri.angel_event = sgs::gs::getInstance().getInfo().rate.angel_event;

		rv.clubset = sgs::gs::getInstance().getInfo().rate.club_mastery;
		rv.rain = sgs::gs::getInstance().getInfo().rate.chuva;
		rv.treasure = sgs::gs::getInstance().getInfo().rate.treasure;

		rv.persist_rain = 0u;

		switch (m_ri.tipo) {
		case RoomInfo::TIPO::GRAND_PRIX:
			m_pGame = new GrandPrix(v_sessions, m_ri, rv, m_ri.channel_rookie, m_gp);
			break;
		default:
			throw exception("[RoomGrandPrix::startGame][Error] Server tentou comecar o jogo na sala[NUMERO="
					+ std::to_string(m_ri.numero) + ", MASTER=" + std::to_string(m_ri.master)
					+ "], mas o tipo da sala nao eh Grand Prix. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_PRIX, 9, 0x5900202));
		}

		m_ri.state = 0;

		p.init_plain((unsigned short)0x230);

		packet_func::room_broadcast(*this, p, 1);

		p.init_plain((unsigned short)0x231);

		packet_func::room_broadcast(*this, p, 1);

		uint32_t rate_pang = sgs::gs::getInstance().getInfo().rate.pang;

		p.init_plain((unsigned short)0x77);

		p.addUint32(rate_pang);

		packet_func::room_broadcast(*this, p, 1);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[RoomGrandPrix::startGame][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		ret = false;
	}

	return ret;
}

void RoomGrandPrix::initFirstInstance() {

	if (m_cs_instancia::getInstance().m_state && m_instancias::getInstance().empty())
		_smp::message_pool::getInstance().push(new message("[RoomGrandPrix::initFirstInstance][Log] Criou primeira instance do Singleton da classe Room Grand Prix static vector.", CL_FILE_LOG_AND_CONSOLE));
}

int RoomGrandPrix::_count_down_to_start(void* _arg1, void* _arg2) {

	RoomGrandPrix *_rgp = reinterpret_cast< RoomGrandPrix* >(_arg1);
	int64_t sec_to_start = reinterpret_cast< int64_t >(_arg2);

	try {

		if (_rgp != nullptr && instancia_valid(_rgp))
			_rgp->count_down_to_start(sec_to_start);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[RoomGrandPrix::_count_down_to_start][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	return 0;
}

void RoomGrandPrix::count_down_to_start(int64_t _sec_to_start) {

	try {

		lock();

		if (_sec_to_start <= 0) {

			if (m_count_down != nullptr) {

				sgs::gs::getInstance().unMakeTime(m_count_down);

				m_count_down = nullptr;
			}

			if (v_sessions.size() >= 1 && startGame())
				sgs::gs::getInstance().sendUpdateRoomInfo(this, 3);
			else if (v_sessions.size() >= 1)

				count_down_to_start(10);

		}else {

			uint32_t wait = 0u;
			int32_t rest = 0;

			unsigned char type = 1u;

			DWORD interval = 0u;
			float diff = 0.f;

			int32_t elapsed_sec = (m_count_down != nullptr) ? (int)std::round(m_count_down->getElapsed() / 1000.f)  : 0;

			_sec_to_start -= elapsed_sec;

			if ((diff = ((_sec_to_start - 10 ) / 30.f )) >= 1.f) {

				if ((_sec_to_start % 30) == 0) {

					interval = 30 * 1000;

					wait = interval * (int)diff;

				}else {

					wait = interval = (_sec_to_start % 30) * 1000;

				}

			}else if ((diff = ((_sec_to_start - 1 ) / 10.f )) >= 1.f) {

				if ((_sec_to_start % 10) == 0) {

					interval = 10 * 1000;

					wait = interval * (int)diff;

				}else {

					wait = interval = (_sec_to_start % 10) * 1000;
				}

			}else {

				diff = std::round(_sec_to_start / 1.f);

				interval = 1000;

				wait = interval * (int)diff;

			}

			packet p((unsigned short)0x40);

			p.addUint8(11);

			p.addUint16(0u);
			p.addUint16(0u);

			p.addUint32((uint32_t)_sec_to_start);

			packet_func::room_broadcast(*this, p, 1);

			if (m_count_down == nullptr || m_count_down->getState() == timer::STOP ||
				m_count_down->getState() == timer::STOPPING || m_count_down->getState() == timer::STOPPED) {

				job _job(RoomGrandPrix::_count_down_to_start, this, (void*)_sec_to_start);

				if (m_count_down != nullptr)
					sgs::gs::getInstance().unMakeTime(m_count_down);

				m_count_down = sgs::gs::getInstance().makeTime(wait, _job, std::vector< DWORD > { interval });
			}
		}

		unlock();

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[RoomGrandPrix::count_down_to_start][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		unlock();
	}
}

void RoomGrandPrix::push_instancia(RoomGrandPrix* _rgp) {

	m_cs_instancia::getInstance().lock();

	m_instancias::getInstance().push_back(RoomGrandPrixInstanciaCtx(_rgp, RoomGrandPrixInstanciaCtx::eSTATE::GOOD));

	m_cs_instancia::getInstance().unlock();
}

void RoomGrandPrix::pop_instancia(RoomGrandPrix* _rgp) {

	m_cs_instancia::getInstance().lock();

	auto index = get_instancia_index(_rgp);

	if (index >= 0)
		m_instancias::getInstance().erase(m_instancias::getInstance().begin() + index);

	m_cs_instancia::getInstance().unlock();
}

void RoomGrandPrix::set_instancia_state(RoomGrandPrix* _rgp, RoomGrandPrixInstanciaCtx::eSTATE _state) {

	m_cs_instancia::getInstance().lock();

	auto index = get_instancia_index(_rgp);

	if (index >= 0)
		m_instancias::getInstance()[index].m_state = _state;

	m_cs_instancia::getInstance().unlock();
}

int RoomGrandPrix::get_instancia_index(RoomGrandPrix* _rgp) {

	int index = -1;

	for (auto i = 0u; i < m_instancias::getInstance().size(); ++i) {

		if (m_instancias::getInstance()[i].m_rgp == _rgp) {

			index = (int)i;

			break;
		}
	}

	return index;
}

bool RoomGrandPrix::instancia_valid(RoomGrandPrix* _rgp) {

	bool valid = false;

	m_cs_instancia::getInstance().lock();

	auto index = get_instancia_index(_rgp);

	if (index >= 0)
		valid = (m_instancias::getInstance()[index].m_state == RoomGrandPrixInstanciaCtx::eSTATE::GOOD);

	m_cs_instancia::getInstance().unlock();

	return valid;
}
