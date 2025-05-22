#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "mock_server/proto/extractor.grpc.pb.h"  // Gerado pelo protoc

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using extractor::ExtractorService;
using extractor::Empty;
using extractor::FlightDataResponse;

class ExtractorClient {
public:
    ExtractorClient(std::shared_ptr<Channel> channel)
        : stub_(ExtractorService::NewStub(channel)) {}

    void GetFlightDataAndPrint() {
        Empty request;
        FlightDataResponse response;
        ClientContext context;

        Status status = stub_->GetFlightData(&context, request, &response);

        if (status.ok()) {
            std::cout << "== Voos recebidos do servidor Python ==" << std::endl;
            for (const auto& voo : response.voos()) {
    std::cout << "Origem: " << voo.cidade_origem()
              << ", Destino: " << voo.cidade_destino()
              << ", Assentos Ocupados: " << voo.assentos_ocupados()
              << ", Assentos Totais: " << voo.assentos_totais()
              << ", Assentos Disponíveis: " << voo.assentos_disponiveis()
              << ", Data: " << voo.dia() << "/" << voo.mes() << "/" << voo.ano()
              << std::endl;
}

        } else {
            std::cerr << "Erro na chamada gRPC: " << status.error_message() << std::endl;
        }
    }

private:
    std::unique_ptr<ExtractorService::Stub> stub_;
};

int main() {
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(16 * 1024 * 1024);  // 16 MB
    args.SetMaxSendMessageSize(16 * 1024 * 1024);     // 16 MB

    auto channel = grpc::CreateCustomChannel("localhost:50051", grpc::InsecureChannelCredentials(), args);
    ExtractorClient client(channel);
    client.GetFlightDataAndPrint();
    return 0;
}
