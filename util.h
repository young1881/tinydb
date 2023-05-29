#ifndef UTIL_H
#define UTIL_H

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
        PREPARE_UNRECOGNIZED_STATEMENT
    };

#endif