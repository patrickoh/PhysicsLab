#pragma once

#include "Common.h"
#include <string>

typedef long long int sint64;

static class QueryPerformance
{
	public:

		static sint64 ts,tf;

		static void Start()
		{
			QueryPerformanceCounter((LARGE_INTEGER*)(&ts));
		}

		static void Finish()
		{
			QueryPerformanceCounter((LARGE_INTEGER*)(&tf));
		}

		static std::string Result()
		{
			std::stringstream ss;
			ss << (tf - ts);
			return ss.str();
		}
};
