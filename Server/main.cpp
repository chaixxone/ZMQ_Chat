// ----------------------------Don't-touch-this!-----------------------------

#include <iostream>
#include "server.hpp"
#include <memory>

int main(int argc, char** argv)
{
#ifndef NDEBUG
    argv[1] = "tcp://localhost:5555";
#else
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <bind-endpoint>" << std::endl;
        return 1;
    }
#endif

    Server* server = new Server(argv[1]);
    server->Run();
    
    delete server;

    return 0;
}

// ---------------------------------------------------------------------------