#include "Dataframe.h"
#include "Series.h"
#include "Extrator.h"

int main() {
    /*
    // Criando colunas de diferentes tipos
    Series intColumn("Idade", "int");
    Series strColumn("Nome", "string");
    Series boolColumn("Ativo", "bool");
    //Series testColumn("Teste", "int");

    // Adicionando elementos às colunas
    intColumn.bAdicionaElemento(25);
    intColumn.bAdicionaElemento(30);
    intColumn.bAdicionaElemento(22);
    // intColumn.bRemovePeloIndex(0);

    strColumn.bAdicionaElemento("Alice");
    strColumn.bAdicionaElemento("Bob");
    strColumn.bAdicionaElemento("Charlie");

    boolColumn.bAdicionaElemento(true);
    boolColumn.bAdicionaElemento(false);
    boolColumn.bAdicionaElemento(true);

    //testColumn.bAdicionaElemento(1);
    //testColumn.bAdicionaElemento(2);
    //testColumn.bAdicionaElemento(3);
    //testColumn.bAdicionaElemento(4);

    // Criando um DataFrame
    Dataframe df;
    df.adicionaColuna(intColumn);
    df.adicionaColuna(strColumn);
    df.adicionaColuna(boolColumn);
    // df.addColumn(testColumn);

    // Adicionando uma linha
    vector<VDTYPES> novaLinha = {20, "Matheus", true};
    df.adicionaLinha(novaLinha);

    // Removendo uma linha
    df.removeLinha(3);

    // Exibindo informações
    cout << "Shape do DataFrame (Linhas, Colunas): ";
    auto shape = df.getShape();
    cout << "(" << shape.first << ", " << shape.second << ")" << endl;

    // Imprimindo colunas
    cout << "\nDados das colunas:" << endl;
    for (auto &col : df.columns) {
        col.printColuna();
    }

    // Imprimindo as Médias
    cout << "Média das colunas:" << endl;
    for (auto &col : df.columns) {
        cout << col.mean() << endl;
    }
    */

    ExtratorSQL extrator("../database.db");
    extrator.ExtratorColunas("Funcionarios");
    Dataframe df = extrator.ConstrutorDataframe("Funcionarios");

    cout << "Shape do DataFrame (Linhas, Colunas): ";
    auto shape = df.getShape();
    cout << "(" << shape.first << ", " << shape.second << ")" << endl;

    cout << "\nDados das colunas:" << endl;
    for (auto &col : df.columns) {
        col.printColuna();
    }

    return 0;
}
