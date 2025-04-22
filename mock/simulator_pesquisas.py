import csv
import random
import threading
import datetime
import sqlite3
from concurrent.futures import ThreadPoolExecutor
import os
from collections import defaultdict

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

lock = threading.Lock()

def data_aleatoria_2025():
    inicio_ano = datetime.date(2025, 4, 7)
    fim_ano = datetime.date(2025, 12, 31)
    delta_dias = (fim_ano - inicio_ano).days
    dias_aleatorios = random.randint(0, delta_dias)
    return inicio_ano + datetime.timedelta(days=dias_aleatorios)

def carregar_dicionario_hoteis(caminho_csv):
    hoteis_por_cidade = defaultdict(set)
    with open(caminho_csv, 'r', encoding='utf-8') as f:
        leitor = csv.DictReader(f)
        for linha in leitor:
            cidade = linha['cidade_destino'].strip()
            hotel = linha['nome_hotel'].strip()
            hoteis_por_cidade[cidade].add(hotel)
    return {cidade: list(hoteis) for cidade, hoteis in hoteis_por_cidade.items()}

def gerar_registro_viagem(hoteis_por_cidade):
    origem = random.choice(cidades_com_aeroportos)
    destino = random.choice([cidade for cidade in cidades_com_aeroportos if cidade != origem])
    data_ida = data_aleatoria_2025()
    dias_estadia = random.randint(1, 15)
    data_volta = data_ida + datetime.timedelta(days=dias_estadia)
    hoteis = hoteis_por_cidade.get(destino, ["Hotel Padrão"])
    hotel_escolhido = random.choice(hoteis)

    return (
        origem, destino, hotel_escolhido,
        data_ida.day, data_ida.month, data_ida.year,
        data_volta.day, data_volta.month, data_volta.year
    )

def criar_tabela_sqlite(caminho_db):
    conn = sqlite3.connect(caminho_db)
    cursor = conn.cursor()
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS viagens (
            cidade_origem TEXT,
            cidade_destino TEXT,
            nome_hotel TEXT,
            data_ida_dia INTEGER,
            data_ida_mes INTEGER,
            data_ida_ano INTEGER,
            data_volta_dia INTEGER,
            data_volta_mes INTEGER,
            data_volta_ano INTEGER
        )
    """)
    conn.commit()
    conn.close()

def inserir_registros_no_banco(registros, caminho_db):
    with lock:
        conn = sqlite3.connect(caminho_db)
        cursor = conn.cursor()
        cursor.executemany("""
            INSERT INTO viagens VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, registros)
        conn.commit()
        conn.close()

def gerar_dados_em_thread(numero_registros, caminho_db, thread_id, hoteis_por_cidade):
    registros = [gerar_registro_viagem(hoteis_por_cidade) for _ in range(numero_registros)]
    inserir_registros_no_banco(registros, caminho_db)
    print(f"Thread {thread_id} concluiu a geração de {numero_registros} registros.")

def main():
    base_dir = os.path.dirname(__file__)
    caminho_entrada_hoteis = os.path.join(base_dir, 'data/dados_hoteis_2025.csv')
    caminho_db = os.path.join(base_dir, 'data/dados_viagens_2025.db')
    total_registros = 10000
    num_threads = 4
    registros_por_thread = total_registros // num_threads

    hoteis_por_cidade = carregar_dicionario_hoteis(caminho_entrada_hoteis)
    criar_tabela_sqlite(caminho_db)

    print(f"Gerando {total_registros} registros de viagens em banco de dados SQLite usando {num_threads} threads...")

    with ThreadPoolExecutor(max_workers=num_threads) as executor:
        for i in range(num_threads):
            executor.submit(gerar_dados_em_thread, registros_por_thread, caminho_db, i+1, hoteis_por_cidade)

    print(f"Geração concluída! Os dados foram salvos em '{caminho_db}'")

if __name__ == "__main__":
    main()
