// ConcreteExtractor.h
#include "framework/BaseClasses.h"
#include "framework/Dataframe.h"
#include "framework/Manager.h"
#include <thread>
#include <chrono>
#include <iostream>

class Filter : public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor
    
        Dataframe run(std::vector<Dataframe*> input) override {
            if (input.size() < 2) {
                throw std::invalid_argument("Filter requires at least two input Dataframes.");
            }
    
            // Obtém o número de linhas dos dois dataframes
            int nrows1 = input[0]->getShape().second;
            int nrows2 = input[1]->getShape().second;
        
            // Retorna o dataframe com menos linhas
            return (nrows1 < nrows2) ? *input[0] : *input[1];
        }

        // Definição pelo usuário das estatísticas
        // Neste exemplo, ele apenas calcula quantas linhas têm o Rio como destino
        // e retorna um vetor com "duas" estatísticas iguais a esse valor
        std::vector<float> calculateStats(std::vector<Dataframe*> input) {
            Dataframe data = *input[0];
            Dataframe filtered = data.filtroByValue("cidade_destino", "Rio de Janeiro");
            float nrows = filtered.getShape().second;
            std::vector<float> result = {nrows, nrows};
            return result;
        }
    };

class DataPrinter : public Loader<Dataframe> {
    public:
        using Loader::Loader; // Inherit constructor

        private:
            bool headerPrinted = false;

        void run(Dataframe df) override {
            if (!headerPrinted) {
                // df.printHeader(std::cout, df); // Print the dataframe head
                headerPrinted = true;
            }

            // Print the dataframe contents
            // std::cout << df;
        }
    };

int main() {
    // Inicializa o Manager
    Manager<Dataframe> manager(7);

    // Cria os extratores
    Extrator<Dataframe> extrator1("./mock/data/dados_viagens_2025.csv", "csv", 1000);
    manager.addExtractor(&extrator1);
    
    Extrator<Dataframe> extrator2("./mock/data/dados_viagens_2025.csv", "csv", 1000);
    manager.addExtractor(&extrator2);

    // Inicializa o vetor de buffers de input
    // O input buffer dos transformers sempre vai ser agora um vetor de ponteiros de buffer de dataframe
    // Ele é inicializado e depois esses ponteiros são adicionados
    // pegando os endereços dos buffers que se deseja colocar como input desse transformer
    std::vector<Buffer<Dataframe>*> inputs_buffers;
    inputs_buffers.push_back(&extrator1.get_output_buffer());
    inputs_buffers.push_back(&extrator2.get_output_buffer());

    // Inicializa o transformer com o vetor de buffers de input e com o número de buffers de output
    Filter transformer(inputs_buffers, 2);
    manager.addTransformer(&transformer);

    // Para criar um próximo bloco que recebe como input um dos blocos de output,
    // basta passar como buffer de input o get_output_buffer() do bloco anterior
    // A atribuição do buffer de saída seguinte é automática
    DataPrinter loader1(transformer.get_output_buffer());
    manager.addLoader(&loader1);

    DataPrinter loader2(transformer.get_output_buffer());
    manager.addLoader(&loader2);


    auto start_time = std::chrono::high_resolution_clock::now();

    // Start the pipeline
    manager.run();

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Tempo de execução: " << duration << " ms" << std::endl;

    // Pegando as estatísticas no final e printando
    std::vector<float> stats = transformer.getStats();

    for (int i = 0; i < stats.size(); i++)
    {
        std::cout << stats[i] << " ";
    }
    std::cout << std::endl;

    // The manager's destructor will clean up everything
    return 0;
}
