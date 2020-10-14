#include "gui.hpp"

#include <cstdint>

#include "DescriptorPool.hpp"
#include "RenderSystem.hpp"

#include "imgui.h"

#include <GLFW/glfw3.h>

namespace {
	inline static constexpr uint32_t __glsl_shader_vert_spv[] = {
			0x07230203, 0x00010000, 0x00080001, 0x0000002e, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
			0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
			0x000a000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x0000000b, 0x0000000f, 0x00000015,
			0x0000001b, 0x0000001c, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d,
			0x00000000, 0x00030005, 0x00000009, 0x00000000, 0x00050006, 0x00000009, 0x00000000, 0x6f6c6f43,
			0x00000072, 0x00040006, 0x00000009, 0x00000001, 0x00005655, 0x00030005, 0x0000000b, 0x0074754f,
			0x00040005, 0x0000000f, 0x6c6f4361, 0x0000726f, 0x00030005, 0x00000015, 0x00565561, 0x00060005,
			0x00000019, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x00000019, 0x00000000,
			0x505f6c67, 0x7469736f, 0x006e6f69, 0x00030005, 0x0000001b, 0x00000000, 0x00040005, 0x0000001c,
			0x736f5061, 0x00000000, 0x00060005, 0x0000001e, 0x73755075, 0x6e6f4368, 0x6e617473, 0x00000074,
			0x00050006, 0x0000001e, 0x00000000, 0x61635375, 0x0000656c, 0x00060006, 0x0000001e, 0x00000001,
			0x61725475, 0x616c736e, 0x00006574, 0x00030005, 0x00000020, 0x00006370, 0x00040047, 0x0000000b,
			0x0000001e, 0x00000000, 0x00040047, 0x0000000f, 0x0000001e, 0x00000002, 0x00040047, 0x00000015,
			0x0000001e, 0x00000001, 0x00050048, 0x00000019, 0x00000000, 0x0000000b, 0x00000000, 0x00030047,
			0x00000019, 0x00000002, 0x00040047, 0x0000001c, 0x0000001e, 0x00000000, 0x00050048, 0x0000001e,
			0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x0000001e, 0x00000001, 0x00000023, 0x00000008,
			0x00030047, 0x0000001e, 0x00000002, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
			0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040017,
			0x00000008, 0x00000006, 0x00000002, 0x0004001e, 0x00000009, 0x00000007, 0x00000008, 0x00040020,
			0x0000000a, 0x00000003, 0x00000009, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000003, 0x00040015,
			0x0000000c, 0x00000020, 0x00000001, 0x0004002b, 0x0000000c, 0x0000000d, 0x00000000, 0x00040020,
			0x0000000e, 0x00000001, 0x00000007, 0x0004003b, 0x0000000e, 0x0000000f, 0x00000001, 0x00040020,
			0x00000011, 0x00000003, 0x00000007, 0x0004002b, 0x0000000c, 0x00000013, 0x00000001, 0x00040020,
			0x00000014, 0x00000001, 0x00000008, 0x0004003b, 0x00000014, 0x00000015, 0x00000001, 0x00040020,
			0x00000017, 0x00000003, 0x00000008, 0x0003001e, 0x00000019, 0x00000007, 0x00040020, 0x0000001a,
			0x00000003, 0x00000019, 0x0004003b, 0x0000001a, 0x0000001b, 0x00000003, 0x0004003b, 0x00000014,
			0x0000001c, 0x00000001, 0x0004001e, 0x0000001e, 0x00000008, 0x00000008, 0x00040020, 0x0000001f,
			0x00000009, 0x0000001e, 0x0004003b, 0x0000001f, 0x00000020, 0x00000009, 0x00040020, 0x00000021,
			0x00000009, 0x00000008, 0x0004002b, 0x00000006, 0x00000028, 0x00000000, 0x0004002b, 0x00000006,
			0x00000029, 0x3f800000, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
			0x00000005, 0x0004003d, 0x00000007, 0x00000010, 0x0000000f, 0x00050041, 0x00000011, 0x00000012,
			0x0000000b, 0x0000000d, 0x0003003e, 0x00000012, 0x00000010, 0x0004003d, 0x00000008, 0x00000016,
			0x00000015, 0x00050041, 0x00000017, 0x00000018, 0x0000000b, 0x00000013, 0x0003003e, 0x00000018,
			0x00000016, 0x0004003d, 0x00000008, 0x0000001d, 0x0000001c, 0x00050041, 0x00000021, 0x00000022,
			0x00000020, 0x0000000d, 0x0004003d, 0x00000008, 0x00000023, 0x00000022, 0x00050085, 0x00000008,
			0x00000024, 0x0000001d, 0x00000023, 0x00050041, 0x00000021, 0x00000025, 0x00000020, 0x00000013,
			0x0004003d, 0x00000008, 0x00000026, 0x00000025, 0x00050081, 0x00000008, 0x00000027, 0x00000024,
			0x00000026, 0x00050051, 0x00000006, 0x0000002a, 0x00000027, 0x00000000, 0x00050051, 0x00000006,
			0x0000002b, 0x00000027, 0x00000001, 0x00070050, 0x00000007, 0x0000002c, 0x0000002a, 0x0000002b,
			0x00000028, 0x00000029, 0x00050041, 0x00000011, 0x0000002d, 0x0000001b, 0x0000000d, 0x0003003e,
			0x0000002d, 0x0000002c, 0x000100fd, 0x00010038
	};

	inline static constexpr uint32_t __glsl_shader_frag_spv[] = {
			0x07230203, 0x00010000, 0x00080001, 0x0000001e, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
			0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
			0x0007000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000d, 0x00030010,
			0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d,
			0x00000000, 0x00040005, 0x00000009, 0x6c6f4366, 0x0000726f, 0x00030005, 0x0000000b, 0x00000000,
			0x00050006, 0x0000000b, 0x00000000, 0x6f6c6f43, 0x00000072, 0x00040006, 0x0000000b, 0x00000001,
			0x00005655, 0x00030005, 0x0000000d, 0x00006e49, 0x00050005, 0x00000016, 0x78655473, 0x65727574,
			0x00000000, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000d, 0x0000001e,
			0x00000000, 0x00040047, 0x00000016, 0x00000022, 0x00000000, 0x00040047, 0x00000016, 0x00000021,
			0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006,
			0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003,
			0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040017, 0x0000000a, 0x00000006,
			0x00000002, 0x0004001e, 0x0000000b, 0x00000007, 0x0000000a, 0x00040020, 0x0000000c, 0x00000001,
			0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000001, 0x00040015, 0x0000000e, 0x00000020,
			0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000, 0x00040020, 0x00000010, 0x00000001,
			0x00000007, 0x00090019, 0x00000013, 0x00000006, 0x00000001, 0x00000000, 0x00000000, 0x00000000,
			0x00000001, 0x00000000, 0x0003001b, 0x00000014, 0x00000013, 0x00040020, 0x00000015, 0x00000000,
			0x00000014, 0x0004003b, 0x00000015, 0x00000016, 0x00000000, 0x0004002b, 0x0000000e, 0x00000018,
			0x00000001, 0x00040020, 0x00000019, 0x00000001, 0x0000000a, 0x00050036, 0x00000002, 0x00000004,
			0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x00050041, 0x00000010, 0x00000011, 0x0000000d,
			0x0000000f, 0x0004003d, 0x00000007, 0x00000012, 0x00000011, 0x0004003d, 0x00000014, 0x00000017,
			0x00000016, 0x00050041, 0x00000019, 0x0000001a, 0x0000000d, 0x00000018, 0x0004003d, 0x0000000a,
			0x0000001b, 0x0000001a, 0x00050057, 0x00000007, 0x0000001c, 0x00000017, 0x0000001b, 0x00050085,
			0x00000007, 0x0000001d, 0x00000012, 0x0000001c, 0x0003003e, 0x00000009, 0x0000001d, 0x000100fd,
			0x00010038
	};
}

void GUI::initialize(GLFWwindow* window, vk::RenderPass renderPass, int frameCount) {
	_window = window;

    IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	_time = 0.0;

    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    io.BackendPlatformName = "imgui_impl_glfw";
	io.BackendRendererName = "imgui_impl_vulkan";

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io.SetClipboardTextFn = SetClipboardText;
    io.GetClipboardTextFn = GetClipboardText;
//    io.ClipboardUserData = RenderSystem::Get()->window();

    _mouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    _mouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    _mouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    _mouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    _mouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
#if GLFW_HAS_NEW_CURSORS
    g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
#else
    _mouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    _mouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    _mouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    _mouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
#endif

	_frameIndex = 0;
	_frameCount = frameCount;
	_renderBuffers.resize(frameCount);


	ImGui::StyleColorsDark();

	auto& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.ChildRounding = 0.0f;
	style.FrameRounding = 0.0f;
	style.GrabRounding = 0.0f;
	style.PopupRounding = 0.0f;
	style.ScrollbarRounding = 0.0f;

	vk::DescriptorPoolSize pool_sizes[] = {
			{vk::DescriptorType::eSampler, 1000},
			{vk::DescriptorType::eCombinedImageSampler, 1000},
			{vk::DescriptorType::eSampledImage, 1000},
			{vk::DescriptorType::eStorageImage, 1000},
			{vk::DescriptorType::eUniformTexelBuffer, 1000},
			{vk::DescriptorType::eStorageTexelBuffer, 1000},
			{vk::DescriptorType::eUniformBuffer, 1000},
			{vk::DescriptorType::eStorageBuffer, 1000},
			{vk::DescriptorType::eUniformBufferDynamic, 1000},
			{vk::DescriptorType::eStorageBufferDynamic, 1000},
			{vk::DescriptorType::eInputAttachment, 1000}
	};

	vk::DescriptorPoolCreateInfo descriptor_pool_create_info {
		.maxSets = 1000,
		.poolSizeCount = std::size(pool_sizes),
		.pPoolSizes = pool_sizes
	};
	_descriptorPool = DescriptorPool::create(1000, pool_sizes);

	auto vert_module = core->device().createShaderModule({ .codeSize = sizeof(__glsl_shader_vert_spv), .pCode = __glsl_shader_vert_spv }, nullptr);
	auto frag_module = core->device().createShaderModule({ .codeSize = sizeof(__glsl_shader_frag_spv), .pCode = __glsl_shader_frag_spv }, nullptr);

	vk::SamplerCreateInfo sampler_create_info {
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eRepeat,
		.addressModeV = vk::SamplerAddressMode::eRepeat,
		.addressModeW = vk::SamplerAddressMode::eRepeat,
		.maxAnisotropy = 1.0f,
		.minLod = -1000,
		.maxLod = 1000
	};

	_fontSampler = core->device().createSampler(sampler_create_info, nullptr);

	vk::DescriptorSetLayoutBinding descriptor_set_layout_binding {
		.binding = 0,
		.descriptorType = vk::DescriptorType::eCombinedImageSampler,
		.descriptorCount = 1,
		.stageFlags = vk::ShaderStageFlagBits::eFragment,
		.pImmutableSamplers = &_fontSampler
	};

	vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {
		.bindingCount = 1,
		.pBindings = &descriptor_set_layout_binding
	};
	_fontDescriptorSetLayout = core->device().createDescriptorSetLayout(descriptor_set_layout_create_info, nullptr);
	_fontDescriptor = _descriptorPool.allocate(_fontDescriptorSetLayout);

	vk::PushConstantRange push_constant {
		vk::ShaderStageFlagBits::eVertex, 0, sizeof(float[4])
	};

	vk::PipelineLayoutCreateInfo pipeline_layout_create_info {
		.setLayoutCount = 1,
		.pSetLayouts = &_fontDescriptorSetLayout,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &push_constant
	};
	_pipelineLayout = core->device().createPipelineLayout(pipeline_layout_create_info, nullptr);

	vk::PipelineShaderStageCreateInfo stages[2] = {
			{ .stage = vk::ShaderStageFlagBits::eVertex, .module = vert_module, .pName = "main" },
			{ .stage = vk::ShaderStageFlagBits::eFragment, .module = frag_module, .pName = "main" },
	};

	vk::VertexInputBindingDescription bindings[]{
			{0, sizeof(ImDrawVert), vk::VertexInputRate::eVertex}
	};

	vk::VertexInputAttributeDescription attributes[] {
			{0, bindings[0].binding, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, pos)},
			{1, bindings[0].binding, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, uv)},
			{2, bindings[0].binding, vk::Format::eR8G8B8A8Unorm, offsetof(ImDrawVert, col)}
	};

	vk::PipelineVertexInputStateCreateInfo vertexInputState {
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = bindings,
		.vertexAttributeDescriptionCount = 3,
		.pVertexAttributeDescriptions = attributes
	};

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState {
		.topology = vk::PrimitiveTopology::eTriangleList,
		.primitiveRestartEnable = false
	};

	vk::PipelineViewportStateCreateInfo viewportState {
		.viewportCount = 1,
		.pViewports = nullptr,
		.scissorCount = 1,
		.pScissors = nullptr
	};

	vk::PipelineRasterizationStateCreateInfo rasterizationState {
		.polygonMode = vk::PolygonMode::eFill,
		.cullMode = vk::CullModeFlagBits::eNone,
		.frontFace = vk::FrontFace::eCounterClockwise,
		.lineWidth = 1.0f
	};

	vk::PipelineMultisampleStateCreateInfo multisampleState {
		.rasterizationSamples = vk::SampleCountFlagBits::e1
	};

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

	vk::PipelineDepthStencilStateCreateInfo depthStencilState;

	vk::PipelineColorBlendStateCreateInfo colorBlendState {
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachmentState
	};

	vk::DynamicState dynamic_states[] {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
	};

	vk::PipelineDynamicStateCreateInfo dynamicState {
		.dynamicStateCount = 2,
		.pDynamicStates = dynamic_states
	};

	vk::GraphicsPipelineCreateInfo pipeline_create_info {
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
		.layout = _pipelineLayout,
		.renderPass = renderPass,
	};

	core->device().createGraphicsPipelines(nullptr, 1, &pipeline_create_info, nullptr, &_pipeline);
	core->device().destroyShaderModule(vert_module, nullptr);
	core->device().destroyShaderModule(frag_module, nullptr);

	createFontsTexture();
}

void GUI::createFontsTexture() {
//	auto cmd = vkx::allocate(_commandPool, vk::CommandBufferLevel::ePrimary);
//	cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
//
//	ImGuiIO &io = ImGui::GetIO();
//
//	unsigned char *pixels;
//	int width, height;
//	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
//	size_t upload_size = width * height * 4 * sizeof(char);
//
//	_fontTexture = Texture::create2D(vk::Format::eR8G8B8A8Unorm, width, height, _fontSampler);
//	_fontTexture.updateDescriptorSet(_fontDescriptor);
//
//	// Copy to Image:
//	{
//		vk::BufferCreateInfo srcBufferCreateInfo { .size = upload_size, .usage = vk::BufferUsageFlagBits::eTransferSrc };
//
//		auto srcBuffer = Buffer::create(srcBufferCreateInfo, { .usage = VMA_MEMORY_USAGE_CPU_ONLY });
//		std::memcpy(srcBuffer.map(), pixels, upload_size);
//		srcBuffer.unmap();
//
//		vk::ImageMemoryBarrier copy_barrier {
//			.dstAccessMask = vk::AccessFlagBits::eTransferWrite,
//			.oldLayout = vk::ImageLayout::eUndefined,
//			.newLayout = vk::ImageLayout::eTransferDstOptimal,
//			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//			.image = _fontTexture.image,
//			.subresourceRange {
//				.aspectMask = vk::ImageAspectFlagBits::eColor,
//				.levelCount = 1,
//				.layerCount = 1
//			}
//		};
//
//		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr, 0, nullptr, 1, &copy_barrier);
//
//		vk::BufferImageCopy region {
//			.imageSubresource {
//				.aspectMask = vk::ImageAspectFlagBits::eColor,
//				.layerCount = 1
//			},
//			.imageExtent = {
//				.width = static_cast<uint32_t>(width),
//				.height = static_cast<uint32_t>(height),
//				.depth = 1
//			}
//		};
//
//		cmd.copyBufferToImage(srcBuffer, _fontTexture.image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
//
//		vk::ImageMemoryBarrier use_barrier {
//			.srcAccessMask = vk::AccessFlagBits::eTransferWrite,
//			.dstAccessMask = vk::AccessFlagBits::eShaderRead,
//			.oldLayout = vk::ImageLayout::eTransferDstOptimal,
//			.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
//			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//			.image = _fontTexture.image,
//			.subresourceRange = {
//				.aspectMask = vk::ImageAspectFlagBits::eColor,
//				.levelCount = 1,
//				.layerCount = 1,
//			}
//		};
//
//		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1, &use_barrier);
//
//		cmd.end();
//
//		vk::SubmitInfo submitInfo{
//			.commandBufferCount = 1,
//			.pCommandBuffers = &cmd
//		};
//		core->graphicsQueue().submit(1, &submitInfo, nullptr);
//		core->graphicsQueue().waitIdle();
//
//		srcBuffer.destroy();
//	}
//
//	io.Fonts->TexID = (ImTextureID) (VkDescriptorSet) _fontDescriptor;
}

void GUI::terminate() {
    for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
    {
        glfwDestroyCursor(_mouseCursors[cursor_n]);
        _mouseCursors[cursor_n] = NULL;
    }

	ImGui::DestroyContext();

	for (uint32_t n = 0; n < _frameCount; n++) {
		_renderBuffers[n].destroy();
	}
	_renderBuffers.clear();
	_fontTexture.destroy();

	_descriptorPool.free(_fontDescriptor);
	_descriptorPool.destroy();

	core->device().destroySampler(_fontSampler, nullptr);
	core->device().destroyDescriptorSetLayout(_fontDescriptorSetLayout, nullptr);
	core->device().destroyPipelineLayout(_pipelineLayout, nullptr);
	core->device().destroyPipeline(_pipeline, nullptr);
}

void GUI::begin() {
	ImGuiIO &io = ImGui::GetIO();

	// Setup display size (every frame to accommodate for window resizing)
	int w, h;
	int display_w, display_h;
	glfwGetWindowSize(_window, &w, &h);
	glfwGetFramebufferSize(_window, &display_w, &display_h);
	io.DisplaySize = ImVec2((float) w, (float) h);
	if (w > 0 && h > 0)
		io.DisplayFramebufferScale = ImVec2((float) display_w / w, (float) display_h / h);

	// Setup time step
	double current_time = glfwGetTime();
	io.DeltaTime = _time > 0.0 ? (float) (current_time - _time) : (float) (1.0f / 60.0f);
	_time = current_time;

	updateMousePosAndButtons();
	updateMouseCursor();
    updateGamepads();


	ImGui::NewFrame();
}

void GUI::end() {
	ImGui::Render();
}

void GUI::draw(vk::CommandBuffer cmd) {
	auto draw_data = ImGui::GetDrawData();

	int fb_width = (int) (draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	int fb_height = (int) (draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
	if (fb_width <= 0 || fb_height <= 0) {
		return;
	}

	auto rb = &_renderBuffers[_frameIndex];

	if (draw_data->TotalVtxCount > 0) {
		size_t vertex_buffer_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
		size_t index_buffer_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);

		rb->SetVertexBufferSize(vertex_buffer_size);
		rb->SetIndexBufferSize(index_buffer_size);

		auto vtx_dst = static_cast<ImDrawVert *>(rb->VertexBuffer.map());
		auto idx_dst = static_cast<ImDrawIdx *>(rb->IndexBuffer.map());

		for (int n = 0; n < draw_data->CmdListsCount; n++) {
			auto cmd_list = draw_data->CmdLists[n];
			memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtx_dst += cmd_list->VtxBuffer.Size;
			idx_dst += cmd_list->IdxBuffer.Size;
		}

		rb->VertexBuffer.unmap();
		rb->IndexBuffer.unmap();
	}

	setupRenderState(draw_data, cmd, rb, fb_width, fb_height);

	ImVec2 clip_off = draw_data->DisplayPos;
	ImVec2 clip_scale = draw_data->FramebufferScale;

	int global_vtx_offset = 0;
	int global_idx_offset = 0;
	for (int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList *cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
			auto pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback != nullptr) {
				if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
					setupRenderState(draw_data, cmd, rb, fb_width, fb_height);
				} else {
					pcmd->UserCallback(cmd_list, pcmd);
				}
			} else {
				ImVec4 clip_rect{
						(pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
						(pcmd->ClipRect.y - clip_off.y) * clip_scale.y,
						(pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
						(pcmd->ClipRect.w - clip_off.y) * clip_scale.y
				};

				if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f) {
					if (clip_rect.x < 0.0f) {
						clip_rect.x = 0.0f;
					}
					if (clip_rect.y < 0.0f) {
						clip_rect.y = 0.0f;
					}

					vk::Rect2D scissor{
							{int32_t(clip_rect.x), int32_t(clip_rect.y)},
							{
									uint32_t(clip_rect.z - clip_rect.x),
									uint32_t(clip_rect.w - clip_rect.y)
							}
					};

					cmd.setScissor(0, 1, &scissor);
					cmd.drawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
				}
			}
		}
		global_idx_offset += cmd_list->IdxBuffer.Size;
		global_vtx_offset += cmd_list->VtxBuffer.Size;
	}

	_frameIndex = (_frameIndex + 1) % _frameCount;
}

void GUI::setupRenderState(ImDrawData* draw_data, vk::CommandBuffer cmd, RenderBuffer* rb, int fb_width, int fb_height) {
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, 1, &_fontDescriptor, 0, nullptr);

	if (draw_data->TotalVtxCount > 0) {
		vk::DeviceSize vertex_offset{0};

		cmd.bindVertexBuffers(0, 1, reinterpret_cast<const vk::Buffer *>(&rb->VertexBuffer), &vertex_offset);
		cmd.bindIndexBuffer(rb->IndexBuffer, 0, sizeof(ImDrawIdx) == 2 ? vk::IndexType::eUint16 : vk::IndexType::eUint32);
	}

	vk::Viewport viewport{0, 0, float(fb_width), float(fb_height), 0, 1};

	cmd.setViewport(0, 1, &viewport);

	float scale[2] {
		2.0f / draw_data->DisplaySize.x,
		2.0f / draw_data->DisplaySize.y
	};

	float translate[2] {
		-1.0f - draw_data->DisplayPos.x * scale[0],
		-1.0f - draw_data->DisplayPos.y * scale[1]
	};

	cmd.pushConstants(_pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(float[2]), scale);
	cmd.pushConstants(_pipelineLayout, vk::ShaderStageFlagBits::eVertex, sizeof(float[2]), sizeof(float[2]), translate);
}

void GUI::updateMousePosAndButtons() {
	// Update buttons
	ImGuiIO &io = ImGui::GetIO();
	for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) {
		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		io.MouseDown[i] = _mouseJustPressed[i] || glfwGetMouseButton(_window, i) != 0;
		_mouseJustPressed[i] = false;
	}

	// Update mouse position
	const ImVec2 mouse_pos_backup = io.MousePos;
	io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

	const bool focused = glfwGetWindowAttrib(_window, GLFW_FOCUSED) != 0;

	if (focused) {
		if (io.WantSetMousePos) {
			glfwSetCursorPos(_window, (double) mouse_pos_backup.x, (double) mouse_pos_backup.y);
		} else {
			double mouse_x, mouse_y;
			glfwGetCursorPos(_window, &mouse_x, &mouse_y);
			io.MousePos = ImVec2((float) mouse_x, (float) mouse_y);
		}
	}
}

void GUI::updateMouseCursor() {
	ImGuiIO &io = ImGui::GetIO();
	if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		return;

	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
	if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor) {
		// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	} else {
		// Show OS mouse cursor
		// FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with GLFW 3.2, but 3.3 works here.
		glfwSetCursor(_window, _mouseCursors[imgui_cursor] ? _mouseCursors[imgui_cursor] : _mouseCursors[ImGuiMouseCursor_Arrow]);
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void GUI::updateGamepads() {
    ImGuiIO& io = ImGui::GetIO();
    memset(io.NavInputs, 0, sizeof(io.NavInputs));
    if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
        return;

    // Update gamepad inputs
    #define MAP_BUTTON(NAV_NO, BUTTON_NO)       { if (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == GLFW_PRESS) io.NavInputs[NAV_NO] = 1.0f; }
    #define MAP_ANALOG(NAV_NO, AXIS_NO, V0, V1) { float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; v = (v - V0) / (V1 - V0); if (v > 1.0f) v = 1.0f; if (io.NavInputs[NAV_NO] < v) io.NavInputs[NAV_NO] = v; }
    int axes_count = 0, buttons_count = 0;
    const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
    const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
    MAP_BUTTON(ImGuiNavInput_Activate,   0);     // Cross / A
    MAP_BUTTON(ImGuiNavInput_Cancel,     1);     // Circle / B
    MAP_BUTTON(ImGuiNavInput_Menu,       2);     // Square / X
    MAP_BUTTON(ImGuiNavInput_Input,      3);     // Triangle / Y
    MAP_BUTTON(ImGuiNavInput_DpadLeft,   13);    // D-Pad Left
    MAP_BUTTON(ImGuiNavInput_DpadRight,  11);    // D-Pad Right
    MAP_BUTTON(ImGuiNavInput_DpadUp,     10);    // D-Pad Up
    MAP_BUTTON(ImGuiNavInput_DpadDown,   12);    // D-Pad Down
    MAP_BUTTON(ImGuiNavInput_FocusPrev,  4);     // L1 / LB
    MAP_BUTTON(ImGuiNavInput_FocusNext,  5);     // R1 / RB
    MAP_BUTTON(ImGuiNavInput_TweakSlow,  4);     // L1 / LB
    MAP_BUTTON(ImGuiNavInput_TweakFast,  5);     // R1 / RB
    MAP_ANALOG(ImGuiNavInput_LStickLeft, 0,  -0.3f,  -0.9f);
    MAP_ANALOG(ImGuiNavInput_LStickRight,0,  +0.3f,  +0.9f);
    MAP_ANALOG(ImGuiNavInput_LStickUp,   1,  +0.3f,  +0.9f);
    MAP_ANALOG(ImGuiNavInput_LStickDown, 1,  -0.3f,  -0.9f);
    #undef MAP_BUTTON
    #undef MAP_ANALOG
    if (axes_count > 0 && buttons_count > 0)
        io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    else
        io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
}

const char* GUI::GetClipboardText(void* user_data) {
	return glfwGetClipboardString((GLFWwindow *) user_data);
}

void GUI::SetClipboardText(void* user_data, const char* text) {
	glfwSetClipboardString((GLFWwindow *) user_data, text);
}

void GUI::mouseButtonCallback(int button, int action, int mods) {
	if (action == GLFW_PRESS && button >= 0 && button < IM_ARRAYSIZE(_mouseJustPressed))
		_mouseJustPressed[button] = true;
}

void GUI::scrollCallback(double xoffset, double yoffset) {
	ImGuiIO &io = ImGui::GetIO();
	io.MouseWheelH += (float) xoffset;
	io.MouseWheel += (float) yoffset;
}

void GUI::keyCallback(int key, int scancode, int action, int mods) {
	ImGuiIO &io = ImGui::GetIO();
	if (action == GLFW_PRESS)
		io.KeysDown[key] = true;
	if (action == GLFW_RELEASE)
		io.KeysDown[key] = false;

	// Modifiers are not reliable across systems
	io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
	io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
	io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
	io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void GUI::charCallback(unsigned int c) {
	ImGuiIO &io = ImGui::GetIO();
	io.AddInputCharacter(c);
}