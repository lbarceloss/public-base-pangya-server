
#pragma once
#ifndef _STDA_PAPEL_SHOP_TYPE_HPP
#define _STDA_PAPEL_SHOP_TYPE_HPP

#include <memory>
#include "../../Projeto IOCP/UTIL/util_time.h"

#include <memory.h>

namespace stdA {

#if defined(__linux__)
#pragma pack(1)
#endif

	enum PAPEL_SHOP_TYPE : unsigned char {
		PST_COMMUN,
		PST_COOKIE,
		PST_RARE
	};

	enum PAPEL_SHOP_BALL_COLOR : unsigned char {
		PSBC_BLUE,
		PSBC_GREEN,
		PSBC_RED,
	};

	struct ctx_papel_shop {
		void clear() { memset(this, 0, sizeof(ctx_papel_shop)); };
		std::string toString() {
			return "NUMERO=" + std::to_string(numero) + ", PRICE_NORMAL=" + std::to_string(price_normal)
					+ ", PRICE_BIG=" + std::to_string(price_big) + ", LIMITTED_PER_DAY=" + std::to_string((unsigned short)limitted_per_day)
					+ ", UPDATE_DATE=" + _formatDate(update_date);
		};
		uint32_t numero;
		uint64_t price_normal;
		uint64_t price_big;
		unsigned char limitted_per_day : 1;
		SYSTEMTIME update_date;
	};

	struct ctx_papel_shop_item {
		void clear() { memset(this, 0, sizeof(ctx_papel_shop_item)); };
		uint32_t _typeid;
		uint32_t probabilidade;
		int32_t numero;
		PAPEL_SHOP_TYPE tipo;
		unsigned char active : 1;
	};

	struct ctx_papel_shop_ball {
		ctx_papel_shop_ball(uint32_t _ul = 0u) {
			clear();
		};
		void clear() { memset(this, 0, sizeof(ctx_papel_shop_ball)); };
		PAPEL_SHOP_BALL_COLOR color;
		ctx_papel_shop_item ctx_psi;
		uint32_t qntd;
		void* item;
	};

	struct ctx_papel_shop_coupon {
		void clear() { memset(this, 0, sizeof(ctx_papel_shop_coupon)); }
		uint32_t _typeid;
		unsigned char active : 1;
	};

#if defined(__linux__)
#pragma pack()
#endif
}

#endif
