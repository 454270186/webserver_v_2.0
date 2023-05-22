#include "minheap.h"

MinHeap::MinHeap(int heap_size) {
    heap_.reserve(heap_size);
}

MinHeap::~MinHeap() {
    heap_.clear();
}