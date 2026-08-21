
#pragma once
#ifndef _STDA_EXTRA_POWER_INTERFACE_HPP
#define _STDA_EXTRA_POWER_INTERFACE_HPP

namespace stdA {

	class IExtraPower {

		public:
			virtual float getTotal(unsigned char _psf) = 0;
	};
}

#endif
