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
    inicio_ano = datetime.date(2025, 1, 1)
    fim_ano = datetime.date(2025, 12, 31)
    delta_dias = (fim_ano - inicio_ano).days
    dias_aleatorios = random.randint(0, delta_dias)
    data_aleatoria = inicio_ano + datetime.timedelta(days=dias_aleatorios)
    return data_aleatoria.strftime('%d/%m/%Y')

# Gera uma entrada de dados de viagem
def gerar_registro_viagem():
    # Selecionar cidades diferentes para origem e destino
    origem = random.choice(cidades_com_aeroportos)
    destino = random.choice([cidade for cidade in cidades_com_aeroportos if cidade != origem])

    # Gerar datas de ida e volta
    data_ida = data_aleatoria_2025()

    # Converter string para objeto de data para calcular a data de volta
    data_ida_obj = datetime.datetime.strptime(data_ida, '%d/%m/%Y')

    # Adicionar entre 1 e 30 dias para a volta
    dias_estadia = random.randint(1, 30)
    data_volta_obj = data_ida_obj + datetime.timedelta(days=dias_estadia)
    data_volta = data_volta_obj.strftime('%d/%m/%Y')

    return [origem, destino, data_ida, data_volta]

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
    arquivo_saida = 'dados_viagens_2025.csv'
    total_registros = 10000  # Total de registros a serem gerados
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

if __name__ == "__main__":
    main()
