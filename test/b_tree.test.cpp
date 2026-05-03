#include "b_tree.hpp"
#include "page_io.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

std::filesystem::path make_unique_test_root() {
    return "test_data_b_tree";
}

void cleanup(const std::filesystem::path &root) {
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
}

std::string btree_dir(const std::filesystem::path &root, const char *table_subdir) {
    return (root / table_subdir).string();
}

void assert_true(bool cond, const char *msg) {
    if (!cond) {
        std::cerr << "FAIL: " << msg << std::endl;
        std::abort();
    }
}

void test_empty_search_throws() {
    const std::filesystem::path root = make_unique_test_root();
    std::filesystem::create_directories(root / "t");
    const std::string dir = btree_dir(root, "t");
    BTree tree("pk", dir);
    bool threw = false;
    try {
        (void)tree.search(0);
    } catch (const std::runtime_error &) {
        threw = true;
    }
    assert_true(threw, "search on empty tree should throw");
    assert_true(!tree.contains(0), "contains false on empty");
    cleanup(root);
}

void test_single_insert_search_contains() {
    const std::filesystem::path root = make_unique_test_root();
    std::filesystem::create_directories(root / "t");
    BTree tree("pk", btree_dir(root, "t"));
    tree.insert(42, 9001);
    assert_true(tree.contains(42), "contains after insert");
    assert_true(tree.search(42) == 9001, "search value");
    cleanup(root);
}

void test_sequential_keys_values() {
    const std::filesystem::path root = make_unique_test_root();
    std::filesystem::create_directories(root / "t");
    BTree tree("pk", btree_dir(root, "t"));
    constexpr int n = 300;
    for (int i = 0; i < n; ++i) {
        const uint64_t key = static_cast<uint64_t>(i);
        const uint64_t val = static_cast<uint64_t>(i) * 100000u + 3u;
        tree.insert(key, val);
    }
    for (int i = 0; i < n; ++i) {
        const uint64_t key = static_cast<uint64_t>(i);
        const uint64_t want = static_cast<uint64_t>(i) * 100000u + 3u;
        assert_true(tree.contains(key), "contains after many inserts");
        assert_true(tree.search(key) == want, "search after many inserts");
    }
    cleanup(root);
}

void test_stress_many_splits() {
    const std::filesystem::path root = make_unique_test_root();
    std::filesystem::create_directories(root / "t");
    BTree tree("pk", btree_dir(root, "t"));
    constexpr int n = 1200;
    for (int i = 0; i < n; ++i) {
        tree.insert(static_cast<uint64_t>(i), static_cast<uint64_t>(i) ^ 0x9E3779B97F4A7C15ULL);
    }
    for (int i = 0; i < n; ++i) {
        const uint64_t want = static_cast<uint64_t>(i) ^ 0x9E3779B97F4A7C15ULL;
        assert_true(tree.search(static_cast<uint64_t>(i)) == want, "stress search");
    }
    cleanup(root);
}

void test_remove_and_search() {
    const std::filesystem::path root = make_unique_test_root();
    std::filesystem::create_directories(root / "t");
    BTree tree("pk", btree_dir(root, "t"));
    tree.insert(1, 100);
    tree.insert(2, 200);
    tree.insert(5, 500);
    assert_true(tree.remove(2), "remove existing");
    assert_true(!tree.contains(2), "removed key not contains");
    assert_true(tree.search(1) == 100, "neighbor left");
    assert_true(tree.search(5) == 500, "neighbor right");
    bool threw = false;
    try {
        (void)tree.search(2);
    } catch (const std::runtime_error &) {
        threw = true;
    }
    assert_true(threw, "search removed throws");
    assert_true(!tree.remove(99), "remove missing returns false");
    cleanup(root);
}

void test_remove_all_then_empty() {
    const std::filesystem::path root = make_unique_test_root();
    std::filesystem::create_directories(root / "t");
    BTree tree("pk", btree_dir(root, "t"));
    for (int i = 0; i < 50; ++i) {
        tree.insert(static_cast<uint64_t>(i), 1);
    }
    for (int i = 0; i < 50; ++i) {
        assert_true(tree.remove(static_cast<uint64_t>(i)), "remove each");
    }
    for (int i = 0; i < 50; ++i) {
        assert_true(!tree.contains(static_cast<uint64_t>(i)), "all gone");
    }
    cleanup(root);
}

void test_sparse_keys() {
    const std::filesystem::path root = make_unique_test_root();
    std::filesystem::create_directories(root / "t");
    BTree tree("pk", btree_dir(root, "t"));
    const uint64_t keys[] = {1, 1000000, 3, 999999, 2};
    const uint64_t vals[] = {10, 20, 30, 40, 50};
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
        tree.insert(keys[i], vals[i]);
    }
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
        assert_true(tree.search(keys[i]) == vals[i], "sparse key search");
    }
    cleanup(root);
}

void test_boundary_equals_separator() {
    const std::filesystem::path root = make_unique_test_root();
    std::filesystem::create_directories(root / "t");
    BTree tree("pk", btree_dir(root, "t"));
    tree.insert(10, 1);
    tree.insert(20, 2);
    tree.insert(15, 3);
    assert_true(tree.search(10) == 1, "boundary 10");
    assert_true(tree.search(15) == 3, "boundary 15");
    assert_true(tree.search(20) == 2, "boundary 20");
    cleanup(root);
}

// Verify BTree root and allocation metadata persist when reopening after splits.
void test_reopen_same_directory_split_tree() {
    const std::filesystem::path root = make_unique_test_root();
    std::filesystem::create_directories(root / "t");
    const std::string dir = btree_dir(root, "t");
    {
        BTree tree("pk", dir);
        for (int i = 0; i < 1200; ++i) {
            tree.insert(static_cast<uint64_t>(i), static_cast<uint64_t>(i) + 1000u);
        }
    }
    {
        BTree tree2("pk", dir);
        for (int i = 0; i < 1200; ++i) {
            assert_true(tree2.contains(static_cast<uint64_t>(i)), "reopen contains");
            assert_true(tree2.search(static_cast<uint64_t>(i)) == static_cast<uint64_t>(i) + 1000u,
                        "reopen search");
        }
    }
    cleanup(root);
}

} // namespace

int main() {
    std::cout << "b_tree.test.cpp\n";
    std::filesystem::remove_all("test_data_b_tree");
    PageIO::set_data_dir("test_data_b_tree");
    test_empty_search_throws();
    test_single_insert_search_contains();
    test_sequential_keys_values();
    test_stress_many_splits();
    test_remove_and_search();
    test_remove_all_then_empty();
    test_sparse_keys();
    test_boundary_equals_separator();
    test_reopen_same_directory_split_tree();
    std::cout << "All B+ tree tests passed.\n";
    return 0;
}
