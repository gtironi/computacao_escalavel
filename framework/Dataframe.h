#ifndef DATAFRAME_HPP
#define DATAFRAME_HPP

#include <iostream>
#include <string>
#include <vector>
#include <any>
#include <algorithm>
#include <iomanip>
#include "Series.h"
using namespace std;

static inline string anyToString(const any& value) {
    try {
        if (value.type() == typeid(int))      return to_string(any_cast<int>(value));
        if (value.type() == typeid(double))   return to_string(any_cast<double>(value));
        if (value.type() == typeid(bool))     return any_cast<bool>(value) ? "true" : "false";
        if (value.type() == typeid(string))   return any_cast<string>(value);
    } catch (...) {
        return "[err]";
    }
    return "[unknown]";
}

/**
 * @class Dataframe
 * @brief Representa um conjunto de colunas (Series) organizadas como um DataFrame.
 */
class Dataframe {
public:
    vector<string> vstrColumnsName;
    vector<Series> columns;

    Dataframe() = default;
    Dataframe(const Dataframe& other) = default;
    Dataframe& operator=(const Dataframe& other) = default;

    bool adicionaColuna(const Series& novaSerie) {
        if (columns.empty()) {
            vstrColumnsName.push_back(novaSerie.strGetName());
            columns.push_back(novaSerie);
            return true;
        }
        size_t sizeNeeded = columns[0].iGetSize();
        if (novaSerie.iGetSize() != sizeNeeded) {
            cerr << "Erro: A nova coluna possui tamanho diferente das colunas existentes." << endl;
            return false;
        }
        vstrColumnsName.push_back(novaSerie.strGetName());
        columns.push_back(novaSerie);
        return true;
    }

    pair<int, int> getShape() const {
        int iNumColunas = static_cast<int>(vstrColumnsName.size());
        int iNumLinhas  = columns.empty() ? 0 : static_cast<int>(columns[0].iGetSize());
        return make_pair(iNumLinhas, iNumColunas);
    }

    bool adicionaLinha(const vector<any>& novaLinha) {
        if (novaLinha.size() != columns.size()) {
            cerr << "Número de valores não corresponde ao número de colunas.\n";
            return false;
        }
        for (size_t i = 0; i < columns.size(); i++) {
            columns[i].bAdicionaElemento(novaLinha[i]);
        }
        return true;
    }

    bool removeLinha(int iIndex) {
        auto shape = getShape();
        if (iIndex < 0 || iIndex >= shape.first) {
            cerr << "Índice inválido para remoção de linha." << endl;
            return false;
        }
        for (auto& col : columns) {
            col.bRemovePeloIndex(iIndex);
        }
        return true;
    }

    Dataframe filtroByValue(const string& strNomeColuna, const any& valor) {
        Dataframe auxDf;
        auxDf.vstrColumnsName = vstrColumnsName;
        for (auto& col : columns) {
            auxDf.columns.emplace_back(col.strGetName(), col.strGetType());
        }
        auto it = find(vstrColumnsName.begin(), vstrColumnsName.end(), strNomeColuna);
        if (it == vstrColumnsName.end()) {
            cerr << "Coluna '" << strNomeColuna << "' não encontrada." << endl;
            return auxDf;
        }
        int colIndex = static_cast<int>(distance(vstrColumnsName.begin(), it));
        const int total = getShape().first;
        // Pré-conta quantas linhas
        size_t matchCount = 0;
        for (int i = 0; i < total; ++i) {
            any cellVal = columns[colIndex].retornaElemento(i);
            if (cellVal.type() == valor.type() && cellVal.has_value() && valor.has_value()) {
                if (anyToString(cellVal) == anyToString(valor)) matchCount++;
            }
        }
        for (auto& col : auxDf.columns) col.reserve(matchCount);

        for (int i = 0; i < total; ++i) {
            any cellVal = columns[colIndex].retornaElemento(i);
            if (cellVal.type() == valor.type() && cellVal.has_value() && valor.has_value()) {
                if (anyToString(cellVal) == anyToString(valor)) {
                    for (size_t j = 0; j < columns.size(); ++j) {
                        auxDf.columns[j].bAdicionaElemento(columns[j].retornaElemento(i));
                    }
                }
            }
        }
        return auxDf;
    }

    void hStack(Dataframe& other) {
        if (columns.size() != other.columns.size()) {
            cerr << "DataFrames com tamanhos diferentes. Não é possível empilhar." << endl;
            return;
        }
        for (size_t i = 0; i < columns.size(); i++) {
            int total = other.getShape().first;
            for (int row = 0; row < total; ++row) {
                columns[i].bAdicionaElemento(other.columns[i].retornaElemento(row));
            }
        }
    }

    friend ostream& operator<<(ostream& os, const Dataframe& dfInput) {
        if (dfInput.columns.empty()) {
            os << "[Empty DataFrame]\n";
            return os;
        }
        auto shape = dfInput.getShape();
        size_t num_rows = shape.first, num_cols = shape.second;
        int col_width = 15, index_width = 5;

        // Imprime cabeçalho
        os << left << setw(index_width) << "" << "  ";
        for (auto& name : dfInput.vstrColumnsName) {
            string header_name = (name.size() > size_t(col_width))
                ? (name.substr(0, col_width - 3) + "...")
                : name;
            os << left << setw(col_width) << header_name << "  ";
        }
        os << "\n" << left << setw(index_width) << "" << "  ";
        for (size_t i = 0; i < num_cols; i++) {
            os << left << setw(col_width) << string(col_width, '-') << "  ";
        }
        os << "\n";

        // Imprime dados
        for (size_t i = 0; i < num_rows; ++i) {
            os << left << setw(index_width) << i << "  ";
            for (size_t j = 0; j < num_cols; ++j) {
                string val_str = anyToString(dfInput.columns[j].retornaElemento(i));
                if (val_str.length() > size_t(col_width))
                    val_str = val_str.substr(0, col_width - 3) + "...";
                os << left << setw(col_width) << val_str << "  ";
            }
            os << "\n";
        }
        return os;
    }
};

#endif
