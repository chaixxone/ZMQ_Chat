#include <iostream>
#include <csignal>
#include <limits>
#include <fstream>

#include <vld.h>
#include <nlohmann/json.hpp>

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
                std::lock_guard<std::mutex> lock{ mtx };
                std::string messageIdStr = message.has_value() ? std::to_string(message->ID.value()) : "";
                std::cout << message->ChatID << '\t' << messageIdStr << '\t' << message->Author << '\t' << message->Content << '\n';
            }
        }
    }

    void processClientConsoleInput(std::shared_ptr<Client> client)
    {
        const size_t clientsListStartPos = 9;
        const size_t clientChangeNamePrefix = 13;
        std::string line;

        while (alive)
        {
            std::getline(std::cin, line);

            if (client->HasRequestToChat())
            {
                client->ReplyChatInvite(line);
            }
            else if (line.substr(0, clientsListStartPos) == "/connect:")
            {
                // example [/connect:cli1 cli2:55]
                // UPD: example [/connect:cli1 cli2]
                std::string clientListStr = line.substr(clientsListStartPos);
                client->RequestToCreateChat(clientListStr);
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

    std::string dotEnvFile = "conf.json";
    std::filesystem::path configFilePath = std::filesystem::current_path().append(dotEnvFile);

    if (!std::filesystem::exists(configFilePath))
    {
        std::cerr << "conf.json doesn't present in the same directory as the executable\n";
        return -1;
    }
    
    QApplication app{ argc, argv };
    
    auto messageQueue = std::make_shared<MessageQueue>();
    auto client = std::make_shared<Client>(host, self, messageQueue, configFilePath.string());
    auto messageObserver = std::make_shared<QtMessageObserver>();
    messageObserver->Subscribe(client);
    UI::ChatUI chat{ client, messageObserver };
    chat.resize(1280, 720);
    chat.show();

    std::thread clientConsoleInputThread(processClientConsoleInput, client);
    QObject::connect(QApplication::instance(), &QApplication::aboutToQuit, [&clientConsoleInputThread, threadAlivePtr = &alive]() {
        *threadAlivePtr = false;

        if (clientConsoleInputThread.joinable())
        {
            clientConsoleInputThread.join();
        }
    });

    return app.exec();
}