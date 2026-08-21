
#pragma once
#ifndef _STDA_BOX_SYSTEM_HPP
#define _STDA_BOX_SYSTEM_HPP

#if defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#endif

#include "../TYPE/box_type.hpp"
#include "../SESSION/player.hpp"

#include <map>
#include <vector>

#ifndef SPINNING_CUBE_TYPEID
#define SPINNING_CUBE_TYPEID 0x1A00015B
#endif

#define OPENNED_SPINNING_CUBE_TYPEID 0x1A000161
#define KEY_OF_SPINNING_CUBE_TYPEID 0x1A00015C

#define PAPEL_BOX_TYPEID 0x1A000208

#ifndef PANG_POUCH_TYPEID
#define PANG_POUCH_TYPEID 0x1A000010
#endif

namespace stdA {
	class BoxSystem {
		public:
			BoxSystem();
			virtual ~BoxSystem();

			  void load();

			  bool isLoad();

			  ctx_box* findBox(uint32_t _typeid);

			  ctx_box_item* drawBox(player& _session, ctx_box& _ctx_b);

		protected:
			  void initialize();

			  void clear();

		private:
			  std::map< uint32_t, ctx_box > m_box;

			  bool m_load;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif
	};

	typedef Singleton< BoxSystem > sBoxSystem;
}

#endif
