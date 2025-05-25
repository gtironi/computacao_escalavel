import grpc
import time
import random
import threading
import signal
import sys
import csv # Import the csv module

from mock_client.proto import extractor_pb2
from mock_client.proto import extractor_pb2_grpc
from mock_client.client import generate_all

SERVER_ADDRESS = 'localhost:50051'
NUM_THREADS = 4
CSV_FILE_PATH = 'response_times.csv' # Define the CSV file path

stop_event = threading.Event()
csv_lock = threading.Lock() # Create a lock for thread-safe CSV writing

def initialize_csv():
    """Initializes the CSV file with headers if it doesn't exist."""
    with open(CSV_FILE_PATH, 'w', newline='') as csvfile:
        csv_writer = csv.writer(csvfile)
        csv_writer.writerow(['Thread ID', 'Response Time (seconds)'])

def grpc_worker(thread_id):
    with grpc.insecure_channel(SERVER_ADDRESS) as channel:
        stub = extractor_pb2_grpc.ExtractorServiceStub(channel)

        while not stop_event.is_set():
            try:
                all_reservas, all_pesquisas, all_voos = generate_all.gerar_dados()

                request_data = extractor_pb2.AllDataSend(
                    reservas=all_reservas,
                    voos=all_voos,
                    pesquisas=all_pesquisas
                )

                print(f"[Thread {thread_id}] Enviando dados ao servidor...")
                start_time = time.time() # Record the start time
                response = stub.GetAllData(request_data, timeout=10)
                end_time = time.time()   # Record the end time
                response_time = end_time - start_time # Calculate the duration

                print(f"[Thread {thread_id}] Resposta:")
                print(f"  Stats1: {response.stats1}")
                print(f"  Stats2: {response.stats2}")
                print(f"  Stats3: {response.stats3}")
                print(f"  Stats4: {response.stats4}")
                print(f"  Stats5: {response.stats5}")
                print(f"[Thread {thread_id}] Tempo de resposta: {response_time:.4f} segundos") # Print the response time

                # Write to CSV in a thread-safe manner
                with csv_lock:
                    with open(CSV_FILE_PATH, 'a', newline='') as csvfile:
                        csv_writer = csv.writer(csvfile)
                        csv_writer.writerow([thread_id, f"{response_time:.4f}"])

            except grpc.RpcError as e:
                print(f"[Thread {thread_id}] gRPC Error: {e.code()} - {e.details()}")
            except Exception as e:
                print(f"[Thread {thread_id}] Erro inesperado: {e}")

            intervalo = random.randint(3, 10)
            print(f"[Thread {thread_id}] Dormindo por {intervalo} segundos...\n")
            time.sleep(intervalo)

def signal_handler(sig, frame):
    print("\n[Main] Interrupção detectada. Encerrando todas as threads...")
    stop_event.set()

def run():
    def signal_handler(sig, frame):
        print("\n[Main] Sinal recebido. Encerrando...")
        stop_event.set()

    signal.signal(signal.SIGINT, signal_handler)

    initialize_csv() # Initialize the CSV file with headers

    threads = []
    for i in range(NUM_THREADS):
        t = threading.Thread(target=grpc_worker, args=(i+1,))
        t.start()
        threads.append(t)

    print(f"Cliente gRPC rodando com {NUM_THREADS} threads...")

    try:
        while not stop_event.is_set():
            time.sleep(0.5)
    except KeyboardInterrupt:
        print("\n[Main] Ctrl+C detectado (via KeyboardInterrupt). Encerrando...")
        stop_event.set()

    for t in threads:
        t.join()
    print("[Main] Todas as threads foram encerradas.")

if __name__ == "__main__":
    run()