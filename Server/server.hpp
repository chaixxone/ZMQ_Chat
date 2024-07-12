#pragma once
#include <unordered_set>
#include <unordered_map>
#include <zmq.hpp>

class Server
{
public:
    Server(std::string binding);
    void Run();

private:
    void _askClients(const std::pair<size_t, std::string>& chatInfo, const std::unordered_set<std::string>& clients);
    void _callback(const std::string& action, const std::string& message, const std::unordered_set<std::string>& clients);
    std::unordered_set<std::string> _parseClients(const std::string& clients);
    void _handleSendMessage(const std::string& clientId, const std::string& dataStr);
    void _prepareNewChatSession(const std::string& clientId, const std::string& actionStr, const std::string& dataStr);
    void _handleResponseForInvite(zmq::message_t& identity, const std::string& clientId, const std::string& dataStr, bool isAccepted);

    zmq::context_t _context;
    zmq::socket_t _socket;
    std::unordered_set<std::string> _clients;
    std::unordered_map<size_t, std::unordered_set<std::string>> _activeChats;
    std::unordered_map<size_t, std::unordered_set<std::string>> _pendingChatInvites;
};
