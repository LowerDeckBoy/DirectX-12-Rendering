#pragma once

#include <chrono>

class TimeUtils {
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> Start{};
	std::chrono::time_point<std::chrono::high_resolution_clock> End{};
	std::chrono::duration<float, std::ratio<1, 1000>> Duration{};

public:
	auto Timer_Start() {
		Start = std::chrono::high_resolution_clock::now();
	}
	auto Timer_End(bool bPrint = false) {
		End = std::chrono::high_resolution_clock::now();
		Duration = End - Start;
		if (bPrint) {
			auto msg{ "Model load time:\t" + std::to_string(Duration.count()) + "ms\n" };
			::OutputDebugStringA(msg.c_str());
		}
	}
};