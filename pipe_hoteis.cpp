// ConcreteExtractor.h
#include "framework/BaseClasses.h"
#include "framework/Transformer.h"
#include "framework/Dataframe.h"
#include "framework/Manager.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <numeric>

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

string multiplication(string str1, string str2){
    double num1 = stod(str1);
    double num2 = stod(str2);
    return to_string(num1 * num2);
}

class filter_hotel : public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {
            Dataframe df_filtred = (*input[0]).filtroByValue("ocupado", 0);
            return df_filtred;
        }
};

class join: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {

            Dataframe df_merged ;


            if ((input[0] -> columns.empty()))
            {
                df_merged = *input[0];
            }
            else if ((input[1] -> columns.empty()))
            {
                df_merged = *input[1];
            }
            else{

                df_merged = input[0]->merge(*input[1], {"cidade_destino"});
            }

            return df_merged;
        }
};

class disponibilidade: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {

            input[0]->bColumnOperation("data_ida_dia_count", "quantidade_pessoas_count", division, "disponibilidade");
            return *input[0];
        }
};


class faturamento: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {

            input[0]->bColumnOperation("preco_sum", "quantidade_pessoas_count", division, "preco_medio");

            input[0]->bColumnOperation("preco_medio", "data_ida_dia_count", multiplication, "faturamento_esperado");

            return *input[0];
        }
};

class DataPrinter : public Loader<Dataframe> {
    public:
        using Loader::Loader; // Inherit constructor

        private:
            bool headerPrinted = false;

        void run(Dataframe df) override {
            if (!headerPrinted) {
                //df.printHeader(std::cout, df); // Print the dataframe head
                headerPrinted = true;
            }

            // Print the dataframe contents
            //std::cout << df;
        }
    };



int main() {
    // Inicializa o Manager
    Manager<Dataframe> manager(7);

    // Pipeline Hoteis e Pesquisas ------------------------------------------------------------------------
    Extrator<Dataframe> extrator_pesquisa("./mock/data/dados_viagens_2025.csv", "csv", 100);
    manager.addExtractor(&extrator_pesquisa);

    Extrator<Dataframe> extrator_hoteis("./mock/data/dados_hoteis_2025.csv", "csv", 1000);
    manager.addExtractor(&extrator_hoteis);

    filter_hotel filtroocupacao(make_input_vector(extrator_hoteis.get_output_buffer()));
    manager.addTransformer(&filtroocupacao);

    std::vector<std::string> vstrColumnsToAggregate = {"quantidade_pessoas", "preco"};
    std::vector<string> group = {"cidade_destino"};
    std::vector<string> ops = {"sum"};

    GroupByTransformer<Dataframe> groupby_hotel(make_input_vector(filtroocupacao.get_output_buffer()),
                                                group,
                                                vstrColumnsToAggregate,
                                                ops);
    manager.addTransformer(&groupby_hotel);

    vstrColumnsToAggregate = {"data_ida_dia", "data_ida_mes", "data_ida_ano"};
    group = {"cidade_destino"};
    ops = {"sum"};

    GroupByTransformer<Dataframe> groupby_pesquisa(make_input_vector(extrator_pesquisa.get_output_buffer()),
                                                    group,
                                                    vstrColumnsToAggregate,
                                                    ops);
    manager.addTransformer(&groupby_pesquisa);

    std::vector<Buffer<Dataframe>*> inputs_buffers;
    inputs_buffers.push_back(&groupby_hotel.get_output_buffer());
    inputs_buffers.push_back(&groupby_pesquisa.get_output_buffer());

    join join_transformer(inputs_buffers, 2);
    manager.addTransformer(&join_transformer);

    disponibilidade disp(make_input_vector(join_transformer.get_output_buffer()));
    manager.addTransformer(&disp);

    faturamento fatur(make_input_vector(join_transformer.get_output_buffer()));
    manager.addTransformer(&fatur);

    DataPrinter loader1(disp.get_output_buffer());
    manager.addLoader(&loader1);

    DataPrinter loader2(fatur.get_output_buffer());
    manager.addLoader(&loader2);

    auto start_time = std::chrono::high_resolution_clock::now();

    // Start the pipeline
    manager.run();

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Tempo de execução: " << duration << " ms" << std::endl;

    // The manager's destructor will clean up everything
    return 0;
}
