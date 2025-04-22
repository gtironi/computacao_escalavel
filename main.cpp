// ConcreteExtractor.h
#include "framework/BaseClasses.h"
#include "framework/Dataframe.h"
#include "framework/Manager.h"
#include "framework/Transformer.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <numeric>

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

class ocupacao_pesquisa: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {

            input[0]->bColumnOperation("preco_sum", "count", division, "preco_dividido");
            // cout << *input[0] << endl;
            return *input[0];
        }        
};

class groupby_voo : public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {

            std::vector<std::string> vstrColumnsToAggregate = {"assentos_ocupados", "assentos_totais"};

            std::vector<std::string> vstrColumnsToGroup = {"cidade_destino"};

            Dataframe df_gruped = input[0]-> dfGroupby(vstrColumnsToGroup, vstrColumnsToAggregate, true, false, false);

            //std::cout << df_gruped;

            return df_gruped;
        }
};

class taxa_ocup_voo: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {
            Dataframe df = *input[0];

            // Chama o método da própria instância Dataframe
            df.bColumnOperation(
                "assentos_ocupados_sum",           // Nome da coluna 1 (numerador)
                "assentos_totais_sum",             // Nome da coluna 2 (denominador)
                division,
                "ocupacao_relativa"            // Nome da nova coluna a ser criada
            );

            return df;

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
            //std::cout << df;
        }
    };

int main() {
    std::vector<int> paralelismos = {1, 3, 5, 7, 9, 11};

    for (int p : paralelismos) {
        std::vector<long long> tempos_execucao;

        for (int i = 0; i < 10; ++i) {
            // Inicializa o Manager
            Manager<Dataframe> manager(p);

            // Cria os extratores
            Extrator<Dataframe> extrator_pesquisa("./mock/data/dados_viagens_2025.csv", "csv", 1000);
            manager.addExtractor(&extrator_pesquisa);

            Extrator<Dataframe> extrator_hoteis("./mock/data/dados_hoteis2_2025.csv", "csv", 50);
            manager.addExtractor(&extrator_hoteis);

            std::vector<std::string> vstrColumnsToAggregate = {"quantidade_pessoas", "preco"};
            std::vector<string> group = {"cidade_destino"};
            std::vector<string> ops = {"sum"};

            GroupByTransformer<Dataframe> groupby_hotel(make_input_vector(extrator_hoteis.get_output_buffer()),
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

            join join_transformer(inputs_buffers, 1);
            manager.addTransformer(&join_transformer);

            ocupacao_pesquisa ocup_1(make_input_vector(join_transformer.get_output_buffer()));
            manager.addTransformer(&ocup_1);

            DataPrinter loader2(ocup_1.get_output_buffer());
            manager.addLoader(&loader2);

            Extrator<Dataframe> extrator_voos("./mock/data/dados_voos_2025.csv", "csv", 1000);
            manager.addExtractor(&extrator_voos);

            vstrColumnsToAggregate = {"quantidade_pessoas", "preco"};
            group = {"cidade_destino"};
            ops = {"sum", "sum"}; // uma operação por coluna

            GroupByTransformer<Dataframe> groupbyvoo(
                make_input_vector(extrator_hoteis.get_output_buffer()),
                group,
                vstrColumnsToAggregate,
                ops
            );

            manager.addTransformer(&groupby_hotel);

            taxa_ocup_voo taxaocupvoo(make_input_vector(groupbyvoo.get_output_buffer()));
            manager.addTransformer(&taxaocupvoo);

            DataPrinter loader1(taxaocupvoo.get_output_buffer());
            manager.addLoader(&loader1);

            auto start_time = std::chrono::high_resolution_clock::now();
            manager.run();
            auto end_time = std::chrono::high_resolution_clock::now();

            long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            tempos_execucao.push_back(duration);
        }

        double media = std::accumulate(tempos_execucao.begin(), tempos_execucao.end(), 0.0) / tempos_execucao.size();
        std::cout << "Paralelismo: " << p << " -> Tempo médio: " << media << " ms" << std::endl;
    }

    return 0;
}
