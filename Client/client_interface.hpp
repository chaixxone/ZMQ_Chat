#pragma once
#include <string>
#include <zmq.hpp>

class IClient
{
public:
	virtual void RequestToCreateChat(std::string& clients, std::string& chatId) = 0;
	virtual void SendMessageToChat(std::string& messageStr, const std::string& actionStr) = 0;
};
