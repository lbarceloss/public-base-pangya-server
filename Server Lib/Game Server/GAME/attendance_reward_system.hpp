
#pragma once
#ifndef _STDA_ATTENDANCE_REWARD_SYSTEM_HPP
#define _STDA_ATTENDANCE_REWARD_SYSTEM_HPP

#if defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

#include "../SESSION/player.hpp"
#include "../../Projeto IOCP/PACKET/packet.h"
#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"
#include "../../Projeto IOCP/TYPE/singleton.h"

#include <vector>

namespace stdA {
	class AttendanceRewardSystem {
		public:
			AttendanceRewardSystem();
			virtual ~AttendanceRewardSystem();

			  void load();

			  bool isLoad();

			  void requestCheckAttendance(player& _session, packet *_packet);
			  void requestUpdateCountLogin(player& _session, packet *_packet);

		protected:
			  void initialize();

			  void clear();

			  void sendGrandPrixTicket(player& _session);

			  AttendanceRewardItemCtx* drawReward(unsigned char _tipo);

			  bool passedOneDay(player& _session);

			static void SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg);

		private:
			  std::vector< AttendanceRewardItemCtx > v_item;

			  bool m_load;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif
	};

	typedef Singleton< AttendanceRewardSystem > sAttendanceRewardSystem;
}

#endif
