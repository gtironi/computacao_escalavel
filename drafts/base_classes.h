#ifndef BASE_CLASSES_H
#define BASE_CLASSES_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>
#include <memory>  // Added for smart pointers

// Removed "using namespace std" - better practice for headers

// Thread-safe Buffer
template <typename T>
class Buffer {
private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cond;

public:
    void push(T value) {
        std::unique_lock<std::mutex> lock(mtx);
        queue.push(std::move(value));
        cond.notify_one();
    }
    
    T pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cond.wait(lock, [this] { return !queue.empty(); });
        T value = std::move(queue.front());
        queue.pop();
        return value;
    }    

    // Additional useful methods
    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.size();
    }
};

// Base class for pipeline stages
class Stage {
public:
    virtual void run() = 0;
    virtual ~Stage() = default;
};

class Extractor : public Stage {
protected:
    Buffer<std::string> output_buffer;

public:
    Buffer<std::string>& get_output_buffer() { return output_buffer; }
    virtual void run() override = 0;
};

class Transformer : public Stage {
protected:
    Buffer<std::string>& input_buffer;
    Buffer<std::string> output_buffer;

public:
    explicit Transformer(Buffer<std::string>& in) : input_buffer(in) {}
    Buffer<std::string>& get_output_buffer() { return output_buffer; }
    virtual void run() override = 0;
};

class Loader : public Stage {
protected:
    Buffer<std::string>& input_buffer;

public:
    explicit Loader(Buffer<std::string>& buffer) : input_buffer(buffer) {}
    virtual void run() override = 0;
};

#endif // BASE_CLASSES_H