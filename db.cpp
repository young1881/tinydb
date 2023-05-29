#include <iostream>
#include <string>
#include "db.h"


void DB::start()
{
    while (true)
    {
        print_prompt();
        
        std::string input_line;
        std::getline(std::cin, input_line);

        if (parse_meta_command(input_line))
        {
            continue;
        }
    }
}

void DB::print_prompt()
{
    std::cout << "db > ";
}
