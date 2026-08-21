
#pragma once
#ifndef _STDA_DROP_SYSTEM_HPP
#define _STDA_DROP_SYSTEM_HPP

#if defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#include <unistd.h>
#endif

#include <map>
#include <vector>
#include "../TYPE/game_type.hpp"
#include "../SESSION/player.hpp"
#include "../../Projeto IOCP/TYPE/singleton.h"

namespace stdA {
	class DropSystem {
		public:
			struct stDropCourse {
				stDropCourse(uint32_t _ul = 0u) {
					clear();
				};
				~stDropCourse() {};
				struct stDropItem {
					enum eTIPO : unsigned char {
						ALL_PROBABILITY,
						SEQUENCE_DROP,
						LAST_HOLE_PROBABILITY,
					};
					enum ePROB_TIPO : unsigned char {
						_3HOLES_ALL,
						_6HOLES_SEQUENCE,
						_9HOLES,
						_18HOLES,
					};
					void clear() { memset(this, 0, sizeof(stDropItem)); };
					uint32_t _typeid;
					unsigned char tipo;
					uint32_t qntd;
					uint32_t probabilidade[4];
					unsigned char active : 1;
				};
				void clear() {

					course = 0u;

					if (!v_item.empty()) {
						v_item.clear();
						v_item.shrink_to_fit();
					}
				};
				unsigned char course;
				std::vector< stDropItem > v_item;
			};

			struct stCourseInfo {
				void clear() { memset(this, 0, sizeof(stCourseInfo)); };
				unsigned char course;
				unsigned char hole;
				unsigned char seq_hole;
				unsigned char qntd_hole;
				uint32_t artefact;
				unsigned char char_motion : 1;
				unsigned char angel_wings : 2;
				uint32_t rate_drop;
			};

			struct stConfig {
				void clear() { memset(this, 0, sizeof(stConfig)); };
				uint32_t rate_mana_artefact;
				uint32_t rate_grand_prix_ticket;
				uint32_t rate_SSC_ticket;
			};

		public:
			DropSystem();
			virtual ~DropSystem();

			  void load();

			  bool isLoad();

			  DropItem drawArtefactPang(stCourseInfo& _ci, uint32_t _num_players);
			  std::vector< DropItem > drawCourse(stDropCourse& _dc, stCourseInfo& _ci);
			  DropItem drawManaArtefact(stCourseInfo& _ci);
			  DropItem drawGrandPrixTicket(stCourseInfo& _ci, player& _session);
			  std::vector< DropItem > drawSSCTicket(stCourseInfo& _ci);

			  stDropCourse* findCourse(unsigned char _course);

		protected:
			  void initialize();

			  void clear();

		private:
			  std::map< unsigned char, stDropCourse > m_course;

			  stConfig m_config;

			  bool m_load;

#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif
	};

	typedef Singleton< DropSystem > sDropSystem;
}

#endif
