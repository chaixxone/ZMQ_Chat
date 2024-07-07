#pragma once
#include <string>
#include <zmq.hpp>

class IClient
{
public:
	virtual void RequestToCreateChat(std::string& clients) = 0;
	virtual void SendMessageToChat(const std::string& messageStr, const std::string& actionStr) = 0;
};