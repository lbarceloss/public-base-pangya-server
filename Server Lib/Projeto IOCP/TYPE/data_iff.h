
#if defined(_WIN32)
#pragma pack(1)
#endif

#pragma once
#ifndef _STDA_IFF_DATA_IFF_H
#define _STDA_IFF_DATA_IFF_H

#include <memory.h>
#include <string>
#include "../UTIL/hex_util.h"
#include "../UTIL/util_time.h"
#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include "../UTIL/WinPort.h"
#endif

#include <algorithm>
#include <numeric>

#define EMPTY_ARRAY_PRICE(_price) (bool)!std::count_if(_price, (_price + (sizeof(_price) / sizeof(_price[0]))), [](auto& el) { return el != 0; })
#define SUM_ARRAY_PRICE_ULONG(_price) (unsigned int)std::accumulate(_price, (_price + (sizeof(_price) / sizeof(_price[0]))), 0u)

#if defined(__linux__)
#pragma pack(1)
#endif

namespace stdA {
	namespace IFF {

		struct Head {
			void clear() {
				memset(this, 0, sizeof(Head));
			};
			unsigned short count_element;
			unsigned short flag_ligacao;
			unsigned int  version;
		};

		struct ShopDados {
			void clear() {
				memset(this, 0, sizeof(ShopDados));
			};
			unsigned int  price;
			unsigned int  sale_price;
			unsigned int  sell_price;
			struct FlagShop {
				struct _8bits {
					unsigned char bit0 : 1;
					unsigned char bit1 : 1;
					unsigned char bit2 : 1;
					unsigned char bit3 : 1;
					unsigned char bit4 : 1;
					unsigned char bit5 : 1;
					unsigned char bit6 : 1;
					unsigned char bit7 : 1;
				};
				union {
					unsigned short us_flag_shop;
					unsigned char uc_bytes[2];
					struct _stFlagShop {
						unsigned char is_cash : 1;
						unsigned char can_send_mail_and_personal_shop : 1;
						unsigned char can_dup : 1;
						unsigned char unknown : 1;
						unsigned char block_mail_and_personal_shop : 1;
						unsigned char is_saleable : 1;
						unsigned char is_giftable : 1;
						unsigned char only_display : 1;
					} stFlagShop;
					struct _stIconShop {
						unsigned short : 8;
						unsigned short is_new : 1;
						unsigned short is_hot : 1;
						unsigned short unknown_bit2 : 1;
						unsigned short unknown_bit3 : 1;
						unsigned short unknown_bit4 : 1;
						unsigned short unknown_bit5 : 1;
						unsigned short unknown_bit6 : 1;
						unsigned short unknown_bit7 : 1;
					} stIconShop;

				} uFlagShop;
				struct TimeShop {

					unsigned char active : 1;
					unsigned char dia;
				} time_shop;
			};
			FlagShop flag_shop;
		};

		struct TikiShopDados {
			void clear() {
				memset(this, 0, sizeof(TikiShopDados));
			};
			bool isActived() {
				return (tipo_tiki_shop == 1u || tipo_tiki_shop == 2u || tipo_tiki_shop == 3u) && tiki_pang > 0u && milage_pts > 0u;
			};
			unsigned int  qnt_per_tikis_pts;
			unsigned int  tiki_pts;
			unsigned short milage_pts;
			unsigned short bonus_prob;
			unsigned short bonus[2];
			unsigned int  tipo_tiki_shop;
			unsigned int  tiki_pang;
		};

		struct DateDados {
			void clear() {
				memset(this, 0, sizeof(DateDados));
			};
			unsigned int  active_date : 1, : 0;
			SYSTEMTIME date[2];
		};

		struct Base {
			void clear() {
				memset(this, 0, sizeof(Base));
			};
			unsigned int  active;
			unsigned int  _typeid;
			char name[64];
			struct stLevel {
				void clear() { memset(this, 0, sizeof(stLevel)); };
				bool goodLevel(unsigned char _level) {
					if (is_max && _level <= level)
						return true;
					else if (!is_max && _level >= level)
						return true;

					return false;
				};
				unsigned char level : 7;
				unsigned char is_max : 1;
			};
			stLevel level;
			char icon[43];
			ShopDados shop;
			TikiShopDados tiki;
			DateDados date;
			std::string toString() {
#ifdef _DEBUG
				return "Typeid: " + std::to_string(_typeid)
					 + "(0x" + hex_util::ltoaToHex(_typeid) + ")"
					 + "\r\nName: " + name + "\r\n";
#else
				return "Typeid: " + std::to_string(_typeid)
					+ "(0x" + hex_util::ltoaToHex(_typeid) + ")\r\n";
#endif
			};
		};

		struct Item : public Base {
			void clear() {
				memset(this, 0, sizeof(Item));
			};
			unsigned int  tipo_item;
			char mpet[40];
			short c[5];
			short point;
		};

		struct Achievement : public Base {
			void clear() {
				memset(this, 0, sizeof(Achievement));
			};
			unsigned int  typeid_quest_index;
			unsigned int  achievement_tipo;
			char quest_name[10][129];
			unsigned short s_unknown;
			unsigned int  quest_typeid[10];
			unsigned int  l_unknown;
		};

		struct QuestStuff : public Base {
			void clear() {
				memset(this, 0, sizeof(QuestStuff));
			};
			struct CounterItem {
				unsigned int  _typeid[5];
				int qntd[5];
			};
			struct RewardItem {
				unsigned int  _typeid[3];
				unsigned int  qntd[3];
				unsigned int  time[3];
			};
			CounterItem counter_item;
			RewardItem reward_item;

		};

		struct QuestItem : public Base {
			void clear() {
				memset(this, 0, sizeof(QuestItem));
			};
			unsigned int  ulUnknown;
			unsigned int  type;
			struct Quest {
				unsigned int  qntd;
				unsigned int  _typeid[10];
			};
			struct Reward {
				unsigned int  _typeid[2];
				unsigned int  qntd[2];
				unsigned int  time[2];
			};
			Quest quest;
			Reward reward;
			unsigned int  ulUnknown2;
		};

		struct CounterItem : Base {
			void clear() {
				memset(this, 0, sizeof(CounterItem));
			};
			unsigned char ucUnknown[88];
		};

		struct Part : public Base {

			enum PART_TYPE : unsigned int  {
				TOP,
				BOTTOM,
				HEAD,
				ARM,
				FOOT,
				ETC,
				HAIR,
				UCC,
				UCC_BLANK,
				UCC_COPY,
			};
			void clear() {
				memset(this, 0, sizeof(Part));
			};
			char mpet[40];
			PART_TYPE type_item;
			union u_part_type {
				unsigned int  ul_part_type;
				struct {
					unsigned char slot0 : 1;
					unsigned char slot1 : 1;
					unsigned char slot2 : 1;
					unsigned char slot3 : 1;
					unsigned char slot4 : 1;
					unsigned char slot5 : 1;
					unsigned char slot6 : 1;
					unsigned char slot7 : 1;
					unsigned char slot8 : 1;
					unsigned char slot9 : 1;
					unsigned char slot10 : 1;
					unsigned char slot11 : 1;
					unsigned char slot12 : 1;
					unsigned char slot13 : 1;
					unsigned char slot14 : 1;
					unsigned char slot15 : 1;
					unsigned char slot16 : 1;
					unsigned char slot17 : 1;
					unsigned char slot18 : 1;
					unsigned char slot19 : 1;
					unsigned char slot20 : 1;
					unsigned char slot21 : 1;
					unsigned char slot22 : 1;
					unsigned char slot23 : 1;
				} _slots;
				char getSlot(unsigned int  _i) {
					switch (_i) {
					case 0:
						return (char)_slots.slot0;
					case 1:
						return (char)_slots.slot1;
					case 2:
						return (char)_slots.slot2;
					case 3:
						return (char)_slots.slot3;
					case 4:
						return (char)_slots.slot4;
					case 5:
						return (char)_slots.slot5;
					case 6:
						return (char)_slots.slot6;
					case 7:
						return (char)_slots.slot7;
					case 8:
						return (char)_slots.slot8;
					case 9:
						return (char)_slots.slot9;
					case 10:
						return (char)_slots.slot10;
					case 11:
						return (char)_slots.slot11;
					case 12:
						return (char)_slots.slot12;
					case 13:
						return (char)_slots.slot13;
					case 14:
						return (char)_slots.slot14;
					case 15:
						return (char)_slots.slot15;
					case 16:
						return (char)_slots.slot16;
					case 17:
						return (char)_slots.slot17;
					case 18:
						return (char)_slots.slot18;
					case 19:
						return (char)_slots.slot19;
					case 20:
						return (char)_slots.slot20;
					case 21:
						return (char)_slots.slot21;
					case 22:
						return (char)_slots.slot22;
					case 23:
						return (char)_slots.slot23;
					default:
						return -1;
					}
				};
			};
			u_part_type position_mask;
			u_part_type hide_mask;
			char texture[3][40];
			char texture_org[3][40];
			unsigned short c[5];
			unsigned short slot[5];
			unsigned char equippable_with[40];
			unsigned int sub_part[2];
			unsigned short character_slot;
			unsigned short flag_caddie_card_slot;
			unsigned short npc_slot;
			unsigned short point;
			unsigned int  valor_rental;
			unsigned int  ul_unknown3;
			std::string toString() {
#ifdef _DEBUG
				return "Typeid: " + std::to_string(_typeid)
					+ "(0x" + hex_util::ltoaToHex(_typeid) + ")"
					+ "\r\nType: " + std::to_string(type_item)
					+ "\r\nPosition Slot	1	2	3	4	5	6	7	8	9	10	11	12"
					+ "\r\n		"	+ std::to_string((unsigned short)position_mask._slots.slot0) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot1) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot2) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot3) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot4) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot5) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot6) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot7) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot8) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot9) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot10) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot11) + "	"
					+ "\r\nPosition Slot	13	14	15	16	17	18	16	20	21	22	23	24"
					+ "\r\n		"	+ std::to_string((unsigned short)position_mask._slots.slot12) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot13) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot14) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot15) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot16) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot17) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot18) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot19) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot20) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot21) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot22) + "	"
								+ std::to_string((unsigned short)position_mask._slots.slot23) + "	"
					+ "\r\nHide Slot	1	2	3	4	5	6	7	8	9	10	11	12"
					+ "\r\n		"	+ std::to_string((unsigned short)hide_mask._slots.slot0) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot1) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot2) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot3) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot4) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot5) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot6) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot7) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot8) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot9) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot10) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot11) + "	"
					+ "\r\nHide Slot	13	14	15	16	17	18	16	20	21	22	23	24"
					+ "\r\n		"	+ std::to_string((unsigned short)hide_mask._slots.slot12) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot13) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot14) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot15) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot16) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot17) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot18) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot19) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot20) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot21) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot22) + "	"
								+ std::to_string((unsigned short)hide_mask._slots.slot23) + "	"
					+ "\r\nC:	" + std::to_string(c[0]) + "	" + std::to_string(c[1]) + "	"
							+ std::to_string(c[2]) + "	" + std::to_string(c[3]) + "	" + std::to_string(c[4])
					+ "\r\nSlot:	" + std::to_string(slot[0]) + "	" + std::to_string(slot[1]) + "	"
							+ std::to_string(slot[2]) + "	" + std::to_string(slot[3]) + "	" + std::to_string(slot[4])
					+ "\r\nSell Shop: " + std::to_string(shop.sell_price)
					+ "\r\nFlag Shop	1	2	3	4	5	6	7	8	9	10	11	12	13	14	15	16"
					+ "\r\n			" + std::to_string((unsigned short)shop.flag_shop.uFlagShop.stFlagShop.is_cash) + "	"
										+ std::to_string((unsigned short)shop.flag_shop.uFlagShop.stFlagShop.can_send_mail_and_personal_shop) + "	"
										+ std::to_string((unsigned short)shop.flag_shop.uFlagShop.stFlagShop.can_dup) + "	"
										+ std::to_string((unsigned short)shop.flag_shop.uFlagShop.stFlagShop.unknown) + "	"
										+ std::to_string((unsigned short)shop.flag_shop.uFlagShop.stFlagShop.block_mail_and_personal_shop) + "	"
										+ std::to_string((unsigned short)shop.flag_shop.uFlagShop.stFlagShop.is_saleable) + "	"
										+ std::to_string((unsigned short)shop.flag_shop.uFlagShop.stFlagShop.is_giftable) + "	"
										+ std::to_string((unsigned short)shop.flag_shop.uFlagShop.stFlagShop.only_display) + "	"
										+ std::to_string((unsigned short)shop.flag_shop.uFlagShop.stIconShop.is_new) + "	"
										+ std::to_string((unsigned short)shop.flag_shop.uFlagShop.stIconShop.is_hot) + "	"
										+ std::to_string((unsigned short)shop.flag_shop.uFlagShop.stIconShop.unknown_bit2) + "	"
										+ std::to_string((unsigned short)shop.flag_shop.uFlagShop.stIconShop.unknown_bit3) + "	"
										+ std::to_string((unsigned short)shop.flag_shop.uFlagShop.stIconShop.unknown_bit4) + "	"
										+ std::to_string((unsigned short)shop.flag_shop.uFlagShop.stIconShop.unknown_bit5) + "	"
										+ std::to_string((unsigned short)shop.flag_shop.uFlagShop.stIconShop.unknown_bit6) + "	"
										+ std::to_string((unsigned short)shop.flag_shop.uFlagShop.stIconShop.unknown_bit7)
					+ "\r\n";
#else
				return "Typeid: " + std::to_string(_typeid) + "(0x" + hex_util::ltoaToHex(_typeid) + ")\r\n";
#endif
			};
		};

		struct SetItem : public Base {
			enum SUB_TYPE : unsigned char {
				COMMON,
				CHARACTER,
				PARTS,
				CLUBSET = 4,
				BALL,
				ITEM,
				CADDIE,
				CARD,
				AUXPART,
			};

			enum SUB_TYPE_CHAR : unsigned char {
				NURI,
				HANA,
				AZER,
				CECILIA,
				MAX,
				KOOH,
				ARIN,
				KAZ,
				LUCIA,
				NELL,
				SPIKA,
				NURI_R,
				HANA_R,
				AZER_R,
				CECILIA_R,
				STC_CARD = 0xFD,
				EQUIP_ITEM,
				NOEQUIP_ITEM,
			};
			void clear() { memset(this, 0, sizeof(SetItem)); };
			struct Packege {
				unsigned int  qntd;
				unsigned int  item_typeid[10];
				unsigned short item_qntd[10];
			};
			Packege packege;
			unsigned short c[5];
			unsigned short point;
		};

		struct Mascot : public Base {
			void clear() { memset(this, 0, sizeof(Mascot)); };
			char mpet[40];
			char textura[40];
			unsigned char price[5];
			unsigned char c[5];
			struct Efeito {
				short power_drive;
				short drop_rate;
				short power_gague;
				short pang_rate;
				short exp_rate;
				unsigned char item_slot;
			};
			struct Mensagem {
				unsigned char active;
				short flag;
				unsigned int  change_price;
			};
			struct BonusPangya {
				unsigned short pang;
				unsigned short flag;
			};
			Efeito efeito;
			Mensagem msg;
			BonusPangya bonus_pangya;
		};

		struct AuxPart : public Base {
			void clear() { memset(this, 0, sizeof(AuxPart)); };
			unsigned short cc[5];
			unsigned char c[5];
			unsigned char slot[5];
			struct stEfeito {
				unsigned short power_drive;
				unsigned short drop_rate;
				unsigned short power_gauge;
				unsigned short pang_rate;
				unsigned short exp_rate;
				unsigned short unknown;
				std::string toString() {
					return "Efeito [\n\tPOWER_DRIVE: " + std::to_string(power_drive)
						+ ";\n\tDROP_RATE: " + std::to_string(drop_rate)
						+ ";\n\tPOWER_GAUGE: " + std::to_string(power_gauge)
						+ ";\n\tPANG_RATE: " + std::to_string(pang_rate)
						+ ";\n\tEXP_RATE: " + std::to_string(exp_rate)
						+ ";\n\tUNKNOWN: " + std::to_string(unknown)
						+".\n\t]";
				};
			};
			stEfeito efeito;

			unsigned int  ulUnknown2;
			std::string toString() {
#ifdef _DEBUG
				return "Typeid: " + std::to_string(_typeid)
					+ "(0x" + hex_util::ltoaToHex(_typeid) + ")"
					+ "\r\nCC[]: [CC0=" + std::to_string(cc[0]) + ", CC1=" + std::to_string(cc[1]) + ", CC2=" + std::to_string(cc[2])
							+ ", CC3=" + std::to_string(cc[3]) + ", CC4=" + std::to_string(cc[4]) + "]"
					+ "\r\nC[]: [C0=" + std::to_string((unsigned short)c[0]) + ", C1=" + std::to_string((unsigned short)c[1]) + ", C2="
							+ std::to_string((unsigned short)c[2]) + ", C3=" + std::to_string((unsigned short)c[3]) + ", C4=" + std::to_string((unsigned short)c[4]) + "]"
					+ "\r\nSlot[]: [Slot0=" + std::to_string((unsigned short)slot[0]) + ", Slot1=" + std::to_string((unsigned short)slot[1]) + ", Slot2="
							+ std::to_string((unsigned short)slot[2]) + ", Slot3=" + std::to_string((unsigned short)slot[3]) + ", Slot4=" + std::to_string((unsigned short)slot[4]) + "]"
					+ "\r\n" + efeito.toString()
					+ "\r\nulUnknown2: " + std::to_string(ulUnknown2) + "\r\n";
#else
				return "Typeid: " + std::to_string(_typeid)
					+ "(0x" + hex_util::ltoaToHex(_typeid) + ")";
#endif
			};
		};

		struct Ball : public Base {
			void clear() { memset(this, 0, sizeof(Ball)); };
			unsigned int  ulUnknown;
			char mpet[40];
			unsigned int  bound;
			unsigned int  roll;
			char seq[7][40];
			char fx[7][40];
			unsigned short c[5];
			unsigned short point;
			std::string toString() {
#ifdef _DEBUG
				return "Typeid: " + std::to_string(_typeid)
					+ "(0x" + hex_util::ltoaToHex(_typeid) + ")"
					+ "\r\nC[]: [C0=" + std::to_string((unsigned short)c[0]) + ", C1=" + std::to_string((unsigned short)c[1]) + ", C2="
							+ std::to_string((unsigned short)c[2]) + ", C3=" + std::to_string((unsigned short)c[3]) + ", C4=" + std::to_string((unsigned short)c[4]) + "]"
					+ "\r\nPoint: " + std::to_string(point) + "\r\n";
#else
				return "Typeid: " + std::to_string(_typeid)
					+ "(0x" + hex_util::ltoaToHex(_typeid) + ")";
#endif
			};
		};

		struct Caddie : public Base {
			void clear() { memset(this, 0, sizeof(Caddie)); };
			unsigned int  valor_mensal;
			char mpet[40];
			unsigned short c[5];
			unsigned short point;
		};

		struct CaddieItem : public Base {
			enum Type : unsigned char {
				COOKIE,
				PANG,
				ESPECIAL,
				UPGRADE
			};
			void clear() { memset(this, 0, sizeof(CaddieItem)); };
			char mpet[40];
			char textura[40];
			unsigned short price[5];
			unsigned short unit_power_guage_start;
		};

		struct CadieMagicBox {
			void clear() { memset(this, 0, sizeof(CadieMagicBox)); };
			unsigned int  seq;
			unsigned int  active;
			unsigned int  setor;
			unsigned int  character;
			unsigned int  level;
			unsigned int  ulUnknown;
			struct ItemReceive {
				unsigned int  _typeid;
				unsigned int  qntd;
			};
			struct ItemTrade {
				unsigned int  _typeid[4];
				unsigned int  qntd[4];
			};
			ItemReceive item_receive;
			ItemTrade item_trade;
			unsigned int  box_random_id;
			char name[40];
			SYSTEMTIME date[2];
		};

		struct CadieMagicBoxRandom {
			void clear() { memset(this, 0, sizeof(CadieMagicBoxRandom)); };
			unsigned int  id;
			struct ItemRandom {
				unsigned int  _typeid;
				unsigned int  qntd;
				unsigned int  rate;
			};
			ItemRandom item_random;
		};

		struct Card : public Base {
			void clear() { memset(this, 0, sizeof(Card)); };
			enum CARD_SUB_TYPE : unsigned {
				T_CHARACTER,
				T_CADDIE,
				T_SPECIAL,
				T_PACK,
				T_BOX_PACK,
				T_NPC,
			};
			unsigned char tipo;

			char mpet[41];
			unsigned short c[5];
			struct Efeito {
				unsigned short type;
				unsigned short qntd;
			};
			Efeito efeito;
			char textura[3][40];
			unsigned short tempo;
			unsigned short volume;
			unsigned int  position;
			unsigned int  flag1;
			unsigned int  flag2;
			std::string toString() {
				return "Typeid: " + std::to_string(_typeid)
					+ "(0x" + hex_util::ltoaToHex(_typeid) + ")"
					+ "\r\nTipo: " + std::to_string((unsigned short)tipo)
					+ "\r\nEfeito[Type=" + std::to_string(efeito.type) + ", Qntd=" + std::to_string(efeito.qntd) + "]"
					+ "\r\nTempo: " + std::to_string(tempo)
					+ "\r\nVolume(Pack): " + std::to_string(volume)
					+ "\r\nPosistion(Book): " + std::to_string(position)
					+ "\r\nC[C0=" + std::to_string(c[0]) + ", C1=" + std::to_string(c[1]) + ", C2=" + std::to_string(c[2]) + ", C3=" + std::to_string(c[3]) + ", C4=" + std::to_string(c[4]) + "]"
					+ "\r\nFlag[F1=" + std::to_string(flag1) + ", F2=" + std::to_string(flag2) + "]\r\n";
			};
		};

		struct Character : public Base {
			void clear() { memset(this, 0, sizeof(Character)); };
			char mpet[40];
			char textura[3][40];
			unsigned short c[5];
			unsigned char num_parts;
			unsigned char num_accessorios;
			unsigned int  club_type;
			float scale_club_set;
			unsigned char c_stat[5];
			char camera[43];
		};

		struct CharacterMastery {
			void clear() { memset(this, 0, sizeof(CharacterMastery)); };
			unsigned int  active;
			unsigned int  _typeid;
			unsigned int  seq;
			unsigned int  stats;
			unsigned int  level;
			struct Condition {
				unsigned int  condition[5];
				unsigned int  qntd[5];
			};
			Condition condition;
		};

		struct Club : public Base {
			void clear() { memset(this, 0, sizeof(Club)); };
			char mpet[40];
			unsigned short tipo;
			unsigned short c[5];
		};

		struct ClubSet : public Base {
			void clear() { memset(this, 0, sizeof(ClubSet)); };
			unsigned int  club[4];
			unsigned short c[5];
			unsigned short slot[5];
			struct WorkShop {
				int tipo;
				unsigned int  rank_s_stat;
				unsigned int  total_recovery;
				float rate;
				unsigned int  tipo_rank_s;
				unsigned int  flag_transformar;

			};
			WorkShop work_shop;
			unsigned int  ulUnknown;
			unsigned int  text_pangya;
		};

		struct ClubSetWorkShopLevelUpLimit {
			void clear() { memset(this, 0, sizeof(ClubSetWorkShopLevelUpLimit)); };
			unsigned int  tipo;
			unsigned int  rank;
			unsigned short c[5];
			unsigned short option;
		};

		struct ClubSetWorkShopLevelUpProb {
			void clear() { memset(this, 0, sizeof(ClubSetWorkShopLevelUpProb)); };
			unsigned int  tipo;
			unsigned int  c[5];
		};

		struct ClubSetWorkShopRankUpExp {
			void clear() { memset(this, 0, sizeof(ClubSetWorkShopRankUpExp)); };
			unsigned int  tipo;
			unsigned int  rank[6];
		};

		struct Course : public Base {
			void clear() { memset(this, 0, sizeof(Course)); };
			char mpet[40];
			char gbin[40];
			struct Star {
				union {
					unsigned char ucStar;
					struct StarMask {
						unsigned char star_num : 4;
						unsigned char star_size : 4;
					};
					StarMask star_mask;
				} uStarMask;
			};
			Star star;
			char xml[43];
			float rate_pang;
			char seq[40];
			unsigned int  ulUnknown[12];
			struct ParScore {
				unsigned char par_hole[18];
				unsigned char min_score_hole[18];
				unsigned char max_score_hole[18];
			};
			ParScore par_score_hole;
			unsigned short usUnknown;
		};

		struct CutinInfomation {

			union uCondition {
				uCondition(uint32_t _ul = 0u) : ulCondition(_ul) {};
				void clear() { ulCondition = 0u; };
				uint32_t ulCondition;
				struct {
					uint32_t power_shot : 1;
					uint32_t double_power_short : 1;
					uint32_t power_shot_failed : 1;
					uint32_t chipin : 1, : 0;
				}stBit;
			};
			void clear() { memset(this, 0, sizeof(CutinInfomation)); };
			unsigned int  active;
			unsigned int  _typeid;
			unsigned int  rare_typeid;
			unsigned int  rarity;
			uCondition	  tipo;
			unsigned int  sector;
			unsigned int  character_id;
			struct Img {
				char sprite[40];
				unsigned int  tipo;
			};
			Img img[4];
			unsigned int  tempo;
		};

		struct Enchant {
			void clear() { memset(this, 0, sizeof(Enchant)); };
			unsigned int  active;
			unsigned int  _typeid;
			uint64_t pang;
		};

		struct Furniture : public Base {
			void clear() { memset(this, 0, sizeof(Furniture)); };
			char mpet[40];
			unsigned short num;
			unsigned short is_own;
			unsigned short is_move;
			unsigned short is_function;
			unsigned int  etc;
			struct Location {
				float x;
				float y;
				float z;
				float r;
			};
			Location location;
			char textura[3][40];
			char textura_org[3][40];
			unsigned short c[5];
			unsigned short use_time;
		};

		struct HairStyle : public Base {
			void clear() { memset(this, 0, sizeof(HairStyle)); };
			unsigned char cor;
			unsigned char character;
			unsigned short usUnknown;
		};

		struct Match {
			void clear() { memset(this, 0, sizeof(Match)); };
			unsigned int  active;
			unsigned int  _typeid;
			char name[80];
			unsigned char level;
			char trophy[6][40];
			unsigned char ucUnknown2[3];
		};

		struct Skin : public Base {
			void clear() { memset(this, 0, sizeof(Skin)); };
			char mpet[40];
			unsigned char horizontal_scroll;
			unsigned char vertical_scroll;
			unsigned short price[5];
		};

		struct Ability {
			enum class eEFFECT_TYPE : unsigned int  {
				NONE,
				PIXEL,
				PIXEL_BY_WIND_NO_ITEM,
				PIXEL_OVER_WIND_NO_ITEM,
				PIXEL_BY_WIND,
				PIXEL_2,
				PIXEL_WITH_WEAK_WIND,
				POWER_GAUGE_TO_START_HOLE,
				POWER_GAUGE_MORE_ONE,
				POWER_GUAGE_TO_START_GAME,
				PAWS_NOT_ACCUMULATE,
				SWITCH_TWO_EFFECT,
				EARCUFF_DIRECTION_WIND,
				COMBINE_ITEM_EFFECT,
				SAFETY_CLIENT_RANDOM,
				PIXEL_RANDOM,
				WIND_1M_RANDOM,
				PIXEL_BY_WIND_MIDDLE_DOUBLE,
				GROUND_100_PERCENT_RONDOM,
				ASSIST_MIRACLE_SIGN,
				VECTOR_SIGN,
				ASSIST_TRAJECTORY_SHOT,
				PAWS_ACCUMULATE,
				POWER_GAUGE_FREE,
				SAFETY_RANDOM,
				ONE_IN_ALL_STATS,
				POWER_GAUGE_BY_MISS_SHOT,
				PIXEL_BY_WIND_2,
				PIXEL_WITH_RAIN,
				NO_RAIN_EFFECT,
				PUTT_MORE_10Y_RANDOM,
				UNKNOWN_31,
				MIRACLE_SIGN_RANDOM,
				UNKNOWN_33,
				DECREASE_1M_OF_WIND,
			};
			void clear() { memset(this, 0, sizeof(Ability)); };
			unsigned int  _typeid;
			struct Efeito {
				unsigned int  EfeitoOrNo[3];
				unsigned int  type[3];
				float rate[3];
			};
			Efeito efeito;
			unsigned char ucUnknown[32];
			unsigned int  flag1;
			unsigned int  flag2;
			std::string toString() {
#ifdef _DEBUG
				return "Typeid: " + std::to_string(_typeid)
					+ "(0x" + hex_util::ltoaToHex(_typeid) + ")"
					+ "\r\nEfeitoOuNao: " + std::to_string(efeito.EfeitoOrNo[0])
					+ ", " + std::to_string(efeito.EfeitoOrNo[1])
					+ ", " + std::to_string(efeito.EfeitoOrNo[2])
					+ "\r\nEfeito:      " + std::to_string(efeito.type[0])
					+ ", " + std::to_string(efeito.type[1])
					+ ", " + std::to_string(efeito.type[2])
					+ "\r\nRate:        " + std::to_string(efeito.rate[0])
					+ ", " + std::to_string(efeito.rate[1])
					+ ", " + std::to_string(efeito.rate[2])
					+ "\r\nFlag1: " + std::to_string(flag1)
					+ "\tFlag2: " + std::to_string(flag2)
					+ hex_util::BufferToHexString(ucUnknown, sizeof(ucUnknown));
#else
				return "Typeid: " + std::to_string(_typeid)
					+ "(0x" + hex_util::ltoaToHex(_typeid) + ")";
#endif
			};
		};

		struct Desc {
			void clear() { memset(this, 0, sizeof(Desc)); };
			unsigned int  _typeid;
			char description[512];
			std::string toString() {
#ifdef _DEBUG
				return "Typeid: " + std::to_string(_typeid)
					+ "(0x" + hex_util::ltoaToHex(_typeid) + ")"
					+ "\r\nDescription:\r\n" + description + "\r\n";
#else
				return "Typeid: " + std::to_string(_typeid)
					+ "(0x" + hex_util::ltoaToHex(_typeid) + ")";
#endif
			};
		};

		struct GrandPrixAIOptionalData {
			void clear() { memset(this, 0, sizeof(GrandPrixAIOptionalData)); };
			unsigned int  active;
			unsigned int  id;
			char name[36];
			unsigned int  BetterOrNo;
			unsigned int  char_id;
			unsigned int  _class;
			unsigned int  parts_typeid[24];
			unsigned char ucUnknown[452];
			std::string toString() {
#ifdef _DEBUG
				return "Info Bot\r\nName: " + std::string(name)
					+ "\r\nID: " + std::to_string(id)
					+ "\r\nCharacter ID: " + std::to_string(char_id)
					+ "\r\nFirster Class: " + std::to_string(BetterOrNo)
					+ "\r\nClass: " + std::to_string(_class)
					+ hex_util::BufferToHexString(ucUnknown, sizeof(ucUnknown));
#else
				return "Info Bot\r\nTypeid: " + std::to_string(id);
#endif
			};
		};

		struct GrandPrixConditionEquip {
			void clear() { memset(this, 0, sizeof(GrandPrixConditionEquip)); };
			unsigned int  active;
			unsigned int  _typeid;
			unsigned int  item_typeid;
			char info[516];
		};

		struct GrandPrixData {
		public:
			enum GP_ABA : unsigned char {
				ROOKIE,
				BEGINNER,
				JUNIOR,
				SENIOR,
			};

		public:
			void clear() { memset(this, 0, sizeof(GrandPrixData)); };
			unsigned int  active;
			unsigned int  _typeid;
			unsigned int  typeid_link;
			unsigned int  type;
			unsigned short time_hole;
			char name[64];
			unsigned short usUnknown;
			struct Ticket {
				unsigned int  _typeid;
				unsigned int  qntd;
			};
			Ticket ticket;
			char img[41];
			struct Flag {
				unsigned char natural;
				unsigned char short_game;
				unsigned char hole_cup_x2;
			};
			Flag flag;
			unsigned int  rule;
			struct CourseInfo {
				unsigned int  course;
				unsigned int  modo;
				unsigned char qntd_hole;
			};
			CourseInfo course_info;
			unsigned char level_min;
			unsigned char level_max;
			unsigned char ucUnknown;
			unsigned int  condition[2];
			struct BOT {
				int score_max;
				int score_med;
				int score_min;
			};
			BOT bot;
			unsigned int  _class;
			unsigned int  pang;
			struct Reward {
				unsigned int  _typeid[5];
				unsigned int  qntd[5];
				unsigned int  time[5];
			};
			Reward reward;
			SYSTEMTIME open;
			SYSTEMTIME start;
			SYSTEMTIME end;
			unsigned int  ulUnknown;
			unsigned int  clear_gp_typeid;
			unsigned int  lock_yn;
			char info[516];
		};

		struct GrandPrixRankReward {
			void clear() { memset(this, 0, sizeof(GrandPrixRankReward)); };
			unsigned int  active;
			unsigned int  _typeid;
			unsigned int  rank;
			struct Reward {
				unsigned int  _typeid[5];
				unsigned int  qntd[5];
				unsigned int  time[5];
			};
			Reward reward;
			unsigned int  trophy_typeid;
		};

		struct GrandPrixSpecialHole {
			void clear() { memset(this, 0, sizeof(GrandPrixSpecialHole)); };
			unsigned int  active;
			unsigned int  _typeid;
			unsigned int  seq;
			unsigned int  course;
			unsigned int  hole;
		};

		struct MemorialShopCoinItem {
			enum TYPE_FILTER : unsigned int  {
				SPRING = 1,
				SUMMER,
				FALL,
				WINTER,
				CLUBSET,
				SETITEM,
				EAR,
				WING,
				LUVA,
				RING_R,
				RING_L,
				CADDIE,
				MASCOT,
				SUMMER_HOLYDAY,
				XMAS,
				HALLOWEEN,
				MAN,
				WOMAN,
				NURI,
				HANA,
				AZER,
				CECI,
				MAX,
				KOOH,
				ARIN,
				KAZ,
				LUCIA,
				NELL,
				SPIKA,
				NURI_R,
				HANA_R,
				AZER_R,
				CECI_R,
			};
			void clear() { memset(this, 0, sizeof(MemorialShopCoinItem)); };
			unsigned int  active;
			unsigned int  _typeid;
			unsigned int  type;
			unsigned int  probability;
			struct GachaRange {
				unsigned int  number_min;
				unsigned int  number_max;
				bool empty() {
					return (number_min == 0 && number_max == 0);
				};
				bool isBetweenGacha(unsigned int  _number) {
					return (number_min <= _number && _number <= number_max);
				};
			};
			GachaRange gacha_range;

			unsigned int  filter[10];
			bool hasFilter(unsigned int  _filter) {
				if (_filter == 0)
					return false;

				for (auto i = 0u; i < (sizeof(filter) / sizeof(filter[0])); ++i)
					if (filter[i] == _filter)
						return true;

				return false;
			};
			bool emptyFilter() {
				unsigned int  count = 0u;

				for (auto i = 0u; i < (sizeof(filter) / sizeof(filter[0])); ++i)
					count += filter[i];

				return count == 0;
			};
			std::string toString() {
#ifdef _DEBUG
				return "Typeid: " + std::to_string(_typeid) + "(0x" + hex_util::ltoaToHex(_typeid) + ")"
					+ "\r\nType: " + std::to_string(type)
					+ "\r\nProbability: " + std::to_string(probability)
					+ "\r\nFilter(s): " + std::to_string(filter[0])
					+ ", " + std::to_string(filter[1])
					+ ", " + std::to_string(filter[2])
					+ ", " + std::to_string(filter[3])
					+ ", " + std::to_string(filter[4])
					+ ", " + std::to_string(filter[5])
					+ ", " + std::to_string(filter[6])
					+ ", " + std::to_string(filter[7])
					+ ", " + std::to_string(filter[8])
					+ ", " + std::to_string(filter[9]) + "\r\n";
#else
				return "Typeid: " + std::to_string(_typeid);
#endif
			};
		};

		struct MemorialShopRareItem {
			void clear() { memset(this, 0, sizeof(MemorialShopRareItem)); };
			unsigned int  active;
			struct Gacha {
				unsigned int  number;
				unsigned int  count;
			};
			Gacha gacha;
			unsigned int  _typeid;
			unsigned int  probability;
			unsigned int  rare_type;

			unsigned int  filter[10];
			char s_string[28];
			std::string toString() {
#ifdef _DEBUG
				return "Typeid: " + std::to_string(_typeid) + "(0x" + hex_util::ltoaToHex(_typeid) + ")"
					+ "\r\nRare Type: " + std::to_string(rare_type)
					+ "\r\nProbability: " + std::to_string(probability)
					+ "\r\nFilter(s): " + std::to_string(filter[0])
						+ ", " + std::to_string(filter[1])
						+ ", " + std::to_string(filter[2])
						+ ", " + std::to_string(filter[3])
						+ ", " + std::to_string(filter[4])
						+ ", " + std::to_string(filter[5])
						+ ", " + std::to_string(filter[6])
						+ ", " + std::to_string(filter[7])
						+ ", " + std::to_string(filter[8])
						+ ", " + std::to_string(filter[9])
					+ "\r\nString S (Index): " + std::string(s_string) + "\r\n";
#else
				return "Typeid: " + std::to_string(_typeid);
#endif
			};
		};

		struct AddonPart {
			void clear() { memset(this, 0, sizeof(AddonPart)); };
			unsigned int  active;
			unsigned int  _typeid;
			char name[40];
			char textura[6][40];
		};

		struct ArtifactManaInfo {
			void clear() { memset(this, 0, sizeof(ArtifactManaInfo)); };
			unsigned int  active;
			unsigned int  artifact_typeid;
			unsigned int  mana_typeid;
			char info[132];
			unsigned int  type;
			unsigned int  ulUnknown;
			std::string toString() {
#ifdef _DEBUG
				return "Artifact Typeid: " + std::to_string(artifact_typeid) + "(0x" + hex_util::ltoaToHex(artifact_typeid) + ")"
					+ "\r\nMana Typeid: " + std::to_string(mana_typeid) + "(0x" + hex_util::ltoaToHex(mana_typeid) + ")"
					+ "\r\nType: " + std::to_string(type)
					+ "\r\nulUnknown: " + std::to_string(ulUnknown)
					+ "\r\nInfo.\r\n" + std::string(info);
#else
				return "Artifact Typeid: " + std::to_string(artifact_typeid) + "(0x" + hex_util::ltoaToHex(artifact_typeid) + ")"
					+ "\r\nMana Typeid: " + std::to_string(mana_typeid) + "(0x" + hex_util::ltoaToHex(mana_typeid) + ")";
#endif
			}
		};

		struct CaddieVoiceTable {
			void clear() { memset(this, 0, sizeof(CaddieVoiceTable)); };
			unsigned int  _typeid;
			char name[64];
			unsigned char type;
			char shot_name[65];
			std::string toString() {
#ifdef _DEBUG
				return "Name: " + std::string(name)
					+ "\r\nTypeid: " + std::to_string(_typeid) + "(0x" + hex_util::ltoaToHex(_typeid) + ")"
					+ "\r\nType: " + std::to_string(type)
					+ "\r\nShot Name: " + std::string(shot_name);
#else
				return "Typeid: " + std::to_string(_typeid) + "(0x" + hex_util::ltoaToHex(_typeid) + ")";
#endif
			};
		};

		struct ErrorCodeInfo {
			void clear() { memset(this, 0, sizeof(ErrorCodeInfo)); };
			unsigned int  active;
			unsigned int  code;
			unsigned int  type;
			char info[260];
			std::string toString() {
#ifdef _DEBUG
				return "ErrorCode: " + std::to_string(code) + " (0x" + hex_util::ltoaToHex(code) + ")"
					+ "\r\nType: " + std::to_string(type)
					+ "\r\nInfo.\r\n" + std::string(info);
#else
				return "ErrorCode: " + std::to_string(code) + " (0x" + hex_util::ltoaToHex(code) + ")";
#endif
			};
		};

		struct FurnitureAbility {

			union uAbilityType {
				uAbilityType(uint32_t _ul = 0u) : ulAbilityType(_ul) {};
				void clear() { ulAbilityType = 0u; };
				uint32_t ulAbilityType;
				struct {
					uint32_t buff : 1, : 0;
				}stBit;
			};
			union uSuccessType {
				uSuccessType(uint16_t _ul = 0u) : ulSuccessType(_ul) {};
				void clear() { ulSuccessType = 0u; };
				uint16_t ulSuccessType;
				struct {
					uint16_t stay : 1;
					uint16_t putin : 1;
					uint16_t putout : 1, : 0;
				}stBit;
			};
			union uEffectType {
				uEffectType(uint16_t _ul = 0u) : ulEffectType(_ul) {};
				void clear() { ulEffectType = 0u; };
				uint16_t ulEffectType;
				struct {
					uint16_t me : 1;
					uint16_t _friend : 1;
					uint16_t guild : 1;
					uint16_t all : 1, : 0;
				}stBit;
			};
			void clear() { memset(this, 0, sizeof(FurnitureAbility)); };
			unsigned int  active;
			unsigned int  _typeid;
			uAbilityType  type;
			unsigned int  stay_time;
			uSuccessType success_type;
			uEffectType  effect_type;
			unsigned int  set_in_typeid;
			unsigned int  max_qntd;
			SYSTEMTIME    date;
			unsigned int  during_time;
			struct Item {
				unsigned int  _typeid;
				unsigned int  probability;
			};
			Item item;
			unsigned int  max_count_by_user;
			std::string toString() {
#ifdef _DEBUG
				return "Typeid: " + std::to_string(_typeid) + "(0x" + hex_util::ltoaToHex(_typeid) + ")"
					+ "\r\nType: " + std::to_string(type.ulAbilityType)
					+ "\r\nSetInTypeid): " + std::to_string(set_in_typeid)
					+ "\r\nMax Qntd: " + std::to_string(max_qntd)
					+ "\r\nSuccessType: " + std::to_string(success_type.ulSuccessType)
					+ "\r\nEffectType: " + std::to_string(effect_type.ulEffectType)
					+ "\r\nMaxCountByUser: " + std::to_string(max_count_by_user)
					+ "\r\nStayTime: " + std::to_string(stay_time)
					+ "\r\nDuringTime " + std::to_string(during_time)
					+ "\r\nDate Start: " + _formatDate(date);
#else
				return "Typeid: " + std::to_string(_typeid) + "(0x" + hex_util::ltoaToHex(_typeid) + ")";
#endif
			};
		};

		struct HoleCupDropItem {
			void clear() { memset(this, 0, sizeof(HoleCupDropItem)); };
			unsigned int  _typeid;
			char animation[40];
		};

		struct LevelUpPrizeItem {
			void clear() { memset(this, 0, sizeof(LevelUpPrizeItem)); };
			unsigned char active;
			char name[33];
			unsigned short level;
			struct Reward {
				unsigned int  _typeid[2];
				unsigned int  qntd[2];
				unsigned int  time[2];
			};
			Reward reward;
			char description[132];
		};

		struct NonVisibleItemTable {
			void clear() { memset(this, 0, sizeof(NonVisibleItemTable)); };
			unsigned int  active;
			unsigned int  type;
			unsigned int  _typeid;
			struct Date {
				SYSTEMTIME start;
				SYSTEMTIME end;
			};
			Date date;
		};

		struct PointShop {
			void clear() { memset(this, 0, sizeof(PointShop)); };
			unsigned int  active;
			unsigned int  _typeid;
			unsigned int  point;
			unsigned int  qntd;
			unsigned int  flag;
		};

		struct ScratchRewardSetting {
			void clear() { memset(this, 0, sizeof(ScratchRewardSetting)); };
			unsigned int  active;
			unsigned int  _typeid;
		};

		struct SetEffectTable {
			enum eEFFECT : unsigned char {
				ANIMATION = 1,
				UNKNOWN_V2,
				CUTIN,
				PIXEL,
				BASE,
				ONE_ALL_STATS,
				WIND_DECREASE,
				PATINHA,
			};
			enum eEFFECT_TYPE : unsigned char {
				UNKNOWN_V1 = 1,
				GAME = 2,
				ROOM = 4,
				LOUNGE = 8,
			};
			void clear() { memset(this, 0, sizeof(SetEffectTable)); };
			unsigned int  id;
			struct Effect {
				unsigned int  effect[3];
				unsigned int  type[3];
			};
			Effect effect;
			struct Item {
				unsigned int  _typeid[5];
				unsigned char active[5];
			};
			Item item;
			unsigned char ucUnknown[11];
			unsigned short slot[5];
			unsigned short effect_add_power;
			std::string toString() {
#ifdef _DEBUG
				return "ID: " + std::to_string(id)
					+ "\r\nType(s): " + std::to_string(effect.type[0])
						+ ", " + std::to_string(effect.type[1])
						+ ", " + std::to_string(effect.type[2])
					+ "\r\nEffect(s): " + std::to_string(effect.effect[0])
						+ ", " + std::to_string(effect.effect[1])
						+ ", " + std::to_string(effect.effect[2])
					+ "\r\nItem(ns) Typeid: " + std::to_string(item._typeid[0])
						+ ", " + std::to_string(item._typeid[1])
						+ ", " + std::to_string(item._typeid[2])
						+ ", " + std::to_string(item._typeid[3])
						+ ", " + std::to_string(item._typeid[4])
					+ "\r\nItem(ns) active: " + std::to_string(item.active[0])
						+ ", " + std::to_string(item.active[1])
						+ ", " + std::to_string(item.active[2])
						+ ", " + std::to_string(item.active[3])
						+ ", " + std::to_string(item.active[4])
					+ "\r\nSlot(s): " + std::to_string(slot[0])
						+ ", " + std::to_string(slot[1])
						+ ", " + std::to_string(slot[2])
						+ ", " + std::to_string(slot[3])
						+ ", " + std::to_string(slot[4])
					+ "\r\nEffect Add Power: " + std::to_string(effect_add_power)
					+ "\r\nucUnknown\r\n" + hex_util::BufferToHexString(ucUnknown, sizeof(ucUnknown));
#else
				return "ID: " + std::to_string(id);
#endif
			};
		};

		struct ShopLimitItem {
			void clear() { memset(this, 0, sizeof(ShopLimitItem)); };
			unsigned int  active;
			unsigned int  type;
			unsigned int  _typeid;
			unsigned int  level_max;
			unsigned int  level_min;
			unsigned int  ulUnknown;
			unsigned int  ulUnknown2;
			struct Date {
				SYSTEMTIME start;
				SYSTEMTIME end;
			};
			Date date;
		};

		struct SpecialPrizeItem {
			void clear() { memset(this, 0, sizeof(SpecialPrizeItem)); };
			unsigned int  _typeid;
			unsigned int  type;
			float rate;
			std::string toString() {
#ifdef _DEBUG
				return "Typeid: " + std::to_string(_typeid) + "(0x" + hex_util::ltoaToHex(_typeid) + ")"
					+ "\r\nType: " + std::to_string(type)
					+ "\r\nRate: " + std::to_string(rate);
#else
				return "Typeid: " + std::to_string(_typeid) + "(0x" + hex_util::ltoaToHex(_typeid) + ")";
#endif
			};
		};

		struct SubscriptionItemTable {
			void clear() { memset(this, 0, sizeof(SubscriptionItemTable)); };
			unsigned int  active;
			unsigned int  type;
			unsigned int  _typeid;
			struct Date {
				SYSTEMTIME start;
				SYSTEMTIME end;
			};
			Date date;
		};

		struct TikiShopBase {
			void clear() { memset(this, 0, sizeof(TikiShopBase)); };
			unsigned int  id;
			unsigned char type;
			char name[35];
		};

		struct TikiPointTable : public TikiShopBase {
			void clear() { memset(this, 0, sizeof(TikiPointTable)); };
			unsigned int  min;
			unsigned int  max;
		};

		struct TikiRecipe : public TikiShopBase {
			void clear() { memset(this, 0, sizeof(TikiRecipe)); };
			unsigned int  recipe_qntd[3];
		};

		struct TikiSpecialTable : public TikiShopBase {
			void clear() { memset(this, 0, sizeof(TikiSpecialTable)); };
			unsigned int  qntd;
			unsigned int  recipe_qntd[4];
		};

		struct TimeLimitItem {
			void clear() { memset(this, 0, sizeof(TimeLimitItem)); };
			unsigned int  active;
			unsigned int  _typeid;
			char name[40];
			char icon[40];
			unsigned int  type;
			unsigned int  percent;
			unsigned int  time;
		};

		struct TwinsItemTable {
			void clear() { memset(this, 0, sizeof(TwinsItemTable)); };
			unsigned int  active;
			unsigned int  type;
			unsigned int  _typeid[5];
		};

    }
}

#if defined(__linux__)
#pragma pack()
#endif

#endif
