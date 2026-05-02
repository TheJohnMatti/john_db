#include "page_io.hpp"
#include <fstream>
#include <filesystem>

const LogicalPage null_page{};

LogicalPage PageIO::read_page(const std::string &table_name, const std::string &page_name) {
    LogicalPage result;
    const std::filesystem::path path =
        std::filesystem::path("tables") / table_name / ("page_" + page_name + ".data");
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) return result;
    file.read((char*)&result, PAGE_SIZE);
    return result;
}

LogicalPage PageIO::read_page(const std::string &table_name, const size_t page_number) {
    return read_page(table_name, std::to_string(page_number));
}

void PageIO::write_page(const std::string &table_name, const std::string &page_name, const LogicalPage &page_data) {
    const std::filesystem::path path =
        std::filesystem::path("tables") / table_name / ("page_" + page_name + ".data");
    std::ofstream file(path, std::ios::out | std::ios::binary);
    file.write((char*)&page_data, PAGE_SIZE);
}

void PageIO::write_page(const std::string &table_name, const size_t page_number, const LogicalPage &page_data) {
    write_page(table_name, std::to_string(page_number), page_data);
}

void PageIO::create_page(const std::string &table_name, const std::string &page_name) {
    write_page(table_name, page_name, null_page);
}

void PageIO::create_page(const std::string &table_name, const size_t page_number) {
    create_page(table_name, std::to_string(page_number));
}
