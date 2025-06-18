#include <iostream>
#include "server.hpp"

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

    try
    {
        Server server{ bindEndpoint };
        server.Run();
    }
    catch (const zmq::error_t& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
