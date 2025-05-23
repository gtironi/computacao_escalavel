# server/grpc_server.py
import grpc
from concurrent import futures
from mock_client.proto import extractor_pb2
from mock_client.proto import extractor_pb2_grpc
from mock_client.client import generate_all

class ExtractorClient(extractor_pb2_grpc.ExtractorServiceServicer):   
    def GetAllData(self, request, context):
        all_reservas, all_pesquisas, all_voos = generate_all.gerar_dados()
        response = extractor_pb2.AllDataResponse() # Create an empty response object

        # Use extend() to add the lists of protobuf objects to the repeated fields
        response.reservas.extend(all_reservas)
        response.pesquisas.extend(all_pesquisas)
        response.voos.extend(all_voos)

        return response # Return the populated response object


def run():
    server_address = 'localhost:50051'

    print(f"Attempting to connect to gRPC server at {server_address}...")
    with grpc.insecure_channel(server_address) as channel:
        all_reservas, all_pesquisas, all_voos = generate_all.gerar_dados()
        print(type(all_reservas))
        stub = extractor_pb2_grpc.ExtractorServiceStub(channel)
        request_data = extractor_pb2.AllDataSend(
            reservas=all_reservas,
            voos=all_voos,
            pesquisas=all_pesquisas
        )
        try:
                # Call the GetAllData RPC method on the C++ server
                print("Calling GetAllData RPC on the C++ server...")
                response = stub.GetAllData(request_data, timeout=10) # Added a timeout for robustness

                # Process the response from the C++ server
                print("\n--- Received Response from C++ Server ---")
                print(f"Stats 1: {response.stats1}")
                print(f"Stats 2: {response.stats2}")
                print(f"Stats 3: {response.stats3}")
                print(f"Stats 4: {response.stats4}")
                print(f"Stats 5: {response.stats5}")
                print("-----------------------------------------")

                # If your C++ server were to return the lists (voos, reservas, pesquisas)
                # in the AllDataResponse, you would access them here:
                # print("\nReturned Voos:")
                # for voo in response.voos:
                #     print(f"  Origin: {voo.cidade_origem}, Destination: {voo.cidade_destino}")

        except grpc.RpcError as e:
            print(f"Error during gRPC call: {e.code()} - {e.details()}")
            if e.code() == grpc.StatusCode.UNAVAILABLE:
                print(f"Server at {server_address} is unavailable. Make sure your C++ server is running.")
            elif e.code() == grpc.StatusCode.DEADLINE_EXCEEDED:
                print("The gRPC call timed out. The server might be slow or unresponsive.")
        except Exception as e:
            print(f"An unexpected error occurred: {e}")

if __name__ == "__main__":
    run()
