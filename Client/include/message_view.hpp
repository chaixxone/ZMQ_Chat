#pragma once

#include <string>
#include <optional>
#include <utils/client_actions.hpp>
#include <utils/helpers.hpp>

struct MessageView
{
	std::string Author;
	std::string Content;
	std::optional<size_t> ID;
	int ChatID;
	Utils::Action Action;
	Utils::ChatMessageFlags Flags = Utils::NoFlags;
};