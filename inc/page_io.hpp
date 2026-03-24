#pragma once

#include <iostream>
#include "page.hpp"
#include "table.hpp"

namespace PageIO {
    LogicalPage read_page(const std::string&, const std::string&);
    LogicalPage read_page(const std::string&, const size_t);
    void write_page(const std::string&, const std::string&, const LogicalPage&);
    void write_page(const std::string&, const size_t, const LogicalPage&);
    void create_page(const std::string&, const std::string&);
    void create_page(const std::string&, const size_t);
};