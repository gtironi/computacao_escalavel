import csv
import random
from datetime import datetime, timedelta
import os
from concurrent.futures import ThreadPoolExecutor

# Lista de cidades
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

# Intervalo de datas
inicio_ano = datetime(2025, 4, 7)
fim_ano = datetime(2025, 12, 31)
todas_datas = [(inicio_ano + timedelta(days=i)) for i in range((fim_ano - inicio_ano).days + 1)]

# Função que uma thread vai executar
def gerar_dados_voos_em_thread(datas, arquivo_saida, thread_id):
    with open(arquivo_saida, 'a', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        for data in datas:
            dia, mes, ano = data.day, data.month, data.year
            for origem in cidades:
                destinos_possiveis = [c for c in cidades if c != origem]
                num_voos = random.randint(7, 18) ## Mudar aqui para aumentar a base -------------------------------------------
                destinos = random.sample(destinos_possiveis, min(num_voos, len(destinos_possiveis)))
                for destino in destinos:
                    assentos_totais = random.choice([100, 200])
                    taxa_ocupacao = random.uniform(0.2, 0.95)
                    assentos_ocupados = int(assentos_totais * taxa_ocupacao)
                    assentos_disponiveis = assentos_totais - assentos_ocupados
                    writer.writerow([
                        origem, destino, assentos_ocupados, assentos_totais,
                        assentos_disponiveis, dia, mes, ano
                    ])

def main():
    arquivo_saida = os.path.join(os.path.dirname(__file__), 'data/dados_voos_2025.csv')
    num_threads = 4
    datas_por_thread = len(todas_datas) // num_threads

    # Criar o arquivo e escrever o cabeçalho
    with open(arquivo_saida, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow([
            'cidade_origem', 'cidade_destino', 'assentos_ocupados',
            'assentos_totais', 'assentos_disponiveis', 'dia', 'mes', 'ano'
        ])

    print(f"Gerando dados de voos entre {len(cidades)} cidades de {inicio_ano.date()} a {fim_ano.date()} usando {num_threads} threads...")

    # Criar as threads
    with ThreadPoolExecutor(max_workers=num_threads) as executor:
        for i in range(num_threads):
            inicio = i * datas_por_thread
            fim = (i + 1) * datas_por_thread if i < num_threads - 1 else len(todas_datas)
            executor.submit(gerar_dados_voos_em_thread, todas_datas[inicio:fim], arquivo_saida, i + 1)

    print(f"Arquivo gerado com sucesso: {arquivo_saida}")

if __name__ == "__main__":
    main()
