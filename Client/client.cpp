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
    _chatId(-1),
    _hasRequestToChat(false)
{
    _socket.set(zmq::sockopt::routing_id, _identity);
    _socket.set(zmq::sockopt::linger, 0);
    _socket.connect(endpoint);

    SendRequest(identity, Utils::Action::Connect, -1);

    _receiver = std::thread(&Client::ReceiveMessage, this);
}

void Client::Attach(std::shared_ptr<IMessageObserver> messageObserver)
{
    _messageObserver = messageObserver;
}

void Client::RequestChangeIdentity(std::string& desiredIdentity)
{    
    SendRequest(desiredIdentity, Utils::Action::ChangeName, -1);
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

int Client::GetChatId() const noexcept
{
    return _chatId;
}

void Client::SendRequest(std::string& messageStr, Utils::Action action, int chatIdInt)
{
    std::string actionStr = Utils::actionToString(action);
    zmq::message_t actionFrame(actionStr);
    zmq::message_t message(messageStr);
    zmq::message_t chatId(std::to_string(chatIdInt));

    bool result = _socket.send(actionFrame, zmq::send_flags::sndmore) 
        && _socket.send(message, zmq::send_flags::sndmore)
        && _socket.send(chatId, zmq::send_flags::none);

    if (!result)
    {
        std::cerr << "Failed to send message to chat.\n";
    }
}

void Client::SendMessageToChat(std::string& messageStr, int chatIdInt)
{
    SendRequest(messageStr, Utils::Action::SendMessage, chatIdInt);
}

void Client::RequestToCreateChat(std::string& clients, int chatId)
{
    if (!clients.empty() && clients.back() == ' ') clients.pop_back();
    std::cout << "I am requesting: " << clients << ", to create chat " << chatId << '\n';
    SendRequest(clients, Utils::Action::CreateChat, chatId);
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

void Client::ReceiveMessage()
{
    while (_alive)
    {
        zmq::message_t action;
        zmq::message_t data;
        zmq::message_t messageId;
        zmq::message_t author;
        zmq::message_t chatId;
        bool messageReceivedResult = _socket.recv(action, zmq::recv_flags::dontwait)
            && _socket.recv(data, zmq::recv_flags::dontwait)
            && _socket.recv(messageId, zmq::recv_flags::dontwait)
            && _socket.recv(author, zmq::recv_flags::dontwait)
            && _socket.recv(chatId, zmq::recv_flags::dontwait);

        if (messageReceivedResult)
        {
            std::string actionStr = action.to_string();
            std::string dataStr = data.to_string();
            std::string messageIdStr = messageId.to_string();
            std::string authorStr = author.to_string();

            Utils::Action actionEnum = Utils::stringToAction(actionStr);
            int chatIdInt = std::stoi(chatId.to_string());
            std::optional<size_t> messageId = messageIdStr.empty() ? std::nullopt : std::optional(std::stoull(messageIdStr));
            _messageQueue->Enqueue(MessageView{ authorStr, dataStr, messageId, chatIdInt, actionEnum });

            if (_messageObserver)
            {
                _messageObserver->Update();
            }

            switch (actionEnum)
            {
            case Utils::Action::CreateChat:
                std::cout << "[" << _identity << "]" << " I am invited to chat " << chatIdInt << '\n';
                _hasRequestToChat = true;
                _pendingChatId = chatIdInt;
                std::cout << "[Server] Do you wish to create chat with " << dataStr << "? (y/n)\n";
                break;
            case Utils::Action::NewChat:
                _chatId = std::stoi(dataStr);
                std::cout << "[Server] Now you are in chat with id=" << dataStr << '\n';
                break;
            case Utils::Action::IncomingMessage:
                break;
            case Utils::Action::NewClientName:
                ChangeIdentity(dataStr);
                break;
            default:
                std::cout << "Error: unknown action!\n";
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void Client::Reply(const std::string& reply)
{
    std::string chatIDstr = std::to_string(_pendingChatId);

    if (reply == "y")
    {
        std::cout << "accepted!\n";
        SendRequest(chatIDstr, Utils::Action::AcceptCreateChat, _pendingChatId);
    }
    else
    {
        SendRequest(chatIDstr, Utils::Action::Unknown, _pendingChatId);
    }

    _hasRequestToChat = false;
}