#pragma once

#include <vk/VulkanBuffer.h>

namespace vmc
{
    class Mesh
    {
    public:
        Mesh(VulkanBuffer&& vertexBuffer, VulkanBuffer&& indexBuffer, uint32_t indicesCount);

        Mesh(const Mesh&) = delete;

        Mesh(Mesh&& other) noexcept;

        ~Mesh() = default;

        Mesh& operator=(const Mesh&) = delete;

        Mesh& operator=(Mesh&&) = delete;

        const VulkanBuffer& getVertexBuffer() const;

        const VulkanBuffer& getIndexBuffer() const;

        uint32_t getIndicesCount() const;

    private:
        uint32_t indicesCount = 0;

        VulkanBuffer vertexBuffer;

        VulkanBuffer indexBuffer;
    };
}