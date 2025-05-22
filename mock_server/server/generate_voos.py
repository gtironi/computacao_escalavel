# server/generate_voos.py

import random
from datetime import datetime, timedelta
from mock_server.proto import extractor_pb2

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

def gerar_voos():
    inicio_ano = datetime(2025, 4, 7)
    fim_ano = datetime(2025, 12, 31)
    todas_datas = [(inicio_ano + timedelta(days=i)) for i in range((fim_ano - inicio_ano).days + 1)]

    voos = []
    for data in todas_datas:
        dia, mes, ano = data.day, data.month, data.year
        for origem in cidades:
            destinos_possiveis = [c for c in cidades if c != origem]
            num_voos = random.randint(5, 15)
            destinos = random.sample(destinos_possiveis, min(num_voos, len(destinos_possiveis)))
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
                
                voos.append(voo)

    return voos

