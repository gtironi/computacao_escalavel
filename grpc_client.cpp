#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "mock_server/proto/extractor.pb.h"
#include "mock_server/proto/extractor.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using extractor::ExtractorService;
using google::protobuf::Empty;
using extractor::AllDataResponse; // Now directly using AllDataResponse

class ExtractorClient {
public:
    ExtractorClient(std::shared_ptr<Channel> channel)
        : stub_(ExtractorService::NewStub(channel)) {}

    // New method to get all data and print a summary
    void GetAllDataAndPrintSummary() {
        Empty request;
        AllDataResponse response; // Use the combined response message
        ClientContext context;

        // Call the GetAllData RPC
        Status status = stub_->GetAllData(&context, request, &response);

        if (status.ok()) {
            std::cout << "OK" << std::endl;
            std::cout << "Número de voos recebidos: " << response.voos_size() << std::endl;
            std::cout << "Número de reservas recebidas: " << response.reservas_size() << std::endl;
            std::cout << "Número de pesquisas recebidas: " << response.pesquisas_size() << std::endl;

            // Optional: Print first few items to verify content (can be removed for simplest output)
            // if (response.voos_size() > 0) {
            //     std::cout << "Primeiro voo: " << response.voos(0).cidade_origem() << " -> " << response.voos(0).cidade_destino() << std::endl;
            // }
            // if (response.reservas_size() > 0) {
            //     std::cout << "Primeira reserva: " << response.reservas(0).nome_hotel() << " em " << response.reservas(0).cidade_destino() << std::endl;
            // }
            // if (response.pesquisas_size() > 0) {
            //     std::cout << "Primeira pesquisa: " << response.pesquisas(0).cidade_origem() << " -> " << response.pesquisas(0).cidade_destino() << std::endl;
            // }   

        } else {
            std::cerr << "Erro na chamada gRPC: " << status.error_message() << std::endl;
            std::cerr << "Código de erro: " << status.error_code() << std::endl;
        }
    }

private:
    std::unique_ptr<ExtractorService::Stub> stub_;
};

int main() {
    // It's good practice to set max message sizes, especially for large datasets.
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(128 * 1024 * 1024); // Increased to 128 MB for potentially very large datasets
    args.SetMaxSendMessageSize(16 * 1024 * 1024);    // 16 MB (client usually sends small requests)

    // Create a channel to connect to the gRPC server
    auto channel = grpc::CreateCustomChannel("localhost:50051", grpc::InsecureChannelCredentials(), args);
    
    // Create the client instance
    ExtractorClient client(channel);
    
    // Call the new method to get all data and print summary
    client.GetAllDataAndPrintSummary();

    return 0;
}