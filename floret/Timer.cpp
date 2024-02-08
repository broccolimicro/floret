#include "Timer.h"

Timer::Timer() {
	begin = chrono::steady_clock::now();
}

Timer::~Timer() {
}

void Timer::reset() {
	begin = chrono::steady_clock::now();
}

float Timer::since() {
	return (float)(chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - begin).count())/1e6;
}
