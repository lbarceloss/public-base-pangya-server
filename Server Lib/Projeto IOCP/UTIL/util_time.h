
#pragma once
#ifndef _STDA_UTIL_TIME_H
#define _STDA_UTIL_TIME_H

#include <cstdint>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include "WinPort.h"
#endif

#include <string>

#define STDA_10_MICRO_PER_MICRO		(10ll)
#define STDA_10_MICRO_PER_MILLI		(STDA_10_MICRO_PER_MICRO * 1000ll)
#define STDA_10_MICRO_PER_SEC		(STDA_10_MICRO_PER_MILLI * 1000ll)
#define STDA_10_MICRO_PER_MIN		(STDA_10_MICRO_PER_SEC * 60ll)
#define STDA_10_MICRO_PER_HOUR		(STDA_10_MICRO_PER_MIN * 60ll)
#define STDA_10_MICRO_PER_DAY		(STDA_10_MICRO_PER_HOUR * 24ll)

namespace stdA {

    int _translateDate(std::string date_src, SYSTEMTIME* date_dst);

	int _translateTime(std::string _date_src, SYSTEMTIME* _date_dst);

	int translateDateSystem(time_t time_unix, SYSTEMTIME* date_dst);
	int translateDateLocal(time_t time_unix, SYSTEMTIME* date_dst);

	std::string _formatDate(SYSTEMTIME& _date);

	std::string _formatTime(SYSTEMTIME& _date);

	std::string formatDateSystem(time_t _time_unix);
	std::string formatDateLocal(time_t _time_unix);

	time_t TzLocalUnixToUnixUTC(time_t _time_unix);
	SYSTEMTIME TzLocalUnixToSystemTime(time_t _time_unix);

	time_t TzLocalTimeToUnixUTC(SYSTEMTIME& _st);
	SYSTEMTIME TzLocalTimeToSystemTime(SYSTEMTIME& _st);

	time_t UnixUTCToTzLocalUnix(time_t _time_unix);
	SYSTEMTIME UnixUTCToTzLocalTime(time_t _time_unix);

	time_t SystemTimeToTzLocalUnix(SYSTEMTIME& _st);
	SYSTEMTIME SystemTimeToTzLocalTime(SYSTEMTIME& _st);

	time_t FileTimeToUnix(FILETIME ft);
	FILETIME UnixToFileTime(time_t time);

	time_t SystemTimeToUnix(SYSTEMTIME st);
	SYSTEMTIME UnixToSystemTime(time_t time);

	time_t GetLocalTimeAsUnix();
	time_t GetSystemTimeAsUnix();

	time_t StrToUnix(std::string _date_time);

	int64_t getHourDiff(SYSTEMTIME& _st1, SYSTEMTIME& _st2);

	int64_t getTimeDiff(SYSTEMTIME& _st1, SYSTEMTIME& _st2);

	int64_t getLocalTimeDiff(SYSTEMTIME& _st);

	int64_t getLocalTimeDiffDESC(SYSTEMTIME& _st);

	int64_t getSystemTimeDiff(SYSTEMTIME& _st);

	int64_t getSystemTimeDiffDESC(SYSTEMTIME& _st);

	int64_t getDateDiff(SYSTEMTIME& _st1, SYSTEMTIME& _st2);

	int64_t getLocalDateDiff(SYSTEMTIME& _st);

	int64_t getLocalDateDiffDESC(SYSTEMTIME& _st);

	int64_t getSystemDateDiff(SYSTEMTIME& _st);

	int64_t getSystemDateDiffDESC(SYSTEMTIME& _st);

	bool isSameDay(SYSTEMTIME& _st1, SYSTEMTIME& _st2);

	bool isSameDayNow(SYSTEMTIME& _st);

	bool isEmpty(SYSTEMTIME& _st);
}

#endif
