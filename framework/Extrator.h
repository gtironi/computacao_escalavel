#ifndef EXTRATOR_H
#define EXTRATOR_H

#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include "Series.h"
#include "Dataframe.h"

using namespace std;

/**
 * @brief Converte um tipo SQL para seu equivalente em C++.
 * @param sqlType Tipo SQL (ex: "VARCHAR(20)", "DECIMAL(10,2)", "INT").
 * @return Tipo equivalente em C++ (ex: "string", "float", "int").
 */
string mapSQLToCpp(const string& sqlType) {
    static map<string, string> typeMap = {
        {"INT", "int"},
        {"INTEGER", "int"},
        {"BIGINT", "long long"},
        {"SMALLINT", "short"},
        {"TINYINT", "char"},
        {"FLOAT", "float"},
        {"REAL", "double"},
        {"DOUBLE", "double"},
        {"DECIMAL", "float"},   
        {"NUMERIC", "float"},   
        {"BOOLEAN", "bool"},
        {"TEXT", "string"},
        {"CHAR", "string"},
        {"VARCHAR", "string"},
        {"DATE", "string"},
        {"DATETIME", "string"},
        {"TIMESTAMP", "string"}
    };

    regex varcharRegex(R"(VARCHAR\(\d+\))");
    regex charRegex(R"(CHAR\(\d+\))");
    regex decimalRegex(R"(DECIMAL\(\d+,\d+\))");
    regex numericRegex(R"(NUMERIC\(\d+,\d+\))");

    if (regex_match(sqlType, varcharRegex) || regex_match(sqlType, charRegex)) {
        return "string"; 
    } 
    if (regex_match(sqlType, decimalRegex) || regex_match(sqlType, numericRegex)) {
        return "float"; 
    }

    return typeMap.count(sqlType) ? typeMap[sqlType] : "Desconhecido";
}

/**
 * @brief Classe base para extratores de dados.
 */
class Extrator {
protected:
    vector<string> strColumnsName; 
    vector<string> strColumnsType; 
    Dataframe df; 

public:
    Extrator() = default;
    virtual ~Extrator() = default;

    Dataframe getDataframe() const {
        return df;
    }

    vector<string> getColumnsName() const {
        return strColumnsName;
    }

    vector<string> getColumnsType() const {
        return strColumnsType;
    }
};

/**
 * @brief Classe para extrair dados de arquivo CSV.
 */
class ExtratorCSV : public Extrator {
private:
    ifstream file; 

public:
    explicit ExtratorCSV(const string& strPathToCSV) {
        file.open(strPathToCSV);
        if (!file.is_open()) {
            cerr << "Falha ao abrir o arquivo: " << strPathToCSV << endl;
        }
    }

    ~ExtratorCSV() override {
        if (file.is_open()) { 
            file.close();
            cout << "Arquivo fechado com sucesso." << endl;
        }
    }

    void ExtratorColunas() {
        string line;
        if (getline(file, line)) { 
            stringstream ss(line);
            string cell;

            while (getline(ss, cell, ',')) {
                strColumnsName.push_back(cell); 
            }
        } else {
            cerr << "Erro ao ler o cabeçalho do CSV." << endl;
        }
    }

    void ConstrutorDataframe() {
        for (const string& col : strColumnsName) {
            Series auxSerie(col, "string");
            df.adicionaColuna(auxSerie);
        }

        string line;
        bool isFirstRow = true; 

        while (getline(file, line)) {
            if (isFirstRow) { 
                isFirstRow = false; 
                continue;
            }

            stringstream ss(line);
            string cell;
            vector<VDTYPES> convertedRow;

            size_t colIndex = 0;
            while (getline(ss, cell, ',')) {
                convertedRow.push_back(cell); 
                colIndex++;
            }

            df.adicionaLinha(convertedRow); 
        }

        for (auto &col : df.columns) {
            col.AjustandoType();
        }
    }
};

/**
 * @brief Classe para extrair dados de um banco de dados SQLite.
 */
class ExtratorSQL : public Extrator {
private:
    sqlite3* bancoDeDados;

public:
    explicit ExtratorSQL(const string& strPathToBd) : bancoDeDados(nullptr) {
        int exit = sqlite3_open(strPathToBd.c_str(), &bancoDeDados);
        if (exit) {
            cerr << "Erro ao abrir o banco de dados: " << sqlite3_errmsg(bancoDeDados) << endl;
        } else {
            cout << "Banco de dados aberto com sucesso." << endl;
        }
    }

    ~ExtratorSQL() override {
        if (bancoDeDados) {
            sqlite3_close(bancoDeDados);
            cout << "Banco de dados fechado com sucesso." << endl;
        }
    }

    void ExtratorColunas(const string& nomeTabela) {
        string sql = "PRAGMA table_info(" + nomeTabela + ");";
        sqlite3_stmt* stmt;
        strColumnsName.clear();
        strColumnsType.clear();

        if (sqlite3_prepare_v2(bancoDeDados, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                strColumnsName.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
                strColumnsType.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
            }
            sqlite3_finalize(stmt);
        } else {
            cerr << "Erro ao preparar consulta SQL." << endl;
        }
    }

    void ConstrutorDataframe(const string& nomeTabela) {
        for (size_t i = 0; i < strColumnsName.size(); ++i) {
            const string& coluna = strColumnsName[i];  
            const string& tipo = strColumnsType[i];   

            string sql = "SELECT " + coluna + " FROM " + nomeTabela + ";";
            sqlite3_stmt* stmt;

            if (sqlite3_prepare_v2(bancoDeDados, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                Series auxSerie(coluna, mapSQLToCpp(tipo));

                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    const char* valor = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                    auxSerie.bAdicionaElemento(valor ? valor : "NULL");
                }

                df.adicionaColuna(auxSerie);
                sqlite3_finalize(stmt);
            } else {
                cerr << "Erro ao preparar consulta SQL para coluna: " << coluna << endl;
            }
        }

        for (auto &col : df.columns) {
            col.AjustandoType();
        }
    }
};

#endif
