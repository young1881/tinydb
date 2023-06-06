#include "table.h"

Pager::Pager(const char *filename)
{
    file_descriptor = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
    if (file_descriptor < 0)
    {
        std::cerr << "Error: cannot open file " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    file_length = lseek(file_descriptor, 0, SEEK_END);
    num_pages = file_length / PAGE_SIZE;

    if (file_length % PAGE_SIZE != 0)
    {
        std::cerr << "Db file is not a whole number of pages. Corrupt file." << std::endl;
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        pages[i] = nullptr;
    }
}

void *Pager::get_page(uint32_t page_num)
{
    if (page_num > TABLE_MAX_PAGES)
    {
        std::cout << "Tried to fetch page number out of bounds." << page_num << " > "
                  << TABLE_MAX_PAGES << std::endl;
        exit(EXIT_FAILURE);
    }

    if (pages[page_num] == nullptr)
    {
        // Cache miss. Allocate memory and load from file.
        void *page = malloc(PAGE_SIZE);
        uint32_t num_pages = file_length / PAGE_SIZE;

        // We might save a partial page at the end of the file
        if (file_length % PAGE_SIZE)
        {
            num_pages += 1;
        }

        if (page_num <= num_pages)
        {
            lseek(file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
            ssize_t bytes_read = read(file_descriptor, page, PAGE_SIZE);
            if (bytes_read == -1)
            {
                std::cout << "Error reading file: " << errno << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        pages[page_num] = page;

        if (page_num >= num_pages)
        {
            this->num_pages = page_num + 1;
        }
    }

    return pages[page_num];
}

void Pager::pager_flush(uint32_t page_num)
{
    if (pages[page_num] == nullptr)
    {
        std::cout << "Tried to flush null page" << std::endl;
        exit(EXIT_FAILURE);
    }

    off_t offset = lseek(file_descriptor, page_num * PAGE_SIZE, SEEK_SET);

    if (offset == -1)
    {
        std::cout << "Error seeking: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_written =
        write(file_descriptor, pages[page_num], PAGE_SIZE);

    if (bytes_written == -1)
    {
        std::cout << "Error writing: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }
}

Table::~Table()
{
    for (uint32_t i = 0; i < pager.num_pages; i++)
    {
        if (pager.pages[i] == nullptr)
        {
            continue;
        }
        pager.pager_flush(i);
        free(pager.pages[i]);
        pager.pages[i] = nullptr;
    }

    int result = close(pager.file_descriptor);
    if (result == -1)
    {
        std::cout << "Error closing db file." << std::endl;
        exit(EXIT_FAILURE);
    }
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        void *page = pager.pages[i];
        if (page)
        {
            free(page);
            pager.pages[i] = nullptr;
        }
    }
}

Cursor::Cursor(Table *&table, bool option)
{
    this->table = table;
    page_num = table->root_page_num;
    LeafNode root_node = table->pager.get_page(table->root_page_num);
    uint32_t num_cells = *root_node.leaf_node_num_cells();
    if (option)
    {
        // start at the beginning of table;
        cell_num = 0;
        end_of_table = (num_cells == 0);
    }
    else
    {
        // end of the table
        cell_num = num_cells;
        end_of_table = true;
    }
}

void *Cursor::cursor_value()
{
    void *page = table->pager.get_page(page_num);
    return LeafNode(page).leaf_node_value(cell_num);
}

void Cursor::cursor_advance()
{
    LeafNode leaf_node = table->pager.get_page(page_num);
    cell_num += 1;
    if (cell_num >= *leaf_node.leaf_node_num_cells())
    {
        end_of_table = true;
    }
}

void Cursor::leaf_node_insert(uint32_t key, Row &value)
{
    LeafNode leaf_node = table->pager.get_page(page_num);
    uint32_t num_cells = *leaf_node.leaf_node_num_cells();

    if (num_cells >= LEAF_NODE_MAX_CELLS)
    {
        // Node full
        std::cout << "Need to implement splitting a leaf node." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (cell_num < num_cells)
    {
        // make room for new cell
        for (uint32_t i = num_cells; i > cell_num; i--)
        {
            memcpy(leaf_node.leaf_node_cell(i), leaf_node.leaf_node_cell(i - 1),
                   LEAF_NODE_CELL_SIZE);
        }
    }

    // insert new cell
    *(leaf_node.leaf_node_num_cells()) += 1;
    *(leaf_node.leaf_node_key(cell_num)) = key;
    serialize_row(value, leaf_node.leaf_node_value(cell_num));
}

void serialize_row(Row &source, void *destination)
{
    memcpy((char *)destination + ID_OFFSET, &(source.id), ID_SIZE);
    memcpy((char *)destination + USERNAME_OFFSET, &(source.username), USERNAME_SIZE);
    memcpy((char *)destination + EMAIL_OFFSET, &(source.email), EMAIL_SIZE);
}

void deserialize_row(void *source, Row &destination)
{
    memcpy(&(destination.id), (char *)source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination.username), (char *)source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination.email), (char *)source + EMAIL_OFFSET, EMAIL_SIZE);
}

// void *Table::row_slot(uint32_t row_num)
// {
//     uint32_t page_num = row_num / ROWS_PER_PAGE;
//     void *page = pager.get_page(page_num);
//     uint32_t row_offset = row_num % ROWS_PER_PAGE;
//     uint32_t byte_offset = row_offset * ROW_SIZE;
//     return (char *)page + byte_offset;
// }

// void *row_slot(Table &table, uint32_t row_num)
// {
//     uint32_t page_num = row_num / ROWS_PER_PAGE;
//     void *page = table.pages[page_num];
//     if (page == NULL)
//     {
//         // Allocate memory only when we try to access page
//         page = table.pages[page_num] = malloc(PAGE_SIZE);
//     }
//     uint32_t row_offset = row_num % ROWS_PER_PAGE;
//     uint32_t byte_offset = row_offset * ROW_SIZE;
//     return (char *)page + byte_offset;
// }
