#include "page_io.hpp"
#include <fstream>
#include <filesystem>

RawPage PageIO::read_page(std::string &table_name, std::string &page_name) {
    RawPage result;
    std::filesystem::path path{".\\tables\\" + table_name + "\\" + page_name};
    std::ifstream file(path, std::ios::in | std::ios::binary);
    file.read(result.data, PAGE_SIZE);
    return result;
}

void PageIO::write_page(std::string &table_name, std::string &page_name, RawPage page_data) {
    std::filesystem::path path{".\\tables\\" + table_name + "\\" + page_name};
    std::ofstream file(path, std::ios::out | std::ios::binary);
    file.write(page_data.data, PAGE_SIZE);
}