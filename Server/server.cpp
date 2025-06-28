#include "server.hpp"
#include <iostream>
#include <sstream>
#include <utils/client_actions.hpp>

Server::Server(std::string binding) : _context(1), _socket(_context, zmq::socket_type::router)
{
    _socket.bind(binding);
}

void Server::Run()
{
    while (true)
    {
        zmq::message_t identity, action, data, chatId;
        auto identityResult = _socket.recv(identity, zmq::recv_flags::none);
        auto actionResult = _socket.recv(action, zmq::recv_flags::none);
        auto dataResult = _socket.recv(data, zmq::recv_flags::none);
        auto chatIdResult = _socket.recv(chatId, zmq::recv_flags::none);

        std::string clientId = identity.to_string();
        std::string actionStr = action.to_string();
        std::string dataStr = data.to_string();
        std::string chatIdStr = chatId.to_string();
        int chatIdNumber = std::stoi(chatIdStr);
        std::cout << clientId << " " << actionStr << " " << dataStr << chatIdNumber << '\n';

        Utils::Action actionEnum = Utils::stringToAction(actionStr);

        switch (actionEnum)
        {
        case Utils::Action::Connect:
            HandleConnection(identity, dataStr);
            break;
        case Utils::Action::SendMessage:
            HandleSendMessage(clientId, dataStr, chatIdNumber);
            break;
        case Utils::Action::CreateChat:
            PrepareNewChatSession(clientId, dataStr, chatIdNumber);
            break;
        case Utils::Action::AcceptCreateChat:
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

void Server::HandleSendMessage(const std::string& clientId, const std::string& dataStr, int chatId)
{
    size_t delimiter = dataStr.find_first_of(":");

    static size_t messageId = 0;

    if (chatId == -1)
    {
        std::cerr << "[Server] Refusing to take message from " << clientId << ": no correct chat id in dataFrame\n";
        return;
    }

    MessageDispatch("incoming_message", dataStr, _activeChats[chatId], std::to_string(messageId++), clientId, chatId);
}

void Server::PrepareNewChatSession(const std::string& clientId, const std::string& dataStr, int chatId)
{
    auto clients = ParseClients(dataStr, clientId);

    std::cout << "[Server] Client " << clientId << " asked to create a chat (" << chatId << ") with " << dataStr << '\n';

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

        MessageDispatch("new_chat", std::to_string(chatId), { clientId });
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

    MessageDispatch(chatInfoStr, asker, clients, "", asker, chatId);

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

void Server::MessageDispatch(
    const std::string& actionStr, 
    const std::string& message, 
    const std::unordered_set<std::string>& clients,
    const std::string& messageIdStr,
    const std::string& authorStr,
    int chatIdInt
)
{
    for (const auto& client : clients)
    {
        try
        {
            zmq::message_t clientId(client);
            zmq::message_t action(actionStr);
            zmq::message_t data(message);
            zmq::message_t messageId(messageIdStr);
            zmq::message_t author(authorStr);
            zmq::message_t chatId(std::to_string(chatIdInt));

            _socket.send(clientId, zmq::send_flags::sndmore);
            _socket.send(action, zmq::send_flags::sndmore);
            _socket.send(data, zmq::send_flags::sndmore);
            _socket.send(messageId, zmq::send_flags::sndmore);
            _socket.send(author, zmq::send_flags::sndmore);
            _socket.send(chatId, zmq::send_flags::none);
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
