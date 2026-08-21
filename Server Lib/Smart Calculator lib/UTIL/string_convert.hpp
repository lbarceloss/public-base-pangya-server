
#pragma once
#ifndef _STDA_STRING_CONVERT_HPP
#define _STDA_STRING_CONVERT_HPP

#include <string>

namespace stdA {

	inline void copyStrToSzString(std::string _str, char* _szStr) {

		if (_szStr == nullptr)
			return;

		if (_str.empty())
			return;

#if defined(_WIN32)
		memcpy_s(_szStr, _str.length(), _str.data(), _str.length());
#elif defined(__linux__)
		memcpy(_szStr, _str.data(), _str.length());
#endif

		_szStr[_str.length()] = '\0';
	};
}

#endif
