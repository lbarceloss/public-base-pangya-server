
#pragma once
#ifndef _STDA_IPHELP_H
#define _STDA_IPHELP_H

#include <vector>
#include <iphlpapi.h>

namespace stdA {
	class ip_help {
		public:
			static std::vector< IP_ADAPTER_INFO > getVectorAdaptersInfo();
			static std::string toPad(const std::string _s, size_t _num, char _ch_pad = ' ');
			static std::string makeMACString(BYTE *_buff, ULONG _size);
	};
}

#endif
