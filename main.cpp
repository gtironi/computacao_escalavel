// ConcreteExtractor.h
#include "framework/BaseClasses.h"
#include "framework/Dataframe.h"
#include "framework/Manager.h"
#include <thread>
#include <chrono>
#include <iostream>

<<<<<<< Updated upstream
class Filter : public Transformer<Dataframe> {
=======
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
>>>>>>> Stashed changes
    public:
        using Transformer::Transformer; // Herda o construtor
    
        Dataframe run(std::vector<Dataframe*> input) override {
<<<<<<< Updated upstream
            if (input.size() < 2) {
                throw std::invalid_argument("Filter requires at least two input Dataframes.");
            }
    
            // Obtém o número de linhas dos dois dataframes
            int nrows1 = input[0]->getShape().second;
            int nrows2 = input[1]->getShape().second;
=======
            cout << "Chegou" << endl;
            Dataframe df_filtred = (*input[0]).filtroByValue("ocupado", 0);
            return df_filtred;
        }
};

class join: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {

            Dataframe df_merged ;

            // cout << *input[0] << endl;

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

// class disponibilidade: public Transformer<Dataframe> {
//     public:
//         using Transformer::Transformer; // Herda o construtor

//         Dataframe run(std::vector<Dataframe*> input) override {

//             input[0]->bColumnOperation("data_ida_dia_count", "quantidade_pessoas_count", division, "disponibilidade");
//             return *input[0];
//         }
// };
class disponibilidade: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {

            input[0]->bColumnOperation("preco_sum", "count_sum", division, "preco_medio");

            // std::cout << *input[0];
            return *input[0];
        }
};

class taxa_ocup: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {

            input[0]->bColumnOperation("count", "quantidade_pessoas_sum", division, "taxa_ocup");

            return *input[0];
        }
};

class faturamento: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {

            input[0]->bColumnOperation("count", "quantidade_pessoas_sum", minimun, "minimo");
            input[0]->bColumnOperation("minimo", "preco_medio", multiplication, "faturamento_esperado");

            return *input[0];
        }
};

class taxa_ocup_voo: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        vector<float> calculateStats(std::vector<Dataframe*> input) override {
            vector<float> calculateStats;
            float n_cidades_diferente = (*input[0]).getShape().first;
            calculateStats.push_back(n_cidades_diferente);

            return calculateStats;
        }
>>>>>>> Stashed changes
        
            // Retorna o dataframe com menos linhas
            return (nrows1 < nrows2) ? *input[0] : *input[1];
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
    // Create manager with 7 threads
    Manager<Dataframe> manager(7);

<<<<<<< Updated upstream
    // Create pipeline components
    Extrator<Dataframe> extrator1("./mock/data/dados_viagens_2025.csv", "csv", 1000);
    manager.addExtractor(&extrator1);
    
    Extrator<Dataframe> extrator2("./mock/data/dados_viagens_2025.csv", "csv", 1000);
    manager.addExtractor(&extrator2);

    std::vector<Buffer<Dataframe>*> inputs_buffers;
    inputs_buffers.push_back(&extrator1.get_output_buffer());
    inputs_buffers.push_back(&extrator2.get_output_buffer());
=======
    // Pipeline Hoteis e Pesquisas ------------------------------------------------------------------------
    Extrator<Dataframe> extrator_pesquisa("./mock/data/dados_pesquisa_2025.db", "sql", 1000, "Viagens");
    manager.addExtractor(&extrator_pesquisa);

    Extrator<Dataframe> extrator_reservas("./mock/data/dados_reservas_2025.csv", "csv", 25000);
    manager.addExtractor(&extrator_reservas);

    filter_hotel filtroocupacao(make_input_vector(extrator_reservas.get_output_buffer()));
    manager.addTransformer(&filtroocupacao);

    std::vector<std::string> vstrColumnsToAggregate = {"quantidade_pessoas", "preco"};
    std::vector<string> group = {"cidade_destino", "data_ida_dia", "data_ida_mes"};
    std::vector<string> ops = {"sum"};

    GroupByTransformer<Dataframe> groupby_hotel(make_input_vector(filtroocupacao.get_output_buffer()),
                                                group,
                                                vstrColumnsToAggregate,
                                                ops);
    manager.addTransformer(&groupby_hotel);

    disponibilidade disp(make_input_vector(groupby_hotel.get_output_buffer()));
    manager.addTransformer(&disp);

    vstrColumnsToAggregate = {};
    group = {"cidade_destino", "data_ida_dia", "data_ida_mes"};
    ops = {"count"};

    GroupByTransformer<Dataframe> groupby_pesquisa(make_input_vector(extrator_pesquisa.get_output_buffer()),
                                                    group,
                                                    vstrColumnsToAggregate,
                                                    ops);
    manager.addTransformer(&groupby_pesquisa);

    std::vector<Buffer<Dataframe>*> inputs_buffers;
    inputs_buffers.push_back(&disp.get_output_buffer());
    inputs_buffers.push_back(&groupby_pesquisa.get_output_buffer());
>>>>>>> Stashed changes

    Filter transformer(inputs_buffers, 2);
    manager.addTransformer(&transformer);

<<<<<<< Updated upstream
    DataPrinter loader1(transformer.get_output_buffer());
    manager.addLoader(&loader1);

    DataPrinter loader2(transformer.get_output_buffer());
=======
    taxa_ocup taxa_ocupacao(make_input_vector(join_transformer.get_output_buffer()));
    manager.addTransformer(&taxa_ocupacao);

    DataPrinter loader1(disp.get_output_buffer());
    manager.addLoader(&loader1);

    faturamento fatur(make_input_vector(join_transformer.get_output_buffer()));
    manager.addTransformer(&fatur);

    DataPrinter loader2(fatur.get_output_buffer());
>>>>>>> Stashed changes
    manager.addLoader(&loader2);


    auto start_time = std::chrono::high_resolution_clock::now();

    // Start the pipeline
    manager.run();

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Tempo de execução: " << duration << " ms" << std::endl;

    // The manager's destructor will clean up everything
<<<<<<< Updated upstream
    return 0;
=======
    
    return;

}

int main() {
    string strCsvPath1 = "./mock/data/dados_pesquisa_2025.db";
    string strCsvPath2 = "./mock/data/dados_reservas_2025.csv";
    string strCsvPath3 = "./mock/data/dados_voos_2025.csv";

    // Execução programada a cada 10 minutos
    TimeTrigger timeTrigger(pipeline, 600);
    
    // Verificação dos arquivos a cada 1 minuto
    EventTrigger trigger(strCsvPath1, pipeline, 60);
    EventTrigger trigger2(strCsvPath2, pipeline, 60);
    EventTrigger trigger3(strCsvPath3, pipeline, 60);

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
>>>>>>> Stashed changes
}
