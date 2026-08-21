
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "room_grand_zodiac_event.hpp"

#include "../PACKET/packet_func_sv.h"

#include "../../Projeto IOCP/THREAD POOL/threadpool_base.hpp"

#include "../../Projeto IOCP/UTIL/util_time.h"

#include "../Game Server/game_server.h"

#include "grand_zodiac.hpp"

#include "../UTIL/lottery.hpp"

#if defined(_WIN32)
#include <DbgHelp.h>
#endif

using namespace stdA;

#define CHECK_SESSION_BEGIN(method) if (!_session.getState()) \
										throw exception("[RoomGrandZodiacEvent::" + std::string((method)) + "][Error] player nao esta connectado", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_ZODIAC_EVENT, 12, 0)); \

#define REQUEST_BEGIN(method) CHECK_SESSION_BEGIN(std::string("request") + (method)) \
							  if (_packet == nullptr) \
									throw exception("[RoomGrandZodiacEvent::request" + std::string((method)) + "][Error] _packet is nullptr", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_ZODIAC_EVENT, 12, 0)); \

RoomGrandZodiacEvent::RoomGrandZodiacEvent(unsigned char _channel_owner, RoomInfoEx _ri) : room(_channel_owner, _ri), m_state_rgze(), m_create_room{ 0 },
	m_wait_time_start(nullptr),
#if defined(_WIN32)
	m_hEvent_wait_start(INVALID_HANDLE_VALUE), m_hEvent_wait_start_pulse(INVALID_HANDLE_VALUE),
#elif defined(__linux__)
	m_hEvent_wait_start(nullptr), m_hEvent_wait_start_pulse(nullptr),
#endif
	m_timer_count_down(nullptr) {

	push_instancia(this);

	GetLocalTime(&m_create_room);

#if defined(_WIN32)
	if ((m_hEvent_wait_start = CreateEvent(NULL, TRUE, FALSE, NULL)) == INVALID_HANDLE_VALUE)
		throw exception("[RoomGrandZodiacEvent::RoomGrandZodiacEvent][Error] ao criar evento wait time start.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_ZODIAC_EVENT, 1050, GetLastError()));
#elif defined(__linux__)

	m_hEvent_wait_start = new Event(true, 0u);

	if (!m_hEvent_wait_start->is_good()) {

		delete m_hEvent_wait_start;

		m_hEvent_wait_start = nullptr;

		throw exception("[RoomGrandZodiacEvent::RoomGrandZodiacEvent][Error] ao criar evento wait time start.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_ZODIAC_EVENT, 1050, errno));
	}
#endif

#if defined(_WIN32)
	if ((m_hEvent_wait_start_pulse = CreateEvent(NULL, FALSE, FALSE, NULL)) == INVALID_HANDLE_VALUE)
		throw exception("[RoomGrandZodiacEvent::RoomGrandZodiacEvent][Error] ao criar evento wait time start pulse.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_ZODIAC_EVENT, 1050, GetLastError()));
#elif defined(__linux__)

	m_hEvent_wait_start_pulse = new Event(false, 0u);

	if (!m_hEvent_wait_start_pulse->is_good()) {

		delete m_hEvent_wait_start_pulse;

		m_hEvent_wait_start_pulse = nullptr;

		throw exception("[RoomGrandZodiacEvent::RoomGrandZodiacEvent][Error] ao criar evento wait time start pulse.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_ZODIAC_EVENT, 1050, errno));
	}
#endif

	m_wait_time_start = new thread(1063 , RoomGrandZodiacEvent::_waitTimeStart, (LPVOID)this);
}

RoomGrandZodiacEvent::~RoomGrandZodiacEvent() {

	clear_timer_count_down();

	finish_thread_sync_wait_time_start();

	pop_instancia(this);
}

bool RoomGrandZodiacEvent::isAllReady() {

	return !_haveInvited();
}

bool RoomGrandZodiacEvent::startGame() {

	packet p;

	bool ret = true;

	try {

		if (m_pGame != nullptr)
			throw exception("[RoomGrandZodiacEvent::startGame][Error] Server tentou comecar o jogo na sala[NUMERO="
					+ std::to_string(m_ri.numero) + "], mas ja tem um jogo inicializado. Hacker ou Bug",
					STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_ZODIAC_EVENT, 8, 0x5900202));

		if (!isAllReady())
			throw exception("[RoomGrandZodiacEvent::startGame][Error] Server tentou comecar o jogo na sala[NUMERO="
					+ std::to_string(m_ri.numero) + ", MASTER=" + std::to_string(m_ri.master)
					+ "], mas nem todos jogadores estao prontos. Hacker ou Bug.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_ZODIAC_EVENT, 8, 0x5900202));

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
		case RoomInfo::TIPO::GRAND_ZODIAC_INT:
		case RoomInfo::TIPO::GRAND_ZODIAC_ADV:
			m_pGame = new GrandZodiac(v_sessions, m_ri, rv, m_ri.channel_rookie);
			break;
		default:
			throw exception("[RoomGrandZodiacEvent::startGame][Error] Server tentou comecar o jogo na sala[NUMERO="
					+ std::to_string(m_ri.numero) + ", MASTER=" + std::to_string(m_ri.master)
					+ "], mas o tipo da sala nao eh Grand Zodiac. Hacker ou Bug", STDA_MAKE_ERROR(STDA_ERROR_TYPE::ROOM_GRAND_ZODIAC_EVENT, 9, 0x5900202));
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

		m_state_rgze.setStateWithLock(eSTATE_ROOM_GRAND_ZODIAC_EVENT_SYNC::WAIT_END_GAME);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[RoomGrandZodiacEvent::startGame][Error] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		ret = false;
	}

	return ret;
}

void RoomGrandZodiacEvent::initFirstInstance() {

	if (m_cs_instancia::getInstance().m_state && m_instancias::getInstance().empty())
		_smp::message_pool::getInstance().push(new message("[RoomGrandZodiacEvent::initFirstInstance][Log] Criou primeira instance do Singleton da classe Room Grand Zodiac Event static vector.", CL_FILE_LOG_AND_CONSOLE));
}

#if defined(_WIN32)
DWORD WINAPI RoomGrandZodiacEvent::_waitTimeStart(LPVOID lpParameter) {
#elif defined(__linux__)
void* RoomGrandZodiacEvent::_waitTimeStart(LPVOID lpParameter) {
#endif
	BEGIN_THREAD_SETUP(RoomGrandZodiacEvent);

	result = pTP->waitTimeStart();

	END_THREAD_SETUP("waitTimeStart()");
}

#if defined(_WIN32)
DWORD RoomGrandZodiacEvent::waitTimeStart() {
#elif defined(__linux__)
void* RoomGrandZodiacEvent::waitTimeStart() {
#endif

	try {

		_smp::message_pool::getInstance().push(new message("[RoomGrandZodiacEvent::waitTimeStart][Log] waitTimeStart iniciado com sucesso!", CL_FILE_LOG_AND_CONSOLE));

		packet p;

		DWORD retWait = WAIT_TIMEOUT;

#if defined(_WIN32)
		HANDLE wait_events[2] = { m_hEvent_wait_start, m_hEvent_wait_start_pulse };

		while ((retWait = WaitForMultipleObjects((sizeof(wait_events) / sizeof(wait_events[0])), wait_events, false, 1000 )) == WAIT_TIMEOUT || retWait == (WAIT_OBJECT_0 + 1)) {
#elif defined(__linux__)
		std::vector< Event* > wait_events = { m_hEvent_wait_start, m_hEvent_wait_start_pulse };

		while ((retWait = Event::waitMultipleEvent(wait_events.size(), wait_events, false, 1000 )) == WAIT_TIMEOUT || retWait == (WAIT_OBJECT_0 + 1)) {
#endif

			try {

				m_state_rgze.lock();

				switch (m_state_rgze.getState()) {
					case eSTATE_ROOM_GRAND_ZODIAC_EVENT_SYNC::WAIT_TIME_START:
					{

						if (m_timer_count_down == nullptr) {

							if ((getLocalTimeDiff(m_create_room) / STDA_10_MICRO_PER_MIN) >= 2 && v_sessions.size() > 0) {

								count_down(10);

								m_state_rgze.setState(eSTATE_ROOM_GRAND_ZODIAC_EVENT_SYNC::WAIT_10_SECONDS_START);

							}else if (_getRealNumPlayersWithoutInvited() == m_ri.max_player) {

								packet p((unsigned short)0x40);

								p.addUint8(12);

								p.addUint16(0u);
								p.addUint16(0u);

								p.addUint32(10u);

								packet_func::room_broadcast(*this, p, 1);

								count_down(10);

								m_state_rgze.setState(eSTATE_ROOM_GRAND_ZODIAC_EVENT_SYNC::WAIT_10_SECONDS_START);
							}
						}

						break;
					}
					case eSTATE_ROOM_GRAND_ZODIAC_EVENT_SYNC::WAIT_10_SECONDS_START:
					{

						if (m_pGame == nullptr && v_sessions.size() == 0)
							m_state_rgze.setState(eSTATE_ROOM_GRAND_ZODIAC_EVENT_SYNC::WAIT_TIME_START);

						break;
					}
					case eSTATE_ROOM_GRAND_ZODIAC_EVENT_SYNC::WAIT_END_GAME:
					{

						break;
					}
				}

				m_state_rgze.unlock();

			}catch (exception& e) {

				m_state_rgze.unlock();

				_smp::message_pool::getInstance().push(new message("[RoomGrandZodiacEvent::waitTimeStart][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
			}
		}

	}catch (exception& e) {
		_smp::message_pool::getInstance().push(new message("[RoomGrandZodiacEvent::waitTimeStart][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}catch (std::exception& e) {
		_smp::message_pool::getInstance().push(new message(std::string("[RoomGrandZodiacEvent::waitTimeStart][ErrorSystem] ") + e.what(), CL_FILE_LOG_AND_CONSOLE));
	}catch (...) {
		_smp::message_pool::getInstance().push(new message("[RoomGrandZodiacEvent::waitTimeStart][ErrorSystem] waitTimeStart() -> Exception (...) c++ nao tratada ou uma excessao de C(nullptr e etc)\n", CL_FILE_LOG_AND_CONSOLE));
	}

#ifdef _DEBUG
	_smp::message_pool::getInstance().push(new message("[RoomGrandZodiacEvent::waitTimeStart][Log] Saindo de waitTimeStart()...", CL_FILE_LOG_AND_CONSOLE));
#else
	_smp::message_pool::getInstance().push(new message("[RoomGrandZodiacEvent::waitTimeStart][Log] Saindo de waitTimeStart()...", CL_ONLY_FILE_LOG));
#endif

#if defined(_WIN32)
	return 0;
#elif defined(__linux__)
	return (void*)0;
#endif
}

int RoomGrandZodiacEvent::_count_down_time(void* _arg1, void* _arg2) {

	RoomGrandZodiacEvent *rgze = reinterpret_cast< RoomGrandZodiacEvent* >(_arg1);
	int64_t sec_to_start = reinterpret_cast< int64_t >(_arg2);

	try {

		if (rgze != nullptr && instancia_valid(rgze))
			rgze->count_down(sec_to_start);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[RoomGrandZodiacEvent::_count_down_time][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	return 0;
}

void RoomGrandZodiacEvent::count_down(int64_t _sec_to_start) {

	try {

		lock();

		if (_sec_to_start <= 0) {

			clear_timer_count_down();

			if (v_sessions.size() >= 1 && startGame())
				sgs::gs::getInstance().sendUpdateRoomInfo(this, 3);
			else if (v_sessions.size() >= 1)

				count_down(10);

		}else {

			uint32_t wait = 0u;
			int32_t rest = 0;

			unsigned char type = 1u;

			DWORD interval = 0u;
			float diff = 0.f;

			int32_t elapsed_sec = (m_timer_count_down != nullptr) ? (int)std::round(m_timer_count_down->getElapsed() / 1000.f)  : 0;

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

			if (m_timer_count_down == nullptr || m_timer_count_down->getState() == timer::STOP ||
				m_timer_count_down->getState() == timer::STOPPING || m_timer_count_down->getState() == timer::STOPPED) {

				job _job(RoomGrandZodiacEvent::_count_down_time, this, (void*)_sec_to_start);

				if (m_timer_count_down != nullptr)
					clear_timer_count_down();

				m_timer_count_down = sgs::gs::getInstance().makeTime(wait, _job, std::vector< DWORD > { interval });
			}
		}

		unlock();

	}catch (exception& e) {

		unlock();

		_smp::message_pool::getInstance().push(new message("[RoomGrandZodiacEvent::count_down][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

void RoomGrandZodiacEvent::finish_thread_sync_wait_time_start() {

	try {

		if (m_wait_time_start != nullptr) {

#if defined(_WIN32)
			if (m_wait_time_start != INVALID_HANDLE_VALUE)
				SetEvent(m_hEvent_wait_start);
#elif defined(__linux__)
			if (m_wait_time_start != nullptr)
				m_hEvent_wait_start->set();
#endif

			m_wait_time_start->waitThreadFinish(INFINITE);

			delete m_wait_time_start;
		}

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[RoomGrandZodiacEvent::finish_thread_sync_wait_time_start][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		if (m_wait_time_start != nullptr) {

			m_wait_time_start->exit_thread();

			delete m_wait_time_start;
		}
	}

	m_wait_time_start = nullptr;

#if defined(_WIN32)
	if (m_hEvent_wait_start != INVALID_HANDLE_VALUE)
		CloseHandle(m_hEvent_wait_start);

	if (m_hEvent_wait_start_pulse != INVALID_HANDLE_VALUE)
		CloseHandle(m_hEvent_wait_start_pulse);

	m_hEvent_wait_start = INVALID_HANDLE_VALUE;
	m_hEvent_wait_start_pulse = INVALID_HANDLE_VALUE;
#elif defined(__linux__)
	if (m_hEvent_wait_start != nullptr)
		delete m_hEvent_wait_start;

	if (m_hEvent_wait_start_pulse != nullptr)
		delete m_hEvent_wait_start_pulse;

	m_hEvent_wait_start = nullptr;
	m_hEvent_wait_start_pulse = nullptr;
#endif
}

void RoomGrandZodiacEvent::clear_timer_count_down() {

	try {

		if (m_timer_count_down != nullptr)
			sgs::gs::getInstance().unMakeTime(m_timer_count_down);

	}catch (exception& e) {

		_smp::message_pool::getInstance().push(new message("[RoomGrandZodiacEvent::clear_timer_count_down][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	m_timer_count_down = nullptr;
}

void RoomGrandZodiacEvent::push_instancia(RoomGrandZodiacEvent* _rgze) {

	m_cs_instancia::getInstance().lock();

	m_instancias::getInstance().push_back(RoomGrandZodiacEventInstanciaCtx(_rgze, RoomGrandZodiacEventInstanciaCtx::eSTATE::GOOD));

	m_cs_instancia::getInstance().unlock();
}

void RoomGrandZodiacEvent::pop_instancia(RoomGrandZodiacEvent* _rgze) {

	m_cs_instancia::getInstance().lock();

	auto index = get_instancia_index(_rgze);

	if (index >= 0)
		m_instancias::getInstance().erase(m_instancias::getInstance().begin() + index);

	m_cs_instancia::getInstance().unlock();
}

void RoomGrandZodiacEvent::set_instancia_state(RoomGrandZodiacEvent* _rgze, RoomGrandZodiacEventInstanciaCtx::eSTATE _state) {

	m_cs_instancia::getInstance().lock();

	auto index = get_instancia_index(_rgze);

	if (index >= 0)
		m_instancias::getInstance()[index].m_state = _state;

	m_cs_instancia::getInstance().unlock();
}

int RoomGrandZodiacEvent::get_instancia_index(RoomGrandZodiacEvent* _rgze) {

	int index = -1;

	for (auto i = 0u; i < m_instancias::getInstance().size(); ++i) {

		if (m_instancias::getInstance()[i].m_rgze == _rgze) {

			index = (int)i;

			break;
		}
	}

	return index;
}

bool RoomGrandZodiacEvent::instancia_valid(RoomGrandZodiacEvent* _rgze) {

	bool valid = false;

	m_cs_instancia::getInstance().lock();

	auto index = get_instancia_index(_rgze);

	if (index >= 0)
		valid = (m_instancias::getInstance()[index].m_state == RoomGrandZodiacEventInstanciaCtx::eSTATE::GOOD);

	m_cs_instancia::getInstance().unlock();

	return valid;
}
