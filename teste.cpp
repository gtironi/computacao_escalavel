#include "framework/BaseClasses.h"
#include "framework/Manager.h"
#include "framework/Dataframe.h"
#include <iostream>

class DataPrinter : public Loader<Dataframe> {
public:
    using Loader::Loader;
    void run(Dataframe df) override {
        // std::cout << df;
    }
};

int main() {
    Manager<Dataframe> manager(1);

    Extrator<Dataframe> extrator_viagens("./mock/data/dados_pesquisa_2025.db", "sql", 1000, "Viagens");
    manager.addExtractor(&extrator_viagens);

    DataPrinter printer(extrator_viagens.get_output_buffer());
    manager.addLoader(&printer);

    manager.run();

    return 0;
}
