#pragma once

#include <iostream>

const int PAGE_SIZE = 4096;

struct RawPage {
    char data[PAGE_SIZE];
};