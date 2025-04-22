// ConcreteExtractor.h
#include "framework/BaseClasses.h"
#include "framework/Transformer.h"
#include "framework/Dataframe.h"
#include "framework/Manager.h"
#include "framework/Triggers.h"
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

string minimun(string str1, string str2){
    double num1 = stod(str1);
    double num2 = stod(str2);
    return to_string(num1 < num2 ? num1 : num2);
}

class filter_hotel : public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor
        vector<float> calculateStats(std::vector<Dataframe*> input) override {
            vector<float> calculateStats;
            Dataframe df_filtred = (*input[0]).filtroByValue("ocupado", 0);
            float n_vagos = df_filtred.getShape().first;
            float n_reservados = (*input[0]).getShape().first - n_vagos;
            calculateStats.push_back(n_vagos);
            calculateStats.push_back(n_reservados);

            Dataframe df_filtred_2 = (*input[0]).filtroByValue("cidade_destino", std::string("Rio de Janeiro"));
            Dataframe df_filtred_3 = (*input[0]).filtroByValue("cidade_destino", std::string("Campo Grande"));
            float n_rio_janeiro = df_filtred_2.getShape().first;
            float n_campo_grande = df_filtred_3.getShape().first;
            calculateStats.push_back(n_rio_janeiro);
            calculateStats.push_back(n_campo_grande);


            return calculateStats;
        }

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

            // input[0] -> printColsName();

            if ((input[0] -> columns.empty()))
            {
                df_merged = *input[0];
            }
            else if ((input[1] -> columns.empty()))
            {
                df_merged = *input[1];
            }
            else{

                df_merged = input[0]->merge(*input[1], {"cidade_destino", "data_ida_dia", "data_ida_mes"});
            }

            return df_merged;
        }
};

class PrecoMedio: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {
            input[0]->bColumnOperation("preco_sum", "count_reservas", division, "preco_medio");
            return *input[0];
        }
};

class TaxaOcupacao: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {
            // std::cout << "Taxa 1" << std::endl;
            input[0]->bColumnOperation("count_pesquisas", "quantidade_pessoas_sum", division, "taxa_ocupacao");
            // std::cout << "Taxa 2" << std::endl;
            return *input[0];
        }
};



// Classe do transformador que calcula o faturamento esperado em cada cidade e dia
class Faturamento: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        // Definição do método do processamento
        Dataframe run(std::vector<Dataframe*> input) override {

            input[0]->bColumnOperation("count_reservas", "quantidade_pessoas_sum", minimun, "demanda");
            input[0]->bColumnOperation("preco_medio", "demanda", multiplication, "faturamento_esperado");

            return *input[0];
        }
};


// Classe do transformador que calcula a taxa de ocupação dos voos
class TaxaOcupacaoVoos: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        // Definição do método de cálculo das estatísticas
        vector<float> calculateStats(std::vector<Dataframe*> input) override {
            vector<float> calculateStats;
            float n_cidades_diferente = (*input[0]).getShape().first;
            calculateStats.push_back(n_cidades_diferente);

            return calculateStats;
        }
        
        // Definição do método do processamento
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
                //df.printHeader(std::cout, df); // Print the dataframe head
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
            // Pipeline Voos ------------------------------------------------------------------------

            // Inicializa o extrator dos dados de voo e o adiciona ao manager
            Extrator<Dataframe> extrator_voos("./mock/data/dados_voos_2025.csv", "csv", 15000);
            manager.addExtractor(&extrator_voos);

            std::vector<std::string> vstrColumnsToAggregate = {"assentos_ocupados", "assentos_totais"};
            std::vector<string> group = {"cidade_destino"};
            std::vector<string> ops = {"sum"};

            // Inicializa o agrupador dos voos e o adiciona ao manager
            GroupByTransformer<Dataframe> groupby_voo(
                &extrator_voos.get_output_buffer(),
                group,
                vstrColumnsToAggregate,
                ops,
                "count_voos"
            );
            manager.addTransformer(&groupby_voo);

            // Inicializa o calculador da taxa de ocupação dos voos e o adiciona ao manager
            TaxaOcupacaoVoos taxa_ocupacao_voos;
            taxa_ocupacao_voos.addInputBuffer(&groupby_voo.get_output_buffer());
            manager.addTransformer(&taxa_ocupacao_voos);

            // Inicializa o loader e o adiciona ao manager
            DataPrinter loader_ocupacao_voos(taxa_ocupacao_voos.get_output_buffer());
            manager.addLoader(&loader_ocupacao_voos);

            auto start_time = std::chrono::high_resolution_clock::now();

            // Start the pipeline
            manager.run();

            auto end_time = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            //std::cout << "Tempo de execução: " << duration << " ms" << std::endl;
            //cout << "Número de Cidades Destino Diferente em toda a base " << taxaocupvoo.getStats()[0] << endl;
            tempos_execucao.push_back(duration);
        }

        double media = std::accumulate(tempos_execucao.begin(), tempos_execucao.end(), 0.0) / tempos_execucao.size();
        std::cout << "Paralelismo: " << p << " -> Tempo médio: " << media << " ms" << std::endl;
        }

    return 0;
}
