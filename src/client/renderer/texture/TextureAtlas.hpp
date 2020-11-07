#pragma once

#include "util/math/Rect2D.hpp"

#include "client/renderer/TextureUVCoordinateSet.hpp"

#include "NativeImage.hpp"

#include <string>
#include <vector>
#include <map>

#include "client/util/Handle.hpp"

#include "nlohmann/json.hpp"

using Json = nlohmann::json;

struct TextureAtlasSprite {
	struct Info {
		std::string path;
		NativeImage image;

		void *pixels() const {
			return image.pixels;
		}

		int width() const {
			return image.width;
		}

		int height() const {
			return image.height;
		}
	};

	Info info;
	int originX;
	int originY;

	TextureAtlasSprite(Info info, int originX, int originY)
		: info(std::move(info)), originX(originX), originY(originY) {}
};

struct SheetData {
	std::vector<TextureAtlasSprite> sprites;
	int width;
	int height;

	SheetData(std::vector<TextureAtlasSprite>&& sprites, int width, int height)
		: sprites(std::move(sprites))
		, width(width)
		, height(height) {}
};

struct TextureAtlasPack {
	struct Holder {
		TextureAtlasSprite::Info info;
		int width;
		int height;

		Holder(TextureAtlasSprite::Info info)
			: info(std::move(info))
			, width(info.width())
			, height(info.height()) {}
	};

	struct Slot {
		Slot() = default;

		Slot(int x, int y, int w, int h) : rect{x, y, w, h} {}

		bool canFit(int width, int height) {
			return width <= rect.width && height <= rect.height;
		}

		bool addSlot(Holder* holderIn) {
			if (canFit(holderIn->width, holderIn->height)) {
				if (holder == nullptr) {
					holder = holderIn;

					int originX = rect.x;
					int originY = rect.y;
					int w = holder->width;
					int h = holder->height;

					if (rect.width > w) {
						subSlots.emplace_back(new Slot(originX + w, originY, rect.width - w, h));
					}
					if (rect.height > h) {
						subSlots.emplace_back(new Slot(originX, originY + h, rect.width, rect.height - h));
					}

					return true;
				}

				for (auto& slot : subSlots) {
					if (slot->addSlot(holderIn)) {
						return true;
					}
				}
			}
			return false;
		}

		template <typename Fn>
		void getAllStitchSlots(Fn&& fn) {
			if (holder != nullptr) {
				fn(this);
			}

			for (auto& slot : subSlots) {
				slot->getAllStitchSlots(std::forward<Fn>(fn));
			}
		}

		std::vector<std::unique_ptr<Slot>> subSlots;
		Holder* holder = nullptr;
		Rect2D rect;
	};

	void addSprite(const TextureAtlasSprite::Info& info) {
		holders.emplace_back(std::make_unique<Holder>(info));
	}

	bool expandAndAllocateSlot(Holder* holder) {
		Slot* slot = nullptr;

		int textureSize = std::max(
			/*nextPowerOf2*/(currentWidth + holder->width),
			/*nextPowerOf2*/(currentHeight + holder->height)
		);

		slots.reserve(2);
		slots.emplace_back(new Slot(currentWidth, 0, textureSize - currentWidth, currentHeight));
		if (slots.back()->canFit(holder->width, holder->height)) {
			slot = slots.back().get();
		}
		slots.emplace_back(new Slot(0, currentHeight, textureSize, textureSize - currentHeight));
		if (!slot && slots.back()->canFit(holder->width, holder->height)) {
			slot = slots.back().get();
		}
		currentWidth = textureSize;
		currentHeight = textureSize;

		return slot->addSlot(holder);
	}

	bool allocateSlot(Holder* holder) {
		for (auto& slot : slots) {
			if (slot->addSlot(holder)) {
				return true;
			}
		}

		return expandAndAllocateSlot(holder);
	}

	std::optional<SheetData> build() {
		std::sort(holders.begin(), holders.end(), [](auto& b, auto& a) -> bool {
			return a->width < b->width || (a->width == b->width) && (a->height < b->height);
		});

		int textureSize = std::max(
			nextPowerOf2(holders[0]->width),
			nextPowerOf2(holders[0]->height)
		);
		currentWidth = textureSize;
		currentHeight = textureSize;

		slots.emplace_back(new Slot(0, 0, textureSize, textureSize));
		for (auto&& holder : holders) {
			if (!allocateSlot(holder.get())) {
				return std::nullopt;
			}
		}

		std::vector<TextureAtlasSprite> sprites;
		getAllSlots([&sprites](Slot *slot) {
			sprites.emplace_back(std::move(slot->holder->info), slot->rect.x, slot->rect.y);
		});

		return SheetData(std::move(sprites), currentWidth, currentHeight);
	}

	template <typename Fn>
	void getAllSlots(Fn&& fn) {
		for (auto&& slot : slots) {
			slot->getAllStitchSlots(std::forward<Fn>(fn));
		}
	}

	inline static constexpr unsigned nextPowerOf2(unsigned n) noexcept {
		n--;
		n |= n >> 1u;
		n |= n >> 2u;
		n |= n >> 4u;
		n |= n >> 8u;
		n |= n >> 16u;
		n++;
		return n;
	}

private:
	std::vector<std::unique_ptr<Holder>> holders;
	std::vector<std::unique_ptr<Slot>> slots;

	int currentWidth;
	int currentHeight;
};

struct TextureAtlasTextureItem {
	std::string name;
	std::vector<TextureUVCoordinateSet> textures;

	const TextureUVCoordinateSet& operator[](int i) const {
		return textures[i];
	}

	const TextureUVCoordinateSet& get(int i) const {
		return textures[i];
	}
};

struct ParsedAtlasNodeElement {
	std::string path;
//	std::optional<rgb24> overlay_color;
//	std::optional<rgb24> tint_color;
};

struct ParsedAtlasNode {
	ParsedAtlasNode(std::string name) : name(std::move(name)) {}

	std::string name;
	int quad;
	std::vector<ParsedAtlasNodeElement> elements;
};

struct TextureAtlas : Texture {
	std::optional<SheetData> sheet;
	std::string texture_name;
	int padding;
	int num_mip_levels;

//	TextureAtlasTextureItem* pMissingTexture;

	std::map<std::string, TextureAtlasTextureItem> items;

	void _readElement(Json& data, ParsedAtlasNodeElement& element) {
		if (data.is_string()) {
			element.path = data.get<std::string>();
		} else {
			element.path = data.at("path").get<std::string>();
//			if (auto overlay_color = value.get("overlay_color")) {
//				element.overlay_color = overlay_color->as_string();
//			}
//			if (auto tint_color = value.get("tint_color")) {
//				element.tint_color = tint_color->get<std::string>();
//			}
		}
	}

	void _readNode(Json& data, ParsedAtlasNode& node) {
		node.quad = data.value<int>("quad", 0);

		auto textures = data.at("textures");
		if (textures.is_array()) {
			for (auto& texture : textures) {
				_readElement(texture, node.elements.emplace_back());
			}
		} else {
			_readElement(textures, node.elements.emplace_back());
		}
	}

	void _loadAtlasNodes(Json& texture_data, std::vector<ParsedAtlasNode>& nodes) {
		for (auto& item : texture_data.items()) {
			_readNode(item.value(), nodes.emplace_back(item.key()));
		}
	}

	void loadMetaFile(Handle<ResourceManager> resourceManager) {
		auto object = Json::parse(resourceManager->loadFile("textures/terrain_texture.json").value());

		auto resource_pack_name = object.at("resource_pack_name").get<std::string>();
		texture_name = object.at("texture_name").get<std::string>();
		padding = object.at("padding").get<int>();
		num_mip_levels = object.at("num_mip_levels").get<int>();

		auto& texture_data = object.at("texture_data");

		std::vector<ParsedAtlasNode> nodes;
		_loadAtlasNodes(texture_data, nodes);

		std::set<std::string> requireTextures;
		for (auto& node : nodes) {
			for (auto &element : node.elements) {
				requireTextures.emplace(element.path);
			}
		}

		TextureAtlasPack textureAtlasPack;
		for (auto &path : requireTextures) {
			textureAtlasPack.addSprite({path, resourceManager->loadTextureData(path).value()});
		}
		sheet = textureAtlasPack.build();

		std::map<std::string, TextureAtlasSprite> sprites;
		for (auto &sprite : sheet->sprites) {
			sprites.emplace(sprite.info.path, sprite);
		}

		for (auto &node : nodes) {
			auto& item = items[node.name];

			item.textures.reserve(node.elements.size());

			for (auto &element : node.elements) {
				auto sprite = sprites.at(element.path);

				Rect2D rect{
						sprite.originX,
						sprite.originY,
						sprite.info.width(),
						sprite.info.height()
				};

				auto& textureUvCoordinateSet = item.textures.emplace_back();
				textureUvCoordinateSet.minU = float(rect.x) / float(sheet->width);
				textureUvCoordinateSet.minV = float(rect.y) / float(sheet->height);
				textureUvCoordinateSet.maxU = float(rect.x + rect.width) / float(sheet->width);
				textureUvCoordinateSet.maxV = float(rect.y + rect.height) / float(sheet->height);
			}
		}
	}

	TextureAtlasTextureItem& getTextureItem(const std::string& name) {
		return items.at(name);
	}

	void loadTexture(TextureManager* textureManager) override {
		std::vector<uint8_t> pixels{};
		pixels.resize(sheet->width * sheet->height * 4);
		for (auto& sprite : sheet->sprites) {
			Rect2D rect {
				sprite.originX,
				sprite.originY,
				sprite.info.width(),
				sprite.info.height()
			};

			int i2 = 0;
			for (int y = rect.y; y < rect.y + rect.height; y++) {
				auto i = (y * sheet->width + rect.x) * 4;

				for (int x = 0; x < rect.width; x++) {
					pixels[i++] = reinterpret_cast<unsigned char*>(sprite.info.pixels())[i2++];
					pixels[i++] = reinterpret_cast<unsigned char*>(sprite.info.pixels())[i2++];
					pixels[i++] = reinterpret_cast<unsigned char*>(sprite.info.pixels())[i2++];
					pixels[i++] = reinterpret_cast<unsigned char*>(sprite.info.pixels())[i2++];
				}
			}
		}

		renderTexture = textureManager->createTexture(vk::Format::eR8G8B8A8Unorm, sheet->width, sheet->height, 4, pixels.data());
	}
};
