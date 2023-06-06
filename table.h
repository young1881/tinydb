#ifndef TABLE_H
#define TABLE_H

#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)
#define TABLE_MAX_PAGES 100

class Row
{
public:
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
    Row()
    {
        id = 0;
        username[0] = '\0';
        email[0] = '\0';
    }
    Row(u_int32_t id, const char *username, const char *email)
    {
        this->id = id;
        strncpy(this->username, username, COLUMN_USERNAME_SIZE + 1);
        strncpy(this->email, email, COLUMN_EMAIL_SIZE + 1);
    }
};

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

class Pager
{
private:
    int file_descriptor;
    uint32_t file_length;
    void *pages[TABLE_MAX_PAGES];
    uint32_t num_pages;

public:
    Pager(const char *filename);
    void *get_page(uint32_t page_num);
    void pager_flush(uint32_t page_num);

    friend class Table;
};

enum NodeType
{
    NODE_INTERNAL,
    NODE_LEAF
};

/*
 * Common Node Header Layout
 */
const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
const uint8_t COMMON_NODE_HEADER_SIZE =
    NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

/*
 * Leaf Node Header Layout
 */
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE =
    COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE;

/*
 * Leaf Node Body Layout
 */
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET =
    LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS =
    LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

class LeafNode
{
private:
    void *node;

public:
    LeafNode(void *node) : node(node) {}
    void initialize_leaf_node()
    {
        *leaf_node_num_cells() = 0;
    }
    uint32_t *leaf_node_num_cells()
    {
        return (uint32_t *)((char *)node + LEAF_NODE_NUM_CELLS_OFFSET);
    }
    void *leaf_node_cell(uint32_t cell_num)
    {
        return (char *)node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
    }
    uint32_t *leaf_node_key(uint32_t cell_num)
    {
        return (uint32_t *)leaf_node_cell(cell_num);
    }
    void *leaf_node_value(uint32_t cell_num)
    {
        return (char *)leaf_node_cell(cell_num) + LEAF_NODE_KEY_SIZE;
    }
    void print_leaf_node()
    {
        uint32_t num_cells = *leaf_node_num_cells();
        std::cout << "leaf (size " << num_cells << ")" << std::endl;
        for (uint32_t i = 0; i < num_cells; i++)
        {
            uint32_t key = *leaf_node_key(i);
            std::cout << "  - " << i << " : " << key << std::endl;
        }
    }
};

class Table
{
private:
    uint32_t root_page_num;
    Pager pager;

public:
    Table(const char *filename) : pager(filename)
    {
        root_page_num = 0;
        if (pager.num_pages == 0)
        {
            // New file. Initialize page 0 as leaf node.
            LeafNode root_node = pager.get_page(0);
            root_node.initialize_leaf_node();
        }
    }
    ~Table();
    
    friend class Cursor;
    friend class DB;
};

class Cursor
{
private:
    Table *table;
    uint32_t page_num;
    uint32_t cell_num;
    bool end_of_table;

public:
    Cursor(Table *&table, bool option);
    void *cursor_value();
    void cursor_advance();
    void leaf_node_insert(uint32_t key, Row &value);

    friend class DB;
};

void serialize_row(Row &source, void *destination);
void deserialize_row(void *source, Row &destination);
void *row_slot(Table &table, uint32_t row_num);


#endif