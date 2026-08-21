
#if defined(_WIN32)
#pragma pack(1)
#endif

#pragma once
#ifndef _STDA_IFF_H
#define _STDA_IFF_H

#include <cstdint>

#if INTPTR_MAX == INT64_MAX
#define PATH_LIBZIP_LIB "../../Projeto IOCP/ZIP/lib/zip-64.lib"
#elif INTPTR_MAX == INT32_MAX
#define PATH_LIBZIP_LIB "../../Projeto IOCP/ZIP/lib/zip.lib"
#else
#error Unknown pointer size or missing size macros!
#endif

#if defined(__linux__)
#include "WinPort.h"
#include <pthread.h>
#endif

#define PATH_PANGYA_IFF "data/pangya_jp.iff"

#define IFF_VERSION 0x0D

#include "../TYPE/data_iff.h"

#include "../TYPE/singleton.h"

#include <string>
#include <map>
#include <vector>

namespace stdA {
    class iff {
		public:
			enum IFF_GROUP_ID: uint32_t {
				CHARACTER = 1,
				PART,
				CLUB,
				CLUBSET,
				BALL,
				ITEM,
				CADDIE,
				CAD_ITEM,
				SET_ITEM,
				COURSE,
				MATCH,
				ENCHANT = 13,
				SKIN,
				HAIR_STYLE,
				MASCOT,
				FURNITURE = 18,
				ACHIEVEMENT,
				COUNTER_ITEM = 27,
				AUX_PART,
				QUEST_STUFF,
				QUEST_ITEM,
				CARD,
			};

        public:
            iff();
            ~iff();

			  bool isLoad();

			  void load();
			  void reset();
			  void reload();

			  IFF::Part *findPart(uint32_t _typeid);
			  IFF::AuxPart *findAuxPart(uint32_t _typeid);
			  IFF::Ball *findBall(uint32_t _typeid);
			  IFF::Caddie *findCaddie(uint32_t _typeid);
			  IFF::CaddieItem *findCaddieItem(uint32_t _typeid);
			  IFF::CadieMagicBox *findCadieMagicBox(uint32_t _seq);
			  IFF::Card *findCard(uint32_t _typeid);
			  IFF::Character *findCharacter(uint32_t _typeid);
			  IFF::Club *findClub(uint32_t _typeid);
			  IFF::ClubSet *findClubSet(uint32_t _typeid);
			  IFF::Achievement *findAchievement(uint32_t _typeid);
			  IFF::CounterItem *findCounterItem(uint32_t _typeid);
			  IFF::Item *findItem(uint32_t _typeid);
			  IFF::Mascot *findMascot(uint32_t _typeid);
			  IFF::QuestItem *findQuestItem(uint32_t _typeid);
			  IFF::QuestStuff *findQuestStuff(uint32_t _typeid);
			  IFF::SetItem *findSetItem(uint32_t _typeid);
			  IFF::ClubSetWorkShopLevelUpProb *findClubSetWorkShopLevelUpProb(uint32_t _tipo);
			  IFF::ClubSetWorkShopRankUpExp *findClubSetWorkShopRankExp(uint32_t _tipo);
			  IFF::Course *findCourse(uint32_t _typeid);
			  IFF::CutinInfomation *findCutinInfomation(uint32_t _typeid);
			  IFF::Enchant *findEnchant(uint32_t _typeid);
			  IFF::Furniture *findFurniture(uint32_t _typeid);
			  IFF::HairStyle *findHairStyle(uint32_t _typeid);
			  IFF::Match *findMatch(uint32_t _typeid);
			  IFF::Skin *findSkin(uint32_t _typeid);
			  IFF::Ability *findAbility(uint32_t _typeid);
			  IFF::Desc *findDesc(uint32_t _typeid);
			  IFF::GrandPrixAIOptionalData *findGrandPrixAIOptionalData(uint32_t _id);
			  IFF::GrandPrixConditionEquip *findGrandPrixConditionEquip(uint32_t _typeid);
			  IFF::GrandPrixData *findGrandPrixData(uint32_t _typeid);
			  IFF::MemorialShopCoinItem *findMemorialShopCoinItem(uint32_t _typeid);
			  IFF::ArtifactManaInfo *findArtifactManaInfo(uint32_t _typeid);
			  IFF::ErrorCodeInfo *findErrorCodeInfo(uint32_t _code);
			  IFF::HoleCupDropItem *findHoleCupDropItem(uint32_t _typeid);
			  IFF::LevelUpPrizeItem *findLevelUpPrizeItem(uint32_t _level);
			  IFF::NonVisibleItemTable *findNonVisibleItemTable(uint32_t _typeid);
			  IFF::PointShop *findPointShop(uint32_t _typeid);
			  IFF::ShopLimitItem *findShopLimitItem(uint32_t _typeid);
			  IFF::SpecialPrizeItem *findSpecialPrizeItem(uint32_t _typeid);
			  IFF::SubscriptionItemTable *findSubscriptionItemTable(uint32_t _typeid);
			  IFF::SetEffectTable *findSetEffectTable(uint32_t _id);
			  IFF::TikiPointTable *findTikiPointTable(uint32_t _id);
			  IFF::TikiRecipe *findTikiRecipe(uint32_t _id);
			  IFF::TikiSpecialTable *findTikiSpecialTable(uint32_t _id);
			  IFF::TimeLimitItem *findTimeLimitItem(uint32_t _typeid);

			  std::vector< IFF::AddonPart > findAddonPart(uint32_t _typeid);
			  std::vector< IFF::CadieMagicBoxRandom > findCadieMagicBoxRandom(uint32_t _id);
			  std::vector< IFF::CharacterMastery > findCharacterMastery(uint32_t _typeid);
			  std::vector< IFF::ClubSetWorkShopLevelUpLimit > findClubSetWorkShopLevelUpLimit(uint32_t _tipo);
			  std::vector< IFF::GrandPrixRankReward > findGrandPrixRankReward(uint32_t _typeid);
			  std::vector< IFF::GrandPrixSpecialHole > findGrandPrixSpecialHole(uint32_t _typeid);
			  std::vector< IFF::MemorialShopRareItem > findMemorialShopRareItem(uint32_t _gacha_num);
			  std::vector< IFF::CaddieVoiceTable > findCaddieVoiceTable(uint32_t _typeid);
			  std::vector< IFF::FurnitureAbility > findFurnitureAbility(uint32_t _typeid);
			  std::vector< IFF::TwinsItemTable > findTwinsItemTable(uint32_t _type);

			  IFF::SetEffectTable *findFirstItemInSetEffectTable(uint32_t _typeid);
			  std::vector< IFF::SetEffectTable > findAllItemInSetEffectTable(uint32_t _typeid);

			  IFF::Base *findCommomItem(uint32_t _typeid);

			  bool ItemEquipavel(uint32_t _typeid);

			  bool IsCanOverlapped(uint32_t _typeid);

			  bool IsBuyItem(uint32_t _typeid);

			  bool IsGiftItem(uint32_t _typeid);

			  bool IsOnlyDisplay(uint32_t _typeid);

			  bool IsOnlyPurchase(uint32_t _typeid);

			  bool IsOnlyGift(uint32_t _typeid);

			  bool IsItemEquipable(uint32_t _typeid);

			  bool IsTitle(uint32_t _typeid);

			  std::vector< IFF::ClubSet > findClubSetOriginal(uint32_t _typeid);

			  std::map< uint32_t, IFF::Achievement >& getAchievement();

			  std::map< uint32_t, IFF::QuestItem >& getQuestItem();

			  std::map< uint32_t, IFF::CounterItem >& getCounterItem();

			  std::map< uint32_t, IFF::Item >& getItem();

			  std::map< uint32_t, IFF::Card >& getCard();

			  std::map< uint32_t, IFF::Skin >& getSkin();

			  std::map< uint32_t, IFF::AuxPart >& getAuxPart();

			  std::map< uint32_t, IFF::Ball >& getBall();

			  std::map< uint32_t, IFF::Character >& getCharacter();

			  std::map< uint32_t, IFF::Caddie >& getCaddie();

			  std::map< uint32_t, IFF::CaddieItem >& getCaddieItem();

			  std::vector< IFF::CadieMagicBox >& getCadieMagicBox();

			  std::map< uint32_t, IFF::ClubSet >& getClubSet();

			  std::map< uint32_t, IFF::HairStyle >& getHairStyle();

			  std::map< uint32_t, IFF::Part >& getPart();

			  std::map< uint32_t, IFF::Mascot >& getMascot();

			  std::map< uint32_t, IFF::SetItem >& getSetItem();

			  std::map< uint32_t, IFF::Desc >& getDesc();

			  std::map< uint32_t, IFF::LevelUpPrizeItem >& getLevelUpPrizeItem();

			  std::map< uint32_t, IFF::MemorialShopCoinItem >& getMemorialShopCoinItem();

			  std::vector< IFF::MemorialShopRareItem >& getMemorialShopRareItem();

			  std::map< uint32_t, IFF::Course >& getCourse();

			  std::map< uint32_t, IFF::TimeLimitItem >& getTimeLimitItem();

			  std::map< uint32_t, IFF::GrandPrixAIOptionalData >& getGrandPrixAIOptionalData();

			  std::map< uint32_t, IFF::GrandPrixData >& getGrandPrixData();

			  std::map< uint32_t, IFF::Ability >& getAbility();

			  std::map< uint32_t, IFF::SetEffectTable >& getSetEffectTable();

			 std::map< uint32_t, IFF::QuestStuff >& getQuestStuff();

			 std::map< uint32_t, IFF::Club >& getClub();

			 std::map< uint32_t, IFF::ClubSetWorkShopLevelUpProb >& getClubSetWorkShopLevelUpProb();

			 std::map< uint32_t, IFF::ClubSetWorkShopRankUpExp >& getClubSetWorkShopRankUpExp();

			 std::map< uint32_t, IFF::CutinInfomation >& getCutinInfomation();

			 std::map< uint32_t, IFF::Enchant >& getEnchant();

			 std::map< uint32_t, IFF::Furniture >& getFurniture();

			 std::map< uint32_t, IFF::Match >& getMatch();

			 std::map< uint32_t, IFF::GrandPrixConditionEquip >& getGrandPrixConditionEquip();

			 std::map< uint32_t, IFF::ArtifactManaInfo >& getArtifactManaInfo();

			 std::map< uint32_t, IFF::ErrorCodeInfo >& getErrorCodeInfo();

			 std::map< uint32_t, IFF::HoleCupDropItem >& getHoleCupDropItem();

			 std::map< uint32_t, IFF::NonVisibleItemTable >& getNonVisibleItemTable();

			 std::map< uint32_t, IFF::PointShop >& getPointShop();

			 std::map< uint32_t, IFF::ShopLimitItem >& getShopLimitItem();

			 std::map< uint32_t, IFF::SpecialPrizeItem >& getSpecialPrizeItem();

			 std::map< uint32_t, IFF::SubscriptionItemTable >& getSubscriptionItemTable();

			 std::map< uint32_t, IFF::TikiPointTable >& getTikiPointTable();

			 std::map< uint32_t, IFF::TikiRecipe >& getTikiRecipe();

			 std::map< uint32_t, IFF::TikiSpecialTable >& getTikiSpecialTable();

			 std::vector< IFF::AddonPart >& getAddonPart();

			 std::vector< IFF::CadieMagicBoxRandom >& getCadieMagicBoxRandom();

			 std::vector< IFF::CharacterMastery >& getCharacterMastery();

			 std::vector< IFF::ClubSetWorkShopLevelUpLimit >& getClubSetWorkShopLevelUpLimit();

			 std::vector< IFF::GrandPrixRankReward >& getGrandPrixRankReward();

			 std::vector< IFF::GrandPrixSpecialHole >& getGrandPrixSpecialHole();

			 std::vector< IFF::CaddieVoiceTable >& getCaddieVoiceTable();

			 std::vector< IFF::FurnitureAbility >& getFurnitureAbility();

			 std::vector< IFF::TwinsItemTable >& getTwinsItemTable();

		private:

              std::map< uint32_t, IFF::Achievement > load_achievement();
			  std::map< uint32_t, IFF::QuestItem > load_quest_item();
			  std::map< uint32_t, IFF::QuestStuff > load_quest_stuff();
			  std::map< uint32_t, IFF::CounterItem > load_counter_item();

			  std::map< uint32_t, IFF::Item > load_item();

			  std::map< uint32_t, IFF::Part > load_part();

			  std::map< uint32_t, IFF::AuxPart > load_aux_part();

			  std::map< uint32_t, IFF::Ball > load_ball();

			  std::map< uint32_t, IFF::Caddie > load_caddie();

			  std::map< uint32_t, IFF::CaddieItem > load_caddie_item();

			  std::vector< IFF::CadieMagicBox > load_cadie_magic_box();

			  std::vector< IFF::CadieMagicBoxRandom > load_cadie_magic_box_random();

			  std::map< uint32_t, IFF::Card > load_card();

			  std::map< uint32_t, IFF::Character > load_character();

			  std::vector< IFF::CharacterMastery > load_character_mastery();

			  std::map< uint32_t, IFF::Club > load_club();

			  std::map< uint32_t, IFF::ClubSet > load_club_set();

			  std::vector< IFF::ClubSetWorkShopLevelUpLimit > load_club_set_work_shop_level_up_limit();

			  std::map< uint32_t, IFF::ClubSetWorkShopLevelUpProb > load_club_set_work_shop_level_up_prob();

			  std::map< uint32_t, IFF::ClubSetWorkShopRankUpExp > load_club_set_work_shop_rank_up_exp();

			  std::map< uint32_t, IFF::Course > load_course();

			  std::map< uint32_t, IFF::CutinInfomation > load_cutin_infomation();

			  std::map< uint32_t, IFF::Enchant > load_enchant();

			  std::map< uint32_t, IFF::Furniture > load_furniture();

			  std::map< uint32_t, IFF::HairStyle > load_hair_style();

			  std::map< uint32_t, IFF::Match > load_match();

			  std::map< uint32_t, IFF::Skin > load_skin();

			  std::map< uint32_t, IFF::Ability > load_ability();

			  std::map< uint32_t, IFF::Desc > load_desc();

			  std::map< uint32_t, IFF::GrandPrixAIOptionalData > load_grand_prix_ai_optional_data();

			  std::map< uint32_t, IFF::GrandPrixConditionEquip > load_grand_prix_condition_equip();

			  std::map< uint32_t, IFF::GrandPrixData > load_grand_prix_data();

			  std::vector< IFF::GrandPrixRankReward > load_grand_prix_rank_reward();

			  std::vector< IFF::GrandPrixSpecialHole > load_grand_prix_special_hole();

			  std::map< uint32_t, IFF::MemorialShopCoinItem > load_memorial_shop_coin_item();

			  std::vector< IFF::MemorialShopRareItem > load_memorial_shop_rare_item();

			  std::vector< IFF::AddonPart > load_addon_part();

			  std::map< uint32_t, IFF::ArtifactManaInfo > load_artifact_mana_info();

			  std::vector< IFF::CaddieVoiceTable > load_caddie_voice_table();

			  std::map< uint32_t, IFF::ErrorCodeInfo > load_error_code_info();

			  std::vector< IFF::FurnitureAbility > load_furniture_ability();

			  std::map< uint32_t, IFF::HoleCupDropItem > load_hole_cup_drop_item();

			  std::map< uint32_t, IFF::LevelUpPrizeItem > load_level_up_prize_item();

			  std::map< uint32_t, IFF::NonVisibleItemTable > load_non_visible_item_table();

			  std::map< uint32_t, IFF::PointShop > load_point_shop();

			  std::map< uint32_t, IFF::ShopLimitItem > load_shop_limit_item();

			  std::map< uint32_t, IFF::SpecialPrizeItem > load_special_prize_item();

			  std::map< uint32_t, IFF::SubscriptionItemTable > load_subscription_item_table();

			  std::map< uint32_t, IFF::SetEffectTable > load_set_effect_table();

			  std::map< uint32_t, IFF::TikiPointTable > load_tiki_point_table();

			  std::map< uint32_t, IFF::TikiRecipe > load_tiki_recipe();

			  std::map< uint32_t, IFF::TikiSpecialTable > load_tiki_special_table();

			  std::map< uint32_t, IFF::TimeLimitItem > load_time_limit_item();

			  std::vector< IFF::TwinsItemTable > load_twins_item_table();

			  std::map< uint32_t, IFF::SetItem > load_set_item();

			  std::map< uint32_t, IFF::Mascot > load_mascot();

		public:

			  uint32_t getItemGroupIdentify(uint32_t _typeid);
			  uint32_t getItemSubGroupIdentify24(uint32_t _typeid);
			  uint32_t getItemSubGroupIdentify22(uint32_t _typeid);
			  uint32_t getItemSubGroupIdentify21(uint32_t _typeid);
			  uint32_t getItemCharIdentify(uint32_t _typeid);
			  uint32_t getItemCharPartNumber(uint32_t _typeid);
			  uint32_t getItemCharTypeNumber(uint32_t _typeid);
			  uint32_t getItemIdentify(uint32_t _typeid);
			  uint32_t getItemTitleNum(uint32_t _typeid);

			 uint32_t getMatchTypeIdentity(uint32_t _typeid);

			 uint32_t getCaddieItemType(uint32_t _typeid);
			 uint32_t getCaddieIdentify(uint32_t _typeid);

			 uint32_t getEnchantSlotStat(uint32_t _typeid);

			 uint32_t getItemAuxPartNumber(uint32_t _typeid);

			  uint32_t getGrandPrixAba(uint32_t _typeid);

			  uint32_t getGrandPrixType(uint32_t _typeid);

			  bool isGrandPrixEvent(uint32_t _typeid);

			  bool isGrandPrixNormal(uint32_t _typeid);

		protected:
			 std::map< uint32_t, IFF::Part > m_part;
			 std::map< uint32_t, IFF::Item > m_item;
			 std::map< uint32_t, IFF::SetItem > m_set_item;
			 std::map< uint32_t, IFF::Mascot > m_mascot;
			 std::map< uint32_t, IFF::Achievement > m_achievement;
			 std::map< uint32_t, IFF::CounterItem > m_counter_item;
			 std::map< uint32_t, IFF::QuestStuff > m_quest_stuff;
			 std::map< uint32_t, IFF::QuestItem > m_quest_item;
			 std::map< uint32_t, IFF::AuxPart > m_aux_part;
			 std::map< uint32_t, IFF::Ball > m_ball;
			 std::map< uint32_t, IFF::Caddie > m_caddie;
			 std::map< uint32_t, IFF::CaddieItem > m_caddie_item;
			 std::map< uint32_t, IFF::Card > m_card;
			 std::map< uint32_t, IFF::Character > m_character;
			 std::map< uint32_t, IFF::Club > m_club;
			 std::map< uint32_t, IFF::ClubSet > m_club_set;
			 std::map< uint32_t, IFF::ClubSetWorkShopLevelUpProb > m_club_set_work_shop_level_up_prob;
			 std::map< uint32_t, IFF::ClubSetWorkShopRankUpExp > m_club_set_work_shop_rank_exp;
			 std::map< uint32_t, IFF::Course > m_course;
			 std::map< uint32_t, IFF::CutinInfomation > m_cutin_infomation;
			 std::map< uint32_t, IFF::Enchant > m_enchant;
			 std::map< uint32_t, IFF::Furniture > m_furniture;
			 std::map< uint32_t, IFF::HairStyle > m_hair_style;
			 std::map< uint32_t, IFF::Match > m_match;
			 std::map< uint32_t, IFF::Skin > m_skin;
			 std::map< uint32_t, IFF::Ability > m_ability;
			 std::map< uint32_t, IFF::Desc > m_desc;
			 std::map< uint32_t, IFF::GrandPrixAIOptionalData > m_grand_prix_ai_optinal_data;
			 std::map< uint32_t, IFF::GrandPrixConditionEquip > m_grand_prix_condition_equip;
			 std::map< uint32_t, IFF::GrandPrixData > m_grand_prix_data;
			 std::map< uint32_t, IFF::MemorialShopCoinItem > m_memorial_shop_coin_item;
			 std::map< uint32_t, IFF::ArtifactManaInfo > m_artifact_mana_info;
			 std::map< uint32_t, IFF::ErrorCodeInfo > m_error_code_info;
			 std::map< uint32_t, IFF::HoleCupDropItem > m_hole_cup_drop_item;
			 std::map< uint32_t, IFF::LevelUpPrizeItem > m_level_up_prize_item;
			 std::map< uint32_t, IFF::NonVisibleItemTable > m_non_visible_item_table;
			 std::map< uint32_t, IFF::PointShop > m_point_shop;
			 std::map< uint32_t, IFF::ShopLimitItem > m_shop_limit_item;
			 std::map< uint32_t, IFF::SpecialPrizeItem > m_special_prize_item;
			 std::map< uint32_t, IFF::SubscriptionItemTable > m_subscription_item_table;
			 std::map< uint32_t, IFF::SetEffectTable > m_set_effect_table;
			 std::map< uint32_t, IFF::TikiPointTable > m_tiki_point_table;
			 std::map< uint32_t, IFF::TikiRecipe > m_tiki_recipe;
			 std::map< uint32_t, IFF::TikiSpecialTable > m_tiki_special_table;
			 std::map< uint32_t, IFF::TimeLimitItem > m_time_limit_item;

			 std::vector< IFF::AddonPart > m_addon_part;
			 std::vector< IFF::CadieMagicBox > m_cadie_magic_box;
			 std::vector< IFF::CadieMagicBoxRandom > m_cadie_magic_box_random;
			 std::vector< IFF::CharacterMastery > m_character_mastery;
			 std::vector< IFF::ClubSetWorkShopLevelUpLimit > m_club_set_work_shop_level_up_limit;
			 std::vector< IFF::GrandPrixRankReward > m_grand_prix_rank_reward;
			 std::vector< IFF::GrandPrixSpecialHole > m_grand_prix_special_hole;
			 std::vector< IFF::MemorialShopRareItem > m_memorial_shop_rare_item;
			 std::vector< IFF::CaddieVoiceTable > m_caddie_voice_table;
			 std::vector< IFF::FurnitureAbility > m_furniture_ability;
			 std::vector< IFF::TwinsItemTable > m_twins_item_table;

			 bool m_loaded;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif
    };

	typedef Singleton< iff > sIff;
}

#endif
