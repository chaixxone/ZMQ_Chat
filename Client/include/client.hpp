#pragma once
#include "client_interface.hpp"
#include "message_queue.hpp"
#include <thread>

class Client : public IClient
{
public:
    Client(std::string endpoint, std::string identity, std::shared_ptr<MessageQueue> messageQueue);
    ~Client();
    void SendRequest(const std::string request);
    void RequestToCreateChat(std::string& clients, const std::string& chatId) override;
    void SendMessageToChat(std::string& messageStr, const std::string& actionStr = "send_message") override;
    bool HasRequestToChat() const;
    void Reply(const std::string& reply);
    void RequestChangeIdentity(std::string& desiredIdentity);
    std::optional<MessageView> TryGetMessage();

private:
    void ReceiveMessage();
    static std::string GenerateTemporaryId();
    void ChangeIdentity(const std::string& identity);

    zmq::context_t _context;
    zmq::socket_t _socket;
    std::string _endpoint;
    std::string _identity;
    std::thread _receiver;
    std::shared_ptr<MessageQueue> _messageQueue;
    int _chatId;
    bool _isInChat;
    bool _hasRequestToChat;
    std::atomic_bool _alive = true;
};
