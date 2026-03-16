#pragma once

#include <iostream>
#include "page.hpp"
#include "table.hpp"

namespace PageIO {
    RawPage read_page(std::string&, std::string&);
    void write_page(std::string&, std::string&, RawPage);
};