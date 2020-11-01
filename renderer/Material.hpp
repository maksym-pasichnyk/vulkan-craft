#pragma once

#include "RenderSystem.hpp"
#include "DescriptorPool.hpp"

#include "texture/Texture.hpp"

struct Material {
	inline static constinit vk::PushConstantRange constants[] {
		{vk::ShaderStageFlagBits::eVertex, 0, sizeof(CameraTransform)}
	};

	RenderSystem* core = RenderSystem::Instance();

	vk::Sampler sampler;

	vk::DescriptorSetLayout descriptorSetLayout;
	vk::DescriptorSet descriptorSet;

	vk::PipelineLayout pipelineLayout;
	vk::Pipeline pipeline;

	Material(AppPlatform* platform, ResourceManager* resourceManager, RenderContext* renderContext) {
		vk::SamplerCreateInfo samplerCreateInfo{
			.magFilter = vk::Filter::eNearest,
			.minFilter = vk::Filter::eNearest,
			.mipmapMode = vk::SamplerMipmapMode::eNearest,
			.addressModeU = vk::SamplerAddressMode::eRepeat,
			.addressModeV = vk::SamplerAddressMode::eRepeat,
			.addressModeW = vk::SamplerAddressMode::eRepeat,
			.maxAnisotropy = 0,
			.minLod = 0,
			.maxLod = 0
		};

		sampler = core->device().createSampler(samplerCreateInfo, nullptr);

		vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding{
				.binding = 0,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.descriptorCount = 1,
				.stageFlags = vk::ShaderStageFlagBits::eFragment,
				.pImmutableSamplers = nullptr
		};

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
				.bindingCount = 1,
				.pBindings = &descriptorSetLayoutBinding
		};
		descriptorSetLayout = core->device().createDescriptorSetLayout(descriptorSetLayoutCreateInfo, nullptr);
		descriptorSet = renderContext->descriptorPool.allocate(descriptorSetLayout);

		/*-----------------------------------------------------------------------------------------------------------*/

		vk::ShaderModule vertShader = createShader(platform, "shaders/default.vert.spv");
		vk::ShaderModule fragShader = createShader(platform, "shaders/default.frag.spv");

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
				.setLayoutCount = 1,
				.pSetLayouts = &descriptorSetLayout,
				.pushConstantRangeCount = std::size(constants),
				.pPushConstantRanges = constants
		};
		pipelineLayout = core->device().createPipelineLayout(pipelineLayoutCreateInfo, nullptr);

		vk::PipelineShaderStageCreateInfo stages[] {
				{.stage = vk::ShaderStageFlagBits::eVertex, .module = vertShader, .pName = "main"},
				{.stage = vk::ShaderStageFlagBits::eFragment, .module = fragShader, .pName = "main"},
		};

		vk::VertexInputBindingDescription bindings[] {
				{0, sizeof(Vertex), vk::VertexInputRate::eVertex}
		};

		vk::VertexInputAttributeDescription attributes[]{
				{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)},
				{1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
				{2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, coords)},
		};

		vk::PipelineVertexInputStateCreateInfo vertexInputState{
				.vertexBindingDescriptionCount = std::size(bindings),
				.pVertexBindingDescriptions = bindings,
				.vertexAttributeDescriptionCount = std::size(attributes),
				.pVertexAttributeDescriptions = attributes
		};

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState{
				.topology = vk::PrimitiveTopology::eTriangleList,
				.primitiveRestartEnable = false
		};

		vk::PipelineViewportStateCreateInfo viewportState{
				.viewportCount = 1,
				.pViewports = nullptr,
				.scissorCount = 1,
				.pScissors = nullptr
		};

		vk::PipelineRasterizationStateCreateInfo rasterizationState{
				.polygonMode = vk::PolygonMode::eFill,
				.cullMode = vk::CullModeFlagBits::eNone,
				.frontFace = vk::FrontFace::eCounterClockwise,
				.lineWidth = 1.0f
		};

		vk::PipelineMultisampleStateCreateInfo multisampleState {};

		vk::PipelineColorBlendAttachmentState colorBlendAttachmentState{
				.blendEnable = VK_TRUE,
				.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
				.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
				.colorBlendOp = vk::BlendOp::eAdd,
				.srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
				.dstAlphaBlendFactor = vk::BlendFactor::eZero,
				.alphaBlendOp = vk::BlendOp::eAdd,
				.colorWriteMask = vk::ColorComponentFlagBits::eR |
						vk::ColorComponentFlagBits::eG |
						vk::ColorComponentFlagBits::eB |
						vk::ColorComponentFlagBits::eA
		};

		vk::PipelineDepthStencilStateCreateInfo depthStencilState {
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = vk::CompareOp::eLess
		};

		vk::PipelineColorBlendStateCreateInfo colorBlendState{
				.attachmentCount = 1,
				.pAttachments = &colorBlendAttachmentState
		};

		vk::DynamicState dynamicStates[] {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		vk::PipelineDynamicStateCreateInfo dynamicState {
			.dynamicStateCount = 2,
			.pDynamicStates = dynamicStates
		};

		vk::GraphicsPipelineCreateInfo pipelineCreateInfo{
				.stageCount = 2,
				.pStages = stages,
				.pVertexInputState = &vertexInputState,
				.pInputAssemblyState = &inputAssemblyState,
				.pViewportState = &viewportState,
				.pRasterizationState = &rasterizationState,
				.pMultisampleState = &multisampleState,
				.pDepthStencilState = &depthStencilState,
				.pColorBlendState = &colorBlendState,
				.pDynamicState = &dynamicState,
				.layout = pipelineLayout,
				.renderPass = renderContext->renderPass,
		};

		core->device().createGraphicsPipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline);
		core->device().destroyShaderModule(vertShader, nullptr);
		core->device().destroyShaderModule(fragShader, nullptr);
	}

	void SetTexture(Texture* texture) {
		vk::DescriptorImageInfo imageInfo{
				.sampler = sampler,
				.imageView = texture->view,
				.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
		};

		vk::WriteDescriptorSet writeDescriptorSet{
				.dstSet = descriptorSet,
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.pImageInfo = &imageInfo,
		};

		core->device().updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	}

	void destroy() {
		core->device().destroyDescriptorSetLayout(descriptorSetLayout, nullptr);
		core->device().destroyPipelineLayout(pipelineLayout, nullptr);
		core->device().destroyPipeline(pipeline, nullptr);
	}

private:
	inline vk::ShaderModule createShader(AppPlatform* platform, const std::string& path) {
		auto bytes = platform->readAssetFile(path);

		vk::ShaderModuleCreateInfo shaderModuleCreateInfo {
			.codeSize = bytes.size(),
			.pCode = reinterpret_cast<const uint32_t *>(bytes.data())
		};

		return core->device().createShaderModule(shaderModuleCreateInfo);
	}
};