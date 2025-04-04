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
    /**
     * @brief Construtor padrão.
     */
    Extrator() = default;
    
    /**
     * @brief Destrutor padrão. 
     */
    virtual ~Extrator() = default;

    /**
     * @brief Método getter para o DataFrame.
     * 
     * @return DataFrame com os dados extraídos.
     */
    Dataframe getDataframe() const {
        return df;
    }

    /**
     * @brief Método getter para os nomes das colunas.
     * 
     * @return Vetor de strings com os nomes das colunas.
     */
    vector<string> getColumnsName() const {
        return strColumnsName;
    }

    /**
     * @brief Método getter para os tipos das colunas.
     * 
     * @return Vetor de strings com os tipos das colunas.
     */
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
    // Implementação temporária para simular a queue de threads
    vector<Dataframe> vctDataframes;

    /**
     * @brief Construtor da classe ExtratorCSV.
     * 
     * @param strPathToCSV Caminho para o arquivo CSV.
     */
    explicit ExtratorCSV(const string& strPathToCSV) {
        file.open(strPathToCSV);
        if (!file.is_open()) {
            cerr << "Falha ao abrir o arquivo: " << strPathToCSV << endl;
        }
    }

    /**
     * @brief Destrutor da classe ExtratorCSV.
     */
    ~ExtratorCSV() override {
        if (file.is_open()) { 
            file.close();
            cout << "Arquivo fechado com sucesso." << endl;
        }
    }

    /**
     * @brief Constrói um DataFrame a partir de um bloco de texto CSV.
     * 
     * @param strBlocoDeTextoCSV Bloco de texto CSV.
     * @return DataFrame construído a partir do bloco de texto.
     */
    Dataframe dfConstroiDataframe(const string& strBlocoDeTextoCSV){
        Dataframe dfAuxiliar;

        // As colunas do DataFrame auxiliar são as mesmas do DataFrame original
        dfAuxiliar.vstrColumnsName = strColumnsName;

        // Vou criar o esboço do DataFrame auxiliar
        for (auto col : strColumnsName){
            Series auxSerie(col, "string");
            dfAuxiliar.columns.push_back(auxSerie);
        }

        // Agora vou preencher o DataFrame auxiliar com os dados do bloco de texto
        stringstream ss(strBlocoDeTextoCSV);
        string line;
        while (getline(ss, line)) {

            stringstream ssLine(line);
            string cell;
            vector<VDTYPES> convertedRow;

            size_t colIndex = 0;
            while (getline(ssLine, cell, ',')) {
                convertedRow.push_back(cell); 
                colIndex++;
            }

            dfAuxiliar.adicionaLinha(convertedRow); 
        }

        for (auto &col : dfAuxiliar.columns) {
            col.AjustandoType();
        }

        return dfAuxiliar;
    }

    /**
     * @brief Extrai dados do arquivo CSV em blocos de tamanho especificado.
     * 
     * @param iTamanhoBatch Tamanho do bloco a ser lido.
     */
    void ExtratorThreads(int iTamanhoBatch){
        // Vou chamar o método que cria as colunas e salva em strColumnsName
        ExtratorColunas();

        // Vou iterar sobre o arquivo, separando em blocos de iTamanhoBatch linhas, criando um DataFrame para cada bloco
        string line;
        string strBlocoDeTextoCSV;
        int iContador = 0;
        while (getline(file, line)) {
            // Ignora a primeira linha (cabeçalho)
            if (iContador == 0) { 
                iContador++;
                continue; 
            }

            strBlocoDeTextoCSV += line + "\n"; 

            if (iContador % iTamanhoBatch == 0) {
                Dataframe dfAuxiliar = dfConstroiDataframe(strBlocoDeTextoCSV);
                vctDataframes.push_back(dfAuxiliar);
                strBlocoDeTextoCSV.clear(); 
            }
            iContador++;
        }

        // Adiciona o último bloco, se houver
        if (!strBlocoDeTextoCSV.empty()) { 
            Dataframe dfAuxiliar = dfConstroiDataframe(strBlocoDeTextoCSV);
            vctDataframes.push_back(dfAuxiliar);
        }
    }

    /**
     * @brief Extrai os nomes das colunas do arquivo CSV.
     */
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

    /**
     * @brief Constrói o DataFrame a partir do arquivo CSV.
     */
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
    /**
     * @brief Construtor da classe ExtratorSQL.
     * 
     * @param strPathToBd Caminho para o banco de dados SQLite.
     */
    explicit ExtratorSQL(const string& strPathToBd) : bancoDeDados(nullptr) {
        int exit = sqlite3_open(strPathToBd.c_str(), &bancoDeDados);
        if (exit) {
            cerr << "Erro ao abrir o banco de dados: " << sqlite3_errmsg(bancoDeDados) << endl;
        } else {
            cout << "Banco de dados aberto com sucesso." << endl;
        }
    }

    /**
     * @brief Destrutor da classe ExtratorSQL.
     */
    ~ExtratorSQL() override {
        if (bancoDeDados) {
            sqlite3_close(bancoDeDados);
            cout << "Banco de dados fechado com sucesso." << endl;
        }
    }
    
    /**
     * @brief Extrai os nomes e tipos das colunas de uma tabela específica.
     * 
     * @param nomeTabela Nome da tabela a ser extraída.
     */
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

    /**
     * @brief Constrói o DataFrame a partir de uma tabela específica.
     * 
     * @param nomeTabela Nome da tabela a ser extraída.
     */
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
