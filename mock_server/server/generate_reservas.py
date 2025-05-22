# server/generate_reservas.py
import random
from datetime import datetime, timedelta
from faker import Faker
from mock_server.proto import extractor_pb2

fake = Faker("pt_BR")
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


tipos_quarto = ["Standard", "Deluxe", "Suíte", "Executivo", "Presidencial"]

def gerar_reservas():
    datas = [(datetime(2025, 4, 7) + timedelta(days=i)) for i in range((datetime(2025, 12, 31) - datetime(2025, 4, 7)).days + 1)]
    reservas = []

    for cidade in cidades:
        for _ in range(5):  # 5 hotéis por cidade
            nome_hotel = f"Hotel {fake.last_name()} {random.choice(['Resort', 'Plaza', 'Suites'])}"
            for tipo in tipos_quarto:
                for data in datas:
                    reservas.append(extractor_pb2.ReservaRow(
                        tipo_quarto=tipo,
                        nome_hotel=nome_hotel,
                        cidade_destino=cidade,
                        numero_quarto=random.randint(1, 100),
                        quantidade_pessoas=random.randint(1, 4),
                        preco=round(random.uniform(150, 1200), 2),
                        ocupado=random.choice([True, False]),
                        data_ida_dia=data.day,
                        data_ida_mes=data.month,
                        data_ida_ano=data.year
                    ))

    return reservas
