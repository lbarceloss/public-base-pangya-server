
#if defined(_WIN32)
#pragma pack(1)
#endif

#include "exception.h"
#include "../../Projeto IOCP/UTIL/string_util.hpp"

using namespace stdA;

exception::exception(std::string message_error, uint32_t code_error) {
	m_message_error = message_error;
	m_code_error = code_error;

	m_message_error_full = m_message_error + "\t Error Code: " + std::to_string(m_code_error);
};

exception::exception(std::wstring message_error, uint32_t code_error) {
	m_code_error = code_error;

	m_message_error = WcToMb(message_error);

	m_message_error_full = m_message_error + "\t Error Code: " + std::to_string(m_code_error);
};

exception::~exception() {
	m_message_error.shrink_to_fit();
	m_code_error = 0;
};

const std::string exception::getMessageError() {
	return m_message_error;
};

const uint32_t exception::getCodeError() {
	return m_code_error;
};

const std::string exception::getFullMessageError() {
	return m_message_error_full;
};
