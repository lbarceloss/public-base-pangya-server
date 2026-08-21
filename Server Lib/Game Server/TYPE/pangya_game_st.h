
#pragma once
#ifndef _STDA_PANGYA_GAME_ST_H
#define _STDA_PANGYA_GAME_ST_H

#if defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

#include "../../Projeto IOCP/UTIL/exception.h"

#include "../../Projeto IOCP/TYPE/pangya_st.h"
#include "../../Projeto IOCP/TYPE/stda_error.h"

#include <vector>
#include <map>

#include <algorithm>
#include <iomanip>

#include "../../Projeto IOCP/UTIL/util_time.h"

#include <cmath>
#include <cstdint>

#define CLEAR_10_DAILY_QUEST_TYPEID 0x78800001u

namespace stdA {

#if defined(__linux__)
#pragma pack(1)
#endif

#define ASSIST_ITEM_TYPEID			0x1BE00016u

#define GRAND_PRIX_TICKET			0x1A000264u
#define LIMIT_GRAND_PRIX_TICKET		50

#define MULLIGAN_ROSE_TYPEID		0x1800000Eu

#define DEFAULT_COMET_TYPEID 0x14000000u

#define AIR_KNIGHT_SET 0x10000000u

#define CLUB_PATCHER_TYPEID 0x1A00018Fu

#define MILAGE_POINT_TYPEID 0x1A0002A7u
#define TIKI_POINT_TYPEID 0x1A0002A6u

#define SPECIAL_SHUFFLE_COURSE_TICKET_TYPEID	0x1A0000F7u

#define PANG_POUCH_TYPEID	0x1A000010u
#define EXP_POUCH_TYPEID	0x1A00015Du
#define CP_POUCH_TYPEID		0x1A000160u

#define DECREASE_COMBO_VALUE	10

#define MEDIDA_PARA_YARDS 0.3125f

constexpr float GOOD_PLAYER_ICON = 3.f;
constexpr float QUITER_ICON_1 = 20.f;
constexpr float QUITER_ICON_2 = 30.f;

const uint32_t active_item_cant_have_2_inveroty[]{ 402653229u, 402653231u, };

constexpr auto TROFEL_GM_EVENT_TYPEID = 0x2D0A3B00u;

constexpr unsigned char cadie_cauldron_Hermes_random_id = 2u;
constexpr unsigned char cadie_cauldron_Jester_random_id = 3u;
constexpr unsigned char cadie_cauldron_Twilight_random_id = 4u;

const uint32_t cadie_cauldron_Hermes_item_typeid[]{ 0x08010032u, 0x0804e058u, 0x0808e025u, 0x080ce041u, 0x0810a030u, 0x0814e05eu, 0x0818a060u, 0x081ce02fu, 0x0820e02fu };
const uint32_t cadie_cauldron_Jester_item_typeid[]{ 0x08000848u, 0x08040863u, 0x0808082bu, 0x080c002cu, 0x08100033u, 0x0814003eu, 0x0818005eu, 0x081c002bu, 0x08200018u, 0x0824000eu, 0x0828001du, 0x08380004u, 0x0830000cu, 0x082c0004u };
const uint32_t cadie_cauldron_Twilight_item_typeid[]{ 0x0801a812u, 0x08050811u, 0x0809081du, 0x080d481cu, 0x0811201bu, 0x08162810u, 0x08196013u, 0x081da817u, 0x0821a80cu };

	enum eBROADCAST_TYPES : unsigned char {
		BT_HIDE_BROADCAST,
		BT_SPINNING_CUBE_RARE,
		BT_SPINNING_CUBE_WIN_PANG_POUCH,
		BT_GOLDEN_TIME_START_OF_DAY = 11,
		BT_GOLDEN_TIME_START_ROUND,
		BT_GOLDEN_TIME_ROUND_MORE_PEOPLE,
		BT_GOLDEN_TIME_ROUND_REWARD_PLAYER,
		BT_GOLDEN_TIME_FINISH_ROUND,
		BT_GOLDEN_TIME_FINISH_OF_DAY,
		BT_GOLDEN_TIME_FINISH,
		BT_GOLDEN_TIME_ROUND_NOT_HAVE_WINNERS,
		BT_MESSAGE_PLAIN = 20,
		BT_GRAND_ZODIAC_EVENT_START_TIME,
	};

	struct player_info {
		player_info() {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(player_info));
		};
		uint32_t uid;
		BlockFlag block_flag;
		unsigned short level;
		char id[22];
		char nickname[22];
		char pass[40];
	};

#if defined(__linux__)
#pragma pack()
#endif
	struct stSyncUpdateDB {
		enum eSTATE_UPDATE : unsigned char {
			NONE,
			REQUEST_UPDATE,
			UPDATE_CONFIRMED,
			ERROR_UPDATE,
		};
		stSyncUpdateDB() : m_state(eSTATE_UPDATE::NONE) {

#if defined(_WIN32)
			InitializeCriticalSection(&m_cs);
			InitializeConditionVariable(&m_cv);
#elif defined(__linux__)
			INIT_PTHREAD_MUTEXATTR_RECURSIVE;
			INIT_PTHREAD_MUTEX_RECURSIVE(&m_cs);
			DESTROY_PTHREAD_MUTEXATTR_RECURSIVE;

			pthread_cond_init(&m_cv, nullptr);
#endif

		};
		~stSyncUpdateDB() {

#if defined(_WIN32)

			WakeAllConditionVariable(&m_cv);

			DeleteCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_cond_broadcast(&m_cv);

			pthread_mutex_destroy(&m_cs);

			pthread_cond_destroy(&m_cv);
#endif
		};
		void requestUpdateOnDB() {

			uint32_t timeout_count = 3;

#if defined(_WIN32)
			DWORD error = 0u;
#elif defined(__linux__)
			int error = 0;
#endif

#if defined(_WIN32)
			EnterCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_lock(&m_cs);
#endif

			if (m_state == eSTATE_UPDATE::REQUEST_UPDATE) {

				while (m_state == eSTATE_UPDATE::REQUEST_UPDATE && timeout_count > 0) {

#if defined(_WIN32)
					if (SleepConditionVariableCS(&m_cv, &m_cs, 10000 ) == 0 && (error = GetLastError()) != ERROR_TIMEOUT) {

						LeaveCriticalSection(&m_cs);

						throw exception("[stSyncUpdateDB::requestUpdateOnDB][Error] nao conseguiu pegar o sinal do Condition Variable.",
							STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANGYA_GAME_ST, 34, error));
					}

					if (error == ERROR_TIMEOUT)
						--timeout_count;
#elif defined(__linux__)
					timespec wait_time = _milliseconds_to_timespec_clock_realtime(10000 );

					if ((error = pthread_cond_timedwait(&m_cv, &m_cs, &wait_time)) != 0 && error != ETIMEDOUT) {

						pthread_mutex_unlock(&m_cs);

						throw exception("[stSyncUpdateDB::requestUpdateOnDB][Error] nao conseguiu pegar o sinal do Condition Variable.",
							STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANGYA_GAME_ST, 34, error));
					}

					if (error == ETIMEDOUT)
						--timeout_count;
#endif
				}

				if (timeout_count == 0)
					_smp::message_pool::getInstance().push(new message("[stSyncUpdateDB::requestUpdateOnDB][WARNING] 30 seconds consumed, change state forced.", CL_FILE_LOG_AND_CONSOLE));

			}

			m_state = eSTATE_UPDATE::REQUEST_UPDATE;

#if defined(_WIN32)
			LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_unlock(&m_cs);
#endif

		};
		void confirmUpdadeOnDB() {

#if defined(_WIN32)
			EnterCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_lock(&m_cs);
#endif

			if (m_state != eSTATE_UPDATE::REQUEST_UPDATE) {

#if defined(_WIN32)
				LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
				pthread_mutex_unlock(&m_cs);
#endif

				throw exception("[stSyncUpdateDB::confirmUpdateOnDB][Error] m_state is wrong not REQUEST_UPDATE [value="
					+ std::to_string((unsigned short)m_state) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANGYA_GAME_ST, 35, 0));
			}

			m_state = eSTATE_UPDATE::UPDATE_CONFIRMED;

#if defined(_WIN32)

			WakeAllConditionVariable(&m_cv);
#elif defined(__linux__)

			pthread_cond_broadcast(&m_cv);
#endif

#if defined(_WIN32)
			LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_unlock(&m_cs);
#endif

		};
		void errorUpdateOnDB() {

#if defined(_WIN32)
			EnterCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_lock(&m_cs);
#endif

			if (m_state != eSTATE_UPDATE::REQUEST_UPDATE) {

#if defined(_WIN32)
				LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
				pthread_mutex_unlock(&m_cs);
#endif

				throw exception("[stSyncUpdateDB::errorUpdateOnDB][Error] m_state is wrong not REQUEST_UPDATE [value="
					+ std::to_string((unsigned short)m_state) + "]", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANGYA_GAME_ST, 35, 0));
			}

			m_state = eSTATE_UPDATE::ERROR_UPDATE;

#if defined(_WIN32)

			WakeAllConditionVariable(&m_cv);
#elif defined(__linux__)

			pthread_cond_broadcast(&m_cv);
#endif

#if defined(_WIN32)
			LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
			pthread_mutex_unlock(&m_cs);
#endif

		};
	private:
		eSTATE_UPDATE m_state;
#if defined(_WIN32)
		CRITICAL_SECTION m_cs;
		CONDITION_VARIABLE m_cv;
#elif defined(__linux__)
		pthread_mutex_t m_cs;
		pthread_cond_t m_cv;
#endif
	};

	struct stPlayerLocationDB : public stSyncUpdateDB {
		stPlayerLocationDB(uint32_t _ul = 0u) : stSyncUpdateDB() {

			clear();
		};
		~stPlayerLocationDB() {

			clear();
		};
		void clear() {

			channel = -1;
			lobby = -1;
			room = -1;
			place = 0u;
		};
		char channel;
		char lobby;
		short room;
		unsigned char place;
	};

#if defined(__linux__)
#pragma pack(1)
#endif

	struct CPLog {
		public:
			enum TYPE : unsigned char {
				BUY_SHOP,
				GIFT_SHOP,
				TICKER,
				CP_POUCH,
			};

			struct stItem {
				stItem(uint32_t _ul = 0u) {
					clear();
				};
				stItem(uint32_t __typeid, uint32_t _qntd, uint64_t _cp)
					: _typeid(__typeid), qntd(_qntd), price(_cp) {

				};
				void clear() {
					memset(this, 0, sizeof(stItem));
				};
				uint32_t _typeid;
				uint32_t qntd;
				uint64_t price;
			};

		public:
			CPLog(uint32_t _ul = 0u) {
				clear();
			};
			~CPLog() {};

			void clear() {

				m_type = BUY_SHOP;
				m_mail_id = -1;
				m_cookie = 0ull;

				if (!v_item.empty()) {
					v_item.clear();
					v_item.shrink_to_fit();
				}
			};

			TYPE getType() {
				return m_type;
			};

			void setType(TYPE _type) {
				m_type = _type;
			};

			int32_t getMailId() {
				return m_mail_id;
			};

			void setMailId(int32_t _mail_id) {
				m_mail_id = _mail_id;
			};

			uint64_t getCookie() {

				uint64_t total = m_cookie;

				std::for_each(v_item.begin(), v_item.end(), [&](auto& _el) {
					total += _el.price;
				});

				return total;
			};

			void setCookie(uint64_t _cp) {
				m_cookie = _cp;
			};

			uint32_t getItemCount() {
				return (uint32_t)v_item.size();
			};

			std::vector< stItem >& getItens() {
				return v_item;
			};

			void putItem(uint32_t _typeid, uint32_t _qntd, uint64_t _cp) {
				v_item.push_back({ _typeid, _qntd, _cp });
			};

			void putItem(stItem _item) {
				v_item.push_back(_item);
			};

			std::string toString() {

				std::string str = "TYPE=" + std::to_string((unsigned short)m_type)
						+ ", mail_id=" + std::to_string(m_mail_id)
						+ ", cookie=" + std::to_string(getCookie())
						+ ", item(ns) quantidade=" + std::to_string(v_item.size());

				for (auto& el : v_item)
					str = ", {TYPEID=" + std::to_string(el._typeid) + ", QNTD=" + std::to_string(el.qntd) + ", PRICE=" + std::to_string(el.price) + "}";

				return str;
			};

		protected:
			TYPE m_type;
			int32_t m_mail_id;
			uint64_t m_cookie;
			std::vector< stItem > v_item;
	};

	union uMemberInfoStateFlag {
		void clear() { memset(this, 0, sizeof(uMemberInfoStateFlag)); };
		unsigned char ucByte;
		struct {
			unsigned char channel : 1;
			unsigned char visible : 1;
			unsigned char whisper : 1;
			unsigned char sexo : 1;
			unsigned char azinha : 1;
			unsigned char icon_angel : 1;
			unsigned char quiter_1 : 1;
			unsigned char quiter_2 : 1;
		}stFlagBit;
	};

	struct PlayerPapelShopInfo {
		void clear() { memset(this, 0, sizeof(PlayerPapelShopInfo)); };
		short remain_count;
		short current_count;
		short limit_count;
	};

	union uCapability {
		uCapability(uint32_t _ul = 0u) : ulCapability(_ul) {};
		void clear() { ulCapability = 0u; };
		uint32_t ulCapability;
		struct {
			uint32_t A_I_MODE : 1;
			uint32_t : 1;
			uint32_t game_master : 1;
			uint32_t gm_edit_site : 1;
			uint32_t block_give_item_gm : 1;
			uint32_t : 2;
			uint32_t gm_normal : 1;
			uint32_t : 2;
			uint32_t mantle : 1;
			uint32_t : 3;
			uint32_t premium_user : 1;
			uint32_t title_gm : 1, : 0;
		}stBit;
	};

	struct MemberInfo {
		MemberInfo() {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(MemberInfo));
			oid = -1;
		};
		char id[22];
		char nick_name[22];
		char guild_name[17];
		char guild_mark_img[12];
		unsigned char ucUnknown35[35];
		uint32_t ulUnknown;

		uCapability capability;
		uint32_t ulUnknown2;
		int32_t  oid;
		uint32_t ulUnknown3;
		uint64_t ullUnknown;
		unsigned guild_uid;
		uint32_t guild_mark_img_no;
		uMemberInfoStateFlag state_flag;
		unsigned short flag_login_time;
		PlayerPapelShopInfo papel_shop;
		unsigned char ucUnknown16[16];
		char id_NT[22];
		unsigned char ucUnknown107[106];
	};

	struct MemberInfoEx : public MemberInfo {
		MemberInfoEx() : MemberInfo() {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(MemberInfoEx));
			sala_numero = -1;
			oid = -1;
		};
		int32_t uid;
		int guild_point;
		int64_t guild_pang;
		__int16 sala_numero;
		__int8 sexo;
		__int8 level;
		__int8 do_tutorial;
		__int8 event_1;
		__int8 event_2;
		int school;
		int manner_flag;
		SYSTEMTIME papel_shop_last_update;
	};

	union uMedalWin {
		void clear() { memset(this, 0, sizeof(uMedalWin)); };
		unsigned char ucMedal;
		struct _stMedal {
			unsigned char lucky : 1;
			unsigned char speediest : 1;
			unsigned char best_drive : 1;
			unsigned char best_chipin : 1;
			unsigned char best_long_puttin : 1;
			unsigned char best_recovery : 1, : 0;
		}stMedal;
	};

	struct stMedal {
		void clear() { memset(this, 0, sizeof(stMedal)); };
		void add(stMedal& _medal) {

			lucky += _medal.lucky;
			fast += _medal.fast;
			best_drive += _medal.best_drive;
			best_chipin += _medal.best_chipin;
			best_puttin += _medal.best_puttin;
			best_recovery += _medal.best_recovery;

		};
		void add(uMedalWin _medal_win) {

			if (_medal_win.stMedal.lucky)
				lucky++;
			else if (_medal_win.stMedal.speediest)
				fast++;
			else if (_medal_win.stMedal.best_drive)
				best_drive++;
			else if (_medal_win.stMedal.best_chipin)
				best_chipin++;
			else if (_medal_win.stMedal.best_long_puttin)
				best_puttin++;
			else if (_medal_win.stMedal.best_recovery)
				best_recovery++;

		};
		uint32_t lucky;
		uint32_t fast;
		uint32_t best_drive;
		uint32_t best_chipin;
		uint32_t best_puttin;
		uint32_t best_recovery;
	};

	struct UserInfo {
		UserInfo(uint32_t _ul = 0u) {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(UserInfo));
		};
		void add(UserInfo& _ui) {

			if (_ui.best_drive > best_drive)
				best_drive = _ui.best_drive;

			if (_ui.best_long_putt > best_long_putt)
				best_long_putt = _ui.best_long_putt;

			if (_ui.best_chip_in > best_chip_in)
				best_chip_in = _ui.best_chip_in;

			if (_ui.combo < 0) {

				if (combo <= DECREASE_COMBO_VALUE)
					combo = 0;
				else
					combo += _ui.combo;

			}else {

				combo += _ui.combo;

				if (combo > all_combo)
					all_combo += _ui.combo;
			}

			if (_ui.quitado < 0) {

				if ((quitado + _ui.quitado) <= 0)
					quitado = 0;
				else
					quitado += _ui.quitado;

			}else
				quitado += _ui.quitado;

			if ((skin_all_in_count + _ui.skin_all_in_count) >= 5) {

				skin_all_in_count = 0l;
				skin_pang += 1000;

			}else
				skin_all_in_count += _ui.skin_all_in_count;

			tacada += _ui.tacada;
			putt += _ui.putt;
			tempo += _ui.tempo;
			tempo_tacada += _ui.tempo_tacada;
			acerto_pangya += _ui.acerto_pangya;
			timeout += _ui.timeout;
			ob += _ui.ob;
			total_distancia += _ui.total_distancia;
			hole += _ui.hole;
			hole_in += (_ui.hole - _ui.hole_in);
			hio += _ui.hio;
			bunker += _ui.bunker;
			fairway += _ui.fairway;
			albatross += _ui.albatross;
			putt_in += _ui.putt_in;
			media_score += _ui.media_score;
			best_score[0] += _ui.best_score[0];
			best_score[1] += _ui.best_score[1];
			best_score[2] += _ui.best_score[2];
			best_score[3] += _ui.best_score[3];
			best_score[4] += _ui.best_score[4];
			best_pang[0] += _ui.best_pang[0];
			best_pang[1] += _ui.best_pang[1];
			best_pang[2] += _ui.best_pang[2];
			best_pang[3] += _ui.best_pang[3];
			best_pang[4] += _ui.best_pang[4];
			sum_pang += _ui.sum_pang;
			event_flag += _ui.event_flag;
			jogado += _ui.jogado;
			team_game += _ui.team_game;
			team_win += _ui.team_win;
			team_hole += _ui.team_hole;
			ladder_point += _ui.ladder_point;
			ladder_hole += _ui.ladder_hole;
			ladder_win += _ui.ladder_win;
			ladder_lose += _ui.ladder_lose;
			ladder_draw += _ui.ladder_draw;
			skin_pang += _ui.skin_pang;
			skin_win += _ui.skin_win;
			skin_lose += _ui.skin_lose;
			skin_run_hole += _ui.skin_run_hole;

			skin_strike_point += _ui.skin_strike_point;
			disconnect += _ui.disconnect;
			jogados_disconnect += _ui.jogados_disconnect;
			event_value += _ui.event_value;
			sys_school_serie += _ui.sys_school_serie;
			game_count_season += _ui.game_count_season;

			medal.add(_ui.medal);

		};
		int32_t tacada;
		int32_t putt;
		int32_t tempo;
		int32_t tempo_tacada;
		float best_drive;
		int32_t acerto_pangya;
		int32_t timeout;
		int32_t ob;
		int32_t total_distancia;
		int32_t hole;
		int32_t hole_in;
		int32_t hio;
		short bunker;
		int32_t fairway;
		int32_t albatross;
		int32_t mad_conduta;
		int32_t putt_in;
		float best_long_putt;
		float best_chip_in;
		uint32_t exp;
		unsigned char level;
		uint64_t pang;
		int32_t media_score;
		char best_score[5];
		unsigned char event_flag;
		int64_t best_pang[5];
		int64_t sum_pang;
		int32_t jogado;
		int32_t team_hole;
		int32_t team_win;
		int32_t team_game;
		int32_t ladder_point;
		int32_t ladder_hole;
		int32_t ladder_win;
		int32_t ladder_lose;
		int32_t ladder_draw;
		int32_t combo;
		int32_t all_combo;
		int32_t quitado;
		int64_t skin_pang;
		int32_t skin_win;
		int32_t skin_lose;
		int32_t skin_all_in_count;
		int32_t skin_run_hole;
		int32_t skin_strike_point;
		int32_t jogados_disconnect;
		short event_value;
		int32_t disconnect;
		stMedal medal;
		int32_t sys_school_serie;
		int32_t game_count_season;
		short _16bit_nao_sei;
		float getMediaScore() {

			if ((hole - hole_in) == 0)
				return 0.f;

			return (18.f / (hole - hole_in)) * media_score + 72.f;
		};
		float getPangyaShotRate() {

			if (tacada == 0)
				return 0.f;

			return ((float)acerto_pangya / tacada) * 100.f;
		};
		float getFairwayRate() {

			if ((hole - hole_in) == 0)
				return 0.f;

			return ((float)fairway / (hole - hole_in)) * 100.f;
		};
		float getPuttRate() {

			if (putt == 0)
				return 0.f;

			return ((float)putt_in / putt) * 100.f;
		};
		float getOBRate() {

			if ((tacada + putt) == 0)
				return 0.f;

			return ((float)ob / (tacada + putt)) * 100.f;
		};
		float getMatchWinRate() {

			if (team_game == 0)
				return 0.f;

			return ((float)team_win / team_game) * 100.f;
		};
		float getShotTimeRate() {

			if ((tacada + putt) == 0)
				return 0.f;

			return ((float)tempo_tacada / (tacada + putt)) * 100.f;
		};
		float getQuitRate() {

			if (jogado == 0)
				return 0.f;

			return quitado * 100.f / jogado;
		};
		std::string toString() {
			return "Tacada: " + std::to_string(tacada) + "  Putt: " + std::to_string(putt) + "  Tempo: " + std::to_string(tempo) + "  Tempo Tacada: " + std::to_string(tempo_tacada)
				+ "  Best drive: " + std::to_string(best_drive) + "  Acerto pangya: " + std::to_string(acerto_pangya) + "  timeout: " + std::to_string(timeout)
				+ "  OB: " + std::to_string(ob) + "  Total distancia: " + std::to_string(total_distancia) + "  hole: " + std::to_string(hole)
				+ "  Hole in: " + std::to_string(hole_in) + "  HIO: " + std::to_string(hio) + "  Bunker: " + std::to_string(bunker) + "  Fairway: " + std::to_string(fairway)
				+ "  Albratross: " + std::to_string(albatross) + "  Mad conduta: " + std::to_string(mad_conduta) + "  Putt in: " + std::to_string(putt_in)
				+ "  Best long puttin: " + std::to_string(best_long_putt) + "  Best chipin: " + std::to_string(best_chip_in) + "  Exp: " + std::to_string(exp)
				+ "  Level: " + std::to_string((unsigned short)level) + "  Pang: " + std::to_string(pang) + "  Media score: " + std::to_string(media_score)
				+ "  Best score[" + std::to_string((unsigned short)best_score[0]) + ", " + std::to_string((unsigned short)best_score[1]) + ", " + std::to_string((unsigned short)best_score[2])
				+ ", " + std::to_string((unsigned short)best_score[3]) + ", " + std::to_string((unsigned short)best_score[4]) + "]  Event flag: " + std::to_string((unsigned short)event_flag)
				+ "  Best pang[" + std::to_string(best_pang[0]) + ", " + std::to_string(best_pang[1]) + ", " + std::to_string(best_pang[2]) + ", " + std::to_string(best_pang[3])
				+ ", " + std::to_string(best_pang[4]) + "]  Soma pang: " + std::to_string(sum_pang) + "  Jogado: " + std::to_string(jogado) + "  Team Hole: " + std::to_string(team_hole)
				+ "  Team win: " + std::to_string(team_win) + "  Team game: " + std::to_string(team_game) + "  Ladder point: " + std::to_string(ladder_point)
				+ "  Ladder hole: " + std::to_string(ladder_hole) + "  Ladder win: " + std::to_string(ladder_win) + "  Ladder lose: " + std::to_string(ladder_lose)
				+ "  Ladder draw: " + std::to_string(ladder_draw) + "  Combo: " + std::to_string(combo) + "  All combo: " + std::to_string(all_combo)
				+ "  Quitado: " + std::to_string(quitado) + "  Skin pang: " + std::to_string(skin_pang) + "  Skin win: " + std::to_string(skin_win)
				+ "  Skin lose: " + std::to_string(skin_lose) + "  Skin all in count: " + std::to_string(skin_all_in_count) + "  Skin run hole: " + std::to_string(skin_run_hole)
				+ "  Disconnect(MY): " + std::to_string(disconnect) + "  Jogados Disconnect(MY): " + std::to_string(jogados_disconnect) + "  Event value: " + std::to_string(event_value)
				+ "  Skin Strike Point: " + std::to_string(skin_strike_point) + "  Sistema School Serie: " + std::to_string(sys_school_serie)
				+ "  Game count season: " + std::to_string(game_count_season) + "  _16bit nao sei: " + std::to_string(_16bit_nao_sei);
		};
	};

	struct UserInfoEx : public UserInfo {
		UserInfoEx(uint32_t _ul = 0u) : UserInfo() {
			clear();
		};
		void clear() { memset(this, 0, sizeof(UserInfoEx)); };
		void add(UserInfoEx& _ui, uint64_t _total_pang_win_game = 0ull) {

			UserInfo::add(_ui);

			if (_total_pang_win_game > 0)
				total_pang_win_game += _total_pang_win_game;

		};
		uint64_t total_pang_win_game;
	};

	struct TrofelInfo {
		void clear() {
			memset(this, 0, sizeof(TrofelInfo));
		};
		void update(uint32_t _type, unsigned char _rank) {

			if (_type > 12)
				throw exception("[TrofelInfo::update][Error] _type[VALUE=" + std::to_string(_type) + "] is invalid", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANGYA_GAME_ST, 200, 0));

			if (_rank == 0u || _rank > 3)
				throw exception("[TrofelInfo::update][Error] _rank[VALUE=" + std::to_string((unsigned short)_rank) + "] is invalid", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANGYA_GAME_ST, 201, 0));

			if (_type < 6) {

				ama_6_a_1[_type][_rank - 1]++;

			}else {

				pro_1_a_7[_type - 6][_rank - 1]++;
			}

		};
		uint32_t getSumGold() {

			uint32_t gold_sum = 0u;

			for (auto& el : ama_6_a_1)
				gold_sum += el[0];

			for (auto& el : pro_1_a_7)
				gold_sum += el[0];

			return gold_sum;
		};
		uint32_t getSumSilver() {

			uint32_t silver_sum = 0u;

			for (auto& el : ama_6_a_1)
				silver_sum += el[1];

			for (auto& el : pro_1_a_7)
				silver_sum += el[1];

			return silver_sum;
		};
		uint32_t getSumBronze() {

			uint32_t bronze_sum = 0u;

			for (auto& el : ama_6_a_1)
				bronze_sum += el[2];

			for (auto& el : pro_1_a_7)
				bronze_sum += el[2];

			return bronze_sum;
		};
		short ama_6_a_1[6][3];
		short pro_1_a_7[7][3];

	};

	struct TrofelEspecialInfo {
		void clear() {
			memset(this, 0, sizeof(TrofelEspecialInfo));
		};
		int32_t id;
		int32_t _typeid;
		int32_t qntd;
	};

	struct UserEquip {
		void clear() {
			memset(this, 0, sizeof(UserEquip));
		};
		int32_t caddie_id;
		int32_t character_id;
		int32_t clubset_id;
		int32_t ball_typeid;
		int32_t item_slot[10];
		int32_t skin_id[6];
		int32_t skin_typeid[6];
		int32_t mascot_id;
		int32_t poster[2];
#define m_title skin_typeid[5]
	};

	struct MapStatistics {
		MapStatistics(uint32_t _ul = 0u) {
			clear();
		};
		void clear(unsigned char _course = 0) {
			memset(this, 0, sizeof(MapStatistics));
			best_score = 127;
			course = _course;
		};
		unsigned char isRecorded() {

			return (best_score != 127 ? 1 : 0);
		};
		unsigned char course;
		int32_t tacada;
		int32_t putt;
		int32_t hole;
		int32_t fairway;
		int32_t hole_in;
		int32_t putt_in;
		int32_t total_score;
		char best_score;
		int64_t best_pang;
		int32_t character_typeid;
		unsigned char event_score;
	};

	struct MapStatisticsEx : public MapStatistics {
		MapStatisticsEx(uint32_t _ul = 0u) : MapStatistics() {
			clear();
		};
		MapStatisticsEx(MapStatisticsEx& _cpy) {
			*this = _cpy;
		};
		MapStatisticsEx(MapStatistics& _cpy) : MapStatistics(_cpy) {
			tipo = 0u;
		};
		void clear(unsigned char _course = 0) {
			memset(this, 0, sizeof(MapStatisticsEx));
			best_score = 127;
			course = _course;
		};
		unsigned char tipo;
	};

	struct CaddieInfo {
		CaddieInfo(uint32_t _ul = 0u) {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(CaddieInfo));
		};
		int32_t id;
		uint32_t _typeid;
		uint32_t parts_typeid;
		unsigned char level;
		uint32_t exp;
		unsigned char rent_flag;
		unsigned short end_date_unix;
		unsigned short parts_end_date_unix;
		unsigned char purchase;
		short check_end;
	};

	struct CaddieInfoEx : public CaddieInfo {
		CaddieInfoEx(uint32_t _ul = 0u) : CaddieInfo(_ul) {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(CaddieInfoEx));
		};
		inline void updatePartsEndDate() {

			int64_t diff_end_parts_date = isEmpty(end_parts_date) ? 0ll : getLocalTimeDiffDESC(end_parts_date);

			if (diff_end_parts_date <= 0) {

				parts_end_date_unix = 0u;

				if (parts_typeid > 0) {

					parts_typeid = 0u;

					need_update = 1u;
				}

			}else
				parts_end_date_unix = ((diff_end_parts_date /= STDA_10_MICRO_PER_HOUR) == 0 ? 1  : (unsigned short)diff_end_parts_date);

		};
		inline void updateEndDate() {

			int64_t diff_end_date = isEmpty(end_date) ? 0ll : getLocalTimeDiffDESC(end_date);;

			if (diff_end_date <= 0)
				end_date_unix = 0u;
			else
				end_date_unix = ((diff_end_date /= STDA_10_MICRO_PER_DAY) == 0 ? 1  : (unsigned short)diff_end_date);

		};
		CaddieInfo* getInfo() {

			updateEndDate();

			updatePartsEndDate();

			return this;
		};
		SYSTEMTIME end_date;
		SYSTEMTIME end_parts_date;
		unsigned char need_update : 1;
	};

	struct ClubSetInfo {
		ClubSetInfo() {
			clear();
		};
		ClubSetInfo(int32_t _id, int32_t __typeid, short *_slot_c) {
			clear();

			id = _id;
			_typeid = __typeid;

			if (_slot_c != nullptr)
#if defined(_WIN32)
				memcpy_s(slot_c, sizeof(slot_c), _slot_c, sizeof(slot_c));
#elif defined(__linux__)
				memcpy(slot_c, _slot_c, sizeof(slot_c));
#endif
		};
		void clear() {
			memset(this, 0, sizeof(ClubSetInfo));
		};
		int32_t id;
		int32_t _typeid;
		short slot_c[5];
		short enchant_c[5];
	};

	struct MascotInfo {
		MascotInfo(uint32_t _ul = 0u) {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(MascotInfo));
		};
		int32_t id;
		uint32_t _typeid;
		unsigned char level;
		uint32_t exp;
		char message[30];
		short tipo;
		SYSTEMTIME data;
		unsigned char flag;
	};

	struct MascotInfoEx : public MascotInfo {
		MascotInfoEx(uint32_t _ul = 0u) : MascotInfo(_ul) {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(MascotInfoEx));
		};
		bool checkUpdate() {

			if (getLocalTimeDiffDESC(data) <= 0)
				need_update = 1;

			return (need_update == 1);
		}
		unsigned char is_cash;
		uint32_t price;
		unsigned char need_update : 1;
	};

	struct WarehouseItem {
		WarehouseItem(uint32_t _ul = 0u) {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(WarehouseItem));
		};
		int32_t id;
		int32_t _typeid;
		int32_t ano;
		short c[5];
		unsigned char purchase;
		unsigned char flag;
		int64_t apply_date;
		int64_t end_date;
		unsigned char type;
		struct UCC {
			void clear() { memset(this, 0, sizeof(UCC)); };
			char name[40];
			unsigned char trade;
			char idx[9];

			unsigned char status;
			unsigned short seq;
			char copier_nick[22];
			int32_t copier;
		};
		UCC ucc;
		struct Card {
			void clear() { memset(this, 0, sizeof(Card)); };
			int32_t character[4];
			int32_t caddie[4];
			int32_t NPC[4];
		};
		Card card;
		struct ClubsetWorkshop {
			void clear() { memset(this, 0, sizeof(ClubsetWorkshop)); };
			short flag;
			short c[5];
			int32_t mastery;
			int32_t recovery_pts;
			int32_t level;
			int32_t rank;
			int32_t calcRank(short* _c) {
				int32_t total = c[0] + _c[0] + c[1] + _c[1] + c[2] + _c[2] + c[3] + _c[3] + c[4] + _c[4];

				if (total >= 30 && total < 60)
					return (total - 30) / 5;

				return -1;
			};
			int32_t calcLevel(short* _c) {
				int32_t total = c[0] + _c[0] + c[1] + _c[1] + c[2] + _c[2] + c[3] + _c[3] + c[4] + _c[4];

				if (total >= 30 && total < 60)
					return (total - 30) % 5;

				return -1;
			};
			static int32_t s_calcRank(short* _c) {
				int32_t total = _c[0] + _c[1] + _c[2] + _c[3] + _c[4];

				if (total >= 30 && total < 60)
					return (total - 30) / 5;

				return -1;
			};
			static int32_t s_calcLevel(short* _c) {
				int32_t total = _c[0] + _c[1] + _c[2] + _c[3] + _c[4];

				if (total >= 30 && total < 60)
					return (total - 30) % 5;

				return -1;
			};
		};
		ClubsetWorkshop clubset_workshop;
	};

	struct WarehouseItemEx: public WarehouseItem {
		WarehouseItemEx(uint32_t _ul = 0u) : WarehouseItem() {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(WarehouseItemEx));
		};

		uint32_t apply_date_unix_local;
		uint32_t end_date_unix_local;
	};

	struct ClubSetWorkshopLasUpLevel {
		void clear() { memset(this, 0, sizeof(ClubSetWorkshopLasUpLevel)); };
		int32_t clubset_id;
		uint32_t stat;
	};

	struct ClubSetWorkshopTransformClubSet {
		void clear() { memset(this, 0, sizeof(ClubSetWorkshopTransformClubSet)); };
		int32_t clubset_id;
		uint32_t stat;
		uint32_t transform_typeid;
	};

	struct TradeItem {
		void clear() { memset(this, 0, sizeof(TradeItem)); };
		uint32_t _typeid;
		int32_t id;
		uint32_t qntd;
		unsigned char ucUnknown3[3];
		uint64_t pang;
		uint32_t upgrade_custo;
		unsigned short c[5];
		unsigned short usUnknown;
		char sd_idx[9];
		unsigned short sd_seq;
		unsigned char sd_status;
		struct Card {
			void clear() { memset(this, 0, sizeof(Card)); };
			uint32_t character[4];
			uint32_t caddie[4];
			uint32_t NPC[4];
			unsigned short character_slot_count;
			unsigned short caddie_slot_count;
			unsigned short NPC_slot_count;
		};
		Card card;
		char sd_name[41];
		char sd_copier_nick[22];
	};

	struct DolfiniLockerItem {
		void clear() { memset(this, 0, sizeof(DolfiniLockerItem)); };
		uint64_t index;
		TradeItem item;
	};

	struct PersonalShopItem {
		void clear() { memset(this, 0, sizeof(PersonalShopItem)); };
		uint32_t index;
		TradeItem item;
	};

	struct TutorialInfo {
		void clear() {
			memset(this, 0, sizeof(TutorialInfo));
		};
		uint32_t getTutoAll() {
			return rookie | beginner | advancer;
		};
		uint32_t rookie;
		uint32_t beginner;
		uint32_t advancer;
	};

	struct CardInfo {
		void clear() {
			memset(this, 0, sizeof(CardInfo));
		};
		int32_t id;
		int32_t _typeid;
		int32_t slot;
		int32_t efeito;
		int32_t efeito_qntd;
		int32_t qntd;
		SYSTEMTIME use_date;
		SYSTEMTIME end_date;
		unsigned char type;
		unsigned char use_yn;
	};

	struct CardEquipInfo {
		void clear() {
			memset(this, 0, sizeof(CardEquipInfo));
		};
		int32_t id;
		int32_t _typeid;
		int32_t parts_typeid;
		int32_t parts_id;
		int32_t efeito;
		int32_t efeito_qntd;
		int32_t slot;
		SYSTEMTIME use_date;
		SYSTEMTIME end_date;
		int32_t tipo;
		unsigned char use_yn;
	};

	struct CardEquipInfoEx : public CardEquipInfo {
		CardEquipInfoEx(uint32_t _ul = 0u) {
			clear();
		};
		void clear() { memset(this, 0, sizeof(CardEquipInfoEx)); };
		int64_t index;
	};

	struct time32 {
		void clear() {
			memset(this, 0, sizeof(time32));
		};
		void setTime(uint32_t time) {
			high_time = (unsigned short)(time / 0xFFFF);
			low_time = time % 0xFFFF;
		};
		uint32_t getTime() {
			return (uint32_t)((high_time * 0xFFFF) | low_time);
		};
	private:
		unsigned short high_time;
		unsigned short low_time;
	};

	struct ItemBuff {
		enum eTYPE : uint32_t {
			NONE,
			YAM_AND_GOLD,
			RAINBOW,
			RED,
			GREEN,
			YELLOW,
		};
		void clear() {
			memset(this, 0, sizeof(ItemBuff));
		};
		int32_t id;
		int32_t _typeid;
		int32_t parts_typeid;
		int32_t parts_id;
		int32_t efeito;
		int32_t efeito_qntd;
		int32_t slot;
		SYSTEMTIME use_date;
		unsigned char ucUnknown12[12];
		time32 tempo;
		int32_t tipo;
		unsigned char use_yn;
	};

	struct ItemBuffEx : public ItemBuff {
		ItemBuffEx(uint32_t _ul = 0u) { clear(); };
		void clear() { memset(this, 0, sizeof(ItemBuffEx)); };
		int64_t index;
		SYSTEMTIME end_date;
		uint32_t percent;
	};

	struct GuildInfo {
		GuildInfo(uint32_t _ul = 0u) {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(GuildInfo));
			uid = 0;
		};
		uint32_t uid;
		unsigned char leadder;
		char name[32];
		uint32_t index_mark_emblem;
		uint64_t ull_unknown;
		uint64_t pang;
		char _16unknown[16];
		uint32_t point;
	};

	struct GuildInfoEx : public GuildInfo {
		GuildInfoEx(uint32_t _ul = 0u) : GuildInfo(_ul) {
			clear();
		};
		void clear() {

			GuildInfo::clear();

			memset(mark_emblem, sizeof(mark_emblem), 0);
		};
		char mark_emblem[12];
	};

#define MS_NUM_MAPS 22

	struct TreasureHunterInfo {
		void clear() {
			memset(this, 0, sizeof(TreasureHunterInfo));
		};
		unsigned char course;
		uint32_t point;
	};

	struct TreasureHunterItem {
		void clear() { memset(this, 0, sizeof(TreasureHunterItem)); };
		uint32_t _typeid;
		uint32_t qntd;
		uint32_t probabilidade;
		unsigned char flag;
		unsigned char active : 1;
	};

	struct CounterItemInfo {
		CounterItemInfo(uint32_t _ul = 0u) {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(CounterItemInfo));
		};
		bool isValid() { return (id > 0 && _typeid != 0); };
		unsigned char active;
		uint32_t _typeid;
		int32_t id;
		int32_t value;
	};

	struct QuestStuffInfo {
		void clear() {
			memset(this, 0, sizeof(QuestStuffInfo));
		};
		bool isValid() { return (id > 0 && _typeid != 0); };
		int32_t id;
		uint32_t _typeid;
		int32_t counter_item_id;
		uint32_t clear_date_unix;
	};

	struct AchievementInfo {
		enum ACHIEVEMENT_STATUS : unsigned char {
			PENDENTING = 1,
			EXCLUEDED,
			ACTIVED,
			CONCLUEDED,
		};

		~AchievementInfo() {};
		void clear() {
			active = 0;
			_typeid = 0;
			id = 0;
			status = 0;

			if (!v_qsi.empty()) {
				v_qsi.clear();
				v_qsi.shrink_to_fit();
			}

			if (!map_counter_item.empty())
				map_counter_item.clear();
		};
		CounterItemInfo* findCounterItemById(int32_t _id) {
			if (_id < 0)
				throw exception("[AchievementInfo::findCounterItemById][Error] _id is invalid", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANGYA_GAME_ST, 50, 0));

			auto it = map_counter_item.end();

			if ((it = map_counter_item.find(_id)) != map_counter_item.end())
				return &it->second;

			return nullptr;
		};
		CounterItemInfo* findCounterItemByTypeId(uint32_t _typeid) {
			if (_typeid == 0)
				throw exception("[AchievementInfo::findCounterItemByTypeid][Error] _typeid is invalid", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANGYA_GAME_ST, 50, 0));

			auto it = map_counter_item.end();

			if ((it = VECTOR_FIND_ITEM(map_counter_item, second._typeid, == , _typeid)) != map_counter_item.end())
				return &it->second;

			return nullptr;
		};
		QuestStuffInfo* findQuestStuffById(int32_t _id) {
			if (_id < 0)
				throw exception("[AchievementInfo::findQuestStuffById][Error] _id is invalid", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANGYA_GAME_ST, 50, 0));

			auto it = v_qsi.end();

			if ((it = VECTOR_FIND_ITEM(v_qsi, id, == , _id)) != v_qsi.end())
#if defined(_WIN32)
				return it._Ptr;
#elif defined(__linux__)
				return &(*it);
#endif

			return nullptr;
		};
		QuestStuffInfo* findQuestStuffByTypeId(uint32_t _typeid) {
			if (_typeid == 0)
				throw exception("[AchievementInfo::findQuestStuffByTypeId][Error] _typeid is invalid", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANGYA_GAME_ST, 50, 0));

			auto it = v_qsi.end();

			if ((it = VECTOR_FIND_ITEM(v_qsi, _typeid, == , _typeid)) != v_qsi.end())
#if defined(_WIN32)
				return it._Ptr;
#elif defined(__linux__)
				return &(*it);
#endif

			return nullptr;
		};
		uint32_t addCounterByTypeId(uint32_t _typeid, int32_t _value) {
			if (_typeid == 0)
				throw exception("[AchievementInfo::addCounterByTypeId][Error] _typeid is invalid", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANGYA_GAME_ST, 50, 0));

			uint32_t count = 0u;
			std::map< int32_t, CounterItemInfo* > map_cii;

			std::for_each(v_qsi.begin(), v_qsi.end(), [&](auto& el) {

				if (el.clear_date_unix == 0) {
					CounterItemInfo *cii = nullptr;

					if ((cii = findCounterItemById(el.counter_item_id)) != nullptr && cii->_typeid == _typeid)
						map_cii[cii->id] = cii;
				}
			});

			for (auto& it : map_cii) {
				it.second->value += _value;
				++count;
			}

			return count;
		};
		bool checkAllQuestClear() {

			auto size = v_qsi.size(), count = (size_t)0;

			for (auto& el : v_qsi)
				if (el.clear_date_unix != 0)
					++count;

			return count == size;
		};
		unsigned char active;
		uint32_t _typeid;
		int32_t id;
		int32_t status;
		std::map< int32_t, CounterItemInfo > map_counter_item;
		std::vector< QuestStuffInfo > v_qsi;
	};

	struct AchievementInfoEx : AchievementInfo {
		AchievementInfoEx() : AchievementInfo() {
			clear();
		};
		void clear() {
			AchievementInfo::clear();
			quest_base_typeid = 0;
		};

		uint32_t quest_base_typeid;
		std::vector< QuestStuffInfo >::iterator getQuestBase() {

			if (quest_base_typeid == 0)
				return v_qsi.end();

			return VECTOR_FIND_ITEM(v_qsi, _typeid, == , quest_base_typeid);
		};
	};

	struct CouponGacha {
		void clear() {
			memset(this, 0, sizeof(CouponGacha));
		};
		int32_t partial_ticket;
		int32_t normal_ticket;
	};

	struct PremiumTicket {
		void clear() {
			memset(this, 0, sizeof(PremiumTicket));
		};
		int32_t id;
		int32_t _typeid;
		int32_t unix_sec_date;
		int32_t unix_end_date;
	};

	struct RequestInfo {
		void clear() {
			memset(this, 0, sizeof(RequestInfo));
		};
		uint32_t uid;
		unsigned char season;
		unsigned char show;
	};

	struct EquipedItem {
		void clear() { memset(this, 0, sizeof(EquipedItem)); };
		CharacterInfo *char_info;
		CaddieInfoEx *cad_info;
		MascotInfoEx *mascot_info;
		ClubSetInfo csi;
		WarehouseItem *comet;
		WarehouseItem *clubset;
	};

	struct StateCharacterLounge {
		StateCharacterLounge() { clear(); };
		void clear() {
			camera_zoom = 1.f;
			scale_head = 1.f;
			walk_speed = 1.f;
			fUnknown = 1.f;
		};
		float camera_zoom;
		float scale_head;
		float walk_speed;
		float fUnknown;
	};

	struct MyRoomConfig {
		void clear() { memset(this, 0, sizeof(MyRoomConfig)); };
		unsigned short allow_enter;
		unsigned char public_lock;
		char pass[15];
		unsigned char ucUnknown90[90];
	};

	struct MyRoomItem {
		void clear() { memset(this, 0, sizeof(MyRoomItem)); };
		int32_t id;
		uint32_t _typeid;
		unsigned short number;
		struct Location {
			void clear() { memset(this, 0, sizeof(Location)); };
			float x;
			float y;
			float z;
			float r;
		};
		Location location;
		unsigned char equiped : 1, : 0;
	};

	struct DolfiniLocker {
		DolfiniLocker() { clear();  };
		~DolfiniLocker() {};
		void clear() {
			memset(pass, 0, sizeof(pass));
			pang = 0ull;
			locker = 0;
			pass_check = 0;

			if (!v_item.empty()) {
				v_item.clear();
				v_item.shrink_to_fit();
			}
		};
		uint32_t isLocker() {

			if (pass[0] == '\0')
				return 2;
			else if (!locker && pass_check)
				return 76;

			return 76;
		};
		bool ownerItem(uint32_t _typeid) {

			auto it = std::find_if(v_item.begin(), v_item.end(), [&](auto& _el) {
				return (_el.item._typeid == _typeid);
			});

			return (it != v_item.end() ? true : false);
		};
		char pass[7];
		uint64_t pang;
		unsigned char locker;
		unsigned char pass_check : 1, : 0;
		std::vector< DolfiniLockerItem > v_item;
	};

	struct stItem216 {
		void clear() { memset(this, 0, sizeof(stItem216)); };
		unsigned char type;
		uint32_t _typeid;
		int32_t id;
		uint32_t flag_time;
		uint32_t qntd_ant;
		uint32_t qntd_dep;
		uint32_t qntd;
		unsigned short c[5];
		unsigned char ucc_idx[9];
		unsigned char seq;
		uint32_t card_typeid;
		unsigned char card_slot;
	};

	struct stItem {
		void clear() { memset(this, 0, sizeof(stItem)); };
		int32_t id;
		uint32_t _typeid;

		unsigned char type_iff;
		unsigned char type;
		unsigned char flag;
		unsigned char flag_time;
		int32_t qntd;

		char name[64];
		char icon[41];

		struct item_stat {
			void clear() { memset(this, 0, sizeof(item_stat)); };
			int32_t qntd_ant;
			int32_t qntd_dep;
		};

		item_stat stat;

		struct UCC {
			void clear() { memset(this, 0, sizeof(UCC)); };
			char IDX[9];
			uint32_t status;
			uint32_t seq;
		};

		UCC ucc;

		unsigned char is_cash : 1, : 0;
		uint32_t price;
		uint32_t desconto;

		struct stDate {
			void clear() { memset(this, 0, sizeof(stDate)); };
			uint32_t active : 1, : 0;
			struct stDateSys {
				void clear() { memset(this, 0, sizeof(stDateSys)); };
				SYSTEMTIME sysDate[2];
			}date;
		};

		stDate date;
		unsigned short date_reserve;

		short c[5];
#define STDA_C_ITEM_QNTD c[0]
#define STDA_C_ITEM_TICKET_REPORT_ID_HIGH c[1]
#define STDA_C_ITEM_TICKET_REPORT_ID_LOW c[2]
#define STDA_C_ITEM_TIME c[3]
	};

	struct stItemEx : public stItem {
		stItemEx(uint32_t _ul = 0u) {
			clear();
		}
		void clear() { memset(this, 0, sizeof(stItemEx)); };
		struct ClubSetWorkshop {
			void clear() { memset(this, 0, sizeof(ClubSetWorkshop)); };
			unsigned short c[5];
			uint32_t mastery;
			char level;
			uint32_t rank;
			uint32_t recovery;
		};
		ClubSetWorkshop clubset_workshop;
	};

	struct Location {
		void clear() { memset(this, 0, sizeof(Location)); };
		double diffXZ(Location& _l) {
			return sqrt(pow(x - _l.x, 2) + pow(z - _l.z, 2));
		};
		static double diffXZ(Location& _l1, Location& _l2) {
			return sqrt(pow(_l1.x - _l2.x, 2) + pow(_l1.z - _l2.z, 2));
		};
		double diff(Location& _l) {
			return sqrt(pow(x - _l.x, 2) + pow(y - _l.y, 2) + pow(z - _l.z, 2));
		};
		static double diff(Location& _l1, Location& _l2) {
			return sqrt(pow(_l1.x - _l2.x, 2) + pow(_l1.y - _l2.y, 2) + pow(_l1.z - _l2.z, 2));
		};
		std::string toString() {
			return "X: " + std::to_string(x) + " Y: " + std::to_string(y) + " Z: " + std::to_string(z) + " R: " + std::to_string(r);
		};
		float x;
		float y;
		float z;
		float r;
	};

	struct ChannelInfo {

		union uFlag {
			uFlag(uint32_t _ul = 0u) : ulFlag(_ul) {};
			void clear() { ulFlag = 0u; };
			uint32_t ulFlag;
			struct {
				uint32_t : 9;
				uint32_t junior_bellow : 1;
				uint32_t junior_above : 1;
				uint32_t only_rookie : 1;
				uint32_t beginner_between_junior : 1;
				uint32_t junior_between_senior : 1, : 0;
			}stBit;
		};
		void clear() {
			memset(this, 0, sizeof(ChannelInfo));
		};
		char name[64];
		short max_user;
		short curr_user;
		unsigned char id;

		uFlag flag;
		int32_t flag2;
		int32_t min_level_allow;
		int32_t max_level_allow;
	};

	struct ServerInfoEx2 : ServerInfoEx {
		~ServerInfoEx2() {};
		void clear() {
			ServerInfoEx::clear();

			if (!v_ci.empty()) {
				v_ci.clear();
				v_ci.shrink_to_fit();
			}
		};
		std::vector< ChannelInfo > v_ci;
	};

	struct MsgOffInfo {
		void clear() {
			memset(this, 0, sizeof(MsgOffInfo));
		};
		int32_t from_uid;
		short id;
		char nick[22];
		char msg[64];
		char date[17];
	};

	struct AttendanceRewardInfo {
		AttendanceRewardInfo(uint32_t _ul = 0u) {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(AttendanceRewardInfo));
		};
		unsigned char login;
		struct item {
			void clear() {
				memset(this, 0, sizeof(item));
			};
			int32_t _typeid;
			int32_t qntd;
		};
		item now;
		item after;
		int32_t counter;
	};

	struct AttendanceRewardInfoEx : public AttendanceRewardInfo {
		AttendanceRewardInfoEx(uint32_t _ul = 0u) : AttendanceRewardInfo(_ul) {
			clear();
		};
		void clear() { memset(this, 0, sizeof(AttendanceRewardInfoEx)); };
		SYSTEMTIME last_login;
	};

	struct AttendanceRewardItemCtx {
		void clear() { memset(this, 0, sizeof(AttendanceRewardItemCtx)); };
		uint32_t _typeid;
		int32_t qntd;
		unsigned char tipo;
	};

	struct Last5PlayersGame {
		Last5PlayersGame(uint32_t _ul = 0u) {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(Last5PlayersGame));
		};
		struct LastPlayerGame {
			void clear() {
				memset(this, 0, sizeof(LastPlayerGame));
			};
			uint32_t sex;
			char nick[22];
			char id[22];
			uint32_t uid;
		};
		void add(player_info& _pi, uint32_t _sex) {

			if (players[0].uid != _pi.uid) {

				auto it = std::find_if(players, players + 5, [&](auto& _el) {
					return _el.uid == _pi.uid;
				});

				if (it != (players + 5 ))
					std::rotate(it, it + 1, players + 5 );

				std::rotate(players, players + 4, players + 5 );

				players[0].uid = _pi.uid;
#if defined(_WIN32)
				memcpy_s(players[0].id, sizeof(players[0].id), _pi.id, sizeof(players[0].id));
#elif defined(__linux__)
				memcpy(players[0].id, _pi.id, sizeof(players[0].id));
#endif

			}

			players[0].sex = _sex;
#if defined(_WIN32)
			memcpy_s(players[0].nick, sizeof(players[0].nick), _pi.nickname, sizeof(players[0].nick));
#elif defined(__linux__)
			memcpy(players[0].nick, _pi.nickname, sizeof(players[0].nick));
#endif
		};
		LastPlayerGame players[5];
	};

	struct FriendInfo {
		FriendInfo(uint32_t _ul = 0u) {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(FriendInfo));

#if defined(_WIN32)
			memcpy_s(apelido, sizeof(apelido), "Friend", 7);
#elif defined(__linux__)
			memcpy(apelido, "Friend", 7);
#endif
		};
		uint32_t uid;
		unsigned char sex;
		char id[22];
		char nickname[22];
		char apelido[15];
	};

	struct DailyQuestInfo {
		DailyQuestInfo(uint32_t _ul = 0u) {
			clear();
		};
		DailyQuestInfo(uint32_t _typeid_0, uint32_t _typeid_1, uint32_t _typeid_2, SYSTEMTIME& _st)
			: _typeid{ _typeid_0, _typeid_1, _typeid_2 }, date(_st) {
		}
		void clear() { memset(this, 0, sizeof(DailyQuestInfo)); };
		std::string toString() {
			return "QUEST_TYPEID_0=" + std::to_string(_typeid[0]) + ", QUEST_TYPEID_1=" + std::to_string(_typeid[1]) + ", QUEST_TYPEID_2="
					+ std::to_string(_typeid[2]) + ", UPDATE_DATE=" + _formatDate(date);
		};

		SYSTEMTIME date;
		uint32_t _typeid[3];
	};

	struct DailyQuestInfoUser {
		void clear() {
			memset(this, 0, sizeof(DailyQuestInfoUser));
		};
		uint32_t now_date;
		uint32_t accept_date;
		uint32_t current_date;
		uint32_t count;
		uint32_t _typeid[3];
	};

	struct RemoveDailyQuestUser {
		void clear() {
			memset(this, 0, sizeof(RemoveDailyQuestUser));
		};
		int32_t id;
		uint32_t _typeid;
	};

	struct AddDailyQuestUser {
		void clear() { memset(this, 0, sizeof(AddDailyQuestUser)); };
		char name[64];
		uint32_t _typeid;
		uint32_t quest_typeid;
		int status;
	};

	struct PlayerCanalInfo {
		void clear() {
			memset(this, 0, sizeof(PlayerCanalInfo));
		};
		uint32_t uid;
		uint32_t oid;
		short sala_numero;
		char nickname[22];
		unsigned char level;

		uCapability capability;
		int32_t title;
		int32_t team_point;
		union uStateFlag {
			void clear() {
				memset(this, 0, sizeof(uStateFlag));
			};
			unsigned char ucByte;
			struct {
				unsigned char away : 1;
				unsigned char sexo : 1;
				unsigned char quiter_1 : 1;
				unsigned char quiter_2 : 1;
				unsigned char azinha : 1;
				unsigned char icon_angel : 1;
				unsigned char ucUnknown_bit7 : 1;
				unsigned char ucUnknown_bit8 : 1;
			}sBit;
		};
		uStateFlag state_flag;
		uint32_t guid_uid;
		uint32_t guild_index_mark;
		char guild_mark_img[12];
		unsigned short flag_visible_gm;
		int32_t l_unknown;
		char nickNT[22];
		char unknown106[106];
	};

	struct PlayerRoomInfo {
		PlayerRoomInfo() { clear(); };
		void clear() {
			memset(this, 0, sizeof(PlayerRoomInfo));

			state_flag.clear();
		};
		uint32_t oid;
		char nickname[22];
		char guild_name[20];
		unsigned char position;

		uCapability capability;
		uint32_t title;
		uint32_t char_typeid;
		uint32_t skin[6];
		struct StateFlag {
			void clear() {
				memset(this, 0, sizeof(StateFlag));

				uFlag.stFlagBit.ready = 0;
			};
			union {
				unsigned short usFlag;
				unsigned char ucByte[2];
				struct {
					unsigned char team : 1;
					unsigned char team2 : 1;
					unsigned char away : 1;
					unsigned char master : 1;
					unsigned char master2 : 1;
					unsigned char sexo : 1;
					unsigned char quiter_1 : 1;
					unsigned char quiter_2 : 1;
					unsigned char azinha : 1;
					unsigned char ready : 1;
					unsigned char unknown_bit11 : 1;
					unsigned char unknown_bit12 : 1;
					unsigned char unknown_bit13 : 1;
					unsigned char unknown_bit14 : 1;
					unsigned char unknown_bit15 : 1;
					unsigned char unknown_bit16 : 1;
				}stFlagBit;
			}uFlag;
		};
		StateFlag state_flag;
		unsigned char level;
		unsigned char icon_angel;
		unsigned char ucUnknown_0A;
		uint32_t guild_uid;
		char guild_mark_img[12];
		uint32_t guild_mark_index;
		uint32_t uid;
		uint32_t state_lounge;
		unsigned short usUnknown_flg;
		uint32_t state;
		struct stLocation {
			void clear() { memset(this, 0, sizeof(stLocation)); };
			stLocation& operator+=(stLocation& _add_location) {
				x += _add_location.x;
				z += _add_location.z;
				r += _add_location.r;

				return *this;
			};
			float x;
			float z;
			float r;
		};
		stLocation location;
		struct PersonShop {
			void clear() { memset(this, 0, sizeof(PersonShop)); };
			uint32_t active;
			char name[64];
		};
		PersonShop shop;
		union uItemBoost {
			void clear() { memset(this, 0, sizeof(uItemBoost)); };
			unsigned short ulItemBoost;
			struct _stItemBoost {
				unsigned char ucPangMastery : 1, : 0;
				unsigned char ucPangNitro : 1, : 0;
			};
			_stItemBoost stItemBoost;
		};
		uint32_t mascot_typeid;
		uItemBoost flag_item_boost;
		uint32_t ulUnknown_flg;
		unsigned char id_NT[22];
		unsigned char ucUnknown106[106];
		unsigned char convidado : 1, : 0;
		float avg_score;
		unsigned char ucUnknown3[3];
	};

	struct PlayerRoomInfoEx : public PlayerRoomInfo {
		PlayerRoomInfoEx() : PlayerRoomInfo() {

			ci.clear();
		};
		void clear() { memset(this, 0, sizeof(PlayerRoomInfoEx)); };
		CharacterInfo ci;
	};

	struct RoomGuildInfo {
		void clear() {
			memset(this, 0, sizeof(RoomGuildInfo));

		};
		uint32_t guild_1_uid;
		uint32_t guild_2_uid;
		char guild_1_mark[12];
		char guild_2_mark[12];
		unsigned short guild_1_index_mark;
		unsigned short guild_2_index_mark;
		char guild_1_nome[20];
		char guild_2_nome[20];
	};

	struct RoomGrandPrixInfo {
		void clear() {
			memset(this, 0, sizeof(RoomGrandPrixInfo));
		};
		uint32_t dados_typeid;
		uint32_t rank_typeid;
		uint32_t tempo;
		uint32_t active;
	};

	union uNaturalAndShortGame {
		uNaturalAndShortGame(uint32_t _ul = 0u) : ulNaturalAndShortGame(_ul) {};
		void clear() { ulNaturalAndShortGame = 0u; };
		uint32_t ulNaturalAndShortGame;
		struct {
			uint32_t natural : 1;
			uint32_t short_game : 1, : 0;
		}stBit;
	};

	struct RoomInfo {
		enum eCOURSE : unsigned char {
			BLUE_LAGOON,
			BLUE_WATER,
			SEPIA_WIND,
			WIND_HILL,
			WIZ_WIZ,
			WEST_WIZ,
			BLUE_MOON,
			SILVIA_CANNON,
			ICE_CANNON,
			WHITE_WIZ,
			SHINNING_SAND,
			PINK_WIND,
			DEEP_INFERNO = 13,
			ICE_SPA,
			LOST_SEAWAY,
			EASTERN_VALLEY,
			CHRONICLE_1_CHAOS,
			ICE_INFERNO,
			WIZ_CITY,
			ABBOT_MINE,
			MYSTIC_RUINS,
			GRAND_ZODIAC = 64,
			RANDOM = 127,
		};
		enum TIPO : uint32_t {
			STROKE,
			MATCH,
			LOUNGE,
			TOURNEY = 4,
			TOURNEY_TEAM,
			GUILD_BATTLE,
			PANG_BATTLE,
			APPROCH = 10,
			GRAND_ZODIAC_INT,
			GRAND_ZODIAC_ADV = 13,
			GRAND_ZODIAC_PRACTICE,
			SPECIAL_SHUFFLE_COURSE = 18,
			PRACTICE,
			GRAND_PRIX,
		};
		enum MODO : uint32_t {
			M_FRONT,
			M_BACK,
			M_RANDOM,
			M_SHUFFLE,
			M_REPEAT,
			M_SHUFFLE_COURSE,
		};
		enum INFO_CHANGE : uint32_t {
			NAME,
			SENHA,
			TIPO,
			COURSE,
			QNTD_HOLE,
			MODO,
			TEMPO_VS,
			MAX_PLAYER,
			TEMPO_30S,
			STATE_FLAG,
			UNKNOWN,
			HOLE_REPEAT,
			FIXED_HOLE,
			ARTEFATO,
			NATURAL,
		};
		RoomInfo(uint32_t _ul = 0u) {

			clear();

		};
		void clear() {

			memset(this, 0, sizeof(RoomInfo));

			numero = -1;
			senha_flag = 1;
			state = 1;
			_30s = 30;

			guilds.clear();
		};
		char nome[64];
		unsigned char senha_flag : 1, : 0;
		unsigned char state : 1, : 0;
		unsigned char flag;
		unsigned char max_player;
		unsigned char num_player;
		char key[17];
		unsigned char _30s;
		unsigned char qntd_hole;
		unsigned char tipo_show;
		short numero;
		unsigned char modo;

		eCOURSE course;
		uint32_t time_vs;
		uint32_t time_30s;
		uint32_t trofel;
		unsigned short state_flag;
		RoomGuildInfo guilds;
		uint32_t rate_pang;
		uint32_t rate_exp;
		unsigned char flag_gm;
		int		   master;
		unsigned char tipo_ex;
		uint32_t artefato;

		uNaturalAndShortGame natural;
		RoomGrandPrixInfo grand_prix;
	};

	struct RoomInfoEx : public RoomInfo {
		RoomInfoEx(uint32_t _ul = 0u) : RoomInfo(_ul) {

			hole_repeat = 0u;
			fixed_hole = 0u;
			tipo = 0u;
			state_afk = 0u;
			channel_rookie = 0u;
			angel_event = 0u;
			bet_crazy_club = 0u;
		};
		void clear() {

			RoomInfo::clear();

			hole_repeat = 0u;
			fixed_hole = 0u;
			tipo = 0u;
			state_afk = 0u;
			channel_rookie = 0u;
			angel_event = 0u;
			bet_crazy_club = 0u;
		};
		char senha[64];
		unsigned char tipo;
		unsigned char hole_repeat;
		uint32_t fixed_hole;
		unsigned char state_afk : 1, : 0;
		unsigned char channel_rookie : 1;
		unsigned char angel_event : 1;

		unsigned char bet_crazy_club : 1;
	};

	struct RateValue {
		void clear() { memset(this, 0, sizeof(RateValue)); };
		uint32_t pang;
		uint32_t exp;
		uint32_t clubset;
		uint32_t rain;
		uint32_t treasure;
		unsigned char persist_rain;
	};

	struct ClientVersion {
		ClientVersion() {
			memset(this, 0, sizeof(ClientVersion));

			flag = REDUZI_VERSION;
		};
		ClientVersion(uint32_t _high, uint32_t _low) {
			memset(this, 0, sizeof(ClientVersion));

			high = _high;
			low = _low;

			flag = REDUZI_VERSION;
		};
		ClientVersion(const char _region[3], const char _season[3], uint32_t _high, uint32_t _low) {
			memset(this, 0, sizeof(ClientVersion));

			if (_region == nullptr || _season == nullptr)
				throw exception("Error argument invalid, _region or _season is nullptr. ClientVersion::ClientVersion()", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CLIENTVERSION, 7, 0));

#if defined(_WIN32)
			strcpy_s(region, _region);
			strcpy_s(season, _season);
#elif defined(__linux__)
			strcpy(region, _region);
			strcpy(season, _season);
#endif

			high = _high;
			low = _low;

			flag = COMPLETE_VERSION;
		};
		static ClientVersion make_version(std::string& _cv) {

			ClientVersion cv;

			if (_cv.empty())
				throw exception("Error cv is empty, ClientVersion::make_version()", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CLIENTVERSION, 1, 0));

			std::vector< std::string > v_s;

			const char* start = _cv.data();
			const char *tmp = start;

			size_t i;

			while ((tmp = strpbrk(tmp, ".")) != nullptr) {
				v_s.push_back(std::string(start, tmp - start));

				if ((i = strspn(tmp, ".")))
					tmp += i;

				start = tmp;
			}

#if defined(_WIN32)
			if (start < _cv.end()._Ptr)
				v_s.push_back(std::string(start, _cv.end()._Ptr - start));
#elif defined(__linux__)
			if (start < &(*_cv.end()))
				v_s.push_back(std::string(start, &(*_cv.end()) - start));
#endif

			if (v_s.empty())
				throw exception("Error Invalid argument. ClientVersion::make_version()", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CLIENTVERSION, 2, 0));

			if (v_s.size() < 2)
				throw exception("Erro Not string token enough, ClientVersion::make_version()", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CLIENTVERSION, 3, 0));

			try {
				if (v_s.size() == 2)
					cv = { (unsigned int )std::stoi(v_s[0]), (unsigned int )std::stoi(v_s[1]) };
				else if (v_s.size() == 4)
					cv = { v_s[0].c_str(), v_s[1].c_str(), (unsigned int )std::stoi(v_s[2]), (unsigned int )std::stoi(v_s[3]) };
				else
					throw exception("Error pegou token string estranho. ClientVersion::make_version()", STDA_MAKE_ERROR(STDA_ERROR_TYPE::CLIENTVERSION, 4, 0));
			}catch (std::invalid_argument& e) {
				throw exception("Error invalid argument std::stoul(), ClientVersion::make_version(). " + std::string(e.what()), STDA_MAKE_ERROR(STDA_ERROR_TYPE::CLIENTVERSION, 5, errno));
			}catch (std::out_of_range& e) {
				throw exception("Error out of range std::stoul(), ClientVersion::make_version(). " + std::string(e.what()), STDA_MAKE_ERROR(STDA_ERROR_TYPE::CLIENTVERSION, 6, errno));
			}

			v_s.clear();
			v_s.shrink_to_fit();

			return cv;
		};
		std::string fixedValue(uint32_t _value, uint32_t _width) {

			std::ostringstream results;

			results.fill('0');

			results.setf(std::ios_base::internal, std::ios_base::adjustfield);

			results << std::setw((_value < 0 ? _width + 1 : _width)) << _value;

			return results.str();
		};
		std::string toString() {
			return std::string(region) + "." + std::string(season) + "." + fixedValue(high, 2) + "." + fixedValue(low, 2);
		};
		char region[3];
		char season[3];
		uint32_t high;
		uint32_t low;
		unsigned char flag : 1;
		enum : unsigned char {
			REDUZI_VERSION,
			COMPLETE_VERSION,
		};
	};

	struct ItemPangyaBase {
		void clear() {
			memset(this, 0, sizeof(ItemPangyaBase));
		};
		unsigned char tipo;
		int32_t _typeid;
		int32_t id;
		int32_t tipo_unidade_add;
		int32_t qntd_ant;
		int32_t qntd_dep;
		int32_t qntd;
		unsigned char unknown[8];
		short qntd_time;
	};

	struct ItemPangya : public ItemPangyaBase {
		ItemPangya() : ItemPangyaBase() {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(ItemPangya));
		};
		char sd_idx[9];
		int32_t sd_status;
		int32_t sd_seq;
		unsigned char unknown2[5];
	};

	struct BuyItem {
		void clear() { memset(this, 0, sizeof(BuyItem)); };
		uint32_t id;
		uint32_t _typeid;
		unsigned short time;
		unsigned short usUnknown;
		uint32_t qntd;
		uint32_t pang;
		uint32_t cookie;
		unsigned char ucUnknown13[13];
	};

	struct EmailInfo {
		EmailInfo(uint32_t _ul = 0u) {
			clear();
		};
		~EmailInfo() {
			clear();
		};
		void clear() {

			id = -1;
			memset(from_id, 0, sizeof(from_id));
			memset(gift_date, 0, sizeof(gift_date));
			memset(msg, 0, sizeof(msg));
			lida_yn = 0u;

			if (!itens.empty()) {
				itens.clear();
				itens.shrink_to_fit();
			}
		};
		int32_t id;
		char from_id[22];
		char gift_date[20];
		char msg[100];
		unsigned char lida_yn;
		struct item {
			void clear() { memset(this, 0, sizeof(item)); };
			int32_t id;
			uint32_t _typeid;
			unsigned char flag_time;
			int32_t qntd;
			int32_t tempo_qntd;
			uint64_t pang;
			uint64_t cookie;
			int32_t gm_id;
			int32_t flag_gift;
			char ucc_img_mark[9];
			unsigned char ucUnknown3[3];
			short type;
		};
		std::vector< EmailInfo::item > itens;
	};

	struct EmailInfoEx : public EmailInfo {
		EmailInfoEx(uint32_t _ul = 0u) : EmailInfo(_ul), visit_count(0l) {
		};
		void clear() {

			EmailInfo::clear();

			visit_count = 0l;
		};
		int32_t visit_count;
	};

	struct MailBox {
		void clear() {
			memset(this, 0, sizeof(MailBox));
		};
		int32_t id;
		char from_id[30];
		char msg[80];
		char unknown2[18];
		int32_t visit_count;
		unsigned char lida_yn;
		int32_t item_num;
		EmailInfo::item item;
	};

	struct TicketReportScrollInfo {
		TicketReportScrollInfo(uint32_t _ul = 0u) {
			clear();
		};
		~TicketReportScrollInfo() {};
		void clear() {

			id = -1;
			date = { 0 };

			if (!v_players.empty()) {
				v_players.clear();
				v_players.shrink_to_fit();
			}
		};
		struct stPlayerDados {
			stPlayerDados(uint32_t _ul = 0u) {
				clear();
			};
			void clear() {
				memset(this, 0, sizeof(stPlayerDados));

				ucUnknown_flg = 2u;
			};
			uint32_t uid;
			uint64_t pang;
			uint64_t bonus_pang;
			uint32_t trofel_typeid;
			uint32_t exp;
			uint32_t mascot_typeid;
			unsigned char premium_user : 1, : 0;
			unsigned char item_boost : 3, : 0;
			uint32_t level;
			char score;
			uMedalWin medalha;
			unsigned char trofel;
			char id[22];
			char nickname[22];
			uint32_t ulUnknown;
			uint32_t guild_uid;
			uint32_t mark_index;
			char guild_mark_img[12];
			uint32_t tipo;
			unsigned char state;
			unsigned char ucUnknown_flg;
			SYSTEMTIME finish_time;
		};
		int32_t id;
		SYSTEMTIME date;
		std::vector< stPlayerDados > v_players;
	};

	struct InviteChannelInfo {
#define STDA_INVITE_TIME_MILLISECONDS		5000
		void clear() { memset(this, 0, sizeof(InviteChannelInfo)); };
		short room_number;
		uint32_t invite_uid;
		uint32_t invited_uid;
		SYSTEMTIME time;
	};

	struct CommandInfo {
		CommandInfo(uint32_t _ul = 0u) {
			clear();
		};
		void clear() { memset(this, 0, sizeof(CommandInfo)); };
		std::string toString() {
			return "IDX=" + std::to_string(idx) + ", ID=" + std::to_string(id) + ", ARG1="
				+ std::to_string(arg[0]) + ", ARG2=" + std::to_string(arg[1]) + ", ARG3="
				+ std::to_string(arg[2]) + ", ARG4=" + std::to_string(arg[3]) + ", ARG5="
				+ std::to_string(arg[4]) + ", TARGET=" + std::to_string(target) + ", FLAG="
				+ std::to_string(flag) + ", VALID=" + std::to_string((unsigned short)valid) + ", RESERVEDATE=" + std::to_string(reserveDate);
		};
		uint32_t idx;
		uint32_t id;
		uint32_t arg[5];
		uint32_t target;
		unsigned short flag;
		unsigned char valid : 1;
		time_t reserveDate;
	};

	struct UpdateItem {
		enum UI_TYPE : unsigned char {
			CADDIE,
			CADDIE_PARTS,
			MASCOT,
			WAREHOUSE,
		};
		UpdateItem(uint32_t _ul = 0u) {
			clear();
		};
		UpdateItem(UI_TYPE _type, uint32_t __typeid, int32_t _id)
			: type(_type), _typeid(__typeid), id(_id) {

		};
		void clear() {
			memset(this, 0, sizeof(UpdateItem));
		};
		UI_TYPE type;
		uint32_t _typeid;
		int32_t id;
	};

	struct GrandPrixClear {
		GrandPrixClear(uint32_t _ul = 0u) {
			clear();
		};
		GrandPrixClear(uint32_t __typeid, uint32_t _position)
			: _typeid(__typeid), position(_position) {
		};
		void clear() {
			memset(this, 0, sizeof(GrandPrixClear));
		};
		uint32_t _typeid;
		uint32_t position;
	};

	struct GuildUpdateActivityInfo {
		enum TYPE_UPDATE : unsigned char {
			TU_ACCEPTED_MEMBER,
			TU_EXITED_MEMBER,
			TU_KICKED_MEMBER,
		};
		void clear() {
			memset(this, 0, sizeof(GuildUpdateActivityInfo));
		};
		uint64_t index;
		uint32_t club_uid;
		uint32_t owner_uid;
		uint32_t player_uid;
		TYPE_UPDATE type;
		time_t reg_date;
	};

	struct ChangePlayerItemRoom {
		ChangePlayerItemRoom(uint32_t _ul = 0u) {
			clear();
		};
		void clear() {
			memset(this, 0, sizeof(ChangePlayerItemRoom));
		};
		enum TYPE_CHANGE : unsigned char {
			TC_CADDIE = 1,
			TC_BALL,
			TC_CLUBSET,
			TC_CHARACTER,
			TC_MASCOT,
			TC_ITEM_EFFECT_LOUNGE,
			TC_ALL,
			TC_UNKNOWN = 255,
		};
		struct stItemEffectLounge {
			void clear() { memset(this, 0, sizeof(stItemEffectLounge)); };
			enum TYPE_EFFECT : uint32_t {
				TE_BIG_HEAD = 1,
				TE_FAST_WALK,
				TE_TWILIGHT,
			};
			uint32_t item_id;
			TYPE_EFFECT effect;
		};
		TYPE_CHANGE type;
		uint32_t caddie;
		uint32_t ball;
		uint32_t clubset;
		uint32_t character;
		uint32_t mascot;
		stItemEffectLounge effect_lounge;
	};

#if defined(__linux__)
#pragma pack()
#endif
}

#endif
