#ifndef DATAFRAME_H
#define DATAFRAME_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <numeric> 
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <typeinfo>
#include <regex>
#include <any>

using namespace std;

/**
 * @brief Representa uma data simples (dia, mês e ano).
 *
 * A estrutura DateDay encapsula uma data composta por dia, mês e ano.
 * Possui construtores para inicialização a partir de valores individuais, de uma string ou de um objeto DateTime,
 * métodos para validação e operadores de comparação e fluxo.
 */
struct DateDay {
    int dia;  ///< Dia do mês.
    int mes;  ///< Mês do ano.
    int ano;  ///< Ano.

    /**
     * @brief Construtor que inicializa a data com valores fornecidos.
     * @param d Dia.
     * @param m Mês.
     * @param a Ano.
     * @throws runtime_error se a data for inválida.
     */
    DateDay(int d, int m, int a) : dia(d), mes(m), ano(a) {
        if (!isValid()) throw runtime_error("Data inválida");
    }
    
    /**
     * @brief Construtor padrão.
     *
     * Inicializa a data com o valor 1-1-1.
     */
    DateDay() : dia(1), mes(1), ano(1) {}
    
    /**
     * @brief Construtor de cópia.
     * @param dt Instância de DateDay a ser copiada.
     */
    DateDay(const DateDay& dt) : dia(dt.dia), mes(dt.mes), ano(dt.ano) {}

    /**
     * @brief Construtor de conversão a partir de um DateTime.
     *
     * Converte um objeto DateTime para DateDay, descartando os componentes de tempo.
     * @param dt Objeto DateTime.
     * @throws runtime_error se a data resultante for inválida.
     */
    DateDay(const struct DateTime& dt);
    
    /**
     * @brief Construtor que inicializa a data a partir de uma string.
     *
     * A string deve estar no formato "DD-MM-YYYY".
     * @param data String representando a data.
     * @throws runtime_error se o formato ou a data forem inválidos.
     */
    DateDay(const string& data) {
        size_t pos1 = data.find('-');
        size_t pos2 = data.find('-', pos1 + 1);
        if (pos1 == string::npos || pos2 == string::npos)
            throw runtime_error("Formato de data inválido");
        dia = stoi(data.substr(0, pos1));
        mes = stoi(data.substr(pos1 + 1, pos2 - pos1 - 1));
        ano = stoi(data.substr(pos2 + 1));
        if (!isValid()) throw runtime_error("Data inválida");
    }

    /// Destrutor padrão.
    ~DateDay() = default;
    
    /**
     * @brief Verifica se a data é válida.
     * @return true se a data for válida, false caso contrário.
     */
    bool isValid() const {
        if (ano < 1 || mes < 1 || mes > 12 || dia < 1) return false;
        int diasPorMes[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        bool bissexto = (ano % 400 == 0) || (ano % 4 == 0 && ano % 100 != 0);
        if (bissexto) diasPorMes[1] = 29;
        return dia <= diasPorMes[mes - 1];
    }

    /**
     * @brief Operador de inserção em fluxo.
     * @param os Fluxo de saída.
     * @param dt Instância de DateDay.
     * @return Fluxo de saída atualizado.
     */
    friend ostream& operator<<(ostream& os, const DateDay& dt) {
        os << dt.dia << "-" << dt.mes << "-" << dt.ano;
        return os;
    }

    /**
     * @brief Operador de extração de fluxo.
     * @param is Fluxo de entrada.
     * @param dt Instância de DateDay.
     * @return Fluxo de entrada atualizado.
     * @throws runtime_error se o formato da string for inválido.
     */
    friend istream& operator>>(istream& is, DateDay& dt) {
        string data;
        is >> data;
        size_t pos1 = data.find('-');
        size_t pos2 = data.find('-', pos1 + 1);
        if (pos1 == string::npos || pos2 == string::npos)
            throw runtime_error("Formato de data inválido");
        dt.dia = stoi(data.substr(0, pos1));
        dt.mes = stoi(data.substr(pos1 + 1, pos2 - pos1 - 1));
        dt.ano = stoi(data.substr(pos2 + 1));
        if (!dt.isValid()) throw runtime_error("Data inválida");
        return is;
    }

    /**
     * @brief Operador de atribuição.
     * @param other Objeto DateDay a ser atribuído.
     * @return Referência para o objeto atribuído.
     */
    DateDay& operator=(const DateDay& other) {
        if (this != &other) {
            dia = other.dia;
            mes = other.mes;
            ano = other.ano;
        }
        return *this;
    }

    /**
     * @brief Operador de igualdade.
     * @param other Objeto DateDay a ser comparado.
     * @return true se as datas forem iguais; false caso contrário.
     */
    bool operator==(const DateDay& other) const {
        return (ano == other.ano) && (mes == other.mes) && (dia == other.dia);
    }

    /**
     * @brief Operador "menor que".
     * @param other Objeto DateDay a ser comparado.
     * @return true se esta data for anterior a other; false caso contrário.
     */
    bool operator<(const DateDay& other) const {
        if (ano != other.ano) return ano < other.ano;
        else if (mes != other.mes) return mes < other.mes;
        else return dia < other.dia;
    }

    /**
     * @brief Operador "maior que".
     * @param other Objeto DateDay a ser comparado.
     * @return true se esta data for posterior a other; false caso contrário.
     */
    bool operator>(const DateDay& other) const {
        return other < *this;
    }

    /**
     * @brief Operador de desigualdade.
     * @param other Objeto DateDay a ser comparado.
     * @return true se as datas forem diferentes; false caso contrário.
     */
    bool operator!=(const DateDay& other) const {
        return !(*this == other);
    }

    /**
     * @brief Operador "menor ou igual".
     * @param other Objeto DateDay a ser comparado.
     * @return true se esta data for anterior ou igual a other; false caso contrário.
     */
    bool operator<=(const DateDay& other) const {
        return !(*this > other);
    }
    
    /**
     * @brief Operador "maior ou igual".
     * @param other Objeto DateDay a ser comparado.
     * @return true se esta data for posterior ou igual a other; false caso contrário.
     */
    bool operator>=(const DateDay& other) const {
        return !(*this < other);
    }
};

/**
 * @brief Representa uma data e hora.
 *
 * A estrutura DateTime encapsula uma data (dia, mês e ano) e o horário (hora, minuto e segundo).
 * Fornece construtores para inicialização a partir de valores individuais, de uma string ou de um objeto DateDay,
 * além de métodos de validação, operadores de fluxo e comparação.
 */
struct DateTime {
    int dia;     ///< Dia do mês.
    int mes;     ///< Mês do ano.
    int ano;     ///< Ano.
    int hora;    ///< Hora do dia (0-23).
    int minuto;  ///< Minuto (0-59).
    int segundo; ///< Segundo (0-59).

    /**
     * @brief Construtor que inicializa a data e hora com valores fornecidos.
     * @param d Dia.
     * @param m Mês.
     * @param a Ano.
     * @param h Hora.
     * @param min Minuto.
     * @param s Segundo.
     * @throws runtime_error se a data/hora for inválida.
     */
    DateTime(int d, int m, int a, int h, int min, int s) : dia(d), mes(m), ano(a), hora(h), minuto(min), segundo(s) {
        if (!isValid()) throw runtime_error("Data inválida");
    }

    /**
     * @brief Construtor padrão.
     *
     * Inicializa a data com 1-1-1 e o horário com 00:00:00.
     */
    DateTime() : dia(1), mes(1), ano(1), hora(0), minuto(0), segundo(0) {}

    /**
     * @brief Construtor de cópia.
     * @param dt Objeto DateTime a ser copiado.
     */
    DateTime(const DateTime& dt) : dia(dt.dia), mes(dt.mes), ano(dt.ano), hora(dt.hora), minuto(dt.minuto), segundo(dt.segundo) {}

    /**
     * @brief Construtor que inicializa a data/hora a partir de um DateDay.
     *
     * O horário é definido como 00:00:00.
     * @param dt Objeto DateDay.
     */
    DateTime(const DateDay& dt) : dia(dt.dia), mes(dt.mes), ano(dt.ano), hora(0), minuto(0), segundo(0) {}

    /**
     * @brief Construtor que inicializa a data/hora a partir de uma string.
     *
     * A string deve estar no formato "DD-MM-YYYY HH:MM:SS".
     * @param data String representando a data e hora.
     * @throws runtime_error se o formato ou a data/hora forem inválidos.
     */
    DateTime(const string& data) {
        size_t pos1 = data.find('-');
        size_t pos2 = data.find('-', pos1 + 1);
        size_t pos3 = data.find(' ', pos2 + 1);
        size_t pos4 = data.find(':', pos3 + 1);
        size_t pos5 = data.find(':', pos4 + 1);
        if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos || pos4 == string::npos || pos5 == string::npos)
            throw runtime_error("Formato de data inválido");
        dia = stoi(data.substr(0, pos1));
        mes = stoi(data.substr(pos1 + 1, pos2 - pos1 - 1));
        ano = stoi(data.substr(pos2 + 1, pos3 - pos2 - 1));
        hora = stoi(data.substr(pos3 + 1, pos4 - pos3 - 1));
        minuto = stoi(data.substr(pos4 + 1, pos5 - pos4 - 1));
        segundo = stoi(data.substr(pos5 + 1));
        if (!isValid()) throw runtime_error("Data inválida");
    }

    /// Destrutor padrão.
    ~DateTime() = default;

    /**
     * @brief Verifica se a data e hora são válidas.
     * @return true se válidas; false caso contrário.
     */
    bool isValid() const {
        if (ano < 1 || mes < 1 || mes > 12 || dia < 1 || hora < 0 || hora > 23 || minuto < 0 || minuto > 59 || segundo < 0 || segundo > 59)
            return false;
        int diasPorMes[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        bool bissexto = (ano % 400 == 0) || (ano % 4 == 0 && ano % 100 != 0);
        if (bissexto) diasPorMes[1] = 29;
        return dia <= diasPorMes[mes - 1];
    }

    /**
     * @brief Operador de inserção em fluxo.
     * @param os Fluxo de saída.
     * @param dt Objeto DateTime.
     * @return Fluxo de saída atualizado.
     */
    friend ostream& operator<<(ostream& os, const DateTime& dt) {
        os << dt.dia << "-" << dt.mes << "-" << dt.ano << " " << dt.hora << ":" << dt.minuto << ":" << dt.segundo;
        return os;
    }

    /**
     * @brief Operador de extração de fluxo.
     * @param is Fluxo de entrada.
     * @param dt Objeto DateTime.
     * @return Fluxo de entrada atualizado.
     * @throws runtime_error se o formato da string for inválido.
     */
    friend istream& operator>>(istream& is, DateTime& dt) {
        string data;
        is >> data;
        size_t pos1 = data.find('-');
        size_t pos2 = data.find('-', pos1 + 1);
        size_t pos3 = data.find(' ', pos2 + 1);
        size_t pos4 = data.find(':', pos3 + 1);
        size_t pos5 = data.find(':', pos4 + 1);
        if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos || pos4 == string::npos || pos5 == string::npos)
            throw runtime_error("Formato de data inválido");
        dt.dia = stoi(data.substr(0, pos1));
        dt.mes = stoi(data.substr(pos1 + 1, pos2 - pos1 - 1));
        dt.ano = stoi(data.substr(pos2 + 1, pos3 - pos2 - 1));
        dt.hora = stoi(data.substr(pos3 + 1, pos4 - pos3 - 1));
        dt.minuto = stoi(data.substr(pos4 + 1, pos5 - pos4 - 1));
        dt.segundo = stoi(data.substr(pos5 + 1));
        if (!dt.isValid()) throw runtime_error("Data inválida");
        return is;
    }

    /**
     * @brief Operador de atribuição.
     * @param other Objeto DateTime a ser atribuído.
     * @return Referência para o objeto atribuído.
     */
    DateTime& operator=(const DateTime& other) {
        if (this != &other) {
            dia = other.dia;
            mes = other.mes;
            ano = other.ano;
            hora = other.hora;
            minuto = other.minuto;
            segundo = other.segundo;
        }
        return *this;
    }

    /**
     * @brief Operador de igualdade.
     * @param other Objeto DateTime a ser comparado.
     * @return true se os objetos forem iguais; false caso contrário.
     */
    bool operator==(const DateTime& other) const {
        return (ano == other.ano) && (mes == other.mes) && (dia == other.dia) && (hora == other.hora) && (minuto == other.minuto) && (segundo == other.segundo);
    }

    /**
     * @brief Operador "menor que".
     * @param other Objeto DateTime a ser comparado.
     * @return true se este objeto for anterior a other; false caso contrário.
     */
    bool operator<(const DateTime& other) const {
        if (ano != other.ano) return ano < other.ano;
        else if (mes != other.mes) return mes < other.mes;
        else if (dia != other.dia) return dia < other.dia;
        else if (hora != other.hora) return hora < other.hora;
        else if (minuto != other.minuto) return minuto < other.minuto;
        else return segundo < other.segundo;
    }

    /**
     * @brief Operador "maior que".
     * @param other Objeto DateTime a ser comparado.
     * @return true se este objeto for posterior a other; false caso contrário.
     */
    bool operator>(const DateTime& other) const {
        return other < *this;
    }

    /**
     * @brief Operador de desigualdade.
     * @param other Objeto DateTime a ser comparado.
     * @return true se os objetos forem diferentes; false caso contrário.
     */
    bool operator!=(const DateTime& other) const {
        return !(*this == other);
    }

    /**
     * @brief Operador "menor ou igual".
     * @param other Objeto DateTime a ser comparado.
     * @return true se este objeto for anterior ou igual a other; false caso contrário.
     */
    bool operator<=(const DateTime& other) const {
        return !(*this > other);
    }

    /**
     * @brief Operador "maior ou igual".
     * @param other Objeto DateTime a ser comparado.
     * @return true se este objeto for posterior ou igual a other; false caso contrário.
     */
    bool operator>=(const DateTime& other) const {
        return !(*this < other);
    }
};

/**
 * @brief Implementação do construtor de conversão de DateTime para DateDay.
 *
 * Converte um objeto DateTime em um DateDay, descartando as informações de tempo.
 * @param dt Objeto DateTime a ser convertido.
 * @throws runtime_error se a data resultante for inválida.
 */
DateDay::DateDay(const DateTime& dt) : dia(dt.dia), mes(dt.mes), ano(dt.ano) {
    if (!isValid())
        throw runtime_error("Data inválida");
}

#define DTYPES int, double, string, bool, char, DateDay, DateTime
#define VDTYPES variant<DTYPES>

/**
 * @brief Mapeia os tipos de dados para suas representações em string.
 */
const map<string, string> TYPEMAP = {
    {typeid(int).name(), "int"},
    {typeid(double).name(), "double"},
    {typeid(string).name(), "string"},
    {typeid(bool).name(), "bool"},
    {typeid(char).name(), "char"},
    {typeid(DateDay).name(), "dateday"},
    {typeid(DateTime).name(), "datetime"}
};

/**
 * @class Series
 * @brief Representa uma coluna de um DataFrame, contendo um nome, um tipo e um vetor de dados.
 * @tparam T Tipo dos dados armazenados na Series
 */
template <typename T>
class Series {
private:
    string strColumnName;  ///< Nome da coluna
    string strColumnType;  ///< Tipo da coluna
    vector<T> vecColumnData; ///< Dados armazenados na coluna

public:
    /**
     * @brief Construtor da classe Series
     * @param columnName Nome da coluna
     * @param columnType Tipo da coluna
     */
    Series(const string& columnName, const string& columnType) : strColumnName(columnName), strColumnType(columnType) {}
    
    /**
     * @brief Construtor básico apenas com nome
     * @param columnName Nome da coluna
     */
    Series(const string& columnName) : strColumnName(columnName) {
        // Determinar tipo automaticamente baseado em T
        if constexpr (is_same_v<T, int>)
            strColumnType = "int";
        else if constexpr (is_same_v<T, double>)
            strColumnType = "double";
        else if constexpr (is_same_v<T, string>)
            strColumnType = "string";
        else if constexpr (is_same_v<T, bool>)
            strColumnType = "bool";
        else if constexpr (is_same_v<T, char>)
            strColumnType = "char";
        else if constexpr (is_same_v<T, DateDay>)
            strColumnType = "dateday";
        else if constexpr (is_same_v<T, DateTime>)
            strColumnType = "datetime";
        else
            strColumnType = "any";
    }

    /**
     * @brief Construtor completo com nome e dados
     * @param columnName Nome da coluna
     * @param data Vetor de dados
     */
    Series(const string& columnName, const vector<T>& data) : 
        strColumnName(columnName), vecColumnData(data) {
        // Determinar tipo automaticamente
        if constexpr (is_same_v<T, int>)
            strColumnType = "int";
        else if constexpr (is_same_v<T, double>)
            strColumnType = "double";
        else if constexpr (is_same_v<T, string>)
            strColumnType = "string";
        else if constexpr (is_same_v<T, bool>)
            strColumnType = "bool";
        else if constexpr (is_same_v<T, char>)
            strColumnType = "char";
        else if constexpr (is_same_v<T, DateDay>)
            strColumnType = "dateday";
        else if constexpr (is_same_v<T, DateTime>)
            strColumnType = "datetime";
        else
            strColumnType = "any";
    }
    
    /**
     * @brief Construtor padrão
     */
    Series() : strColumnName(""), strColumnType("") {}

    /**
     * @brief Construtor de cópia da classe Series
     * @param other Objeto Series a ser copiado
     */
    Series(const Series& other) : 
        strColumnName(other.strColumnName), 
        strColumnType(other.strColumnType), 
        vecColumnData(other.vecColumnData) {}

    /**
     * @brief Construtor de movimento
     * @param other Objeto Series a ser movido
     */
    Series(Series&& other) noexcept : 
        strColumnName(std::move(other.strColumnName)), 
        strColumnType(std::move(other.strColumnType)), 
        vecColumnData(std::move(other.vecColumnData)) {}

    /**
     * @brief Destrutor virtual padrão
     */
    virtual ~Series() = default;

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
    const vector<T>& getData() const {
        return vecColumnData;
    }

    /**
     * @brief Sobrecarga do operador de inserção para exibir a Series.
     * @param os Fluxo de saída.
     * @param series Objeto Series a ser exibido.
     * @return Fluxo de saída atualizado.
     */
    friend ostream& operator<<(ostream& os, const Series& series) {
        os << series.strColumnName << " <" << series.strColumnType << ">: [";
        
        size_t limit = min(series.vecColumnData.size(), static_cast<size_t>(5));
        
        for (size_t i = 0; i < limit; ++i) {
            if constexpr (is_same_v<T, string>) { 
                os << '"' << series.vecColumnData[i] << '"'; 
            } else {
                os << series.vecColumnData[i];
            }

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
    Series& operator=(const Series& other) {
        if (this != &other) {
            strColumnName = other.strColumnName;
            strColumnType = other.strColumnType;
            vecColumnData = other.vecColumnData;
        }
        return *this;
    }

    /**
     * @brief Sobrecarga do operador de atribuição por movimento
     * @param other Objeto Series a ser movido
     * @return Referência para o objeto atual
     */
    Series& operator=(Series&& other) noexcept {
        if (this != &other) {
            strColumnName = std::move(other.strColumnName);
            strColumnType = std::move(other.strColumnType);
            vecColumnData = std::move(other.vecColumnData);
        }
        return *this;
    }

    /**
     * @brief Adiciona um elemento à coluna
     * @param elemento Elemento a ser adicionado
     * @return true se a operação for bem-sucedida, false caso contrário
     */
    bool bAdicionaElemento(const T& elemento) {
        try {
            vecColumnData.push_back(elemento);
            return true;
        } catch (...) {
            cerr << "Falha ao adicionar elemento." << endl;
            return false;
        }
    }

    /**
     * @brief Adiciona um elemento à coluna (versão para movimento)
     * @param elemento Elemento a ser adicionado
     * @return true se a operação for bem-sucedida, false caso contrário
     */
    bool bAdicionaElemento(T&& elemento) {
        try {
            vecColumnData.push_back(std::move(elemento));
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
     * @brief Remove um elemento da coluna com base no índice.
     * @param iIndex O índice do elemento que deve ser removido.
     * @return true se a remoção for bem-sucedida, false caso contrário.
     */
    bool bRemovePeloIndex(int iIndex) {
        if (iIndex < 0 || iIndex >= static_cast<int>(vecColumnData.size())) {
            cerr << "Índice fora dos limites." << endl;
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
    const T& retornaElemento(int iIndex) const {
        if (iIndex >= 0 && iIndex < static_cast<int>(vecColumnData.size())) {
            return vecColumnData[iIndex];
        } else {
            throw out_of_range("Índice fora dos limites: " + to_string(iIndex));
        }
    }

    /**
     * @brief Empilha duas Series horizontalmente, desde que tenham o mesmo tipo.
     * @param other A outra Series a ser empilhada.
     */
    void hStack(const Series<T>& other) {
        if (this->strGetType() != other.strGetType()) {
            cout << "Tipos de colunas diferentes. Não é possível empilhar." << endl;
            return;
        }

        this->vecColumnData.insert(this->vecColumnData.end(), other.vecColumnData.begin(), other.vecColumnData.end());
    }

    /**
     * @brief Método que permite adicionar um elemento como no exemplo fornecido
     * @param value Valor a ser adicionado
     */
    void addData(const T& value) {
        bAdicionaElemento(value);
    }

    /**
     * @brief Método que permite adicionar um elemento por movimento
     * @param value Valor a ser movido para a série
     */
    void addData(T&& value) {
        bAdicionaElemento(std::move(value));
    }
};

// Manter a classe Series original para compatibilidade
using SeriesLegacy = Series<VDTYPES>;

#endif