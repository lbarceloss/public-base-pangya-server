
#if defined(_WIN32)
#pragma pack(1)
#endif

#include "block_exec_one_per_time.hpp"

#define IS_INVALID_HANDLE(_handle) ((_handle) == INVALID_HANDLE_VALUE || (_handle) == NULL)

using namespace stdA;

ThreadSafeCS::ThreadSafeCS() : m_lock_spin(0l) {

#if defined(_WIN32)
	InitializeCriticalSection(&m_cs);
#elif defined(__linux__)
	INIT_PTHREAD_MUTEXATTR_RECURSIVE;
	INIT_PTHREAD_MUTEX_RECURSIVE(&m_cs);
	DESTROY_PTHREAD_MUTEXATTR_RECURSIVE;
#endif
}

ThreadSafeCS::~ThreadSafeCS() {

	if (m_lock_spin > 0l)
		unlock();

#if defined(_WIN32)
	DeleteCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_destroy(&m_cs);
#endif
}

void ThreadSafeCS::lock() {

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	m_lock_spin++;
}

bool ThreadSafeCS::try_lock() {

	bool ret = false;

#if defined(_WIN32)
	if ((ret = TryEnterCriticalSection(&m_cs))) {
#elif defined(__linux__)
	int err_ret = 0;

	if ((err_ret = pthread_mutex_trylock(&m_cs)) == 0) {

		ret = true;
#endif
		m_lock_spin++;
	}

	return ret;
}

void ThreadSafeCS::unlock() {

	if (m_lock_spin <= 0l) {

		_smp::message_pool::getInstance().push(new message("[ThreadSafeCS::unlock][WARNING] already unlock.", CL_FILE_LOG_AND_CONSOLE));

		return;
	}

	m_lock_spin--;

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif
}

EventCV::EventCV() : m_id(INVALID_PACKET_ID) {

#if defined(_WIN32)
	InitializeConditionVariable(&m_cv);
#elif defined(__linux__)
	pthread_cond_init(&m_cv, nullptr);
#endif
}

EventCV::~EventCV() {
#if defined(__linux__)
	pthread_cond_destroy(&m_cv);
#endif
}

void EventCV::waitEvent(ThreadSafeCS& _cs, unsigned short _id, DWORD _wait_time) {

	if (_id == INVALID_PACKET_ID) {

		_smp::message_pool::getInstance().push(new message("[EventCV::waitEvent][Error] invalid id(" + std::to_string(_id) + ")", CL_FILE_LOG_AND_CONSOLE));

		return;
	}

	DWORD lastErr = 0u;

	while (m_id != _id) {

#if defined(_WIN32)
		if (!SleepConditionVariableCS(&m_cv, &_cs.m_cs, _wait_time)) {

			if ((lastErr = GetLastError()) == ERROR_TIMEOUT && _wait_time == INFINITE)
				_smp::message_pool::getInstance().push(new message("[EventCV::waitEvent][Error] SleepConditionVariableCS erro unknown, timeout with _wait_time = INFINITE. ErrCode: "
						+ std::to_string(lastErr), CL_FILE_LOG_AND_CONSOLE));
			else if (lastErr != ERROR_TIMEOUT)
				_smp::message_pool::getInstance().push(new message("[EventCV::waitEvent][Error] SleepConditionVariableCS ErrCode: " + std::to_string(lastErr), CL_FILE_LOG_AND_CONSOLE));
			else
				WakeConditionVariable(&m_cv);

		}else
			WakeConditionVariable(&m_cv);
#elif defined(__linux__)
		int error = 0;

		if ((int32_t)_wait_time == INFINITE) {

			if ((error = pthread_cond_wait(&m_cv, &_cs.m_cs)) != 0) {

				if (error == ETIMEDOUT)
					_smp::message_pool::getInstance().push(new message("[EventCV::waitEvent][Error] pthread_cond_wait erro unknown, timeout with _wait_time = INFINITE. ErrCode: "
							+ std::to_string(error), CL_FILE_LOG_AND_CONSOLE));
				else
					_smp::message_pool::getInstance().push(new message("[EventCV::waitEvent][Error] pthread_cond_wait ErrCode: " + std::to_string(error), CL_FILE_LOG_AND_CONSOLE));

			}else
				pthread_cond_signal(&m_cv);

		}else {

			timespec wait_time = _milliseconds_to_timespec_clock_realtime(_wait_time);

			if ((error = pthread_cond_timedwait(&m_cv, &_cs.m_cs, &wait_time)) != 0) {

				if (error != ETIMEDOUT)
					_smp::message_pool::getInstance().push(new message("[EventCV::waitEvent][Error] pthread_cond_timedwait ErrCode: " + std::to_string(error), CL_FILE_LOG_AND_CONSOLE));
				else
					pthread_cond_signal(&m_cv);

			}else
				pthread_cond_signal(&m_cv);
		}
#endif
	}

	m_id = INVALID_PACKET_ID;
}

void EventCV::setEvent(unsigned short _id) {

	if (_id == INVALID_PACKET_ID) {

		_smp::message_pool::getInstance().push(new message("[EventCV::setEvent][Error] invalid id(" + std::to_string(_id) + ")", CL_FILE_LOG_AND_CONSOLE));

		return;
	}

	m_id = _id;

#if defined(_WIN32)
	WakeConditionVariable(&m_cv);
#elif defined(__linux__)
	pthread_cond_signal(&m_cv);
#endif
}

#if STDA_BLOCK_PACKET_ONE_TIME_VER == 0x2
SyncBlockExecOnePerTime::SyncBlockExecOnePerTime(uint32_t _uid) : m_mp_event_block{}, m_ts_cs(), m_uid(_uid) {
#else
SyncBlockExecOnePerTime::SyncBlockExecOnePerTime() : m_mp_event_block{}, m_ts_cs() {
#endif
}

SyncBlockExecOnePerTime::~SyncBlockExecOnePerTime() {

	try {

		MapIdentifyPacketKey::iterator first_ev;

		m_ts_cs.lock();

		while (!m_mp_event_block.empty() && (first_ev = m_mp_event_block.begin()) != m_mp_event_block.end()) {

			m_mp_event_block.erase(first_ev);
		}

#if STDA_BLOCK_PACKET_ONE_TIME_VER == 0x1
		MapPlayerEvent::iterator first_pe;

		while (!m_mp_player.empty() && (first_pe = m_mp_player.begin()) != m_mp_player.end()) {

			m_mp_player.erase(first_pe);
		}
#endif

		m_ts_cs.unlock();

	}catch (exception& e) {

		m_ts_cs.unlock();

		_smp::message_pool::getInstance().push(new message("[SyncBlockExecOnePerTime::~SyncBlockExecOnePerTime][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

#if STDA_BLOCK_PACKET_ONE_TIME_VER == 0x2
IdentifyPacketKey SyncBlockExecOnePerTime::enter(unsigned short _packet_id) {
#else
IdentifyPacketKey SyncBlockExecOnePerTime::enter(uint32_t _uid, unsigned short _packet_id) {
#endif

#if STDA_BLOCK_PACKET_ONE_TIME_VER == 0x2
	IdentifyPacketKey ipk(m_uid, _packet_id);

	if (_packet_id == INVALID_PACKET_ID) {

		_smp::message_pool::getInstance().push(new message("[SyncBlockExecOnePerTime::enter][Error] Player[UID=" + std::to_string(m_uid)
				+ "] _packet_id(" + std::to_string(_packet_id) + ") is invalid.", CL_FILE_LOG_AND_CONSOLE));

		return ipk;
	}
#else
	IdentifyPacketKey ipk(_uid, _packet_id);

	if (_packet_id == INVALID_PACKET_ID) {

		_smp::message_pool::getInstance().push(new message("[SyncBlockExecOnePerTime::enter][Error] Player[UID=" + std::to_string(_uid)
				+ "] _packet_id(" + std::to_string(_packet_id) + ") is invalid.", CL_FILE_LOG_AND_CONSOLE));

		return ipk;
	}
#endif

	try {

		DWORD err = 0;

		m_ts_cs.lock();

#if STDA_BLOCK_PACKET_ONE_TIME_VER == 0x2
		EventCV *hEvent = &m_mp_player;
#else
		EventCV *hEvent = getPlayer(_uid);

		if (hEvent == nullptr) {

			m_ts_cs.unlock();

			_smp::message_pool::getInstance().push(new message("[SyncBlockExecOnePerTime::enter][Error] Nao conseguiu pegar o player[UID=" + std::to_string(_uid)
					+ "] Packet[ID=" + std::to_string(_packet_id) + "].", CL_FILE_LOG_AND_CONSOLE));

			return ipk;
		}
#endif

		auto it = m_mp_event_block.find(ipk);

		if (it != m_mp_event_block.end()) {

			if (it->second.active && it->second.thread_id !=
#if defined(_WIN32)
				GetCurrentThreadId()
#elif defined(__linux__)
				gettid()
#endif
			)
				hEvent->waitEvent(m_ts_cs, _packet_id);

			it->second.active = true;
#if defined(_WIN32)
			it->second.thread_id = GetCurrentThreadId();
#elif defined(__linux__)
			it->second.thread_id = gettid();
#endif

			m_ts_cs.unlock();

			return ipk;

		}else {

			auto it_insert = m_mp_event_block.insert(std::make_pair(ipk, stState{ true,
#if defined(_WIN32)
				GetCurrentThreadId()
#elif defined(__linux__)
				(uint32_t)gettid()
#endif
			}));

			if (!it_insert.second && it_insert.first == m_mp_event_block.end())
				_smp::message_pool::getInstance().push(new message("[SyncBlockExeOnePerTime::enter][Error] Player[UID=" + std::to_string(ipk.m_key._stkey.uid)
						+ "] Packet[ID=" + std::to_string(_packet_id) + "] nao conseguiu inserir o block no map.", CL_FILE_LOG_AND_CONSOLE));
		}

		m_ts_cs.unlock();

	}catch (exception& e) {

		m_ts_cs.unlock();

		_smp::message_pool::getInstance().push(new message("[SyncBlockExecOnePerTime::enter][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	return ipk;
}

void SyncBlockExecOnePerTime::leave(IdentifyPacketKey* _ipk) {

	if (_ipk == nullptr) {

		_smp::message_pool::getInstance().push(new message("[SyncBlockExecOnePerTime::leave][Error] _ipk is invalid(nullptr).", CL_FILE_LOG_AND_CONSOLE));

		return;
	}

	try {

		m_ts_cs.lock();

#if STDA_BLOCK_PACKET_ONE_TIME_VER == 0x2
		EventCV *hEvent = &m_mp_player;
#else
		EventCV *hEvent = getPlayer(_ipk->m_key._stkey.uid);

		if (hEvent == nullptr) {

			m_ts_cs.unlock();

			_smp::message_pool::getInstance().push(new message("[SyncBlockExecOnePerTime::leave][Error] Nao conseguiu pegar o Player[UID=" + std::to_string(_ipk->m_key._stkey.uid)
					+ "] Packet[ID=" + std::to_string(_ipk->m_key._stkey.packet_id) + "].", CL_FILE_LOG_AND_CONSOLE));

			return;
		}
#endif

		auto it = m_mp_event_block.find(*_ipk);

		if (it != m_mp_event_block.end()) {

			it->second.active = false;

			hEvent->setEvent((unsigned short)_ipk->m_key._stkey.packet_id);
		}

		m_ts_cs.unlock();

	}catch (exception& e) {

		m_ts_cs.unlock();

		_smp::message_pool::getInstance().push(new message("[SyncBlockExecOnePerTime::leave][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}

#if STDA_BLOCK_PACKET_ONE_TIME_VER == 0x1
EventCV* SyncBlockExecOnePerTime::getPlayer(uint32_t _uid) {

	EventCV* hEvent = nullptr;

	if (_uid == 0u) {

		_smp::message_pool::getInstance().push(new message("[SyncBlockExecOnePerTime::getPlayer][Error] Player[UID=" + std::to_string(_uid) + "] is invalid.", CL_FILE_LOG_AND_CONSOLE));

		return hEvent;
	}

	try {

		m_ts_cs.lock();

		auto it = m_mp_player.find(_uid);

		if (it == m_mp_player.end()) {

			auto it_insert = m_mp_player.insert(std::make_pair(_uid, EventCV()));

			if (!it_insert.second && it_insert.first == m_mp_player.end()) {

				hEvent = nullptr;

				_smp::message_pool::getInstance().push(new message("[SyncBlockExecOnePerTime::getPlayer][Error] Nao conseguiu inserir o player[UID="
						+ std::to_string(_uid) + "] no map.", CL_FILE_LOG_AND_CONSOLE));

			}else
				hEvent = &it_insert.first->second;

		}else
			hEvent = &it->second;

		m_ts_cs.unlock();

	}catch (exception& e) {

		m_ts_cs.unlock();

		_smp::message_pool::getInstance().push(new message("[SyncBlockExecOnePerTime::getPlayer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}

	return hEvent;
}

void SyncBlockExecOnePerTime::removePlayer(uint32_t _uid) {

	if (_uid == 0u) {

		_smp::message_pool::getInstance().push(new message("[SyncBlockExecOnePerTime::removePlayer][Error] Player[UID="
				+ std::to_string(_uid) + "] is invalid.", CL_FILE_LOG_AND_CONSOLE));

		return;
	}

	try {

		m_ts_cs.lock();

		auto it = m_mp_player.find(_uid);

		if (it != m_mp_player.end())
			m_mp_player.erase(it);
		else
			_smp::message_pool::getInstance().push(new message("[SyncBlockExecOnePerTime::removePlayer][Error] Nao tem o player[UID="
					+ std::to_string(_uid) + "] no map.", CL_FILE_LOG_AND_CONSOLE));

		m_ts_cs.unlock();

	}catch (exception& e) {

		m_ts_cs.unlock();

		_smp::message_pool::getInstance().push(new message("[SyncBlockExecOnePerTime::removePlayer][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));
	}
}
#endif

#if STDA_BLOCK_PACKET_ONE_TIME_VER == 0x2
SmartBlock::SmartBlock(SyncBlockExecOnePerTime* _sbeopt, unsigned short _packet_id) : m_sbeopt(_sbeopt), m_ipk{ 0u, 0u } {

	if (m_sbeopt != nullptr)
		m_ipk = m_sbeopt->enter(_packet_id);
}

SmartBlock::~SmartBlock() {

	if (m_sbeopt != nullptr)
		m_sbeopt->leave(&m_ipk);
}
#else
SmartBlock::SmartBlock(uint32_t _uid, unsigned short _packet_id)
	: m_ipk(sSyncBlockExecOnePerTime::getInstance().enter(_uid, _packet_id)) {
}

SmartBlock::~SmartBlock() {

	sSyncBlockExecOnePerTime::getInstance().leave(&m_ipk);
}
#endif
