#include "Dataframe.h"
#include "Series.h"
#include "Extrator.h"

int main() {

    // Criando colunas de diferentes tipos
    // Series intColumn("Idade", "int");
    // Series strColumn("Nome", "string");
    // Series boolColumn("Ativo", "bool");
    // //Series testColumn("Teste", "int");

    // // // Adicionando elementos às colunas
    // intColumn.bAdicionaElemento(25);
    // intColumn.bAdicionaElemento(30);
    // intColumn.bAdicionaElemento(22);
    // // intColumn.bRemovePeloIndex(0);

    // strColumn.bAdicionaElemento("Alice");
    // strColumn.bAdicionaElemento("Bob");
    // strColumn.bAdicionaElemento("Charlie");

    // boolColumn.bAdicionaElemento(true);
    // boolColumn.bAdicionaElemento(false);
    // boolColumn.bAdicionaElemento(true);

    //testColumn.bAdicionaElemento(1);
    //testColumn.bAdicionaElemento(2);
    //testColumn.bAdicionaElemento(3);
    //testColumn.bAdicionaElemento(4);

    // // Criando um DataFrame
    // Dataframe df;
    // df.adicionaColuna(intColumn);
    // df.adicionaColuna(strColumn);
    // df.adicionaColuna(boolColumn);
    // // df.addColumn(testColumn);

    // // Adicionando uma linha
    // vector<VDTYPES> novaLinha = {20, "Matheus", true};
    // df.adicionaLinha(novaLinha);

    // // Removendo uma linha
    // df.removeLinha(1);

    // // Exibindo informações
    // cout << "Shape do DataFrame (Linhas, Colunas): ";
    // auto shape = df.getShape();
    // cout << "(" << shape.first << ", " << shape.second << ")" << endl;

    // // Imprimindo colunas
    // cout << "\nDados das colunas:" << endl;
    // for (auto &col : df.columns) {
    //     col.printColuna();
    // }

    // // Imprimindo as Médias
    // cout << "Média das colunas:" << endl;
    // for (auto &col : df.columns) {
    //     cout << col.mean() << endl;
    // }

    // ExtratorSQL extrator("../database.db");
    // extrator.ExtratorColunas("Funcionarios");
    // extrator.ConstrutorDataframe("Funcionarios");

    // cout << "Shape do DataFrame (Linhas, Colunas): ";
    // auto shape = extrator.getDataframe().getShape();
    // cout << "(" << shape.first << ", " << shape.second << ")" << endl;

    // cout << "\nDados das colunas:" << endl;
    // for (auto &col : extrator.getDataframe().columns) {
    //     cout << col.strGetName() << " - " << col.strGetType() << " - " << col.mean() << endl;
    // }

    // Dataframe df = extrator.getDataframe();
    // Dataframe df_2 = df.filtroByValue("salario", 1500);

    // cout << "Shape do DataFrame (Linhas, Colunas): ";
    // auto shape2 = df_2.getShape();
    // cout << "(" << shape2.first << ", " << shape2.second << ")" << endl;

    /*
    ExtratorCSV extrator("../dadosfake.csv");
    extrator.ExtratorColunas();
    extrator.ConstrutorDataframe();
    Dataframe df = extrator.getDataframe();
    cout << "Shape do DataFrame (Linhas, Colunas): ";
    auto shape = df.getShape();
    cout << "(" << shape.first << ", " << shape.second << ")" << endl;
    cout << "\nDados das colunas:" << endl;
    for (auto &col : df.columns) {
        cout << col.strGetName() << " - " << col.strGetType() << " - " << col.mean() << endl;
    }
    */


    // // Teste dos Métodos Threads-friend CSV
    // ExtratorCSV extrator("../hotel_bookings.csv");
    // extrator.ExtratorThreads(1000);

    // Dataframe df = extrator.vctDataframes[0];
    // cout << "Shape do DataFrame (Linhas, Colunas): ";
    // auto shape = df.getShape();
    // cout << "(" << shape.first << ", " << shape.second << ")" << endl;

    // cout << "\nDados das colunas:" << endl;
    // for (auto &col : df.columns) {
    //     cout << "Nome coluna: " << col.strGetName() << " - Tipo coluna: " << col.strGetType() << endl;
    // }

    // cout << "\nDados do DataFrame:" << endl;
    // cout << df << endl;


    // Teste dos Métodos Threads-friend SQL
    ExtratorSQL extratorSQL("../database.db");
    extratorSQL.ExtratorThreads(3, "Clientes");

    Dataframe dfSQL = extratorSQL.vctDataframes[0];
    cout << "Shape do DataFrame (Linhas, Colunas): ";
    auto shapeSQL = dfSQL.getShape();
    cout << "(" << shapeSQL.first << ", " << shapeSQL.second << ")" << endl;

    cout << "\nDados do DataFrame:" << endl;
    cout << dfSQL << endl;

    return 0;
}
