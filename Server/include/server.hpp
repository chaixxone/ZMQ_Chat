#pragma once
#include <unordered_set>
#include <unordered_map>

#include <zmq.hpp>

#include <utils/client_actions.hpp>
#include <database_connection.hpp>

class Server
{
public:
    Server(zmq::context_t& context, std::string binding, std::unique_ptr<DatabaseConnection> dbConn);
    void Run();

private:
    void AskClients(int pendingInvitesChatId, const std::string& creator, const std::unordered_set<std::string>& clients);
    void MessageDispatch(
        Utils::Action action,
        const std::string& message,
        const std::unordered_set<std::string>& clients,
        const std::string& messageIdStr,
        const std::string& authorStr,
        int chatIdInt
    );
    void MessageDispatch(Utils::Action action, const std::string& message, const std::string& clientId);
    std::unordered_set<std::string> ParseClients(const std::string& clients, const std::string& creator);
    void HandleSendMessage(const std::string& clientId, const std::string& dataStr, int chatId);
    void PrepareNewChatSession(const std::string& clientId, const std::string& dataStr);
    void HandleResponseForInvite(const std::string& clientId, const std::string& dataStr, bool isAccepted);
    void HandleConnection(const std::string& generatedClientId, const std::string& clientId, const std::string& deviceID);
    void HandleRegister(const std::string& clientId, const std::string& data);
    void HandleAuthorize(const std::string& clientId, const std::string& dataStr, const std::string deviceID);
    void HandleLogout(const std::string& clientId, const std::string& sessionId, const std::string& deviceID);
    void HandleAllChatsInfoRequest(const std::string& clientId);
    void HandleClientChatsInfoRequest(const std::string& clientId);
    void HandleGetClientsByName(const std::string& clientId, const std::string& name);
    void HandleClientPendingInvites(const std::string& clientId);

    zmq::context_t& _context;
    zmq::socket_t _socket;
    std::unordered_set<std::string> _clients;
    std::unordered_map<int, std::unordered_set<std::string>> _activeChats;
    std::unordered_map<int, std::unordered_set<std::string>> _pendingChatInvites;
    std::unique_ptr<DatabaseConnection> _databaseConnection;
    bool _running;
};
