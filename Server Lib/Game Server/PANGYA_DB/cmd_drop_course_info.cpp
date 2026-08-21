
#if defined(_WIN32)
#pragma pack(1)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#endif

#include "cmd_drop_course_info.hpp"

using namespace stdA;

CmdDropCourseInfo::CmdDropCourseInfo(bool _waiter) : pangya_db(_waiter), m_course() {
}

CmdDropCourseInfo::~CmdDropCourseInfo() {
}

void CmdDropCourseInfo::lineResult(result_set::ctx_res *_result, uint32_t  ) {

	checkColumnNumber(9, (uint32_t)_result->cols);

	std::map< unsigned char, DropSystem::stDropCourse >::iterator i;

	DropSystem::stDropCourse dc{ 0 };
	DropSystem::stDropCourse::stDropItem di{ 0 };

	dc.course = (unsigned char)IFNULL(atoi, _result->data[0]);

	di.tipo = (unsigned char)IFNULL(atoi, _result->data[1]);
	di._typeid = (uint32_t)IFNULL(atoi, _result->data[2]);
	di.qntd = (uint32_t)IFNULL(atoi, _result->data[3]);

	for (auto i = 0u; i < 4; ++i)
		di.probabilidade[i] = (uint32_t)IFNULL(atoi, _result->data[4 + i]);

	di.active = (unsigned char)IFNULL(atoi, _result->data[8]);

	if ((i = m_course.find(dc.course)) == m_course.end()) {

		dc.v_item.push_back(di);

		m_course.insert(std::make_pair(dc.course, dc));
	}else
		i->second.v_item.push_back(di);
}

response* CmdDropCourseInfo::prepareConsulta(database& _db) {

	if (!m_course.empty())
		m_course.clear();

	auto r = consulta(_db, m_szConsulta);

	checkResponse(r, "nao conseguiu pegar os Drop Course");

	return r;
}

std::map< unsigned char, DropSystem::stDropCourse >& CmdDropCourseInfo::getInfo() {
	return m_course;
}
