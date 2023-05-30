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

        Statement statement;

        if (parse_statement(input_line, statement))
        {
            continue;
        }

        execute_statement(statement);
    }
}

void DB::print_prompt()
{
    std::cout << "db > ";
}

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
        delete(table);
        std::cout << "Bye!" << std::endl;
        exit(EXIT_SUCCESS);
    }
    else
    {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult DB::prepare_statement(std::string &input_line, Statement &statement)
{
    if (!input_line.compare(0, 6, "insert"))
    {
        return prepare_insert(input_line, statement);
    }
    else if (!input_line.compare(0, 6, "select"))
    {
        statement.type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    else
    {
        return PREPARE_UNRECOGNIZED_STATEMENT;
    }
}

PrepareResult DB::prepare_insert(std::string &input_line, Statement &statement)
{
    statement.type = STATEMENT_INSERT;

    char *insert_line = (char *)input_line.c_str();
    char *keyword = strtok(insert_line, " ");
    char *id_string = strtok(NULL, " ");
    char *username = strtok(NULL, " ");
    char *email = strtok(NULL, " ");

    if (id_string == NULL || username == NULL || email == NULL)
    {
        return PREPARE_SYNTAX_ERROR;
    }
    int id = atoi(id_string);
    if (id < 0) {
        return PREPARE_NEGATIVE_ID;
    }
    if (strlen(username) > COLUMN_USERNAME_SIZE || strlen(email) > COLUMN_EMAIL_SIZE)
    {
        return PREPARE_TOO_LONG_STRING;
    }
    
    statement.row_to_insert = Row(id, username, email);
    return PREPARE_SUCCESS;
}

bool DB::parse_statement(std::string &input_line, Statement &statement)
{
    switch (prepare_statement(input_line, statement))
    {
    case PREPARE_SUCCESS:
        return false;
    case PREPARE_SYNTAX_ERROR:
        std::cout << "Syntax error. Could not parse statement." << std::endl;
        return true;
    case PREPARE_NEGATIVE_ID:
        std::cout << "ID must be positive." << std::endl;
        return true;
    case PREPARE_TOO_LONG_STRING:
        std::cout << "String is too long." << std::endl;
        return true;
    case PREPARE_UNRECOGNIZED_STATEMENT:
        std::cout << "Unrecognized keyword at start of '" << input_line << "'." << std::endl;
        return true;
    default:
        break;
    }
    return false;
}

void DB::execute_statement(Statement &statement)
{
    ExecuteResult result;
    switch (statement.type)
    {
    case STATEMENT_INSERT:
        result = execute_insert(statement);
        break;
    case STATEMENT_SELECT:
        result = execute_select(statement);
        break;
    }

    switch (result)
    {
    case EXECUTE_SUCCESS:
        std::cout << "Executed." << std::endl;
        break;
    case EXECUTE_TABLE_FULL:
        std::cout << "Error: Table full." << std::endl;
        break;
    }
}

ExecuteResult DB::execute_insert(Statement &statement)
{
    if (table->num_rows >= TABLE_MAX_ROWS)
    {
        std::cout << "Error: Table full." << std::endl;
        return EXECUTE_TABLE_FULL;
    }

    void *page = table->row_slot(table->num_rows);
    serialize_row(statement.row_to_insert, page);
    table->num_rows++;

    return EXECUTE_SUCCESS;
}

ExecuteResult DB::execute_select(Statement &statement)
{
    for (uint32_t i = 0; i < table->num_rows; i++)
    {
        Row row;
        void *page = table->row_slot(i);
        deserialize_row(page, row);
        std::cout << "(" << row.id << ", " << row.username << ", " << row.email << ")" << std::endl;
    }

    return EXECUTE_SUCCESS;
}