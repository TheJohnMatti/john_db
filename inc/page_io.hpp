#pragma once

#include <iostream>
#include "page.hpp"
#include "table.hpp"

namespace PageIO {
    LogicalPage read_page(const std::string&, const std::string&);
    void write_page(const std::string&, const std::string&, const LogicalPage&);
};