#include <iostream>
#include "server.hpp"

int main(int argc, char** argv)
{
#ifdef DEBUG
    std::strcpy(argv[1], "tcp://localhost:5555");
#else
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <bind-endpoint>" << std::endl;
        return 1;
    }
#endif

    Server server{ argv[1] };
    server.Run();

    return 0;
}
