#include <iostream>
#include "server.hpp"

int main(int argc, char** argv)
{
#ifdef DEBUG
    std::string bindEndpoint = "tcp://localhost:5555";
#else
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <bind-endpoint>" << std::endl;
        return 1;
    }

    std::string bindEndpoint(argv[1]);
#endif

    Server server{ bindEndpoint };
    server.Run();

    return 0;
}
