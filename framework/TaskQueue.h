#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include "Semaphore.h"

// Classe responsável por gerenciar a fila de tarefas que serão executadas por múltiplas threads
class TaskQueue
{
private:
    std::queue<std::function<void()>> tasks; // Fila de tarefas como funções sem argumentos
    std::mutex mtx;                          // Mutex para proteger o acesso à fila
    std::condition_variable cond;            // Variável de condição para controlar o bloqueio/espera
    bool finishedWork = false;               // Indica se o sistema está encerrando as tarefas
    Semaphore numberOfLoaders;               // Semáforo que representa quantos loaders ainda estão ativos
    std::mutex nOfLoadersMtx;                          // Mutex para proteger o acesso à fila

public:
    /**
     * Adiciona uma nova tarefa à fila.
     * A tarefa é uma função (lambda, função normal ou membro).
     */
    void push_task(std::function<void()> task)
    {
        {
            std::lock_guard<std::mutex> lock(mtx); // Garante exclusão mútua ao acessar a fila
            tasks.push(task);                      // Adiciona a tarefa à fila
        }
        cond.notify_one(); // Acorda uma thread que estiver esperando por uma tarefa
    }

    /**
     * Retira e retorna uma tarefa da fila. 
     * Bloqueia a thread se a fila estiver vazia e ainda houver trabalho a ser feito.
     * Se o sistema estiver finalizando e a fila estiver vazia, retorna uma função vazia.
     */
    std::function<void()> pop_task()
    {
        std::cout << "Teste" << std::endl;
        std::unique_lock<std::mutex> lock(mtx);
        cond.wait(lock, [this] {
            return !tasks.empty() || finishedWork; // Espera enquanto a fila está vazia e o trabalho não terminou
        });

        // Se estiver finalizando e a fila estiver vazia, retorna uma função nula (shutdown)
        if (finishedWork && tasks.empty())
        {
            return std::function<void()>(); // Indica que não há mais tarefas
        }

        // Retira a tarefa da fila
        std::function<void()> task = tasks.front();
        tasks.pop();
        return task;
    }

    /**
     * Encerra o sistema, esvaziando a fila e notificando todas as threads.
     */
    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            finishedWork = true;          // Marca que o sistema está finalizando
            while (!is_empty()) {
                tasks.pop();              // Limpa as tarefas restantes
            }
        }
        cond.notify_all();                // Acorda todas as threads esperando
    }

    /**
     * Retorna se a fila está vazia.
     * ATENÇÃO: falta de proteção por mutex aqui pode causar condições de corrida se for usada externamente.
     */
    bool is_empty()
    {
        return tasks.empty();
    }

    /**
     * Verifica se o sistema está em modo de finalização.
     */
    bool isShutdown() { 
        std::lock_guard<std::mutex> lock(mtx);
        return finishedWork;
    }

    /**
     * Retorna uma referência ao semáforo que controla os loaders.
     */
    Semaphore& getNumberOfLoaders() {
        std::lock_guard<std::mutex> lock(nOfLoadersMtx);
        return numberOfLoaders;
    }

    /**
     * Espera até que todos os loaders tenham terminado de adicionar tarefas.
     */
    void waitLoadersFinish()
    {
        std::unique_lock<std::mutex> lock(nOfLoadersMtx);
        cond.wait(lock, [this] {
            return numberOfLoaders.get_count() == 0;
        });
    }

    /**
     * Acorda todas as threads que estiverem esperando na fila.
     * Pode ser usado para encerrar loops em outros componentes.
     */
    void notifyAll()
    {
        cond.notify_all();
    }
};

#endif