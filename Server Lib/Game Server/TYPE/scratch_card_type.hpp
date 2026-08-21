
#pragma once
#ifndef _STDA_SCRATCH_CARD_TYPE_HPP
#define _STDA_SCRATCH_CARD_TYPE_HPP

#include <cstdint>
#include <cstring>

namespace stdA {

	enum SCRATCH_CARD_TYPE {
		SCT_NORMAL = 0,
		SCT_COOKIE = 1,
		SCT_RARE   = 2
	};

	struct ctx_scratch_card_item {
		uint32_t      _typeid;
		int           numero;
		uint32_t      qntd;
		uint32_t      probabilidade;
		int           tipo;
		unsigned char active;
	};

	struct ctx_scratch_card_item_win {
		ctx_scratch_card_item ctx_psi;
		uint32_t              qntd;
		void*                 item;

		void clear() { memset(this, 0, sizeof(*this)); }
	};
}

#endif
