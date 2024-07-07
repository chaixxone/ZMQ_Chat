#include "client.hpp"
#include <iostream>

Client::Client(std::string endpoint, std::string identity) : _context(1), _socket(_context, zmq::socket_type::dealer), _identity(identity), _isInChat(false)
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

// ------------------------------------Ready-to-use------------------------------------------
void Client::SendMessageToChat(const std::string& messageStr, const std::string& actionStr)
{
    zmq::message_t action(actionStr);
    zmq::message_t message(messageStr);
    _socket.send(action, zmq::send_flags::sndmore);
    _socket.send(message, zmq::send_flags::none);
}

void Client::RequestToCreateChat(std::string& clients)
{
    if (clients.back() == ' ') clients.pop_back();
    SendMessageToChat(clients, "create_chat");
}
// ------------------------------------------------------------------------------------------

void Client::_receiveMessage() 
{
    while (true) 
    {
        zmq::message_t action, data;
        _socket.recv(action);
        _socket.recv(data);
        std::string actionStr = action.to_string();
        std::string dataStr = data.to_string();

        // -------------Have-to-be-done-with-server's-askClients-method----------------------
        if (actionStr == "create_chat" && !_isInChat)
        {
            std::cout << "[Server] Do you wish to create chat with " << data << "? (y/n)" << std::endl;
            std::string userInput;
            std::cin >> userInput;

            if (userInput == "y")
            {
                SendMessageToChat("yes", "accept_create_chat");
            }
            else 
            {
                SendMessageToChat("no", "decline_create_chat");
            }
        }
        // -----------------------------------------------------------------------------------
        else if (actionStr == "new_chat")
        {
            _chatId = stoi(dataStr);
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