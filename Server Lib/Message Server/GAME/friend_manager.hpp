
#pragma once
#ifndef _STDA_FRIEND_MANAGER_HPP
#define _STDA_FRIEND_MANAGER_HPP

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include "../../Projeto IOCP/UTIL/WinPort.h"
#include <pthread.h>
#endif

#include "../TYPE/pangya_message_st.hpp"
#include <map>
#include <vector>

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"

namespace stdA {
	class FriendManager {
		public:
			explicit FriendManager();
			FriendManager(player_info& _pi);
			virtual ~FriendManager();

			virtual void init(player_info& _pi);
			virtual void clear();

			bool isInitialized();

			uint32_t countAllFriend();
			uint32_t countGuildMember();
			uint32_t countFriend();

			void requestAddFriend(FriendInfoEx& _fi);

			void requestDeleteFriend(FriendInfoEx& _fi);
			void requestDeleteFriend(uint32_t _uid);

			void requestUpdateFriendInfo(FriendInfoEx& _fi);

			void addFriend(FriendInfoEx& _fi);

			void deleteFriend(FriendInfoEx& _fi);
			void deleteFriend(uint32_t _uid);

			FriendInfoEx* findFriendInAllFriend(uint32_t _uid);
			FriendInfoEx* findGuildMember(uint32_t _uid);
			FriendInfoEx* findFriend(uint32_t _uid);

			std::vector< FriendInfoEx* > getAllFriend(bool _block = false);
			std::vector< FriendInfoEx* > getAllGuildMember();
			std::vector< FriendInfoEx* > getAllFriendAndGuildMember(bool _block = false);

		protected:
			std::map< uint32_t, FriendInfoEx > m_friend;
			player_info m_pi;

			static void SQLDBResponse(uint32_t _msg_id, pangya_db& _pangya_db, void* _arg);

		private:
#if defined(_WIN32)
			CRITICAL_SECTION m_cs;
#elif defined(__linux__)
			pthread_mutex_t m_cs;
#endif

			bool m_state;
	};
}

#endif
