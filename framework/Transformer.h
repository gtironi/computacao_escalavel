#ifndef TRANSFORMER_H
#define TRANSFORMER_H

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

// Função para renomear uma coluna
string rename_column(string str1, string str2){
    return str1;
}

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
    int nextOutputBuffer = -1;

    // Número de buffers de entrada
    int numInputBuffers = 0;

    // Histórico de dataframes usados nas transformações, útil quando há múltiplas entradas
    std::vector<T> historyDataframes;
    // Vetor das estatísticas internas do transformer
    std::vector<float> stats;
    // Mutexes para a atualização das estatísticas e dos dataframes de histórico
    std::mutex statsMtx;
    std::mutex dfsMtx;

    // Número de tasks que ainda estão na fila de tarefas
    Semaphore tasksInTaskQueue;

private:
    // Método para fazer a atualização das estatísticas
    void aggStats(std::vector<float> newStats)
    {
        std::lock_guard<std::mutex> lock(statsMtx);
        // Se o vetor não estiver inicializado, inicializa-o
        if (stats.size() == 0)
        {
            stats.resize(newStats.size());
        }
        // Incrementa os valores
        for (int i = 0; i < stats.size(); i++)
        {
            stats[i] += newStats[i];
        }
    }

public:
    /**
     * Construtor do Transformer
     * @param in - Buffers de entrada
     * @param num_outputs - Número de buffers de saída a serem criados
     */
    Transformer(int num_outputs = 1)
        : output_buffers(num_outputs), numOutputBuffers(num_outputs) {
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
        
        nextOutputBuffer++;
        return output_buffers[nextOutputBuffer];
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
    virtual void enqueue_tasks() {
        // Inicializa o histórico de entradas se houver mais de um buffer de entrada
        if (numInputBuffers > 1) {
            historyDataframes.resize(numInputBuffers);
        }

        while (true) {
            // Verifica se ainda existem dados nos buffers de entrada
            bool canContinue = false;
            for (int i = 0; i < input_buffers.size(); i++) {
                if (!input_buffers[i]->atomicGetInputDataFinished()) {
                    canContinue = true;

                }
            }

            // Se ainda tiver dados...
            if (canContinue) {
                // Verifica se todos os buffers de saída possuem espaço disponível
                bool canSendTask = true;
                for (int i = 0; i < numOutputBuffers; i++) {
                    if (get_output_buffer_by_index(i).get_semaphore().get_count() <= 0) {
                        canSendTask = false;
                        break;
                    }
                }

                // Se todos tiverem...
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

                    // Caso não tenha encontrado,fica esperando no primeiro buffer não finalizado
                    if (!maybe_value.has_value() && numInputBuffers > 1) {
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
                            {
                                std::lock_guard<std::mutex> lock(dfsMtx);
                                historyDataframes[currentInputBuffer].hStack(value);
                            }
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

                        // Incrementa o número de tarefas na task queue
                        tasksInTaskQueue.notify();
                    }
                }
            } else {
                // Nenhum dado restante nos buffers de entrada
                break;
            }
        }

        // Espera todas as tarefas serem processadas
        while (tasksInTaskQueue.get_count() > 0) {}

        // Finaliza os buffers de saída após o fim do processamento
        finishBuffer();

    }

    /**
     * @brief Método abstrato que será implementado por subclasses para aplicar a lógica de transformação.
     * @param dataframe - vetor de ponteiros para os dados de entrada
     * @return T - resultado da transformação
     */
    virtual T run(std::vector<T*> dataframe) = 0;

    // Método "abstrato" do cálculo das estatísticas
    virtual std::vector<float> calculateStats(std::vector<T*> dataframe)
    {
        return std::vector<float>{};
    };

    // Getter das estatísticas
    std::vector<float> getStats()
    {
        std::lock_guard<std::mutex> lock(statsMtx);
        return stats;
    }

    /**
     * @brief Envolve a execução de `run` e o envio dos dados para os buffers de saída.
     * @param value - vetor de ponteiros para os dados de entrada
     */
    void create_task(std::vector<T*> value) {
        // Calcula e agrega as estatísticas
        std::vector<float> currentStats = calculateStats(value);
        aggStats(currentStats);
        
        T data = run(value);
        
        if (data.getShape().first > 0)
        {
            // cout << data << endl;
            for (int i = 0; i < numOutputBuffers; i++) {
                // Incrementa o semáforo e espera até que tenha vaga
                get_output_buffer_by_index(i).get_semaphore().wait();
                // Coloca no buffer
                get_output_buffer_by_index(i).push(data);
            }
        }

        // Incrementa o número de tarefas na fila
        tasksInTaskQueue.wait();
       
    }

    virtual ~Transformer() = default;

    // Setter da fila de tarefas
    void set_taskqueue(TaskQueue* tq) { taskqueue = tq; }

    // Getter da fila de tarefas
    TaskQueue* get_taskqueue() const { return taskqueue; }

    /**
     * @brief Finaliza todos os buffers de saída, indicando que não haverá mais dados.
     */
    virtual void finishBuffer() {
        for (int i = 0; i < numOutputBuffers; i++) {
            get_output_buffer_by_index(i).finalizeInput();
        }
    }

    // Método para adicionar um buffer de entrada
    void addInputBuffer(Buffer<T>* buffer)
    {
        input_buffers.push_back(buffer);
        numInputBuffers++;
    }
};

// Classe específica do transformador de agrupamento
template <typename T>
class GroupByTransformer : public Transformer<T> {
private:
    // Vetores das colunas base de agregação e a serem agregadas
    std::vector<std::string> keys;
    std::vector<std::string> columns;
    // Vetor das operações de agregação a serem executadas
    std::vector<std::string> operations;
    // Dataframe agregado completo
    Dataframe aggregated;
    std::mutex mtx;
    Buffer<T>* input_buffer;
    // Número de tarefas na fila de tarefas
    Semaphore tasksInTaskQueue;
    // Nome da coluna de count (deve ser passado pelo usuário)
    std::string nameCountColumn;
public:
    using Transformer<T>::output_buffers;
    using Transformer<T>::taskqueue;
    using Transformer<T>::numOutputBuffers;

    GroupByTransformer(
        Buffer<T>* input_buffer,
        const std::vector<std::string>& group_keys,
        const std::vector<std::string>& agg_columns,
        const std::vector<std::string>& agg_ops,
        std::string nameCountColumn,
        // Número de buffers de saída
        int num_outputs = 1
    ) : Transformer<T>(num_outputs),
        keys(group_keys),
        columns(agg_columns),
        operations(agg_ops),
        input_buffer(input_buffer),
        nameCountColumn(nameCountColumn) {}

    // Método do processamento da agregação
    T run(std::vector<T*> dataframes) override {
        // Checa se tem sum entre as agregações
        bool sum = false;
        for (int i = 0; i < operations.size(); i++)
        {
            if (operations[i] == "sum")
            {
                sum = true;
                break;
            }
        }

        // Agrega o batch do dataframe recebido
        Dataframe dataframe = *dataframes[0];
        Dataframe littleAggregated = dataframe.dfGroupby(keys, columns, sum, false, true);

        return littleAggregated;
    }

    // Método para criar tasks de agregação de cada batch e união com os anteriores
    void createAggTask(T* value)
    {
        // Checa se tem sum entre as agregações
        bool sum = false;
        for (int i = 0; i < operations.size(); i++)
        {
            if (operations[i] == "sum")
            {
                sum = true;
                break;
            }
        }

        // Agrega o batch
        T littleAggregated = run({value});
        // Junta com o histórico e agrega novamente
        std::lock_guard<std::mutex> lock(mtx);
        aggregated.hStackGroup(littleAggregated);
        tasksInTaskQueue.wait();
    }

    // Método para criar as tarefas que enviam o dataframe para o buffer de saída
    void sendData(int startRow, int endRow)
    {
        // Pega um slice do dataframe
        Dataframe slice = aggregated.slice(startRow, endRow);
        
        // Manda para os buffers de saída
        for (int i = 0; i < numOutputBuffers; i++) {
            // Incrementa o semáforo e espera até que tenha vaga
            this->get_output_buffer_by_index(i).get_semaphore().wait();
            this->get_output_buffer_by_index(i).push(slice);
        }
    }

    // Método para criar as tasks
    void enqueue_tasks() override {
        while (!(input_buffer -> atomicGetInputDataFinished())) {
            // Tenta extrair um dado do buffer
            std::optional<T> maybe_value = input_buffer -> pop();

            // Se não conseguir pegar nenhum dado (buffer vazio no momento), encerra o loop
            if (!maybe_value.has_value()) {
                break;
            }

            // Move o valor extraído
            T value = std::move(*maybe_value);

            // Enfileira a tarefa na fila de execução, chamando o método `createAggTask`
            taskqueue->push_task([this, val = std::move(value)]() mutable {
                this->createAggTask(&val);
            });

            // Incrementa o número de 
            tasksInTaskQueue.notify();
        }

        // Espera até todas as tarefas serem processadas
        while (tasksInTaskQueue.get_count() > 0) {}

        // Renomeia a coluna de count (para não ficar igual à de outras tabelas)
        aggregated.bColumnOperation("count", "count", rename_column, nameCountColumn);
        aggregated.dropCol("count");

        // Manda o dataframe pra frente em batches
        int nRows = aggregated.getShape().first;
        int batchSize = nRows / 10 + 1;
        int endRow = 0;
        int currentRow = 0;

        while (true)
        {
            if (currentRow >= nRows) break;
            endRow = currentRow + batchSize;
            sendData(currentRow, std::min(endRow, nRows));
            currentRow = endRow;
        }

        // Finaliza os buffers de saída após o fim do processamento
        this -> finishBuffer();
    }

    // Método abstrato de cálculo das estatísticas
    std::vector<float> calculateStats(std::vector<T*> dataframe) override {
        return {};
    }

    // Método para finalizar os buffers de saída
    void finishBuffer() override {
        for (int i = 0; i < numOutputBuffers; i++) {
            this -> get_output_buffer_by_index(i).finalizeInput();
        }
    }
};

#endif