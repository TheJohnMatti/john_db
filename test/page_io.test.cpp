#include <iostream>
#include <cassert>
#include <algorithm>
#include <numeric>
#include "page_io.hpp"
#include "page.hpp"

int main() {

    RawPage page_data;
    std::iota(page_data.data, page_data.data+PAGE_SIZE, 0);

    std::string table_name = "test_table", page_name = "test_page";

    PageIO::write_page(table_name, page_name, page_data);

    return 0;
}