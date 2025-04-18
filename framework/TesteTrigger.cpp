#include <iostream>
#include "Triggers.h"

using namespace std;

void printMessage() {
    cout << "Hello, world!" << endl;
}

int main() {
    string filePathCSV = "/home/matheus-carvalho/Projetos/Faculdade/computacao_escalavel/mock/data/dados_viagens_2025.csv";
    string filePathSQL = "/home/matheus-carvalho/Projetos/Faculdade/computacao_escalavel/mock/data/dados_viagens_2025.db";
    
    // testando os Triggers Juntos
    TimeTrigger timeTrigger(printMessage, 60); // executa a cada 1 minutos de qualquer forma
    EventTrigger trigger(filePathCSV, printMessage, 10); // a cada 10 segundos, verifica o arquivo, e eventualmente executa a função
    EventTrigger trigger2(filePathSQL, printMessage, 10); // a cada 10 segundos, verifica o arquivo, e eventualmente executa a função

    // inicializando a execução dos triggers
    timeTrigger.start();
    trigger.start();
    trigger2.start();

    // rodando o teste por 5 minutos
    this_thread::sleep_for(std::chrono::minutes(5));

    // parando os triggers
    timeTrigger.stop();
    trigger.stop();
    trigger2.stop();

    return 0;
}