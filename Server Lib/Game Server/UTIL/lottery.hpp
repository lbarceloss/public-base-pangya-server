
#pragma once
#ifndef _STDA_LOTTERY_HPP
#define _STDA_LOTTERY_HPP

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#endif

#include <vector>
#include <map>

#include <memory.h>

namespace stdA {
	class Lottery {
		public:
			struct LotteryCtx {
				void clear() { memset(this, 0, sizeof(LotteryCtx)); };
				uint32_t prob;
				size_t value;
				uint64_t offset[2];
				unsigned char active : 1, : 0;
			};

		public:
			Lottery(uint64_t _value_rand);
			~Lottery();

			void clear();

			void push(LotteryCtx& _lc);
			void push(uint32_t _prob, size_t _value);

			uint64_t getLimitProbilidade();

			uint32_t getCountItem();

			LotteryCtx* spinRoleta(bool _remove_item_draw = false);

		protected:
			void initialize(uint64_t _value_rand);

			void fill_roleta();
			void clear_roleta();

			void remove_draw_item(LotteryCtx* _lc);

			void shuffle_values_rand();

		private:
			std::map< uint64_t, LotteryCtx* > m_roleta;

			std::vector< LotteryCtx > m_ctx;
			std::vector< uint64_t > m_rand_values;

			uint64_t m_prob_limit;
	};
}

#endif
