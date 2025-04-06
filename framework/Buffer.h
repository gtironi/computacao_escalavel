#ifndef BUFFER_H
#define BUFFER_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <optional>
#include <iostream>
#include "Semaphore.h"

// Implementando um buffer Thread-Safe
template <typename T>
class Buffer {
private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cond;
    int max_size;
    Semaphore semaphore;
    bool finishedWork = false;
    bool lastOne = false;

public:
    Buffer(int max_size = 10) : max_size(max_size), semaphore(max_size) {}
    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(std::move(value));
        cond.notify_one();
    }
    
    std::optional<T> pop() {
        if (getLastOne())
        {
            return std::nullopt;
        }

        std::unique_lock<std::mutex> lock(mtx);
        cond.wait(lock, [this] { return !queue.empty(); });

        // cond.wait(lock, [this] { return !queue.empty(); });
        T value = std::move(queue.front());
        queue.pop();

        semaphore.notify();

        if (getFinishedWork() && get_semaphore().get_count() == get_max_size())
        {
            setLastOne();
        }

        return value;
    }    

    // bool is_empty() const {
    //     std::lock_guard<std::mutex> lock(mtx);
    //     return queue.empty();
    // }

    // bool is_full() const {
    //     std::lock_guard<std::mutex> lock(mtx);
    //     return (queue.size() == max_size);
    // }

    Semaphore& get_semaphore() {
        return semaphore;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.size();
    }
    int get_max_size() const {return max_size;}

    void setFinishedWork() {
        finishedWork = true;
        cond.notify_all();
    }

    bool getFinishedWork()
    {
        return finishedWork;
    }

    bool getLastOne()
    {
        return lastOne;
    }

    bool atomicGetLastOne()
    {
        std::lock_guard<std::mutex> lock(mtx);
        return lastOne;
    }

    void setLastOne() {
        lastOne = true;
    }
};

#endif // BUFFER_H