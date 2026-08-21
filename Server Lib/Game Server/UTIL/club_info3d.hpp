
#pragma once
#ifndef _STDA_CLUB_INFO3D_HPP
#define _STDA_CLUB_INFO3D_HPP

#include "../TYPE/_3d_type.hpp"
#include "../../Projeto IOCP/TYPE/singleton.h"
#include <vector>

namespace stdA {

	class ClubInfo3D {

		public:
			ClubInfo3D(eCLUB_TYPE _type, float _rotation_spin, float _rotation_curve, float _power_factor, float _degree, float _power_base);
			virtual ~ClubInfo3D();

		public:
			eCLUB_TYPE m_type;

			float m_rotation_spin;
			float m_rotation_curve;
			float m_power_factor;
			float m_degree;
			float m_power_base;
	};

	class AllClubInfo3D {

		public:
			AllClubInfo3D();
			virtual ~AllClubInfo3D();

		public:
			std::vector< ClubInfo3D > m_clubs;
	};

	typedef Singleton< AllClubInfo3D > sAllClubInfo3D;
}

#endif
