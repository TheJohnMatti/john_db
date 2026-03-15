#include <iostream>
#include "query_result.hpp"
#include "table.hpp"
#include "predicate.hpp"
#include <vector>
#include <optional>

class MemoryLayer {
    
    public:
    static MemoryLayer &instance();

    private:
    MemoryLayer();

    template<typename T = nullptr_t>
    QueryResult select_query(Table& table, std::vector<std::string>& columns, std::optional<Predicate<T>> predicate) {
        
    }

};
