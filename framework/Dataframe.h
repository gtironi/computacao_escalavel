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
};

#endif