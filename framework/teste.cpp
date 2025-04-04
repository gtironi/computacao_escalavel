#include "BaseClasses.h"
#include "Manager.h"

// ------------------------
// Implementações reais
// ------------------------

#include <string>
#include <iostream>

class CsvExtractor : public Extractor<CsvExtractor, std::string> {
public:
    // Implementação personalizada de run() com 2 argumentos
    std::string run(int linha, int coluna) {
        return "Dado da linha " + std::to_string(linha) + 
               ", coluna " + std::to_string(coluna);
    }

    // Pode também ter outras sobrecargas
    std::string run(const std::string& nome_arquivo) {
        return "Lendo dados do arquivo: " + nome_arquivo;
    }
};
    
// class MyTransformer : public Transformer<int> {
//     public:
//         using Transformer<int>::Transformer;
    
//         void run() override {
//             int val = input_buffer.pop();
//             int result = val * 10;
//             std::cout << "[Transformer] " << val << " -> " << result << std::endl;
//             output_buffer.push(result);
//         }
//     };
    
// class MyLoader : public Loader<int> {
//     public:
//         using Loader<int>::Loader;
    
//         void run() override {
//                 int val = input_buffer.pop();
//                 std::cout << "[Loader] Resultado final: " << val << std::endl;
        
//         }
//     };
    
// ------------------------
// Main
// ------------------------
// A concrete implementation of Extractor
class SquareExtractor : public Extractor<SquareExtractor, int> {
    public:
        SquareExtractor(TaskQueue* tq) { taskqueue = tq; }
    
        int run(int x) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
            return x * x;
        }
    };
    
    int main() {
        TaskQueue taskQueue;
    
        std::thread worker([&taskQueue]() {
            while (true) {
                auto task = taskQueue.pop_task();
                if (!task) break;
                (*task)();
            }
        });
    
        SquareExtractor extractor(&taskQueue);
    
        std::thread producer([&extractor]() {
            for (int i = 1; i <= 5; ++i) {
                extractor.add_task_thread(i);
                std::cout << "Added task for number: " << i << std::endl;
            }
        });
    
        std::thread consumer([&extractor]() {
            for (int i = 1; i <= 5; ++i) {
                int result = extractor.get_output_buffer().pop();
                std::cout << "Got result: " << result << std::endl;
            }
        });
    
        producer.join();
        consumer.join();
        
        taskQueue.shutdown();
        worker.join();
    
        return 0;
    }