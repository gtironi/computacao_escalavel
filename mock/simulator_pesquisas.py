import csv
import random
import threading
import datetime
from concurrent.futures import ThreadPoolExecutor
import os
from collections import defaultdict

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

# Lê o CSV de hotéis e cria um dicionário {cidade: [lista de hotéis únicos]}
def carregar_dicionario_hoteis(caminho_csv):
    hoteis_por_cidade = defaultdict(set)
    with open(caminho_csv, 'r', encoding='utf-8') as f:
        leitor = csv.DictReader(f)
        for linha in leitor:
            cidade = linha['cidade'].strip()
            hotel = linha['nome_hotel'].strip()
            hoteis_por_cidade[cidade].add(hotel)
    return {cidade: list(hoteis) for cidade, hoteis in hoteis_por_cidade.items()}

# Gera uma entrada de dados de viagem
def gerar_registro_viagem(hoteis_por_cidade):
    origem = random.choice(cidades_com_aeroportos)
    destino = random.choice([cidade for cidade in cidades_com_aeroportos if cidade != origem])

    data_ida = data_aleatoria_2025()
    dias_estadia = random.randint(1, 15)
    data_volta = data_ida + datetime.timedelta(days=dias_estadia)

    hoteis = hoteis_por_cidade.get(destino, ["Hotel Padrão"])
    hotel_escolhido = random.choice(hoteis)

    return [
        origem, destino, hotel_escolhido,
        data_ida.day, data_ida.month, data_ida.year,
        data_volta.day, data_volta.month, data_volta.year
    ]

# Função que cada thread executará
def gerar_dados_em_thread(numero_registros, arquivo_saida, thread_id, hoteis_por_cidade):
    registros = []
    for _ in range(numero_registros):
        registros.append(gerar_registro_viagem(hoteis_por_cidade))

    with threading.Lock():
        with open(arquivo_saida, 'a', newline='', encoding='utf-8') as arquivo_csv:
            escritor = csv.writer(arquivo_csv)
            for registro in registros:
                escritor.writerow(registro)

    print(f"Thread {thread_id} concluiu a geração de {numero_registros} registros.")

# Função principal
def main():
    caminho_entrada_hoteis = os.path.join(os.path.dirname(__file__), 'data/dados_hoteis_2025.csv')
    arquivo_saida = os.path.join(os.path.dirname(__file__), 'data/dados_viagens_2025.csv')
    total_registros = 5000
    num_threads = 4
    registros_por_thread = total_registros // num_threads

    # Carrega os hotéis por cidade
    hoteis_por_cidade = carregar_dicionario_hoteis(caminho_entrada_hoteis)

    # Cria o CSV e escreve o cabeçalho
    with open(arquivo_saida, 'w', newline='', encoding='utf-8') as arquivo_csv:
        escritor = csv.writer(arquivo_csv)
        escritor.writerow([
            'cidade_origem', 'cidade_destino', 'nome_hotel',
            'data_ida_dia', 'data_ida_mes', 'data_ida_ano',
            'data_volta_dia', 'data_volta_mes', 'data_volta_ano'
        ])

    print(f"Gerando {total_registros} registros de viagens utilizando {num_threads} threads...")

    with ThreadPoolExecutor(max_workers=num_threads) as executor:
        for i in range(num_threads):
            executor.submit(gerar_dados_em_thread, registros_por_thread, arquivo_saida, i+1, hoteis_por_cidade)

    print(f"Geração concluída! Os dados foram salvos em '{arquivo_saida}'")

if __name__ == "__main__":
    main()
