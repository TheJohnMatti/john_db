#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "data_type.hpp"

struct QueryResult {
    std::vector<std::string> columns;
    std::vector<std::vector<Data>> data;

    void print() {
        for (auto &col : columns) 
            std::cout << col << '\t';
        std::cout << '\n';
        for (auto &row : data) {
            for (auto &entry : row) {
                switch (entry.index()) {
                    case 0:
                        std::cout << '\t';
                        break;
                    case 1:
                        std::cout << std::get<1>(entry) << '\t';
                        break;
                    case 2:
                        std::cout << std::get<2>(entry) << '\t';
                        break;    
                    case 3:
                        std::cout << std::get<3>(entry) << '\t';
                        break;
                    case 4:
                        std::cout << std::get<4>(entry) << '\t';
                        break;
                    default:
                        std::cout << '\t';
                        break;
                }
            }
            std::cout << '\n';
        }
    }

};