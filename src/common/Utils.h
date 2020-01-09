#pragma once

#include <vector>
#include <stdint.h>
#include <string>

namespace vmc
{
	template<typename T>
	T clamp(T value, T min, T max)
	{
		if (value < min) {
			return min;
		}
		if (value > max) {
			return max;
		}
		return value;
	}

	std::vector<uint8_t> readBinaryFile(const std::string& path);
}