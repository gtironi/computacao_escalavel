#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <unordered_map>
#include <future>
#include "base_classes.h"

using namespace std;

class PipelineManager {
    private:
        vector<unique_ptr<Stage>> stages;
        vector<thread> threads;       
    public:
        template <typename T, typename... Args>
        T* add_stage(Args&&... args) {n
            stages.push_back(make_unique<T>(forward<Args>(args)...));
            return static_cast<T*>(stages.back().get());
        }
        
        void run() {
            for (auto& stage : stages) {
                threads.emplace_back(&Stage::run, stage.get());
            }
            for (auto& t : threads) {
                t.join();
            }
        }
    };
    
// Implementação para testar
class MyExtractor : public Extractor {
public:
    void run() override {
        for (int i = 0; i < 5; ++i) {
            std::cout << "Extractor" << i << std::endl;
            output_buffer.push("Data " + to_string(i));
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
};

class MyTransformer : public Transformer {
public:
    MyTransformer(Buffer<string>& in) : Transformer(in) {}
    void run() override {
        for (int i = 0; i < 5; ++i) {
            std::cout << "Transformer" << i << std::endl;
            string data = input_buffer.pop();
            output_buffer.push("Transformed " + data);
        }
    }
};

class MyLoader : public Loader {
public:
    MyLoader(Buffer<string>& buffer) : Loader(buffer) {}
    void run() override {
        for (int i = 0; i < 5; ++i) {
            cout << "Loaded: " << input_buffer.pop() << endl;
        }
    }
};

int main() {
    PipelineManager manager;

    auto extractor = manager.add_stage<MyExtractor>("ashjdlksj.csv");
    auto transformer = manager.add_stage<MyTransformer>(extractor->get_output_buffer());
    auto loader = manager.add_stage<MyLoader>(transformer->get_output_buffer());
    
    manager.run();     
    return 0;
}