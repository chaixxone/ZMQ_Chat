#include <iostream>
#include <limits>
#include "client.hpp"
#include <chat_ui.hpp>
#include <vld.h>
#define UI_TESTING_NO_CLIENT 1

namespace
{
    std::string trim(const std::string& s)
    {
        size_t lindex = 0;
        size_t size = s.size();

        while (lindex < size && s[lindex] == ' ')
        {
            lindex++;
        }

        size_t rindex = lindex;

        while (rindex < size && s[rindex] != ' ')
        {
            rindex++;
        }

        return std::string(s.begin() + lindex, s.begin() + rindex);
    }
}

int main(int argc, char** argv)
{
    std::string host, self;

#ifdef NDEBUG
    if (argc < 3)
    {
        std::cerr << "Usage: client <host> <identity>\n";
        return 1;
    }

    host = argv[1];
    self = argv[2];
#elif !UI_TESTING_NO_CLIENT
    std::cin >> host >> self;
#endif

#if !UI_TESTING_NO_CLIENT
    Client client{ host, self };
#endif

    std::string line;
    std::cin.ignore();
    const size_t clientsListStartPos = 9;
    const size_t clientChangeNamePrefix = 13;
    
    QApplication app{ argc, argv };
    UI::ChatUI chat;
    chat.resize(1280, 720);
    chat.show();

#if !UI_TESTING_NO_CLIENT
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
        else if (line.substr(0, clientChangeNamePrefix) == "/change_name:")
        {
            std::string identifierRawString = line.substr(clientChangeNamePrefix);
            std::string identifier = trim(identifierRawString);
            client.RequestChangeIdentity(identifier);
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
#endif

    return app.exec();
}