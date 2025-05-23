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

bool PRINT_OUTPUT_DFS = false;
bool TRIGGERS = false;
int N_THREADS = 7;

// Função auxiliar para fazer a divisão entre dois valores
string division(string str1, string str2){
    double num1 = stod(str1);
    double num2 = stod(str2);
    if (num2 == 0) {
        return "Erro: Divisão por zero";
    }
    return to_string(num1 / num2);
}

// Função auxiliar para fazer a multiplicação de dois valores
string multiplication(string str1, string str2){
    double num1 = stod(str1);
    double num2 = stod(str2);
    return to_string(num1 * num2);
}

// Função auxiliar para retornar o mínimo entre dois valores
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

// Classe do carregador dos dados finais (no caso, apenas printa)
class DataPrinter : public Loader<Dataframe> {
    public:
        using Loader::Loader; // Inherit constructor

        private:
            bool headerPrinted = false;

        // Definição do método do processamento
        // Para mostrar os prints, descomentar as duas linhas comentadas abaixo
        void run(Dataframe df) override {
            if (PRINT_OUTPUT_DFS)
            {
                if (!headerPrinted) {
                    df.printHeader(std::cout, df); // Print the dataframe head
                    headerPrinted = true;
                }
    
                // Print the dataframe contents
                std::cout << df;
            }
           
        }
    };


// Função para executar o pipeline
std::vector<int> pipeline() {
    // Inicializa o Manager
    Manager<Dataframe> manager(N_THREADS);

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

    // Pipeline Voos ------------------------------------------------------------------------

    // Inicializa o extrator dos dados de voo e o adiciona ao manager
    Extrator<Dataframe> extrator_voos("./mock/data/dados_voos_2025.csv", "csv", 15000);
    manager.addExtractor(&extrator_voos);

    // Setando os parâmetros do agrupador de voos
    vstrColumnsToAggregate = {"assentos_ocupados", "assentos_totais"};
    group = {"cidade_destino"};
    ops = {"sum"};

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

    // Começa o pipeline
    manager.run();

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    // Printando as estatísticas
    cout << "Número de quartos ocupados em toda a base: " << filtro_hotel.getStats()[0] << endl;
    cout << "Número de quartos não ocupados em toda a base: " << filtro_hotel.getStats()[1] << endl;
    cout << "Número de quartos no Rio de Janeiro: " << filtro_hotel.getStats()[2] << endl;
    cout << "Número de quartos em Campo Grande: " << filtro_hotel.getStats()[3] << endl;
    cout << "Número de cidades destino diferentes em toda a base: " << taxa_ocupacao_voos.getStats()[0] << endl;
    cout << "==============================================================================" << endl;
    std::cout << "Tempo de execução: " << duration << " ms" << std::endl;
    
    std::vector<int> stats;
    stats.push_back(filtro_hotel.getStats()[0]);
    stats.push_back(filtro_hotel.getStats()[1]);
    stats.push_back(filtro_hotel.getStats()[2]);
    stats.push_back(filtro_hotel.getStats()[3]);
    stats.push_back(taxa_ocupacao_voos.getStats()[0]);
    return stats;
}

int main() {
    if (TRIGGERS){
        string strCsvPath1 = "./mock/data/dados_pesquisas_2025.db";
        string strCsvPath2 = "./mock/data/dados_reservas_2025.csv";
        string strCsvPath3 = "./mock/data/dados_voos_2025.csv";
    
        // Execução programada a cada 1 minutos
        TimeTrigger timeTrigger(pipeline, 60);
        
        // Verificação dos arquivos a cada 30 segundos
        EventTrigger trigger(strCsvPath1, pipeline, 30);
        EventTrigger trigger2(strCsvPath2, pipeline, 30);
        EventTrigger trigger3(strCsvPath3, pipeline, 30);
    
        // Inicializando a execução dos triggers
        timeTrigger.start();
        trigger.start();
        trigger2.start();
        trigger3.start();
    
        // Rodando o teste por 1 hora
        this_thread::sleep_for(std::chrono::minutes(60));
    
        // Parando os triggers
        timeTrigger.stop();
        trigger.stop();
        trigger2.stop();
        trigger3.stop();

    }
    else {
        std::vector<int> stats = pipeline();
        for (int i = 0; i< stats.size(); i++){
            cout << stats[i] << endl;
        }
    }

}