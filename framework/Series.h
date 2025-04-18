#ifndef SERIES_HPP
#define SERIES_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <typeinfo>
#include <type_traits>
#include <utility>
#include <any>

using namespace std;

/**
 * @class Series
 * @brief Representa uma coluna de um DataFrame, contendo um nome e um vetor de dados.
 * @tparam T Tipo dos dados armazenados na Series
 */
template <typename T>
class Series {
private:
    string strColumnName;  ///< Nome da coluna
    vector<T> vecColumnData; ///< Dados armazenados na coluna
    string strColumnType; ///< Tipo da coluna

    /**
     * @brief Converte um valor any para string
     * @param value Valor a ser convertido
     * @return String representando o valor
     */
    string anyToString(const any& value) const {
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

public:
    /**
     * @brief Construtor da classe Series
     * @param columnName Nome da coluna
     */
    explicit Series(const string& columnName) : strColumnName(columnName) {}

    /**
     * @brief Construtor completo com nome e dados
     * @param columnName Nome da coluna
     * @param data Vetor de dados
     */
    Series(const string& columnName, const vector<T>& data) : 
        strColumnName(columnName), vecColumnData(data.begin(), data.end()) {}

    /**
     * @brief Construtor completo com nome e tipo
     * @param columnName Nome da coluna
     * @param columnType Tipo da coluna
     */
    Series(const string& columnName, const string& columnType)
        : strColumnName(columnName), strColumnType(columnType) {}


    Series(const string& columnName, const string& columnType, const vector<T>& data)
        : strColumnName(columnName), strColumnType(columnType), vecColumnData(data) {}
    
    /**
     * @brief Construtor padrão
     */
    Series() = default;

    /**
     * @brief Construtor de cópia da classe Series
     * @param other Objeto Series a ser copiado
     */
    Series(const Series& other) = default;

    /**
     * @brief Construtor de movimento
     * @param other Objeto Series a ser movido
     */
    Series(Series&& other) noexcept = default;

    /**
     * @brief Destrutor virtual padrão
     */
    ~Series() = default;

    /**
     * @brief Altera o nome da coluna.
     * @param strNovoNome Novo nome da coluna.
     */
    void setName(const string& strNovoNome) {
        strColumnName = strNovoNome;
    }

    /**
     * @brief Retorna o nome da coluna.
     * @return Nome da coluna.
     */
    string strGetName() const {
        return strColumnName;
    }

    /**
     * @brief Retorna o tipo da coluna.
     * @return Tipo da coluna.
     */
    string strGetType() const {
        return strColumnType;
    }
    
    /**
     * @brief Retorna o número de elementos da coluna.
     * @return Tamanho do vetor de dados.
     */
    size_t iGetSize() const {
        return vecColumnData.size();
    }

    /**
     * @brief Pré-aloca espaço para n elementos
     * @param n Número de elementos a reservar
     */
    void reserve(size_t n) {
        vecColumnData.reserve(n);
    }

    /**
     * @brief Retorna o vetor de dados da coluna.
     * @return Vetor contendo os dados.
     */
    const vector<any>& getData() const {
        return vecColumnData;
    }

    /**
     * @brief Obtém referência não-const ao vetor de dados
     * @return Referência ao vetor de dados
     */
    vector<any>& getDataRef() {
        return vecColumnData;
    }

    /**
     * @brief Sobrecarga do operador de inserção para exibir a Series.
     * @param os Fluxo de saída.
     * @param series Objeto Series a ser exibido.
     * @return Fluxo de saída atualizado.
     */
    friend ostream& operator<<(ostream& os, const Series& series) {
        os << series.strColumnName << ": [";
        
        size_t limit = min(series.vecColumnData.size(), static_cast<size_t>(5));
        
        for (size_t i = 0; i < limit; ++i) {
            os << series.anyToString(series.vecColumnData[i]);

            if (i < limit - 1) os << ", ";
        }
        
        if (series.vecColumnData.size() > 5) {
            os << ", ...";
        }

        os << "]\n";
    
        return os;
    }
    
    /**
     * @brief Sobrecarga do operador de atribuição.
     * @param other Objeto Series a ser atribuído.
     * @return Referência para o objeto atribuído.
     */
    Series& operator=(const Series& other) = default;

    /**
     * @brief Sobrecarga do operador de atribuição por movimento
     * @param other Objeto Series a ser movido
     * @return Referência para o objeto atual
     */
    Series& operator=(Series&& other) noexcept = default;

    /**
     * @brief Adiciona um elemento à coluna
     * @param elemento Elemento a ser adicionado
     * @return true se a operação for bem-sucedida, false caso contrário
     */
    bool bAdicionaElemento(const any& elemento) {
        vecColumnData.push_back(elemento);
        return true;
    }

    /**
     * @brief Adiciona um elemento à coluna (versão para movimento)
     * @param elemento Elemento a ser adicionado
     * @return true se a operação for bem-sucedida, false caso contrário
     */
    bool bAdicionaElemento(any&& elemento) {
        vecColumnData.push_back(std::move(elemento));
        return true;
    }

    /**
     * @brief Remove o último elemento da coluna
     * @return true se a remoção for bem-sucedida, false caso contrário
     */
    bool bRemoveUltimoElemento() {
        if (!vecColumnData.empty()) {
            vecColumnData.pop_back();
            return true;
        }
        return false;
    }
    
    /**
     * @brief Remove um elemento da coluna com base no índice.
     * @param iIndex O índice do elemento que deve ser removido.
     * @return true se a remoção for bem-sucedida, false caso contrário.
     */
    bool bRemovePeloIndex(int iIndex) {
        if (iIndex < 0 || iIndex >= static_cast<int>(vecColumnData.size())) {
            return false;
        }

        vecColumnData.erase(vecColumnData.begin() + iIndex);
        return true;
    }

    /**
     * @brief Retorna um elemento da coluna com base no índice fornecido
     * @param iIndex Índice do elemento
     * @return Elemento armazenado no índice especificado
     */
    any retornaElemento(int iIndex) const {
        if (iIndex >= 0 && iIndex < static_cast<int>(vecColumnData.size())) {
            return vecColumnData[iIndex];
        } else {
            throw out_of_range("Índice fora dos limites: " + to_string(iIndex));
        }
    }

    /**
     * @brief Empilha duas Series horizontalmente
     * @param other A outra Series a ser empilhada.
     */
    void hStack(const Series<T>& other) {
        vecColumnData.insert(vecColumnData.end(), other.vecColumnData.begin(), other.vecColumnData.end());
    }

    /**
     * @brief Método que permite adicionar um elemento
     * @param value Valor a ser adicionado
     */
    void addData(const T& value) {
        vecColumnData.push_back(value);
    }

    /**
     * @brief Método que permite adicionar um elemento por movimento
     * @param value Valor a ser movido para a série
     */
    void addData(T&& value) {
        vecColumnData.push_back(std::move(value));
    }
    
    /**
     * @brief Adiciona múltiplos elementos de uma vez
     * @param elements Vetor de elementos a adicionar
     */
    void addDataBatch(const vector<T>& elements) {
        vecColumnData.insert(vecColumnData.end(), elements.begin(), elements.end());
    }
    
    /**
     * @brief Limpa todos os dados da série
     */
    void clear() {
        vecColumnData.clear();
    }
};

// Especialização para strings, otimizada para ETL
// Esta especialização será usada como formato padrão para dados importados
using StringSeries = Series<string>;

#endif