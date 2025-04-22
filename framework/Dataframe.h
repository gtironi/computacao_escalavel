#ifndef DATAFRAME_HPP
#define DATAFRAME_HPP

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <map>
#include <any>
#include <unordered_map>
#include <functional>
#include "Series.h"

using namespace std;

static inline string anyToString(const any &value)
{
    try
    {
        if (value.type() == typeid(string))
        {
            return any_cast<string>(value);
        }
        else if (value.type() == typeid(bool))
        {
            return any_cast<bool>(value) ? "true" : "false";
        }
        else if (value.type() == typeid(int))
        {
            return to_string(any_cast<int>(value));
        }
        else if (value.type() == typeid(double))
        {
            return to_string(any_cast<double>(value));
        }
        else
        {
            return "[Unsupported Type]";
        }
    }
    catch (const bad_any_cast &)
    {
        return "[Error]";
    }
}

/**
 * @class Dataframe
 * @brief Representa um conjunto de colunas (Series) organizadas como um DataFrame.
 */
class Dataframe
{
public:
    vector<string> vstrColumnsName; ///< Vetor que armazena os nomes das colunas
    vector<Series<any>> columns;    ///< Vetor que armazena as colunas do DataFrame

    /**
     * @brief Construtor padrão do DataFrame.
     */
    Dataframe() = default;

    /**
     * @brief Construtor de cópia do DataFrame.
     * @param other Objeto Dataframe a ser copiado.
     */
    Dataframe(const Dataframe &other)
    {
        vstrColumnsName = other.vstrColumnsName;
        columns = other.columns;
    }

    /**
     * @brief Construtor de movimento do DataFrame.
     * @param other Objeto Dataframe a ser movido.
     */
    Dataframe(Dataframe &&other) noexcept
    {
        vstrColumnsName = std::move(other.vstrColumnsName);
        columns = std::move(other.columns);
    }

    /**
     * @brief Sobrecarga do operador de atribuição.
     * @param other Objeto Dataframe a ser atribuído.
     * @return Referência para o objeto atribuído.
     */
    Dataframe &operator=(const Dataframe &other)
    {
        if (this != &other)
        {
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
    Dataframe &operator=(Dataframe &&other) noexcept
    {
        if (this != &other)
        {
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
    bool adicionaColuna(Series<any> novaSerie)
    {

        if (columns.empty())
        {
            vstrColumnsName.push_back(novaSerie.strGetName());
            columns.push_back(novaSerie);
            return true;
        }
        else
        {
            bool viavel = true;

            for (size_t i = 0; i < columns.size(); i++)
            {
                if (novaSerie.iGetSize() != columns[i].iGetSize())
                {
                    viavel = false;
                    break;
                }
            }

            if (viavel)
            {
                vstrColumnsName.push_back(novaSerie.strGetName());
                columns.push_back(novaSerie);
                return true;
            }
            else
            {
                cerr << "Erro: A nova coluna possui tamanho diferente das colunas existentes." << endl;
                return false;
            }
        }
    }

    /**
     * @brief Imprime os nomes das colunas do DataFrame.
     */
    void printColsName()
    {
        cout << "Colunas: ";
        for (const auto &col : vstrColumnsName)
        {
            cout << col << " ";
        }
        cout << endl;
    }

    /**
     * @brief Adiciona uma nova coluna genérica ao DataFrame.
     * @tparam T Tipo dos dados na Series
     * @param novaSerie Objeto Series<T> a ser adicionado.
     * @return true se a adição for bem-sucedida, false caso contrário.
     */
    template <typename T>
    bool adicionaColuna(const Series<T> &novaSerie)
    {
        // Converter Series<T> para Series<any>
        Series<any> serieConvertida(novaSerie.strGetName(), novaSerie.strGetType());

        for (size_t i = 0; i < novaSerie.iGetSize(); i++)
        {
            try
            {
                serieConvertida.bAdicionaElemento(novaSerie.retornaElemento(i));
            }
            catch (const exception &e)
            {
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
    pair<int, int> getShape()
    {
        int iNumColunas = vstrColumnsName.size();
        int iNumLinhas = columns.empty() ? 0 : columns[0].iGetSize();
        pair<int, int> shape = {iNumLinhas, iNumColunas};
        return shape;
    }

    pair<int, int> getShape() const
    {
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
    bool adicionaLinha(const vector<any> &novaLinha)
    {
        if (novaLinha.size() != columns.size())
        {
            cout << "Datapoint com tamanho inválido. É: " << novaLinha.size() << " - Deveria ser: " << columns.size() << endl;
            return false;
        }

        for (size_t i = 0; i < columns.size(); i++)
        {
            string expectedType = columns[i].strGetType();
            bool valid = true;
            try
            {
                if (anyToString(novaLinha[i]).empty())
                {
                    valid = false;
                }
            }
            catch (const bad_any_cast &)
            {
                valid = false;
            }
            if (!valid)
            {
                cerr << "Tipo de dado inválido para a coluna " << columns[i].strGetName() << ". Esperado: " << expectedType << endl;
                return false;
            }
        }

        for (size_t i = 0; i < columns.size(); i++)
        {
            columns[i].bAdicionaElemento(novaLinha[i]);
        }

        return true;
    }

    /**
     * @brief Remove uma linha específica do DataFrame.
     * @param iIndex O índice da linha a ser removida.
     * @return true se a remoção for bem-sucedida, false caso contrário.
     */
    bool removeLinha(int iIndex)
    {
        if (iIndex < 0 || iIndex >= this->getShape().first)
        {
            cerr << "Índice inválido para remoção de linha." << endl;
            return false;
        }

        for (size_t i = 0; i < columns.size(); i++)
        {
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
    Dataframe filtroByValue(const string &strNomeColuna, const any &valor)
    {
        Dataframe auxDf;
        auxDf.vstrColumnsName = vstrColumnsName;

        // Criar estrutura das colunas no novo dataframe
        for (const auto &col : columns)
        {
            auxDf.columns.emplace_back(col.strGetName(), col.strGetType());
        }

        // Localiza a coluna de filtro
        auto it = find(vstrColumnsName.begin(), vstrColumnsName.end(), strNomeColuna);
        if (it == vstrColumnsName.end())
        {
            throw std::invalid_argument("Coluna '" + strNomeColuna + "' não encontrada.");
        }

        int colIndex = distance(vstrColumnsName.begin(), it);
        const auto &data = columns[colIndex].getData();

        // Pré-conta quantas linhas irão para o novo DataFrame
        size_t matchCount = count_if(data.begin(), data.end(), [&](const auto &el)
                                     { return anyToString(el) == anyToString(valor); });

        // Pré-aloca espaço nas colunas do novo dataframe
        for (auto &col : auxDf.columns)
        {
            col.reserve(matchCount);
        }

        // Copia os dados filtrados diretamente
        for (size_t i = 0; i < data.size(); ++i)
        {
            if (anyToString(data[i]) == anyToString(valor))
            {
                for (size_t j = 0; j < columns.size(); ++j)
                {
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
    void hStack(Dataframe &other)
    {
        if (this->columns.empty())
        {
            // Copia os nomes das colunas e os dados do outro DataFrame
            this->vstrColumnsName = other.vstrColumnsName;
            this->columns = other.columns;
            return;
        }

        if (this->columns.size() != other.columns.size())
        {
            cout << "DataFrames com tamanhos diferentes. Não é possível empilhar." << endl;
            return;
        }

        for (size_t i = 0; i < columns.size(); i++)
        {
            if (columns[i].strGetType() != other.columns[i].strGetType())
            {
                cout << "Tipos de colunas diferentes. Não é possível empilhar." << endl;
                return;
            }
        }

        for (size_t i = 0; i < columns.size(); i++)
        {
            columns[i].hStack(other.columns[i]);
        }
    }

    /**
     * @brief Empilha dois DataFrames agrupados horizontalmente, somando valores de soma e contagem
     * @param other DataFrame agrupado a ser empilhado
     */
    void hStackGroup(const Dataframe &other)
    {
        if (this->columns.empty())
        {
            // Se este DataFrame estiver vazio, apenas copie o outro
            this->vstrColumnsName = other.vstrColumnsName;
            this->columns = other.columns;
            return;
        }

        // Verifica se os DataFrames têm a mesma estrutura
        if (this->vstrColumnsName.size() != other.vstrColumnsName.size())
        {
            throw invalid_argument("DataFrames têm números diferentes de colunas");
        }

        // Mapa para armazenar as linhas por chave de agrupamento
        unordered_map<string, vector<int>> groupMap;

        // Identifica colunas de grupo (não terminam com _sum, _mean ou count)
        vector<bool> isGroupColumn;
        for (const auto &colName : vstrColumnsName)
        {
            bool isGroup = !((colName.length() > 4 && colName.substr(colName.length() - 4) == "_sum") ||
                             (colName.length() > 5 && colName.substr(colName.length() - 5) == "_mean") ||
                             colName == "count");
            isGroupColumn.push_back(isGroup);
        }

        // Criar chaves para as linhas existentes
        for (int i = 0; i < getShape().first; ++i)
        {
            string key;
            for (size_t j = 0; j < vstrColumnsName.size(); ++j)
            {
                if (isGroupColumn[j])
                {
                    key += anyToString(columns[j].retornaElemento(i)) + "|";
                }
            }
            groupMap[key].push_back(i);
        }

        // Para cada linha do outro DataFrame
        for (int i = 0; i < other.getShape().first; ++i)
        {
            string key;
            vector<any> newRow;

            // Constrói a chave para esta linha
            for (size_t j = 0; j < other.vstrColumnsName.size(); ++j)
            {
                if (isGroupColumn[j])
                {
                    key += anyToString(other.columns[j].retornaElemento(i)) + "|";
                }
            }

            auto existingGroup = groupMap.find(key);
            if (existingGroup != groupMap.end())
            {
                // Grupo existente - somar valores
                int existingIdx = existingGroup->second[0];
                for (size_t j = 0; j < vstrColumnsName.size(); ++j)
                {
                    if (isGroupColumn[j])
                    {
                        // Mantém o valor original para colunas de grupo
                        newRow.push_back(columns[j].retornaElemento(existingIdx));
                    }
                    else
                    {
                        // Soma valores para colunas de agregação
                        double val1 = stod(anyToString(columns[j].retornaElemento(existingIdx)));
                        double val2 = stod(anyToString(other.columns[j].retornaElemento(i)));

                        if (columns[j].strGetType() == "int")
                            newRow.push_back(static_cast<int>(val1 + val2));
                        else
                            newRow.push_back(val1 + val2);
                    }
                }

                // Remove a linha antiga e adiciona a nova linha somada
                removeLinha(existingIdx);
                adicionaLinha(newRow);

                // Atualiza o índice no mapa
                existingGroup->second[0] = getShape().first - 1;
            }
            else
            {
                // Novo grupo - adiciona a linha diretamente
                for (size_t j = 0; j < other.vstrColumnsName.size(); ++j)
                {
                    newRow.push_back(other.columns[j].retornaElemento(i));
                }
                adicionaLinha(newRow);
                groupMap[key] = {getShape().first - 1};
            }
        }
    }

    /**
     * @brief Agrupa o DataFrame por uma coluna específica e aplica uma função de agregação.
     * @param groupCols Colunas a serem usadas para agrupamento.
     * @param vstrColumnsToAggregate Colunas a serem agregadas (se vazio, todas as colunas serão agregadas).
     * @param soma Se true, realiza a soma das colunas agregadas.
     * @param media Se true, realiza a média das colunas agregadas.
     * @param contagem Se true, adiciona uma coluna de contagem.
     * @return Um novo DataFrame com os dados agrupados e agregados.
     */
    Dataframe dfGroupby(const vector<string> &groupCols, vector<string> vstrColumnsToAggregate = {}, bool soma = true, bool media = true, bool contagem = true)
    {
        Dataframe dfAgrupado;
        vector<int> viIndexColumnsToAggregate;
        vector<int> viIndexColunasDeAgrupamento;

        // Validação das colunas de agrupamento
        for (const auto &nomeCol : groupCols)
        {
            auto it = find(vstrColumnsName.begin(), vstrColumnsName.end(), nomeCol);
            if (it == vstrColumnsName.end())
            {
                throw std::invalid_argument("Coluna de agrupamento '" + nomeCol + "' não encontrada.");
            }
            viIndexColunasDeAgrupamento.push_back(distance(vstrColumnsName.begin(), it));
        }

        // Define as colunas a serem agregadas (se não especificadas)
        if (vstrColumnsToAggregate.empty())
        {
            vstrColumnsToAggregate = vstrColumnsName;
            for (const auto &nomeCol : groupCols)
            {
                vstrColumnsToAggregate.erase(remove(vstrColumnsToAggregate.begin(), vstrColumnsToAggregate.end(), nomeCol), vstrColumnsToAggregate.end());
            }
        }

        // Salva os índices e tipos das colunas agregadas
        vector<string> aggTypes;
        for (const auto &col : vstrColumnsToAggregate)
        {
            auto it = find(vstrColumnsName.begin(), vstrColumnsName.end(), col);
            if (it == vstrColumnsName.end())
            {
                throw std::invalid_argument("Coluna de agregação '" + col + "' não encontrada.");
            }
            int idx = distance(vstrColumnsName.begin(), it);
            viIndexColumnsToAggregate.push_back(idx);
            aggTypes.push_back(columns[idx].strGetType());
        }

        // Monta o DataFrame de saída
        dfAgrupado.vstrColumnsName.clear();
        // Colunas de agrupamento
        for (int idx : viIndexColunasDeAgrupamento)
        {
            dfAgrupado.vstrColumnsName.push_back(vstrColumnsName[idx]);
            dfAgrupado.columns.emplace_back(vstrColumnsName[idx], columns[idx].strGetType());
        }
        // Colunas agregadas
        for (size_t i = 0; i < vstrColumnsToAggregate.size(); i++)
        {
            if (soma)
            {
                dfAgrupado.vstrColumnsName.push_back(vstrColumnsToAggregate[i] + "_sum");
                dfAgrupado.columns.emplace_back(vstrColumnsToAggregate[i] + "_sum", aggTypes[i]);
            }
            if (media)
            {
                string tipo = aggTypes[i];
                if (tipo == "int")
                    tipo = "double";
                dfAgrupado.vstrColumnsName.push_back(vstrColumnsToAggregate[i] + "_mean");
                dfAgrupado.columns.emplace_back(vstrColumnsToAggregate[i] + "_mean", tipo);
            }
        }
        if (contagem)
        {
            dfAgrupado.vstrColumnsName.push_back("count");
            dfAgrupado.columns.emplace_back("count", "string");
        }

        // Tabela hash: chave -> par (vetor com vetores dos valores agregados, valores dos agrupamentos)
        unordered_map<string, pair<vector<vector<string>>, vector<string>>> umapGroupedData;
        int numAggregateColumns = viIndexColumnsToAggregate.size();
        int numRows = columns.empty() ? 0 : columns[0].getData().size();

        for (int i = 0; i < numRows; ++i)
        {
            // Cria a chave composta
            string chave;
            vector<string> valoresChave;
            for (int idx : viIndexColunasDeAgrupamento)
            {
                string valor = anyToString(columns[idx].getData()[i]);
                chave += valor + "|";
                valoresChave.push_back(valor);
            }

            if (umapGroupedData.find(chave) == umapGroupedData.end())
            {
                umapGroupedData[chave] = {vector<vector<string>>(numAggregateColumns), valoresChave};
            }

            // Registro dos valores para cada coluna a agregar
            for (size_t j = 0; j < viIndexColumnsToAggregate.size(); ++j)
            {
                int colIndex = viIndexColumnsToAggregate[j];
                string valor = anyToString(columns[colIndex].getData()[i]);
                umapGroupedData[chave].first[j].push_back(valor);
            }
        }

        // Montagem final das linhas agregadas
        for (const auto &pair : umapGroupedData)
        {
            const auto &valoresChave = pair.second.second;
            const auto &gruposDeValores = pair.second.first;
            vector<any> linha;

            // Adiciona os valores das colunas de agrupamento
            for (const auto &val : valoresChave)
            {
                linha.push_back(val);
            }

            // Para cada coluna agregada, calcular soma e/ou média
            for (size_t j = 0; j < gruposDeValores.size(); ++j)
            {
                double soma_val = 0.0;
                for (const auto &val : gruposDeValores[j])
                {
                    try
                    {
                        if (aggTypes[j] == "int")
                            soma_val += stoi(val);
                        else
                            soma_val += stod(val);
                    }
                    catch (...)
                    {
                        soma_val += 0.0;
                    }
                }
                if (soma)
                    linha.push_back((aggTypes[j] == "int") ? static_cast<int>(soma_val) : soma_val);
                if (media)
                {
                    double mean_val = gruposDeValores[j].empty() ? 0 : soma_val / gruposDeValores[j].size();
                    linha.push_back(mean_val);
                }
            }
            if (contagem)
            {
                int count_val = gruposDeValores.empty() ? 0 : gruposDeValores[0].size();
                linha.push_back(count_val);
            }

            dfAgrupado.adicionaLinha(linha);
        }

        return dfAgrupado;
    }

    /**
     * @brief Realiza uma operação entre duas colunas e adiciona o resultado como uma nova coluna.
     * @param strColumnName1 Nome da primeira coluna.
     * @param strColumnName2 Nome da segunda coluna.
     * @param funOperation Função que define a operação a ser realizada entre as colunas.
     * @param strNewColumnName Nome da nova coluna a ser adicionada.
     * @return true se a operação for bem-sucedida, false caso contrário.
     */
    bool bColumnOperation(const string &strColumnName1, const string &strColumnName2, function<string(string, string)> funOperation, const string &strNewColumnName)
    {
        // Passo 1: criar a nova série
        Series<any> newColumn(strNewColumnName, "string");

        // Passo 2: verificar se as colunas existem
        auto it1 = find(vstrColumnsName.begin(), vstrColumnsName.end(), strColumnName1);
        auto it2 = find(vstrColumnsName.begin(), vstrColumnsName.end(), strColumnName2);

        if (it1 == vstrColumnsName.end() || it2 == vstrColumnsName.end())
        {
            throw invalid_argument("Coluna não encontrada.");
            return false;
        }

        int iIndex1 = distance(vstrColumnsName.begin(), it1);
        int iIndex2 = distance(vstrColumnsName.begin(), it2);

        // Passo 3: operar elemento a elemento
        int iNumLinhas = columns[iIndex1].iGetSize();
        for (int i = 0; i < iNumLinhas; ++i)
        {
            string strValue1 = anyToString(columns[iIndex1].retornaElemento(i));
            string strValue2 = anyToString(columns[iIndex2].retornaElemento(i));
            newColumn.bAdicionaElemento(funOperation(strValue1, strValue2));
        }

        // Passo 4: adicionar a nova coluna
        adicionaColuna(newColumn);

        return true;
    }

    /**
     * @brief Realiza merge (inner join) entre este DataFrame e outro,
     *        usando uma ou mais colunas como chave.
     * @param other DataFrame a ser combinado.
     * @param on Vetor com nomes das colunas-chave.
     * @param joinType Tipo de join (apenas Inner suportado).
     * @return DataFrame resultante do merge.
     */
    Dataframe merge(const Dataframe &other, const vector<string> &on) const
    {
        // 1. Validar colunas-chave e obter índices
        vector<int> idxA, idxB;
        for (const auto &key : on)
        {
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
        for (const auto &col : columns)
        {
            result.columns.emplace_back(col.strGetName(), col.strGetType());
        }

        // Adicionar colunas de df2 que não estão em 'on'
        vector<size_t> other_cols_idx; // Índices das colunas de df2 a incluir
        for (size_t j = 0; j < other.vstrColumnsName.size(); ++j)
        {
            if (find(on.begin(), on.end(), other.vstrColumnsName[j]) == on.end())
            {
                result.vstrColumnsName.push_back(other.vstrColumnsName[j]);
                result.columns.emplace_back(other.vstrColumnsName[j], other.columns[j].strGetType());
                other_cols_idx.push_back(j);
            }
        }

        // 3. Criar lookup para linhas de df2
        unordered_map<string, vector<int>> lookup;
        auto make_key = [&](const Dataframe &df, int row, const vector<int> &idx)
        {
            string k;
            for (int i : idx)
            {
                k += anyToString(df.columns[i].retornaElemento(row)) + "\u0001";
            }
            return k;
        };

        int rowsB = other.getShape().first;
        for (int i = 0; i < rowsB; ++i)
        {
            lookup[make_key(other, i, idxB)].push_back(i);
        }

        // 4. Percorrer linhas de df1 e juntar com correspondentes em df2
        int rowsA = getShape().first;
        vector<any> newRow;
        newRow.reserve(result.vstrColumnsName.size());

        for (int i = 0; i < rowsA; ++i)
        {
            string k = make_key(*this, i, idxA);
            auto it = lookup.find(k);
            if (it == lookup.end())
                continue;

            for (int jB : it->second)
            {
                newRow.clear();

                // Adicionar todas as colunas de df1
                for (const auto &col : columns)
                {
                    newRow.push_back(col.retornaElemento(i));
                }

                // Adicionar colunas de df2 que não estão em 'on'
                for (size_t j : other_cols_idx)
                {
                    newRow.push_back(other.columns[j].retornaElemento(jB));
                }

                // Adicionar a linha ao resultado
                if (!result.adicionaLinha(newRow))
                {
                    cerr << "Erro ao adicionar linha no merge." << endl;
                }
            }
        }

        return result;
    }

    /**
     * @brief Imprime o cabeçalho do DataFrame.
     * @param os Fluxo de saída.
     * @param df DataFrame a ser impresso.
     * @param col_width Largura das colunas.
     * @param index_width Largura do índice.
     */
    void printHeader(ostream &os, const Dataframe &df, int col_width = 15, int index_width = 5)
    {
        os << left << setw(index_width) << "" << "  ";
        for (const auto &name : df.vstrColumnsName)
        {
            string header_name = name.substr(0, col_width);
            if (name.length() > col_width)
            {
                header_name[col_width - 3] = header_name[col_width - 2] = header_name[col_width - 1] = '.';
            }
            os << left << setw(col_width) << header_name << "  ";
        }
        os << "\n";

        os << left << setw(index_width) << "" << "  ";
        for (size_t i = 0; i < df.vstrColumnsName.size(); ++i)
        {
            os << left << setw(col_width) << "------------" << "  ";
        }
        os << "\n";
    }

    /**
     * @brief Sobrecarga do operador de saída para imprimir o DataFrame./framework/TesteTrigger.cpp
     */
    friend ostream &operator<<(ostream &os, const Dataframe &dfInput)
    {
        Dataframe df = dfInput;
        if (df.columns.empty())
        {
            os << "[Empty DataFrame]\n";
            return os;
        }

        pair<int, int> shape = df.getShape();

        size_t num_rows = shape.first;
        size_t num_cols = shape.second;
        int col_width = 15; // Largura fixa das colunas
        int index_width = 5;

        // Print header
        os << left << setw(index_width) << "" << "  ";
        for (auto &col : dfInput.columns)
        {
            string name = col.strGetName();
            string type = col.strGetType();
            name += " <" + type + ">";
            string header_name = (name.size() > size_t(col_width))
                                     ? (name.substr(0, col_width - 3) + "...")
                                     : name;
            os << left << setw(col_width) << header_name << "  ";
        }
        os << "\n"
           << left << setw(index_width) << "" << "  ";
        for (size_t i = 0; i < num_cols; i++)
        {
            os << left << setw(col_width) << string(col_width, '-') << "  ";
        }
        os << "\n";

        // Print rows
        if (num_rows <= 10)
        {
            // Se o número de linhas for menor ou igual a 10, imprime todas as linhas.
            for (size_t i = 0; i < num_rows; ++i)
            {
                os << left << setw(index_width) << i << "  ";
                for (size_t j = 0; j < num_cols; ++j)
                {
                    string val_str = anyToString(df.columns[j].retornaElemento(i));
                    if (val_str.length() > size_t(col_width))
                        val_str = val_str.substr(0, col_width - 3) + "...";
                    os << left << setw(col_width) << val_str << "  ";
                }
                os << "\n";
            }
        }
        else
        {
            // Imprime as 5 primeiras linhas
            for (size_t i = 0; i < 5; ++i)
            {
                os << left << setw(index_width) << i << "  ";
                for (size_t j = 0; j < num_cols; ++j)
                {
                    string val_str = anyToString(df.columns[j].retornaElemento(i));
                    if (val_str.length() > size_t(col_width))
                        val_str = val_str.substr(0, col_width - 3) + "...";
                    os << left << setw(col_width) << val_str << "  ";
                }
                os << "\n";
            }
            // Linha de reticências
            os << left << setw(index_width) << "..." << "  ";
            for (size_t j = 0; j < num_cols; ++j)
            {
                os << left << setw(col_width) << "..." << "  ";
            }
            os << "\n";
            // Imprime a última linha
            size_t last_index = num_rows - 1;
            os << left << setw(index_width) << last_index << "  ";
            for (size_t j = 0; j < num_cols; ++j)
            {
                string val_str = anyToString(df.columns[j].retornaElemento(last_index));
                if (val_str.length() > size_t(col_width))
                    val_str = val_str.substr(0, col_width - 3) + "...";
                os << left << setw(col_width) << val_str << "  ";
            }
            os << "\n";
        }

        return os;
    }

    /**
     * @brief Remove uma coluna do DataFrame pelo nome.
     * @param strNomeColuna Nome da coluna a ser removida.
     */
    void dropCol(const string &strNomeColuna)
    {
        auto it = find(vstrColumnsName.begin(), vstrColumnsName.end(), strNomeColuna);
        if (it != vstrColumnsName.end())
        {
            int index = distance(vstrColumnsName.begin(), it);
            vstrColumnsName.erase(it);
            columns.erase(columns.begin() + index);
        }
        else
        {
            cout << "Coluna não encontrada: " << strNomeColuna << endl;
        }
    }

    /**
     * @brief Remove uma coluna do DataFrame pelo índice.
     * @param iIndex Índice da coluna a ser removida.
     */
    void dropCol(int iIndex)
    {
        if (iIndex >= 0 && iIndex < vstrColumnsName.size())
        {
            vstrColumnsName.erase(vstrColumnsName.begin() + iIndex);
            columns.erase(columns.begin() + iIndex);
        }
        else
        {
            cout << "Índice inválido: " << iIndex << endl;
        }
    }

    /**
     * @brief Remove múltiplas colunas do DataFrame, especificadas pelos nomes.
     * @param vstrColunas Vetor com os nomes das colunas a serem removidas.
     */
    void dropCol(const vector<string> &vstrColunas)
    {
        for (const auto &col : vstrColunas)
        {
            dropCol(col);
        }
    }

    /**
     * @brief Remove múltiplas colunas do DataFrame, especificadas pelos índices.
     * @param viIndices Vetor com os índices das colunas a serem removidas.
     */
    void dropCol(const vector<int> &viIndices)
    {
        for (const auto &index : viIndices)
        {
            dropCol(index);
        }
    }

    /**
     * @brief Define o tipo de uma coluna e realiza a conversão dos valores.
     * @param strNomeColuna Nome da coluna.
     * @param strTipo Novo tipo da coluna.
     */
    void setColType(const string &strNomeColuna, const string &strTipo)
    {
        auto it = find(vstrColumnsName.begin(), vstrColumnsName.end(), strNomeColuna);
        if (it == vstrColumnsName.end())
        {
            cout << "Coluna não encontrada: " << strNomeColuna << endl;
            return;
        }

        int index = distance(vstrColumnsName.begin(), it);
        const auto &data = columns[index].getData();
        Series<any> newSeries(strNomeColuna, strTipo);

        auto convert = [&](const any &value) -> any
        {
            try
            {
                if (strTipo == "int")
                {
                    if (value.type() == typeid(int))
                        return value;
                    else if (value.type() == typeid(double))
                        return static_cast<int>(any_cast<double>(value));
                    else if (value.type() == typeid(string))
                        return stoi(any_cast<string>(value));
                    else
                        throw invalid_argument("Cannot convert to int");
                }
                else if (strTipo == "double")
                {
                    if (value.type() == typeid(double))
                        return value;
                    else if (value.type() == typeid(int))
                        return static_cast<double>(any_cast<int>(value));
                    else if (value.type() == typeid(string))
                        return stod(any_cast<string>(value));
                    else
                        throw invalid_argument("Cannot convert to double");
                }
                else if (strTipo == "string")
                {
                    if (value.type() == typeid(string))
                        return value;
                    else if (value.type() == typeid(int) || value.type() == typeid(double) || value.type() == typeid(bool))
                        return anyToString(value);
                    else
                        throw invalid_argument("Cannot convert to string");
                }
                else if (strTipo == "bool")
                {
                    if (value.type() == typeid(bool))
                        return value;
                    else if (value.type() == typeid(int))
                        return any_cast<int>(value) != 0;
                    else if (value.type() == typeid(string))
                    {
                        string s = any_cast<string>(value);
                        transform(s.begin(), s.end(), s.begin(), ::tolower);
                        if (s == "true")
                            return true;
                        else if (s == "false")
                            return false;
                        else
                            throw invalid_argument("Cannot convert to bool");
                    }
                    else
                        throw invalid_argument("Cannot convert to bool");
                }
                else
                {
                    throw invalid_argument("Tipo inválido: " + strTipo);
                }
            }
            catch (const exception &e)
            {
                throw invalid_argument("Erro ao converter valor '" + anyToString(value) + "' para " + strTipo + ": " + e.what());
            }
        };

        for (const auto &value : data)
        {
            try
            {
                any convertedValue = convert(value);
                newSeries.bAdicionaElemento(convertedValue);
            }
            catch (const exception &e)
            {
                cerr << e.what() << endl;
                return;
            }
        }

        columns[index] = std::move(newSeries);
    }

    /**
     * @brief Obtém um slice do DataFrame entre as linhas 'start' (inclusivo) e 'end' (exclusivo) de forma eficiente.
     * @param start Índice inicial do slice.
     * @param end Índice final do slice.
     * @return Um novo DataFrame contendo o slice solicitado.
     */
    Dataframe slice(int start, int end) const
    {
        int totalRows = this->getShape().first;
        if (start < 0 || end > totalRows || start >= end)
        {
            throw std::invalid_argument("Intervalo de linhas inválido.");
        }

        Dataframe dfSlice;
        // Inicializa as colunas copiando o slice dos vetores de dados de cada Series
        for (size_t i = 0; i < vstrColumnsName.size(); i++)
        {
            dfSlice.vstrColumnsName.push_back(vstrColumnsName[i]);
            dfSlice.columns.push_back(columns[i].slice(start, end));
        }
        return dfSlice;
    }
};

#endif