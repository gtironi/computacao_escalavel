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
            Manager<Dataframe> manager(p);

            Extrator<Dataframe> extrator_voos("./mock/data/dados_voos_2025.csv", "csv", 1000);
            manager.addExtractor(&extrator_voos);

            groupby_voo groupbyvoo(make_input_vector(extrator_voos.get_output_buffer()));
            manager.addTransformer(&groupbyvoo);

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
