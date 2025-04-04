#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "BaseClasses.h"
#include "Manager.h"

// Example derived class that squares numbers
class SquareExtractor : public Extractor<SquareExtractor, int> {
public:
    int run(int x) {
        // Simple implementation - squares the input
        return x * x;
    }
};

int main() {
    // 1. Create an instance of the derived class
    SquareExtractor extractor;
    
    // 2. Create and set up a TaskQueue (assuming you have this class implemented)
    TaskQueue taskqueue = TaskQueue(); // 4 worker threads
    extractor.set_taskqueue(&taskqueue);
    
    // 3. Test synchronous operation (direct run calls)
    std::cout << "Testing direct run() calls:\n";
    std::cout << "5 squared: " << extractor.run(5) << std::endl;
    std::cout << "7 squared: " << extractor.run(7) << std::endl;
    
    // 4. Test create_task (single task)
    std::cout << "\nTesting create_task():\n";
    extractor.create_task(3);
    auto result = extractor.get_output_buffer().pop();
    std::cout << "3 squared: " << result << std::endl;
    
    // 5. Test add_task_thread (multiple tasks)
    std::cout << "\nTesting add_task_thread():\n";
    
    // Start adding tasks in a separate thread
    std::thread producer([&extractor]() {
        for (int i = 1; i <= 5; ++i) {
            extractor.add_task_thread(i);
        }
    });
    
    // Consumer thread to read results
    std::thread consumer([&extractor]() {
        for (int i = 1; i <= 5; ++i) {
            int result = extractor.get_output_buffer().pop();
            std::cout << "Received result: " << result << std::endl;
            extractor.get_output_buffer().get_semaphore();
        }
    });
    
    producer.join();
    consumer.join();
    
    // 6. Cleanup
    taskqueue.shutdown();
    
    return 0;
}