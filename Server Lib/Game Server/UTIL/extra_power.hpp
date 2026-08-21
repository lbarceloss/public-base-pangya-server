
#pragma once
#ifndef _STDA_EXTRA_POWER_HPP
#define _STDA_EXTRA_POWER_HPP

#include "../TYPE/extra_power_interface.hpp"

#include <cstdint>

namespace stdA {

	class ExtraPower : public IExtraPower {

		public:
			struct stPowerQntd {
				public:
					int32_t m_auxpart;
					int32_t m_mascot;
					int32_t m_card;

				public:
					int32_t total() {
						return m_auxpart + m_mascot + m_card;
					};
			};

		public:
			ExtraPower();
			ExtraPower(stPowerQntd _drive, stPowerQntd _power_shot);
			virtual ~ExtraPower();

			stPowerQntd& getPowerDrive();
			stPowerQntd& getPowerShot();

			void setPowerDrive(stPowerQntd _drive);
			void setPowerShot(stPowerQntd _power_shot);

		public:
			virtual float getTotal(unsigned char _psf) override;

		private:

			stPowerQntd m_drive;

			stPowerQntd m_power_shot;
	};
}

#endif
