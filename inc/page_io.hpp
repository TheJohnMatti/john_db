#pragma once

#include <iostream>
#include "page.hpp"
#include "table.hpp"

class PageIO {
    public:
    static LogicalPage read_page(const std::string&, const std::string&);
    static LogicalPage read_page(const std::string&, const size_t);
    static void write_page(const std::string&, const std::string&, const LogicalPage&);
    static void write_page(const std::string&, const size_t, const LogicalPage&);
    static void create_page(const std::string&, const std::string&);
    static void create_page(const std::string&, const size_t);
    static void set_data_dir(std::string dir);
    private:
    static std::string data_dir;
};