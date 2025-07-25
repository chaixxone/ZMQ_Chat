#include <iostream>
#include <thread>
#include <csignal>
#include <chrono>
#include <filesystem>

#include <vld.h>
#include <dotenv.h>

#include <server.hpp>
#include <database_connection.hpp>

using namespace std::chrono_literals;

namespace
{
    std::atomic_bool isInterrupted(false);

    void onInterruptionOccured(int)
    {
        isInterrupted.store(true);
    }
}

int main(int argc, char** argv)
{
#ifdef DEBUG
    std::string bindEndpoint = "tcp://localhost:5555";
#else
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <bind-endpoint>\n";
        return 1;
    }

    std::string bindEndpoint(argv[1]);
#endif

    std::string dotEnvFile = ".env";
    std::filesystem::path dotEnvFilePath = std::filesystem::current_path().append(dotEnvFile);

    if (!std::filesystem::exists(dotEnvFilePath))
    {
        std::cerr << ".env doesn't present in the same directory as the executable\n";
        return -1;
    }

    dotenv::init();
    std::string DB_HOST = std::getenv("DB_HOST");
    std::string DB_USER = std::getenv("DB_USER");
    std::string PASSWORD = std::getenv("PASSWORD");
    std::string SCHEMA = std::getenv("SCHEMA");

    std::unique_ptr<DatabaseConnection> db_conn = CreateDatabaseConnection(DB_HOST, DB_USER, PASSWORD, SCHEMA);

    std::signal(SIGINT, onInterruptionOccured);

    zmq::context_t context(1);
    Server server{ context, bindEndpoint, std::move(db_conn) };

    std::thread serverThread([&server]() {
        server.Run();
    });

    while (!isInterrupted.load())
    {
        std::this_thread::sleep_for(100ms);
    }

    std::cout << "[SYSTEM] Server stopped\n";
    context.shutdown();

    serverThread.join();

    return 0;
}
