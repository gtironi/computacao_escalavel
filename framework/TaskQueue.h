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

// Classe da fila de tarefas
class TaskQueue
{
    private:
        // Fila de tarefas como uma fila de funções
        std::queue<std::function<void()>> tasks;
        std::mutex mtx;
        std::condition_variable cond;
        // Booleano para controlar quando o processo deve terminar
        bool finishedWork = false;
        Semaphore numberOfLoaders;

    public:
        // Método para adicionar uma nova tarefa à fila
        void push_task(std::function<void()> task)
        {
            {
                // Trava o mutex
                std::lock_guard<std::mutex> lock(mtx);
                // Adiciona a tarefa
                tasks.push(task);
                // std::cout << "here" << std::endl;
            }
            // Acorda as threads esperando por algo aparecer na fila
            cond.notify_one();
        }

        // Método para pegar uma tarefa da fila
        std::function<void()> pop_task()
        {
            // Trava o mutex
            std::unique_lock<std::mutex> lock(mtx);
            // Se a fila estiver vazia, espera até ser acordado e
            // ter aparecido algo ou o processo ter finalizado
            cond.wait(lock, [this] 
            {
                return !tasks.empty() || finishedWork;
            });
            // Se o processo for parar e a fila tiver acabado, não retorna nada
            if (finishedWork && tasks.empty())
            {
                // Retorna função vazia para indicar shutdown.
                return std::function<void()>();
            }
            // Pega uma tarefa e a remove da fila
            std::function<void()> task = tasks.front();
            tasks.pop();
            
            return task;
        }

        // Método para encerrar todo o processo
        void shutdown()
        {
            {
                // Trava o mutex
                std::lock_guard<std::mutex> lock(mtx);
                // Indica que o trabalho foi encerrado
                finishedWork = true;
                // Esvazia a fila
                while (!is_empty())
                {
                    tasks.pop();
                }
            }
            // Notifica todas as threads adormecidas
            cond.notify_all();
        }

        // Método para checar se a fila está vazia
        bool is_empty()
        {
            // Trava o mutex
            // std::lock_guard<std::mutex> lock(mtx);
            // Retorna o resultado
            return tasks.empty();
        }

        // Método para verificar se o trabalho foi encerrado
        bool isShutdown() { 
            std::lock_guard<std::mutex> lock(mtx);
            return finishedWork;
        }

        // Método para pegar o semáforo do buffer
        Semaphore& getNumberOfLoaders() {
            return numberOfLoaders;
        }

        // Método para esperar todos os loaders terem acabado de colocar tasks na fila
        void waitLoadersFinish()
        {
            // Trava o mutex
            std::unique_lock<std::mutex> lock(mtx);
            // Espera até todos os loaders terminarem
            cond.wait(lock, [this] 
            {
                return numberOfLoaders.get_count() == 0;
            });
            return;
        }

        // Método para notificar o condition variable da fila
        void notifyAll()
        {
            cond.notify_all();
        }
};

#endif