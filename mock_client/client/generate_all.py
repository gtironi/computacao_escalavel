import random
import csv
import io
import os
from faker import Faker
from datetime import datetime, timedelta
from concurrent.futures import ThreadPoolExecutor
from collections import defaultdict
from mock_client.proto import extractor_pb2

fake = Faker('pt_BR')

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

# --- Gera Reservas ---

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
    if not hoteis_por_cidade:
        return {}
    max_hoteis = max(hoteis_por_cidade.values())
    fator_preco = {}
    for cidade, qtd in hoteis_por_cidade.items():
        fator = 1.0 + ((max_hoteis - qtd) / max_hoteis) * 2
        fator_preco[cidade] = fator
    return fator_preco

def dividir_dict(dict_original, num_partes):
    items = list(dict_original.items())
    return [dict(items[i::num_partes]) for i in range(num_partes)]

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

def gerar_reservas_em_thread(hoteis_por_cidade_thread, fator_preco_cidade):
    reservas_thread = []
    hoteis_gerados_nesta_thread = defaultdict(set)

    for cidade, qtd_hoteis in hoteis_por_cidade_thread.items():
        for _ in range(qtd_hoteis):
            nome_hotel = f"Hotel {fake.last_name()} {random.choice(['Palace', 'Resort', 'Inn', 'Suites', 'Plaza'])}"
            hoteis_gerados_nesta_thread[cidade].add(nome_hotel)
            num_quartos = random.randint(5, 20)
            tipos_quarto_hotel = definir_tipos_quarto_para_hotel(qtd_hoteis)

            quartos_por_tipo = random.choices(tipos_quarto_hotel, k=num_quartos)

            for numero_quarto, tipo_quarto in enumerate(quartos_por_tipo, start=1):
                capacidade = random.randint(2, 4)
                preco_base = random.randint(300, 1000)
                fator_aleatorio = random.uniform(1.0, 2.5)
                multiplicador_tipo = tipo_quarto_multiplicadores[tipo_quarto]
                preco = round(preco_base * fator_preco_cidade[cidade] * fator_aleatorio * multiplicador_tipo, 2)
                
                for data in datas_2025:
                    ocupado = random.random() < 0.6
                    reservas_thread.append(extractor_pb2.ReservaRow(
                        tipo_quarto=tipo_quarto,
                        nome_hotel=nome_hotel,
                        cidade_destino=cidade,
                        numero_quarto=numero_quarto,
                        quantidade_pessoas=capacidade,
                        preco=preco,
                        ocupado=(1 if ocupado else 0),
                        data_ida_dia=data.day,
                        data_ida_mes=data.month,
                        data_ida_ano=data.year
                    ))
    return reservas_thread, hoteis_gerados_nesta_thread


# --- Gera Pesquisas ---

def data_aleatoria_2025():
    inicio_ano = datetime(2025, 4, 7).date()
    fim_ano = datetime(2025, 12, 31).date()
    delta_dias = (fim_ano - inicio_ano).days
    dias_aleatorios = random.randint(0, delta_dias)
    return inicio_ano + timedelta(days=dias_aleatorios)

def gerar_pesquisa_individual(hoteis_por_cidade_disponiveis):
    origem = random.choice(cidades)
    destino = random.choice([c for c in cidades if c != origem])
    
    data_ida = data_aleatoria_2025()
    dias_estadia = random.randint(1, 15)
    data_volta = data_ida + timedelta(days=dias_estadia)

    hoteis_destino = hoteis_por_cidade_disponiveis.get(destino, [])
    if hoteis_destino:
        hotel_escolhido = random.choice(hoteis_destino)
    else:
        hotel_escolhido = f"Hotel {random.choice(['Genérico', 'Qualquer'])}"

    return extractor_pb2.PesquisaRow(
        cidade_origem=origem,
        cidade_destino=destino,
        nome_hotel=hotel_escolhido,
        data_ida_dia=data_ida.day,
        data_ida_mes=data_ida.month,
        data_ida_ano=data_ida.year,
        data_volta_dia=data_volta.day,
        data_volta_mes=data_volta.month,
        data_volta_ano=data_volta.year
    )

def gerar_pesquisas_em_thread(numero_pesquisas, thread_id, hoteis_por_cidade_disponiveis):
    pesquisas_thread = [gerar_pesquisa_individual(hoteis_por_cidade_disponiveis) for _ in range(numero_pesquisas)]
    # print(f"Thread {thread_id} de Pesquisas concluiu a geração de {numero_pesquisas} registros.")
    return pesquisas_thread

# Gera Voos

def gerar_voos_para_data_e_origem(data, origem):
    voos_gerados = []
    dia, mes, ano = data.day, data.month, data.year
    destinos_possiveis = [c for c in cidades if c != origem]
    num_voos_por_origem = random.randint(7, 18)
    # Garante que não tente pegar mais destinos do que o disponível
    destinos = random.sample(destinos_possiveis, min(num_voos_por_origem, len(destinos_possiveis)))
    
    for destino in destinos:
        assentos_totais = random.choice([100, 200])
        taxa_ocupacao = random.uniform(0.2, 0.95)
        assentos_ocupados = int(assentos_totais * taxa_ocupacao)
        assentos_disponiveis = assentos_totais - assentos_ocupados

        voo = extractor_pb2.FlightRow(
            cidade_origem=origem,
            cidade_destino=destino,
            assentos_ocupados=assentos_ocupados,
            assentos_totais=assentos_totais,
            assentos_disponiveis=assentos_disponiveis,
            dia=dia,
            mes=mes,
            ano=ano
        )
        voos_gerados.append(voo)
    return voos_gerados

def gerar_voos_em_thread(partes_combinadas_thread, thread_id):
    voos_thread = []
    for data, origem in partes_combinadas_thread:
        voos_thread.extend(gerar_voos_para_data_e_origem(data, origem))
    # print(f"Thread {thread_id} de Voos concluiu a geração de {len(voos_thread)} registros.")
    return voos_thread


# main_function
def gerar_dados():
    total_hoteis_para_gerar = 80
    num_threads_reservas = 2
    num_threads_pesquisas = 3
    total_pesquisas_para_gerar = 10000
    num_threads_voos = 2 # Número de threads para geração de voos

    # print(f"Iniciando a geração de dados de reservas, pesquisas e voos...\n")

    # Geração de dados de Reservas
    hoteis_por_cidade_distribuidos = distribuir_hoteis_por_cidade(total_hoteis_para_gerar, cidades)
    fator_preco_cidade = calcular_fator_preco(hoteis_por_cidade_distribuidos)
    partes_por_thread_reservas = dividir_dict(hoteis_por_cidade_distribuidos, num_threads_reservas)

    todas_reservas = []
    hoteis_disponiveis_para_pesquisa = defaultdict(set)

    with ThreadPoolExecutor(max_workers=num_threads_reservas) as executor:
        future_to_reservas = {executor.submit(gerar_reservas_em_thread, partes_por_thread_reservas[i], fator_preco_cidade): i for i in range(num_threads_reservas)}
        for future in future_to_reservas:
            reservas_geradas_thread, hoteis_gerados_thread = future.result()
            todas_reservas.extend(reservas_geradas_thread)
            for cidade, hoteis_set in hoteis_gerados_thread.items():
                hoteis_disponiveis_para_pesquisa[cidade].update(hoteis_set)

    # print(f"\nTotal de {len(todas_reservas)} objetos ReservaRow gerados.")

    # Converte os sets de hotéis para listas para random.choice
    hoteis_disponiveis_para_pesquisa = {cidade: list(hoteis) for cidade, hoteis in hoteis_disponiveis_para_pesquisa.items()}

    # Geração de dados de Pesquisas
    registros_por_thread_pesquisas = total_pesquisas_para_gerar // num_threads_pesquisas
    todas_pesquisas = []

    with ThreadPoolExecutor(max_workers=num_threads_pesquisas) as executor:
        future_to_pesquisas = {executor.submit(gerar_pesquisas_em_thread, registros_por_thread_pesquisas, i+1, hoteis_disponiveis_para_pesquisa): i for i in range(num_threads_pesquisas)}
        for future in future_to_pesquisas:
            pesquisas_geradas_thread = future.result()
            todas_pesquisas.extend(pesquisas_geradas_thread)

    # print(f"\nTotal de {len(todas_pesquisas)} objetos PesquisaRow gerados.")

    # Geração de dados de Voos
    todas_combinacoes_data_origem = []
    for data in datas_2025: # datas_2025 já está definida globalmente
        for origem_cidade in cidades:
            todas_combinacoes_data_origem.append((data, origem_cidade))

    # Divide as combinações entre as threads para processamento paralelo
    partes_combinadas_por_thread = [
        todas_combinacoes_data_origem[i::num_threads_voos] for i in range(num_threads_voos)
    ]
    
    todos_voos = []
    with ThreadPoolExecutor(max_workers=num_threads_voos) as executor:
        future_to_voos = {executor.submit(gerar_voos_em_thread, partes_combinadas_por_thread[i], i+1): i for i in range(num_threads_voos)}
        for future in future_to_voos:
            voos_gerados_thread = future.result()
            todos_voos.extend(voos_gerados_thread)

    # print(f"\nTotal de {len(todos_voos)} objetos FlightRow gerados.")


    # 1. Generate Reserva CSV String
    reserva_output = io.StringIO()
    reserva_writer = csv.writer(reserva_output)
    reserva_headers = ["tipo_quarto", "nome_hotel", "cidade_destino", "numero_quarto",
                       "quantidade_pessoas", "preco", "ocupado", "data_ida_dia", "data_ida_mes", "data_ida_ano"]
    reserva_writer.writerow(reserva_headers)
    for reserva in todas_reservas:
        reserva_writer.writerow([
            reserva.tipo_quarto, reserva.nome_hotel, reserva.cidade_destino,
            reserva.numero_quarto, reserva.quantidade_pessoas, reserva.preco,
            int(reserva.ocupado), reserva.data_ida_dia, reserva.data_ida_mes, reserva.data_ida_ano
        ])
    reservas_csv = reserva_output.getvalue()

    # 2. Generate Pesquisa CSV String
    pesquisa_output = io.StringIO()
    pesquisa_writer = csv.writer(pesquisa_output)
    pesquisa_headers = ["cidade_origem", "cidade_destino", "nome_hotel",
                        "data_ida_dia", "data_ida_mes", "data_ida_ano",
                        "data_volta_dia", "data_volta_mes", "data_volta_ano"]
    pesquisa_writer.writerow(pesquisa_headers)
    for pesquisa in todas_pesquisas:
        pesquisa_writer.writerow([
            pesquisa.cidade_origem, pesquisa.cidade_destino, pesquisa.nome_hotel,
            pesquisa.data_ida_dia, pesquisa.data_ida_mes, pesquisa.data_ida_ano,
            pesquisa.data_volta_dia, pesquisa.data_volta_mes, pesquisa.data_volta_ano
        ])
    pesquisas_csv = pesquisa_output.getvalue()

    # 3. Generate Voo CSV String
    voo_output = io.StringIO()
    voo_writer = csv.writer(voo_output)
    voo_headers = ["cidade_origem", "cidade_destino", "assentos_ocupados",
                   "assentos_totais", "assentos_disponiveis", "dia", "mes", "ano"]
    voo_writer.writerow(voo_headers)
    for voo in todos_voos:
        voo_writer.writerow([
            voo.cidade_origem, voo.cidade_destino, voo.assentos_ocupados,
            voo.assentos_totais, voo.assentos_disponiveis, voo.dia, voo.mes, voo.ano
        ])
    voos_csv = voo_output.getvalue()

    
    # Return the three CSV strings
    return reservas_csv, pesquisas_csv, voos_csv
    

if __name__ == "__main__":
    reservas_geradas, pesquisas_geradas, voos_gerados = gerar_dados()