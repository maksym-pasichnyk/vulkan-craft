#pragma once

#include <array>
#include <string>
#include <string_view>

struct ResourceLocation {
	ResourceLocation(std::string_view location)
		: ResourceLocation(split(location)) {}

	ResourceLocation(std::string _namespace, std::string _path)
		: _namespace(std::move(_namespace))
		, _path(std::move(_path)) {}

	ResourceLocation(const ResourceLocation&) = delete;
	ResourceLocation& operator=(const ResourceLocation&) = delete;

	ResourceLocation(ResourceLocation&&) = delete;
	ResourceLocation& operator=(ResourceLocation&&) = delete;

	bool operator==(const ResourceLocation& location) const {
		return _namespace == location._namespace && _path == location._path;
	}

private:
	ResourceLocation(std::array<std::string_view, 2> parts)
		: _namespace(std::move(parts[0]))
		, _path(std::move(parts[1])) {}

	static auto split(std::string_view location, char ch) -> std::array<std::string_view, 2> {
		auto delim = location.find_first_of(ch);
		if (delim != location.npos) {
			return {
				location.substr(0, delim),
				location.substr(delim + 1)
			};
		}

		return { "craft", location };
	}

private:
	std::string _namespace;
	std::string _path;
};