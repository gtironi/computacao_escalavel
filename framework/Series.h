#ifndef DATAFRAME_H
#define DATAFRAME_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <numeric>

using namespace std;

struct DateDay {
    int dia;
    int mes;
    int ano;
    
    DateDay(int d, int m, int a) : dia(d), mes(m), ano(a) {
        if (!isValid()) throw runtime_error("Data inválida");
    }
    
    DateDay() : dia(1), mes(1), ano(1) {}
    
    DateDay(const DateDay& dt) : dia(dt.dia), mes(dt.mes), ano(dt.ano) {}
    
    DateDay(const string& data) {
        size_t pos1 = data.find('-');
        size_t pos2 = data.find('-', pos1 + 1);
        if(pos1 == string::npos || pos2 == string::npos)
            throw runtime_error("Formato de data inválido");
        dia = stoi(data.substr(0, pos1));
        mes = stoi(data.substr(pos1 + 1, pos2 - pos1 - 1));
        ano = stoi(data.substr(pos2 + 1));
        if (!isValid()) throw runtime_error("Data inválida");
    }

    ~DateDay() = default;
    
    // Verifica se a data é válida
    bool isValid() const {
        if (ano < 1 || mes < 1 || mes > 12 || dia < 1) return false;
        int diasPorMes[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        bool bissexto = (ano % 400 == 0) || (ano % 4 == 0 && ano % 100 != 0);
        if (bissexto) diasPorMes[1] = 29;
        return dia <= diasPorMes[mes - 1];
    }

    friend ostream& operator<<(ostream& os, const DateDay& dt) {
        os << dt.dia << "-" << dt.mes << "-" << dt.ano;
        return os;
    }

    friend istream& operator>>(istream& is, DateDay& dt) {
        string data;
        is >> data;
        size_t pos1 = data.find('-');
        size_t pos2 = data.find('-', pos1 + 1);
        if(pos1 == string::npos || pos2 == string::npos)
            throw runtime_error("Formato de data inválido");
        dt.dia = stoi(data.substr(0, pos1));
        dt.mes = stoi(data.substr(pos1 + 1, pos2 - pos1 - 1));
        dt.ano = stoi(data.substr(pos2 + 1));
        if (!dt.isValid()) throw runtime_error("Data inválida");
        return is;
    }

    DateDay& operator=(const DateDay& other) {
        if (this != &other) {
            dia = other.dia;
            mes = other.mes;
            ano = other.ano;
        }
        return *this;
    }

    bool operator==(const DateDay& other) const {
        return (ano == other.ano) && (mes == other.mes) && (dia == other.dia);
    }

    bool operator<(const DateDay& other) const {
        if (ano != other.ano) return ano < other.ano;
        else if (mes != other.mes) return mes < other.mes;
        else return dia < other.dia;
    }

    bool operator>(const DateDay& other) const {
        return other < *this;
    }

    bool operator!=(const DateDay& other) const {
        return !(*this == other);
    }

    bool operator<=(const DateDay& other) const {
        return !(*this > other);
    }
    
    bool operator>=(const DateDay& other) const {
        return !(*this < other);
    }
};

struct DateTime {
    int dia;
    int mes;
    int ano;
    int hora;
    int minuto;
    int segundo;

    DateTime(int d, int m, int a, int h, int min, int s) : dia(d), mes(m), ano(a), hora(h), minuto(min), segundo(s) {
        if (!isValid()) throw runtime_error("Data inválida");
    }

    DateTime() : dia(1), mes(1), ano(1), hora(0), minuto(0), segundo(0) {}

    DateTime(const DateTime& dt) : dia(dt.dia), mes(dt.mes), ano(dt.ano), hora(dt.hora), minuto(dt.minuto), segundo(dt.segundo) {}

    DateTime(const DateDay& dt) : dia(dt.dia), mes(dt.mes), ano(dt.ano), hora(0), minuto(0), segundo(0) {}

    DateTime(const string& data) {
        size_t pos1 = data.find('-');
        size_t pos2 = data.find('-', pos1 + 1);
        size_t pos3 = data.find(' ', pos2 + 1);
        size_t pos4 = data.find(':', pos3 + 1);
        size_t pos5 = data.find(':', pos4 + 1);
        if(pos1 == string::npos || pos2 == string::npos || pos3 == string::npos || pos4 == string::npos || pos5 == string::npos)
            throw runtime_error("Formato de data inválido");
        dia = stoi(data.substr(0, pos1));
        mes = stoi(data.substr(pos1 + 1, pos2 - pos1 - 1));
        ano = stoi(data.substr(pos2 + 1, pos3 - pos2 - 1));
        hora = stoi(data.substr(pos3 + 1, pos4 - pos3 - 1));
        minuto = stoi(data.substr(pos4 + 1, pos5 - pos4 - 1));
        segundo = stoi(data.substr(pos5 + 1));
        if (!isValid()) throw runtime_error("Data inválida");
    }

    ~DateTime() = default;

    // Verifica se a data é válida
    bool isValid() const {
        if (ano < 1 || mes < 1 || mes > 12 || dia < 1 || hora < 0 || hora > 23 || minuto < 0 || minuto > 59 || segundo < 0 || segundo > 59) return false;
        int diasPorMes[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        bool bissexto = (ano % 400 == 0) || (ano % 4 == 0 && ano % 100 != 0);
        if (bissexto) diasPorMes[1] = 29;
        return dia <= diasPorMes[mes - 1];
    }

    friend ostream& operator<<(ostream& os, const DateTime& dt) {
        os << dt.dia << "-" << dt.mes << "-" << dt.ano << " " << dt.hora << ":" << dt.minuto << ":" << dt.segundo;
        return os;
    }

    friend istream& operator>>(istream& is, DateTime& dt) {
        string data;
        is >> data;
        size_t pos1 = data.find('-');
        size_t pos2 = data.find('-', pos1 + 1);
        size_t pos3 = data.find(' ', pos2 + 1);
        size_t pos4 = data.find(':', pos3 + 1);
        size_t pos5 = data.find(':', pos4 + 1);
        if(pos1 == string::npos || pos2 == string::npos || pos3 == string::npos || pos4 == string::npos || pos5 == string::npos)
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

    bool operator==(const DateTime& other) const {
        return (ano == other.ano) && (mes == other.mes) && (dia == other.dia) && (hora == other.hora) && (minuto == other.minuto) && (segundo == other.segundo);
    }

    bool operator<(const DateTime& other) const {
        if (ano != other.ano) return ano < other.ano;
        else if (mes != other.mes) return mes < other.mes;
        else if (dia != other.dia) return dia < other.dia;
        else if (hora != other.hora) return hora < other.hora;
        else if (minuto != other.minuto) return minuto < other.minuto;
        else return segundo < other.segundo;
    }

    bool operator>(const DateTime& other) const {
        return other < *this;
    }

    bool operator!=(const DateTime& other) const {
        return !(*this == other);
    }

    bool operator<=(const DateTime& other) const {
        return !(*this > other);
    }

    bool operator>=(const DateTime& other) const {
        return !(*this < other);
    }
};



#define DTYPES int, double, string, bool, char, DateDay
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
 */
class Series {
private:
    string strColumnName;  ///< Nome da coluna
    string strColumnType;  ///< Tipo da coluna
    vector<VDTYPES> vecColumnData; ///< Dados armazenados na coluna

public:
    /**
     * @brief Construtor da classe Series
     * @param columnName Nome da coluna
     * @param columnType Tipo da coluna
     */
    Series(const string& columnName, const string& columnType) : strColumnName(columnName), strColumnType(columnType) {}

    /**
     * @brief Altera o nome da coluna
     */
    void setName(string& strNovoNome){
        strColumnName = strNovoNome;
    }

    /**
     * @brief Retorna o nome da coluna
     */
    string strGetName() {
        return strColumnName;
    }

    /**
     * @brief Retorna o tipo da coluna
     */
    string strGetType() {
        return strColumnType;
    }

    /**
     * @brief Retorna o número de elementos da coluna
     */
    size_t iGetSize(){
        return vecColumnData.size();
    }

    /**
     * @brief Retorna o vetor de elementos da coluna
     */
    vector<VDTYPES> getData() {
        return vecColumnData;
    }

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
     * @brief Remove um elemento da coluna com base no índice.
     * @param iIndex O índice do elemento que deve ser removido.
     * @return true se a remoção for bem-sucedida, false caso contrário.
     */
    bool bRemovePeloIndex(int iIndex) {
        if (iIndex < 0 || iIndex >= vecColumnData.size()) {
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
        cout << "Column Name: " << this->strGetName() << "\n";
        cout << "Column Type: " << this->strGetType() << "\n";
        cout << "Data: ";
        for (const auto& value : vecColumnData) {
            visit([](const auto& val) { cout << val << " "; }, value);
        }
        cout << "\n";
    }

    /**
     * @brief Calcula a média dos elementos da Series, se forem numéricos.
     * @return O valor da média como float.
     * @note Essa função só funciona para Series de tipos numéricos.
     */
    float mean() {
        if (this->strGetType() == "int" || this->strGetType() == "double" || this->strGetType() == "float") {
            double sum = 0.0;
            int count = 0;

            for (const auto& val : vecColumnData) {
                std::visit([&](auto&& arg) {
                    if constexpr (std::is_arithmetic_v<std::decay_t<decltype(arg)>>) {
                        sum += arg;
                        count++;
                    }
                }, val);
            }

            return count > 0 ? sum / count : 0.0;
        } else {
            cout << "Método desenvolvido apenas para Series do tipo numérica." << endl;
            return 0.0f;
        }
    }
};

#endif
