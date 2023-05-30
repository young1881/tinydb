#ifndef DB_H
#define DB_H

#include "table.h"
#include <iostream>
#include <string>

enum MetaCommandResult
{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
};

enum StatementType
{
    STATEMENT_INSERT,
    STATEMENT_SELECT
};

enum PrepareResult
{
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR,
    PREPARE_NEGATIVE_ID,
    PREPARE_TOO_LONG_STRING,
};

enum ExecuteResult
{
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
};

class Statement
{
public:
    StatementType type;
    Row row_to_insert;
};

class DB
{
private:
    Table* table;

public:
    DB(const char *filename)
    {
        table = new Table(filename);
    }

    void start();
    void print_prompt();

    bool parse_meta_command(std::string command);
    MetaCommandResult do_meta_command(std::string command);

    PrepareResult prepare_statement(std::string &input_line, Statement &statement);
    PrepareResult prepare_insert(std::string &input_line, Statement &statement);
    bool parse_statement(std::string &input_line, Statement &statement);
    
    void execute_statement(Statement &statement);
    ExecuteResult execute_insert(Statement &statement);
    ExecuteResult execute_select(Statement &statement);

    ~DB()
    {
        delete table;
    }
};
#endif
