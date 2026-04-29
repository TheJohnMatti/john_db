
#pragma once

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <string>

constexpr size_t BTREE_PAGE_SIZE = 4096;
constexpr size_t BTREE_MAX_KEYS = 127;
constexpr size_t BTREE_MIN_KEYS = 63;

struct BTreeNode {
    uint32_t count;
    uint8_t is_leaf;
    uint8_t _padding[3];
    uint64_t keys[BTREE_MAX_KEYS];
    uint64_t children[BTREE_MAX_KEYS + 1];
};

static_assert(sizeof(BTreeNode) <= BTREE_PAGE_SIZE, "BTreeNode too large for page");

class BTree {
private:
    std::string tree_name;
    std::string tree_directory;
    uint64_t root_node_id;
    std::string get_node_path(uint64_t node_id) const;
    void ensure_btree_dir() const;
    BTreeNode read_node(uint64_t node_id) const;
    void write_node(uint64_t node_id, const BTreeNode &node) const;
    uint64_t allocate_node_id() const;
    void split_child(uint64_t parent_id, uint32_t child_index);
    void insert_non_full(uint64_t node_id, uint64_t key, uint64_t value);
    bool remove_from_node(uint64_t node_id, uint64_t key);
    uint64_t remove_max(uint64_t node_id);
public:
    BTree(const std::string &name, std::string directory_path);
    void insert(uint64_t key, uint64_t value);
    uint64_t search(uint64_t key) const;
    bool contains(uint64_t key) const;
    bool remove(uint64_t key);
};
