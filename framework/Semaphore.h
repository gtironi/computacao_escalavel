#ifndef SEMAPHORE
#define SEMAPHORE

#include <mutex>
#include <condition_variable>

/**
 * Classe Semaphore - implementação simples de um semáforo contável.
 * 
 * Permite controle de concorrência entre threads, limitando o número de acessos simultâneos
 * a um recurso compartilhado. Utilizada, por exemplo, para limitar o tamanho de buffers.
 */
class Semaphore {
    private:
        int count;                      // Contador atual do semáforo (quantos recursos estão disponíveis)
        int max_count;                  // Máximo permitido (não utilizado diretamente aqui, mas pode ser útil para lógica externa)
        std::mutex mutex;              // Mutex para proteger o acesso ao contador
        std::condition_variable condition; // Usada para suspender/resumir threads com base no estado do contador
    
    public:
        /**
         * Construtor do semáforo.
         * @param count - valor inicial do semáforo (quantos "slots" disponíveis)
         * @param max_count - valor máximo (por padrão, 1 — usado principalmente como referência externa)
         */
        explicit Semaphore(int count = 0, int max_count = 1)
            : count(count), max_count(max_count) {}
    
        virtual ~Semaphore() = default;
    
        /**
         * Bloqueia a thread até que haja pelo menos um slot disponível.
         * Decrementa o contador ao adquirir o recurso.
         */
        void wait() {
            std::unique_lock<std::mutex> lock(mutex);
    
            // Espera até que count > 0 (há recurso disponível)
            condition.wait(lock, [&]() { return count > 0; });
    
            // Após adquirir, decrementa o contador
            count--;
        }
    
        /**
         * Libera um recurso e incrementa o contador e notifica uma thread que esteja esperando.
         */
        void notify() {
            std::unique_lock<std::mutex> lock(mutex);
            count++;
            condition.notify_one(); // Acorda uma thread esperando por recurso
        }
    
        /**
         * Retorna o valor atual do contador (thread-safe)
         */
        int get_count() {
            std::lock_guard<std::mutex> lock(mutex);
            return count;
        }
    };

#endif
