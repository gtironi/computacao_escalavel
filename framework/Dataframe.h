#ifndef DATAFRAME_HPP
#define DATAFRAME_HPP

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <map>
#include <any>
#include <unordered_map>
#include "Series.h"

using namespace std;

static inline string anyToString(const any& value) {
    try {
        if (value.type() == typeid(string)) {
            return any_cast<string>(value);
        } else if (value.type() == typeid(bool)) {
            return any_cast<bool>(value) ? "true" : "false";
        } else if (value.type() == typeid(int)) {
            return to_string(any_cast<int>(value));
        } else if (value.type() == typeid(double)) {
            return to_string(any_cast<double>(value));
        } else {
            return "[Unsupported Type]";
        }
    } catch (const bad_any_cast&) {
        return "[Error]";
    }
}

/**
 * @class Dataframe
 * @brief Representa um conjunto de colunas (Series) organizadas como um DataFrame.
 */
class Dataframe {
public:
    vector<string> vstrColumnsName; ///< Vetor que armazena os nomes das colunas
    vector<Series<any>> columns; ///< Vetor que armazena as colunas do DataFrame

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
     * @brief Construtor de movimento do DataFrame.
     * @param other Objeto Dataframe a ser movido.
     */
    Dataframe(Dataframe&& other) noexcept {
        vstrColumnsName = std::move(other.vstrColumnsName);
        columns = std::move(other.columns);
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
     * @brief Sobrecarga do operador de atribuição por movimento.
     * @param other Objeto Dataframe a ser movido.
     * @return Referência para o objeto atribuído.
     */
    Dataframe& operator=(Dataframe&& other) noexcept {
        if (this != &other) {
            vstrColumnsName = std::move(other.vstrColumnsName);
            columns = std::move(other.columns);
        }
        return *this;
    }

    /**
     * @brief Adiciona uma nova coluna (Series) ao DataFrame.
     * @param novaSerie Objeto Series a ser adicionado.
     * @return true se a adição for bem-sucedida, false caso contrário.
     */
    bool adicionaColuna(Series<any> novaSerie) {

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
     * @brief Adiciona uma nova coluna genérica ao DataFrame.
     * @tparam T Tipo dos dados na Series
     * @param novaSerie Objeto Series<T> a ser adicionado.
     * @return true se a adição for bem-sucedida, false caso contrário.
     */
    template <typename T>
    bool adicionaColuna(const Series<T>& novaSerie) {
        // Converter Series<T> para Series<any>
        Series<any> serieConvertida(novaSerie.strGetName(), novaSerie.strGetType());
        
        for (size_t i = 0; i < novaSerie.iGetSize(); i++) {
            try {
                serieConvertida.bAdicionaElemento(novaSerie.retornaElemento(i));
            } catch (const exception& e) {
                cerr << "Erro ao converter elemento: " << e.what() << endl;
                return false;
            }
        }
        
        return adicionaColuna(serieConvertida);
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

    pair<int, int> getShape() const {
        int iNumColunas = vstrColumnsName.size();
        int iNumLinhas = columns.empty() ? 0 : columns[0].iGetSize();
        pair<int, int> shape = {iNumLinhas, iNumColunas};
        return shape;
    }

    /**
     * @brief Adiciona uma nova linha ao DataFrame, inserindo os valores em cada coluna correspondente.
     * @param novaLinha Vetor de valores para adicionar como uma nova linha.
     * @return true se a linha for adicionada com sucesso, false caso contrário.
     */
    bool adicionaLinha(const vector<any>& novaLinha) {
        if (novaLinha.size() != columns.size()) {
            cout << "Datapoint com tamanho inválido. É: " << novaLinha.size() << " - Deveria ser: " << columns.size() << endl;
            return false;
        }

        for (size_t i = 0; i < columns.size(); i++) {
            string expectedType = columns[i].strGetType();
            bool valid = true;
            try {
                if (anyToString(novaLinha[i]).empty()) {
                    valid = false;
                }
            } catch (const bad_any_cast&) {
                valid = false;
            }
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
     * @brief Remove uma linha específica do DataFrame.
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
     * @param strNomeColuna O nome da coluna na qual a filtragem será aplicada.
     * @param valor O valor a ser buscado na coluna.
     * @return Um novo DataFrame contendo apenas as linhas onde a correspondência ocorreu.
     */
    Dataframe filtroByValue(const string& strNomeColuna, const any& valor) {
        Dataframe auxDf;
        auxDf.vstrColumnsName = vstrColumnsName;
    
        // Criar estrutura das colunas no novo dataframe
        for (const auto& col : columns) {
            auxDf.columns.emplace_back(col.strGetName(), col.strGetType());
        }
    
        // Localiza a coluna de filtro
        auto it = find(vstrColumnsName.begin(), vstrColumnsName.end(), strNomeColuna);
        if (it == vstrColumnsName.end()) {
            throw std::invalid_argument("Coluna '" + strNomeColuna + "' não encontrada.");
        }
    
        int colIndex = distance(vstrColumnsName.begin(), it);
        const auto& data = columns[colIndex].getData();
    
        // Pré-conta quantas linhas irão para o novo DataFrame
        size_t matchCount = count_if(data.begin(), data.end(), [&](const auto& el) {
            return anyToString(el) == anyToString(valor);
        });
    
        // Pré-aloca espaço nas colunas do novo dataframe
        for (auto& col : auxDf.columns) {
            col.reserve(matchCount);
        }
    
        // Copia os dados filtrados diretamente
        for (size_t i = 0; i < data.size(); ++i) {
            if (anyToString(data[i]) == anyToString(valor)) {
                for (size_t j = 0; j < columns.size(); ++j) {
                    auxDf.columns[j].bAdicionaElemento(columns[j].retornaElemento(i));
                }
            }
        }
    
        return auxDf;
    }

    /**
     * @brief Empilha dois DataFrames horizontalmente.
     * @param other O DataFrame a ser empilhado.
     */
    void hStack(Dataframe& other) {
        if (this->columns.empty()) {
            // Copia os nomes das colunas e os dados do outro DataFrame
            this->vstrColumnsName = other.vstrColumnsName;
            this->columns = other.columns;
            return;
        }

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

    Dataframe dfGroupby(const string& strNomeColuna, const string& strAggMethod = "sum", vector<string> vstrColumnsToAggregate = {}) {
        Dataframe dfAgrupado;
        vector<int> viIndexColumnsToAggregate;
        int iIndexMainColumn = -1;

        // Primeiro passo: verificar se a coluna de agrupamento existe e atualizar o indice
        auto it = find(vstrColumnsName.begin(), vstrColumnsName.end(), strNomeColuna);
        if (it == vstrColumnsName.end()) {
            throw std::invalid_argument("Coluna '" + strNomeColuna + "' não encontrada.");
        }
        iIndexMainColumn = distance(vstrColumnsName.begin(), it);


        // Segundo passo: salvar o indice de todas as colunas a serem aggregadas
        if (vstrColumnsToAggregate.empty()) {
            vstrColumnsToAggregate = vstrColumnsName;
            vstrColumnsToAggregate.erase(vstrColumnsToAggregate.begin() + iIndexMainColumn);
        }

        for (const auto& col : vstrColumnsToAggregate) {
            auto it = find(vstrColumnsName.begin(), vstrColumnsName.end(), col);
            if (it == vstrColumnsName.end()) {
                throw std::invalid_argument("Coluna '" + col + "' não encontrada.");
            }
            viIndexColumnsToAggregate.push_back(distance(vstrColumnsName.begin(), it));
        }

        // Terceiro passo: criar o dataframe com as colunas que serão agregadas
        vector<string> vstrColumnsNameAux;
        vstrColumnsNameAux.push_back(strNomeColuna);
        for (string col : vstrColumnsToAggregate) {
            vstrColumnsNameAux.push_back(col);
        }

        dfAgrupado.vstrColumnsName = vstrColumnsNameAux;

        // Criar as colunas no novo dataframe
        for (const auto& col : vstrColumnsNameAux) {
            dfAgrupado.columns.emplace_back(col, "string");
        }

        // adicionando a coluna de contagem
        dfAgrupado.columns.emplace_back("count", "int");
        dfAgrupado.vstrColumnsName.push_back("count");

        // Quarto passo: criar a hash table com os dados agrupados
        unordered_map<string, vector<vector<string>>> umapGroupedData;

        int numAggregateColumns = viIndexColumnsToAggregate.size();
        int numRows = columns[iIndexMainColumn].getData().size();
        
        for (int i = 0; i < numRows; ++i) {
            // Chave da linha atual (valor da coluna principal)
            string key = anyToString(columns[iIndexMainColumn].getData()[i]);
        
            // Se a chave ainda não existe, inicializa os vetores internos
            if (umapGroupedData.find(key) == umapGroupedData.end()) {
                umapGroupedData[key] = vector<vector<string>>(numAggregateColumns);
            }
        
            // Para cada coluna que queremos agregar
            for (size_t j = 0; j < viIndexColumnsToAggregate.size(); ++j) {
                int colIndex = viIndexColumnsToAggregate[j];
                string value = anyToString(columns[colIndex].getData()[i]);
                umapGroupedData[key][j].push_back(value);
            }
        }

        // Quinto passo: criar o dataframe com os dados agregados (SUM)
        for (const auto& pair : umapGroupedData) {
            vector<any> row;
            row.push_back(pair.first); // adiciona a chave 
            int count = 0;

            // Para cada coluna que queremos agregar
            for (size_t j = 0; j < numAggregateColumns; ++j) {
                const auto& values = pair.second[j];
                double sum = 0.0;

                // Calcula a soma dos valores 
                for (const auto& value : values) {
                    sum += stod(value);
                    count++;
                }
                
                if (strAggMethod == "mean") {
                    sum /= values.size();
                }

                row.push_back(to_string(sum)); 
            }

            row.push_back(count);

            // Adiciona a linha ao DataFrame agrupado
            dfAgrupado.adicionaLinha(row);
        }
        
        return dfAgrupado;
    }   

    /**
     * @brief Realiza merge (inner join) entre este DataFrame e outro,
     *        usando uma ou mais colunas como chave.
     * @param other DataFrame a ser combinado.
     * @param on Vetor com nomes das colunas-chave.
     * @param joinType Tipo de join (apenas Inner suportado).
     * @return DataFrame resultante do merge.
     */
    Dataframe merge(const Dataframe& other, const vector<string>& on) const {
        // 1. Validar colunas-chave e obter índices
        vector<int> idxA, idxB;
        for (const auto& key : on) {
            auto itA = find(vstrColumnsName.begin(), vstrColumnsName.end(), key);
            auto itB = find(other.vstrColumnsName.begin(), other.vstrColumnsName.end(), key);
            if (itA == vstrColumnsName.end() || itB == other.vstrColumnsName.end())
                throw invalid_argument("Coluna-chave '" + key + "' não encontrada em ambos DataFrames.");
            idxA.push_back(distance(vstrColumnsName.begin(), itA));
            idxB.push_back(distance(other.vstrColumnsName.begin(), itB));
        }
    
        // 2. Criar DataFrame resultante
        Dataframe result;
        
        // Adicionar todas as colunas de df1
        result.vstrColumnsName = vstrColumnsName;
        for (const auto& col : columns) {
            result.columns.emplace_back(col.strGetName(), col.strGetType());
        }
    
        // Adicionar colunas de df2 que não estão em 'on'
        vector<size_t> other_cols_idx; // Índices das colunas de df2 a incluir
        for (size_t j = 0; j < other.vstrColumnsName.size(); ++j) {
            if (find(on.begin(), on.end(), other.vstrColumnsName[j]) == on.end()) {
                result.vstrColumnsName.push_back(other.vstrColumnsName[j]);
                result.columns.emplace_back(other.vstrColumnsName[j], other.columns[j].strGetType());
                other_cols_idx.push_back(j);
            }
        }
    
        // 3. Criar lookup para linhas de df2
        unordered_map<string, vector<int>> lookup;
        auto make_key = [&](const Dataframe& df, int row, const vector<int>& idx) {
            string k;
            for (int i : idx) {
                k += anyToString(df.columns[i].retornaElemento(row)) + "\u0001";
            }
            return k;
        };
    
        int rowsB = other.getShape().first;
        for (int i = 0; i < rowsB; ++i) {
            lookup[make_key(other, i, idxB)].push_back(i);
        }
    
        // 4. Percorrer linhas de df1 e juntar com correspondentes em df2
        int rowsA = getShape().first;
        vector<any> newRow;
        newRow.reserve(result.vstrColumnsName.size());
    
        for (int i = 0; i < rowsA; ++i) {
            string k = make_key(*this, i, idxA);
            auto it = lookup.find(k);
            if (it == lookup.end()) continue;
    
            for (int jB : it->second) {
                newRow.clear();
                
                // Adicionar todas as colunas de df1
                for (const auto& col : columns) {
                    newRow.push_back(col.retornaElemento(i));
                }
                
                // Adicionar colunas de df2 que não estão em 'on'
                for (size_t j : other_cols_idx) {
                    newRow.push_back(other.columns[j].retornaElemento(jB));
                }
    
                // Adicionar a linha ao resultado
                if (!result.adicionaLinha(newRow)) {
                    cerr << "Erro ao adicionar linha no merge." << endl;
                }
            }
        }
    
        return result;
    }

    /**
     * @brief Método auxiliar para impressão do cabeçalho do DataFrame.
     */
    void printHeader(ostream& os, const Dataframe& df, int col_width = 15, int index_width = 5) {
        os << left << setw(index_width) << "" << "  ";
        for (const auto& name : df.vstrColumnsName) {
            string header_name = name.substr(0, col_width);
            if (name.length() > col_width) {
                header_name[col_width - 3] = header_name[col_width - 2] = header_name[col_width - 1] = '.';
            }
            os << left << setw(col_width) << header_name << "  ";
        }
        os << "\n";

        os << left << setw(index_width) << "" << "  ";
        for (size_t i = 0; i < df.vstrColumnsName.size(); ++i) {
            os << left << setw(col_width) << "------------" << "  ";
        }
        os << "\n";
    }

    /**
     * @brief Sobrecarga do operador de saída para imprimir o DataFrame./framework/TesteTrigger.cpp
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
        for (auto& col : dfInput.columns) {
            string name = col.strGetName();
            string type = col.strGetType();
            name += " <" + type + ">";

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

        for (size_t i = 0; i < num_rows; ++i) {
            os << left << setw(index_width) << i << "  ";
            for (size_t j = 0; j < num_cols; ++j) {
                string val_str = anyToString(df.columns[j].retornaElemento(i));
                if (val_str.length() > col_width)
                    val_str[col_width - 3] = val_str[col_width - 2] = val_str[col_width - 1] = '.';
                os << left << setw(col_width) << val_str.substr(0, col_width) << "  ";
            }
            os << "\n";
        }

        return os;
    }    
};

#endif