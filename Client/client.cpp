#include "client.hpp"
#include <iostream>

Client::Client(std::string endpoint, std::string identity)
    : _context(1), _socket(_context, zmq::socket_type::dealer), _identity(identity), _isInChat(false), _hasRequestToChat(false)
{
    _socket.set(zmq::sockopt::routing_id, _identity);
    _socket.connect(endpoint);

    std::string connection = "!connect!";
    SendMessageToChat(connection, connection);

    _receiver = std::thread(&Client::_receiveMessage, this);
}

Client::~Client()
{
    if (_receiver.joinable())
    {
        _receiver.join();
    }
}

bool Client::HasRequestToChat() const
{
    return _hasRequestToChat;
}

void Client::SendMessageToChat(std::string& messageStr, const std::string& actionStr)
{
    zmq::message_t action(actionStr);

    if (actionStr == "send_message")
    {
        messageStr = std::to_string(_chatId) + ":" + messageStr;
    }

    zmq::message_t message(messageStr);
    bool result = _socket.send(action, zmq::send_flags::sndmore) && _socket.send(message, zmq::send_flags::none);

    if (!result)
    {
        std::cerr << "Failed to send message to chat." << std::endl;
    }
}

void Client::RequestToCreateChat(std::string& clients, std::string& chatId)
{
    if (!clients.empty() && clients.back() == ' ') clients.pop_back();
    std::cout << "I am requesting: " << clients << ", to create chat " << chatId << std::endl;
    std::string chatInfo = "create_chat:" + chatId;
    SendMessageToChat(clients, chatInfo);
    _chatId = static_cast<size_t>(stoi(chatId));
    _isInChat = true;
}

void Client::_receiveMessage()
{
    while (true)
    {
        zmq::message_t action, data;
        _socket.recv(action, zmq::recv_flags::none);
        _socket.recv(data, zmq::recv_flags::none);

        std::string actionStr = action.to_string();
        std::string dataStr = data.to_string();

        if (actionStr.substr(0, 12) == "create_chat:" && !_isInChat)
        {
            _chatId = static_cast<size_t>(stoi(actionStr.substr(12)));
            std::cout << "[" << _identity << "]" << " I am invited to chat " << _chatId << std::endl;
            _hasRequestToChat = true;
            std::cout << "[Server] Do you wish to create chat with " << dataStr << "? (y/n)" << std::endl;
        }
        else if (actionStr == "new_chat")
        {
            _chatId = std::stoi(dataStr);
            std::cout << "[Server] Now you are in chat with id=" << dataStr << std::endl;
            _isInChat = true;
        }
        else if (actionStr == "incoming_message")
        {
            std::cout << dataStr << std::endl;
        }
        else
        {
            std::cout << "Error: unknown action!" << std::endl;
        }
    }
}

void Client::Reply(const std::string& reply)
{
    if (reply == "y")
    {
        std::cout << "accepted!" << std::endl;
        SendMessageToChat(std::to_string(_chatId), "accept_create_chat");
    }
    else
    {
        SendMessageToChat(std::to_string(_chatId), "decline_create_chat");
        _chatId = NULL;
    }

    _hasRequestToChat = false;
}
