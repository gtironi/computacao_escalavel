#ifndef DATAFRAME_H
#define DATAFRAME_H
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <variant>

using namespace std;

#define DTYPES int, double, string, bool, char
#define VDTYPES variant<DTYPES>

/**
 * @brief Mapeia os tipos de dados para suas representações em string.
 */
const map<string, string> TYPEMAP = {
    {typeid(int).name(), "int"},
    {typeid(double).name(), "double"},
    {typeid(string).name(), "string"},
    {typeid(bool).name(), "bool"},
    {typeid(char).name(), "char"}
};

/**
 * @class Series
 * @brief Representa uma coluna de um DataFrame, contendo um nome, um tipo e um vetor de dados.
 */
class Series {
public:
    string strColumnName;  ///< Nome da coluna
    string strColumnType;  ///< Tipo da coluna
    vector<VDTYPES> vecColumnData; ///< Dados armazenados na coluna

    /**
     * @brief Construtor da classe Series
     * @param columnName Nome da coluna
     * @param columnType Tipo da coluna
     */
    Series(const string& columnName, const string& columnType) : strColumnName(columnName), strColumnType(columnType) {}

    /**
     * @brief Adiciona um elemento à coluna
     * @param elemento Elemento a ser adicionado
     * @return true se a operação for bem-sucedida, false caso contrário
     */
    bool bAdicionaElemento(const VDTYPES& elemento) {
        try {
            vecColumnData.push_back(elemento);
            return true;
        } catch (...) {
            cerr << "Falha ao adicionar elemento." << endl;
            return false;
        }
    }

    /**
     * @brief Remove o último elemento da coluna
     * @return true se a remoção for bem-sucedida, false caso contrário
     */
    bool bRemoveUltimoElemento() {
        if (!vecColumnData.empty()) {
            vecColumnData.pop_back();
            return true;
        } else {
            cerr << "Não há elementos para remover." << endl;
            return false;
        }
    }

    /**
     * @brief Retorna um elemento da coluna com base no índice fornecido
     * @param iIndex Índice do elemento
     * @return Elemento armazenado no índice especificado
     */
    VDTYPES retornaElemento(int iIndex) {
        if (iIndex >= 0 && iIndex < vecColumnData.size()) {
            return vecColumnData[iIndex];
        } else {
            cerr << "Índice fora dos limites." << endl;
            return VDTYPES{};
        }
    }

    /**
     * @brief Exibe os dados da coluna no console
     */
    void printColuna() {
        cout << "Column Name: " << strColumnName << "\n";
        cout << "Column Type: " << strColumnType << "\n";
        cout << "Data: ";
        for (const auto& value : vecColumnData) {
            visit([](const auto& val) { cout << val << " "; }, value);
        }
        cout << "\n";
    }
};

#endif
