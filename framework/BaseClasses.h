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
#include <any>
#include "Series.h"
#include <any>

using namespace std;

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
        finishBuffer();
    }

    T run(const string& strTextBlock){
        return dfSubExtractor(strTextBlock);
    }

    /**
     * @brief Constrói um DataFrame a partir de um bloco de texto CSV.
     *
     * @param strBlocoDeTexto Bloco de texto CSV.
     * @return DataFrame construído a partir do bloco de texto.
     */
    Dataframe dfSubExtractor(const string& strBlocoDeTexto) {
        Dataframe dfAuxiliar;
        dfAuxiliar.vstrColumnsName = this->strColumnsName;

        // Preparar as colunas
        for (const auto& col : this->strColumnsName) {
            dfAuxiliar.columns.emplace_back(col, "string");
        }

        stringstream ss(strBlocoDeTexto);
        string line;
    
        // Estimar o número de linhas para pré-alocar espaço
        size_t estimatedRows = count(strBlocoDeTexto.begin(), strBlocoDeTexto.end(), '\n') + 1;
        for (auto& col : dfAuxiliar.columns) {
            col.reserve(estimatedRows);
        }
    
        while (getline(ss, line)) {
            stringstream ssLine(line);
            string cell;
            vector<any> convertedRow;
            convertedRow.reserve(dfAuxiliar.vstrColumnsName.size());

            size_t colIndex = 0;
            while (getline(ssLine, cell, ',')) {
                // Adiciona a célula diretamente como string
                convertedRow.emplace_back(cell);
                colIndex++;
            }
    
            // Completa a linha se necessário
            while (colIndex < dfAuxiliar.vstrColumnsName.size()) {
                convertedRow.emplace_back(string(""));
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

// Classe base genérica para transformação de dados em um pipeline paralelo
// T: Tipo dos dados processados (ex: Dataframe, estrutura customizada, etc.)
template <typename T>
class Transformer {
protected:
    // Buffers de entrada (ponteiros, pois podem ser compartilhados entre componentes)
    std::vector<Buffer<T>*> input_buffers;

    // Buffers de saída (criados pela própria classe)
    std::vector<Buffer<T>> output_buffers;

    // Fila de tarefas onde serão enfileiradas as transformações
    TaskQueue* taskqueue = nullptr;

    // Número de buffers de saída e índice do próximo a ser usado
    int numOutputBuffers;
    int nextOutputBuffer = 0;

    // Número de buffers de entrada
    int numInputBuffers;

    // Histórico de dataframes usados nas transformações, útil quando há múltiplas entradas
    std::vector<T> historyDataframes;

public:
    /**
     * Construtor do Transformer
     * @param in - Buffers de entrada
     * @param num_outputs - Número de buffers de saída a serem criados
     */
    Transformer(std::vector<Buffer<T>*> in, int num_outputs = 1)
        : input_buffers(in), output_buffers(num_outputs), 
          numOutputBuffers(num_outputs), numInputBuffers(input_buffers.size()) {
        
        // Inicializa o histórico de entradas se houver mais de um buffer de entrada
        if (numInputBuffers > 1) {
            historyDataframes.resize(numInputBuffers);
        }
    }

    /**
     * @brief Retorna o próximo buffer de saída disponível (avança o índice)
     * @throws std::out_of_range se exceder o número de buffers disponíveis
     * @return Buffer de output
     */
    Buffer<T>& get_output_buffer() {
        if (nextOutputBuffer >= numOutputBuffers) {
            std::cout << "ERROR: NUMBER OF USED BUFFERS EXCEEDED NUMBER OF CREATED BUFFERS!" << std::endl;
            throw std::out_of_range("Número de buffers excedido!");
        }
        return output_buffers[nextOutputBuffer++];
    }

    /**
     * Retorna o buffer de saída por índice (sem alterar o estado do Transformer)
     * @param index - índice do buffer de saída desejado
     */
    Buffer<T>& get_output_buffer_by_index(int index) { 
        return output_buffers[index]; 
    }

    /**
     * Método principal que enfileira as tarefas na fila de execução.
     * Realiza o controle de semáforos, histórico e empacotamento das entradas para processar.
     */
    void enqueue_tasks() {
        while (true) {
            // Verifica se ainda existem dados nos buffers de entrada
            bool canContinue = false;
            for (int i = 0; i < input_buffers.size(); i++) {
                if (!input_buffers[i]->atomicGetInputDataFinished()) {
                    canContinue = true;
                }
            }

            if (canContinue) {
                // Verifica se todos os buffers de saída possuem espaço disponível
                bool canSendTask = true;
                for (int i = 0; i < numOutputBuffers; i++) {
                    if (get_output_buffer_by_index(i).get_semaphore().get_count() <= 0) {
                        canSendTask = false;
                        break;
                    }
                }

                if (canSendTask) {
                    std::optional<T> maybe_value;
                    int currentInputBuffer = -1;

                    // Tenta extrair dados de algum buffer de entrada
                    for (int i = 0; i < numInputBuffers; i++) {
                        maybe_value = input_buffers[i]->pop(numInputBuffers > 1);
                        if (maybe_value.has_value()) {
                            currentInputBuffer = i;
                            break;
                        }
                    }

                    // Caso não tenha encontrado, tenta novamente verificando se ainda há dados não finalizados
                    if (!maybe_value.has_value()) {
                        for (int i = 0; i < numInputBuffers; i++) {
                            if (!input_buffers[i]->atomicGetInputDataFinished()) {
                                maybe_value = input_buffers[i]->pop();
                                currentInputBuffer = i;
                                break;
                            }
                        }
                    }

                    // Se conseguiu extrair algo
                    if (maybe_value.has_value()) {
                        T value = std::move(*maybe_value);

                        // Armazena o histórico se houver múltiplas entradas
                        if (numInputBuffers > 1) {
                            historyDataframes[currentInputBuffer].hStack(value);
                        }

                        // Prepara argumentos para a transformação
                        std::vector<std::shared_ptr<T>> args(numInputBuffers);
                        for (int i = 0; i < numInputBuffers; i++) {
                            if (i == currentInputBuffer) {
                                args[i] = std::make_shared<T>(std::move(value));
                            } else {
                                args[i] = std::make_shared<T>(historyDataframes[i]);
                            }
                        }

                        // Enfileira tarefa na fila de execução
                        taskqueue->push_task([this, args = std::move(args)]() mutable {
                            std::vector<T*> raw_args;
                            for (auto& ptr : args) {
                                raw_args.push_back(ptr.get());
                            }
                            this->create_task(std::move(raw_args));
                        });
                    }
                }
            } else {
                // Nenhum dado restante nos buffers de entrada
                break;
            }
        }

        // Finaliza os buffers de saída após o fim do processamento
        finishBuffer();
    }

    /**
     * @brief Método abstrato que será implementado por subclasses para aplicar a lógica de transformação.
     * @param dataframe - vetor de ponteiros para os dados de entrada
     * @return T - resultado da transformação
     */
    virtual T run(std::vector<T*> dataframe) = 0;

    /**
     * @brief Envolve a execução de `run` e o envio dos dados para os buffers de saída.
     * @param value - vetor de ponteiros para os dados de entrada
     */
    void create_task(std::vector<T*> value) {
        T data = run(value);
        for (int i = 0; i < numOutputBuffers; i++) {
            // Incrementa o semáforo e espera até que tenha vaga
            get_output_buffer_by_index(i).get_semaphore().wait();
            // Coloca no buffer
            get_output_buffer_by_index(i).push(data);
        }
    }

    virtual ~Transformer() = default;

    // Setter da fila de tarefas
    void set_taskqueue(TaskQueue* tq) { taskqueue = tq; }

    // Getter da fila de tarefas
    TaskQueue* get_taskqueue() const { return taskqueue; }

    /**
     * @brief Finaliza todos os buffers de saída, indicando que não haverá mais dados.
     */
    void finishBuffer() {
        for (int i = 0; i < numOutputBuffers; i++) {
            get_output_buffer_by_index(i).finalizeInput();
        }
    }
};


// Classe base genérica para carregadores (última etapa do pipeline)
// T: Tipo dos dados que serão consumidos (ex: DataFrame, estrutura customizada, etc.)
template <typename T>
class Loader {
protected:
    // Buffer de entrada do qual os dados serão consumidos
    Buffer<T>& input_buffer;

    // Ponteiro para a fila de tarefas responsável pela execução concorrente
    TaskQueue* taskqueue = nullptr;

public:
    /**
     * Construtor do Loader
     * @param buffer - Referência ao buffer de entrada
     */
    explicit Loader(Buffer<T>& buffer) : input_buffer(buffer) {}

    virtual ~Loader() = default;

    // Define a fila de tarefas a ser usada
    void set_taskqueue(TaskQueue* tq) { taskqueue = tq; }

    // Retorna a fila de tarefas
    TaskQueue* get_taskqueue() const { return taskqueue; }

    /**
     * Método responsável por enfileirar as tarefas de carregamento.
     * Extrai dados do buffer de entrada e os envia para a fila de execução paralela.
     */
    void enqueue_tasks() {
        // Enquanto o buffer de entrada ainda não indicou o fim dos dados
        while (!(input_buffer.atomicGetInputDataFinished())) {
            // Tenta extrair um dado do buffer
            std::optional<T> maybe_value = input_buffer.pop();

            // Se não conseguir pegar nenhum dado (buffer vazio no momento), encerra o loop
            if (!maybe_value.has_value()) {
                break;
            }

            // Move o valor extraído
            T value = std::move(*maybe_value);

            // Enfileira a tarefa na fila de execução, chamando o método `create_task`
            taskqueue->push_task([this, val = std::move(value)]() mutable {
                this->create_task(std::move(val));
            });
        }

        // Quando termina de consumir todos os dados:
        // Decrementa o número de loaders ativos
        taskqueue->getNumberOfLoaders().wait();

        // Notifica a fila de que este loader terminou
        taskqueue->notifyAll();
    }

    /**
     * Método virtual puro que deve ser implementado pelas subclasses.
     * Define a lógica de carregamento final dos dados (ex: salvar no disco, exibir, enviar para API, etc.)
     * @param value - Dado a ser carregado
     */
    virtual void run(T value) = 0;

    /**
     * Método auxiliar que apenas chama `run`. 
     * Mantido por consistência com as outras classes (como Transformer).
     * @param value - Dado a ser carregado
     */
    void create_task(T value) {
        run(std::move(value));
    }
};


#endif
