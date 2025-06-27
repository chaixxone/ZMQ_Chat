#pragma once
#include "client_interface.hpp"
#include "message_queue.hpp"
#include <thread>

class Client : public IClient
{
public:
    Client(std::string endpoint, std::string identity, std::shared_ptr<MessageQueue> messageQueue);
    ~Client();
    void RequestToCreateChat(std::string& clients, const std::string& chatId) override;
    void SendMessageToChat(std::string& messageStr, const std::string& actionStr = "send_message") override;
    bool HasRequestToChat() const;
    void Reply(const std::string& reply);
    void RequestChangeIdentity(std::string& desiredIdentity);

private:
    void ReceiveMessage();
    static std::string GenerateTemporaryId();
    void ChangeIdentity(const std::string& identity);

    std::string _endpoint;
    std::string _identity;
    zmq::socket_t _socket;
    std::thread _receiver;
    std::shared_ptr<MessageQueue> _messageQueue;
    zmq::context_t _context;
    size_t _chatId;
    bool _isInChat;
    bool _hasRequestToChat;
    std::atomic_bool _alive = true;
};
