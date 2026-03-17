#include "page_io.hpp"
#include <fstream>
#include <filesystem>

LogicalPage PageIO::read_page(const std::string &table_name, const std::string &page_name) {
    LogicalPage result;
    std::filesystem::path path{".\\tables\\" + table_name + "\\" + page_name};
    std::ifstream file(path, std::ios::in | std::ios::binary);
    file.read((char*)&result, PAGE_SIZE);
    return result;
}

void PageIO::write_page(const std::string &table_name, const std::string &page_name, const LogicalPage &page_data) {
    std::filesystem::path path{".\\tables\\" + table_name + "\\" + page_name};
    std::ofstream file(path, std::ios::out | std::ios::binary);
    file.write((char*)&page_data, PAGE_SIZE);
}