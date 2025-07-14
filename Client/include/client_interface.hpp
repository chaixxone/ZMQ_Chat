#pragma once
#include <string>
#include <zmq.hpp>

class IClient
{
public:
	virtual void RequestToCreateChat(const std::string& clients) = 0;
	virtual void SendMessageToChat(const std::string& messageStr, int chatIdInt) = 0;
};
