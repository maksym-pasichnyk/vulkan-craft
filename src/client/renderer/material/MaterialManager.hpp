#pragma once

#include "client/AppPlatform.hpp"
#include "client/renderer/RenderContext.hpp"
#include "client/util/Handle.hpp"

#include "Material.hpp"

#include "nlohmann/json.hpp"

using Json = nlohmann::json;

struct MaterialManager {
	MaterialManager() {}

	void loadMetaFile(Handle<AppPlatform> platform, Handle<RenderContext> renderContext) {
		std::string_view bytes = R"({
			"agent": {
				"fragmentShader": "shaders/entity.frag.spv",
				"vertexShader": "shaders/entity.vert.spv",
				"depthFunc": 0
			}
		})";

		auto obj = Json::parse(bytes);
		for (auto& [name, value] : obj.items()) {
			auto vertexShader = createShader(platform, value.at("vertexShader").get<std::string>());
			auto fragmentShader = createShader(platform, value.at("fragmentShader").get<std::string>());

			vk::PipelineShaderStageCreateInfo stages[] {
					{.stage = vk::ShaderStageFlagBits::eVertex, .module = vertexShader, .pName = "main"},
					{.stage = vk::ShaderStageFlagBits::eFragment, .module = fragmentShader, .pName = "main"},
			};

			materials.emplace(name, new Material(renderContext, stages));

			core->device().destroyShaderModule(vertexShader, nullptr);
			core->device().destroyShaderModule(fragmentShader, nullptr);
		}
	}

	Handle<Material> getMaterial(const std::string& name) {
		return Handle(materials.at(name));
	}

private:
	RenderSystem* core = RenderSystem::Instance();

	inline vk::ShaderModule createShader(Handle<AppPlatform> platform, const std::string& path) {
		auto bytes = platform->readAssetFile(path);

		vk::ShaderModuleCreateInfo shaderModuleCreateInfo {
			.codeSize = bytes.size(),
			.pCode = reinterpret_cast<const uint32_t *>(bytes.data())
		};

		return core->device().createShaderModule(shaderModuleCreateInfo);
	}

	std::map<std::string, std::unique_ptr<Material>> materials;
};