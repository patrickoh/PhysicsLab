#pragma once

#include "Common.h"
#include <string>
#include <map>

typedef long long int sint64;

static class QueryPerformance
{
	public:

		static sint64 ts,tf;

		static std::map<std::string, sint64> results;

		static void Start()
		{
			QueryPerformanceCounter((LARGE_INTEGER*)(&ts));
		}

		static void Finish(std::string resultName)
		{
			QueryPerformanceCounter((LARGE_INTEGER*)(&tf));
			results[resultName] = tf - ts;
		}
};
