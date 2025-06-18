#pragma once
#include "client_interface.hpp"
#include <thread>

class Client : public IClient
{
public:
    Client(std::string endpoint, std::string identity);
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
    zmq::context_t _context;
    zmq::socket_t _socket;
    std::string _identity;
    std::thread _receiver;
    size_t _chatId;
    bool _isInChat;
    bool _hasRequestToChat;
};
