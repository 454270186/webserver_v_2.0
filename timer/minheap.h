#ifndef _MIN_HEAP
#define _MIN_HEAP
#include <vector>
#include <chrono>
#include <functional>

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::milliseconds MS;
typedef std::chrono::high_resolution_clock::time_point TimeStamp;

using std::vector;


/*
    i节点的父节点下标为 (i-1)/2
    i节点的左右子节点下标分别为 (2*i + 1) 和 (2*i + 2)
*/

struct TimeNode {
    int id;
    TimeStamp exp;
    TimeoutCallBack callback_func;

    bool operator<(const TimeNode& t) {
        return exp < t.exp;
    }
};


class MinHeap {
public:
    MinHeap(int heap_size = 64);
    ~MinHeap();
private:
    vector<TimeNode> heap_{};
};

#endif