#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>

using namespace std;

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

    public:
        vector<string> strColumnsName;
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
     * @brief Extrai os nomes das tabelas do banco de dados SQLite.
     */
    void ExtratorColunas(const std::string& nomeTabela) {
        std::string sql = "PRAGMA table_info(" + nomeTabela + ");";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(bancoDeDados, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                strColumnsName.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            }
            sqlite3_finalize(stmt);
        } else {
            std::cerr << "Erro ao preparar consulta SQL." << std::endl;
        }
    }
};