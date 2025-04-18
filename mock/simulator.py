import csv
import random
import threading
import datetime
from concurrent.futures import ThreadPoolExecutor
import os

# Lista de cidades com aeroportos
cidades_com_aeroportos = [
    "São Paulo", "Rio de Janeiro", "Brasília", "Belo Horizonte", "Salvador",
    "Recife", "Fortaleza", "Curitiba", "Porto Alegre", "Belém", "Manaus",
    "Florianópolis", "Goiânia", "Natal", "Maceió", "João Pessoa", "Aracaju",
    "Vitória", "Cuiabá", "Campo Grande", "São Luís", "Teresina", "Palmas",
    "Rio Branco", "Macapá", "Boa Vista", "Porto Velho", "Nova York", "Miami",
    "Los Angeles", "Londres", "Paris", "Tóquio", "Pequim", "Dubai", "Roma",
    "Madri", "Lisboa", "Amsterdã", "Frankfurt", "Toronto", "Buenos Aires",
    "Santiago", "Lima", "Cidade do México", "Bogotá"
]

# Gera uma data aleatória em 2025
def data_aleatoria_2025():
    inicio_ano = datetime.date(2025, 4, 7)
    fim_ano = datetime.date(2025, 12, 31)
    delta_dias = (fim_ano - inicio_ano).days
    dias_aleatorios = random.randint(0, delta_dias)
    data_aleatoria = inicio_ano + datetime.timedelta(days=dias_aleatorios)
    return data_aleatoria

# Gera uma entrada de dados de viagem
def gerar_registro_viagem():
    # Cidades para origem e destino
    origem = random.choice(cidades_com_aeroportos)
    destino = random.choice([cidade for cidade in cidades_com_aeroportos if cidade != origem]) #Todas menos a de origem

    # Datas de ida
    data_ida = data_aleatoria_2025()

    # Data de volta 91 a 15 dias depois)
    dias_estadia = random.randint(1, 15)
    data_volta_obj = data_ida + datetime.timedelta(days=dias_estadia)

    # Transforma em string
    data_volta_str = data_volta_obj.strftime('%d/%m/%Y')
    data_ida_str = data_ida.strftime('%d/%m/%Y')

    return [origem, destino, data_ida_str, data_volta_str]

# Função que cada thread executará
def gerar_dados_em_thread(numero_registros, arquivo_saida, thread_id):
    registros = []
    for _ in range(numero_registros):
        registros.append(gerar_registro_viagem())

    # Bloquear o arquivo durante a escrita para evitar conflitos
    with threading.Lock():
        with open(arquivo_saida, 'a', newline='', encoding='utf-8') as arquivo_csv:
            escritor = csv.writer(arquivo_csv)
            for registro in registros:
                escritor.writerow(registro)

    print(f"Thread {thread_id} concluiu a geração de {numero_registros} registros.")

# Função principal
def main():
    arquivo_saida = os.path.join(os.path.dirname(__file__), 'data/dados_viagens_2025.csv')
    total_registros = 5000  # Total de registros a serem gerados
    num_threads = 4  # Número de threads
    registros_por_thread = total_registros // num_threads

    # Criar o arquivo e escrever o cabeçalho
    with open(arquivo_saida, 'w', newline='', encoding='utf-8') as arquivo_csv:
        escritor = csv.writer(arquivo_csv)
        escritor.writerow(['cidade_origem', 'cidade_destino', 'data_ida', 'data_volta'])

    print(f"Gerando {total_registros} registros de viagens utilizando {num_threads} threads...")

    # Usar ThreadPoolExecutor para gerenciar as threads
    with ThreadPoolExecutor(max_workers=num_threads) as executor:
        for i in range(num_threads):
            executor.submit(gerar_dados_em_thread, registros_por_thread, arquivo_saida, i+1)

    print(f"Geração concluída! Os dados foram salvos em '{arquivo_saida}'")

if __name__ == "__main__":
    main()
