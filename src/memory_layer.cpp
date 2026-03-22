#include "memory_layer.hpp"

MemoryLayer::MemoryLayer() {}

MemoryLayer &MemoryLayer::instance() {
    static MemoryLayer memory_layer;
    return memory_layer;
}