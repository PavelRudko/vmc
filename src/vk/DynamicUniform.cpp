#include "DynamicUniform.h"
#include <stdexcept>

namespace vmc
{
	VkDeviceSize calculateAlignment(const VulkanDevice& device, VkDeviceSize elementSize)
	{
		auto minAlignment = device.getProperties().limits.minUniformBufferOffsetAlignment;
		VkDeviceSize alignment = elementSize;
		if (minAlignment > 0) {
			alignment = (alignment + minAlignment - 1) & ~(minAlignment - 1);
		}

		return alignment;
	}

	DynamicUniform::DynamicUniform(const VulkanDevice& device, DescriptorPool& descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkDeviceSize elementSize, uint32_t maxElements) :
		alignment(calculateAlignment(device, elementSize)),
		buffer(device, alignment* maxElements, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY),
		elementSize(elementSize)
	{
		mappedData = buffer.map();
		descriptorSet = descriptorPool.allocate(descriptorSetLayout);

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buffer.getHandle();
		bufferInfo.range = alignment;
		bufferInfo.offset = 0;

		VkWriteDescriptorSet writeInfo{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writeInfo.descriptorCount = 1;
		writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		writeInfo.dstBinding = 0;
		writeInfo.pBufferInfo = &bufferInfo;
		writeInfo.dstSet = descriptorSet;

		vkUpdateDescriptorSets(device.getHandle(), 1, &writeInfo, 0, nullptr);
	}

	DynamicUniform::DynamicUniform(DynamicUniform&& other) noexcept :
		buffer(std::move(other.buffer)),
		offset(other.offset),
		mappedData(other.mappedData),
		elementSize(other.elementSize),
		alignment(other.alignment),
		descriptorSet(other.descriptorSet)
	{
		other.mappedData = nullptr;
	}

	DynamicUniform::~DynamicUniform()
	{
		if (mappedData) {
			buffer.unmap();
		}
	}

	void DynamicUniform::reset()
	{
		offset = 0;
	}

	VkDeviceSize DynamicUniform::pushData(const void* data)
	{
		if (offset + alignment >= buffer.getSize()) {
			throw std::runtime_error("Dynamic uniform is out of size.");
		}

		VkDeviceSize elementOffset = offset;
		memcpy((uint8_t*)mappedData + offset, data, elementSize);
		offset += alignment;
		return elementOffset;
	}

	void DynamicUniform::flush()
	{
		buffer.flush();
	}

	VkDescriptorSet DynamicUniform::getDescriptorSet() const
	{
		return descriptorSet;
	}
}