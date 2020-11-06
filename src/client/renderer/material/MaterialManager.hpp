#pragma once

#include "client/AppPlatform.hpp"
#include "client/renderer/RenderContext.hpp"

#include "client/util/Json.hpp"

#include "Material.hpp"

#include "client/util/Handle.hpp"

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

		auto obj = json::Parser{bytes}.parse()->as_object().value();
		for (auto& [name, value] : obj) {
			auto vertexShader = createShader(platform, value.get("vertexShader").value().as_string().value());
			auto fragmentShader = createShader(platform, value.get("fragmentShader").value().as_string().value());

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