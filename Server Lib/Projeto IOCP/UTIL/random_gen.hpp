
#pragma once
#ifndef _STDA_RANDOM_GEN_HPP
#define _STDA_RANDOM_GEN_HPP

#if defined(__linux__)
#include "WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

#include "../TYPE/singleton.h"

#include <random>

namespace stdA {
    class RandomGen {
        public:
            RandomGen();
            virtual ~RandomGen();

			bool isGood();

		private:

			uint64_t _rDevice();

			uint64_t _rIbeMt19937_64_chrono();

			uint64_t _rIbeMt19937_64_rdevice();

			uint64_t _rDeviceRange(uint64_t _min, uint64_t _max);

			uint64_t _rIbeMt19937_64_chronoRange(uint64_t _min, uint64_t _max);

			uint64_t _rIbeMt19937_64_rdeviceRange(uint64_t _min, uint64_t _max);

		public:

			uint64_t rDevice();

			uint64_t rIbeMt19937_64_chrono();

			uint64_t rIbeMt19937_64_rdevice();

			uint64_t rDeviceRange(uint64_t _min, uint64_t _max);

			uint64_t rIbeMt19937_64_chronoRange(uint64_t _min, uint64_t _max);

			uint64_t rIbeMt19937_64_rdeviceRange(uint64_t _min, uint64_t _max);

		private:
			bool init();
			void destroy();

        private:
			std::random_device *m_rd;
			std::independent_bits_engine< std::mt19937_64, 64, std::uint_fast64_t > *m_ibe_mt19937_64;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif

			bool m_state;
    };

	typedef Singleton< RandomGen > sRandomGen;
}

#endif
