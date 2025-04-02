#ifndef DATAFRAME_H
#define DATAFRAME_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <numeric>

using namespace std;

struct Datetime {
    int dia;
    int mes;
    int ano;
    
    Datetime(int d, int m, int a) : dia(d), mes(m), ano(a) {
        if (!isValid()) throw runtime_error("Data inválida");
    }
    
    Datetime() : dia(0), mes(0), ano(0) {}
    
    Datetime(const Datetime& dt) : dia(dt.dia), mes(dt.mes), ano(dt.ano) {}
    
    Datetime(const string& data) {
        size_t pos1 = data.find('-');
        size_t pos2 = data.find('-', pos1 + 1);
        dia = stoi(data.substr(0, pos1));
        mes = stoi(data.substr(pos1 + 1, pos2 - pos1 - 1));
        ano = stoi(data.substr(pos2 + 1));
        if (!isValid()) throw runtime_error("Data inválida");
    }

    ~Datetime() = default;
    
    bool isValid() const {
        if (ano < 1 || mes < 1 || mes > 12 || dia < 1) return false;
        int diasPorMes[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        bool bissexto = (ano % 400 == 0) || (ano % 4 == 0 && ano % 100 != 0);
        if (bissexto) diasPorMes[1] = 29;
        return dia <= diasPorMes[mes - 1];
    }

    friend ostream& operator<<(ostream& os, const Datetime& dt) {
        os << dt.dia << "-" << dt.mes << "-" << dt.ano;
        return os;
    }

    friend istream& operator>>(istream& is, Datetime& dt) {
        string data;
        is >> data;
        size_t pos1 = data.find('-');
        size_t pos2 = data.find('-', pos1 + 1);
        dt.dia = stoi(data.substr(0, pos1));
        dt.mes = stoi(data.substr(pos1 + 1, pos2 - pos1 - 1));
        dt.ano = stoi(data.substr(pos2 + 1));
        if (!dt.isValid()) throw runtime_error("Data inválida");
        return is;
    }

    Datetime& operator=(const Datetime& other) {
        if (this != &other) {
            dia = other.dia;
            mes = other.mes;
            ano = other.ano;
        }
        return *this;
    }
};

#define DTYPES int, double, string, bool, char, Datetime
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
    {typeid(Datetime).name(), "datetime"}
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
