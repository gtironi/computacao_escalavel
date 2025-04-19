#ifndef BUFFER_H
#define BUFFER_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <optional>
#include <iostream>
#include "Semaphore.h"

// Classe Buffer - estrutura thread-safe para comunicação entre etapas do pipeline
// Usa mutex, semáforo e condition_variable para garantir segurança em ambientes concorrentes

template <typename T>
class Buffer {
private:
    std::queue<T> queue;             // Fila que armazena os dados
    std::mutex mtx;                  // Mutex para garantir acesso exclusivo à fila
    std::condition_variable cond;    // Variável de condição para controle de espera/notificação
    int max_size;                    // Capacidade máxima do buffer
    Semaphore semaphore;             // Semáforo para controlar o número de elementos permitidos

    // Flag que indica se todas as tarefas que produzem dados para esse buffer já foram criadas
    bool inputTasksCreated = false;

    // Flag que indica que os dados de entrada acabaram completamente
    bool inputDataFinished = false;

public:
    /**
     * Construtor do buffer.
     * @param max_size - Capacidade máxima do buffer (default: 5000)
     */
    Buffer(int max_size = 5000) : max_size(max_size), semaphore(max_size) {}

    /**
     * Insere um valor no buffer.
     * Deve ser chamado somente após adquirir o semáforo externamente (em geral).
     */
    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(std::move(value));
        cond.notify_one(); // Acorda uma thread consumidora que esteja esperando
    }

    /**
     * Retira um valor do buffer.
     * @param multiInput - se true, a verificação inicial retorna null se a fila estiver vazia (usado em pipelines com múltiplas entradas)
     * @param test - parâmetro reservado para testes futuros (atualmente não utilizado)
     * @return std::optional<T> contendo o valor, ou nullopt se os dados de entrada acabaram
     */
    std::optional<T> pop(bool multiInput = false, bool test = false) {
        std::unique_lock<std::mutex> lock(mtx);

        // Se for multi-input e estiver vazio, retorna imediatamente
        if (multiInput && queue.empty()) {
            return std::nullopt;
        }

        // Espera até que tenha dados ou os dados acabem
        cond.wait(lock, [this] {
            return (!queue.empty()) || (getInputDataFinished());
        });

        // Se os dados acabaram, não tem mais o que processar
        if (getInputDataFinished()) {
            return std::nullopt;
        }

        // Retira o próximo valor
        T value = std::move(queue.front());
        queue.pop();

        // Libera espaço no buffer (semáforo sobe)
        semaphore.notify();

        // Se todas as tarefas de input foram criadas e o buffer está totalmente vazio
        if (getInputTasksCreated() && get_semaphore().get_count() == get_max_size()) {
            setInputDataFinished();
        }

        return value;
    }

    // Acessa o semáforo do buffer (usado externamente para controle de concorrência)
    Semaphore& get_semaphore() {
        return semaphore;
    }

    // Retorna o número atual de elementos na fila
    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.size();
    }

    // Retorna a capacidade máxima do buffer
    int get_max_size() const {
        return max_size;
    }

    /**
     * Indica que todas as tarefas que irão produzir dados para esse buffer já foram criadas.
     * Importante para que o consumidor saiba que não virão mais tarefas no futuro.
     */
    void setInputTasksCreated() {
        std::lock_guard<std::mutex> lock(mtx);
        inputTasksCreated = true;
        cond.notify_all(); // Acorda qualquer thread que esteja esperando
    }

    // Retorna se as tarefas de input já foram todas criadas
    bool getInputTasksCreated() {
        return inputTasksCreated;
    }

    // Retorna se os dados de entrada acabaram (sem locking)
    bool getInputDataFinished() {
        return inputDataFinished;
    }

    // Retorna se os dados de entrada acabaram (com locking - seguro em ambientes concorrentes)
    bool atomicGetInputDataFinished() {
        std::lock_guard<std::mutex> lock(mtx);
        return inputDataFinished;
    }

    // Define explicitamente que os dados de entrada acabaram
    void setInputDataFinished() {
        inputDataFinished = true;
    }

    /**
     * Método chamado pelas etapas finais do pipeline para avisar que não haverá mais dados.
     * Importante para liberar consumidores que estão esperando indefinidamente.
     */
    void finalizeInput() {
        std::lock_guard<std::mutex> lock(mtx);
        inputTasksCreated = true;

        // Se o buffer está vazio, os dados realmente acabaram
        if (get_semaphore().get_count() == get_max_size()) {
            inputDataFinished = true;
        }

        cond.notify_all(); // Acorda quem estiver esperando
    }
};


#endif // BUFFER_H