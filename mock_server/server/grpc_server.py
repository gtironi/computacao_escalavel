# server/grpc_server.py
import grpc
from concurrent import futures
from mock_server.proto import extractor_pb2
from mock_server.proto import extractor_pb2_grpc
from mock_server.server import generate_voos, generate_reservas, generate_pesquisas

class ExtractorServicer(extractor_pb2_grpc.ExtractorServiceServicer):
    def GetFlightData(self, request, context):
        return extractor_pb2.FlightDataResponse(voos=generate_voos.gerar_voos())

    def GetReservaData(self, request, context):
        return extractor_pb2.ReservaDataResponse(reservas=generate_reservas.gerar_reservas())

    def GetPesquisaData(self, request, context):
        return extractor_pb2.PesquisaDataResponse(pesquisas=generate_pesquisas.gerar_pesquisas())

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    extractor_pb2_grpc.add_ExtractorServiceServicer_to_server(ExtractorServicer(), server)
    server.add_insecure_port('[::]:50051')
    server.start()
    print("Servidor gRPC iniciado na porta 50051...")
    server.wait_for_termination()

if __name__ == "__main__":
    serve()
