// ConcreteExtractor.h
#include "framework/BaseClasses.h"
#include "framework/Dataframe.h"
#include "framework/Manager.h"
#include <thread>
#include <chrono>
#include <iostream>

std::vector<Buffer<Dataframe>*> make_input_vector(Buffer<Dataframe>& buffer) {
    return { &buffer };
}

// Função utilitária no topo do arquivo
template<typename... Buffers>
std::vector<Buffer<Dataframe>*> make_input_vector(Buffers&... buffers) {
    return { &buffers... };
}

string division(string str1, string str2){
    double num1 = stod(str1);
    double num2 = stod(str2);
    if (num2 == 0) {
        return "Erro: Divisão por zero";
    }
    return to_string(num1 / num2);
}

class groupby_voo : public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {

            std::vector<std::string> vstrColumnsToAggregate = {"assentos_ocupados", "assentos_totais"};

            Dataframe df_gruped = input[0]-> dfGroupby("cidade_destino", "sum", vstrColumnsToAggregate);

            return df_gruped;
        }
};

class taxa_ocup_voo: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {
            Dataframe* df = input[0];

            // Chama o método da própria instância Dataframe
            df->bColumnOperation(
                "assentos_ocupados",           // Nome da coluna 1 (numerador)
                "assentos_totais",             // Nome da coluna 2 (denominador)
                division,
                "ocupacao_relativa"            // Nome da nova coluna a ser criada
            );

            return *df;

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
            std::cout << df;
        }
    };

int main() {
    // Inicializa o Manager
    Manager<Dataframe> manager(7);

    // Cria os extratores
    // Extrator<Dataframe> extrator_pesquisa("./mock/data/dados_viagens_2025.csv", "db", 1000);
    // manager.addExtractor(&extrator_pesquisa);

    // Extrator<Dataframe> extrator_hoteis("./mock/data/dados_hoteis_2025.csv", "csv", 1000);
    // manager.addExtractor(&extrator_hoteis);

    Extrator<Dataframe> extrator_voos("./mock/data/dados_voos_2025.csv", "csv", 1000);
    manager.addExtractor(&extrator_voos);

    groupby_voo groupbyvoo(make_input_vector(extrator_voos.get_output_buffer()));
    manager.addTransformer(&groupbyvoo);

    taxa_ocup_voo taxaocupvoo(make_input_vector(groupbyvoo.get_output_buffer()));
    manager.addTransformer(&taxaocupvoo);

    // Para criar um próximo bloco que recebe como input um dos blocos de output,
    // basta passar como buffer de input o get_output_buffer() do bloco anterior
    // A atribuição do buffer de saída seguinte é automática
    DataPrinter loader1(taxaocupvoo.get_output_buffer());
    manager.addLoader(&loader1);

    auto start_time = std::chrono::high_resolution_clock::now();

    // Start the pipeline
    manager.run();

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Tempo de execução: " << duration << " ms" << std::endl;

    // The manager's destructor will clean up everything
    return 0;
}
