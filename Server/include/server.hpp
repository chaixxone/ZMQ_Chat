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
    void AskClients(const std::pair<size_t, std::string>& chatInfo, const std::unordered_set<std::string>& clients);
    void MessageDispatch(
        const std::string& action,
        const std::string& message,
        const std::unordered_set<std::string>& clients,
        const std::string& messageIdStr = "",
        const std::string& authorStr = "",
        int chatIdInt = -1
    );
    std::unordered_set<std::string> ParseClients(const std::string& clients, const std::string& creator);
    void HandleSendMessage(const std::string& clientId, const std::string& dataStr);
    void PrepareNewChatSession(const std::string& clientId, const std::string& actionStr, const std::string& dataStr);
    void HandleResponseForInvite(zmq::message_t& identity, const std::string& clientId, const std::string& dataStr, bool isAccepted);
    void HandleConnection(zmq::message_t& clientId, const std::string& desiredIdentity);

    zmq::context_t _context;
    zmq::socket_t _socket;
    std::unordered_set<std::string> _clients;
    std::unordered_map<size_t, std::unordered_set<std::string>> _activeChats;
    std::unordered_map<size_t, std::unordered_set<std::string>> _pendingChatInvites;
};
