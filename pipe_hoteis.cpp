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

// Classe de filtro do hotel, que filtra os hotéis ocupados
class FiltroHotel : public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor
        // Definição do método de cálculo das estatísticas
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

        // Definição do método do processamento
        Dataframe run(std::vector<Dataframe*> input) override {
            Dataframe df_filtred = (*input[0]).filtroByValue("ocupado", 0);
            return df_filtred;
        }
};

// Classe do transformador que calcula o preço médio das reservas
class PrecoMedio: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        // Definição do método do processamento
        Dataframe run(std::vector<Dataframe*> input) override {
            input[0]->bColumnOperation("preco_sum", "count_reservas", division, "preco_medio");
            return *input[0];
        }
};

// Classe do bloco de Join das bases de reservas e pesquisas
class Join: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        // Definição do método do processamento
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
                
                df_merged = input[0]->merge(*input[1], {"cidade_destino", "data_ida_dia", "data_ida_mes"});
            }

            return df_merged;
        }
};

// Classe do transformador que calcula a taxa de ocupação dos hotéis
class TaxaOcupacaoHoteis: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        // Definição do método do processamento
        Dataframe run(std::vector<Dataframe*> input) override {
            input[0]->bColumnOperation("count_pesquisas", "quantidade_pessoas_sum", division, "taxa_ocupacao_hoteis");
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


          // Pipeline Hoteis e Pesquisas ------------------------------------------------------------------------
    
            // Inicializa o extrator dos dados de pesquisa e o adiciona ao manager
            Extrator<Dataframe> extrator_pesquisa("./mock/data/dados_pesquisas_2025.db", "sql", 1000, "Viagens");
            manager.addExtractor(&extrator_pesquisa);

            // Inicializa o extrator dos dados de reserva e o adiciona ao manager
            Extrator<Dataframe> extrator_reservas("./mock/data/dados_reservas_2025.csv", "csv", 25000);
            manager.addExtractor(&extrator_reservas);

            // Inicializa o filtro dos hotéis e o adiciona ao manager
            FiltroHotel filtro_hotel;
            filtro_hotel.addInputBuffer(&extrator_reservas.get_output_buffer());
            manager.addTransformer(&filtro_hotel);

            // Setando os parâmetros do agrupador de reservas
            std::vector<std::string> vstrColumnsToAggregate = {"quantidade_pessoas", "preco"};
            std::vector<string> group = {"cidade_destino", "data_ida_dia", "data_ida_mes"};
            std::vector<string> ops = {"sum"};

            // Inicializa o agrupador das reservas e o adiciona ao manager
            GroupByTransformer<Dataframe> groupby_reservas(&filtro_hotel.get_output_buffer(),
                                                        group,
                                                        vstrColumnsToAggregate,
                                                        ops, "count_reservas");
            manager.addTransformer(&groupby_reservas);

            // Inicializa o calculador do preço médio das reservas e o adiciona ao manager
            PrecoMedio preco_medio;
            preco_medio.addInputBuffer(&groupby_reservas.get_output_buffer());
            manager.addTransformer(&preco_medio);

            // Setando os parâmetros do agrupador de pesquisas
            vstrColumnsToAggregate = {};
            group = {"cidade_destino", "data_ida_dia", "data_ida_mes"};
            ops = {"count"};

            // Inicializa o agrupador das pesquisas e o adiciona ao manager
            GroupByTransformer<Dataframe> groupby_pesquisas(&extrator_pesquisa.get_output_buffer(),
                                                            group,
                                                            vstrColumnsToAggregate,
                                                            ops, "count_pesquisas");
            manager.addTransformer(&groupby_pesquisas);
            
            // Inicializa o bloco de Join e o adiciona ao manager
            Join join(2);
            join.addInputBuffer(&preco_medio.get_output_buffer());
            join.addInputBuffer(&groupby_pesquisas.get_output_buffer());
            manager.addTransformer(&join);

            // Inicializa o calculador da taxa de ocupação dos hotéis e o adiciona ao manager
            TaxaOcupacaoHoteis taxa_ocupacao_hoteis;
            taxa_ocupacao_hoteis.addInputBuffer(&join.get_output_buffer());
            manager.addTransformer(&taxa_ocupacao_hoteis);

            // Inicializa o calculador do faturamento esperado das cidades e o adiciona ao manager
            Faturamento faturamento;
            faturamento.addInputBuffer(&join.get_output_buffer());
            manager.addTransformer(&faturamento);

            // Inicializa os loaders e os adiciona ao manager
            DataPrinter loader_ocupacao_hoteis(taxa_ocupacao_hoteis.get_output_buffer());
            manager.addLoader(&loader_ocupacao_hoteis);

            DataPrinter loader_faturamento(faturamento.get_output_buffer());
            manager.addLoader(&loader_faturamento);


        auto start_time = std::chrono::high_resolution_clock::now();

        // Start the pipeline
        manager.run();

        auto end_time = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        // std::cout << "Tempo de execução: " << duration << " ms" << std::endl;
        // cout << "Número de quartos ocupados em toda a base " << filtroocupacao.getStats()[0] << endl;
        // cout << "Número de quartos não ocupados em toda a base " << filtroocupacao.getStats()[1] << endl;
        // cout << "Número de quartos no Rio de Janeiro " << filtroocupacao.getStats()[2] << endl;
        // cout << "Número de quartos em Campo Grande " << filtroocupacao.getStats()[3] << endl;
        tempos_execucao.push_back(duration);
        }

        double media = std::accumulate(tempos_execucao.begin(), tempos_execucao.end(), 0.0) / tempos_execucao.size();
        std::cout << "Paralelismo: " << p << " -> Tempo médio: " << media << " ms" << std::endl;
        }

    return 0;
}
