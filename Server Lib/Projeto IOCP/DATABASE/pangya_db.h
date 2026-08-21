
#pragma once
#ifndef _STDA_PANGYA_DB_H
#define _STDA_PANGYA_DB_H

#include "../TYPE/pangya_st.h"

#include "../TYPE/list_fifo.h"
#include "../TYPE/list_async.h"
#include "exec_query.h"

#define CLEAR_MYSQL_RES(_r) if ((_r) != nullptr) delete (_r); \
							(_r) = nullptr; \

#define CLEAR_ALL_MYSQL_RES(_r) if ((_r) != nullptr) delete (_r); \
							(_r) = nullptr; \

#define BEGIN_RESULT_READ(column, code) if (r != nullptr && r->getNumResultSet() > 0) { \
											for (auto num_result = 0u; num_result < r->getNumResultSet(); ++num_result) { \
												if (r->getResultSetAt(num_result) != nullptr && r->getResultSetAt(num_result)->getNumLines() > 0 && r->getResultSetAt(num_result)->getState() == result_set::HAVE_DATA) { \
													for (auto _result = r->getResultSetAt(num_result)->getFirstLine(); _result != nullptr; _result = _result->next) { \
														if ((column) != 0 && _result->cols != (column)) { \
															CLEAR_ALL_MYSQL_RES(r); \
															throw exception("Numero de colunas solicitadas sao diferentes do recuperado do banco de dados. BEGIN_RESULT_READ().", STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANGYA_DB, (code), 1)); \
														} \

#define END_RESULT_READ(msg, code) } \
								}else { \
									CLEAR_MYSQL_RES(r->getResultSetAt(num_result)); \
									 \
								} \
								CLEAR_MYSQL_RES(r->getResultSetAt(num_result)); \
								  \
							} \
						}else if (query.getType() == exec_query::_QUERY || query.getType() == exec_query::_PROCEDURE) \
							throw exception("Nao conseguiu recuperar " + std::string((msg)), STDA_MAKE_ERROR(STDA_ERROR_TYPE::PANGYA_DB, (code), 3)); \

#define MED_RESULT_READ(_query) pangya_base_db::m_query_pool.push((_query)); \
						DWORD wait_time = INFINITE; \
						while (1) { \
							try { \
								(_query)->waitEvent(wait_time); \
							}catch (exception& e) { \
								if (STDA_SOURCE_ERROR_DECODE(e.getCodeError()) == STDA_ERROR_TYPE::EXEC_QUERY) { \
									  \
									if (STDA_ERROR_DECODE(e.getCodeError()) == 7 && wait_time == INFINITE) { \
										wait_time = 1000;  \
										continue; \
									} \
									pangya_base_db::m_query_pool.remove((_query)); \
									throw; \
								}else \
									throw; \
							} \
							\
							break; \
						} \
						response *r = (_query)->getRes(); \

#define IFNULL(_func, _data) ((_data) == nullptr ? 0 : _func((_data)))

namespace stdA {

	class pangya_base_db {
		public:
			pangya_base_db();
			~pangya_base_db();

			static void register_server(ServerInfoEx& si);
			static std::vector< ServerInfo > getServerList();

		public:
			static list_fifo_asyc< exec_query > m_query_pool;
			static list_async< exec_query* > m_cache_query;

			static bool compare(exec_query* _query1, exec_query* _query2);
	};
}

#endif
