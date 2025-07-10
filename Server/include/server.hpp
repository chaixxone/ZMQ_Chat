#pragma once
#include <unordered_set>
#include <unordered_map>
#include <zmq.hpp>
#include <utils/client_actions.hpp>

class Server
{
public:
    Server(std::string binding);
    void Run();

private:
    void AskClients(int PendingInvitesChatId, const std::string& creator, const std::unordered_set<std::string>& clients);
    void MessageDispatch(
        Utils::Action action,
        const std::string& message,
        const std::unordered_set<std::string>& clients,
        const std::string& messageIdStr = "",
        const std::string& authorStr = "",
        int chatIdInt = -1
    );
    std::unordered_set<std::string> ParseClients(const std::string& clients, const std::string& creator);
    void HandleSendMessage(const std::string& clientId, const std::string& dataStr, int chatId);
    void PrepareNewChatSession(const std::string& clientId, const std::string& dataStr, int chatId);
    void HandleResponseForInvite(zmq::message_t& identity, const std::string& clientId, const std::string& dataStr, bool isAccepted);
    void HandleConnection(zmq::message_t& clientId, const std::string& desiredIdentity);
    void HandleAllChatsInfoRequest(const std::string& clientId);
    void HandleClientChatsInfoRequest(const std::string& clientId);
    void HandleGetClientsByName(const std::string& clientId, const std::string& name);

    zmq::context_t _context;
    zmq::socket_t _socket;
    std::unordered_set<std::string> _clients;
    std::unordered_map<size_t, std::unordered_set<std::string>> _activeChats;
    std::unordered_map<size_t, std::unordered_set<std::string>> _pendingChatInvites;
};
