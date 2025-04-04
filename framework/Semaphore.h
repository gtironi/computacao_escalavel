#ifndef SEMAPHORE
#define SEMAPHORE

#include <mutex>
#include <condition_variable>

class Semaphore {
private:
    int count;
    int max_count;
    std::mutex mutex;
    std::condition_variable condition;

public:
    explicit Semaphore(int count = 0, int max_count = 1)
        : count(count), max_count(max_count) {}

    virtual ~Semaphore() = default;

    // Equivalent to acquire / wait
    void wait() {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, [&]() { return count > 0; });
        count--;
    }

    // Equivalent to release / notify
    void notify() {
        std::unique_lock<std::mutex> lock(mutex);
        if (count < max_count) {
            count++;
            condition.notify_one();
        }
    }


    int get_count() {
        std::unique_lock<std::mutex> lock(mutex);
        return count;
    }
};

#endif
