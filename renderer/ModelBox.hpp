#pragma once

#include "TexturedQuad.hpp"

#include <optional>

struct ModelBox {
	std::array<std::optional<TexturedQuad>, 6> quads;
};