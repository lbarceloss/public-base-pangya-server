
#if defined(_WIN32)
#pragma pack(1)
#endif

#include "response.h"
#include "../UTIL/exception.h"
#include "../TYPE/stda_error.h"

using namespace stdA;

response::response() : m_rows_affected(0ll) {};

response::~response() {
    clear();
};

void response::clear() {
    while (!m_result_set.empty()) {
        if (m_result_set.front() != nullptr)
            delete m_result_set.front();

		m_result_set.erase(m_result_set.begin());
    }

    m_result_set.shrink_to_fit();
};

void response::addResultSet(result_set *_result_set) {

    m_result_set.push_back(_result_set);
};

size_t response::getNumResultSet() {
    return m_result_set.size();
};

result_set*& response::getResultSetAt(size_t _index) {
    if ((int)_index < 0 || _index >= m_result_set.size())
        throw exception("Index out of range.", STDA_MAKE_ERROR(STDA_ERROR_TYPE::_RESPONSE, 1, 0));

    return m_result_set[_index];
};

void response::setRowsAffected(int64_t _rows_affected) {
	m_rows_affected = _rows_affected;
};

int64_t response::getRowsAffected() {
	return m_rows_affected;
};
