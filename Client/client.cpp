#include "client.hpp"
#include <iostream>
#include <random>

const short CREATE_CHAT_PREFIX_LENGTH = 12;

Client::Client(std::string endpoint, std::string identity)
    : _context(1), _socket(_context, zmq::socket_type::dealer), _endpoint(endpoint), _identity(GenerateTemporaryId()), _isInChat(false), _hasRequestToChat(false)
{
    _socket.set(zmq::sockopt::routing_id, _identity);
    _socket.set(zmq::sockopt::linger, 0);
    _socket.connect(endpoint);

    SendMessageToChat(identity, "!connect!");

    _receiver = std::thread(&Client::ReceiveMessage, this);
    _receiver.detach();
}

void Client::RequestChangeIdentity(std::string& desiredIdentity)
{    
    SendMessageToChat(desiredIdentity, "!connect!");
}

void Client::ChangeIdentity(const std::string& identity)
{
    _identity = identity;
    _socket.disconnect(_endpoint);
    _socket.set(zmq::sockopt::routing_id, _identity);
    _socket.connect(_endpoint);
}

std::string Client::GenerateTemporaryId()
{
    static std::string alphanum =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t size = alphanum.size();
    const size_t length = 36;

    std::string temporaryIdentifier;
    temporaryIdentifier.reserve(length);
    std::random_device rd;
    std::mt19937 mt{ rd() };
    std::uniform_int_distribution<size_t> dist(0, size - 1);

    for (size_t i = 0; i < length; i++) 
    {
        temporaryIdentifier += alphanum[dist(mt)];
    }

    return temporaryIdentifier;
}

Client::~Client()
{

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
        std::cerr << "Failed to send message to chat.\n";
    }
}

void Client::RequestToCreateChat(std::string& clients, const std::string& chatId)
{
    if (!clients.empty() && clients.back() == ' ') clients.pop_back();
    std::cout << "I am requesting: " << clients << ", to create chat " << chatId << '\n';
    std::string chatInfo = "create_chat:" + chatId;
    SendMessageToChat(clients, chatInfo);
    _chatId = static_cast<size_t>(stoi(chatId));
}

void Client::ReceiveMessage()
{
    while (true)
    {
        zmq::message_t action;
        zmq::message_t data;
        zmq::message_t messageId;
        _socket.recv(action, zmq::recv_flags::none);
        _socket.recv(data, zmq::recv_flags::none);
        _socket.recv(messageId, zmq::recv_flags::none);

        std::string actionStr = action.to_string();
        std::string dataStr = data.to_string();
        std::string messageIdStr = messageId.to_string();

        if (actionStr.substr(0, CREATE_CHAT_PREFIX_LENGTH) == "create_chat:" && !_isInChat)
        {
            _chatId = static_cast<size_t>(stoi(actionStr.substr(CREATE_CHAT_PREFIX_LENGTH)));
            std::cout << "[" << _identity << "]" << " I am invited to chat " << _chatId << '\n';
            _hasRequestToChat = true;
            std::cout << "[Server] Do you wish to create chat with " << dataStr << "? (y/n)\n";
        }
        else if (actionStr == "new_chat")
        {
            _chatId = std::stoi(dataStr);
            std::cout << "[Server] Now you are in chat with id=" << dataStr << '\n';
            _isInChat = true;
        }
        else if (actionStr == "incoming_message")
        {
            std::cout << messageIdStr << '\t' << dataStr << '\n';
        }
        else if (actionStr == "new_name")
        {
            ChangeIdentity(dataStr);
        }
        else
        {
            std::cout << "Error: unknown action!\n";
        }
    }
}

void Client::Reply(const std::string& reply)
{
    std::string chatIDstr = std::to_string(_chatId);

    if (reply == "y")
    {
        std::cout << "accepted!\n";
        SendMessageToChat(chatIDstr, "accept_create_chat");
    }
    else
    {
        SendMessageToChat(chatIDstr, "decline_create_chat");
        _chatId = NULL;
    }

    _hasRequestToChat = false;
}