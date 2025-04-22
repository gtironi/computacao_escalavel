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
                //df.printHeader(std::cout, df); // Print the dataframe head
                headerPrinted = true;
            }

            // Print the dataframe contents
            //std::cout << df;
        }
    };

void pipeline(){
    // Inicializa o Manager
    Manager<Dataframe> manager(7);

    //Pipeline Voos ------------------------------------------------------------------------
    Extrator<Dataframe> extrator_voos("./mock/data/dados_voos_2025.csv", "csv", 1000);
    manager.addExtractor(&extrator_voos);

    std::vector<std::string> vstrColumnsToAggregate = {"assentos_ocupados", "assentos_totais"};
    std::vector<string> group = {"cidade_destino"};
    std::vector<string> ops = {"sum"};

    GroupByTransformer<Dataframe> groupbyvoo(
        make_input_vector(extrator_voos.get_output_buffer()),
        group,
        vstrColumnsToAggregate,
        ops
    );

    manager.addTransformer(&groupbyvoo);

    taxa_ocup_voo taxaocupvoo(make_input_vector(groupbyvoo.get_output_buffer()));
    manager.addTransformer(&taxaocupvoo);

    DataPrinter loader3(taxaocupvoo.get_output_buffer());
    manager.addLoader(&loader3);

    auto start_time = std::chrono::high_resolution_clock::now();

    // Start the pipeline
    manager.run();

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Tempo de execução: " << duration << " ms" << std::endl;
}

int main() {
    string strFilePath = "./mock/data/dados_voos_2025.csv";

    TimeTrigger trigger(pipeline, 600);
    EventTrigger eventTrigger(strFilePath, pipeline, 60);

    trigger.start();
    eventTrigger.start();

    std::this_thread::sleep_for(std::chrono::minutes(60));

    trigger.stop();
    eventTrigger.stop();
    return 0;
}
