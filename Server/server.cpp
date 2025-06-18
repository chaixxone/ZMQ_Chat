#include "server.hpp"
#include <iostream>
#include <sstream>

namespace 
{
    enum class Action
    {
        Connect,
        SendMessage,
        CreateChat,
        AcceptCreateChat,
        Unknown
    };

    const short CREATE_CHAT_PREFIX_LENGTH = 12;

    Action stringToAction(const std::string& actionStr)
    {

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
        else if (actionStr.substr(0, CREATE_CHAT_PREFIX_LENGTH) == "create_chat:")
        {
            return Action::CreateChat;
        }

        return Action::Unknown;
    }
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
        std::cout << clientId << " " << actionStr << " " << dataStr << '\n';

        Action actionEnum = stringToAction(actionStr);

        switch (actionEnum)
        {
        case Action::Connect:
            HandleConnection(identity, dataStr);
            break;
        case Action::SendMessage:
            HandleSendMessage(clientId, dataStr);
            break;
        case Action::CreateChat:
            PrepareNewChatSession(clientId, actionStr, dataStr);            
            break;
        case Action::AcceptCreateChat:
            HandleResponseForInvite(identity, clientId, dataStr, true);
            break;
        default:
            HandleResponseForInvite(identity, clientId, dataStr, false);
            break;
        }
    }
}

void Server::HandleConnection(zmq::message_t& clientId, const std::string& desiredIdentity)
{
    if (_clients.find(desiredIdentity) != _clients.end())
    {
        MessageDispatch("bad_name", desiredIdentity, { clientId.to_string() });
        return;
    }

    static std::string actionNewNameStr = "new_name";
    MessageDispatch(actionNewNameStr, desiredIdentity, { clientId.to_string() });

    _clients.insert(desiredIdentity);
    std::cout << "[Server] Client " << desiredIdentity << " connected.\n";
}

void Server::HandleSendMessage(const std::string& clientId, const std::string& dataStr)
{
    size_t delimiter = dataStr.find_first_of(":");
    size_t chatId;

    try
    {
        chatId = std::stoi(dataStr.substr(0, delimiter));
    }
    catch (...)
    {
        std::cerr << "[Server] Refusing to take message from " << clientId << ": no correct chat id in dataFrame\n";
        return;
    }

    std::stringstream pureMessage;
    pureMessage << clientId << ": " << dataStr.substr(delimiter + 1);
    MessageDispatch("incoming_message", pureMessage.str(), _activeChats[chatId]);
}

void Server::PrepareNewChatSession(const std::string& clientId, const std::string& actionStr, const std::string& dataStr)
{
    auto clients = ParseClients(dataStr, clientId);
    auto chatIdStr = actionStr.substr(CREATE_CHAT_PREFIX_LENGTH);
    auto chatId = static_cast<size_t>(stoi(chatIdStr));
    std::cout << "[Server] Client " << clientId << " asked to create a chat (" << chatIdStr << ") with " << dataStr << '\n';
    AskClients(std::make_pair(chatId, clientId), clients);
    _activeChats[chatId].insert(clientId);
    MessageDispatch("new_chat", std::to_string(chatId), { clientId });
}

void Server::HandleResponseForInvite(zmq::message_t& identity, const std::string& clientId, const std::string& dataStr, bool isAccepted)
{
    auto chatId = static_cast<size_t>(stoi(dataStr));

    if (_pendingChatInvites.find(chatId) != _pendingChatInvites.end())
    {
        _pendingChatInvites[chatId].erase(clientId);

        if (!isAccepted)
        {
            std::cout << "[Server] Client " << clientId << " declined chat invitation.\n";
            return;
        }

        _activeChats[chatId].insert(clientId);
        std::cout << "[Server] Client " << clientId << " accepted chat invitation.\n";

        zmq::message_t actionChatFrame(std::string("new_chat"));
        zmq::message_t chatIdFrame(std::to_string(chatId));

        _socket.send(identity, zmq::send_flags::sndmore);
        _socket.send(actionChatFrame, zmq::send_flags::sndmore);
        _socket.send(chatIdFrame, zmq::send_flags::none);
    }
}

std::unordered_set<std::string> Server::ParseClients(const std::string& clients, const std::string& creator)
{
    std::unordered_set<std::string> clientSet;
    std::stringstream ss(clients);
    std::string client;
    while (std::getline(ss, client, ' '))
    {
        if (client != creator)
        {
            clientSet.insert(client);
        }
    }
    return clientSet;
}

void Server::AskClients(const std::pair<size_t, std::string>& chatInfo, const std::unordered_set<std::string>& clients)
{
    auto chatId = chatInfo.first;
    auto& asker = chatInfo.second;
    auto chatInfoStr = "create_chat:" + std::to_string(chatId);

    MessageDispatch(chatInfoStr, asker, clients);

    for (const auto& client : clients)
    {
        if (_clients.find(client) != _clients.end())
        {
            _pendingChatInvites[chatId].insert(client);
        }
        else
        {
            std::cerr << "[Server] Client " << client << " doesn't exist\n";
        }
    }
}

void Server::MessageDispatch(const std::string& actionStr, const std::string& message, const std::unordered_set<std::string>& clients)
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
