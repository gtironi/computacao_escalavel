// ConcreteExtractor.h
#include "BaseClasses.h"
#include "Dataframe.h"
#include "Manager.h"
#include <thread>
#include <chrono>

class DataExtractor : public Extractor<Dataframe> {
public:
    Dataframe run() {
        // Create a simple dataframe with sample data
        Dataframe df;
        
        // Create and add columns
        Series idSeries("ID", "int");
        Series valueSeries("Value", "double");
        
        // Add some sample data
        for (int i = 0; i < 5; i++) {
            idSeries.bAdicionaElemento(i);
            valueSeries.bAdicionaElemento(i * 1.1);
        }
        
        df.adicionaColuna(idSeries);
        df.adicionaColuna(valueSeries);
        
        return df;

    }
};

class ValueMultiplier : public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Inherit constructor
        
        Dataframe run(Dataframe input) override {
            Dataframe output = input; // Copy the input
            
            // Find the "Value" column
            auto it = std::find(output.vstrColumnsName.begin(), 
                               output.vstrColumnsName.end(), 
                               "Value");
            
            if (it != output.vstrColumnsName.end()) {
                int colIndex = std::distance(output.vstrColumnsName.begin(), it);
                
                // Multiply each value by 2
                for (auto& val : output.columns[colIndex].getData()) {
                    val = std::get<double>(val) * 2.0;
                }
            }
            
            return output;
        }
    };

    class DataPrinter : public Loader<Dataframe> {
        public:
            using Loader::Loader; // Inherit constructor
            
            void run(Dataframe df) override {
                // Print the dataframe contents
                std::cout << "Processed DataFrame:" << std::endl;
                
                // Print column names
                for (const auto& name : df.vstrColumnsName) {
                    std::cout << name << "\t";
                }
                std::cout << std::endl;
                
                // Print data
                auto shape = df.getShape();
                for (int row = 0; row < shape.first; row++) {
                    for (int col = 0; col < shape.second; col++) {
                        // Visit each value and print it
                        std::visit([](const auto& val) { std::cout << val << "\t"; }, 
                                  df.columns[col].getData()[row]);
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl;
            }
        };



int main() {
    // Create manager with 4 threads
    Manager<Dataframe> manager(4);

    // Create pipeline components
    DataExtractor extractor;
    manager.addExtractor(&extractor);
    
    ValueMultiplier transformer(extractor.get_output_buffer());
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