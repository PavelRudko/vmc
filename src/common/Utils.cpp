#include "Utils.h"
#include <fstream>
#include <stdexcept>

namespace vmc
{
	std::vector<uint8_t> readBinaryFile(const std::string& path)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			throw std::runtime_error("Cannot open file " + path + ".");
		}

		size_t size = file.tellg();
		std::vector<uint8_t> data(size);
		file.seekg(0);
		file.read((char*)data.data(), size);
		file.close();

		return data;
	}
}