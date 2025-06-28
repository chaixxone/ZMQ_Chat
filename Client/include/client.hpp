#pragma once
#include "client_interface.hpp"
#include "message_queue.hpp"
#include <thread>
#include <utils/client_actions.hpp>

class Client
{
public:
    Client(std::string endpoint, std::string identity, std::shared_ptr<MessageQueue> messageQueue);
    ~Client();
    void RequestToCreateChat(std::string& clients, int chatId);
    void SendMessageToChat(std::string& messageStr, int chatIdInt);
    bool HasRequestToChat() const;
    void Reply(const std::string& reply);
    void RequestChangeIdentity(std::string& desiredIdentity);
    std::optional<MessageView> TryGetMessage();
    int GetChatId() const noexcept;

private:
    void SendMessageToChat(std::string& messageStr, Utils::Action action, int chatIdInt);
    void ReceiveMessage();
    static std::string GenerateTemporaryId();
    void ChangeIdentity(const std::string& identity);

    zmq::context_t _context;
    zmq::socket_t _socket;
    std::string _endpoint;
    std::string _identity;
    std::thread _receiver;
    std::shared_ptr<MessageQueue> _messageQueue;
    int _pendingChatId;
    int _chatId;
    bool _isInChat;
    bool _hasRequestToChat;
    std::atomic_bool _alive = true;
};
