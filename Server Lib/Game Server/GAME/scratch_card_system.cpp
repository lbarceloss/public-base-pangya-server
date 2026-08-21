
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "scratch_card_system.hpp"

#include "../../Projeto IOCP/UTIL/exception.h"
#include "../../Projeto IOCP/TYPE/stda_error.h"
#include "../../Projeto IOCP/UTIL/message_pool.h"

#include "../UTIL/lottery.hpp"
#include "../PANGYA_DB/cmd_scratchy_item.hpp"
#include "../../Projeto IOCP/DATABASE/normal_manager_db.hpp"

using namespace stdA;

static const uint32_t scratch_card_coupon_typeid[] = {
	436207779u, 436207664u, 436207667u, 436207668u
};

ScratchCardSystem::ScratchCardSystem() : m_load(false), m_ctx_psi{} {
#if defined(_WIN32)
	InitializeCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_init(&m_cs, nullptr);
#endif
}

ScratchCardSystem::~ScratchCardSystem() {
#if defined(_WIN32)
	DeleteCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_destroy(&m_cs);
#endif
}

void ScratchCardSystem::load() {

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	try {

		m_load = false;
		m_ctx_psi.clear();

		CmdScratchyItem cmd(true);

		snmdb::NormalManagerDB::getInstance().add(0, &cmd, nullptr, nullptr);

		cmd.waitEvent();

		if (cmd.getException().getCodeError() != 0)
			throw cmd.getException();

		m_ctx_psi = cmd.getInfo();

		if (m_ctx_psi.empty())
			_smp::message_pool::getInstance().push(new message("[ScratchCardSystem::load][Log] scratchy_item vazia; a raspadinha nao vai sortear nada.", CL_FILE_LOG_AND_CONSOLE));

		m_load = true;

	}
	catch (exception& e) {

#if defined(_WIN32)
		LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
		pthread_mutex_unlock(&m_cs);
#endif

		_smp::message_pool::getInstance().push(new message("[ScratchCardSystem::load][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		return;
	}

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif
}

bool ScratchCardSystem::isLoad() {

	bool r = false;

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	r = (m_load && !m_ctx_psi.empty());

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif

	return r;
}

WarehouseItemEx* ScratchCardSystem::hasCoupon(player& _session) {

	WarehouseItemEx* pWi = nullptr;

	for (auto t : scratch_card_coupon_typeid) {

		if ((pWi = _session.m_pi.findWarehouseItemByTypeid(t)) != nullptr)
			return pWi;
	}

	return nullptr;
}

std::vector< ctx_scratch_card_item_win > ScratchCardSystem::Play(player& _session) {

	std::vector< ctx_scratch_card_item_win > v_item;

	ctx_scratch_card_item_win ctx_b{ 0 };

#if defined(_WIN32)
	EnterCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_lock(&m_cs);
#endif

	try {

		Lottery lottery((uint64_t)&m_ctx_psi);

		for (auto& el : m_ctx_psi)
			if (el.active && (el.numero == -1 || el.numero == 0))
				lottery.push(el.probabilidade, (size_t)&el);

		Lottery::LotteryCtx* lc = lottery.spinRoleta();

		if (lc == nullptr)
			throw exception("[ScratchCardSystem::Play][Error] nao conseguiu sortear item da raspadinha.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::LOTTERY, 1, 0));

		auto ctx_psi = (ctx_scratch_card_item*)lc->value;

		ctx_b.clear();
		ctx_b.ctx_psi = *ctx_psi;
		ctx_b.qntd = (ctx_psi->qntd == 0 ? 1u : ctx_psi->qntd);
		ctx_b.ctx_psi.qntd = ctx_b.qntd;

		v_item.push_back(ctx_b);

	}
	catch (exception& e) {

#if defined(_WIN32)
		LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
		pthread_mutex_unlock(&m_cs);
#endif

		_smp::message_pool::getInstance().push(new message("[ScratchCardSystem::Play][ErrorSystem] " + e.getFullMessageError(), CL_FILE_LOG_AND_CONSOLE));

		return v_item;
	}

#if defined(_WIN32)
	LeaveCriticalSection(&m_cs);
#elif defined(__linux__)
	pthread_mutex_unlock(&m_cs);
#endif

	return v_item;
}
