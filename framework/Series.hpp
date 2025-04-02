#ifndef DATAFRAME_HPP
#define DATAFRAME_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <typeinfo>

using namespace std;

#define DTYPES int, double, string, bool, char
#define VDTYPES variant<DTYPES>

const map<string, string> TYPEMAP = {
    {typeid(int).name(), "int"},
    {typeid(double).name(), "double"},
    {typeid(string).name(), "string"},
    {typeid(bool).name(), "bool"},
    {typeid(char).name(), "char"}
};

class Serie {
public:
    string strColumnName;
    string strColumnType;
    vector<VDTYPES> vecColumnData;

    // método construtor
    Serie(const string& columnName, const string& columnType) : strColumnName(columnName), strColumnType(columnType) {}

    // adicionar elemento
    bool bAdicionaElemento(const VDTYPES& elemento) {
        try {
            vecColumnData.push_back(elemento);
            return true;
        } catch (...) {
            cout << "Falha ao adicionar elemento." << endl;
            return false;
        }
    }

    // remover elemento
    bool bRemoveUltimoElemento() {
        if (!vecColumnData.empty()) {
            vecColumnData.pop_back();
            return true;
        } else {
            cerr << "Não há elementos para remover." << endl;
            return false;
        }
    }

    // ordena coluna

    // retorna elemento
    VDTYPES retornaElemento(int iIndex) {
        if (iIndex >= 0 && iIndex < vecColumnData.size()) {
            return vecColumnData[iIndex];
        } else {
            cerr << "Índice fora dos limites." << endl;
            return VDTYPES{};
        }
    }

    // print
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