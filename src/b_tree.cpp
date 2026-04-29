#include "b_tree.hpp"

#include <utility>

BTree::BTree(const std::string &name, std::string directory_path)
    : tree_name(name), tree_directory(std::move(directory_path)), root_node_id(0) {
    ensure_btree_dir();
    std::filesystem::path root_path = get_node_path(root_node_id);
    if (std::filesystem::exists(root_path)) {
        BTreeNode existing_root = read_node(root_node_id);
    } else {
        BTreeNode root{};
        root.is_leaf = 1;
        root.count = 0;
        write_node(root_node_id, root);
    }
}

void BTree::insert(uint64_t key, uint64_t value) {
    BTreeNode root = read_node(root_node_id);
    if (root.count == BTREE_MAX_KEYS) {
        uint64_t new_root_id = allocate_node_id();
        BTreeNode new_root{};
        new_root.is_leaf = 0;
        new_root.count = 0;
        new_root.children[0] = root_node_id;
        write_node(new_root_id, new_root);
        root_node_id = new_root_id;
        split_child(new_root_id, 0);
        insert_non_full(new_root_id, key, value);
    } else {
        insert_non_full(root_node_id, key, value);
    }
}

uint64_t BTree::search(uint64_t key) const {
    uint64_t node_id = root_node_id;
    while (true) {
        BTreeNode node = read_node(node_id);
        uint32_t i = 0;
        while (i < node.count && key > node.keys[i]) {
            i++;
        }
        if (i < node.count && key == node.keys[i]) {
            return node.children[i];
        }
        if (node.is_leaf) {
            throw std::runtime_error("Key not found in B-tree");
        }
        node_id = node.children[i];
    }
}

bool BTree::contains(uint64_t key) const {
    try {
        search(key);
        return true;
    } catch (...) {
        return false;
    }
}

std::string BTree::get_node_path(uint64_t node_id) const {
    return tree_directory + "/" + tree_name + "/node_" + std::to_string(node_id) + ".data";
}

void BTree::ensure_btree_dir() const {
    std::filesystem::path dir = tree_directory + "/" + tree_name;
    if (!std::filesystem::is_directory(dir)) {
        std::filesystem::create_directories(dir);
    }
}

BTreeNode BTree::read_node(uint64_t node_id) const {
    BTreeNode node{};
    std::filesystem::path path = get_node_path(node_id);
    std::ifstream file(path, std::ios::binary);
    if (file.is_open()) {
        file.read((char*)&node, sizeof(BTreeNode));
        file.close();
    }
    return node;
}

void BTree::write_node(uint64_t node_id, const BTreeNode &node) const {
    std::filesystem::path path = get_node_path(node_id);
    std::ofstream file(path, std::ios::binary);
    file.write((char*)&node, sizeof(BTreeNode));
    file.close();
}

uint64_t BTree::allocate_node_id() const {
    static uint64_t next_id = 0;
    return next_id++;
}

void BTree::split_child(uint64_t parent_id, uint32_t child_index) {
    BTreeNode parent = read_node(parent_id);
    uint64_t left_child_id = parent.children[child_index];
    BTreeNode left_child = read_node(left_child_id);
    uint64_t right_child_id = allocate_node_id();
    BTreeNode right_child{};
    right_child.is_leaf = left_child.is_leaf;
    right_child.count = BTREE_MIN_KEYS;
    std::copy(left_child.keys + BTREE_MIN_KEYS + 1, left_child.keys + left_child.count, right_child.keys);
    if (!left_child.is_leaf) {
        std::copy(left_child.children + BTREE_MIN_KEYS + 1, left_child.children + left_child.count + 1, right_child.children);
    }
    left_child.count = BTREE_MIN_KEYS;
    for (uint32_t i = parent.count; i > child_index; i--) {
        parent.keys[i] = parent.keys[i - 1];
        parent.children[i + 1] = parent.children[i];
    }
    parent.keys[child_index] = left_child.keys[BTREE_MIN_KEYS];
    parent.children[child_index + 1] = right_child_id;
    parent.count++;
    write_node(parent_id, parent);
    write_node(left_child_id, left_child);
    write_node(right_child_id, right_child);
}

void BTree::insert_non_full(uint64_t node_id, uint64_t key, uint64_t value) {
    BTreeNode node = read_node(node_id);
    uint32_t i = node.count;
    if (node.is_leaf) {
        while (i > 0 && node.keys[i - 1] > key) {
            node.keys[i] = node.keys[i - 1];
            node.children[i] = node.children[i - 1];
            i--;
        }
        node.keys[i] = key;
        node.children[i] = value;
        node.count++;
        write_node(node_id, node);
    } else {
        while (i > 0 && node.keys[i - 1] > key) {
            i--;
        }
        uint64_t child_id = node.children[i];
        BTreeNode child = read_node(child_id);
        if (child.count == BTREE_MAX_KEYS) {
            split_child(node_id, i);
            node = read_node(node_id);
            if (key > node.keys[i]) {
                i++;
            }
        }
        insert_non_full(node.children[i], key, value);
    }
}
