#include <thread>
#include <functional>
#include <chrono>
#include <atomic>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

/**
 * @brief Classe que representa um trigger de tempo.
 * @details Esta classe permite executar uma função periodicamente em um intervalo de tempo especificado.
 */
class TimeTrigger {
private:
    std::function<void()> func_;
    int iInterval;
    std::atomic<bool> bRunning;
    std::thread thread_;
public:
    /**
     * @brief Construtor da classe Trigger.
     * @param func Função a ser chamada periodicamente.
     * @param interval_seconds Intervalo em segundos entre as chamadas da função.
     */
    TimeTrigger(std::function<void()> func, int interval_seconds)
        : func_(func), iInterval(interval_seconds), bRunning(false) {}
        
    /**
     * @brief Inicia a execução da função periodicamente.
     * @details A função será chamada a cada intervalo especificado, e o tempo de execução da função será subtraído do intervalo.
     */
    void start() {
        bRunning = true;
        thread_ = std::thread([this]() {
            while (bRunning) {
                auto start = std::chrono::high_resolution_clock::now();
                func_(); // executa o que for passado
                auto end = std::chrono::high_resolution_clock::now();

                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start);
                auto sleep_time = std::chrono::seconds(iInterval) - elapsed;

                if (sleep_time.count() > 0)
                    std::this_thread::sleep_for(sleep_time);
            }
        });
    }

    /**
     * @brief Para a execução da função periodicamente.
     * @details A função não será mais chamada após esta chamada.
     */
    void stop() {
        bRunning = false;
        if (thread_.joinable())
            thread_.join();
    }

    ~TimeTrigger() {
        stop();
    }
};

/**
 * @brief Classe que representa um trigger de evento baseado em arquivo.
 * @details Esta classe permite executar uma função sempre que um arquivo específico é modificado.
 */
class EventTrigger {
private:
    std::string strFilePath;
    std::function<void()> fFunction;
    std::filesystem::file_time_type last_write_time_;
    int iInterval;
    std::atomic<bool> bRunning;
    std::thread thread_;

public:
    /**
     * @brief Construtor da classe EventTrigger.
     * @param path Caminho do arquivo a ser monitorado.
     * @param function Função a ser chamada quando o arquivo for modificado.
     * @param poll_interval_sec Intervalo em segundos para verificar a modificação do arquivo.
     */
    EventTrigger(const std::string& path, std::function<void()> function, int poll_interval_sec)
        : strFilePath(path), fFunction(function), iInterval(poll_interval_sec), bRunning(false) {}

    /**
     * @brief Inicia o monitoramento do arquivo.
     * @details A função será chamada sempre que o arquivo for modificado.
     */
    void start() {
        using namespace std::chrono_literals;

        if (!std::filesystem::exists(strFilePath)) {
            std::cerr << "Arquivo não encontrado: " << strFilePath << std::endl;
            return;
        }

        last_write_time_ = std::filesystem::last_write_time(strFilePath);
        bRunning = true;

        thread_ = std::thread([this]() {
            while (bRunning) {
                std::this_thread::sleep_for(std::chrono::seconds(iInterval));

                if (!std::filesystem::exists(strFilePath)) {
                    std::cerr << "Arquivo desapareceu: " << strFilePath << std::endl;
                    continue;
                }

                auto current_write_time = std::filesystem::last_write_time(strFilePath);

                if (current_write_time != last_write_time_) {
                    last_write_time_ = current_write_time;

                    std::cout << "Mudança detectada em " << strFilePath << ". Executando ação...\n";
                    fFunction(); 
                }
            }
        });
    }

    /**
     * @brief Para o monitoramento do arquivo.
     */
    void stop() {
        bRunning = false;
        if (thread_.joinable())
            thread_.join();
    }

    ~EventTrigger() {
        stop();
    }
};