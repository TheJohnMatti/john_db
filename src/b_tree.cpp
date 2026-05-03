#include "b_tree.hpp"

#include <utility>
#include <unordered_set>

namespace {

uint64_t infer_root_node_id(const std::filesystem::path &node_dir) {
    std::unordered_set<uint64_t> node_ids;
    std::unordered_set<uint64_t> referenced_ids;
    std::filesystem::directory_iterator it(node_dir);
    for (const auto &entry : it) {
        if (!entry.is_regular_file()) continue;
        const std::string filename = entry.path().filename().string();
        const std::string prefix = "node_";
        const std::string suffix = ".data";
        if (filename.rfind(prefix, 0) != 0 || filename.size() <= prefix.size() + suffix.size()) continue;
        const std::string id_str = filename.substr(prefix.size(), filename.size() - prefix.size() - suffix.size());
        try {
            const uint64_t node_id = std::stoull(id_str);
            node_ids.insert(node_id);
        } catch (...) {
            continue;
        }
    }
    if (node_ids.empty()) {
        return 0;
    }
    for (uint64_t node_id : node_ids) {
        const std::filesystem::path path = node_dir / ("node_" + std::to_string(node_id) + ".data");
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) continue;
        BTreeNode node{};
        file.read(reinterpret_cast<char *>(&node), sizeof(BTreeNode));
        if (!node.is_leaf) {
            for (uint32_t i = 0; i <= node.count; ++i) {
                referenced_ids.insert(node.children[i]);
            }
        } else {
            const uint64_t next_id = node.children[BTREE_MAX_KEYS];
            if (next_id != 0) {
                referenced_ids.insert(next_id);
            }
        }
    }
    uint64_t root_candidate = 0;
    bool found_root = false;
    for (uint64_t node_id : node_ids) {
        if (!referenced_ids.count(node_id)) {
            if (!found_root || node_id < root_candidate) {
                root_candidate = node_id;
                found_root = true;
            }
        }
    }
    return found_root ? root_candidate : 0;
}

uint64_t infer_next_node_id(const std::filesystem::path &node_dir) {
    uint64_t max_id = 0;
    bool any = false;
    for (const auto &entry : std::filesystem::directory_iterator(node_dir)) {
        if (!entry.is_regular_file()) continue;
        const std::string filename = entry.path().filename().string();
        const std::string prefix = "node_";
        const std::string suffix = ".data";
        if (filename.rfind(prefix, 0) != 0 || filename.size() <= prefix.size() + suffix.size()) continue;
        const std::string id_str = filename.substr(prefix.size(), filename.size() - prefix.size() - suffix.size());
        try {
            const uint64_t node_id = std::stoull(id_str);
            max_id = std::max(max_id, node_id);
            any = true;
        } catch (...) {
            continue;
        }
    }
    return any ? max_id + 1 : 1;
}

} // namespace

uint32_t BTree::internal_child_index(const BTreeNode &node, uint64_t key) {
    uint32_t i = 0;
    while (i < node.count && key >= node.keys[i]) {
        ++i;
    }
    return i;
}

uint64_t BTree::leaf_next(const BTreeNode &node) {
    return node.children[BTREE_MAX_KEYS];
}

void BTree::set_leaf_next(BTreeNode &node, uint64_t next_id) {
    node.children[BTREE_MAX_KEYS] = next_id;
}

BTree::BTree(const std::string &name, std::string directory_path, bool skip_inference)
    : tree_name(name), tree_directory(std::move(directory_path)), root_node_id(0), next_node_id(1) {
    ensure_btree_dir();
    const std::filesystem::path metadata_path = get_metadata_path();
    if (std::filesystem::exists(metadata_path)) {
        read_metadata();
    } else if (skip_inference) {
        // For new tables, just create a fresh BTree without inference
        BTreeNode root{};
        root.is_leaf = 1;
        root.count = 0;
        set_leaf_next(root, 0);
        write_node(root_node_id, root);
        next_node_id = 1;
        write_metadata();
    } else {
        const std::filesystem::path node_dir = std::filesystem::path(tree_directory) / tree_name;
        const std::filesystem::path root_path = get_node_path(root_node_id);
        if (std::filesystem::exists(root_path)) {
            root_node_id = infer_root_node_id(node_dir);
            next_node_id = infer_next_node_id(node_dir);
            write_metadata();
        } else {
            BTreeNode root{};
            root.is_leaf = 1;
            root.count = 0;
            set_leaf_next(root, 0);
            write_node(root_node_id, root);
            next_node_id = 1;
            write_metadata();
        }
    }
}

void BTree::insert(uint64_t key, uint64_t value) {
    BTreeNode root = read_node(root_node_id);
    if (root.count == BTREE_MAX_KEYS) {
        const uint64_t new_root_id = allocate_node_id();
        BTreeNode new_root{};
        new_root.is_leaf = 0;
        new_root.count = 0;
        new_root.children[0] = root_node_id;
        write_node(new_root_id, new_root);
        root_node_id = new_root_id;
        write_metadata();
        split_child(new_root_id, 0);
        insert_non_full(new_root_id, key, value);
    } else {
        insert_non_full(root_node_id, key, value);
    }
}

uint64_t BTree::search(uint64_t key) const {
    uint64_t node_id = root_node_id;
    for (;;) {
        const BTreeNode node = read_node(node_id);
        if (node.is_leaf) {
            uint32_t i = 0;
            while (i < node.count && key > node.keys[i]) {
                ++i;
            }
            if (i < node.count && node.keys[i] == key) {
                return node.children[i];
            }
            throw std::runtime_error("Key not found in B-tree");
        }
        node_id = node.children[internal_child_index(node, key)];
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

bool BTree::remove(uint64_t key) {
    return remove_from_node(root_node_id, key);
}

std::filesystem::path BTree::get_node_path(uint64_t node_id) const {
    return std::filesystem::path(tree_directory) / tree_name
           / ("node_" + std::to_string(node_id) + ".data");
}

std::filesystem::path BTree::get_metadata_path() const {
    return std::filesystem::path(tree_directory) / tree_name / "metadata.data";
}

void BTree::ensure_btree_dir() const {
    const std::filesystem::path dir = std::filesystem::path(tree_directory) / tree_name;
    if (!std::filesystem::is_directory(dir)) {
        std::filesystem::create_directories(dir);
    }
}

void BTree::read_metadata() {
    const std::filesystem::path metadata_path = get_metadata_path();
    std::ifstream file(metadata_path);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to read B-tree metadata: " + metadata_path.string());
    }
    file >> root_node_id >> next_node_id;
    if (!file || next_node_id == 0) {
        throw std::runtime_error("Invalid B-tree metadata: " + metadata_path.string());
    }
}

void BTree::write_metadata() const {
    const std::filesystem::path metadata_path = get_metadata_path();
    std::ofstream file(metadata_path);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to write B-tree metadata: " + metadata_path.string());
    }
    file << root_node_id << '\n';
    file << next_node_id << '\n';
}

BTreeNode BTree::read_node(uint64_t node_id) const {
    BTreeNode node{};
    const std::filesystem::path path = get_node_path(node_id);
    std::ifstream file(path, std::ios::binary);
    if (file.is_open()) {
        file.read(reinterpret_cast<char *>(&node), sizeof(BTreeNode));
        file.close();
    }
    return node;
}

void BTree::write_node(uint64_t node_id, const BTreeNode &node) const {
    const std::filesystem::path path = get_node_path(node_id);
    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char *>(&node), sizeof(BTreeNode));
    file.close();
}

uint64_t BTree::allocate_node_id() {
    const uint64_t node_id = next_node_id++;
    write_metadata();
    return node_id;
}

void BTree::split_child(uint64_t parent_id, uint32_t child_index) {
    BTreeNode parent = read_node(parent_id);
    const uint64_t left_child_id = parent.children[child_index];
    BTreeNode left_child = read_node(left_child_id);
    const uint64_t right_child_id = allocate_node_id();
    BTreeNode right_child{};
    right_child.is_leaf = left_child.is_leaf;

    if (left_child.is_leaf) {
        right_child.count = BTREE_MAX_KEYS - BTREE_MIN_KEYS;
        std::copy(left_child.keys + BTREE_MIN_KEYS, left_child.keys + BTREE_MAX_KEYS, right_child.keys);
        std::copy(left_child.children + BTREE_MIN_KEYS, left_child.children + BTREE_MAX_KEYS,
                  right_child.children);
        set_leaf_next(right_child, leaf_next(left_child));
        set_leaf_next(left_child, right_child_id);

        parent.keys[child_index] = left_child.keys[BTREE_MIN_KEYS];
        left_child.count = BTREE_MIN_KEYS;
    } else {
        right_child.count = BTREE_MIN_KEYS;
        std::copy(left_child.keys + BTREE_MIN_KEYS + 1, left_child.keys + left_child.count, right_child.keys);
        std::copy(left_child.children + BTREE_MIN_KEYS + 1, left_child.children + left_child.count + 1,
                  right_child.children);
        set_leaf_next(right_child, 0);

        left_child.count = BTREE_MIN_KEYS;
        parent.keys[child_index] = left_child.keys[BTREE_MIN_KEYS];
    }

    for (uint32_t i = parent.count; i > child_index; i--) {
        parent.keys[i] = parent.keys[i - 1];
        parent.children[i + 1] = parent.children[i];
    }
    parent.children[child_index + 1] = right_child_id;
    parent.count++;

    write_node(parent_id, parent);
    write_node(left_child_id, left_child);
    write_node(right_child_id, right_child);
}

void BTree::insert_non_full(uint64_t node_id, uint64_t key, uint64_t value) {
    BTreeNode node = read_node(node_id);
    if (node.is_leaf) {
        uint32_t i = node.count;
        while (i > 0 && node.keys[i - 1] > key) {
            node.keys[i] = node.keys[i - 1];
            node.children[i] = node.children[i - 1];
            i--;
        }
        node.keys[i] = key;
        node.children[i] = value;
        node.count++;
        write_node(node_id, node);
        return;
    }

    uint32_t i = internal_child_index(node, key);
    uint64_t child_id = node.children[i];
    BTreeNode child = read_node(child_id);
    if (child.count == BTREE_MAX_KEYS) {
        split_child(node_id, i);
        node = read_node(node_id);
        if (i < node.count && key >= node.keys[i]) {
            ++i;
        }
        child_id = node.children[i];
    }
    insert_non_full(child_id, key, value);
}

bool BTree::remove_from_node(uint64_t node_id, uint64_t key) {
    BTreeNode node = read_node(node_id);
    if (node.is_leaf) {
        uint32_t i = 0;
        while (i < node.count && key > node.keys[i]) {
            ++i;
        }
        if (i < node.count && node.keys[i] == key) {
            for (uint32_t j = i + 1; j < node.count; j++) {
                node.keys[j - 1] = node.keys[j];
                node.children[j - 1] = node.children[j];
            }
            node.count--;
            write_node(node_id, node);
            return true;
        }
        return false;
    }

    const uint32_t i = internal_child_index(node, key);
    return remove_from_node(node.children[i], key);
}
