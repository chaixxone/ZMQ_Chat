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
#else
    std::cin >> host >> self;
#endif
    Client client{ host, self };

    std::string line;
    std::cin.ignore();
    const size_t clientsListStartPos = 9;

    while (true)
    {
        std::getline(std::cin, line);

        if (client.HasRequestToChat())
        {
            client.Reply(line);
        }
        else if (line.substr(0, clientsListStartPos) == "/connect:")
        {
            // example [/connect:cli1 cli2:55]
            size_t clientListEndsPos = line.rfind(':');
            std::string clientListStr = line.substr(clientsListStartPos, clientListEndsPos - clientsListStartPos);
            client.RequestToCreateChat(clientListStr, line.substr(clientListEndsPos + 1));
        }
        else if (line != "/quit")
        {
            client.SendMessageToChat(line);
        }
        else
        {
            break;
        }
    }

    return 0;
}