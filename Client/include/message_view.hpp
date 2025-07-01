#pragma once

#include <string>
#include <utils/client_actions.hpp>

struct MessageView
{
	std::string Author;
	std::string Content;
	std::string ID;
	int ChatID;
	Utils::Action Action;
};