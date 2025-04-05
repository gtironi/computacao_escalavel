#ifndef DATAFRAME_HPP
#define DATAFRAME_HPP

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <map>
#include <variant>
#include "Series.h"

using namespace std;

string variantToString(const VDTYPES& value) {
    std::stringstream ss;
    std::visit([&ss](auto&& arg) {
        // Special handling for strings to potentially add quotes if desired,
        // but for width calculation, raw string is better.
        // For bool, print "true"/"false" instead of 1/0
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, bool>) {
            ss << (arg ? "true" : "false");
        } else {
            ss << arg; // Use existing operator<< for DateDay, DateTime, etc.
        }
    }, value);
    return ss.str();
}

/**
 * @class Dataframe
 * @brief Representa um conjunto de colunas (Series) organizadas como um DataFrame.
 */
class Dataframe {
public:
    vector<string> vstrColumnsName; ///< Vetor que armazena os nomes das colunas
    vector<Series> columns;        ///< Vetor que armazena as colunas do DataFrame

    /**
     * @brief Construtor padrão do DataFrame.
     */
    Dataframe() = default;

    /**
     * @brief Construtor de cópia do DataFrame.
     * @param other Objeto Dataframe a ser copiado.
     */
    Dataframe(const Dataframe& other) {
        vstrColumnsName = other.vstrColumnsName;
        columns = other.columns;
    }

    /**
     * @brief Sobrecarga do operador de atribuição.
     * @param other Objeto Dataframe a ser atribuído.
     * @return Referência para o objeto atribuído.
     */
    Dataframe& operator=(const Dataframe& other) {
        if (this != &other) {
            vstrColumnsName = other.vstrColumnsName;
            columns = other.columns;
        }
        return *this;
    }

    /**
     * @brief Adiciona uma nova coluna (Series) ao DataFrame.
     * @param novaSerie Objeto Series a ser adicionado.
     * @return true se a adição for bem-sucedida, false caso contrário.
     */
    bool adicionaColuna(Series novaSerie) {
        
        if (columns.empty()) {
            vstrColumnsName.push_back(novaSerie.strGetName());
            columns.push_back(novaSerie);
            return true;

        } else {
            bool viavel = true;

            
            for (size_t i = 0; i < columns.size(); i++) {
                if (novaSerie.iGetSize() != columns[i].iGetSize()) {  
                    viavel = false;
                    break; 
                }
            }

            if (viavel) {
                vstrColumnsName.push_back(novaSerie.strGetName());
                columns.push_back(novaSerie);
                return true;
            } else {
                cerr << "Erro: A nova coluna possui tamanho diferente das colunas existentes." << endl;
                return false;
            }
        }
    }

    /**
     * @brief Retorna o número de linhas e colunas do DataFrame.
     * @return Um par contendo (número de linhas, número de colunas).
     */
    pair<int, int> getShape() {
        int iNumColunas = vstrColumnsName.size();
        int iNumLinhas = columns.empty() ? 0 : columns[0].iGetSize();
        pair<int, int> shape = {iNumLinhas, iNumColunas};
        return shape;
    }

    /**
     * @brief Adiciona uma nova linha ao DataFrame, inserindo os valores em cada coluna correspondente.
     * @tparam Args Tipos dos valores a serem adicionados.
     * @param args Valores a serem adicionados como uma nova linha.
     * @return true se a linha for adicionada com sucesso, false caso contrário.
     */
    bool adicionaLinha(vector<VDTYPES> novaLinha) {
        if (novaLinha.size() != columns.size()) {
            cout << "Datapoint com tamanho inválido. É: " << novaLinha.size() << " - Deveria ser: " << columns.size() << endl;
            return false;
        }

        for (size_t i = 0; i < columns.size(); i++) {
            string expectedType = columns[i].strGetType();
            bool valid = visit([&expectedType](const auto& val) -> bool {
                auto it = TYPEMAP.find(typeid(val).name());
                if (it == TYPEMAP.end()) {
                    return false;
                }
                return it->second == expectedType;
            }, novaLinha[i]);
            if (!valid) {
                cerr << "Tipo de dado inválido para a coluna " << columns[i].strGetName() << ". Esperado: " << expectedType << endl;
                return false;
            }
        }

        for (size_t i = 0; i < columns.size(); i++) {
            columns[i].bAdicionaElemento(novaLinha[i]);
        }

        return true;
    }

    /**
     * @brief Remove uma linha específica do DataFrame, removendo o elemento correspondente de cada coluna.
     * @param iIndex O índice da linha a ser removida.
     * @return true se a remoção for bem-sucedida, false caso contrário.
     */
    bool removeLinha(int iIndex) {
        if (iIndex < 0 || iIndex >= this->getShape().first) {
            cerr << "Índice inválido para remoção de linha." << endl;
            return false;
        }

        for (size_t i = 0; i < columns.size(); i++) {
            columns[i].bRemovePeloIndex(iIndex);
        }

        return true;
    }

    /**
     * @brief Filtra o DataFrame com base em um valor específico de uma coluna.
     * 
     * @param strNomeColuna O nome da coluna na qual a filtragem será aplicada.
     * @param valor O valor a ser buscado na coluna.
     * @return Um novo DataFrame contendo apenas as linhas onde a correspondência ocorreu.
     */
    Dataframe filtroByValue(const string& strNomeColuna, const VDTYPES& valor) {
        Dataframe auxDf; 
        auxDf.vstrColumnsName = vstrColumnsName;
        
        for (auto col : columns){
            Series auxSerie(col.strGetName(), col.strGetType());
            auxDf.columns.push_back(auxSerie);
        }
        
        vector<int> index; 
        
        auto it = find(vstrColumnsName.begin(), vstrColumnsName.end(), strNomeColuna);
        if (it == vstrColumnsName.end()) {
            cerr << "Erro: Coluna '" << strNomeColuna << "' não encontrada." << endl;
            return auxDf;
        }
        
        int colIndex = distance(vstrColumnsName.begin(), it);
        
        for (size_t i = 0; i < columns[colIndex].getData().size(); i++) {
            
            if (columns[colIndex].getData()[i] == valor) {
                index.push_back(i);
            }
        }

        for (int i : index) {
            vector<VDTYPES> auxRow;
            for (auto& col : columns) {
                if (i < col.getData().size()) {
                    auxRow.push_back(col.getData()[i]);
                }
            }
            auxDf.adicionaLinha(auxRow);
        }

        return auxDf;
    }

    /**
     * @brief Empilha dois DataFrames horizontalmente, desde que tenham o mesmo número de colunas e tipos de colunas.
     * @param other O DataFrame a ser empilhado.
     */
    void hStack(Dataframe& other) {
        if (this->columns.size() != other.columns.size()) {
            cout << "DataFrames com tamanhos diferentes. Não é possível empilhar." << endl;
            return;
        }

        for (size_t i = 0; i < columns.size(); i++) {
            if (columns[i].strGetType() != other.columns[i].strGetType()) {
                cout << "Tipos de colunas diferentes. Não é possível empilhar." << endl;
                return;
            }
        }

        for (size_t i = 0; i < columns.size(); i++) {
            columns[i].hStack(other.columns[i]);
        }
    }
    
    /**
     * @brief Sobrecarga do operador de saída para imprimir o DataFrame.
     * @param os O fluxo de saída.
     * @param dfInput O DataFrame a ser impresso.
     * @return O fluxo de saída atualizado.
     */
    friend ostream& operator<<(ostream& os, const Dataframe& dfInput) {
        Dataframe df = dfInput;
        if (df.columns.empty()) {
            os << "[Empty DataFrame]\n";
            return os;
        }
        
        pair<int, int> shape = df.getShape();

        size_t num_rows = shape.first;
        size_t num_cols = shape.second;
        int col_width = 15; // Largura fixa das colunas
        int index_width = 5;
    
        os << left << setw(index_width) << "" << "  ";
        for (const auto& name : df.vstrColumnsName) {
            string header_name = name.substr(0, col_width);
            if (name.length() > col_width) header_name[col_width - 3] = header_name[col_width - 2] = header_name[col_width - 1] = '.';
            os << left << setw(col_width) << header_name << "  ";
        }
        os << "\n";
        os << left << setw(index_width) << "" << "  ";
        for (size_t i = 0; i < num_cols; ++i) {
            os << left << setw(col_width) << "------------" << "  ";
        }
        os << "\n";
    
        size_t max_display_rows = 10;
        bool truncate = num_rows > max_display_rows;
    
        for (size_t i = 0; i < num_rows; ++i) {
            if (truncate && i == 5) {
                os << left << setw(index_width) << "..." << "  ";
                for (size_t j = 0; j < num_cols; ++j) {
                    os << left << setw(col_width) << "..." << "  ";
                }
                os << "\n";
                i = num_rows - 1; // Pula para a última linha
            }
    
            os << left << setw(index_width) << i << "  ";
            for (size_t j = 0; j < num_cols; ++j) {
                string val_str = variantToString(df.columns[j].retornaElemento(i));
                if (val_str.length() > col_width) val_str[col_width - 3] = val_str[col_width - 2] = val_str[col_width - 1] = '.';
                os << left << setw(col_width) << val_str.substr(0, col_width) << "  ";
            }
            os << "\n";
        }
    
        os << "\n[" << num_rows << " rows x " << num_cols << " columns]\n";
        return os;
    }
};

#endif