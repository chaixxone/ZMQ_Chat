#include "server.hpp"
#include <iostream>
#include <sstream>

enum class Action
{
    Connect,
    SendMessage,
    CreateChat,
    AcceptCreateChat,
    Unknown
};

static Action stringToAction(const std::string& actionStr)
{
    const short CREATE_CHAT_PREFX_LENGTH = 12;

    static const std::unordered_map<std::string, Action> actionMap = {
        {"!connect!", Action::Connect},
        {"send_message", Action::SendMessage},
        {"create_chat", Action::CreateChat},
        {"accept_create_chat", Action::AcceptCreateChat}
    };

    auto it = actionMap.find(actionStr);

    if (it != actionMap.end())
    {
        return it->second;
    }
    else if (actionStr.substr(0, CREATE_CHAT_PREFX_LENGTH) == "create_chat:")
    {
        return Action::CreateChat;
    }

    return Action::Unknown;
}

Server::Server(std::string binding) : _context(1), _socket(_context, zmq::socket_type::router)
{
    _socket.bind(binding);
}

void Server::Run()
{
    while (true)
    {
        zmq::message_t identity, action, data;
        _socket.recv(identity, zmq::recv_flags::none);
        _socket.recv(action, zmq::recv_flags::none);
        _socket.recv(data, zmq::recv_flags::none);

        std::string clientId = identity.to_string();
        std::string actionStr = action.to_string();
        std::string dataStr = data.to_string();
        std::cout << clientId << " " << actionStr << " " << dataStr << std::endl;

        Action actionEnum = stringToAction(actionStr);

        switch (actionEnum)
        {
        case Action::Connect:
            _clients.insert(clientId);
            std::cout << "[Server] Client " << clientId << " connected." << std::endl;
            break;
        case Action::SendMessage:
            _handleSendMessage(clientId, dataStr);
            break;
        case Action::CreateChat:
            _prepareNewChatSession(clientId, actionStr, dataStr);
            break;
        case Action::AcceptCreateChat:
            _handleResponseForInvite(identity, clientId, dataStr, true);
            break;
        default:
            _handleResponseForInvite(identity, clientId, dataStr, false);
            break;
        }
    }
}

void Server::_handleSendMessage(const std::string& clientId, const std::string& dataStr)
{
    size_t delimiter = dataStr.find_first_of(":");
    size_t chatId;

    try
    {
        chatId = std::stoi(dataStr.substr(0, delimiter));
    }
    catch (...)
    {
        std::cerr << "[Server] Refusing to take message from " << clientId << ": no correct chat id in dataFrame" << std::endl;
        return;
    }

    std::stringstream pureMessage;
    pureMessage << clientId << ": " << dataStr.substr(delimiter + 1);
    _callback("incoming_message", pureMessage.str(), _activeChats[chatId]);
}

void Server::_prepareNewChatSession(const std::string& clientId, const std::string& actionStr, const std::string& dataStr)
{
    auto clients = _parseClients(dataStr);
    auto chatIdStr = actionStr.substr(12);
    auto chatId = static_cast<size_t>(stoi(chatIdStr));
    std::cout << "[Server] Client " << clientId << " asked to create a chat (" << chatIdStr << ") with " << dataStr << std::endl;
    _askClients(std::make_pair(chatId, clientId), clients);
    _activeChats[chatId].insert(clientId);
}

void Server::_handleResponseForInvite(zmq::message_t& identity, const std::string& clientId, const std::string& dataStr, bool isAccepted)
{
    auto chatId = static_cast<size_t>(stoi(dataStr));

    if (_pendingChatInvites.find(chatId) != _pendingChatInvites.end())
    {
        if (!isAccepted)
        {
            _pendingChatInvites[chatId].erase(clientId);
            std::cout << "[Server] Client " << clientId << " declined chat invitation." << std::endl;
            return;
        }

        _activeChats[chatId].insert(clientId);
        std::cout << "[Server] Client " << clientId << " accepted chat invitation." << std::endl;

        zmq::message_t actionChatFrame(std::string("new_chat"));
        zmq::message_t chatIdFrame(std::to_string(chatId));

        _socket.send(identity, zmq::send_flags::sndmore);
        _socket.send(actionChatFrame, zmq::send_flags::sndmore);
        _socket.send(chatIdFrame, zmq::send_flags::none);
    }
}

std::unordered_set<std::string> Server::_parseClients(const std::string& clients)
{
    std::unordered_set<std::string> clientSet;
    std::stringstream ss(clients);
    std::string client;
    while (std::getline(ss, client, ' '))
    {
        clientSet.insert(client);
    }
    return clientSet;
}

void Server::_askClients(const std::pair<size_t, std::string>& chatInfo, const std::unordered_set<std::string>& clients)
{
    auto chatId = chatInfo.first;
    auto& asker = chatInfo.second;
    _pendingChatInvites[chatId].insert(asker);
    auto chatInfoStr = "create_chat:" + std::to_string(chatId);

    _callback(chatInfoStr, asker, clients);

    for (const auto& client : clients)
    {
        if (_clients.find(client) != _clients.end())
        {
            _pendingChatInvites[chatId].insert(client);
        }
        else
        {
            std::cerr << "[Server] Client " << client << " doesn't exist" << std::endl;
        }
    }
}

void Server::_callback(const std::string& actionStr, const std::string& message, const std::unordered_set<std::string>& clients)
{
    for (const auto& client : clients)
    {
        try
        {
            zmq::message_t clientId(client);
            zmq::message_t action(actionStr);
            zmq::message_t data(message);

            _socket.send(clientId, zmq::send_flags::sndmore);
            _socket.send(action, zmq::send_flags::sndmore);
            _socket.send(data, zmq::send_flags::none);
        }
        catch (zmq::error_t& e)
        {
            std::cerr << e.what() << std::endl;
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}
