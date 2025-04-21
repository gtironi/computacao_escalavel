import os
import csv
import random
from faker import Faker
from datetime import datetime, timedelta
from threading import Lock
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor

# Configurações iniciais
fake = Faker('pt_BR')
random.seed(42)
lock = Lock()

cidades = [
    "São Paulo", "Rio de Janeiro", "Brasília", "Belo Horizonte", "Salvador",
    "Recife", "Fortaleza", "Curitiba", "Porto Alegre", "Belém", "Manaus",
    "Florianópolis", "Goiânia", "Natal", "Maceió", "João Pessoa", "Aracaju",
    "Vitória", "Cuiabá", "Campo Grande", "São Luís", "Teresina", "Palmas",
    "Rio Branco", "Macapá", "Boa Vista", "Porto Velho", "Nova York", "Miami",
    "Los Angeles", "Londres", "Paris", "Tóquio", "Pequim", "Dubai", "Roma",
    "Madri", "Lisboa", "Amsterdã", "Frankfurt", "Toronto", "Buenos Aires",
    "Santiago", "Lima", "Cidade do México", "Bogotá"
]

tipo_quarto_multiplicadores = {
    "Standard": 1.0,
    "Deluxe": 1.5,
    "Suíte": 2.0,
    "Executivo": 3.0,
    "Presidencial": 5.0
}

def gerar_datas_2025():
    inicio = datetime(2025, 4, 7)
    fim = datetime(2025, 12, 31)
    delta = (fim - inicio).days + 1
    return [(inicio + timedelta(days=i)) for i in range(delta)]

datas_2025 = gerar_datas_2025()

def distribuir_hoteis_por_cidade(total_hoteis, cidades):
    hoteis_por_cidade = defaultdict(int)
    cidades_sorteadas = random.choices(cidades, k=total_hoteis)
    for cidade in cidades_sorteadas:
        hoteis_por_cidade[cidade] += 1
    return hoteis_por_cidade

def calcular_fator_preco(hoteis_por_cidade):
    max_hoteis = max(hoteis_por_cidade.values())
    fator_preco = {}
    for cidade, qtd in hoteis_por_cidade.items():
        fator = 1.0 + ((max_hoteis - qtd) / max_hoteis) * 2  # 1.0 a 3.0
        fator_preco[cidade] = fator
    return fator_preco

def dividir_dict(dict_original, num_partes):
    items = list(dict_original.items())
    return [dict(items[i::num_partes]) for i in range(num_partes)]

# Função para definir os tipos de quarto que o hotel terá
def definir_tipos_quarto_para_hotel(qtd_hoteis_na_cidade):
    tipos = ["Standard", "Deluxe"]

    if random.random() < 0.8:
        tipos.append("Suíte")
    if random.random() < 0.4:
        tipos.append("Executivo")

    chance_presidencial = 0.2 if qtd_hoteis_na_cidade < 10 else 0.1
    if random.random() < chance_presidencial:
        tipos.append("Presidencial")

    return tipos

def gerar_dados_em_thread(hoteis_por_cidade_thread, fator_preco_cidade, arquivo_saida, id_thread):
    registros = []

    for cidade, qtd_hoteis in hoteis_por_cidade_thread.items():
        for _ in range(qtd_hoteis):
            nome_hotel = f"Hotel {fake.last_name()} {random.choice(['Palace', 'Resort', 'Inn', 'Suites', 'Plaza'])}"
            num_quartos = random.randint(5, 20)
            tipos_quarto_hotel = definir_tipos_quarto_para_hotel(qtd_hoteis)

            # Distribui os quartos proporcionalmente entre os tipos disponíveis
            quartos_por_tipo = random.choices(tipos_quarto_hotel, k=num_quartos)

            for numero_quarto, tipo_quarto in enumerate(quartos_por_tipo, start=1):
                capacidade = random.randint(2, 4)
                preco_base = random.randint(300, 1000)
                fator_aleatorio = random.uniform(1.0, 2.5)
                multiplicador_tipo = tipo_quarto_multiplicadores[tipo_quarto]
                preco = round(preco_base * fator_preco_cidade[cidade] * fator_aleatorio * multiplicador_tipo, 2)

                for data in datas_2025:
                    dia = data.day
                    mes = data.month
                    ano = data.year
                    ocupado = random.random() < 0.6
                    registros.append([
                        tipo_quarto,
                        nome_hotel,
                        cidade,
                        numero_quarto,
                        capacidade,
                        preco,
                        1 if ocupado else 0,
                        dia,
                        mes,
                        ano
                    ])

    with lock:
        with open(arquivo_saida, 'a', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerows(registros)
        print(f"[Thread-{id_thread}] Gerou {len(registros)} registros.")

def main():
    arquivo_saida = os.path.join(os.path.dirname(__file__), 'data/dados_hoteis_2025.csv')
    num_threads = 6  ## Mudar aqui para aumentar a base -------------------------------------------
    hoteis_por_thread = 30   ## Mudar aqui para aumentar a base ------------------------------------------- Total de hotéis = threads * hoteis_por_thread
    total_hoteis = num_threads * hoteis_por_thread
    os.makedirs(os.path.dirname(arquivo_saida), exist_ok=True)

    hoteis_por_cidade = distribuir_hoteis_por_cidade(total_hoteis, cidades)
    fator_preco_cidade = calcular_fator_preco(hoteis_por_cidade)

    with open(arquivo_saida, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow([
            'tipo_quarto', 'nome_hotel', 'cidade',  'numero_quarto',
            'quantidade_pessoas', 'preco', 'ocupado', 'dia', 'mes', 'ano'
        ])

    print(f"Gerando dados com {total_hoteis} hotéis entre 07/04 e 31/12/2025 usando {num_threads} threads...\n")

    partes_por_thread = dividir_dict(hoteis_por_cidade, num_threads)

    with ThreadPoolExecutor(max_workers=num_threads) as executor:
        for i in range(num_threads):
            executor.submit(
                gerar_dados_em_thread,
                partes_por_thread[i],
                fator_preco_cidade,
                arquivo_saida,
                i + 1
            )

if __name__ == "__main__":
    main()
