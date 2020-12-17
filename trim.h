// Original from: https://stackoverflow.com/a/217605
#pragma once
#ifndef INC_1091_PARALLEL_PROGRAMMING_1216_REPORT_TRIM_H
#define INC_1091_PARALLEL_PROGRAMMING_1216_REPORT_TRIM_H

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

// trim from start
static inline std::string &ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
									std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
						 std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
	return ltrim(rtrim(s));
}

#endif //INC_1091_PARALLEL_PROGRAMMING_1216_REPORT_TRIM_H
