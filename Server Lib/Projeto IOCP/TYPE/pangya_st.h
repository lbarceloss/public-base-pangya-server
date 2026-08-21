
#if defined(_WIN32)
#pragma pack(1)
#endif

#pragma once
#ifndef _STDA_PANGYA_ST_H
#define _STDA_PANGYA_ST_H

#include <vector>
#include <map>
#include <string>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include "../UTIL/WinPort.h"
#endif

#include <cstdint>

#include "../UTIL/iff.h"
#include "../TYPE/stdAType.h"

#include "../UTIL/message_pool.h"

#define	BLOCK_UNKNOWN_1				 (uint64_t)(1)
#define	BLOCK_PLAY_ALL				 (uint64_t)(1 << 1)
#define BLOCK_SHOP					 (uint64_t)(1 << 2)
#define BLOCK_GIFT_SHOP				 (uint64_t)(1 << 3)
#define BLOCK_PAPEL_SHOP			 (uint64_t)(1 << 4)
#define BLOCK_SHOP_LOUNGE			 (uint64_t)(1 << 5)
#define BLOCK_STROKE				 (uint64_t)(1 << 6)
#define BLOCK_MATCH					 (uint64_t)(1 << 7)
#define BLOCK_TOURNEY				 (uint64_t)(1 << 8)
#define BLOCK_SHORT_GAME			 (uint64_t)(1 << 9)
#define BLOCK_GUILD_BATTLE			 (uint64_t)(1 << 10)
#define BLOCK_PANG_BATTLE			 (uint64_t)(1 << 11)
#define BLOCK_APPROACH				 (uint64_t)(1 << 12)
#define BLOCK_LOUNGE				 (uint64_t)(1 << 13)
#define BLOCK_SCRATCHY				 (uint64_t)(1 << 14)
#define BLOCK_UNKNOWN_15			 (uint64_t)(1 << 15)
#define BLOCK_RANK_SERVER			 (uint64_t)(1 << 16)
#define BLOCK_TICKER				 (uint64_t)(1 << 17)
#define BLOCK_MAIL_BOX				 (uint64_t)(1 << 18)
#define BLOCK_GRAND_ZODIAC			 (uint64_t)(1 << 19)
#define BLOCK_SINGLE_PLAY			 (uint64_t)(1 << 20)
#define BLOCK_GRAND_PRIX			 (uint64_t)(1 << 21)
#define BLOCK_UNKNOWN_22			 (uint64_t)(1 << 22)
#define BLOCK_UNKNOWN_23			 (uint64_t)(1 << 23)
#define BLOCK_GUILD					 (uint64_t)(1 << 24)
#define BLOCK_SSC					 (uint64_t)(1 << 25)
#define BLOCK_UNKNOWN_26			 (uint64_t)(1 << 26)
#define BLOCK_UNKNOWN_27			 (uint64_t)(1 << 27)
#define BLOCK_MEMORIAL_SHOP			 (uint64_t)(1 << 28)
#define BLOCK_SHORT_GAME_29			 (uint64_t)(1 << 29)
#define BLOCK_CHAR_MASTERY			 (uint64_t)(1 << 30)
#define BLOCK_UNKNOWN_31			 (uint64_t)(1 << 31)
#define BLOCK_COPOUND_CARD_SYSTEM	 (uint64_t)(1 << 32)
#define BLOCK_RECYCLE_SYSTEM		 (uint64_t)(1 << 33)

#define S_FLAG_NORMAL				(unsigned int )(0)
#define S_FLAG_MANTLE				(unsigned int )(1 << 5)
#define S_FLAG_ROOKIE				(unsigned int )(1 << 7)
#define S_FLAG_NATURAL				(unsigned int )(1 << 8)
#define S_FLAG_VERDE				(unsigned int )(1 << 10)
#define S_FLAG_AZUL					(unsigned int )(1 << 11)
#define S_FLAG_GRAND_PRIX			(unsigned int )(1 << 12)

#define SE_FLAG_NORMAL				(unsigned short)(0)
#define SE_FLAG_PANG				(unsigned short)(1 << 1)
#define SE_FLAG_EXP					(unsigned short)(1 << 2)
#define SE_FLAG_ANGEL				(unsigned short)(1 << 3)
#define SE_FLAG_EXP_3				(unsigned short)(1 << 4)
#define SE_FLAG_CLUB_MASTERY		(unsigned short)(1 << 8)

#define LAST_ELEMENT_IN_ARRAY(_element_arr) ((_element_arr) + (sizeof((_element_arr)) / sizeof((_element_arr)[0])))

#define N_ELEMENT_IN_ARRAY(_element_arr, _n) (((_n) > (sizeof((_element_arr)) / sizeof((_element_arr)[0]))) ?   (_element_arr) : ((_element_arr) + (sizeof((_element_arr)[0]) * (_n))))

const unsigned int  angel_wings[]{ 134309888u, 134580224u, 134842368u, 135120896u, 135366656u, 135661568u, 135858176u, 136194048u, 136398848u, 136660992u, 137185294u, 137447424u, 138004480u };
const unsigned int  gacha_angel_wings[]{ 134309903u, 134580239u, 134842383u, 135120911u, 135366671u, 135661583u, 135858191u, 136194063u, 136398863u, 136661007u, 136923153u, 137185284u, 137447436u, 138004492u };

namespace stdA {

#if defined(__linux__)
#pragma pack(1)
#endif

	union uFlag {
		uFlag(uint64_t _ull = 0u) : ullFlag(_ull) {};
		void clear() { ullFlag = 0u; };
		uint64_t ullFlag;
		struct {
			uint64_t : 1;
			uint64_t all_game : 1;
			uint64_t buy_and_gift_shop : 1;
			uint64_t gift_shop : 1;
			uint64_t papel_shop : 1;
			uint64_t personal_shop : 1;
			uint64_t stroke : 1;
			uint64_t match : 1;
			uint64_t tourney : 1;
			uint64_t team_tourney : 1;
			uint64_t guild_battle : 1;
			uint64_t pang_battle : 1;
			uint64_t approach : 1;
			uint64_t lounge : 1;
			uint64_t scratchy : 1;
			uint64_t : 1;
			uint64_t rank_server : 1;
			uint64_t ticker : 1;
			uint64_t mail_box : 1;
			uint64_t grand_zodiac : 1;
			uint64_t single_play : 1;
			uint64_t grand_prix : 1;
			uint64_t : 2;
			uint64_t guild : 1;
			uint64_t ssc : 1;
			uint64_t : 2;
			uint64_t memorial_shop : 1;
			uint64_t short_game : 1;
			uint64_t char_mastery : 1;
			uint64_t : 1;
			uint64_t lolo_copound_card : 1;
			uint64_t cadie_recycle : 1;
			uint64_t legacy_tiki_shop : 1, : 0;
		}stBit;
	};

	union uProperty {
		uProperty(unsigned int  _ul = 0u) : ulProperty(_ul) {};
		void clear() { ulProperty = 0u; };
		unsigned int  ulProperty;
		struct {
			unsigned int  : 4;
			unsigned int  mantle : 1;
			unsigned int  : 1;
			unsigned int  only_rookie : 1;
			unsigned int  natural : 1;
			unsigned int  : 1;
			unsigned int  verde : 1;
			unsigned int  azul : 1;
			unsigned int  grand_prix : 1, : 0;
		}stBit;
	};

	union uEventFlag {
		uEventFlag(unsigned short _us = 0u) : usEventFlag(_us) {};
		void clear() { usEventFlag = 0u; };
		unsigned short usEventFlag;
		struct {
			unsigned short : 1;
			unsigned short pang_x_plus : 1;
			unsigned short exp_x2 : 1;
			unsigned short angel_wing : 1;
			unsigned short exp_x_plus : 1;
			unsigned short : 2;
			unsigned short club_mastery_x_plus : 1, : 0;
		}stBit;
	};

	struct ServerInfo {
		void clear() {
			memset(this, 0, sizeof(ServerInfo));
			uid = -1;
		};
		char nome[40];
		int  uid;
		int max_user;
		int curr_user;
		char ip[18];
		int port;

		uProperty propriedade;
		int angelic_wings_num;

		uEventFlag event_flag;
		__int16 event_map;
		__int16 app_rate;
		__int16 unknown;
		__int16 img_no;
	};

	struct RateConfigInfo {
		RateConfigInfo(unsigned int  _ul = 0u) {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(RateConfigInfo));
		};
		unsigned short scratchy;
		unsigned short papel_shop_rare_item;
		unsigned short papel_shop_cookie_item;
		unsigned short treasure;
		unsigned short pang;
		unsigned short exp;
		unsigned short club_mastery;
		unsigned short chuva;
		unsigned short memorial_shop;
		unsigned short grand_zodiac_event_time;
		unsigned short angel_event;
		unsigned short grand_prix_event;
		unsigned short golden_time_event;
		unsigned short login_reward_event;
		unsigned short bot_gm_event;
		unsigned short smart_calculator;
		unsigned int  countBitGrandPrixEvent() {

			unsigned int  count = 0u;

			for (auto i = 0u; i < 16u; ++i)
				if ((grand_prix_event >> i) & 1)
					count++;

			return count;
		};
		std::vector< unsigned int  > getValueBitGrandPrixEvent() {

			std::vector< unsigned int  > v_value;

			for (auto i = 0u; i < 16u; ++i)
				if ((grand_prix_event >> i) & 1)
					v_value.push_back(i + 1);

			return v_value;
		};
		bool checkBitGrandPrixEvent(unsigned int  _type) {

			if (_type == 0u)
				return false;

			return ((grand_prix_event >> (_type - 1)) & 1);
		};
		std::string toString() {
			return "GRAND_ZODIAC_EVENT_TIME=" + std::to_string(grand_zodiac_event_time) + ", GOLDEN_TIME_EVENT=" + std::to_string(golden_time_event)
				+ ", ANGEL_EVENT=" + std::to_string(angel_event) + ", GRAND_PRIX_EVENT=" + std::to_string(grand_prix_event)
				+ ", LOGIN_REWARD_EVENT=" + std::to_string(login_reward_event) + ", BOT_GM_EVENT=" + std::to_string(bot_gm_event)
				+ ", SMART_CALCULATOR_SYSTEM=" + std::to_string(smart_calculator) +
				+ ", SCRATCHY=" + std::to_string(scratchy) + ", PAPEL_SHOP_RARE_ITEM=" + std::to_string(papel_shop_rare_item)
				+ ", PAPEL_SHOP_COOKIE_ITEM=" + std::to_string(papel_shop_cookie_item) + ", TREASURE=" + std::to_string(treasure)
				+ ", PANG=" + std::to_string(pang) + ", EXP=" + std::to_string(exp) + ", CLUB_MASTERY=" + std::to_string(club_mastery)
				+ ", CHUVA=" + std::to_string(chuva) + ", MEMORIAL_SHOP=" + std::to_string(memorial_shop);
		};
	};

	struct ServerInfoEx : public ServerInfo {
		void clear() {
			memset(this, 0, sizeof(ServerInfoEx));
			uid = -1;
		};
		__int8 tipo;
		char version[40];
		char version_client[40];
		RateConfigInfo rate;
		uFlag flag;
	};

	struct IDStateBlockFlag {
		IDStateBlockFlag(uint64_t _id_state) : id_state{ _id_state }, block_time(-1) {
		};
		IDStateBlockFlag(uint64_t _id_state, int _block_time)
			: id_state{ _id_state }, block_time(_block_time) {
		};
		void clear() {

			memset(this, 0, sizeof(IDStateBlockFlag));

			block_time = - 1;
		};
		union _uIDState {
			_uIDState(uint64_t _ull = 0u) : ull_IDState(_ull) {};
			void clear() {
				ull_IDState = 0u;
			};
			struct _stIDState{
				unsigned char L_BLOCK_TEMPORARY : 1;
				unsigned char L_BLOCK_FOREVER : 1;
				unsigned char L_BLOCK_LOUNGE : 1;
				unsigned char L_BLOCK_SHOP_LOUNGE : 1;
				unsigned char L_BLOCK_GIFT_SHOP : 1;
				unsigned char L_BLOCK_PAPEL_SHOP : 1;
				unsigned char L_BLOCK_SCRATCHY : 1;
				unsigned char L_BLOCK_TICKER : 1;
				unsigned char L_BLOCK_MEMORIAL_SHOP : 1;
				unsigned char L_BLOCK_ALL_IP : 1;
				unsigned char L_BLOCK_MAC_ADDRESS : 1;
			};
			uint64_t ull_IDState;
			_stIDState st_IDState;
		};

	public:
		_uIDState id_state;
		int block_time;
	};

	struct BlockFlag {
		BlockFlag(uint64_t _id_state = 0u) : m_id_state{ _id_state }, m_flag{ 0u } {
		};
		void clear() {
			memset(this, 0, sizeof(BlockFlag));
		};
		void setIDState(uint64_t _id_state) {
			m_id_state = { _id_state };

			if ((m_id_state.id_state.st_IDState.L_BLOCK_LOUNGE ))
				m_flag.stBit.lounge = 1u;
			if ((m_id_state.id_state.st_IDState.L_BLOCK_SHOP_LOUNGE ))
				m_flag.stBit.personal_shop = 1u;
			if ((m_id_state.id_state.st_IDState.L_BLOCK_GIFT_SHOP ))
				m_flag.stBit.gift_shop = 1u;
			if ((m_id_state.id_state.st_IDState.L_BLOCK_PAPEL_SHOP ))
				m_flag.stBit.papel_shop = 1u;
			if ((m_id_state.id_state.st_IDState.L_BLOCK_SCRATCHY ))
				m_flag.stBit.scratchy = 1u;
			if ((m_id_state.id_state.st_IDState.L_BLOCK_TICKER ))
				m_flag.stBit.ticker = 1u;
			if ((m_id_state.id_state.st_IDState.L_BLOCK_MEMORIAL_SHOP ))
				m_flag.stBit.memorial_shop = 1u;
		};

		IDStateBlockFlag m_id_state;
		uFlag m_flag;
	};

	struct chat_macro_user {
		chat_macro_user (unsigned int  _ul = 0u) {
			clear();
		};
		void clear() { memset(this, 0, sizeof(chat_macro_user)); };
		char macro[9][64];
	};

	struct KeysOfLogin {
		void clear() {
			memset(this, 0, sizeof(KeysOfLogin));
		};
		unsigned char valid;
		char keys[2][10];
	};

	struct AuthKeyInfo {
		void clear() { memset(this, 0, sizeof(AuthKeyInfo)); };
		unsigned char valid;
		char key[10];
	};

	typedef AuthKeyInfo AuthKeyLoginInfo;

	struct AuthKeyGameInfo : public AuthKeyInfo {
		AuthKeyGameInfo() : AuthKeyInfo() {
			clear();
		};
		void clear() { memset(this, 0, sizeof(AuthKeyGameInfo)); };
		unsigned int  server_uid;
	};

	struct CharacterInfo {
		enum Stats : unsigned {
			S_POWER,
			S_CONTROL,
			S_ACCURACY,
			S_SPIN,
			S_CURVE,
		};
		void clear() {
			memset(this, 0, sizeof(CharacterInfo));
		};
		int _typeid;
		int id;
		unsigned char default_hair;
		unsigned char default_shirts;
		unsigned char gift_flag;
		unsigned char purchase;
		int parts_typeid[24];
		int parts_id[24];
		char cblank1[216];
		int auxparts[5];
		int cut_in[4];
		unsigned char pcl[5];
		int mastery;
		int card_character[4];
		int card_caddie[4];
		int card_NPC[4];
		char getSlotOfStatsFromCharEquipedPartItem(Stats __stat) {

			char value = 0;
			IFF::Part *part = nullptr;

			if (__stat > Stats::S_CURVE)
				return -1;

			for (auto i = 0u; i < (sizeof(parts_typeid) / sizeof(parts_typeid[0])); ++i) {

				if (parts_id[i] != 0 && (part = sIff::getInstance().findPart(parts_typeid[i])) != nullptr)
					value += part->slot[__stat];
			}

			return value;
		};
		char getSlotOfStatsFromCharEquipedAuxPart(Stats __stat) {

			char value = 0;
			IFF::AuxPart *aux_part = nullptr;

			if (__stat > Stats::S_CURVE)
				return -1;

			for (auto i = 0u; i < (sizeof(auxparts) / sizeof(auxparts[0])); ++i) {

				if (auxparts[i] != 0 && (aux_part = sIff::getInstance().findAuxPart(auxparts[i])) != nullptr)
					value += aux_part->slot[__stat];
			}

			return value;
		};
		char getSlotOfStatsFromSetEffectTable(Stats __stat) {

			char value = 0;
			int ret = 0;

			IFF::SetEffectTable *iff_SET = nullptr;

			std::vector< unsigned int  > check_id;

			if (__stat > Stats::S_CURVE)
				return -1;

			for (auto i = 0u; i < (sizeof(parts_typeid) / sizeof(parts_typeid[0])); ++i) {

				if (parts_typeid[i] != 0l) {

					iff_SET = sIff::getInstance().findFirstItemInSetEffectTable(parts_typeid[i]);

					if (iff_SET != nullptr) {

						if (check_id.empty() || std::find(check_id.begin(), check_id.end(), iff_SET->id) == check_id.end()) {

							check_id.push_back(iff_SET->id);

							ret = 1;

							for (auto j = 0u; j < (sizeof(iff_SET->item._typeid) / sizeof(iff_SET->item._typeid[0])); ++j) {

								if (iff_SET->item._typeid[j] != 0u) {

									if (sIff::getInstance().getItemGroupIdentify(iff_SET->item._typeid[j]) == iff::PART) {

										if (!isPartEquiped(iff_SET->item._typeid[j])) {

											ret = 0;

											break;
										}

									}else if (sIff::getInstance().getItemGroupIdentify(iff_SET->item._typeid[j]) == iff::AUX_PART) {

										if (!isAuxPartEquiped(iff_SET->item._typeid[j])) {

											ret = 0;

											break;
										}

									}
								}
							}

							if (!ret)
								continue;

							std::for_each(iff_SET->effect.effect, LAST_ELEMENT_IN_ARRAY(iff_SET->effect.effect), [&](auto& _el) {

								if (_el == IFF::SetEffectTable::eEFFECT::ONE_ALL_STATS)
									value++;
							});

							value += iff_SET->slot[__stat];
						}
					}
				}
			}

			for (auto i = 0u; i < (sizeof(auxparts) / sizeof(auxparts[0])); ++i) {

				if (auxparts[i] != 0l) {

					iff_SET = sIff::getInstance().findFirstItemInSetEffectTable(auxparts[i]);

					if (iff_SET != nullptr) {

						if (check_id.empty() || std::find(check_id.begin(), check_id.end(), iff_SET->id) == check_id.end()) {

							check_id.push_back(iff_SET->id);

							ret = 1;

							for (auto j = 0u; j < (sizeof(iff_SET->item._typeid) / sizeof(iff_SET->item._typeid[0])); ++j) {

								if (iff_SET->item._typeid[j] != 0u) {

									if (sIff::getInstance().getItemGroupIdentify(iff_SET->item._typeid[j]) == iff::PART) {

										if (!isPartEquiped(iff_SET->item._typeid[j])) {

											ret = 0;

											break;
										}

									}else if (sIff::getInstance().getItemGroupIdentify(iff_SET->item._typeid[j]) == iff::AUX_PART) {

										if (!isAuxPartEquiped(iff_SET->item._typeid[j])) {

											ret = 0;

											break;
										}

									}
								}
							}

							if (!ret)
								continue;

							std::for_each(iff_SET->effect.effect, LAST_ELEMENT_IN_ARRAY(iff_SET->effect.effect), [&](auto& _el) {

								if (_el == IFF::SetEffectTable::eEFFECT::ONE_ALL_STATS)
									value++;
							});

							value += iff_SET->slot[__stat];
						}
					}
				}
			}

			return value;
		};
		char getSlotOfStatsFromCharEquipedCard(Stats __stat) {

			char value = 0;
			IFF::Card *card = nullptr;

			if (__stat > Stats::S_CURVE)
				return -1;

			for (auto i = 0u; i < (sizeof(card_character) / sizeof(card_character[0])); ++i) {

				if (card_character[i] != 0 && (card = sIff::getInstance().findCard(card_character[i])) != nullptr)
					value += card->c[__stat];
			}

			return value;
		};
		unsigned char AngelEquiped() {

			auto angel = std::find_if(angel_wings, LAST_ELEMENT_IN_ARRAY(angel_wings), [&](auto& el) {
				return (sIff::getInstance().getItemCharIdentify(el) == (_typeid & 0x000000FF));
			});

			auto gacha_angel = std::find_if(gacha_angel_wings, LAST_ELEMENT_IN_ARRAY(gacha_angel_wings), [&](auto& el) {
				return (sIff::getInstance().getItemCharIdentify(el) == (_typeid & 0x000000FF));
			});

			unsigned int  part_num = 0u;

			if (angel != LAST_ELEMENT_IN_ARRAY(angel_wings) && (part_num = sIff::getInstance().getItemCharPartNumber(*angel)) >= 0u
					&& parts_typeid[part_num] == *angel)
				return 1u;
			else if (gacha_angel != LAST_ELEMENT_IN_ARRAY(gacha_angel_wings) && (part_num = sIff::getInstance().getItemCharPartNumber(*gacha_angel)) >= 0u
					&& parts_typeid[part_num] == *gacha_angel)
				return 2u;

			return 0u;
		};
		bool isEquipedPartSlotThirdCaddieCardSlot() {

			IFF::Part *part = nullptr;

			for (auto i = 0u; i < (sizeof(parts_typeid) / sizeof(parts_typeid[0])); ++i) {
				if (parts_id[i] != 0 && (part = sIff::getInstance().findPart(parts_typeid[i])) != nullptr)
					if (part->flag_caddie_card_slot)
						return true;
			}

			return false;
		};
		bool isPartEquiped(unsigned int  _part_typeid, int _id) {

			if (_part_typeid == 0)
				return false;

			if (sIff::getInstance().getItemCharIdentify(_part_typeid) != (_typeid & 0x000000FF))
				return false;

			auto part_num = sIff::getInstance().getItemCharPartNumber(_part_typeid);

			if (parts_typeid[part_num] != _part_typeid || parts_id[part_num] != _id)
				return  false;

			return true;
		};
		bool isPartEquiped(unsigned int  _part_typeid) {

			if (_part_typeid == 0)
				return false;

			if (sIff::getInstance().getItemCharIdentify(_part_typeid) != (_typeid & 0x000000FF))
				return false;

			auto part_num = sIff::getInstance().getItemCharPartNumber(_part_typeid);

			if (parts_typeid[part_num] != _part_typeid)
				return false;

			return true;
		};
		bool isAuxPartEquiped(unsigned int  _auxPart_typeid) {

			if (_auxPart_typeid == 0)
				return false;

			for (auto i = 0u; i < (sizeof(auxparts) / sizeof(auxparts[0])); ++i)
				if (auxparts[i] == _auxPart_typeid)
					return true;

			return false;
		};
		void unequipPart(IFF::Part* _part) {

			if (_part == nullptr) {

				_smp::message_pool::getInstance().push(new message("[CharacterInfo::unequipPart][Error] IFF::Part* _part is invalid(nullptr).", CL_FILE_LOG_AND_CONSOLE));

				return;
			}

			for (auto i = 0u; i < (sizeof(parts_typeid) / sizeof(parts_typeid[0])); ++i) {

				if (_part->position_mask.getSlot(i) == 1) {

					unsigned int  def_part = ((i | (unsigned int )(_typeid << 5)) << 13) | 0x8000400;

					parts_typeid[i] = (sIff::getInstance().findPart(def_part) != nullptr) ? def_part : 0;
					parts_id[i] = 0;
				}
			}
		};
		void unequipPart(unsigned int  _typeid) {

			if (_typeid == 0u)
				return;

			auto part = sIff::getInstance().findPart(_typeid);

			if (part != nullptr)
				unequipPart(part);
			else {

				_smp::message_pool::getInstance().push(new message("[CharacterInfo::unequipPart][Error][WARNIG] Part[TYPEID=" + std::to_string(_typeid)
						+ "], mas ele nao existe no IFF_STRUCT do server, desequipa sem usar a funcao do character. Hacker ou Bug.", CL_FILE_LOG_AND_CONSOLE));

				for (auto i = 0u; i < (sizeof(parts_typeid) / sizeof(parts_typeid[0])); ++i) {

					if (parts_typeid[i] == _typeid) {

						unsigned int  def_part = ((i | (unsigned int )(_typeid << 5)) << 13) | 0x8000400;

						parts_typeid[i] = (sIff::getInstance().findPart(def_part) != nullptr) ? def_part : 0;
						parts_id[i] = 0;

						break;
					}
				}
			}

		};
		void unequipAuxPart(unsigned int  _typeid) {

			if (_typeid == 0u)
				return;

			for (auto i = 0u; i < (sizeof(auxparts) / sizeof(auxparts[0])); ++i) {

				if (auxparts[i] == _typeid) {

					auxparts[i] = 0l;

					break;
				}
			}
		};
		void initComboDef() {

			if (_typeid == 0)
				return;

			memset(parts_typeid, 0, sizeof(parts_typeid));
			memset(parts_id, 0, sizeof(parts_id));

			unsigned int  part_typeid = 0u;

			for (auto i = 0u; i < (sizeof(parts_typeid) / sizeof(parts_typeid[0])); ++i) {
				part_typeid = (((_typeid << 5 ) | i) << 13 ) | 0x8000400;

				if (sIff::getInstance().findPart(part_typeid) != nullptr)
					parts_typeid[i] = part_typeid;
			}
		};
	};

	struct IPBan {
		enum _TYPE : unsigned char {
			IP_BLOCK_NORMAL,
			IP_BLOCK_RANGE,
		};
		IPBan(unsigned int  _ul = 0u) {
			clear();
		}
		void clear() { memset(this, 0, sizeof(IPBan)); };
		_TYPE type;
		unsigned int  ip;
		unsigned int  mask;
	};

	struct AuthServerKey {
		AuthServerKey(unsigned int  _ul = 0u) {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(AuthServerKey));
		};
		bool isValid() {
			return (valid == 1 && key[0] != '\0');
		};
		bool checkKey(std::string& _str) {
			return (isValid() && _str.compare(key) == 0);
		};
		unsigned int  server_uid;
		char key[17];
		unsigned char valid : 1;
	};

	struct AuthServerPlayerInfo {
		AuthServerPlayerInfo(unsigned int  _uid = 0u) : uid(_uid), id(""), ip(""), option(-1) {};
		AuthServerPlayerInfo(unsigned int  _uid, std::string _id, std::string _ip)
			: uid(_uid), id(_id), ip(_ip), option(1) {
		};
		void clear() {

			uid = 0u;
			option = -1;

			if (!id.empty()) {
				id.clear();
				id.shrink_to_fit();
			}

			if (!ip.empty()) {
				ip.clear();
				ip.shrink_to_fit();
			}
		};
		unsigned int  uid;
		std::string id;
		std::string ip;
		int option;
	};

	struct CommandOtherServerHeader {
		CommandOtherServerHeader(unsigned int  _ul = 0u) {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(CommandOtherServerHeader));
		};
		unsigned int  send_server_uid_or_type;
		unsigned short command_id;
	};

	struct CommandOtherServerHeaderEx : public CommandOtherServerHeader {
		public:
			struct stCommand {
				stCommand(unsigned short _size = 0u)
					: buff(nullptr), size(0u), state(false) {

					init(_size);
				};
				~stCommand() {

					destroy();
				};
				void destroy() {

					if (buff != nullptr)
						delete[] buff;

					buff = nullptr;

					state = false;
				};
				void init(unsigned short _size) {

					if (_size > 0u) {

						size = _size;

						if (buff != nullptr)
							destroy();

						buff = new unsigned char[size];

						state = true;
					}

				};
				bool is_good() {
					return state;
				};
				unsigned char *buff;
				unsigned short size;
			protected:
				bool state;
			};

		public:
			CommandOtherServerHeaderEx(unsigned int  _ul = 0u)
				: CommandOtherServerHeader(_ul), command{ 0 } {

				clear();
			};
			~CommandOtherServerHeaderEx() {
				clear();
			};
			void clear() {

				CommandOtherServerHeader::clear();

				command.destroy();
			};

		public:
			stCommand command;
	};

#if defined(__linux__)
#pragma pack()
#endif
}

#endif
