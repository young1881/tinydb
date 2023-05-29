#ifndef DB_H
#define DB_H

#include "util.h"
#include <iostream>
#include <string>


class DB
{
public:
    void start();
    void print_prompt();

    bool parse_meta_command(std::string command);
    MetaCommandResult do_meta_command(std::string command);
    PrepareResult prepare_statement(std::string &input_line, Statement &statement)
};

#endif