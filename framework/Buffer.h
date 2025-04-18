#ifndef BUFFER_H
#define BUFFER_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <optional>
#include <iostream>
#include "Semaphore.h"

// Implementando um buffer Thread-Safe
template <typename T>
class Buffer {
private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cond;
    int max_size;
    Semaphore semaphore;
    // Atributo que indica se o trabalho anterior já parou de colocar tarefas novas na fila
    bool inputTasksCreated = false;
    // Atributo que indica quando os dados de input acabaram
    bool inputDataFinished = false;

public:
    Buffer(int max_size = 5000) : max_size(max_size), semaphore(max_size) {}

    // Método para adicionar coisas no buffer
    void push(T value) {
        // Trava o mutex
        std::lock_guard<std::mutex> lock(mtx);
        // Adiciona na fila
        queue.push(std::move(value));
        // Avisa qualquer thread que esteja esperando algo entrar no buffer
        cond.notify_one();
    }
    
    // Método para retirar o próximo elemento do buffer
    std::optional<T> pop(bool multiInput = false) {
        // Se os dados de input já tiverem acabado, retorna null
        if (getInputDataFinished() || (multiInput && queue.empty()))
        {
            return std::nullopt;
        }

        // Trava o mutex
        std::unique_lock<std::mutex> lock(mtx);
        // Espera até alguma coisa aparecer no buffer
        cond.wait(lock, [this] { return !queue.empty(); });

        // Pega o próximo valor
        T value = std::move(queue.front());
        // Tira da fila
        queue.pop();

        // Aumenta o semáforo dele (libera um espaço no buffer)
        semaphore.notify();

        // Se todas as tarefas que colocam dados nesse buffer já tiverem sido criadas
        // e se não tiver mais nenhuma dessas tarefas esperando na fila
        // ou dados no buffer esperando para serem pegos...
        if (getInputTasksCreated() && get_semaphore().get_count() == get_max_size())
        {
            // Define que os dados de input acabaram
            setInputDataFinished();
        }

        return value;
    }    

    // Método para pegar o semáforo do buffer
    Semaphore& get_semaphore() {
        return semaphore;
    }

    // Método para pegar o tamanho atual da fila do buffer
    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.size();
    }

    // Método para pegar o tamanho máximo do buffer
    int get_max_size() const {return max_size;}

    // Método para definir quando as tarefas de input tiverem sido todas criadas
    void setInputTasksCreated() {
        inputTasksCreated = true;
        // TODO
        cond.notify_all();
    }

    // Método para pegar o valor do inputTasksCreated
    bool getInputTasksCreated()
    {
        return inputTasksCreated;
    }

    // Método para pegar o valor do inputDataFinished
    bool getInputDataFinished()
    {
        return inputDataFinished;
    }

    // Método para pegar atomicamente o valor do inputDataFinished
    bool atomicGetInputDataFinished()
    {
        std::lock_guard<std::mutex> lock(mtx);
        return inputDataFinished;
    }

    // Método para definir quando os dados de entrada acabarem
    void setInputDataFinished() {
        inputDataFinished = true;
    }
};

#endif // BUFFER_H