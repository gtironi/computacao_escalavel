#ifndef BASE_CLASSES_H
#define BASE_CLASSES_H
#include "Buffer.h"
#include "Dataframe.h"
#include "TaskQueue.h"
#include <utility> // Para std::forward
#include <tuple>
#include <optional>
#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <any>
#include "Series.h"
#include <any>

using namespace std;

/**
 * @brief Classe base para extratores de dados.
 */
template <typename T>
class Extrator
{
protected:
    vector<string> strColumnsName;
    Buffer<Dataframe> outputBuffer;
    TaskQueue *taskqueue = nullptr;
    string strFilesFlag;
    int iTamanhoBatch;
    ifstream file;
    sqlite3 *bancoDeDados;
    string strNomeTabela;

public:
    /**
     * @brief Construtor padrão.
     */
    Extrator(const string &strFilesPath, const string &strFilesFlag, int iTamanhoBatch, const string &strNomeTabela = "Nada")
    {
        this->strFilesFlag = strFilesFlag;
        this->iTamanhoBatch = iTamanhoBatch;
        this->strNomeTabela = strNomeTabela;

        if (this->strFilesFlag == "csv")
        {
            this->file = ifstream(strFilesPath);
            if (!this->file.is_open())
            {
                throw runtime_error("Falha ao abrir o arquivo.");
            }
            else
            {
                lerCabecalhoCSV();
            }
        }

        else if (this->strFilesFlag == "sql")
        {
            int exit = sqlite3_open(strFilesPath.c_str(), &this->bancoDeDados);
            if (exit)
            {
                throw runtime_error("Erro ao abrir o banco de dados: " + string(sqlite3_errmsg(this->bancoDeDados)));
            }
            else
            {
                lerCabecalhoSQL(this->strNomeTabela);
            }
        }
    };

    /**
     * @brief Destrutor padrão.
     */
    virtual ~Extrator()
    {
        if (this->strFilesFlag == "csv")
        {
            if (this->file.is_open())
            {
                this->file.close();
            }
        }
        else if (this->strFilesFlag == "sql")
        {
            if (this->bancoDeDados)
            {
                sqlite3_close(this->bancoDeDados);
            }
        }
    };

    /**
     * @brief Retorna o ponteiro da fila de tarefas.
     * @return Ponteiro para TaskQueue.
     */
    TaskQueue *get_taskqueue() const { return taskqueue; }

    /**
     * @brief Define o ponteiro da fila de tarefas.
     * @param tq Ponteiro para TaskQueue.
     */
    void set_taskqueue(TaskQueue *tq) { taskqueue = tq; }

    /**
     * @brief Retorna referência para o buffer de saída.
     * @return Referência ao Buffer de Dataframe.
     */
    Buffer<T> &get_output_buffer() { return outputBuffer; }

    /**
     * @brief Retorna a flag dos arquivos (csv ou sql).
     * @return String representando o tipo de arquivo.
     */
    string getFilesFlag() const { return this->strFilesFlag; }

    /**
     * @brief Retorna o tamanho do batch.
     * @return Tamanho do batch.
     */
    int getBatchSize() const { return this->iTamanhoBatch; }

    /**
     * @brief Define a fila de tarefas.
     * @param tq Ponteiro para TaskQueue.
     */
    void setTaskQueue(TaskQueue *tq) { this->taskqueue = tq; }

    /**
     * @brief Define a flag dos arquivos.
     * @param strFilesFlag Valor que indica o tipo de arquivo.
     */
    void setFilesFlag(const string &strFilesFlag) { this->strFilesFlag = strFilesFlag; }

    /**
     * @brief Define o tamanho do batch.
     * @param iTamanhoBatch Tamanho do batch.
     */
    void setBatchSize(int iTamanhoBatch) { this->iTamanhoBatch = iTamanhoBatch; }

    /**
     * @brief Lê o cabeçalho de um arquivo CSV e popula o vetor de nomes de colunas.
     */
    void lerCabecalhoCSV()
    {
        string line;
        if (getline(this->file, line))
        {
            stringstream ss(line);
            string cell;

            while (getline(ss, cell, ','))
            {
                strColumnsName.push_back(cell);
            }
        }
        else
        {
            cerr << "Erro ao ler o cabeçalho do CSV." << endl;
        }
    }

    /**
     * @brief Lê o cabeçalho de uma tabela SQL e popula o vetor de nomes de colunas.
     * @param strNomeTabela Nome da tabela no banco de dados.
     */
    void lerCabecalhoSQL(const string &strNomeTabela)
    {
        string sql = "PRAGMA table_info(" + strNomeTabela + ");";
        sqlite3_stmt *stmt;
        this->strColumnsName.clear();

        if (sqlite3_prepare_v2(this->bancoDeDados, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                this->strColumnsName.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));
            }
            sqlite3_finalize(stmt);
        }
        else
        {
            cerr << "Erro ao preparar consulta SQL." << endl;
        }
    }

    /**
     * @brief Enfileira as tarefas de extração dos dados conforme o tipo de arquivo.
     */
    void enqueue_tasks()
    {
        string strBlocoDeTexto;
        int iContador = 0;

        if (this->strFilesFlag == "csv")
        {
            string line;
            while (getline(file, line))
            {
                iContador++;
                strBlocoDeTexto += line + "\n";
                if (iContador % this->iTamanhoBatch == 0)
                {
                    string value = strBlocoDeTexto;
                    taskqueue->push_task([this, val = value]() mutable
                                         { this->create_task(val); });
                    this->outputBuffer.get_semaphore().wait();
                    strBlocoDeTexto.clear();
                }
            }

            // Adiciona o último bloco, se houver
            if (!strBlocoDeTexto.empty())
            {
                string value = strBlocoDeTexto;
                taskqueue->push_task([this, val = value]() mutable
                                     { this->create_task(val); });
                this->outputBuffer.get_semaphore().wait();
            }
        }
        else if (this->strFilesFlag == "sql")
        {
            string sql = "SELECT * FROM " + this->strNomeTabela + ";";
            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(this->bancoDeDados, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
            {
                while (sqlite3_step(stmt) == SQLITE_ROW)
                {
                    string line;

                    // Constrói uma linha de dados separada por vírgula
                    for (size_t i = 0; i < this->strColumnsName.size(); ++i)
                    {
                        const char *valor = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
                        line += valor ? valor : "NULL";
                        if (i < this->strColumnsName.size() - 1)
                        {
                            line += ","; // Adiciona vírgula apenas entre os valores
                        }
                    }

                    strBlocoDeTexto += line + "\n"; // Adiciona a linha ao bloco

                    // Quando atinge o tamanho do batch, processa o bloco
                    if (iContador++ % this->iTamanhoBatch == 0)
                    {
                        string value = strBlocoDeTexto;
                        taskqueue->push_task([this, val = value]() mutable
                                             { this->create_task(val); });
                        strBlocoDeTexto.clear();
                    }
                }

                // Se ainda houver dados pendentes no bloco, processa o restante
                if (!strBlocoDeTexto.empty())
                {
                    string value = strBlocoDeTexto;
                    taskqueue->push_task([this, val = value]() mutable
                                         { this->create_task(val); });
                    this->outputBuffer.get_semaphore().wait();
                }

                sqlite3_finalize(stmt);
            }
            else
            {
                throw runtime_error("Erro ao preparar consulta SQL.");
            }
        }

        // Avisa ao buffer de saída que os dados acabaram
        finishBuffer();
    }

    /**
     * @brief Executa o processo de extração de dados a partir de um bloco de texto.
     * @param strTextBlock Bloco de texto com dados CSV ou extraídos do SQL.
     * @return Resultado do método dfSubExtractor.
     */
    T run(const string &strTextBlock)
    {
        return dfSubExtractor(strTextBlock);
    }

    /**
     * @brief Constrói um DataFrame a partir de um bloco de texto CSV.
     *
     * @param strBlocoDeTexto Bloco de texto CSV.
     * @return DataFrame construído a partir do bloco de texto.
     */
    Dataframe dfSubExtractor(const string &strBlocoDeTexto)
    {
        Dataframe dfAuxiliar;
        dfAuxiliar.vstrColumnsName = this->strColumnsName;

        // Preparar as colunas
        for (const auto &col : this->strColumnsName)
        {
            dfAuxiliar.columns.emplace_back(col, "string");
        }

        stringstream ss(strBlocoDeTexto);
        string line;

        // Estimar o número de linhas para pré-alocar espaço
        size_t estimatedRows = count(strBlocoDeTexto.begin(), strBlocoDeTexto.end(), '\n') + 1;
        for (auto &col : dfAuxiliar.columns)
        {
            col.reserve(estimatedRows);
        }

        while (getline(ss, line))
        {
            stringstream ssLine(line);
            string cell;
            vector<any> convertedRow;
            convertedRow.reserve(dfAuxiliar.vstrColumnsName.size());

            size_t colIndex = 0;
            while (getline(ssLine, cell, ','))
            {
                // Adiciona a célula diretamente como string
                convertedRow.emplace_back(cell);
                colIndex++;
            }

            // Completa a linha se necessário
            while (colIndex < dfAuxiliar.vstrColumnsName.size())
            {
                convertedRow.emplace_back(string(""));
                colIndex++;
            }

            dfAuxiliar.adicionaLinha(convertedRow);
        }

        return dfAuxiliar;
    }

    /**
     * @brief Cria uma tarefa a partir de um bloco de texto e realiza a extração dos dados.
     * @param value Bloco de texto a ser processado.
     */
    void create_task(const string &value)
    {
        T data = run(value);
        this->outputBuffer.push(data);
    }

    /**
     * @brief Retorna os nomes das colunas extraídas.
     * @return Vetor de strings com os nomes das colunas.
     */
    vector<string> getColumnsName() const
    {
        return strColumnsName;
    }

    /**
     * @brief Conclui o buffer de saída sinalizando que não há mais tarefas.
     */
    void finishBuffer()
    {
        outputBuffer.finalizeInput();
    }

};

// Classe base genérica para carregadores (última etapa do pipeline)
// T: Tipo dos dados que serão consumidos (ex: DataFrame, estrutura customizada, etc.)
template <typename T>
class Loader
{
protected:
    // Buffer de entrada do qual os dados serão consumidos
    Buffer<T> &input_buffer;

    // Ponteiro para a fila de tarefas responsável pela execução concorrente
    TaskQueue *taskqueue = nullptr;

public:
    /**
     * @brief Construtor do Loader
     * @param buffer Referência ao buffer de entrada.
     */
    explicit Loader(Buffer<T> &buffer) : input_buffer(buffer) {}

    /**
     * @brief Destrutor virtual do Loader.
     */
    virtual ~Loader() = default;

    /**
     * @brief Define a fila de tarefas a ser utilizada.
     * @param tq Ponteiro para TaskQueue.
     */
    void set_taskqueue(TaskQueue *tq) { taskqueue = tq; }

    /**
     * @brief Retorna o ponteiro da fila de tarefas.
     * @return Ponteiro para TaskQueue.
     */
    TaskQueue *get_taskqueue() const { return taskqueue; }

    /**
     * Método responsável por enfileirar as tarefas de carregamento.
     * Extrai dados do buffer de entrada e os envia para a fila de execução paralela.
     */
    void enqueue_tasks()
    {
        // Enquanto o buffer de entrada ainda não indicou o fim dos dados
        while (!(input_buffer.atomicGetInputDataFinished()))
        {

            // Tenta extrair um dado do buffer
            std::optional<T> maybe_value = input_buffer.pop(false, true);


            // Se não conseguir pegar nenhum dado (buffer vazio no momento), encerra o loop
            if (!maybe_value.has_value())
            {
                break;
            }

            // Move o valor extraído
            T value = std::move(*maybe_value);


            // Enfileira a tarefa na fila de execução, chamando o método `create_task`
            taskqueue->push_task([this, val = std::move(value)]() mutable
                                 { this->create_task(std::move(val)); });
        }

        // Quando termina de consumir todos os dados:
        // Decrementa o número de loaders ativos
        

        taskqueue->getNumberOfLoaders().wait();

        // Notifica a fila de que este loader terminou
        taskqueue->notifyAll();
    }

    /**
     * @brief Método virtual responsável por executar o carregamento final dos dados.
     * @param value Dado a ser carregado.
     */
    virtual void run(T value) = 0;

    /**
     * @brief Método auxiliar que chama o método run.
     * @param value Dado a ser carregado.
     */
    void create_task(T value)
    {
        run(std::move(value));
    }
};

#endif
