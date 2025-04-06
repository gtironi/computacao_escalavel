// ConcreteExtractor.h
#include "framework/BaseClasses.h"
#include "framework/Dataframe.h"
#include "framework/Manager.h"
#include <thread>
#include <chrono>
#include <iostream>

class ValueMultiplier : public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Inherit constructor

        Dataframe run(Dataframe input) override {
            Dataframe output = input.filtroByValue("cidade_destino", "Rio de Janeiro"); // Copy the input
            return output;
        }
    };

class DataPrinter : public Loader<Dataframe> {
    public:
        using Loader::Loader; // Inherit constructor

        private:
            bool headerPrinted = false;

        void run(Dataframe df) override {
            if (!headerPrinted) {
                df.printHeader(std::cout, df);
                headerPrinted = true;
            }

            // Print the dataframe contents
            std::cout << df;
        }
    };

int main() {
    // Create manager with 4 threads
    Manager<Dataframe> manager(1);

    // Create pipeline components
    Extrator<Dataframe> extrator("./mock/data/dados_viagens_2025.csv", "csv", 10);
    manager.addExtractor(&extrator);

    ValueMultiplier transformer(extrator.get_output_buffer());
    manager.addTransformer(&transformer);

    DataPrinter loader(transformer.get_output_buffer());
    manager.addLoader(&loader);

    auto start_time = std::chrono::high_resolution_clock::now();

    // Start the pipeline by adding tasks to the extractor
    manager.run();

    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "Trabalho será finalizado" << std::endl;
    manager.stop();
    std::cout << "Trabalho finalizado" << std::endl;

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Tempo de execução: " << duration << " ms" << std::endl;

    // The manager's destructor will clean up everything
    return 0;
}
