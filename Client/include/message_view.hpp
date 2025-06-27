#pragma once

#include <string>

struct MessageView
{
	std::string Author;
	std::string Content;
	std::string ID;
	size_t ChatID;
};