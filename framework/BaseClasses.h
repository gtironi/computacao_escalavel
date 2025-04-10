#ifndef BASE_CLASSES_H
#define BASE_CLASSES_H
#include "Buffer.h"
#include "Dataframe.h"
#include "TaskQueue.h"
#include <utility>  // Para std::forward
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
#include "Series.h"


using namespace std;

// // Classe base para todos. Todos os stages precisam de uma task queue que será gerenciada pelo manager global
// class Stage {
//     protected:
//         TaskQueue* taskqueue = nullptr;
//         // std::atomic<bool> running{true};
//     public:

//         // Função que será escrita pelo usuário ao criar uma classe que herda de algum stage
//         virtual void run() = 0;

//         // Função que é um wrapper de run que será mandado para ser executada
//         virtual void create_task() = 0;

//         void set_taskqueue(TaskQueue* tq) { taskqueue = tq; }
//         TaskQueue* get_taskqueue() const { return taskqueue; }

//         virtual ~Stage() = default;

// };

/**
 * @brief Classe base para extratores de dados.
 */

template <typename T>
class Extrator {
protected:
    vector<string> strColumnsName;
    Buffer<Dataframe> outputBuffer;
    TaskQueue* taskqueue = nullptr;
    string strFilesFlag;
    int iTamanhoBatch;
    ifstream file;
    sqlite3* bancoDeDados;
    string strNomeTabela;

public:
    /**
     * @brief Construtor padrão.
     */
    Extrator(const string& strFilesPath, const string& strFilesFlag, int iTamanhoBatch, const string& strNomeTabela = "Nada"){
        this->strFilesFlag = strFilesFlag;
        this->iTamanhoBatch = iTamanhoBatch;
        this->strNomeTabela = strNomeTabela;

        if (this->strFilesFlag == "csv"){
            this->file = ifstream(strFilesPath);
            if (!this->file.is_open()) {
                throw runtime_error("Falha ao abrir o arquivo.");
            }
            else {
                lerCabecalhoCSV();
            }
        }

        else if (this->strFilesFlag == "sql"){
            int exit = sqlite3_open(strFilesPath.c_str(), &this->bancoDeDados);
            if (exit) {
                throw runtime_error("Erro ao abrir o banco de dados: " + string(sqlite3_errmsg(this->bancoDeDados)));
            } else {
                lerCabecalhoSQL(this->strNomeTabela);
            }
        }
    };

    /**
     * @brief Destrutor padrão.
     */
    virtual ~Extrator(){
        if (this->strFilesFlag == "csv")
        {
            if (this->file.is_open()) {
                this->file.close();
            }
        }
        else if (this->strFilesFlag == "sql")
        {
            if (this->bancoDeDados) {
                sqlite3_close(this->bancoDeDados);
            }
        }
    };

    TaskQueue* get_taskqueue() const { return taskqueue; }
    void set_taskqueue(TaskQueue* tq) { taskqueue = tq; }
    Buffer<T>& get_output_buffer() { return outputBuffer; }
    string getFilesFlag() const { return this->strFilesFlag; }
    int getBatchSize() const { return this->iTamanhoBatch; }

    void setTaskQueue(TaskQueue* tq) { this->taskqueue = tq; }
    // void setOutputBuffer(Buffer<T>& outputBuffer) { this->outputBuffer = outputBuffer; }
    void setFilesFlag(const string& strFilesFlag) { this->strFilesFlag = strFilesFlag; }
    void setBatchSize(int iTamanhoBatch) { this->iTamanhoBatch = iTamanhoBatch; }

    void lerCabecalhoCSV() {
        string line;
        if (getline(this->file, line)) {
            stringstream ss(line);
            string cell;

            while (getline(ss, cell, ',')) {
                strColumnsName.push_back(cell);

            }
        } else {
            cerr << "Erro ao ler o cabeçalho do CSV." << endl;
        }
    }

    void lerCabecalhoSQL(const string& strNomeTabela) {
        string sql = "PRAGMA table_info(" + strNomeTabela + ");";
        sqlite3_stmt* stmt;
        this->strColumnsName.clear();

        if (sqlite3_prepare_v2(this->bancoDeDados, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                this->strColumnsName.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            }
            sqlite3_finalize(stmt);
        } else {
            cerr << "Erro ao preparar consulta SQL." << endl;
        }
    }

     void enqueue_tasks(){
        string strBlocoDeTexto;
        int iContador = 0;

        if (this->strFilesFlag == "csv")
        {
            string line;
            while (getline(file, line)) {
                // Ignora a primeira linha (cabeçalho)
                if (iContador == 0) {
                    iContador++;
                    continue;
                }

                strBlocoDeTexto += line + "\n";

                if (iContador % this->iTamanhoBatch == 0) {
                    string value = strBlocoDeTexto;
                    taskqueue->push_task([this, val = value]() mutable {
                        this->create_task(val);
                    });
                    this->outputBuffer.get_semaphore().wait();
                }
                // cout << iContador << endl;
                iContador++;
            }

            // Adiciona o último bloco, se houver
            if (!strBlocoDeTexto.empty()) {
                string value = strBlocoDeTexto;
                taskqueue->push_task([this, val = value]() mutable {
                    this->create_task(val);
                });
                this->outputBuffer.get_semaphore().wait();
            }

        }
        else if (this->strFilesFlag == "sql"){
            string sql = "SELECT * FROM " + this->strNomeTabela+ ";";
            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(this->bancoDeDados, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    string line;

                    // Constrói uma linha de dados separada por vírgula
                    for (size_t i = 0; i < this->strColumnsName.size(); ++i) {
                        const char* valor = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                        line += valor ? valor : "NULL";
                        if (i < this->strColumnsName.size() - 1) {
                            line += ",";  // Adiciona vírgula apenas entre os valores
                        }
                    }

                    strBlocoDeTexto += line + "\n"; // Adiciona a linha ao bloco

                    // Quando atinge o tamanho do batch, processa o bloco
                    if (++iContador % this->iTamanhoBatch == 0) {
                        string value = strBlocoDeTexto;
                        taskqueue->push_task([this, val = value]() mutable {
                            this->create_task(val);
                        });
                        // this->outputBuffer.get_semaphore().wait();
                    }
                }

                // Se ainda houver dados pendentes no bloco, processa o restante
                if (!strBlocoDeTexto.empty()) {
                    string value = strBlocoDeTexto;
                    taskqueue->push_task([this, val = value]() mutable {
                        this->create_task(val);
                    });
                    this->outputBuffer.get_semaphore().wait();
                }

                sqlite3_finalize(stmt);
            } else {
                throw runtime_error("Erro ao preparar consulta SQL.");
            }
        }

        // Avisa ao buffer de saída que os dados acabaram
        outputBuffer.setInputTasksCreated();
    }

    T run(const string& strTextBlock){
        return dfSubExtractor(strTextBlock);
    }

    /**
     * @brief Constrói um DataFrame a partir de um bloco de texto CSV.
     *
     * @param strBlocoDeTextoCSV Bloco de texto CSV.
     * @return DataFrame construído a partir do bloco de texto.
     */
     Dataframe dfSubExtractor(const string& strBlocoDeTexto){
        Dataframe dfAuxiliar;

        // As colunas do DataFrame auxiliar são as mesmas do DataFrame original
        dfAuxiliar.vstrColumnsName = this->strColumnsName;

        // Vou criar o esboço do DataFrame auxiliar
        for (auto col : this->strColumnsName){
            Series auxSerie(col, "string");
            dfAuxiliar.columns.push_back(auxSerie);
        }

        // Agora vou preencher o DataFrame auxiliar com os dados do bloco de texto
        stringstream ss(strBlocoDeTexto);
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
        return dfAuxiliar;
    }

    void create_task(const string& value) {
        T data = run(value);
        this->outputBuffer.push(data);
     }


    /**
     * @brief Método getter para os nomes das colunas.
     *
     * @return Vetor de strings com os nomes das colunas.
     */
    vector<string> getColumnsName() const {
        return strColumnsName;
    }

    void finishBuffer()
    {
        outputBuffer.setInputTasksCreated();
    }
};

// Classe do transformador
template <typename T>
class Transformer {
    protected:
        // Buffer das entradas
        Buffer<T>& input_buffer;
        // Buffer das saídas
        Buffer<T> output_buffer;
        // Fila de tarefas onde salva das tasks
        TaskQueue* taskqueue = nullptr;

    public:
        explicit Transformer(Buffer<T>& in) : input_buffer(in) {}

        // Método que enfileira as tarefas na fila de tarefas
        void enqueue_tasks(){
            // Enquanto o buffer de entrada não tiver terminado...
            while (!input_buffer.atomicGetInputDataFinished()) {
                // Se tiver espaço no buffer de saída...
                if (output_buffer.get_semaphore().get_count() > 0)
                {
                    // Tenta pegar um dataframe do buffer de entrada
                    // (Redundante com a verificação do while, mas pode evitar problemas
                    // como tentar tirar algo do buffer com ele vazio)
                    std::optional<T> maybe_value = input_buffer.pop();
                    // Se não tiver retornado um dataframe, encerra o método
                    if (!maybe_value.has_value()) {
                        return;
                    }
                    T value = std::move(*maybe_value);
                    // Adiciona a tarefa do transformador com esse dataframe na fila
                    taskqueue->push_task([this, val = std::move(value)]() mutable {
                        this->create_task(std::move(val));
                    });
                    // Diminui o semáforo do buffer de saída (reserva um espaço para a saída da tarefa)
                    output_buffer.get_semaphore().wait();
                }
            }
            // Depois de acabado, avisa o buffer de saída que acabou
            finishBuffer();
        }

        // Função para pegar o buffer de saída
        Buffer<T>& get_output_buffer() { return output_buffer; }
        virtual T run(T dataframe) = 0;

        // Função que encapsula a tarefa e o salvamento no buffer
        void create_task(T value) {
            T data = run(value);
            output_buffer.push(data);
        }

        virtual ~Transformer() = default;
        void set_taskqueue(TaskQueue* tq) { taskqueue = tq; }
        TaskQueue* get_taskqueue() const { return taskqueue; }

        // Método para avisar ao buffer de saída que os dados acabaram
        void finishBuffer()
        {
            output_buffer.setInputTasksCreated();
        }
};

// Classe dos carregadores
template <typename T>
class Loader {
    protected:
        Buffer<T>& input_buffer;
        TaskQueue* taskqueue = nullptr;

    public:
    explicit Loader(Buffer<T>& buffer) : input_buffer(buffer) {}
    virtual ~Loader() = default;
    void set_taskqueue(TaskQueue* tq) { taskqueue = tq; }
    TaskQueue* get_taskqueue() const { return taskqueue; }

    // Método que enfileira as tarefas na fila de tarefas
    void enqueue_tasks(){
        // Enquanto o buffer de entrada não tiver terminado...
        while (!(input_buffer.atomicGetInputDataFinished())) {
            // Tenta pegar um dataframe do buffer de entrada
            // (Redundante com a verificação do while, mas pode evitar problemas
            // como tentar tirar algo do buffer com ele vazio)
            std::optional<T> maybe_value = input_buffer.pop();
            // Se não tiver retornado um dataframe, encerra o método
            if (!maybe_value.has_value()) {
                return;
            }
            T value = std::move(*maybe_value);
            // Adiciona a tarefa do carregador com esse dataframe na fila
            taskqueue->push_task([this, val = std::move(value)]() mutable {
                this->create_task(std::move(val));
            });
        }

        // Diminui o número de loaders trabalhando
        taskqueue -> getNumberOfLoaders().wait();
        // Avisa a fila que mais um loader terminou
        taskqueue -> notifyAll();
    }

    virtual void run(T value) = 0;

    // Função que encapsula a tarefa e o salvamento no buffer
    // (redundante, mas para manter a consistência)
    void create_task(T value) {
        run(std::move(value));
    }
};

#endif
