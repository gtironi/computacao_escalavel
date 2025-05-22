# server/grpc_server.py
import grpc
from concurrent import futures
from mock_server.proto import extractor_pb2
from mock_server.proto import extractor_pb2_grpc
from mock_server.server import generate_all

class ExtractorServicer(extractor_pb2_grpc.ExtractorServiceServicer):   
    def GetAllData(self, request, context):
        all_reservas, all_pesquisas, all_voos = generate_all.gerar_dados()
        response = extractor_pb2.AllDataResponse() # Create an empty response object

        # Use extend() to add the lists of protobuf objects to the repeated fields
        response.reservas.extend(all_reservas)
        response.pesquisas.extend(all_pesquisas)
        response.voos.extend(all_voos)

        return response # Return the populated response object



def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    extractor_pb2_grpc.add_ExtractorServiceServicer_to_server(ExtractorServicer(), server)
    server.add_insecure_port('[::]:50051')
    server.start()
    print("Servidor gRPC iniciado na porta 50051...")
    server.wait_for_termination()

if __name__ == "__main__":
    serve()
