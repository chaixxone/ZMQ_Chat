#include <iostream>
#include <limits>
#include "client.hpp"

int main(int argc, char** argv) 
{
    std::string host, self;

#ifdef NDEBUG
    if (argc < 3) 
    {
        std::cerr << "Usage: client <host> <identity>" << std::endl;
        return 1;
    }

    host = argv[1];
    self = argv[2];

    Client* client = new Client(host, self);
#else
    std::cin >> host >> self;
    Client client(host, self);
#endif

    std::string line;

    while (true) 
    {
        std::getline(std::cin, line);

        if (line.substr(0, 10) == "/connect:")
        {
            client->RequestToCreateChat(line.substr(10));
        }
        else if (line != "/quit")
        {
            client->SendMessageToChat(line);
        }
        else
        {
            delete client;
            break;
        }
    }

    return 0;
}