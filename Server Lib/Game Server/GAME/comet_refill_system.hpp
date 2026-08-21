
#pragma once
#ifndef _STDA_COMET_REFILL_SYSTEM_HPP
#define _STDA_COMET_REFILL_SYSTEM_HPP

#if defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

#include "../TYPE/comet_refill_type.hpp"

#include "../../Projeto IOCP/TYPE/singleton.h"

#include <map>

namespace stdA {
	class CometRefillSystem {
		public:
			CometRefillSystem();
			virtual ~CometRefillSystem();

			  void load();

			  bool isLoad();

			  ctx_comet_refill* findCometRefill(uint32_t _typeid);

			  uint32_t drawsCometRefill(ctx_comet_refill& _ctx_cr);

		protected:
			  void initialize();

			  void clear();

		private:
			  std::map< uint32_t, ctx_comet_refill > m_comet_refill;

			  bool m_load;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif
	};

	typedef Singleton< CometRefillSystem > sCometRefillSystem;
}

#endif
