#ifndef BUFFER_H
#define BUFFER_H

#include <mutex>
#include <condition_variable>
#include <queue>
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

public:
    Buffer(int max_size = 10) : max_size(max_size), semaphore(max_size) {}
    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(std::move(value));
        cond.notify_one();
    }
    
    T pop() {
        std::unique_lock<std::mutex> lock(mtx);
        // Bloqueia até que tenha algo na fila mas não impede de colocar elementos
        cond.wait(lock, [this] { return !queue.empty(); });
        T value = std::move(queue.front());
        queue.pop();
        semaphore.notify();

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
};

#endif // BUFFER_H