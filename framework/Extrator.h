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
    Dataframe dfExtratorCSVFilho(const string& strBlocoDeTextoCSV){
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
    void vExtratorCSVPai(int iTamanhoBatch){
        // Vou chamar o método que cria as colunas e salva em strColumnsName
        vContrutorVetorColunas();

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
                Dataframe dfAuxiliar = dfExtratorCSVFilho(strBlocoDeTextoCSV);
                vctDataframes.push_back(dfAuxiliar);
                strBlocoDeTextoCSV.clear(); 
            }
            iContador++;
        }

        // Adiciona o último bloco, se houver
        if (!strBlocoDeTextoCSV.empty()) { 
            Dataframe dfAuxiliar = dfExtratorCSVFilho(strBlocoDeTextoCSV);
            vctDataframes.push_back(dfAuxiliar);
        }
    }

    /**
     * @brief Extrai os nomes das colunas do arquivo CSV.
     */
    void vContrutorVetorColunas() {
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
    void vConstrutorDataframeBruto() {
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
    // Implementação temporária para simular a queue de threads
    vector<Dataframe> vctDataframes;

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
     * @brief Constrói um DataFrame a partir de um bloco de texto SQL.
     */
    Dataframe dfExtratorSQLFilho(const string& strBlocoDeTextoSQL) {
        Dataframe dfAuxiliar;

        // As colunas do DataFrame auxiliar são as mesmas do DataFrame original
        dfAuxiliar.vstrColumnsName = strColumnsName;

        // Vou criar o esboço do DataFrame auxiliar
        for (auto col : strColumnsName){
            Series auxSerie(col, "string");
            dfAuxiliar.columns.push_back(auxSerie);
        }

        // Agora vou preencher o DataFrame auxiliar com os dados do bloco de texto
        stringstream ss(strBlocoDeTextoSQL);
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
     * @brief Extrai dados do banco de dados em blocos de tamanho especificado.
     */
    void vExtratorCSVPai(int iTamanhoBatch, const string& nomeTabela) {
        // Chama o método que extrai as colunas da tabela
        vContrutorVetorColunas(nomeTabela);
    
        // Consulta SQL para selecionar todos os dados da tabela
        string sql = "SELECT * FROM " + nomeTabela + ";"; 
        sqlite3_stmt* stmt;
        int iContador = 0;
        string strBlocoDeTextoSQL;
        
        if (sqlite3_prepare_v2(bancoDeDados, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                string line;
    
                // Constrói uma linha de dados separada por vírgula
                for (size_t i = 0; i < strColumnsName.size(); ++i) {
                    const char* valor = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    line += valor ? valor : "NULL";
                    if (i < strColumnsName.size() - 1) {
                        line += ",";  // Adiciona vírgula apenas entre os valores
                    }
                }
    
                strBlocoDeTextoSQL += line + "\n"; // Adiciona a linha ao bloco
    
                // Quando atinge o tamanho do batch, processa o bloco
                if (++iContador % iTamanhoBatch == 0) {
                    Dataframe dfAuxiliar = dfExtratorSQLFilho(strBlocoDeTextoSQL);
                    vctDataframes.push_back(dfAuxiliar);
                    strBlocoDeTextoSQL.clear();  // Reseta o bloco de texto
                }
            }
    
            // Se ainda houver dados pendentes no bloco, processa o restante
            if (!strBlocoDeTextoSQL.empty()) {
                Dataframe dfAuxiliar = dfExtratorSQLFilho(strBlocoDeTextoSQL);
                vctDataframes.push_back(dfAuxiliar);
            }
    
            sqlite3_finalize(stmt);
        } else {
            cerr << "Erro ao preparar consulta SQL." << endl;
        }
    }
    
    /**
     * @brief Extrai os nomes e tipos das colunas de uma tabela específica.
     * 
     * @param nomeTabela Nome da tabela a ser extraída.
     */
    void vContrutorVetorColunas(const string& nomeTabela) {
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
    void vConstrutorDataframeBruto(const string& nomeTabela) {
        for (size_t i = 0; i < strColumnsName.size(); ++i) {
            const string& coluna = strColumnsName[i];  
            const string& tipo = strColumnsType[i];   

            string sql = "SELECT " + coluna + " FROM " + nomeTabela + ";";
            sqlite3_stmt* stmt;

            if (sqlite3_prepare_v2(bancoDeDados, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                Series auxSerie(coluna, "string");

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
