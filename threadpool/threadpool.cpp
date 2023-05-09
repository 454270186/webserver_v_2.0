#include "threadpool.h"
#include <exception>
using std::exception;
using std::thread;
using std::function;
using std::unique_lock;

Threadpool::Threadpool(int thread_number, int max_requests) 
: m_thread_number(thread_number), m_max_requests(max_requests) {
    if (thread_number <= 0 || max_requests <= 0) {
        throw exception();
    }

    for (int i = 0; i < m_thread_number; i++) {
        printf("create %d thread\n", i);
        m_threads.emplace_back(&Threadpool::worker, this);
        m_threads[i].detach();
    }
}

Threadpool::~Threadpool() {
    m_is_close.store(true);
    m_cond.notify_all(); // notify all work thread
}

void Threadpool::worker() {
    while (!m_is_close.load()) {
        function<void()> task;
        {
            unique_lock<std::mutex> lock(m_mtx);
            
            // wait for new task
            m_cond.wait(lock, [this]() { return !this->m_workque.empty() || this->m_is_close.load(); });

            if (m_is_close.load() && m_workque.empty()) {
                return;
            }

            task = std::move(m_workque.front());
            m_workque.pop();
        }

        task();
    }
}

template<class Func>
bool Threadpool::append(Func&& task) {
    if (m_is_close.load()) {
        return false;
    }

    {
        unique_lock<std::mutex> lock(m_mtx);
        if (m_workque.size() >= m_max_requests) {
            return false;
        }

        m_workque.emplace(std::forward<Func>(task));
    }

    m_cond.notify_one();
    return true;
}