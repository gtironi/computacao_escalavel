// ConcreteExtractor.h
#include "BaseClasses.h"
#include "Dataframe.h"
#include "Manager.h"
#include <thread>
#include <chrono>

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

        void run(Dataframe df) override {
            // Print the dataframe contents
            std::cout << df << std::endl;
        }
    };



int main() {
    // Create manager with 4 threads
    Manager<Dataframe> manager(4);

    // Create pipeline components
    Extrator<Dataframe> extrator("../mock/data/dados_viagens_2025.csv", "csv", 10);
    manager.addExtractor(&extrator);

    ValueMultiplier transformer(extrator.get_output_buffer());
    manager.addTransformer(&transformer);

    DataPrinter loader(transformer.get_output_buffer());
    manager.addLoader(&loader);

    // Start the pipeline by adding tasks to the extractor
    // We'll process 3 dataframes
    manager.run();

    // In a real application, you'd have proper termination conditions
    // Here we just wait a bit to let the pipeline process everything
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Trabalho será finalizado" << std::endl;
    manager.stop();
    std::cout << "Trabalho finalizado" << std::endl;

    // The manager's destructor will clean up everything
    return 0;
}
