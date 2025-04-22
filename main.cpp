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
            std::cout << "Taxa 1" << std::endl;
            input[0]->bColumnOperation("count_pesquisas", "quantidade_pessoas_sum", division, "taxa_ocupacao");
            std::cout << "Taxa 2" << std::endl;
            return *input[0];
        }
};


class faturamento: public Transformer<Dataframe> {
    public:
        using Transformer::Transformer; // Herda o construtor

        Dataframe run(std::vector<Dataframe*> input) override {

            std::cout << "Fat 1" << std::endl;
            input[0]->bColumnOperation("count_reservas", "quantidade_pessoas_sum", minimun, "demanda");
            std::cout << "Fat 2" << std::endl;
            input[0]->bColumnOperation("preco_medio", "demanda", multiplication, "faturamento_esperado");
            std::cout << "Fat 3" << std::endl;

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
            // Dataframe novo = (*input[0]).filtroByValue("");
            // cout << "O número de assentos ocupados " << novo.columns[0] << endl;
            // calculateStats.push_back(n_assentos_ocup);

            return calculateStats;
        }
        

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
            // std::cout << df;
        }
    };



void pipeline() {
    // Inicializa o Manager
    Manager<Dataframe> manager(7);

    // Pipeline Hoteis e Pesquisas ------------------------------------------------------------------------
    Extrator<Dataframe> extrator_pesquisa("./mock/data/dados_pesquisas_2025.db", "sql", 1000, "Viagens");
    manager.addExtractor(&extrator_pesquisa);

    Extrator<Dataframe> extrator_reservas("./mock/data/dados_reservas_2025.csv", "csv", 25000);
    manager.addExtractor(&extrator_reservas);

    filter_hotel filtroocupacao(make_input_vector(extrator_reservas.get_output_buffer()));
    manager.addTransformer(&filtroocupacao);

    std::vector<std::string> vstrColumnsToAggregate = {"quantidade_pessoas", "preco"};
    std::vector<string> group = {"cidade_destino", "data_ida_dia", "data_ida_mes"};
    std::vector<string> ops = {"sum"};

    GroupByTransformer<Dataframe> groupby_reservas(make_input_vector(filtroocupacao.get_output_buffer()),
                                                   group,
                                                   vstrColumnsToAggregate,
                                                   ops, "count_reservas");
    manager.addTransformer(&groupby_reservas);

    PrecoMedio preco_medio(make_input_vector(groupby_reservas.get_output_buffer()));
    manager.addTransformer(&preco_medio);

    vstrColumnsToAggregate = {};
    group = {"cidade_destino", "data_ida_dia", "data_ida_mes"};
    ops = {"count"};

    GroupByTransformer<Dataframe> groupby_pesquisa(make_input_vector(extrator_pesquisa.get_output_buffer()),
                                                    group,
                                                    vstrColumnsToAggregate,
                                                    ops, "count_pesquisas");
    manager.addTransformer(&groupby_pesquisa);


    std::vector<Buffer<Dataframe>*> inputs_buffers;
    inputs_buffers.push_back(&preco_medio.get_output_buffer());
    inputs_buffers.push_back(&groupby_pesquisa.get_output_buffer());

    join join_transformer(inputs_buffers, 2);
    manager.addTransformer(&join_transformer);

    TaxaOcupacao taxa_ocupacao(make_input_vector(join_transformer.get_output_buffer()));
    manager.addTransformer(&taxa_ocupacao);

    faturamento fatur(make_input_vector(join_transformer.get_output_buffer()));
    manager.addTransformer(&fatur);

    DataPrinter loader1(taxa_ocupacao.get_output_buffer());
    manager.addLoader(&loader1);

    DataPrinter loader2(fatur.get_output_buffer());
    manager.addLoader(&loader2);

    //Pipeline Voos ------------------------------------------------------------------------
    // Extrator<Dataframe> extrator_voos("./mock/data/dados_voos_2025.csv", "csv", 15000);
    // manager.addExtractor(&extrator_voos);

    // vstrColumnsToAggregate = {"assentos_ocupados", "assentos_totais"};
    // group = {"cidade_destino"};
    // ops = {"sum"};

    // GroupByTransformer<Dataframe> groupbyvoo(
    //     make_input_vector(extrator_voos.get_output_buffer()),
    //     group,
    //     vstrColumnsToAggregate,
    //     ops
    // );

    // manager.addTransformer(&groupbyvoo);

    // taxa_ocup_voo taxaocupvoo(make_input_vector(groupbyvoo.get_output_buffer()));
    // manager.addTransformer(&taxaocupvoo);

    // DataPrinter loader3(taxaocupvoo.get_output_buffer());
    // manager.addLoader(&loader3);

    auto start_time = std::chrono::high_resolution_clock::now();

    // Start the pipeline
    manager.run();

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Tempo de execução: " << duration << " ms" << std::endl;
    cout << "Número de quartos ocupados em toda a base " << filtroocupacao.getStats()[0] << endl;
    cout << "Número de quartos não ocupados em toda a base " << filtroocupacao.getStats()[1] << endl;
    cout << "Número de quartos no Rio de Janeiro " << filtroocupacao.getStats()[2] << endl;
    cout << "Número de quartos em Campo Grande " << filtroocupacao.getStats()[3] << endl;
    // cout << "Número de Cidades Destino Diferente em toda a base " << taxaocupvoo.getStats()[0] << endl;

    // The manager's destructor will clean up everything
    
    return;

}

int main() {
    string strCsvPath1 = "./mock/data/dados_pesquisas_2025.db";
    string strCsvPath2 = "./mock/data/dados_reservas_2025.csv";
    string strCsvPath3 = "./mock/data/dados_voos_2025.csv";

    // Execução programada a cada 10 minutos
    // TimeTrigger timeTrigger(pipeline, 600);
    
    // // Verificação dos arquivos a cada 1 minuto
    // EventTrigger trigger(strCsvPath1, pipeline, 60);
    // EventTrigger trigger2(strCsvPath2, pipeline, 60);
    // EventTrigger trigger3(strCsvPath3, pipeline, 60);

    // // Inicializando a execução dos triggers
    // timeTrigger.start();
    // trigger.start();
    // trigger2.start();
    // trigger3.start();

    // // Rodando o teste por 1 hora
    // this_thread::sleep_for(std::chrono::minutes(60));

    // // Parando os triggers
    // timeTrigger.stop();
    // trigger.stop();
    // trigger2.stop();
    // trigger3.stop();

    pipeline();
}
