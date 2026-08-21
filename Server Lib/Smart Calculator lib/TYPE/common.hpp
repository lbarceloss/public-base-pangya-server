
#pragma once
#ifndef _STDA_COMMON_HPP
#define _STDA_COMMON_HPP

#include <vector>
#include <cstring>

namespace stdA {

	enum class ePACKET : unsigned char {
		LOGIN,
		HAND_SHAKE,
		COMMAND_SERVER,
		REPLY_SERVER,
		REPLY_SMART,
		REPLY_STADIUM,
		COMMAND_SMART_PING,
		COMMAND_SMART_LIST_MACRO,
		COMMAND_SMART_CALC,
		COMMAND_SMART_MACRO,
		COMMAND_SMART_EXPRESSION,
		COMMAND_STADIUM_PING,
		COMMAND_STADIUM_LIST_MACRO,
		COMMAND_STADIUM_LIST_CALC,
		COMMAND_STADIUM_OPEN_CALC,
		COMMAND_STADIUM_CALC,
		COMMAND_STADIUM_MACRO,
		COMMAND_STADIUM_EXPRESSION,
	};

	template <class _PK> class Packet {
		public:
			Packet(_PK _tipo) : m_tipo(_tipo) {};
			virtual ~Packet() {};

			template <typename _VL> friend Packet<_PK>& operator << (Packet<_PK>&_packet, const _VL& _data) {

				auto size = sizeof(_data);
				auto i = _packet.m_data.size();

				_packet.m_data.resize(i + size);

#if defined(_WIN32)
				memcpy_s(_packet.m_data.data() + i, size, &_data, size);
#elif defined(__linux__)
				memcpy(_packet.m_data.data() + i, &_data, size);
#endif

				return _packet;
			}

			template <typename _VL> friend Packet<_PK>& operator >> (Packet<_PK>& _packet, _VL& _data) {

				auto size = sizeof(_data);

				if (size > _packet.m_data.size())
					return _packet;

#if defined(_WIN32)
				memcpy_s(&_data, size, _packet.m_data.data(), size);
#elif defined(__linux__)
				memcpy(&_data, _packet.m_data.data(), size);
#endif

				_packet.m_data.erase(_packet.m_data.begin(), _packet.m_data.begin() + size);

				return _packet;
			}

			friend Packet<_PK>& operator << (Packet<_PK>& _packet, const std::string _data) {

				_packet.m_data.insert(_packet.m_data.end(), _data.begin(), _data.end());

				_packet.m_data.push_back(0);

				return _packet;
			}

			friend Packet<_PK>& operator >> (Packet<_PK>& _packet, std::string& _data) {

				auto size = strlen((const char*)_packet.m_data.data());

				_data.assign(_packet.m_data.begin(), _packet.m_data.begin() + size);

				_packet.m_data.erase(_packet.m_data.begin(), _packet.m_data.begin() + size + 1 );

				return _packet;
			}

			size_t read(void* _buff, size_t _size) {

				if (_size > m_data.size())
					return 0u;

#if defined(_WIN32)
				memcpy_s(_buff, _size, m_data.data(), _size);
#elif defined(__linux__)
				memcpy(_buff, m_data.data(), _size);
#endif

				m_data.erase(m_data.begin(), m_data.begin() + _size);

				return _size;
			}

			size_t write(void* _buff, size_t _size) {

				m_data.insert(m_data.end(), (unsigned char*)_buff, ((unsigned char*)_buff) + _size);

				return _size;
			}

			size_t size() const {
				return m_data.size();
			}

			std::vector< unsigned char > makePacket() {

				std::vector< unsigned char > raw;

				unsigned short size = (unsigned short)m_data.size();

				raw.assign((unsigned char*)&size, ((unsigned char*)&size) + sizeof(size));
				raw.push_back((unsigned char)m_tipo);
				raw.insert(raw.end(), m_data.begin(), m_data.end());

				return raw;
			}

		public:
			_PK m_tipo;
			std::vector< unsigned char > m_data;
	};
}

#endif
