#include "query_processor.hpp"
#include <utility>
#include <array>

QueryProcessor::QueryProcessor() {
    const std::array special = {'(', ')', ',', '=', '>', '<'};
    for (auto &i : special) special_characters[i] = 1;
};

Query QueryProcessor::get_tokens(std::string_view query) {
    Query res;
    for (int i=0; i<query.size();) {
        if (query[i] == ' ') {
            i++; 
            continue;
        }
        if (special_characters[query[i]]) {
            res.push_back(query.substr(i++, 1));
            continue;
        }
        int j;
        for (j=i; j<query.size() && query[j] != ' '; j++){}
        res.push_back(query.substr(i, j-i));
        i = j+1;
    }
    return res;
}

void QueryProcessor::process(std::string_view query) {
    Query tokens = get_tokens(query);
    if (!tokens.size()) return;
    std::string_view &starter = tokens[0];
    auto it = FIRST_TO_HANDLER.find((std::string)starter);
    if (it == FIRST_TO_HANDLER.end()) {
        std::cout << "Unknown command: " << starter << std::endl;
        return;
    }
    const HandlerEnum &handler = it->second;
    auto handler_func = [handler](){
        switch (handler) {
            case SELECT: return select_handler;
            case INSERT: return insert_handler;
            case UPDATE: return update_handler;
            case DELETE: return delete_handler;
            case CREATE: return create_handler;
            case ALTER: return alter_handler;
            case DROP: return drop_handler;
            default: return unknown_handler;
        } 
    }();

    (this->*handler_func)(tokens);
}

void QueryProcessor::select_handler(Query& query) {

}
void QueryProcessor::insert_handler(Query& query) {

}
void QueryProcessor::update_handler(Query& query) {

}
void QueryProcessor::delete_handler(Query& query) {

}
void QueryProcessor::create_handler(Query& query) {

}
void QueryProcessor::alter_handler(Query& query) {

}
void QueryProcessor::drop_handler(Query& query) {

}
void QueryProcessor::unknown_handler(Query& query) {

}