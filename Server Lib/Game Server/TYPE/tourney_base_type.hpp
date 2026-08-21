
#pragma once
#ifndef _STDA_TOURNEY_BASE_TYPE_HPP
#define _STDA_TOURNEY_BASE_TYPE_HPP

#include <memory>

namespace stdA {

#if defined(__linux__)
#pragma pack(1)
#endif

	struct Medal {
		Medal(uint32_t _ul = 0u) {
			clear();
		};
		void clear() {
			oid = -1;
			item_typeid = 0u;
		};
		int32_t oid;
		uint32_t item_typeid;
	};

#if defined(__linux__)
#pragma pack()
#endif
}

#endif
