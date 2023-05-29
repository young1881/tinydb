#include "util.h"
#include "db.h"


bool DB::parse_meta_command(std::string command)
{
    if (command[0] == '.')
    {
        switch (do_meta_command(command))
        {
        case META_COMMAND_SUCCESS:
            return true;
        case META_COMMAND_UNRECOGNIZED_COMMAND:
            std::cout << "Unrecognized command: " << command << std::endl;
            return true;
        }
    }
    return false;
}

MetaCommandResult DB::do_meta_command(std::string command)
{
    if (command == ".exit")
    {
        std::cout << "Bye!" << std::endl;
        exit(EXIT_SUCCESS);
    }
    else
    {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult DB::prepare_statement(std::string &input_line, Statement &statement) {
    if (!input_line.compare(0, 6, "insert")) {
        statement.type = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    }
    else if (!input_line.compare(0, 6, "select")) {
        statement.type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    else {
        return PREPARE_UNRECOGNIZED_STATEMENT;
    }
}