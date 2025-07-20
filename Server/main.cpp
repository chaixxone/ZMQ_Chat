#include <iostream>
#include <thread>
#include <csignal>
#include <chrono>
#include <vld.h>
#include "server.hpp"

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

    std::signal(SIGINT, onInterruptionOccured);

    zmq::context_t context(1);
    Server server{ context, bindEndpoint };

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
