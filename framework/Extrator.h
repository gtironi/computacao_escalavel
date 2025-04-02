#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include <regex>

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
        {"TEXT", "std::string"},
        {"CHAR", "std::string"},
        {"VARCHAR", "std::string"},
        {"DATE", "std::string"},
        {"DATETIME", "std::string"},
        {"TIMESTAMP", "std::string"}
    };

    regex varcharRegex(R"(VARCHAR\(\d+\))");
    regex charRegex(R"(CHAR\(\d+\))");
    regex decimalRegex(R"(DECIMAL\(\d+,\d+\))");
    regex numericRegex(R"(NUMERIC\(\d+,\d+\))");

    if (std::regex_match(sqlType, varcharRegex) || std::regex_match(sqlType, charRegex)) {
        return "std::string"; 
    } 
    if (std::regex_match(sqlType, decimalRegex) || std::regex_match(sqlType, numericRegex)) {
        return "float"; 
    }

    return typeMap.count(sqlType) ? typeMap[sqlType] : "Desconhecido";
}

/**
 * @brief Classe base para extratores de dados.
 */
class Extrator {
    public:
        Extrator() = default;
        virtual ~Extrator() = default;
};

/**
 * @brief Classe para extrair dados de um banco de dados SQLite.
 */
class ExtratorSQL : public Extrator {
    private:
        sqlite3* bancoDeDados;
        vector<string> strColumnsName;
        vector<string> strColumnsType;
    public:
        /**
         * @brief Construtor que abre a conexão com o banco de dados SQLite.
         * @param strPathToBd Caminho para o arquivo do banco de dados.
         */
        ExtratorSQL(const std::string& strPathToBd) : bancoDeDados(nullptr) {
            int exit = sqlite3_open(strPathToBd.c_str(), &bancoDeDados);
            if (exit) {
                std::cerr << "Erro ao abrir o banco de dados: " << sqlite3_errmsg(bancoDeDados) << std::endl;
            } else {
                std::cout << "Banco de dados aberto com sucesso." << std::endl;
            }
        }

        /**
         * @brief Destrutor que fecha a conexão com o banco de dados.
         */
        ~ExtratorSQL() {
            if (bancoDeDados) {
                sqlite3_close(bancoDeDados);
                std::cout << "Banco de dados fechado com sucesso." << std::endl;
            }
        }

        /**
         * @brief Retorna o vetor com os nomes das colunas.
         */
        vector<string> getColumnsName(){
            return strColumnsName;
        }

        /**
         * @brief Retorna o vetor com os tipos das colunas.
         */
        vector<string> getColumnsType(){
            return strColumnsType;
        }

        /**
         * @brief Extrai os nomes das colunas e seus tipos do banco de dados SQLite.
         * @param nomeTabela Nome da tabela a ser analisada.
         */
        void ExtratorColunas(const std::string& nomeTabela) {
            std::string sql = "PRAGMA table_info(" + nomeTabela + ");";
            sqlite3_stmt* stmt;

            if (sqlite3_prepare_v2(bancoDeDados, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    // nome
                    strColumnsName.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));

                    // tipo
                    strColumnsType.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
                }
                sqlite3_finalize(stmt);
            } else {
                std::cerr << "Erro ao preparar consulta SQL." << std::endl;
            }
        }
};