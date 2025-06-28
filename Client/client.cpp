#include "client.hpp"
#include <utils/client_actions.hpp>
#include <utils/helpers.hpp>
#include <iostream>
#include <random>

Client::Client(std::string endpoint, std::string identity, std::shared_ptr<MessageQueue> message_queue) :
    _context(1), 
    _socket(_context, zmq::socket_type::dealer), 
    _endpoint(endpoint), 
    _identity(GenerateTemporaryId()), 
    _messageQueue(message_queue),
    _isInChat(false), 
    _chatId(-1),
    _hasRequestToChat(false)
{
    _socket.set(zmq::sockopt::routing_id, _identity);
    _socket.set(zmq::sockopt::linger, 0);
    _socket.connect(endpoint);

    SendMessageToChat(identity, "!connect!", -1);

    _receiver = std::thread(&Client::ReceiveMessage, this);
}

void Client::RequestChangeIdentity(std::string& desiredIdentity)
{    
    SendMessageToChat(desiredIdentity, "!connect!", -1);
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
    _alive = false;
    if (_receiver.joinable())
    {
        _receiver.join();
    }
}

bool Client::HasRequestToChat() const
{
    return _hasRequestToChat;
}

void Client::SendMessageToChat(std::string& messageStr, const std::string& actionStr, int chatIdInt)
{
    zmq::message_t action(actionStr);
    zmq::message_t message(messageStr);
    zmq::message_t chatId(std::to_string(chatIdInt));

    bool result = _socket.send(action, zmq::send_flags::sndmore) 
        && _socket.send(message, zmq::send_flags::sndmore)
        && _socket.send(chatId, zmq::send_flags::none);

    if (!result)
    {
        std::cerr << "Failed to send message to chat.\n";
    }
}

void Client::RequestToCreateChat(std::string& clients, int chatId)
{
    if (!clients.empty() && clients.back() == ' ') clients.pop_back();
    std::cout << "I am requesting: " << clients << ", to create chat " << chatId << '\n';
    SendMessageToChat(clients, "create_chat", chatId);
    _chatId = chatId;
}

std::optional<MessageView> Client::TryGetMessage()
{
    if (_messageQueue->IsEmpty())
    {
        return std::nullopt;
    }

    return _messageQueue->Pop();
}

void Client::SendRequest(std::string& requestData, Utils::Action request)
{
    const size_t clientsListStartPos = 9;
    const size_t clientChangeNamePrefix = 13;

    switch (request)
    {
    case Utils::Action::CreateChat:
    {
        // example [/connect:cli1 cli2:55]
        size_t clientListEndsPos = requestData.rfind(':');
        std::string clientListStr = requestData.substr(clientsListStartPos, clientListEndsPos - clientsListStartPos);
        RequestToCreateChat(clientListStr, std::stoi(requestData.substr(clientListEndsPos + 1)));
        break;
    }
    case Utils::Action::ChangeName:
    {
        std::string identifierRawString = requestData.substr(clientChangeNamePrefix);
        std::string identifier = Utils::trim(identifierRawString);
        RequestChangeIdentity(identifier);
        break;
    }
    case Utils::Action::SendMessage:
        SendMessageToChat(requestData, "send_message", _chatId);
        break;
    default:
        break;
    }
}

void Client::ReceiveMessage()
{
    while (_alive)
    {
        zmq::message_t action;
        zmq::message_t data;
        zmq::message_t messageId;
        zmq::message_t author;
        zmq::message_t chatId;
        auto actionResult = _socket.recv(action, zmq::recv_flags::dontwait);
        auto dataResult = _socket.recv(data, zmq::recv_flags::dontwait);
        auto messageIdResult = _socket.recv(messageId, zmq::recv_flags::dontwait);
        auto authorResult = _socket.recv(author, zmq::recv_flags::dontwait);
        auto chatIdResult = _socket.recv(chatId, zmq::recv_flags::dontwait);

        if (actionResult && dataResult && messageIdResult)
        {
            std::string actionStr = action.to_string();
            std::string dataStr = data.to_string();
            std::string messageIdStr = messageId.to_string();
            std::string authorStr = author.to_string();
            std::string chatIdStr = chatId.to_string();
            _messageQueue->Enqueue(MessageView{ authorStr, dataStr, messageIdStr, std::stoi(chatIdStr) });

            if (actionStr.substr(0, Utils::CREATE_CHAT_PREFIX_LENGTH) == "create_chat:" && !_isInChat)
            {
                _chatId = stoi(actionStr.substr(Utils::CREATE_CHAT_PREFIX_LENGTH));
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
                // std::cout << messageIdStr << '\t' << dataStr << '\t' << authorStr << '\t' << chatIdStr << '\n';
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

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void Client::Reply(const std::string& reply)
{
    std::string chatIDstr = std::to_string(_chatId);

    if (reply == "y")
    {
        std::cout << "accepted!\n";
        SendMessageToChat(chatIDstr, "accept_create_chat", _chatId);
    }
    else
    {
        SendMessageToChat(chatIDstr, "decline_create_chat", _chatId);
    }

    _hasRequestToChat = false;
}