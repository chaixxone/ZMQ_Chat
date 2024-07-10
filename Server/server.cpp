#include "server.hpp"
#include <iostream>
#include <sstream>

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

        if (actionStr == "!connect!")
        {
            _clients.insert(clientId);
            std::cout << "[Server] Client " << clientId << " connected." << std::endl;
        }
        else if (actionStr == "send_message")
        {
            size_t delimiter = dataStr.find_first_of(":");
            std::stringstream ss;
            ss << clientId << ": " << dataStr.substr(delimiter + 1);
            size_t chatId;

            try
            {
                chatId = std::stoi(dataStr.substr(0, delimiter));
            }
            catch (const std::exception& exc)
            {
                std::cerr << "[Server] Refusing to take message from " << clientId << ": no correct chat id in dataFrame" << std::endl;
                continue;
            }

            _callback("incoming_message", ss.str(), _activeChats[chatId]);
        }
        else if (actionStr.substr(0, 12) == "create_chat:")
        {
            auto clients = _parseClients(dataStr);
            auto chatId = actionStr.substr(12);
            std::cout << "[Server] Client " << clientId << " asked to create a chat (" << chatId << ")with " << dataStr << std::endl;
            _askClients(std::make_pair(static_cast<size_t>(stoi(chatId)), identity.to_string()), std::vector<std::string>(clients.begin(), clients.end()));
            _activeChats[static_cast<size_t>(stoi(chatId))].insert(identity.to_string());
        }
        else if (actionStr == "accept_create_chat" || actionStr == "decline_create_chat")
        {
            std::cout << "Attension!" << std::endl;

            size_t chatId = std::stoi(dataStr);

            if (_pendingChatInvites.find(chatId) != _pendingChatInvites.end())
            {
                std::cout << "found someone!" << std::endl;

                if (actionStr == "accept_create_chat")
                {
                    _activeChats[chatId].insert(clientId);
                    std::cout << "[Server] Client " << clientId << " accepted chat invitation." << std::endl;

                    zmq::message_t actionChatFrame(std::string("new_chat"));
                    zmq::message_t chatIdFrame(std::to_string(chatId));

                    _socket.send(identity, zmq::send_flags::sndmore);
                    _socket.send(actionChatFrame, zmq::send_flags::sndmore);
                    _socket.send(chatIdFrame, zmq::send_flags::none);
                }
                else if (actionStr == "decline_create_chat")
                {
                    _pendingChatInvites[chatId].erase(clientId);
                    std::cout << "[Server] Client " << clientId << " declined chat invitation." << std::endl;
                }

                if (_pendingChatInvites[chatId].empty())
                {
                    _pendingChatInvites.erase(chatId);
                }
            }
        }
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

void Server::_askClients(const std::pair<size_t, std::string>& chatInfo, const std::vector<std::string>& clients)
{
    auto chatId = chatInfo.first;
    auto& asker = chatInfo.second;
    _pendingChatInvites[chatId].insert(asker);
    auto chatInfoStr = "create_chat:" + std::to_string(chatId);

    for (const auto& client : clients)
    {
        zmq::message_t clientFrame(client);
        zmq::message_t action(chatInfoStr);
        zmq::message_t data(asker);

        _socket.send(clientFrame, zmq::send_flags::sndmore);
        _socket.send(action, zmq::send_flags::sndmore);
        _socket.send(data, zmq::send_flags::none);

        _pendingChatInvites[chatId].insert(client);
    }
}

void Server::_callback(const std::string& action, const std::string& message, const std::unordered_set<std::string>& clients)
{
    for (const auto& client : clients)
    {
        try
        {
            zmq::message_t clientId(client);
            zmq::message_t actionStr(action);
            zmq::message_t data(message);

            _socket.send(clientId, zmq::send_flags::sndmore);
            _socket.send(actionStr, zmq::send_flags::sndmore);
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
