#ifndef DATAFRAME_HPP
#define DATAFRAME_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include "Series.h"

using namespace std;

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
    
};

#endif