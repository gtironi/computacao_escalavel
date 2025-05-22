# server/generate_pesquisas.py
import random
import datetime
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

def gerar_pesquisas():
    pesquisas = []
    for _ in range(1000):
        origem = random.choice(cidades)
        destino = random.choice([c for c in cidades if c != origem])
        data_ida = datetime.date(2025, 4, 7) + datetime.timedelta(days=random.randint(0, 200))
        data_volta = data_ida + datetime.timedelta(days=random.randint(1, 14))
        hotel = f"Hotel {random.choice(['Luxo', 'Confort', 'Sol'])}"
        # print(extractor_pb2.PesquisaRow(
        #     cidade_origem=origem,
        #     cidade_destino=destino,
        #     nome_hotel=hotel,
        #     data_ida_dia=data_ida.day,
        #     data_ida_mes=data_ida.month,
        #     data_ida_ano=data_ida.year,
        #     data_volta_dia=data_volta.day,
        #     data_volta_mes=data_volta.month,
        #     data_volta_ano=data_volta.year
        # ))
        pesquisas.append(extractor_pb2.PesquisaRow(
            cidade_origem=origem,
            cidade_destino=destino,
            nome_hotel=hotel,
            data_ida_dia=data_ida.day,
            data_ida_mes=data_ida.month,
            data_ida_ano=data_ida.year,
            data_volta_dia=data_volta.day,
            data_volta_mes=data_volta.month,
            data_volta_ano=data_volta.year
        ))

    return pesquisas

# if __name__== "__main__":
#     gerar_pesquisas()