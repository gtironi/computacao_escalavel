#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "BaseClasses.h"

using namespace std;

string lerCSVComoString(const string& caminhoArquivo) {
    ifstream arquivo(caminhoArquivo);
    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << caminhoArquivo << endl;
        return "";
    }

    string linha;
    string conteudo;

    int contador = 0;
    while (getline(arquivo, linha)) {
        if (contador == 0) {
            contador++;
            continue; 
        }
        conteudo += linha + "\n";  // Adiciona a linha lida e quebra de linha
        contador++;
    }

    return conteudo;
}

string division(string str1, string str2){
    double num1 = stod(str1);
    double num2 = stod(str2);
    if (num2 == 0) {
        return "Erro: Divisão por zero";
    }
    return to_string(num1 / num2);
}

int main() {
    string caminho = "/home/matheus-carvalho/Projetos/Faculdade/computacao_escalavel/mock/data/dados_hoteis_2025.csv";
    string conteudoCSV = lerCSVComoString(caminho);

    Extrator<Dataframe> ex (caminho, "csv", 1000);
    Dataframe df = ex.run(conteudoCSV);

    cout << "Shape do DataFrame: " << endl;
    cout << df.getShape().first << " " << df.getShape().second << endl;

    cout << "Columnas do DataFrame: " << endl;
    for (const auto& col : df.vstrColumnsName) {
        cout << col << endl;
    }

    Dataframe dfAux = df.dfGroupby("nome_hotel", "sum", {"preco"}, true);
    
    dfAux.bColumnOperation("preco", "count", division, "preco_dividido");

    cout << dfAux << endl;

    return 0;
}
