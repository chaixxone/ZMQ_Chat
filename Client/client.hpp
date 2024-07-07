#pragma once
#include "client_interface.hpp"
#include <thread>

class Client : IClient
{
public:
    Client(std::string endpoint, std::string identity);
    ~Client();
    void RequestToCreateChat(std::string& clients) override;
    void SendMessageToChat(const std::string& messageStr, const std::string& actionStr = "send_message") override;

private:
    void _receiveMessage();
    zmq::context_t _context;
    zmq::socket_t _socket;
    std::string _identity;
    std::thread _receiver;
    uint16_t _chatId;
    bool _isInChat;
};