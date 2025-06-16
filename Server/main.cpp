#include <iostream>
#include <stacktrace>
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

    try
    {
        Server server{ bindEndpoint };
        server.Run();
    }
    catch (const zmq::error_t& e)
    {
#ifdef __GNUC__
        std::cerr << std::stacktrace::current() << '\n';
#endif
        std::cerr << e.what() << '\n';
    }

    return 0;
}
