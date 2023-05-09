#ifndef _THREAD_POOL
#define _THREAD_POOL
#include <vector>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <functional>
#include <atomic>

class Threadpool {
public:
    Threadpool(int thread_number = 8, int max_requests = 10);
    ~Threadpool();

    template<class Func>
    bool append(Func&& task);

private:
    void worker();

private:
    // thread
    std::mutex m_mtx;
    std::condition_variable m_cond; // Determine if there are any tasks that need to be processed
    int m_thread_number;
    std::vector<std::thread> m_threads;

    // pool
    std::atomic_bool m_is_close{false};

    // request(task)
    int m_max_requests;
    std::queue<std::function<void()>> m_workque;
};


#endif