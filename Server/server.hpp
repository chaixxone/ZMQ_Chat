#pragma once
#include "server_interface.hpp"
#include <zmq.hpp>
#include <unordered_set>
#include <unordered_map>

class Server : IServer
{
public:
	Server(std::string binding);
	void Run() override;

private:
	zmq::context_t _context;
	zmq::socket_t _socket;
	std::unordered_set<std::string> _clients;
	std::unordered_map<size_t, std::unordered_set<std::string>> _activeChats;
	void _callback(const std::string& action, const std::string& message, const std::unordered_set<std::string> clients);
};