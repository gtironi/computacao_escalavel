#include "Dataframe.h"
#include "Series.h"

int main() {
    // Criando colunas de diferentes tipos
    Series intColumn("Idade", "int");
    Series strColumn("Nome", "string");
    Series boolColumn("Ativo", "bool");

    // Adicionando elementos às colunas
    intColumn.bAdicionaElemento(25);
    intColumn.bAdicionaElemento(30);
    intColumn.bAdicionaElemento(22);
    intColumn.bRemovePeloIndex(0);

    strColumn.bAdicionaElemento("Alice");
    strColumn.bAdicionaElemento("Bob");
    strColumn.bAdicionaElemento("Charlie");

    boolColumn.bAdicionaElemento(true);
    boolColumn.bAdicionaElemento(false);
    boolColumn.bAdicionaElemento(true);

    // Criando um DataFrame
    Dataframe df;
    df.addSeries(intColumn);
    df.addSeries(strColumn);
    df.addSeries(boolColumn);

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

    return 0;
}
