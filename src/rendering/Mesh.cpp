#include "Mesh.h"

namespace vmc
{
    Mesh::Mesh(VulkanBuffer&& vertexBuffer, VulkanBuffer&& indexBuffer, uint32_t indicesCount) :
        vertexBuffer(std::move(vertexBuffer)),
        indexBuffer(std::move(indexBuffer)),
        indicesCount(indicesCount)
    {
    }

    Mesh::Mesh(Mesh&& other) noexcept :
        vertexBuffer(std::move(other.vertexBuffer)),
        indexBuffer(std::move(other.indexBuffer)),
        indicesCount(other.indicesCount)
    {
    }

    const VulkanBuffer& Mesh::getVertexBuffer() const
    {
        return vertexBuffer;
    }

    const VulkanBuffer& Mesh::getIndexBuffer() const
    {
        return indexBuffer;
    }

    uint32_t Mesh::getIndicesCount() const
    {
        return indicesCount;
    }
}