#include <iostream>
#include <csignal>
#include <limits>

#include <vld.h>

#include <chat_ui.hpp>
#include <client.hpp>
#include <qt_message_observer.hpp>
#include <utils/helpers.hpp>

#define UI_TESTING_NO_CLIENT 1

namespace
{
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
                mtx.lock();
                std::cout << message->ChatID << '\t' << message->ID << '\t' << message->Author << '\t' << message->Content << '\n';
                mtx.unlock();
            }
        }
    }

    void threadProcess(std::shared_ptr<Client> client)
    {
        const size_t clientsListStartPos = 9;
        const size_t clientChangeNamePrefix = 13;
        std::string line;

        while (alive)
        {
            std::getline(std::cin, line);

            if (client->HasRequestToChat())
            {
                client->Reply(line);
            }
            else if (line.substr(0, clientsListStartPos) == "/connect:")
            {
                // example [/connect:cli1 cli2:55]
                size_t clientListEndsPos = line.rfind(':');
                std::string clientListStr = line.substr(clientsListStartPos, clientListEndsPos - clientsListStartPos);
                client->RequestToCreateChat(clientListStr, std::stoi(line.substr(clientListEndsPos + 1)));
            }
            else if (line.substr(0, clientChangeNamePrefix) == "/change_name:")
            {
                std::string identifierRawString = line.substr(clientChangeNamePrefix);
                std::string identifier = Utils::trim(identifierRawString);
                client->RequestChangeIdentity(identifier);
            }
            else if (line != "/quit")
            {
                client->SendMessageToChat(line, client->GetChatId());
            }
            else
            {
                alive = false;
                break;
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
    host = "tcp://localhost:5555";
    self = "debug_client";
#endif
    
    QApplication app{ argc, argv };
    
    auto messageQueue = std::make_shared<MessageQueue>();
    auto client = std::make_shared<Client>(host, self, messageQueue);
    auto messageObserver = std::make_shared<QtMessageObserver>();
    messageObserver->Subscribe(client);
    UI::ChatUI chat{ client };
    chat.resize(1280, 720);
    chat.show();

    std::thread t(threadProcess, client);
    QObject::connect(QApplication::instance(), &QApplication::aboutToQuit, [&t, ptr = &alive]() {
        *ptr = false;

        if (t.joinable())
        {
            t.join();
        }
    });

    return app.exec();
}