#include "server.hpp"
#include <iostream>

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

        // ----------------------------Done-------------------------------------------
        if (actionStr == "!connect!")
        {
            _clients.insert(clientId);
            std::cout << "[Server] Client " << clientId << " connected." << std::endl;
        }
        // ---------------------------------------------------------------------------
        else if (actionStr == "send_message")
        {
            size_t delimeter = dataStr.find_first_of(":");
            std::stringstream ss;
            ss << clientId << ": " << dataStr.substr(delimeter + 1);

            size_t chatId = stoi(dataStr.substr(delimeter - 1));
            _callback("incoming_message", ss.str(), _activeChats[chatId]);
        }
        else if (actionStr == "create_chat")
        {

        }
    }
}

void Server::_callback(const std::string& action, const std::string& message, const std::unordered_set<std::string> clients) 
{
    for (const auto& client : clients) 
    {
        try
        {   
            if (action == "incoming_message")
            {
                zmq::message_t clientId(client);
                zmq::message_t actionStr(action);
                zmq::message_t data(message);

                _socket.send(clientId, zmq::send_flags::sndmore);
                _socket.send(actionStr, zmq::send_flags::sndmore);
                _socket.send(data, zmq::send_flags::none);
            }
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