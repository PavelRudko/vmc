#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

namespace vmc
{
	Image::Image(const std::string& path)
	{
		int w, h, c;
		data = stbi_load(path.c_str(), &w, &h, &c, 0);
		if (!data) {
			throw std::runtime_error("Cannot open image " + path + ".");
		}

		width = w;
		height = h;
		channels = c;
	}

	Image::Image(Image&& other) noexcept :
		data(other.data),
		width(other.width),
		height(other.height),
		channels(other.channels)
	{
		other.data = nullptr;
	}

	Image::~Image()
	{
		if (data) {
			stbi_image_free(data);
		}
	}

	uint32_t Image::getWidth() const
	{
		return width;
	}

	uint32_t Image::getHeight() const
	{
		return height;
	}

	uint32_t Image::getChannels() const
	{
		return channels;
	}

	const uint8_t* Image::getData() const
	{
		return data;
	}
}
