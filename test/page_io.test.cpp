#include <algorithm>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <numeric>
#include "page.hpp"
#include "page_io.hpp"

int main() {
    const std::string table_name = "test_table";
    const std::string page_name = "test_page";

    std::filesystem::create_directories(std::filesystem::path("tables") / table_name);

    LogicalPage page_data{};
    std::iota(page_data.data, page_data.data + SLOT_SPACE, static_cast<char>(0));

    PageIO::write_page(table_name, page_name, page_data);

    const LogicalPage read_back = PageIO::read_page(table_name, page_name);
    for (size_t i = 0; i < SLOT_SPACE; ++i) {
        assert(read_back.data[i] == page_data.data[i]);
    }

    std::cout << "page_io.test.cpp: All tests passed\n";
    return 0;
}
