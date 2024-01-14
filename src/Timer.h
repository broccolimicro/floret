#pragma once

#include <chrono>

using namespace std;

struct Timer {
	Timer();
	~Timer();

	chrono::steady_clock::time_point begin;

	void reset();
	float since();
};
