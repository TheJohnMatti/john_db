#pragma once

#include <iostream>
#include <bitset>
#include <cstdint>
#include <vector>

constexpr size_t PAGE_SIZE = 4096;
constexpr size_t MAX_OCCUPANCY = 256;
constexpr size_t SLOT_SPACE = PAGE_SIZE - sizeof(std::bitset<MAX_OCCUPANCY>);
constexpr size_t MIN_ROW_SIZE = (SLOT_SPACE - 1 + MAX_OCCUPANCY) / MAX_OCCUPANCY;

struct LogicalPage {
    std::bitset<MAX_OCCUPANCY> occupancy;
    char data[SLOT_SPACE];
};