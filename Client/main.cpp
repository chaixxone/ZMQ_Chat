#include <iostream>
#include <csignal>
#include <limits>
#include "client.hpp"

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

    bool alive = true;
    std::mutex mtx;

    void handleSigInt(int)
    {
        alive = false;
    }

    void printMessage(Client& client)
    {
        while (alive)
        {
            std::optional<MessageView> message = client.TryGetMessage();

            if (message)
            {                
                std::lock_guard<std::mutex> lock{ mtx };
                std::string messageIdStr = message.has_value() ? std::to_string(message->ID.value()) : "";
                std::cout << message->ChatID << '\t' << messageIdStr << '\t' << message->Author << '\t' << message->Content << '\n';
            }
        }
    }
}

int main(int argc, char** argv)
{
    std::signal(SIGINT, handleSigInt);
    std::string host, self;

#ifdef NDEBUG
    if (argc < 3)
    {
        std::cerr << "Usage: client <host> <identity>\n";
        return 1;
    }

    host = argv[1];
    self = argv[2];
#else
    std::cin >> host >> self;
#endif
    auto messageQueue = std::make_shared<MessageQueue>();
    Client client{ host, self, messageQueue };

    std::string line;
    std::cin.ignore();
    const size_t clientsListStartPos = 9;
    const size_t clientChangeNamePrefix = 13;

    std::thread messagePrinter(printMessage, std::ref(client));

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
            client.RequestToCreateChat(clientListStr, std::stoi(line.substr(clientListEndsPos + 1)));
        }
        else if (line.substr(0, clientChangeNamePrefix) == "/change_name:")
        {
            std::string identifierRawString = line.substr(clientChangeNamePrefix);
            std::string identifier = trim(identifierRawString);
            client.RequestChangeIdentity(identifier);
        }
        else if (line != "/quit")
        {
            client.SendMessageToChat(line, client.GetChatId());
        }
        else
        {
            alive = false;
            break;
        }
    }

    if (messagePrinter.joinable())
    {
        messagePrinter.join();
    }

    return 0;
}