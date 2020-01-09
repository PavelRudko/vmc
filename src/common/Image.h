#pragma once

#include <stdint.h>
#include <string>
#include <glm/glm.hpp>

namespace vmc
{
	class Image
	{
	public:
		Image(const std::string& path);

		Image(const Image&) = delete;

		Image(Image&&) noexcept;

		~Image();

		uint32_t getWidth() const;

		uint32_t getHeight() const;

		uint32_t getChannels() const;

		const uint8_t* getData() const;

	private:
		uint8_t* data = nullptr;

		uint32_t width = 0;

		uint32_t height = 0;

		uint32_t channels = 0;
	};
}