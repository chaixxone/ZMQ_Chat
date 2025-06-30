#pragma once
#include <string>
#include <zmq.hpp>

class IClient
{
public:
	virtual void RequestToCreateChat(std::string& clients, int chatId) = 0;
	virtual void SendMessageToChat(std::string& messageStr, int chatIdInt) = 0;
};
