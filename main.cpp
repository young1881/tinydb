#include "db.h"
#include <iostream>
#include <string>

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        std::cout << "Must supply a database filename." << std::endl;
        exit(EXIT_FAILURE);
    }
    
    DB db(argv[1]);
    db.start();
    return 0;
}