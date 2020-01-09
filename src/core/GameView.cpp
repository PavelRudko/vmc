#include "GameView.h"
#include <common/Utils.h>
#include <stdexcept>
#include <core/Application.h>
#include <vk/ShaderModule.h>
#include <glm/gtc/matrix_transform.hpp>
#include <common/Image.h>

namespace vmc
{
	GameView::GameView(Application& application) :
		View(application)
	{
		initTexture();
		initPipeline();
		initBuffers();
	}

	GameView::~GameView()
	{
		if (sampler != VK_NULL_HANDLE) {
			vkDestroySampler(application.getDevice().getHandle(), sampler, nullptr);
		}
	}

	void GameView::update(float timeDelta)
	{
	}

	void GameView::render(RenderContext& renderContext)
	{
		auto& uniform = renderContext.getMVPUniform();
		auto uniformDescriptorSet = uniform.getDescriptorSet();

		auto viewMatrix = glm::lookAt(glm::vec3(0, 0, 2.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1.0f, 0));
		auto projectionMatrix = glm::perspective(glm::pi<float>() / 2.0f, (float)renderContext.getWidth() / renderContext.getHeight(), 0.01f, 1000.0f);

		auto commandBuffer = renderContext.startFrame({ 0.8f, 0.9f, 1.0f, 1.0f });

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline->getHandle());

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline->getLayout(), 1, 1, &imageDescriptor, 0, nullptr);

		for (uint32_t i = 0; i < 2; i++) {
			auto modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(i == 0 ? -1 : 1, 0, 0));
			auto mvp = projectionMatrix * viewMatrix * modelMatrix;

			uint32_t uniformOffset = uniform.pushData(&mvp);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline->getLayout(), 0, 1, &uniformDescriptorSet, 1, &uniformOffset);

			VkBuffer vertexBufferHandle = vertexBuffer->getHandle();
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBufferHandle, &offset);

			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
		}

		renderContext.endFrame();
	}

	void GameView::initPipeline()
	{
		auto vertexShaderData = readBinaryFile("data/shaders/default.vert.spv");
		auto fragmentShaderData = readBinaryFile("data/shaders/default.frag.spv");

		VulkanShaderModule vertexShader(application.getDevice(), vertexShaderData, VK_SHADER_STAGE_VERTEX_BIT);
		VulkanShaderModule fragmentShader(application.getDevice(), fragmentShaderData, VK_SHADER_STAGE_FRAGMENT_BIT);

		VkVertexInputBindingDescription colorVertexBinding{};
		colorVertexBinding.binding = 0;
		colorVertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		colorVertexBinding.stride = sizeof(ColorVertex);

		VkVertexInputAttributeDescription positionAttribute{};
		positionAttribute.binding = 0;
		positionAttribute.location = 0;
		positionAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		positionAttribute.offset = offsetof(ColorVertex, position);

		VkVertexInputAttributeDescription colorAttribute{};
		colorAttribute.binding = 0;
		colorAttribute.location = 1;
		colorAttribute.format = VK_FORMAT_R32G32_SFLOAT;
		colorAttribute.offset = offsetof(ColorVertex, uv);

		RenderPipelineDescription pipelineDescription;
		pipelineDescription.renderPass = application.getRenderPass().getHandle();
		pipelineDescription.subpass = 0;

		pipelineDescription.shaderModules.push_back(std::move(vertexShader));
		pipelineDescription.shaderModules.push_back(std::move(fragmentShader));
		
		pipelineDescription.vertexBindings.push_back(colorVertexBinding);
		pipelineDescription.vertexAttributes.push_back(positionAttribute);
		pipelineDescription.vertexAttributes.push_back(colorAttribute);

		pipelineDescription.descriptorSetLayouts.push_back(application.getMVPLayout().getHandle());
		pipelineDescription.descriptorSetLayouts.push_back(samplerLayout->getHandle());

		defaultPipeline = std::make_unique<RenderPipeline>(application.getDevice(), pipelineDescription);
	}

	void GameView::initBuffers()
	{
		auto& stagingManager = application.getStagingManager();

		float s = 0.0625f / 2;

		std::vector<ColorVertex> vertices = {
			{{-0.5f, -0.5f, 0, 1}, {0.0f, 0.0f}},
	        {{0.5f, -0.5f, 0, 1},  {s, 0.0f}},
	        {{0.5f, 0.5f, 0, 1},   {s, s}},
	        {{-0.5f, 0.5f, 0, 1},  {0.0f, s}}
		};

		std::vector<uint32_t> indices = {
			0, 1, 2, 2, 3, 0
		};

		vertexBuffer = std::make_unique<VulkanBuffer>(application.getDevice(), vertices.size() * sizeof(ColorVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		indexBuffer = std::make_unique<VulkanBuffer>(application.getDevice(), indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		stagingManager.start();
		stagingManager.copyToBuffer(vertices.data(), *vertexBuffer, 0, vertexBuffer->getSize());
		stagingManager.copyToBuffer(indices.data(), *indexBuffer, 0, indexBuffer->getSize());
		stagingManager.flush();
	}

	void GameView::initTexture()
	{
		Image rawImage("data/images/main_atlas.png");

		image = std::make_unique<VulkanImage>(application.getDevice(),
			rawImage.getWidth(),
			rawImage.getHeight(),
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY);

		auto& stagingManager = application.getStagingManager();
		stagingManager.start();
		stagingManager.copyToImage(rawImage.getData(), *image, rawImage.getWidth(), rawImage.getHeight());
		stagingManager.flush();

		imageView = std::make_unique<VulkanImageView>(application.getDevice(), *image);

		VkSamplerCreateInfo samplerInfo { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_NEAREST;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;

		if (vkCreateSampler(application.getDevice().getHandle(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create sampler.");
		}

		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		samplerLayout =  std::make_unique<DescriptorSetLayout>(application.getDevice(), std::vector<VkDescriptorSetLayoutBinding>{ samplerLayoutBinding });

		imageDescriptorPool = std::make_unique<DescriptorPool>(application.getDevice(), 0, 0, 1, 1);
		imageDescriptor = imageDescriptorPool->allocate(samplerLayout->getHandle());

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = imageView->getHandle();
		imageInfo.sampler = sampler;

		VkWriteDescriptorSet samplerUpdateInfo{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		samplerUpdateInfo.dstSet = imageDescriptor;
		samplerUpdateInfo.dstBinding = 0;
		samplerUpdateInfo.dstArrayElement = 0;
		samplerUpdateInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerUpdateInfo.descriptorCount = 1;
		samplerUpdateInfo.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(application.getDevice().getHandle(), 1, &samplerUpdateInfo, 0, nullptr);
	}
}

